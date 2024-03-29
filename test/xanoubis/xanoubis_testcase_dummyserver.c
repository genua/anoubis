/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis_sfs.h>
#endif
#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#include <sys/anoubis_alf.h>
#include <sys/anoubis_sfs.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

#include <anoubis_errno.h>
#include <anoubis_protocol.h>
#include <anoubis_notify.h>
#include <anoubis_server.h>
#include "config.h"
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if __clang__
/* help clang static analyzer with the test macros */
void _clang_fail(void) __attribute__((analyzer_noreturn));
#undef	fail_if
#undef	fail_unless
#define	fail_if(expr, ...)	do { if (expr) _clang_fail(); } while (0)
#define	fail_unless(expr, ...)	do { if (!(expr)) _clang_fail(); } while (0)
#endif

#define __used __attribute__((unused))

static char **tc_argv;
static int    tc_argc;

int
policy_dispatch(struct anoubis_policy_comm * policy, u_int64_t token,
    u_int32_t uid __used, const void * buf __used,  size_t len __used,
    void *arg __used, int flags __used)
{
	return anoubis_policy_comm_answer(policy, token, 0, NULL, 0, 1);
}

static void
auth_dummy(struct anoubis_server *server, struct anoubis_msg *m __used,
    uid_t auth_uid __used, void *arg __used)
{
	struct anoubis_auth	*auth = anoubis_server_getauth(server);

	auth->uid = auth->chan->euid;
	auth->state = ANOUBIS_AUTH_SUCCESS;
	auth->error = 0;
	auth->finish_callback(auth->cbdata);
}

static void
list_dummy(struct anoubis_server *server, struct anoubis_msg *m __used,
    uid_t auth_uid __used, void *arg __used)
{
	struct anoubis_msg *answer;

	answer = anoubis_msg_new(sizeof(Anoubis_ListMessage));
	set_value(answer->u.listreply->type, ANOUBIS_P_LISTREP);
	set_value(answer->u.listreply->flags,
	    POLICY_FLAG_START|POLICY_FLAG_END);
	set_value(answer->u.listreply->error, 0);
	set_value(answer->u.listreply->nrec, 0);
	set_value(answer->u.listreply->rectype,
	    get_value(m->u.listreq->listtype));
	anoubis_msg_send(anoubis_server_getchannel(server), answer);
	anoubis_msg_free(answer);
}

void
tc_Communicator_lud_server(const char *sockname)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
	struct achat_channel    *s  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;
	struct anoubis_server	*server;
	struct anoubis_policy_comm *policy;

	struct anoubis_msg		*statNotify;
	struct anoubis_msg		*currNotify;
	struct anoubis_stat_message	*statMsg;
	struct anoubis_notify_group	*ng;
	struct anoubis_notify_head	*head;
	size_t				 statSize;

	s = acc_create();
	fail_if(s == NULL, "couldn't create server channel");
	rc = acc_settail(s, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "server settail failed");
	rc = acc_setsslmode(s, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "server setsslmode failed");
	mark_point();

	bzero(&sa.ss, sizeof(sa.ss));
	sa.un.sun_family = AF_UNIX;
	strlcpy(sa.un.sun_path, sockname, sizeof(sa.un.sun_path));
	rc = acc_setaddr(s, &sa.ss, sizeof(struct sockaddr_un));
	if (rc != ACHAT_RC_OK)
		fail("server setaddr failed", __LINE__);
	mark_point();

	rc = acc_prepare(s);
	fail_if(rc != ACHAT_RC_OK, "server prepare failed");

	rc = acc_open(s);
	fail_if(rc != ACHAT_RC_OK, "server open failed");
	mark_point();

	policy = anoubis_policy_comm_create(&policy_dispatch, NULL);
	server = anoubis_server_create(s, policy);
	fail_if(server == NULL, "Failed to create server protocol");
	anoubis_dispatch_create(server, ANOUBIS_C_AUTHDATA, &auth_dummy, NULL);
	anoubis_dispatch_create(server, ANOUBIS_P_LISTREQ, &list_dummy, NULL);
	mark_point();

	/* assemble status notify */
	statSize = sizeof(Anoubis_NotifyMessage);
	statSize += sizeof(struct anoubis_stat_message);
	statSize += 2 * sizeof(struct anoubis_stat_value);
	statNotify = anoubis_msg_new(statSize);
	fail_if(statNotify == NULL, "Cannot allocate status message");
	set_value(statNotify->u.notify->type, ANOUBIS_N_NOTIFY);
	set_value(statNotify->u.notify->subsystem, ANOUBIS_SOURCE_STAT);
	statNotify->u.notify->token = 0x01;

	statMsg = (struct anoubis_stat_message*)(statNotify->u.notify)->payload;
	statMsg->vals[0].subsystem = ANOUBIS_SOURCE_ALF;
	statMsg->vals[0].key = ALF_STAT_LOADTIME;
	statMsg->vals[0].value = 1;
	statMsg->vals[1].subsystem = ANOUBIS_SOURCE_SFS;
	statMsg->vals[1].key = SFS_STAT_LOADTIME;
	statMsg->vals[1].value = 1;

	int ret = anoubis_server_start(server);
	mark_point();
	fail_if(ret < 0, "Failed to start server\n");
	while(1) {
		char buf[1024];
		size_t size;
		size = 1024;

		achat_rc rc = acc_receivemsg(s, buf, &size);
		fail_if(rc != ACHAT_RC_OK, "acc_receivemsg failed");
		ret = anoubis_server_process(server, buf, size);
		fail_if(ret < 0, "protocol error");
		if (anoubis_server_eof(server))
			break;

		ng = anoubis_server_getnotify(server);
		if (ng != NULL) {
			currNotify = anoubis_msg_clone(statNotify);
			head = anoubis_notify_create_head(currNotify, NULL,
			    NULL);
			fail_if(head == NULL, "Cannot create msg head");
			anoubis_notify(ng, head, 0);
			anoubis_notify_destroy_head(head);
		}
	}
	mark_point();
	anoubis_msg_free(statNotify);
	anoubis_server_destroy(server);
	rc = acc_destroy(s);
	fail_if(rc != ACHAT_RC_OK, "server destroy failed");
}

START_TEST(tc_dummyserver_args)
{
	fail_if(tc_argc != 2, "Number of cmd line arguments");
	fail_if(tc_argv[1] == NULL, "no socket specified for dummy server");
	if (access(tc_argv[1], R_OK | W_OK) != 0) {
		fail("Can't access given socket: %s", anoubis_strerror(errno));
	}
}
END_TEST

 START_TEST(tc_dummyserver_run)
{
	fail_if(tc_argv[1] == NULL, "no socket specified");
	tc_Communicator_lud_server(tc_argv[1]);
}
END_TEST

TCase *
xanoubis_testcase_dummyserver(int argc, char *argv[])
{
	/* Dummy server test case */
	TCase *tc_ds = tcase_create("Dummy server");

	tc_argv = argv;
	tc_argc = argc;

	tcase_set_timeout(tc_ds, 60);
	tcase_add_test(tc_ds, tc_dummyserver_args);
	tcase_add_test(tc_ds, tc_dummyserver_run);

	return (tc_ds);
}

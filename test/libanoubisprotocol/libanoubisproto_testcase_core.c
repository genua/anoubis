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

#include <config.h>
#include <check.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include <anoubis_protocol.h>
#include <anoubis_errno.h>
#include <anoubis_client.h>
#include <anoubis_server.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>
#include <anoubis_notify.h>

#include <anoubischat.h>

void tp_chat_lud_client(const char *sockname)
{
	struct sockaddr_storage	 ss;
	struct sockaddr_un	*ss_sun = (struct sockaddr_un *)&ss;
	struct achat_channel    *c  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;
	struct anoubis_client	*client;
	int			 ret;

	c = acc_create();
	fail_if(c == NULL, "couldn't create client channel");
	rc = acc_settail(c, ACC_TAIL_CLIENT);
	fail_if(rc != ACHAT_RC_OK, "client settail failed");
	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "client setsslmode failed");
	mark_point();

	bzero(&ss, sizeof(ss));
	ss_sun->sun_family = AF_UNIX;
	strncpy(ss_sun->sun_path, sockname, sizeof(ss_sun->sun_path) - 1 );
	rc = acc_setaddr(c, &ss);
	if (rc != ACHAT_RC_OK)
		fail("client setaddr failed", __LINE__);
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc != ACHAT_RC_OK, "client prepare failed");
	if (c->state != ACC_STATE_NOTCONNECTED)
		fail("client state not set correctly", __LINE__);

	sleep(4); /* give the server time to open his socket */
	rc = acc_open(c);
	fail_if(rc != ACHAT_RC_OK, "client open failed");
	if (c->state != ACC_STATE_ESTABLISHED)
		fail("client state not set correctly", __LINE__);
	mark_point();

	client = anoubis_client_create(c);
	fail_if(client == NULL, "Cannot create client");
	mark_point();

	struct anoubis_transaction * curr = anoubis_client_connect_start(
	    client, 3 /* XXX */);
	fail_if(!curr, "connect start");
	int k = 0;
	while(1) {
		struct anoubis_msg * m;
		size_t size = 1024;
		achat_rc rc;

		if (curr && curr->flags & ANOUBIS_T_DONE) {
			fail_if(curr->result, "Transaction error");
			anoubis_transaction_destroy(curr);
			curr = NULL;
		}
		if (curr == 0) {
			if (k >= 10)
				break;
			if (k < 5) {
				curr = anoubis_client_register_start(client,
				    0x123000+k, k+100, 0, 0, 0);
				fail_if(!curr, "register");
			} else {
				curr = anoubis_client_unregister_start(client,
				    0x123000+k, 109-k, 0, 0, 0);
				fail_if(!curr, "unregister");
			}
			k++;
		}
		m = anoubis_msg_new(1024);
		fail_if(!m, "Memalloc");
		rc = acc_receivemsg(c, m->u.buf, &size);
		fail_if(rc != ACHAT_RC_OK, "acc_receivemsg failed");
		anoubis_msg_resize(m, size);
		ret = anoubis_client_process(client, m);
		if (ret != 1)
			errno = -ret;
		fail_if(ret != 1, "protocol error");
	}

	sleep(2);

	anoubis_client_close(client);

	mark_point();

	anoubis_client_destroy(client);
	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "client destroy failed");
	mark_point();
	exit(0);
}


struct event_cbdata {
	struct anoubis_notify_group * grp;
	struct anoubis_msg * m;
};

/* Simplified callback function which works only for a single notify group */
static void notify_callback(void * data, int verdict, int delegate)
{
	struct event_cbdata * cbdata = data;
	anoubis_token_t token = cbdata->m->u.notify->token;
	if (delegate)
		anoubis_notify_end(cbdata->grp, token, ANOUBIS_E_IO, 0, 0);
	else
		anoubis_notify_end(cbdata->grp, token, verdict, 0, 1);
	anoubis_msg_free(cbdata->m);
	free(data);
}

/* XXX Simplified demo notify function. No error checking. */
static void do_notify(struct anoubis_notify_group * grp, pid_t pid)
{
	static u_int64_t token = 1;

	if (!grp)
		return;
	struct anoubis_msg * m = anoubis_msg_new(sizeof(Anoubis_NotifyMessage));
	struct event_cbdata * cb = malloc(sizeof(*cb));
	int ret;

	cb->grp = grp;
	cb->m = m;
	set_value(m->u.notify->type, ANOUBIS_N_ASK);
	m->u.notify->token = ++token;
	set_value(m->u.notify->pid, pid);
	set_value(m->u.notify->rule_id, 0);
	set_value(m->u.notify->uid, 0);
	set_value(m->u.notify->subsystem, 0);
	ret = anoubis_notify(grp, /* XXX */ pid, m, &notify_callback, cb);
	if (ret < 0) {
		anoubis_msg_free(m);
		free(cb);
	}
}

void tp_chat_lud_server(const char *sockname)
{
	struct sockaddr_storage	 ss;
	struct sockaddr_un	*ss_sun = (struct sockaddr_un *)&ss;
	struct achat_channel    *s  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;
	struct anoubis_server	*server;

	s = acc_create();
	fail_if(s == NULL, "couldn't create server channel");
	rc = acc_settail(s, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "server settail failed");
	rc = acc_setsslmode(s, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "server setsslmode failed");
	mark_point();

	bzero(&ss, sizeof(ss));
	ss_sun->sun_family = AF_UNIX;
	strncpy(ss_sun->sun_path, sockname, sizeof(ss_sun->sun_path) - 1 );
	rc = acc_setaddr(s, &ss);
	if (rc != ACHAT_RC_OK)
		fail("server setaddr failed", __LINE__);
	mark_point();

	rc = acc_prepare(s);
	fail_if(rc != ACHAT_RC_OK, "server prepare failed");
	if (s->state != ACC_STATE_NOTCONNECTED)
		fail("server state not set correctly", __LINE__);

	rc = acc_open(s);
	fail_if(rc != ACHAT_RC_OK, "server open failed");
	if (s->state != ACC_STATE_ESTABLISHED)
		fail("server state not set correctly", __LINE__);
	mark_point();

	server = anoubis_server_create(s);
	fail_if(server == NULL, "Failed to create server protocol");
	mark_point();

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
		while(ret > 0)
			ret = anoubis_server_continue(server);
		fail_if(ret < 0, "protocol error");
		if (anoubis_server_eof(server))
			break;
		do_notify(anoubis_server_getnotify(server), 0);
	}
	mark_point();
	anoubis_server_destroy(server);
	rc = acc_destroy(s);
	fail_if(rc != ACHAT_RC_OK, "server destroy failed");
	mark_point();
}

#define TPCONNECT_SOCKETDIR	"/tmp/"
#define TPCONNECT_SOCKETPREFIX	"aproto_socket."

START_TEST(tp_core_comm)
{
	char sockname[FILENAME_MAX];

	bzero(sockname, sizeof(sockname));
	snprintf(sockname, sizeof(sockname) - 1, "%s%s%s%08d",
	    TPCONNECT_SOCKETDIR, TPCONNECT_SOCKETPREFIX, "server", getpid());
	switch (fork()) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		tp_chat_lud_client(sockname);
		break;
	default:
		tp_chat_lud_server(sockname);
		break;
	}
	while(1) {
		pid_t pid = wait(NULL);
		if (pid >= 0)
			continue;
		if (pid == -1 && errno == ECHILD)
			break;
		fail_if(errno != EINTR, "error %d while waiting for children");
	}
	fail_if(access(sockname, F_OK) == 0, "socket %s not removed",
	    sockname);
}
END_TEST

TCase *
libanoubispc_testcase_core(void)
{
	/* Core test case */
	TCase *tp_core = tcase_create("Core");

	tcase_add_test(tp_core, tp_core_comm);
	tcase_set_timeout(tp_core, 30);

	return (tp_core);
}

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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <anoubis_protocol.h>
#include <anoubis_server.h>

#include <check.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char **tc_argv;
static int    tc_argc;

int policy_dispatch(struct anoubis_policy_comm * policy, u_int64_t token,
    u_int32_t uid, void * buf,  size_t len, void *arg)
{
	return anoubis_policy_comm_answer(policy, token, 0, NULL, 0);
}

void
tc_Communicator_lud_server(const char *sockname)
{
	struct sockaddr_storage	 ss;
	struct sockaddr_un	*ss_sun = (struct sockaddr_un *)&ss;
	struct achat_channel    *s  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;
	struct anoubis_server	*server;
	struct anoubis_policy_comm *policy;

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

	policy = anoubis_policy_comm_create(&policy_dispatch, NULL);
	server = anoubis_server_create(s, policy);
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
		fail_if(ret < 0, "protocol error");
		if (anoubis_server_eof(server))
			break;
	}
	mark_point();
	anoubis_server_destroy(server);
	rc = acc_destroy(s);
	fail_if(rc != ACHAT_RC_OK, "server destroy failed");
}

START_TEST(tc_dummyserver_args)
{
	fail_if(tc_argc != 2, "Number of cmd line arguments");
	fail_if(tc_argv[1] == NULL, "no socket specified");
	if (access(tc_argv[1], R_OK | W_OK) != 0) {
		fail("Can't access given socket: %s", strerror(errno));
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

	tcase_set_timeout(tc_ds, 20);
	tcase_add_test(tc_ds, tc_dummyserver_args);
	tcase_add_test(tc_ds, tc_dummyserver_run);

	return (tc_ds);
}
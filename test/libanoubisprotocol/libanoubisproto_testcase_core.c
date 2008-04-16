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
#include <anoubis_policy.h>
#include <anoubis_dump.h>

#include <anoubischat.h>

#define NNOTIFIES 100
struct {
	anoubis_token_t token;
	char sent_reply;
	char got_verdict;
} notifies[NNOTIFIES];
int nnotifies;

void handle_notify(struct anoubis_client * client, struct anoubis_msg * m)
{
	int i, opcode;
	anoubis_token_t token;

	fail_if(nnotifies >= NNOTIFIES, "Did not expect that many messages");
	fail_if(!VERIFY_FIELD(m, token, token), "Short message");
	opcode = get_value(m->u.token->type);
	token = m->u.token->token;
	for(i=0; i<nnotifies; ++i) {
		if (notifies[i].token == token)
			break;
	}
	if (opcode == ANOUBIS_N_NOTIFY || opcode == ANOUBIS_N_ASK) {
		fail_if(!VERIFY_LENGTH(m, sizeof(Anoubis_NotifyMessage)));
		anoubis_msg_free(m);
		fail_if(i != nnotifies);
		nnotifies++;
		notifies[i].token = token;
		notifies[i].sent_reply = 1;
		notifies[i].got_verdict = 0;
		anoubis_client_notifyreply(client, token, i, i%2);
		return;
	}
	fail_if(opcode == ANOUBIS_N_RESOTHER, "RESOTHER unexpected");
	fail_if(opcode != ANOUBIS_N_RESYOU, "Bad opcode");
	fail_if(!VERIFY_LENGTH(m, sizeof(Anoubis_NotifyResultMessage)),
	    "Short Message");
	fail_if(i == nnotifies, "result for nonexisting message");
	fail_if(notifies[i].got_verdict == 1, "Already got verdict");
	fail_if(get_value(m->u.notifyresult->error) != i, "Wrong verdict");
	notifies[i].got_verdict = 1;
}

char * trans = "reg";

void tp_chat_lud_client(const char *sockname)
{
	struct sockaddr_storage	 ss;
	struct sockaddr_un	*ss_sun = (struct sockaddr_un *)&ss;
	struct achat_channel    *c  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;
	struct anoubis_client	*client;
	int			 i, ret;
	struct anoubis_transaction * curr, * policy = NULL;

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

	curr = anoubis_client_connect_start(client, ANOUBIS_PROTO_BOTH);
	fail_if(!curr, "connect start");
	int k = 0;
	while(1) {
		struct anoubis_msg * m;
		size_t size = 1024;
		achat_rc rc;

		if (curr && (curr->flags & ANOUBIS_T_DONE)) {
			if (k == 11)
				break;
			if (geteuid() == 0) {
				fail_if(curr->result, "Transaction error");
			} else {
				if (k==0 || k==1 || k== 10) {
					fail_if(curr->result != 0,
					    "Transaction error");
				} else {
					fail_if(curr->result == 0,
					    "Transaction error expected");
				}
			}
			anoubis_transaction_destroy(curr);
			curr = NULL;
		}
		if (policy && (policy->flags & ANOUBIS_T_DONE)) {
			fail_if(policy->result, "Transaction error");
			anoubis_transaction_destroy(policy);
			policy = NULL;
		}
		if (curr == 0) {
			if (k == 10) {
				trans = "close";
				curr = anoubis_client_close_start(client);
				fail_if(!curr, "close start failed");
				k++;
			} else if (k < 5) {
				curr = anoubis_client_register_start(client,
				    0x123000+k, geteuid()+k, 0, 0);
				fail_if(!curr, "register");
			} else {
				curr = anoubis_client_unregister_start(client,
				    0x123000+k, geteuid()+9-k, 0, 0);
				fail_if(!curr, "unregister");
			}
			k++;
		}
		if (k && k < 10 && policy == NULL) {
			char data[] = "Hello World";
			policy = anoubis_client_policyrequest_start(client,
			    data, sizeof(data));
			fail_if(!policy, "Cannot create policy request");
		}
		m = anoubis_msg_new(1024);
		fail_if(!m, "Memalloc");
		rc = acc_receivemsg(c, m->u.buf, &size);
		fail_if(rc != ACHAT_RC_OK, "acc_receivemsg failed");
		anoubis_msg_resize(m, size);
		anoubis_dump(m, "client recv");
		ret = anoubis_client_process(client, m);
		if (ret != 1)
			errno = -ret;
		fail_if(ret != 1, "protocol error");
		while(anoubis_client_hasnotifies(client)) {
			handle_notify(client, anoubis_client_getnotify(client));
		}
	}
	fail_if(nnotifies == 0, "Did not receive notifies");
	for (i=0; i<nnotifies; ++i) {
		fail_if(notifies[i].sent_reply != notifies[i].got_verdict,
		    "Did not receive verdict for token 0x%llx (sent=%d go=%d)",
		    notifies[i].token, notifies[i].sent_reply,
		    notifies[i].got_verdict);
	}
	mark_point();

	anoubis_client_destroy(client);
	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "client destroy failed");
	mark_point();
	exit(0);
}


/* Simplified callback function which does not care about the event. */
static void notify_callback(struct anoubis_notify_head * head, int verdict,
    void * cbdata)
{
	anoubis_notify_destroy_head(head);
}

/* Demo notify function. No error checking. */
static void do_notify(struct anoubis_notify_group * grp)
{
	static u_int64_t token = 1;

	if (!grp)
		return;
	struct anoubis_msg * m = anoubis_msg_new(sizeof(Anoubis_NotifyMessage));
	struct anoubis_notify_head * head;
	int ret;

	set_value(m->u.notify->type, ANOUBIS_N_ASK);
	m->u.notify->token = ++token;
	set_value(m->u.notify->pid, 0);
	set_value(m->u.notify->rule_id, 0);
	set_value(m->u.notify->uid, geteuid());
	set_value(m->u.notify->subsystem, 0);
	set_value(m->u.notify->operation, 0);
	head = anoubis_notify_create_head(m, &notify_callback, NULL);
	if (!head < 0) {
		anoubis_msg_free(m);
	}
	ret = anoubis_notify(grp, head);
	fail_if(ret < 0);
	if (ret == 0)
		anoubis_notify_destroy_head(head);
}

int policy_dispatch(struct anoubis_policy_comm * policy, u_int64_t token,
    u_int32_t uid, void * buf,  size_t len, void *arg)
{
	return anoubis_policy_comm_answer(policy, token, 0, NULL, 0);
}

void tp_chat_lud_server(const char *sockname)
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
		do_notify(anoubis_server_getnotify(server));
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
libanoubisproto_testcase_core(void)
{
	/* Core test case */
	TCase *tp_core = tcase_create("Core");

	tcase_add_test(tp_core, tp_core_comm);
	tcase_set_timeout(tp_core, 30);

	return (tp_core);
}

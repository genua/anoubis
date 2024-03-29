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
#include <anoubis_crc32.h>
#include <anoubis_dump.h>

#include <anoubis_chat.h>
#include <anoubischeck.h>
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#define __used __attribute__((unused))
#define NNOTIFIES 100
struct {
	anoubis_token_t token;
	char sent_reply;
	char got_verdict;
} notifies[NNOTIFIES];
unsigned int nnotifies;

void handle_notify(struct anoubis_client * client, struct anoubis_msg * m)
{
	unsigned int		i;
	int			opcode;
	anoubis_token_t	token;

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
	fail_if(get_value(m->u.notifyresult->error) != i,
	    "Wrong verdict");
	notifies[i].got_verdict = 1;
}

char * trans = "reg";

void tp_chat_lud_client(const char *sockname)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
	struct achat_channel    *c  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;
	struct anoubis_client	*client;
	unsigned int		 i;
	int			 ret;
	struct anoubis_transaction * curr, * policy = NULL;
	int			 v_server, vmin_server, v_select;

	c = acc_create();
	fail_if(c == NULL, "couldn't create client channel");
	rc = acc_settail(c, ACC_TAIL_CLIENT);
	fail_if(rc != ACHAT_RC_OK, "client settail failed");
	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "client setsslmode failed");
	mark_point();

	bzero(&sa.ss, sizeof(sa.ss));
	sa.un.sun_family = AF_UNIX;
	strlcpy(sa.un.sun_path, sockname, sizeof(sa.un.sun_path));
	rc = acc_setaddr(c, &sa.ss, sizeof(struct sockaddr_un));
	if (rc != ACHAT_RC_OK)
		fail("client setaddr failed", __LINE__);
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc != ACHAT_RC_OK, "client prepare failed");

	sleep(4); /* give the server time to open his socket */
	rc = acc_open(c);
	fail_if(rc != ACHAT_RC_OK, "client open failed");
	mark_point();

	client = anoubis_client_create(c, ANOUBIS_AUTH_TRANSPORT, NULL);
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
				if (k==0 || k==1 || k==10) {
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

	v_server = anoubis_client_serverversion(client);
	vmin_server = anoubis_client_serverminversion(client);
	v_select = anoubis_client_selectedversion(client);
	fail_unless(v_server == ANOUBIS_PROTO_VERSION,
	    "Wrong protocol version (%i)", v_server);
	fail_unless(vmin_server == ANOUBIS_PROTO_MINVERSION,
	    "Wrong protocol min version (%i)", vmin_server);
	fail_unless(v_select == ANOUBIS_PROTO_VERSION,
	    "Wrong version selected (%d)", v_select);

	anoubis_client_destroy(client);
	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "client destroy failed");
	mark_point();
	exit(0);
}


/* Simplified callback function which does not care about the event. */
static void notify_callback(struct anoubis_notify_head * head,
    int verdict __used,
    void * cbdata __used)
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
	set_value(m->u.notify->sfsmatch, ANOUBIS_SFS_NONE);
	set_value(m->u.notify->csumoff, 0);
	set_value(m->u.notify->csumlen, 0);
	set_value(m->u.notify->pathoff, 0);
	set_value(m->u.notify->pathlen, 0);
	set_value(m->u.notify->ctxcsumoff, 0);
	set_value(m->u.notify->ctxcsumlen, 0);
	set_value(m->u.notify->ctxpathoff, 0);
	set_value(m->u.notify->ctxpathlen, 0);
	set_value(m->u.notify->evoff, 0);
	set_value(m->u.notify->evlen, 0);
	head = anoubis_notify_create_head(m, &notify_callback, NULL);
	if (!head < 0) {
		anoubis_msg_free(m);
	}
	ret = anoubis_notify(grp, head, 0);
	fail_if(ret < 0);
	if (ret == 0)
		anoubis_notify_destroy_head(head);
}

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
	struct anoubis_auth * auth = anoubis_server_getauth(server);
	auth->error = 0;
	auth->uid = auth->chan->euid;
	auth->state = ANOUBIS_AUTH_SUCCESS;
	auth->finish_callback(auth->cbdata);
}

void tp_chat_lud_server(const char *sockname)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
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
	mark_point();
	anoubis_dispatch_create(server, ANOUBIS_C_AUTHDATA, &auth_dummy, NULL);

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

START_TEST(tp_core_comm)
{
	char sockname[] = "/tmp/acXXXXXX";
	pid_t pid;

	if (mkstemp(sockname) < 0)
		fail("Error in mkstemp");

	switch ((pid = check_fork())) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		tp_chat_lud_client(sockname);
		check_waitpid_and_exit(0);
		break;
	default:
		tp_chat_lud_server(sockname);
		check_waitpid_and_exit(pid);
		break;
	}
}
END_TEST

static inline void
fixup_crc(struct anoubis_msg *m)
{
	crc32_set(m->u.buf, m->length);
}

/* Test message generation for csmulti get requests. */
START_TEST(tp_csmulti_get)
{
	struct anoubis_csmulti_request	*req;
	struct anoubis_msg		*m;
	int				 ret;

	/* GET must not be used together with a key id */
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_GET2, 0, "foo", 3);
	fail_if(req != NULL);
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_GET2, 0, NULL, 0);
	fail_if(req == NULL);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Verification must succeed. */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	/* NULL path name must fail. */
	ret = anoubis_csmulti_add(req, NULL, NULL, 0);
	fail_if(ret == 0);
	/* Non NULL checksum data for GET must fail. */
	ret = anoubis_csmulti_add(req, "/foo/bar", "foo", 3);
	fail_if(ret == 0);

	ret = anoubis_csmulti_add(req, "/foo/bar", NULL, 0);
	fail_if(ret != 0);
	ret = anoubis_csmulti_add(req, "/bar/baz", NULL, 0);
	fail_if(ret != 0);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Message must verify successfully */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	anoubis_msg_free(m);
	anoubis_csmulti_destroy(req);
}
END_TEST

/* Test message generation for csmulti getsig requests. */
START_TEST(tp_csmulti_getsig)
{
	struct anoubis_csmulti_request	*req;
	struct anoubis_msg		*m;
	int				 ret;

	/* GETSIG must insist on a key id and zero uid. */
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_GETSIG2, 3, "foo", 3);
	fail_if(req != NULL);
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_GETSIG2, 3, NULL, 0);
	fail_if(req != NULL);
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_GETSIG2, 0, "foo", 3);
	fail_if(req == NULL);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Verification must succeed. */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	/* NULL path name must fail. */
	ret = anoubis_csmulti_add(req, NULL, NULL, 0);
	fail_if(ret == 0);
	/* Non NULL checksum data for GETSIG must fail. */
	ret = anoubis_csmulti_add(req, "/foo/bar", "foo", 3);
	fail_if(ret == 0);

	ret = anoubis_csmulti_add(req, "/foo/bar", NULL, 0);
	fail_if(ret != 0);
	ret = anoubis_csmulti_add(req, "/bar/baz", NULL, 0);
	fail_if(ret != 0);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Message must verify successfully */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	anoubis_msg_free(m);
	anoubis_csmulti_destroy(req);
}
END_TEST

/* Test message generation for csmulti add requests. */
START_TEST(tp_csmulti_add)
{
	struct anoubis_csmulti_request	*req;
	struct anoubis_msg		*m;
	int				 ret;

	/* ADD must not be used together with a key id */
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_ADDSUM, 0, "foo", 3);
	fail_if(req != NULL);
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_ADDSUM, 0,
	    NULL, 0);
	fail_if(req == NULL);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Verification must succeed. */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	/* NULL path name must fail. */
	ret = anoubis_csmulti_add(req, NULL, "foo", 3);
	fail_if(ret == 0);
	/* NULL checksum data for ADD must fail. */
	ret = anoubis_csmulti_add(req, "/foo/bar", NULL, 0);
	fail_if(ret == 0);

	ret = anoubis_csmulti_add(req, "/foo/bar", "xx", 2);
	fail_if(ret != 0);
	ret = anoubis_csmulti_add(req, "/bar/baz", "yy", 2);
	fail_if(ret != 0);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Message must verify successfully */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	anoubis_msg_free(m);
	anoubis_csmulti_destroy(req);
}
END_TEST

/* Test message generation for csmulti getsig request. */
START_TEST(tp_csmulti_addsig)
{
	struct anoubis_csmulti_request	*req;
	struct anoubis_msg		*m;
	int				 ret;

	/* ADDSIG must insist on a key id and zero uid. */
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_ADDSIG, 3, "foo", 3);
	fail_if(req != NULL);
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_ADDSIG, 3, NULL, 0);
	fail_if(req != NULL);
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_ADDSIG, 0, "foo", 3);
	fail_if(req == NULL);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Verification must succeed. */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	/* NULL path name must fail. */
	ret = anoubis_csmulti_add(req, NULL, "foo", 3);
	fail_if(ret == 0);
	/* NULL checksum data for ADDSIG must fail. */
	ret = anoubis_csmulti_add(req, "/foo/bar", NULL, 0);
	fail_if(ret == 0);

	ret = anoubis_csmulti_add(req, "/foo/bar", "xx", 2);
	fail_if(ret != 0);
	ret = anoubis_csmulti_add(req, "/bar/baz", "yy", 2);
	fail_if(ret != 0);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Message must verify successfully */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	anoubis_msg_free(m);
	anoubis_csmulti_destroy(req);
}
END_TEST

/* Test message generation for csmulti del requests. */
START_TEST(tp_csmulti_del)
{
	struct anoubis_csmulti_request	*req;
	struct anoubis_msg		*m;
	int				 ret;

	/* GET must not be used together with a key id */
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_DEL, 0, "foo", 3);
	fail_if(req != NULL);
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_DEL, 0, NULL, 0);
	fail_if(req == NULL);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Verification must succeed. */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	/* NULL path name must fail. */
	ret = anoubis_csmulti_add(req, NULL, NULL, 0);
	fail_if(ret == 0);
	/* Non NULL checksum data for DEL must fail. */
	ret = anoubis_csmulti_add(req, "/foo/bar", "foo", 3);
	fail_if(ret == 0);

	ret = anoubis_csmulti_add(req, "/foo/bar", NULL, 0);
	fail_if(ret != 0);
	ret = anoubis_csmulti_add(req, "/bar/baz", NULL, 0);
	fail_if(ret != 0);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Message must verify successfully */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	anoubis_msg_free(m);
	anoubis_csmulti_destroy(req);
}
END_TEST

/* Test message generation for csmulti delsig requests. */
START_TEST(tp_csmulti_delsig)
{
	struct anoubis_csmulti_request	*req;
	struct anoubis_msg		*m;
	int				 ret;

	/* DELSIG must insist on a key id and zero uid. */
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_DELSIG, 3, "foo", 3);
	fail_if(req != NULL);
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_DELSIG, 3, NULL, 0);
	fail_if(req != NULL);
	req = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_DELSIG, 0, "foo", 3);
	fail_if(req == NULL);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Verification must succeed. */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	/* NULL path name must fail. */
	ret = anoubis_csmulti_add(req, NULL, NULL, 0);
	fail_if(ret == 0);
	/* Non NULL checksum data for DELSIG must fail. */
	ret = anoubis_csmulti_add(req, "/foo/bar", "foo", 3);
	fail_if(ret == 0);

	ret = anoubis_csmulti_add(req, "/foo/bar", NULL, 0);
	fail_if(ret != 0);
	ret = anoubis_csmulti_add(req, "/bar/baz", NULL, 0);
	fail_if(ret != 0);

	m = anoubis_csmulti_msg(req);
	fail_if(m == NULL);
	/* Message must verify successfully */
	fixup_crc(m);
	anoubis_dump(m, "csmulti");
	fail_unless(anoubis_msg_verify(m));

	anoubis_msg_free(m);
	anoubis_csmulti_destroy(req);
}
END_TEST

TCase *
libanoubisproto_testcase_core(void)
{
	/* Core test case */
	TCase *tp_core = tcase_create("Core");

	tcase_add_test(tp_core, tp_csmulti_get);
	tcase_add_test(tp_core, tp_csmulti_getsig);
	tcase_add_test(tp_core, tp_csmulti_add);
	tcase_add_test(tp_core, tp_csmulti_addsig);
	tcase_add_test(tp_core, tp_csmulti_del);
	tcase_add_test(tp_core, tp_csmulti_delsig);
	tcase_add_test(tp_core, tp_core_comm);
	tcase_set_timeout(tp_core, 30);

	return (tp_core);
}

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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <anoubischat.h>

#include <anoubis_protocol.h>
#include <anoubis_errno.h>
#include <anoubis_msg.h>
#include <anoubis_client.h>
#include <anoubis_dump.h>
#include <anoubis_transaction.h>
#include "crc32.h"

#ifndef EPROTO
#define EPROTO EINVAL
#endif

#define ANOUBIS_PROTO_VERSION		1

#define ANOUBIS_PROTO_CONNECT		0
#define ANOUBIS_PROTO_POLICY		1
#define ANOUBIS_PROTO_NOTIFY		2
#define ANOUBIS_PROTO_BOTH		3

#define ALLOCLEN ((8192>ACHAT_MAX_MSGSIZE)?ACHAT_MAX_MSGSIZE:8192)

static struct proto_opt anoubis_protos[] = {
	{ ANOUBIS_PROTO_POLICY,		"POLICY"	},
	{ ANOUBIS_PROTO_NOTIFY,		"NOTIFY"	},
	{ -1,				""		}
};

#define ANOUBIS_STATE_INIT		1
#define ANOUBIS_STATE_ERROR		2
#define ANOUBIS_STATE_CONNECTING	3
#define ANOUBIS_STATE_CONNECTED		4
#define ANOUBIS_STATE_CLOSING		5
#define ANOUBIS_STATE_CLOSED		6

#define FLAG_MULTIPLEX			0x0001
#define FLAG_PIPELINE			0x0002
#define FLAG_SENTCLOSEREQ		0x0004
#define FLAG_GOTCLOSEREQ		0x0008
#define FLAG_SENTCLOSEACK		0x0010
#define FLAG_GOTCLOSEACK		0x0020

struct anoubis_client {
	int state;
	int proto;
	int connect_flags;
	struct achat_channel * chan;
	unsigned long auth_uid;
	char * username;
	LIST_HEAD(,anoubis_transaction) ops;
	struct anoubis_msg * notify;
	struct anoubis_msg * tail;
};

static void queue_notify(struct anoubis_client * client, struct anoubis_msg * m)
{
	m->next = client->tail;
	if (client->tail) {
		client->tail->next = m;
	} else {
		client->notify = client->tail = m;
	}
}

static struct anoubis_msg * anoubis_client_rcv(struct anoubis_client * client,
    size_t alloclen)
{
	struct anoubis_msg * m = NULL;
	achat_rc rc;

	if (alloclen < CSUM_LEN)
		goto err;
	m = anoubis_msg_new(alloclen);
	if (!m)
		return NULL;
	rc = acc_receivemsg(client->chan, m->u.buf, &alloclen);
	if (rc != ACHAT_RC_OK)
		goto err;
	anoubis_msg_resize(m, alloclen);
	anoubis_dump(m, "Client read");
	if (!VERIFY_FIELD(m,general,type)
	    || !crc32_check(m->u.buf, m->length))
		goto err;
	if (get_value(m->u.general->type) == ANOUBIS_C_CLOSEREQ) {
		client->state = ANOUBIS_STATE_CLOSING;
		client->connect_flags |= FLAG_GOTCLOSEREQ;
	}
	if (get_value(m->u.general->type) == ANOUBIS_C_CLOSEACK) {
		client->state = ANOUBIS_STATE_CLOSING;
		client->connect_flags |= FLAG_GOTCLOSEACK;
	}
	return m;
err:
	if (m)
		anoubis_msg_free(m);
	return NULL;
}

int anoubis_client_verify(struct anoubis_client * client,
    struct anoubis_msg * m)
{
	if (!VERIFY_FIELD(m,general,type)
	    || !crc32_check(m->u.buf, m->length))
		return -EIO;
	if (get_value(m->u.general->type) == ANOUBIS_C_CLOSEREQ) {
		client->state = ANOUBIS_STATE_CLOSING;
		client->connect_flags |= FLAG_GOTCLOSEREQ;
	}
	if (get_value(m->u.general->type) == ANOUBIS_C_CLOSEACK) {
		client->state = ANOUBIS_STATE_CLOSING;
		client->connect_flags |= FLAG_GOTCLOSEACK;
	}
	return 0;
}

int anoubis_client_send(struct anoubis_client * client, struct anoubis_msg * m)
{
	achat_rc rc;
	crc32_set(m->u.buf, m->length);
	anoubis_dump(m, "Client send");
	rc = acc_sendmsg(client->chan, m->u.buf, m->length);
	anoubis_msg_free(m);
	if (rc != ACHAT_RC_OK)
		return -ECONNABORTED;
	return 0;
}

struct anoubis_client * anoubis_client_create(struct achat_channel * chan)
{
	struct anoubis_client * ret = malloc(sizeof(struct anoubis_client));
	if (!ret)
		return NULL;
	ret->chan = chan;
	ret->state = ANOUBIS_STATE_INIT;
	ret->proto = ANOUBIS_PROTO_CONNECT;
	ret->connect_flags = 0;
	ret->auth_uid = -1;
	ret->username = NULL;
	LIST_INIT(&ret->ops);
	ret->notify = ret->tail = NULL;

	return ret;
}

void anoubis_client_destroy(struct anoubis_client * client)
{
	acc_close(client->chan);
	free(client);
}

static int anoubis_verify_reply(struct anoubis_client * client,
    u_int32_t origop, struct anoubis_msg * m)
{
	int ret = -EPROTO;
	u_int32_t opcode;

	anoubis_client_verify(client, m);
	if (!m)
		return -EPROTO;
	opcode = get_value(m->u.general->type);
	if (opcode != ANOUBIS_REPLY
	    || !VERIFY_LENGTH(m, sizeof(Anoubis_AckMessage)))
		goto err;
	ret = -EPERM;
	if (m->u.ack->token || get_value(m->u.ack->error)
	    || get_value(m->u.ack->opcode) != origop)
		goto err;
	ret = 0;
err:
	anoubis_msg_free(m);
	return ret;
}

static int anoubis_verify_hello(struct anoubis_client * client, int myproto,
    struct anoubis_msg * m)
{
	int ret;
	u_int32_t opcode;

	ret = anoubis_client_verify(client, m);
	if (ret < 0)
		goto err;
	opcode = get_value(m->u.general->type);
	ret = -EPROTO;
	if (opcode != ANOUBIS_C_HELLO
	    || !VERIFY_LENGTH(m, sizeof(Anoubis_HelloMessage)))
		goto err;
	ret = -EPROTONOSUPPORT;
	if (get_value(m->u.hello->min_version) > myproto
	    || myproto > get_value(m->u.hello->version))
		goto err;
	ret = 0;
err:
	anoubis_msg_free(m);
	return ret;
}

static int anoubis_verify_authreply(struct anoubis_client * client,
    struct anoubis_msg * m)
{
	int ret;
	u_int32_t opcode;
	int slen;

	ret = anoubis_client_verify(client, m);
	if (ret < 0)
		goto err;
	ret = -EPROTO;
	opcode = get_value(m->u.general->type);
	if (opcode != ANOUBIS_C_AUTHREPLY
	    || !VERIFY_LENGTH(m, sizeof(Anoubis_AuthReplyMessage)))
		goto err;
	ret = -EPERM;
	if (get_value(m->u.authreply->error))
		goto err;
	slen = m->length - sizeof(Anoubis_AuthReplyMessage);
	ret = -ENOMEM;
	client->username = malloc(slen+1);
	if (client->username == NULL)
		goto err;
	client->auth_uid = get_value(m->u.authreply->uid);
	memcpy(client->username, m->u.authreply->name, slen);
	client->username[slen] = 0;
	ret = 0;
err:
	anoubis_msg_free(m);
	return ret;
}

static int anoubis_verify_stringlist(struct anoubis_client * client,
    u_int32_t expected_opcode, struct proto_opt * opts, unsigned int * selopts,
    struct anoubis_msg * m)
{
	struct stringlist_iterator it;
	int ret;
	int flag;
	u_int32_t opcode;

	ret = anoubis_client_verify(client, m);
	if (ret < 0)
		goto err;
	ret = -EPROTO;
	opcode = get_value(m->u.general->type);
	if (opcode != expected_opcode
	    || !VERIFY_LENGTH(m, sizeof(Anoubis_StringListMessage)))
		goto err;
	stringlist_iterator_init(&it, m, opts);
	(*selopts) = 0;
	while((flag = stringlist_iterator_get(&it)) >= 0)
		(*selopts) |= flag;
	ret = 0;
err:
	anoubis_msg_free(m);
	return ret;
}

static int anoubis_send_general(struct anoubis_client * client,
    u_int32_t opcode)
{
	struct anoubis_msg * m;
	m = anoubis_msg_new(sizeof(Anoubis_GeneralMessage));
	if (!m)
		return -ENOMEM;
	set_value(m->u.general->type, opcode);
	return anoubis_client_send(client, m);
}

static struct proto_opt anoubis_proto_opts[] = {
	{ FLAG_MULTIPLEX,	"PROTOMULTIPLEX" },
	{ FLAG_PIPELINE,	"PIPELINE"	},
	{ -1,			""		}
};

struct anoubis_connect_cbdata {
	struct anoubis_client * client;
	unsigned int proto;
};

/* This function will free the message @m. */
static void anoubis_client_connect_steps(struct anoubis_transaction * t,
    struct anoubis_msg * m)
{
	int ret = -EPROTO;
	struct anoubis_msg * out = NULL;
	struct anoubis_connect_cbdata * cb = t->cbdata;
	struct anoubis_client * client = cb->client;

	assert(t->process == &anoubis_client_connect_steps);

	switch(t->stage) {
	case 1: {
		static const int nextops[] = { ANOUBIS_REPLY, 0 };
		ret = anoubis_verify_hello(client, ANOUBIS_PROTO_VERSION, m);
		if (ret < 0)
			goto err;
		ret = -ENOMEM;
		out = anoubis_msg_new(sizeof(Anoubis_VerselMessage));
		if (!out)
			goto err;
		set_value(out->u.versel->type, ANOUBIS_C_VERSEL);
		set_value(out->u.versel->version, ANOUBIS_PROTO_VERSION);
		set_value(out->u.versel->_pad, 0);
		ret = anoubis_client_send(client, out);
		if (ret < 0)
			goto err;
		anoubis_transaction_progress(t, nextops);
		return;
	} case 2: {
		static const int nextops[] = { ANOUBIS_REPLY, 0 };
		ret = anoubis_verify_reply(client, ANOUBIS_C_VERSEL, m);
		if (ret < 0)
			goto err;
		ret = anoubis_send_general(client, ANOUBIS_C_AUTH);
		if (ret < 0)
			goto err;
		anoubis_transaction_progress(t, nextops);
		return;
	} case 3: {
		static const int nextops[] = { ANOUBIS_C_AUTHREPLY, 0 };
		ret = anoubis_verify_reply(client, ANOUBIS_C_AUTH, m);
		if (ret < 0)
			goto err;
		out = anoubis_msg_new(sizeof(Anoubis_AuthTransportMessage));
		set_value(out->u.authtransport->type, ANOUBIS_C_AUTHDATA);
		set_value(out->u.authtransport->auth_type,
		    ANOUBIS_AUTH_TRANSPORT);
		ret = anoubis_client_send(client, out);
		if (ret < 0)
			goto err;
		anoubis_transaction_progress(t, nextops);
		return;
	} case 4: {
		static const int nextops[] = { ANOUBIS_C_OPTACK, 0 };
		int opts[3] = { FLAG_MULTIPLEX, FLAG_PIPELINE, -1 };
		ret = anoubis_verify_authreply(client, m);
		if (ret < 0)
			goto err;
		ret = -ENOMEM;
		out = anoubis_stringlist_msg(ANOUBIS_C_OPTREQ,
		    anoubis_proto_opts, opts);
		if (!out)
			goto err;
		ret = anoubis_client_send(client, out);
		if (ret < 0)
			goto err;
		anoubis_transaction_progress(t, nextops);
		return;
	} case 5: {
		static const int nextops[] = { ANOUBIS_REPLY, 0 };
		int multiplex = (cb->proto == ANOUBIS_PROTO_BOTH);
		unsigned int selopts = 0;
		int opts[3];
		int k;
		ret = anoubis_verify_stringlist(client, ANOUBIS_C_OPTACK,
		    anoubis_proto_opts, &selopts, m);
		if (ret < 0)
			goto err;
		client->connect_flags |= selopts;
		ret = -EPROTONOSUPPORT;
		if (multiplex && (selopts & FLAG_MULTIPLEX) == 0)
			goto err;
		k = 0;
		if (cb->proto & ANOUBIS_PROTO_POLICY)
			opts[k++] = ANOUBIS_PROTO_POLICY;
		if (cb->proto & ANOUBIS_PROTO_NOTIFY)
			opts[k++] = ANOUBIS_PROTO_NOTIFY;
		opts[k] = -1;
		ret = -ENOMEM;
		out = anoubis_stringlist_msg(ANOUBIS_C_PROTOSEL,
		    anoubis_protos, opts);
		if (!out)
			goto err;
		ret = anoubis_client_send(client, out);
		if (ret < 0)
			goto err;
		anoubis_transaction_progress(t, nextops);
		return;
	} case 6: {
		ret = anoubis_verify_reply(client, ANOUBIS_C_PROTOSEL, m);
		if (ret < 0)
			goto err;
		client->proto = cb->proto;
		client->state = ANOUBIS_STATE_CONNECTED;
		anoubis_transaction_done(t, 0);
		return;
	}
	} /* Switch */
err:
	client->state = ANOUBIS_STATE_ERROR;
	anoubis_transaction_done(t, -ret);
	anoubis_client_close(client);
	return;
}

struct anoubis_transaction * anoubis_client_connect_start(
    struct anoubis_client * client, unsigned int proto)
{
	struct anoubis_transaction * t;
	struct anoubis_connect_cbdata * cb;
	static const int nextops[] = { ANOUBIS_C_HELLO, 0 };
	if (client->state != ANOUBIS_STATE_INIT)
		return NULL;
	if (proto & ~(ANOUBIS_PROTO_BOTH))
		return NULL;
	if ((proto & ANOUBIS_PROTO_BOTH) == 0)
		return NULL;
	cb = malloc(sizeof(struct anoubis_connect_cbdata));
	if (!cb)
		return NULL;
	cb->client = client;
	cb->proto = proto;
	t = anoubis_transaction_create(0,
	    ANOUBIS_T_INITPEER|ANOUBIS_T_FREECBDATA|ANOUBIS_T_DEQUEUE,
	    &anoubis_client_connect_steps, NULL, cb);
	t->stage = 1;
	if (!t) {
		free(cb);
		return NULL;
	}
	anoubis_transaction_setopcodes(t, nextops);
	LIST_INSERT_HEAD(&client->ops, t, next);
	return t;
}

static void anoubis_client_close_steps(struct anoubis_transaction * t,
    struct anoubis_msg * m)
{
	int opcode;
	int ret;
	struct anoubis_client * client = t->cbdata;

	/* Always sanity check incoming message. */
	ret = anoubis_client_verify(client, m);
	if (ret < 0)
		goto err;
	ret = -EPROTO;
	opcode = get_value(m->u.general->type);
	anoubis_msg_free(m);
	if (opcode != ANOUBIS_C_CLOSEREQ && opcode != ANOUBIS_C_CLOSEACK)
		goto err;
	if (opcode == ANOUBIS_C_CLOSEREQ)
		client->connect_flags |= FLAG_GOTCLOSEREQ;
	if (opcode == ANOUBIS_C_CLOSEACK)
		client->connect_flags |= FLAG_GOTCLOSEACK;
	/* Make sure that we already sent a CLOSEREQ. */
	if ((client->connect_flags & FLAG_SENTCLOSEREQ) == 0)
		goto err;
	/* The following checks for a client protocol error. */
	if ((client->connect_flags & FLAG_GOTCLOSEACK)
	    && (client->connect_flags & FLAG_GOTCLOSEREQ) == 0)
		goto err;
	/* Send a CLOSEACK if we can and have not yet done so. */
	if (client->connect_flags & FLAG_GOTCLOSEREQ) {
		if ((client->connect_flags & FLAG_SENTCLOSEACK) == 0) {
			ret = anoubis_send_general(client, ANOUBIS_C_CLOSEACK);
			if (ret < 0)
				goto err;
			client->connect_flags |= FLAG_SENTCLOSEACK;
		}
	}
	/* See if we are done.
	 * Note: We must terminate the connection if we do not expect another
	 * packet. This is guaranteed because:
	 *  - if FLAG_GOTCLOSEACK is not set we wait still need th CLOSEACK
	 *    form the other end.
	 *  - if FLAG_SENTCLOSEACK is not set this means that
	 *    FLAG_GOTCLOSEREQ cannot be set either (see above). Thus we
	 *    still wait for the CLOSEREQ and the CLOSEACK from the
	 *    other end.
	 */
	if (client->connect_flags & FLAG_GOTCLOSEACK
	    && client->connect_flags & FLAG_SENTCLOSEACK) {
		client->state = ANOUBIS_STATE_CLOSED;
		acc_close(client->chan);
		anoubis_transaction_done(t, 0);
	}
	return;
err:
	acc_close(client->chan);
	client->state = ANOUBIS_STATE_ERROR;
	anoubis_transaction_done(t, -ret);
}

struct anoubis_transaction * anoubis_client_close_start(
    struct anoubis_client * client)
{
	struct anoubis_transaction * ret = NULL;
	static const int nextops[] = {
	    ANOUBIS_C_CLOSEREQ, ANOUBIS_C_CLOSEACK, 0
	};

	if (client->connect_flags & FLAG_SENTCLOSEREQ)
		return NULL;
	ret = anoubis_transaction_create(0,
	    ANOUBIS_T_INITSELF|ANOUBIS_T_DEQUEUE,
	    &anoubis_client_close_steps, NULL, client);
	if (!ret)
		goto err;
	anoubis_transaction_setopcodes(ret, nextops);
	if (anoubis_send_general(client, ANOUBIS_C_CLOSEREQ) < 0)
		goto err;
	client->state = ANOUBIS_STATE_CLOSING;
	client->connect_flags |= FLAG_SENTCLOSEREQ;
	LIST_INSERT_HEAD(&client->ops, ret, next);
	return ret;
err:
	acc_close(client->chan);
	client->state = ANOUBIS_STATE_ERROR;
	if (ret)
		free(ret);
	return NULL;
}

static void anoubis_client_process_verdict(struct anoubis_transaction * t,
    struct anoubis_msg * m)
{
	struct anoubis_msg * s;
	struct anoubis_client * client = t->cbdata;
	u_int32_t opcode;
	anoubis_token_t token;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_NotifyResultMessage))) {
		anoubis_msg_free(m);
		return;
	}
	opcode = get_value(m->u.notifyresult->type);
	token = m->u.notifyresult->token;
	if (opcode != ANOUBIS_N_RESYOU && opcode != ANOUBIS_N_RESOTHER) {
		anoubis_msg_free(m);
		return;
	}
	for(s=client->notify; s; s=s->next)
		if (s->u.token->token == token)
			break;
	/* Return ESRCH if:
	 *  - No message with the given token was found.
	 *  - A message was found but is was not of type ANOUBIS_N_ASK
	 *  - The next message on the queue is a previously received answer
	 *    for the message.
	 */
	if (!s || (get_value(s->u.general->type) != ANOUBIS_N_ASK)
	    || (s->next && s->u.token->token == s->next->u.token->token)) {
		anoubis_msg_free(m);
		return;
	}
	m->next = s->next;
	s->next = m;
	if (client->tail == s)
		client->tail = m;
	anoubis_dump(m, "NOTIFY_REPLY");
	/*
	 * XXX: If no answer was sent we should do so right now.
	 * XXX: However, this requires a more sophisticated registeration
	 * XXX: of pending events.  --ceh 02/09
	 */
	return;
}

static int anoubis_client_process_event(struct anoubis_client * client,
    struct anoubis_msg * m, int opcode, anoubis_token_t token)
{
	struct anoubis_transaction * t;
	static const int nextops[] = {
	    ANOUBIS_N_RESYOU, ANOUBIS_N_RESOTHER, 0
	};
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_NotifyMessage))) {
		anoubis_msg_free(m);
		return -EIO;
	}
	if (opcode != ANOUBIS_N_ASK && opcode != ANOUBIS_N_NOTIFY) {
		anoubis_msg_free(m);
		return -EINVAL;
	}
	anoubis_dump(m, "notify");
	if (opcode == ANOUBIS_N_ASK) {
		t = anoubis_transaction_create(token,
		    ANOUBIS_T_INITPEER|ANOUBIS_T_DEQUEUE|ANOUBIS_T_DESTROY,
		    &anoubis_client_process_verdict, NULL, client);
		if (!t) {
			anoubis_msg_free(m);
			return -EINVAL;
		}
		anoubis_transaction_setopcodes(t, nextops);
		LIST_INSERT_HEAD(&client->ops, t, next);
	}
	queue_notify(client, m);
	return 1;
}

static int __anoubis_client_continue(struct anoubis_client * client,
    struct anoubis_msg * m, u_int32_t opcode, anoubis_token_t token, int self)
{
	struct anoubis_transaction * t;
	LIST_FOREACH(t, &client->ops, next) {
		if (anoubis_transaction_match(t, token, self, opcode)) {
			anoubis_transaction_process(t, m);
			return 1;
		}
	}
	anoubis_msg_free(m);
	return 0;
}

static int anoubis_client_continue_connect(struct anoubis_client * client,
    struct anoubis_msg * m, u_int32_t opcode)
{
	return __anoubis_client_continue(client, m, opcode, 0, 0);
}

static int anoubis_client_continue_policy_self(struct anoubis_client * client,
    struct anoubis_msg * m, u_int32_t opcode)
{
	return __anoubis_client_continue(client, m, opcode, 0, 1);
}

static int anoubis_client_continue_self(struct anoubis_client * client,
    struct anoubis_msg * m, u_int32_t opcode, anoubis_token_t token)
{
	return __anoubis_client_continue(client, m, opcode, token, 1);
}

static int anoubis_client_continue_peer(struct anoubis_client * client,
    struct anoubis_msg * m, u_int32_t opcode, anoubis_token_t token)
{
	return __anoubis_client_continue(client, m, opcode, token, 0);
}

/* Will take over and ultimately free the message! */
int anoubis_client_process(struct anoubis_client * client,
    struct anoubis_msg * m)
{
	int ret, opcode;

	/* First do sanity checks on the message. */
	ret = anoubis_client_verify(client, m);
	if (ret < 0)
		goto err;
	ret = -EPROTO;
	anoubis_dump(m, "client process");
	/* No more processing if closed or in case of a permanent error. */
	switch(client->state) {
	case ANOUBIS_STATE_CLOSED:
	case ANOUBIS_STATE_ERROR:
		goto err;
	}
	opcode = get_value(m->u.general->type);
	/*
	 * If we are still connecting, this must belong to an ongoing
	 * opertion.
	 */
	if (client->proto == ANOUBIS_PROTO_CONNECT)
		return anoubis_client_continue_connect(client, m, opcode);
	if (opcode == ANOUBIS_REPLY) {
		ret = -EIO;
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_AckMessage)))
			goto err;
		return anoubis_client_continue_self(client, m, opcode,
		    m->u.ack->token);
	}
	if (ANOUBIS_IS_NOTIFY(opcode)) {
		anoubis_token_t token;
		ret = -EIO;
		if (!VERIFY_FIELD(m, token, token))
			goto err;
		token = m->u.token->token;
		ret = -EPROTO;
		if ((client->proto & ANOUBIS_PROTO_NOTIFY) == 0)
			goto err;
		switch(opcode) {
		case ANOUBIS_N_ASK:
		case ANOUBIS_N_NOTIFY:
			return anoubis_client_process_event(client, m, opcode,
			    m->u.token->token);
		case ANOUBIS_N_RESYOU:
		case ANOUBIS_N_RESOTHER:
			return anoubis_client_continue_peer(client, m, opcode,
			    m->u.token->token);
		case ANOUBIS_N_REGISTER:
		case ANOUBIS_N_UNREGISTER:
		case ANOUBIS_N_DELEGATE:
		case ANOUBIS_N_CTXREQ:
			/* These are not allowed from the server. */
			ret = -EPROTO;
			goto err;
		case ANOUBIS_N_CTXREPLY:
			/* This is not yet supported. */
			ret = -EPROTO;
			goto err;
		default:
			goto err;
		}
	}
	/* Try to dispatch it into an ongoing operation. */
	return anoubis_client_continue_policy_self(client, m, opcode);
err:
	anoubis_msg_free(m);
	return ret;
}

static void anoubis_client_register_steps(struct anoubis_transaction * t,
    struct anoubis_msg * m)
{
	struct anoubis_client * client = t->cbdata;
	int ret = anoubis_client_verify(client, m);
	if (ret < 0)
		goto err;
	ret = -EPROTO;
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_AckMessage))
	    || get_value(m->u.ack->type) != ANOUBIS_REPLY)
		goto err;
	ret = get_value(m->u.ack->error);
err:
	anoubis_msg_free(m);
	anoubis_transaction_done(t, -ret);
}

struct anoubis_transaction * __anoubis_client_register_start_common(
    struct anoubis_client * client, int opcode, anoubis_token_t token,
    pid_t pid, u_int32_t rule_id, uid_t uid, u_int32_t subsystem)
{
	struct anoubis_msg * m;
	static const int nextops[] = { ANOUBIS_REPLY, 0 };
	struct anoubis_transaction * ret;

	if ((client->proto & ANOUBIS_PROTO_NOTIFY) == 0)
		return NULL;
	if (client->state != ANOUBIS_STATE_CONNECTED)
		return NULL;
	if (token == 0)
		return NULL;
	m = anoubis_msg_new(sizeof(Anoubis_NotifyRegMessage));
	if (!m)
		return NULL;
	set_value(m->u.notifyreg->type, opcode);
	m->u.notifyreg->token = token;
	set_value(m->u.notifyreg->pid, pid);
	set_value(m->u.notifyreg->rule_id, rule_id);
	set_value(m->u.notifyreg->uid, uid);
	set_value(m->u.notifyreg->subsystem, subsystem);
	ret = anoubis_transaction_create(token,
	    ANOUBIS_T_INITSELF|ANOUBIS_T_DEQUEUE,
	    &anoubis_client_register_steps, NULL, client);
	if (!ret) {
		anoubis_msg_free(m);
		return NULL;
	}
	if (anoubis_client_send(client, m) < 0) {
		anoubis_transaction_destroy(ret);
		anoubis_msg_free(m);
		return NULL;
	}
	anoubis_transaction_setopcodes(ret, nextops);
	LIST_INSERT_HEAD(&client->ops, ret, next);
	return ret;
}

struct anoubis_transaction * anoubis_client_register_start(
    struct anoubis_client * client, anoubis_token_t token, pid_t pid,
    u_int32_t rule_id, uid_t uid, u_int32_t subsystem)
{
	return  __anoubis_client_register_start_common(client,
	    ANOUBIS_N_REGISTER, token, pid, rule_id, uid, subsystem);
}

struct anoubis_transaction * anoubis_client_unregister_start(
    struct anoubis_client * client, anoubis_token_t token, pid_t pid,
    u_int32_t rule_id, uid_t uid, u_int32_t subsystem)
{
	return  __anoubis_client_register_start_common(client,
	    ANOUBIS_N_UNREGISTER, token, pid, rule_id, uid, subsystem);
}

/* Sync function. */
int anoubis_client_connect(struct anoubis_client * client, unsigned int proto)
{
	struct anoubis_transaction * t;
	struct anoubis_msg * m;
	int ret;

	t = anoubis_client_connect_start(client, proto);
	if (!t)
		return -EINVAL;
	while(1) {
		m = anoubis_client_rcv(client, ALLOCLEN);
		if (!m)
			return -EPROTO;
		/* Will free the message. */
		anoubis_client_continue_connect(client, m,
		    get_value(m->u.general->type));
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}
	ret = t->result;
	anoubis_transaction_destroy(t);
	return ret;
}

void anoubis_client_close(struct anoubis_client * client)
{
	struct anoubis_msg * m;
	struct anoubis_transaction * t;

	t = anoubis_client_close_start(client);
	if (!t) {
		acc_close(client->chan);
		client->state = ANOUBIS_STATE_ERROR;
		return;
	}
	while((m = anoubis_client_rcv(client, ALLOCLEN))) {
		int opcode = get_value(m->u.general->type);
		switch (opcode) {
		case ANOUBIS_C_CLOSEREQ:
		case ANOUBIS_C_CLOSEACK:
			anoubis_client_continue_connect(client, m,
			    get_value(m->u.general->type));
			break;
		default:
			anoubis_msg_free(m);
		}
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}
	anoubis_transaction_destroy(t);
	return;
}
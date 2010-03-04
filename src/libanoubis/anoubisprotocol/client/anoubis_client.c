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
#include <unistd.h>
#include <stddef.h>
#ifdef LINUX
#include <bsdcompat.h>
#endif
#include <anoubis_chat.h>

#include <anoubis_protocol.h>
#include <anoubis_errno.h>
#include <anoubis_msg.h>
#include <anoubis_client.h>
#include <anoubis_dump.h>
#include <anoubis_transaction.h>
#include "anoubis_crc32.h"

#define __used __attribute__((unused))

#ifndef EPROTO
#define EPROTO EINVAL
#endif

#define SENDLEN  3000

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

#define	FLAG_POLICY_PENDING		0x0040

struct anoubis_client {
	int state;
	int proto;
	int flags;
	struct achat_channel * chan;
	unsigned long auth_uid;
	int server_version;
	int server_min_version;
	int selected_version;
	LIST_HEAD(,anoubis_transaction) ops;
	struct anoubis_msg * notify;
	struct anoubis_msg * tail;
	int auth_type;
	anoubis_client_auth_callback_t	auth_callback;
};

/* XXX tartler: The anoubis client library is pretty challenging to
 *              annotate properly, since I don't quite understand the
 *              memory managment implications between struct
 *              anoubis_client and struct anoubis_msg. Since for MS7 the
 *              priority is on anoubisd, we defer enabling memory checks
 *              for this part of libanoubisprotocol.
 */

/*@-memchecks@*/
static void queue_notify(struct anoubis_client * client, struct anoubis_msg * m)
{
	m->next = client->tail;
	if (client->tail) {
		client->tail->next = m;
	} else {
		client->notify = client->tail = m;
	}
}

struct anoubis_msg * anoubis_client_getnotify(struct anoubis_client * client)
{
	struct anoubis_msg * m = client->notify;

	if (m) {
		client->notify = m->next;
		if (!client->notify)
			client->tail = NULL;
		m->next = NULL;
	}

	return m;
}

int anoubis_client_hasnotifies(struct anoubis_client * client)
{
	return (client->notify != NULL);
}

static struct anoubis_msg * anoubis_client_rcv(struct anoubis_client * client)
{
	struct anoubis_msg	*m = NULL;
	achat_rc		 rc;
	size_t			 alloclen = 4096;

	m = anoubis_msg_new(alloclen);
	if (!m)
		return NULL;
	while (1) {
		rc = acc_receivemsg(client->chan, m->u.buf, &alloclen);
		if (rc != ACHAT_RC_NOSPACE)
			break;
		alloclen *= 2;
		if (anoubis_msg_resize(m, alloclen) < 0)
			rc = ACHAT_RC_ERROR;
	}
	if (rc != ACHAT_RC_OK)
		goto err;
	anoubis_msg_resize(m, alloclen);
	if (!anoubis_msg_verify(m) || !crc32_check(m->u.buf, m->length))
		goto err;
	anoubis_dump(m, "Client read");
	if (get_value(m->u.general->type) == ANOUBIS_C_CLOSEREQ) {
		client->state = ANOUBIS_STATE_CLOSING;
		client->flags |= FLAG_GOTCLOSEREQ;
	}
	if (get_value(m->u.general->type) == ANOUBIS_C_CLOSEACK) {
		client->state = ANOUBIS_STATE_CLOSING;
		client->flags |= FLAG_GOTCLOSEACK;
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
		return -ERANGE;
	if (get_value(m->u.general->type) == ANOUBIS_C_CLOSEREQ) {
		client->state = ANOUBIS_STATE_CLOSING;
		client->flags |= FLAG_GOTCLOSEREQ;
	}
	if (get_value(m->u.general->type) == ANOUBIS_C_CLOSEACK) {
		client->state = ANOUBIS_STATE_CLOSING;
		client->flags |= FLAG_GOTCLOSEACK;
	}
	return 0;
}

static int anoubis_client_send(struct anoubis_client * client,
    struct anoubis_msg * m)
{
	int ret = anoubis_msg_send(client->chan, m);
	anoubis_msg_free(m);
	return ret;
}

struct anoubis_client * anoubis_client_create(struct achat_channel * chan,
    int auth_type, anoubis_client_auth_callback_t auth_cb)
{
	struct anoubis_client * ret = malloc(sizeof(struct anoubis_client));
	if (!ret)
		return NULL;
	ret->chan = chan;
	ret->state = ANOUBIS_STATE_INIT;
	ret->proto = ANOUBIS_PROTO_CONNECT;
	ret->flags = 0;
	ret->auth_uid = -1;
	ret->server_version = -1;
	ret->server_min_version = -1;
	ret->selected_version = -1;
	LIST_INIT(&ret->ops);
	ret->notify = ret->tail = NULL;
	ret->auth_type = auth_type;
	ret->auth_callback = auth_cb;

	return ret;
}

void anoubis_client_destroy(struct anoubis_client * client)
{
	if (client->chan) {
		acc_close(client->chan);
		client->chan = NULL;
	}
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

static int anoubis_verify_hello(struct anoubis_client * client,
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
	client->server_version = get_value(m->u.hello->version);
	client->server_min_version = get_value(m->u.hello->min_version);
	if (client->server_min_version <= ANOUBIS_PROTO_VERSION
	    && ANOUBIS_PROTO_VERSION <= client->server_version) {
		client->selected_version = ANOUBIS_PROTO_VERSION;
	} else if (ANOUBIS_PROTO_MINVERSION <= client->server_version
	    && client->server_version <= ANOUBIS_PROTO_VERSION) {
		client->selected_version = client->server_version;
		/* Handle compatibility for older server versions. */
		switch(client->selected_version) {
		case 4:
			break;
		case 3:
			/* No support for key base authentication */
			client->auth_callback = NULL;
			client->auth_type = ANOUBIS_AUTH_TRANSPORT;
			break;
		default:
			goto err;
		}
	} else {
		goto err;
	}
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
	size_t slen;

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
	client->auth_uid = get_value(m->u.authreply->uid);
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
		static const u_int32_t nextops[] = { ANOUBIS_REPLY, 0 };
		ret = anoubis_verify_hello(client, m);
		if (ret < 0)
			goto err;
		ret = -ENOMEM;
		out = anoubis_msg_new(sizeof(Anoubis_VerselMessage));
		if (!out)
			goto err;
		set_value(out->u.versel->type, ANOUBIS_C_VERSEL);
		set_value(out->u.versel->version, client->selected_version);
		set_value(out->u.versel->_pad, 0);
		ret = anoubis_client_send(client, out);
		if (ret < 0)
			goto err;
		anoubis_transaction_progress(t, nextops);
		return;
	} case 2: {
		static const u_int32_t nextops[] = { ANOUBIS_REPLY, 0 };
		ret = anoubis_verify_reply(client, ANOUBIS_C_VERSEL, m);
		if (ret < 0)
			goto err;
		ret = anoubis_send_general(client, ANOUBIS_C_AUTH);
		if (ret < 0)
			goto err;
		anoubis_transaction_progress(t, nextops);
		return;
	} case 3: {
		static const u_int32_t nextops[] = { ANOUBIS_C_AUTHREPLY,
		    ANOUBIS_C_AUTHDATA, 0 };
		ret = anoubis_verify_reply(client, ANOUBIS_C_AUTH, m);
		if (ret < 0)
			goto err;
		out = anoubis_msg_new(
		    sizeof(Anoubis_AuthTransportMessage));
		set_value(out->u.authtransport->type,
		    ANOUBIS_C_AUTHDATA);
		set_value(out->u.authtransport->auth_type,
		    client->auth_type);
		ret = anoubis_client_send(client, out);
		if (ret < 0)
			goto err;
		anoubis_transaction_progress(t, nextops);
		return;
	} case 4: {
		static const u_int32_t nextops[] = { ANOUBIS_C_OPTACK, 0 };
		int opts[3] = { FLAG_MULTIPLEX, FLAG_PIPELINE, -1 };
		int type = get_value(m->u.general->type);
		if (type == ANOUBIS_C_AUTHDATA) {
			out = NULL;
			if (client->auth_callback != NULL) {
				ret = client->auth_callback(client, m, &out);
				if (ret < 0)
					goto err;
			}
			ret = -EPERM;
			if (out == NULL)
				goto err;
			ret = anoubis_client_send(client, out);
			if (ret < 0)
				goto err;
			/* No transaction progress. */
		} else {
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
		}
		return;
	} case 5: {
		static const u_int32_t nextops[] = { ANOUBIS_REPLY, 0 };
		int multiplex = (cb->proto == ANOUBIS_PROTO_BOTH);
		unsigned int selopts = 0;
		int opts[3];
		int k;
		ret = anoubis_verify_stringlist(client, ANOUBIS_C_OPTACK,
		    anoubis_proto_opts, &selopts, m);
		if (ret < 0)
			goto err;
		client->flags |= selopts;
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
	static const u_int32_t nextops[] = { ANOUBIS_C_HELLO, 0 };
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

int
anoubis_client_serverversion(struct anoubis_client *client)
{
	return (client != NULL) ? client->server_version : -1;
}

int
anoubis_client_serverminversion(struct anoubis_client *client)
{
	return (client != NULL) ? client->server_min_version : -1;
}

int
anoubis_client_selectedversion(struct anoubis_client *client)
{
	return (client != NULL) ? client->selected_version : -1;
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
		client->flags |= FLAG_GOTCLOSEREQ;
	if (opcode == ANOUBIS_C_CLOSEACK)
		client->flags |= FLAG_GOTCLOSEACK;
	/* Make sure that we already sent a CLOSEREQ. */
	if ((client->flags & FLAG_SENTCLOSEREQ) == 0)
		goto err;
	/* The following checks for a client protocol error. */
	if ((client->flags & FLAG_GOTCLOSEACK)
	    && (client->flags & FLAG_GOTCLOSEREQ) == 0)
		goto err;
	/* Send a CLOSEACK if we can and have not yet done so. */
	if (client->flags & FLAG_GOTCLOSEREQ) {
		if ((client->flags & FLAG_SENTCLOSEACK) == 0) {
			ret = anoubis_send_general(client, ANOUBIS_C_CLOSEACK);
			if (ret < 0)
				goto err;
			client->flags |= FLAG_SENTCLOSEACK;
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
	if (client->flags & FLAG_GOTCLOSEACK
	    && client->flags & FLAG_SENTCLOSEACK) {
		client->state = ANOUBIS_STATE_CLOSED;
		acc_close(client->chan);
		client->chan = NULL;
		anoubis_transaction_done(t, 0);
	}
	return;
err:
	acc_close(client->chan);
	client->chan = NULL;
	client->state = ANOUBIS_STATE_ERROR;
	anoubis_transaction_done(t, -ret);
}

struct anoubis_transaction * anoubis_client_close_start(
    struct anoubis_client * client)
{
	struct anoubis_transaction * ret = NULL;
	static const u_int32_t nextops[] = {
	    ANOUBIS_C_CLOSEREQ, ANOUBIS_C_CLOSEACK, 0
	};

	if (client->flags & FLAG_SENTCLOSEREQ)
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
	client->flags |= FLAG_SENTCLOSEREQ;
	LIST_INSERT_HEAD(&client->ops, ret, next);
	return ret;
err:
	acc_close(client->chan);
	client->chan = NULL;
	client->state = ANOUBIS_STATE_ERROR;
	if (ret)
		free(ret);
	return NULL;
}

/* ARGUSED */
static int anoubis_client_process_event(struct anoubis_client * client,
    struct anoubis_msg * m, u_int32_t opcode, anoubis_token_t token __used)
{
	switch(opcode) {
	case ANOUBIS_N_ASK:
	case ANOUBIS_N_NOTIFY:
	case ANOUBIS_N_LOGNOTIFY:
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_NotifyMessage)))
			return -ERANGE;
		break;
	case ANOUBIS_N_POLICYCHANGE:
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_PolicyChangeMessage)))
			return -ERANGE;
		break;
	case ANOUBIS_N_STATUSNOTIFY:
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_StatusNotifyMessage)))
			return -ERANGE;
		break;
	case ANOUBIS_N_RESYOU:
	case ANOUBIS_N_RESOTHER:
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_NotifyResultMessage)))
			return -ERANGE;
		break;
	default:
		return -EINVAL;
	}
	anoubis_dump(m, "queue_notify");
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

/* Will take over and ultimately free the message! */
int anoubis_client_process(struct anoubis_client * client,
    struct anoubis_msg * m)
{
	int ret;
	u_int32_t opcode;

	/* Full lenth checks on the message. */
	ret = -EFAULT;
	if (!anoubis_msg_verify(m))
		goto err;
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
		ret = -ERANGE;
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_AckMessage)))
			goto err;
		return anoubis_client_continue_self(client, m, opcode,
		    m->u.ack->token);
	}
	if (ANOUBIS_IS_NOTIFY(opcode)) {
		anoubis_token_t token;
		ret = -ERANGE;
		if (!VERIFY_FIELD(m, token, token))
			goto err;
		token = m->u.token->token;
		ret = -EPROTO;
		if ((client->proto & ANOUBIS_PROTO_NOTIFY) == 0)
			goto err;
		switch(opcode) {
		case ANOUBIS_N_ASK:
		case ANOUBIS_N_NOTIFY:
		case ANOUBIS_N_LOGNOTIFY:
		case ANOUBIS_N_RESYOU:
		case ANOUBIS_N_RESOTHER:
		case ANOUBIS_N_POLICYCHANGE:
		case ANOUBIS_N_STATUSNOTIFY:
			ret = anoubis_client_process_event(client, m, opcode,
			    token);
			if (ret < 0)
				anoubis_msg_free(m);
			return ret;
		case ANOUBIS_N_REGISTER:
		case ANOUBIS_N_UNREGISTER:
		case ANOUBIS_N_DELEGATE:
			/* These are not allowed from the server. */
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

static void anoubis_client_ack_steps(struct anoubis_transaction * t,
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
	ret = - get_value(m->u.ack->error);
err:
	if (m->u.ack->token == 0)
		client->flags &= ~FLAG_POLICY_PENDING;
	if ((t->flags & (ANOUBIS_T_WANTMESSAGE|ANOUBIS_T_WANT_ALL)) == 0)
		anoubis_msg_free(m);
	anoubis_transaction_done(t, -ret);
}

static struct anoubis_transaction * __anoubis_client_register_start_common(
    struct anoubis_client * client, u_int32_t opcode, anoubis_token_t token,
    uid_t uid, u_int32_t rule_id, u_int32_t subsystem)
{
	struct anoubis_msg * m;
	static const u_int32_t nextops[] = { ANOUBIS_REPLY, 0 };
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
	set_value(m->u.notifyreg->rule_id, rule_id);
	set_value(m->u.notifyreg->uid, uid);
	set_value(m->u.notifyreg->subsystem, subsystem);
	ret = anoubis_transaction_create(token,
	    ANOUBIS_T_INITSELF|ANOUBIS_T_DEQUEUE,
	    &anoubis_client_ack_steps, NULL, client);
	if (!ret) {
		anoubis_msg_free(m);
		return NULL;
	}
	anoubis_transaction_setopcodes(ret, nextops);
	LIST_INSERT_HEAD(&client->ops, ret, next);
	if (anoubis_client_send(client, m) < 0) {
		anoubis_transaction_destroy(ret);
		return NULL;
	}
	return ret;
}

struct anoubis_transaction * anoubis_client_register_start(
    struct anoubis_client * client, anoubis_token_t token, uid_t uid,
    u_int32_t rule_id, u_int32_t subsystem)
{
	return  __anoubis_client_register_start_common(client,
	    ANOUBIS_N_REGISTER, token, uid, rule_id, subsystem);
}

struct anoubis_transaction * anoubis_client_unregister_start(
    struct anoubis_client * client, anoubis_token_t token, uid_t uid,
    u_int32_t rule_id, u_int32_t subsystem)
{
	return  __anoubis_client_register_start_common(client,
	    ANOUBIS_N_UNREGISTER, token, uid, rule_id, subsystem);
}

int anoubis_client_notifyreply(struct anoubis_client * client,
    anoubis_token_t token, int verdict, int delegate)
{
	struct anoubis_msg * m;
	u_int32_t opcode;

	if ((client->proto & ANOUBIS_PROTO_NOTIFY) == 0
	    || (client->flags & FLAG_SENTCLOSEACK))
		return -EINVAL;
	m = anoubis_msg_new(sizeof(Anoubis_AckMessage));
	opcode = ANOUBIS_REPLY;
	if (!m)
		return -ENOMEM;
	if (delegate)
		opcode = ANOUBIS_N_DELEGATE;
	set_value(m->u.ack->type, opcode);
	m->u.ack->token = token;
	set_value(m->u.ack->opcode, ANOUBIS_N_ASK);
	set_value(m->u.ack->error, verdict);
	return anoubis_client_send(client, m);
}

static void anoubis_client_policy_steps(struct anoubis_transaction * t,
    struct anoubis_msg * m)
{
	struct anoubis_client * client = t->cbdata;
	int ret = anoubis_client_verify(client, m);
	int flags;
	if (ret < 0)
		goto err;
	ret = -EPROTO;
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_PolicyReplyMessage))
	    || get_value(m->u.policyreply->type) != ANOUBIS_P_REPLY)
		goto err;
	ret = - get_value(m->u.policyreply->error);
	if (ret)
		goto err;
	flags = get_value(m->u.policyreply->flags);
	/* Start must be set exactly once on the first message. */
	if ((t->msg == NULL) == ((flags & POLICY_FLAG_START) == 0)) {
		ret = -EPROTO;
		goto err;
	}
	if ((flags & POLICY_FLAG_END) == 0)
		return;
err:
	/* Do not free the message because of ANOUBIS_T_WANTMESSAGE */
	anoubis_transaction_done(t, -ret);
	LIST_REMOVE(t, next);
	client->flags &= ~FLAG_POLICY_PENDING;
}

static void anoubis_client_list_ack_steps(struct anoubis_transaction * t,
    struct anoubis_msg * m)
{
	struct anoubis_client * client = t->cbdata;
	int ret = anoubis_client_verify(client, m);
	int flags;
	if (ret < 0)
		goto err;
	ret = -EPROTO;
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_ChecksumPayloadMessage))
	    || get_value(m->u.checksumpayload->type) != ANOUBIS_P_CSUM_LIST)
		goto err;
	ret = - get_value(m->u.checksumpayload->error);
	if (ret)
		goto err;
	flags = get_value(m->u.checksumpayload->flags);
	/* Start must be set exactly once on the first message. */
	if ((t->msg == NULL) == ((flags & POLICY_FLAG_START) == 0)) {
		ret = -EPROTO;
		goto err;
	}
	if ((flags & POLICY_FLAG_END) == 0)
		return;
err:
	/* Do not free the message because of ANOUBIS_T_WANTMESSAGE */
	anoubis_transaction_done(t, -ret);
	LIST_REMOVE(t, next);
	client->flags &= ~FLAG_POLICY_PENDING;
}

struct anoubis_transaction *
anoubis_client_policyrequest_start(struct anoubis_client * client,
    void * data, size_t datalen)
{
	struct anoubis_msg * m;
	struct anoubis_transaction * t = NULL;
	static const u_int32_t nextops[] = { ANOUBIS_P_REPLY, -1 };
	if ((client->proto & ANOUBIS_PROTO_POLICY) == 0)
		return NULL;
	if (client->state != ANOUBIS_STATE_CONNECTED)
		return NULL;
	if (!data)
		return NULL;
	if (client->flags & FLAG_POLICY_PENDING)
		return NULL;
	while (datalen) {
		int len = datalen;
		int flags = 0;
		if (len > SENDLEN)
			len = SENDLEN;
		m = anoubis_msg_new(sizeof(Anoubis_PolicyRequestMessage) + len);
		if (!m) {
			if (t)
				anoubis_transaction_destroy(t);
			return NULL;
		}
		if (!t) {
			t = anoubis_transaction_create(0,
			    ANOUBIS_T_INITSELF|ANOUBIS_T_WANT_ALL,
			    &anoubis_client_policy_steps, NULL, client);
			if (!t) {
				anoubis_msg_free(m);
				return NULL;
			}
			flags |= POLICY_FLAG_START;
		}
		set_value(m->u.policyrequest->type, ANOUBIS_P_REQUEST);
		memcpy(m->u.policyrequest->payload, data, len);
		data += len;
		datalen -= len;
		if (datalen == 0)
			flags |= POLICY_FLAG_END;
		set_value(m->u.policyrequest->flags, flags);
		if (anoubis_client_send(client, m) < 0) {
			anoubis_msg_free(m);
			anoubis_transaction_destroy(t);
			return NULL;
		}
	}
	anoubis_transaction_setopcodes(t, nextops);
	LIST_INSERT_HEAD(&client->ops, t, next);
	client->flags |= FLAG_POLICY_PENDING;
	return t;
}

/* If a signature is delivered to the daemon the signature, the keyid and the
 * checksum will be stored in the function parameter payload.
 *
 *	*payload:
 *	---------------------------------------------------------
 *	|	keyid	|	csum	|	sigbuf		|
 *	---------------------------------------------------------
 *
 * idlen is the length of the keyid and cslen is the length of
 * csum + sigbuf.
 *
 * If a signature is requested from daemon, the keyid only is stored in payload,
 * cslen will be 0 and the len of the keyid will be stored in idlen.
 * op == ANOUBIS_CHECKSUM_OP_{GET,DEL}SIG
 *
 *	*payload:
 *	------------------
 *	|	keyid	 |
 *	------------------
 *
 * In both cases will the memory stored in payload together with the path
 *
 *	sent message:
 *	-----------------------------------------
 *	|keyid	|(csum)|(sigbuf)|	path	|
 *	-----------------------------------------
 *
 * In a ADDSUM Operation the parameter payload contains just the checksum
 *
 *	*payload			sent message
 *	-----------------		-----------------
 *	| csum		|	--->	| csum	| path	|
 *	-----------------		-----------------
 * The send mesage will also include the path at the end.
 * In all other operations the memory will be empty.
 */
struct anoubis_transaction *
anoubis_client_csumrequest_start(struct anoubis_client *client,
    int op, const char *path, u_int8_t *payload, short cslen, short idlen,
    uid_t uid, unsigned int flags)
{
	struct anoubis_msg * m;
	struct anoubis_transaction * t = NULL;
	static const u_int32_t generalops[] = { ANOUBIS_REPLY, -1 };
	static const u_int32_t listops[] = { ANOUBIS_P_CSUM_LIST, -1 };
	char * dstpath = NULL;
	char * dstid = NULL;

	if ((client->proto & ANOUBIS_PROTO_POLICY) == 0)
		return NULL;
	if (client->state != ANOUBIS_STATE_CONNECTED)
		return NULL;
	if (client->flags & FLAG_POLICY_PENDING)
		return NULL;

	/* At first we check if the minimal conditions are met */
	switch (op) {
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
		if (idlen == 0)
			return NULL;
		/* FALL TROUGH */
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
		if (!payload)
			return NULL;
		break;
	case ANOUBIS_CHECKSUM_OP_DEL:
	case ANOUBIS_CHECKSUM_OP_GET2:
		if (idlen != 0)
			return NULL;
		break;
	case ANOUBIS_CHECKSUM_OP_GENERIC_LIST:
		break;
	default:
		/* UNKNOWN PROTOCOL */
		return NULL;
		/* NOT REACHED */
	}
	if ((op == ANOUBIS_CHECKSUM_OP_ADDSUM) && (idlen !=0))
		return NULL;
	if (!path || !strlen(path))
		return NULL;
	/* Everything seems fine, lets start the operation */
	if (payload && cslen) {
		if ((op != ANOUBIS_CHECKSUM_OP_ADDSUM) &&
		    (op != ANOUBIS_CHECKSUM_OP_ADDSIG))
			return NULL;
		m = anoubis_msg_new(sizeof(Anoubis_ChecksumAddMessage)
		    + cslen + idlen + strlen(path) + 1);
		if (!m)
			return NULL;
		dstpath = m->u.checksumadd->payload + cslen + idlen;
		dstid = NULL;
		set_value(m->u.checksumadd->uid, uid);
		set_value(m->u.checksumadd->flags, flags);
		set_value(m->u.checksumadd->cslen, cslen);
		memcpy(m->u.checksumadd->payload, payload, cslen + idlen);
	} else {
		m = anoubis_msg_new(sizeof(Anoubis_ChecksumRequestMessage)
		    + strlen(path) + idlen + 1);
		if (!m)
			return NULL;
		dstid = m->u.checksumrequest->payload;
		dstpath = m->u.checksumrequest->payload + idlen;
	}
	if (op == ANOUBIS_CHECKSUM_OP_GENERIC_LIST) {
		t = anoubis_transaction_create(0,
		ANOUBIS_T_INITSELF|ANOUBIS_T_WANT_ALL,
		&anoubis_client_list_ack_steps, NULL, client);
	} else {
		t = anoubis_transaction_create(0,
		    ANOUBIS_T_INITSELF|ANOUBIS_T_DEQUEUE|ANOUBIS_T_WANTMESSAGE,
		    &anoubis_client_ack_steps, NULL, client);
	}
	if (!t) {
		anoubis_msg_free(m);
		return NULL;
	}
	set_value(m->u.checksumrequest->type, ANOUBIS_P_CSUMREQUEST);
	set_value(m->u.checksumrequest->uid, uid);
	set_value(m->u.checksumrequest->flags, flags);
	set_value(m->u.checksumrequest->idlen, idlen);
	set_value(m->u.checksumrequest->operation, op);

	if (op == ANOUBIS_CHECKSUM_OP_GETSIG2 ||
	    op == ANOUBIS_CHECKSUM_OP_GENERIC_LIST ||
	    op == ANOUBIS_CHECKSUM_OP_DELSIG) {
		if (idlen && dstid)
			memcpy(dstid, payload, idlen);
	}
	strlcpy(dstpath, path, strlen(path)+1);
	if (anoubis_client_send(client, m) < 0) {
		anoubis_msg_free(m);
		anoubis_transaction_destroy(t);
		return NULL;
	}

	if (op == ANOUBIS_CHECKSUM_OP_GENERIC_LIST)
		anoubis_transaction_setopcodes(t, listops);
	else
		anoubis_transaction_setopcodes(t, generalops);

	LIST_INSERT_HEAD(&client->ops, t, next);
	client->flags |= FLAG_POLICY_PENDING;
	return t;
}

struct anoubis_csmulti_request *
anoubis_csmulti_create(unsigned int op, uid_t uid, void *keyid, int idlen)
{
	struct anoubis_csmulti_request	*ret;

	if (idlen < 0)
		return NULL;
	switch(op) {
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
		if (uid || !idlen || !keyid)
			return NULL;
		break;
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case ANOUBIS_CHECKSUM_OP_GET2:
	case ANOUBIS_CHECKSUM_OP_DEL:
		if (idlen || keyid)
			return NULL;
		break;
	default:
		return NULL;
	}

	ret = malloc(sizeof(struct anoubis_csmulti_request) + idlen);
	if (!ret)
		return NULL;
	ret->client = NULL;
	ret->op = op;
	ret->uid = uid;
	ret->idlen = idlen;
	ret->last = NULL;
	if (idlen) {
		ret->keyid = (void*)(ret + 1);
		memcpy(ret->keyid, keyid, idlen);
	} else {
		ret->keyid = NULL;
	}
	ret->nreqs = 0;
	ret->reply_msg = NULL;
	TAILQ_INIT(&ret->reqs);
	return ret;
}

/**
 * Search for the request record with the given index in the request
 * queue of the request req.
 *
 * @param req The request.
 * @param idx The index.
 * @return A pointer to the request record.
 */
static struct anoubis_csmulti_record *
anoubis_csmulti_find(struct anoubis_csmulti_request *req, unsigned int idx)
{
	struct anoubis_csmulti_record	*record = req->last;

	while(record) {
		if (record->idx == idx) {
			req->last = record;
			return record;
		}
		record = TAILQ_NEXT(record, next);
	}
	record = TAILQ_FIRST(&req->reqs);
	while(record && record != req->last) {
		if (record->idx == idx) {
			req->last = record;
			return record;
		}
		record = TAILQ_NEXT(record, next);
	}
	return NULL;
}

void
anoubis_csmulti_destroy(struct anoubis_csmulti_request *request)
{
	struct anoubis_csmulti_record	*cur;
	struct anoubis_msg		*tmp;

	if (!request)
		return;
	request->last = NULL;
	while ((cur = TAILQ_FIRST(&request->reqs)) != NULL) {
		TAILQ_REMOVE(&request->reqs, cur, next);
		free(cur);
	}
	while ((tmp = request->reply_msg) != NULL) {
		request->reply_msg = tmp->next;
		anoubis_msg_free(tmp);
	}
	free(request);
}

int
anoubis_csmulti_add(struct anoubis_csmulti_request *request,
    const char *path, const void *csdata, unsigned int cslen)
{
	struct anoubis_csmulti_record	*record;
	void				*data;
	int				 plen;

	if (!path)
		return -EINVAL;
	plen = strlen(path) + 1;
	switch (request->op) {
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
		if (!cslen || !csdata)
			return -EINVAL;
		break;
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
	case ANOUBIS_CHECKSUM_OP_GET2:
	case ANOUBIS_CHECKSUM_OP_DEL:
		if (cslen || csdata)
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}
	record = malloc(sizeof(struct anoubis_csmulti_record) + cslen + plen);
	if (!record)
		return -ENOMEM;
	data = (void *)(record + 1);
	record->path = data + cslen;
	memcpy(record->path, path, plen);
	record->error = EAGAIN;
	record->idx = request->nreqs++;
	record->length = sizeof(Anoubis_CSMultiRequestRecord) + cslen + plen;
	/* Align */
	if (record->length % 4)
		record->length += 4 - record->length % 4;
	switch (request->op) {
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
		record->u.add.cslen = cslen;
		record->u.add.csdata = (void*)(record+1);
		memcpy(data, csdata, cslen);
		break;
	default:
		record->u.get.csum = NULL;
		record->u.get.sig = NULL;
		record->u.get.upgrade = NULL;
	}
	TAILQ_INSERT_TAIL(&request->reqs, record, next);
	return 0;
}

/**
 * Callback handler for CSMULTI request. This handler gets called when
 * the ANOUBIS_T_DONE flag on a CSMULTI request is set. It takes over
 * responsibility for the reply message.
 *
 * @param t The transaction. The anoubis_csmulti_request associated with
 *     the transaction can be found in the callback data.
 * @return None.
 */
static void
anoubis_client_csmulti_finish(struct anoubis_transaction *t)
{
	struct anoubis_csmulti_request	*request = t->cbdata;

	request->client = NULL;
	if (t->result == 0) {
		/* In case of success,  the request owns the message. */
		t->msg->next = request->reply_msg;
		request->reply_msg = t->msg;
		t->msg = NULL;
	}
}

/**
 * The methods processes the reply of a CSMULTI request and stores the
 * data in the anoubis_csmulti_request associated with the transaction.
 *
 * @param t The transaction.
 * @param m The reply message.
 */
static void
anoubis_client_csmulti_steps(struct anoubis_transaction *t,
    struct anoubis_msg *m)
{
	struct anoubis_csmulti_request	*request = t->cbdata;
	struct anoubis_client		*client = request->client;
	unsigned int			 off = 0;
	int				 ret;

	ret = -EPROTO;
	if (!request || !client)
		goto err;
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_CSMultiReplyMessage)))
		goto err;
	if (!get_value(m->u.csmultireply->operation) == request->op)
		goto err;
	ret = -get_value(m->u.csmultireply->error);
	if (ret != 0)
		goto err;

	ret = -EFAULT;
	while (1) {
		Anoubis_CSMultiReplyRecord	*r;
		unsigned int			 rlen;
		unsigned int			 idx;
		unsigned int			 off2;
		struct anoubis_csmulti_record	*record;

		if (!VERIFY_LENGTH(m, sizeof(Anoubis_CSMultiReplyMessage)
		    + off + sizeof(Anoubis_CSMultiReplyRecord)))
			goto err;
		r = (Anoubis_CSMultiReplyRecord *)
		    (m->u.csmultireply->payload + off);
		rlen = get_value(r->length);
		if (rlen == 0)
			break;
		if (rlen < sizeof(Anoubis_CSMultiReplyRecord))
			goto err;
		if (off + rlen < off)
			goto err;
		off += rlen;
		if (!VERIFY_LENGTH(m,
		    sizeof(Anoubis_CSMultiReplyMessage) + off))
			goto err;
		idx = get_value(r->index);
		record = anoubis_csmulti_find(request, idx);
		if (!record)
			continue;
		record->error = get_value(r->error);
		/* No payload data in case of an error. */
		if (record->error)
			continue;
		/* No payload data if request type is not GET */
		if (request->op != ANOUBIS_CHECKSUM_OP_GET2
		    && request->op != ANOUBIS_CHECKSUM_OP_GETSIG2)
			continue;
		/*
		 * Iterate through the checksum data and process it.
		 * The checksum data remains stored in the message and
		 * the result record only points into the message!
		 */
		rlen -= sizeof(Anoubis_CSMultiReplyRecord);
		off2 = 0;
		while (1) {
			struct anoubis_csentry	*e;
			unsigned int		 total;
			unsigned int		 cstype;

			if (rlen < sizeof(struct anoubis_csentry))
				goto err;
			e = (struct anoubis_csentry *)(r->payload + off2);
			total = sizeof(struct anoubis_csentry)
			    + get_value(e->cslen);
			if (total > rlen)
				goto err;
			rlen -= total;
			off2 += total;
			cstype =  get_value(e->cstype);
			if (cstype == ANOUBIS_SIG_TYPE_EOT)
				break;
			switch(cstype) {
			case ANOUBIS_SIG_TYPE_CS:
				record->u.get.csum = e;
				break;
			case ANOUBIS_SIG_TYPE_SIG:
				record->u.get.sig = e;
				break;
			case ANOUBIS_SIG_TYPE_UPGRADECS:
				record->u.get.upgrade = e;
				break;
			}
		}
	}
	ret = 0;
err:
	/* Do not free the message because of ANOUBIS_T_WANTMESSAGE */
	anoubis_transaction_done(t, -ret);
	LIST_REMOVE(t, next);
	client->flags &= ~FLAG_POLICY_PENDING;
}

struct anoubis_msg *
anoubis_csmulti_msg(struct anoubis_csmulti_request *request)
{
	struct anoubis_msg		*m;
	struct anoubis_csmulti_record	*record;
	unsigned int			 length = 0, recoff;
	int				 add = 0;
	unsigned int			 nrec = 0;
	Anoubis_CSMultiRequestRecord	*r;

	/* First we check if the minimal conditions are met */
	switch (request->op) {
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
		add = 1;
		/* FALLTHROUGH */
	case ANOUBIS_CHECKSUM_OP_DELSIG:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
		if (request->idlen == 0 || request->keyid == NULL)
			return NULL;
		break;
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
		add = 1;
		/* FALLTHROUGH */
	case ANOUBIS_CHECKSUM_OP_DEL:
	case ANOUBIS_CHECKSUM_OP_GET2:
		if (request->idlen != 0)
			return NULL;
		break;
	default:
		return NULL;
	}

	/* Step 1: Calculate the total length of the message */

	length = sizeof(Anoubis_CSMultiRequestMessage);
	length += request->idlen;
	/* Align request records on an 4 byte boundary. */
	if (length % 4)
		length += 4 - length % 4;
	recoff = length - offsetof(Anoubis_CSMultiRequestMessage, payload);

	TAILQ_FOREACH(record, &request->reqs, next) {
		/* Only request records that have not yet been asked for. */
		if (record->error != EAGAIN)
			continue;
		/* Limit message size (currently to 8000 bytes) */
		if (length + record->length > 8000)
			break;
		length += record->length;
		nrec++;
	}
	/* Sentinel */
	length += sizeof(Anoubis_CSMultiRequestRecord);

	/* Step 2: Allocate and fill the message */

	m = anoubis_msg_new(length);
	if (!m)
		return NULL;
	set_value(m->u.csmultireq->type, ANOUBIS_P_CSMULTIREQUEST);
	set_value(m->u.csmultireq->operation, request->op);
	set_value(m->u.csmultireq->uid, request->uid);
	set_value(m->u.csmultireq->idlen, request->idlen);
	set_value(m->u.csmultireq->recoff, recoff);
	if (request->idlen)
		memcpy(m->u.csmultireq->payload, request->keyid,
		    request->idlen);
	TAILQ_FOREACH(record, &request->reqs, next) {
		unsigned int	plen = strlen(record->path) + 1;

		r = (Anoubis_CSMultiRequestRecord *)
		    (m->u.csmultireq->payload + recoff);

		/* Only request records that have not yet been asked for. */
		if (record->error != EAGAIN)
			continue;
		set_value(r->length, record->length);
		set_value(r->index, record->idx);
		if (add) {
			unsigned int	cslen = record->u.add.cslen;
			set_value(r->cslen, record->u.add.cslen);
			memcpy(r->payload, record->u.add.csdata,
			    record->u.add.cslen);
			memcpy(r->payload + cslen, record->path, plen);
		} else {
			set_value(r->cslen, 0);
			memcpy(r->payload, record->path, plen);
		}
		recoff += record->length;
		nrec--;
		if (nrec == 0)
			break;
	}
	/* Sentinel */
	r = (Anoubis_CSMultiRequestRecord *)(m->u.csmultireq->payload + recoff);
	set_value(r->length, 0);
	set_value(r->index, 0);
	set_value(r->cslen, 0);

	return m;
}

struct anoubis_transaction *
anoubis_client_csmulti_start(struct anoubis_client *client,
    struct anoubis_csmulti_request *request)
{
	static const u_int32_t ops[] = { ANOUBIS_P_CSMULTIREPLY, -1 };

	struct anoubis_msg		*m;
	struct anoubis_transaction	*t = NULL;

	if ((client->proto & ANOUBIS_PROTO_POLICY) == 0)
		return NULL;
	if (client->state != ANOUBIS_STATE_CONNECTED)
		return NULL;
	if (client->flags & FLAG_POLICY_PENDING)
		return NULL;
	if (request->client)
		return NULL;

	m = anoubis_csmulti_msg(request);
	if (!m)
		return NULL;

	request->client = client;
	t = anoubis_transaction_create(0,
	    ANOUBIS_T_INITSELF|ANOUBIS_T_DEQUEUE|ANOUBIS_T_WANTMESSAGE,
	    &anoubis_client_csmulti_steps, &anoubis_client_csmulti_finish,
	    request);
	if (!t) {
		request->client = NULL;
		anoubis_msg_free(m);
		return NULL;
	}
	if (anoubis_client_send(client, m) < 0) {
		request->client = NULL;
		anoubis_msg_free(m);
		anoubis_transaction_destroy(t);
		return NULL;
	}
	anoubis_transaction_setopcodes(t, ops);
	LIST_INSERT_HEAD(&client->ops, t, next);
	client->flags |= FLAG_POLICY_PENDING;
	return t;
}

struct anoubis_transaction *
anoubis_client_passphrase_start(struct anoubis_client *client,
    const char *passphrase)
{
	struct anoubis_msg		*m;
	struct anoubis_transaction	*t = NULL;
	static const u_int32_t		 nextops[] = { ANOUBIS_REPLY, -1 };

	if ((client->proto & ANOUBIS_PROTO_POLICY) == 0)
		return NULL;
	if (client->state != ANOUBIS_STATE_CONNECTED)
		return NULL;
	if (client->flags & FLAG_POLICY_PENDING)
		return NULL;
	m = anoubis_msg_new(sizeof(Anoubis_PassphraseMessage)
	    + strlen(passphrase) + 1);
	if (!m)
		return NULL;
	set_value(m->u.passphrase->type, ANOUBIS_P_PASSPHRASE);
	memcpy(m->u.passphrase->payload, passphrase, strlen(passphrase) + 1);
	t = anoubis_transaction_create(0, ANOUBIS_T_INITSELF|ANOUBIS_T_DEQUEUE,
	    &anoubis_client_ack_steps, NULL, client);
	if (anoubis_client_send(client, m) < 0) {
		anoubis_msg_free(m);
		anoubis_transaction_destroy(t);
		return NULL;
	}
	anoubis_transaction_setopcodes(t, nextops);
	LIST_INSERT_HEAD(&client->ops, t, next);
	client->flags |= FLAG_POLICY_PENDING;
	return t;
}

/* Sync functions */
int anoubis_client_connect(struct anoubis_client * client, unsigned int proto)
{
	struct anoubis_transaction * t;
	struct anoubis_msg * m;
	int ret;

	t = anoubis_client_connect_start(client, proto);
	if (!t)
		return -EINVAL;
	while(1) {
		m = anoubis_client_rcv(client);
		if (!m)
			return -EPROTO;
		/* Will free the message. */
		anoubis_client_continue_connect(client, m,
		    get_value(m->u.general->type));
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}
	ret = -t->result;
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
		client->chan = NULL;
		client->state = ANOUBIS_STATE_ERROR;
		return;
	}
	while((m = anoubis_client_rcv(client))) {
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

int anoubis_client_wait(struct anoubis_client * client)
{
	struct anoubis_msg * m = anoubis_client_rcv(client);
	if (!m)
		return 0;
	return anoubis_client_process(client, m);
}

static void
anoubis_client_version_steps(struct anoubis_transaction *t,
    struct anoubis_msg *m)
{
	struct anoubis_client * client = t->cbdata;
	int ret = anoubis_client_verify(client, m);

	if (ret < 0)
		goto err;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_VersionMessage))
	    || get_value(m->u.version->type) != ANOUBIS_P_VERSIONREPLY) {
		ret = -EPROTO;
		goto err;
	}
	ret = - get_value(m->u.version->error);

err:
	/* Do not free the message because of ANOUBIS_T_WANTMESSAGE */
	anoubis_transaction_done(t, -ret);
	LIST_REMOVE(t, next);
	client->flags &= ~FLAG_POLICY_PENDING;
}

struct anoubis_transaction *
anoubis_client_version_start(struct anoubis_client *client)
{
	struct anoubis_msg * m;
	struct anoubis_transaction * t = NULL;
	static const u_int32_t nextops[] = { ANOUBIS_P_VERSIONREPLY, -1 };
	if ((client->proto & ANOUBIS_PROTO_POLICY) == 0)
		return NULL;
	if (client->state != ANOUBIS_STATE_CONNECTED)
		return NULL;
	if (client->flags & FLAG_POLICY_PENDING)
		return NULL;
	m = anoubis_msg_new(sizeof(Anoubis_GeneralMessage));
	if (!m)
		return NULL;
	set_value(m->u.general->type, ANOUBIS_P_VERSION);
	t = anoubis_transaction_create(0,
	    ANOUBIS_T_INITSELF|ANOUBIS_T_WANT_ALL,
	    &anoubis_client_version_steps, NULL, client);
	if (anoubis_client_send(client, m) < 0) {
		anoubis_msg_free(m);
		anoubis_transaction_destroy(t);
		return NULL;
	}
	anoubis_transaction_setopcodes(t, nextops);
	LIST_INSERT_HEAD(&client->ops, t, next);
	client->flags |= FLAG_POLICY_PENDING;

	return (t);
}

/*@=memchecks@*/

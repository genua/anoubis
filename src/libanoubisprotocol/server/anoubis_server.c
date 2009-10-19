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
#include <unistd.h>
#include <sys/types.h>
#include <anoubischat.h>
#ifndef NEEDBSDCOMPAT
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <apn.h>

#include "anoubis_protocol.h"
#include "anoubis_server.h"
#include "anoubis_errno.h"
#include "anoubis_msg.h"
#include "anoubis_dump.h"
#include "anoubis_notify.h"
#include "anoubis_policy.h"
#include "crc32.h"

struct dispatcher {
	LIST_ENTRY(dispatcher) next;
	int opcode;
	anoubis_dispatcher_t dispatch;
	void * arg;
};

struct anoubis_server {
	unsigned int proto;
	unsigned int connect_flags;
	unsigned int auth_uid;
	/*@relnull@*/ /*@dependent@*/
	struct achat_channel * chan;
	/*@relnull@*/
	struct anoubis_auth * auth;
	/*@relnull@*/
	struct anoubis_notify_group * notify;
	/*@relnull@*/ /*@dependent@*/
	struct anoubis_policy_comm * policy;
	LIST_HEAD(, dispatcher) dispatch;
};

#define FLAG_VERSEL			0x0001
#define FLAG_AUTH			0x0002
#define FLAG_OPTIONS			0x0004
#define FLAG_PROTOCOLS			0x0008
#define FLAG_MULTIPLEX			0x0010
#define FLAG_PIPELINE			0x0020
#define FLAG_OOB			0x0040
#define FLAG_HELLOSENT			0x0080
#define FLAG_GOTCLOSEREQ		0x0100
#define FLAG_SENTCLOSEREQ		0x0200
#define FLAG_GOTCLOSEACK		0x0400
#define FLAG_SENTCLOSEACK		0x0800
#define FLAG_ERROR			0x1000

struct anoubis_server * anoubis_server_create(struct achat_channel * chan,
    struct anoubis_policy_comm * policy)
{
	struct anoubis_server * ret = malloc(sizeof(struct anoubis_server));
	if (!ret)
		return NULL;
	ret->proto = ANOUBIS_PROTO_CONNECT;
	ret->connect_flags = 0;
	ret->chan = chan;
	ret->auth = NULL;
	ret->auth_uid = (unsigned int) -1;
	ret->policy = policy;
	ret->notify = NULL;
	LIST_INIT(&ret->dispatch);
	return ret;
};

static void channel_close(struct anoubis_server * server)
{
	if (!server)
		return;
	if (server->policy && server->chan)
		anoubis_policy_comm_abort(server->policy, server->chan);
	if (server->chan) {
		acc_close(server->chan);
		server->chan = NULL;
	}
}

void anoubis_server_destroy(struct anoubis_server * server)
{
	if (server->auth) {
		anoubis_auth_destroy(server->auth);
		server->auth = NULL;
	}
	if (server->notify) {
		anoubis_notify_destroy(server->notify);
		server->notify = NULL;
	}
	channel_close(server);
	free(server);
}

static int anoubis_server_send(struct anoubis_server * server,
    /*@only@*/ struct anoubis_msg * m)
{
	int ret;

	crc32_set(m->u.buf, m->length);
	anoubis_dump(m, "Server send");
	ret = acc_sendmsg(server->chan, m->u.buf, m->length);
	anoubis_msg_free(m);
	if (ret != ACHAT_RC_OK)
		return -EIO;
	/*
	 * The connection is closing and we just sent the last pending packet.
	 */
	if ((server->connect_flags & (FLAG_SENTCLOSEREQ|FLAG_GOTCLOSEREQ))
	    == (FLAG_SENTCLOSEREQ|FLAG_GOTCLOSEREQ)) {
		struct anoubis_msg * m;
		/* We sent the close ack, too. */
		if (server->connect_flags & FLAG_SENTCLOSEACK) {
			if (server->connect_flags & FLAG_GOTCLOSEACK)
				channel_close(server);
			return 0;
		}
		m = anoubis_msg_new(sizeof(Anoubis_GeneralMessage));
		if (!m) {
			if (server->connect_flags & FLAG_GOTCLOSEACK)
				channel_close(server);
			return -ENOMEM;
		}
		set_value(m->u.general->type, ANOUBIS_C_CLOSEACK);
		server->connect_flags |= FLAG_SENTCLOSEACK;
		/* Recursive but at most once! */
		return anoubis_server_send(server, m);
	}
	return 0;
}

/* Start communication by sending initial HELLO.*/
int anoubis_server_start(struct anoubis_server * server)
{
	struct anoubis_msg * m;
	int ret;

	m = anoubis_msg_new(sizeof(Anoubis_HelloMessage));
	if (!m)
		return -ENOMEM;

	assert(server->proto == ANOUBIS_PROTO_CONNECT);
	assert((server->connect_flags & FLAG_HELLOSENT) == 0);

	set_value(m->u.hello->type, ANOUBIS_C_HELLO);
	set_value(m->u.hello->version, ANOUBIS_PROTO_VERSION);
	set_value(m->u.hello->min_version, ANOUBIS_PROTO_MINVERSION);
	ret = anoubis_server_send(server, m);
	if (ret < 0)
		return ret;
	server->connect_flags |= FLAG_HELLOSENT;
	return 0;
}

static int __reply(struct anoubis_server * server, anoubis_token_t token,
    int opcode, int error)
{
	struct anoubis_msg * m;
	m = anoubis_msg_new(sizeof(Anoubis_AckMessage));
	if (!m)
		return -ENOMEM;
	set_value(m->u.ack->type, ANOUBIS_REPLY);
	set_value(m->u.ack->error, error);
	set_value(m->u.ack->opcode, opcode);
	m->u.ack->token = token;
	return anoubis_server_send(server, m);
}

static int reply_invalid(struct anoubis_server * server, int opcode)
{
	return __reply(server, 0, opcode, ANOUBIS_E_INVAL);
}

static int reply_ok(struct anoubis_server * server, int opcode)
{
	return __reply(server, 0, opcode, ANOUBIS_E_OK);
}

static int reply_ok_token(struct anoubis_server * server,
    anoubis_token_t token, int opcode)
{
	return __reply(server, token, opcode, ANOUBIS_E_OK);
}

static int reply_invalid_token(struct anoubis_server * server,
    anoubis_token_t token, int opcode)
{
	return __reply(server, token, opcode, ANOUBIS_E_INVAL);
}

static int anoubis_process_versel(struct anoubis_server * server,
    struct anoubis_msg * m, int opcode)
{
	if (!VERIFY_LENGTH(m, sizeof(m->u.versel)))
		return reply_invalid(server, opcode);
	if (server->proto != ANOUBIS_PROTO_CONNECT
	    || (server->connect_flags & FLAG_VERSEL)
	    || get_value(m->u.versel->version) != ANOUBIS_PROTO_VERSION) {
		return reply_invalid(server, opcode);
	}
	server->connect_flags |= FLAG_VERSEL;
	return reply_ok(server, opcode);
}

static struct proto_opt anoubis_options[] = {
	{ FLAG_MULTIPLEX,	"PROTOMULTIPLEX"	},
	{ FLAG_PIPELINE,	"PIPELINE"		},
	{ FLAG_OOB,		"OUTOFBAND"		},
	{ -1,			""			},
};

static int anoubis_process_optreq(struct anoubis_server * server,
    /*@dependent@*/ struct anoubis_msg * m, int opcode)
{
	struct stringlist_iterator it;
	struct anoubis_msg * rep;
	int opts[4];
	int optidx = 0;
	int reqopt;
	if (server->proto != ANOUBIS_PROTO_CONNECT)
		return reply_invalid(server, opcode);
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_StringListMessage)))
		return reply_invalid(server, opcode);
	stringlist_iterator_init(&it, m, anoubis_options);
	while((reqopt = stringlist_iterator_get(&it)) >= 0) {
		switch (reqopt) {
		case FLAG_MULTIPLEX:
			if (server->connect_flags & FLAG_MULTIPLEX)
				break;
			server->connect_flags |= FLAG_MULTIPLEX;
			opts[optidx++] = FLAG_MULTIPLEX;
			break;
		case FLAG_PIPELINE:
			/*
			 * In an event driven environment, pipelining is
			 * difficult. We do not support this yet.
			 */
			break;
		case FLAG_OOB:
			/* OOB not yet supported by protocol layer. */
			break;
		}
	}
	opts[optidx] = -1;
	rep = anoubis_stringlist_msg(ANOUBIS_C_OPTACK, anoubis_options, opts);
	if (!rep)
		return -ENOMEM;
	return anoubis_server_send(server, rep);
}

static void anoubis_auth_finish_callback(void * data);

static int anoubis_process_auth(/*@dependent@*/ struct anoubis_server * server,
    struct anoubis_msg * m, int opcode)
{
	if (server->proto != ANOUBIS_PROTO_CONNECT
	    || (server->connect_flags & FLAG_HELLOSENT) == 0
	    || (server->connect_flags & FLAG_AUTH)
	    || (server->auth != NULL))
		return reply_invalid(server, opcode);
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_GeneralMessage)))
		return reply_invalid(server, opcode);

	/*
	 * server->auth has been checked for non-null at the top of this
	 * function.
	 */
	/*@i@*/	server->auth = anoubis_auth_create(server->chan,
	    &anoubis_auth_finish_callback, server);
	if (!server->auth)
		return -ENOMEM;
	return reply_ok(server, opcode);
}

static int anoubis_process_authdata(struct anoubis_server * server,
    struct anoubis_msg * m, int opcode)
{
	if (server->proto != ANOUBIS_PROTO_CONNECT
	    || (server->connect_flags &  FLAG_HELLOSENT) == 0
	    || (server->connect_flags & FLAG_AUTH)
	    || server->auth == NULL)
		return reply_invalid(server, opcode);
	return anoubis_auth_process(server->auth, m);
}

static void anoubis_auth_finish_callback(void * data)
{
	struct anoubis_server * server = data;
	struct anoubis_msg * m;

	/* More Messages necessary. */
	if (server->auth->state == ANOUBIS_AUTH_SUCCESS) {
		size_t len = 0;
		if (server->auth->username)
			len = strlen(server->auth->username);
		m = anoubis_msg_new(sizeof(Anoubis_AuthReplyMessage) + len);
		if (!m) {
			server->connect_flags |= FLAG_ERROR;
			channel_close(server);
			return;
		}
		set_value(m->u.authreply->type, ANOUBIS_C_AUTHREPLY);
		set_value(m->u.authreply->error, ANOUBIS_E_OK);
		set_value(m->u.authreply->uid, server->auth->uid);
		if (len > 0)
			/*
			 * len contains the length of the string pointed
			 * to server->auth->username, and thus cannot be
			 * possibly null.
			 */
			strncpy(m->u.authreply->name,
			    /*@i@*/server->auth->username, len);
		server->connect_flags |= FLAG_AUTH;
		server->auth_uid = server->auth->uid;
	} else {
		assert(server->auth->state == ANOUBIS_AUTH_FAILURE);
		m = anoubis_msg_new(sizeof(Anoubis_AuthReplyMessage));
		if (!m) {
			server->connect_flags |= FLAG_ERROR;
			channel_close(server);
			return;
		}
		set_value(m->u.authreply->type, ANOUBIS_C_AUTHREPLY);
		set_value(m->u.authreply->error, server->auth->error);
		set_value(m->u.authreply->uid, -1);
	}
	anoubis_auth_destroy(server->auth);
	server->auth = NULL;
	if (anoubis_server_send(server, m) < 0) {
		server->connect_flags |= FLAG_ERROR;
		channel_close(server);
	}
}

static int anoubis_process_protosel(struct anoubis_server * server,
    struct anoubis_msg * m, int opcode)
{
	int k;
	size_t slen;
	static const char * const proto_strings[4] = {
		"POLICY",
		"NOTIFY",
		"POLICY,NOTIFY",
		"NOTIFY,POLICY",
	};
	static const int newproto[4] = {
		ANOUBIS_PROTO_POLICY,
		ANOUBIS_PROTO_NOTIFY,
		ANOUBIS_PROTO_BOTH,
		ANOUBIS_PROTO_BOTH,
	};

	if (server->proto != ANOUBIS_PROTO_CONNECT
	    || (server->connect_flags & FLAG_HELLOSENT) == 0
	    || (server->connect_flags & FLAG_AUTH) == 0
	    || (server->connect_flags & FLAG_VERSEL) == 0)
		return reply_invalid(server, opcode);
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_StringListMessage)))
		return reply_invalid(server, opcode);
	slen = m->length - CSUM_LEN - sizeof(Anoubis_StringListMessage);
	for (k=0; k<4; ++k) {
		if (slen != strlen(proto_strings[k]))
			continue;
		if (strncmp(m->u.stringlist->stringlist,
		    proto_strings[k], slen) != 0)
			continue;
		if ((server->connect_flags & FLAG_MULTIPLEX) == 0
		    && newproto[k] == ANOUBIS_PROTO_BOTH)
			break;
		server->proto = newproto[k];
		if (server->proto & ANOUBIS_PROTO_NOTIFY) {
			/*
			 * At this point, server->notify cannot be
			 * allocated, because the protocol specification
			 * says, that an initialised anoubis_server
			 * structure has set server->proto to some other
			 * value than ANOUBIS_PROTO_CONNECT. That case
			 * is already excluded at the top of this
			 * function.
			 */
			/*@i@*/ server->notify = anoubis_notify_create(
			    server->chan, server->auth_uid);
			if (!server->notify)
				return reply_invalid(server, opcode);
		}
		return reply_ok(server, opcode);
	}
	return reply_invalid(server, opcode);
}

static int anoubis_process_closereq(struct anoubis_server * server, int opcode)
{
	struct anoubis_msg * m;
	int ret;

	/*
	 * Treat CLOSEACK identical to CLOSEREQ except that we also
	 * close the read end of connection in case of a close ack.
	 */
	if (opcode == ANOUBIS_C_CLOSEACK) {
		server->connect_flags |= FLAG_GOTCLOSEACK;
		if (server->connect_flags & FLAG_SENTCLOSEACK)
			channel_close(server);
	}
	server->connect_flags |= FLAG_GOTCLOSEREQ;
	/*
	 * Requests in the policy protocol are initiated by the client.
	 * we do not allow these after the client sent a close request.
	 * Notify requests must be treated on an individual basis.
	 */
	server->proto &= ANOUBIS_PROTO_NOTIFY;
	/* If we already sent a CLOSEREQ we are done. */
	if (server->connect_flags & FLAG_SENTCLOSEREQ)
		return 0;
	m = anoubis_msg_new(sizeof(Anoubis_GeneralMessage));
	if (!m)
		return -ENOMEM;
	set_value(m->u.general->type, ANOUBIS_C_CLOSEREQ);
	server->connect_flags |= FLAG_SENTCLOSEREQ;
	ret = anoubis_server_send(server, m);
	if (ret < 0)
		return ret;
	return 0;
}

static int anoubis_process_nregister(struct anoubis_server * server,
    struct anoubis_msg * m, int opcode, anoubis_token_t token)
{
	int ret;
	u_int32_t uid, ruleid, subsystem;

	if (opcode != ANOUBIS_N_REGISTER && opcode != ANOUBIS_N_UNREGISTER)
		return -EINVAL;
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_NotifyRegMessage))
	    || (server->proto & ANOUBIS_PROTO_NOTIFY) == 0
	    || (server->notify == NULL))
		return reply_invalid_token(server, token, opcode);
	uid = get_value(m->u.notifyreg->uid);
	ruleid = get_value(m->u.notifyreg->rule_id);
	subsystem = get_value(m->u.notifyreg->subsystem);
	if (opcode == ANOUBIS_N_REGISTER) {
		ret = anoubis_notify_register(server->notify, uid,
		    ruleid, subsystem);
	} else {
		ret = anoubis_notify_unregister(server->notify, uid,
		    ruleid, subsystem);
	}
	if (ret < 0)
		return reply_invalid_token(server, token, opcode);
	return reply_ok_token(server, token, opcode);
}

static int
anoubis_process_version(struct anoubis_server *server, struct anoubis_msg *m,
    int opcode)
{
	struct anoubis_msg	*response;

	if (opcode != ANOUBIS_P_VERSION)
		return -EINVAL;

	response = anoubis_msg_new(sizeof(Anoubis_VersionMessage));
	if (!response)
		return -ENOMEM;

	set_value(response->u.version->type, ANOUBIS_P_VERSIONREPLY);
	set_value(response->u.version->error, 0);
	set_value(response->u.version->protocol, 0);
	set_value(response->u.version->apn, 0);

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_GeneralMessage))
	    || get_value(m->u.general->type) != ANOUBIS_P_VERSION
	    || (server->proto & ANOUBIS_PROTO_POLICY) == 0) {
		set_value(response->u.version->error, EINVAL);
		return (anoubis_server_send(server, response));
	}

	set_value(response->u.version->protocol, ANOUBIS_PROTO_VERSION);
	set_value(response->u.version->apn, apn_parser_version());

	return (anoubis_server_send(server, response));
}

static int anoubis_process_reply(struct anoubis_server * server,
    struct anoubis_msg * m, int opcode, anoubis_token_t token)
{
	int verdict;
	int delegate;

	if (opcode != ANOUBIS_REPLY && opcode != ANOUBIS_N_DELEGATE)
		return -EINVAL;
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_AckMessage))
	    || (server->proto & ANOUBIS_PROTO_NOTIFY) == 0
	    || (server->notify == NULL))
		return reply_invalid_token(server, token, opcode);
	verdict = get_value(m->u.ack->error);
	delegate = (opcode == ANOUBIS_N_DELEGATE);
	if (anoubis_notify_answer(server->notify, token, verdict, delegate) < 0)
		return reply_invalid_token(server, token, opcode);
	return 0;
}

int anoubis_dispatch_create(struct anoubis_server *server, int opcode,
    anoubis_dispatcher_t dispatch, void *arg)
{
	struct dispatcher	*disp;

	if (dispatch == NULL)
		return -EINVAL;
	LIST_FOREACH(disp, &server->dispatch, next) {
		if (disp->opcode == opcode)
			return -EBUSY;
	}
	disp = malloc(sizeof(struct dispatcher));
	if (!disp)
		return -ENOMEM;
	disp->opcode = opcode;
	disp->dispatch = dispatch;
	disp->arg = arg;
	LIST_INSERT_HEAD(&server->dispatch, disp, next);
	return 0;
}

static struct dispatcher *
anoubis_find_dispatcher(struct anoubis_server *server, int opcode)
{
	struct dispatcher	*disp = NULL;
	if ((server->proto & ANOUBIS_PROTO_POLICY) == 0
	    || (server->connect_flags & FLAG_AUTH) == 0)
		return NULL;
	LIST_FOREACH(disp, &server->dispatch, next) {
		if (disp->opcode == opcode)
			break;
	}
	return disp;
}

static int
anoubis_process_dispatcher(struct anoubis_server *server,
    struct dispatcher *disp, struct anoubis_msg * m)
{
	if (!disp)
		return -EINVAL;
	disp->dispatch(server, m, server->auth_uid, disp->arg);
	return 0;
}

/*
 * Process a message from the wire. Caller must release buf!
 * Return values:
 *   - negtiver errno on error.
 *   - zero in case of success.
 */
int anoubis_server_process(struct anoubis_server * server, void * buf,
    size_t len)
{
	anoubis_token_t token;
	int flags;
	struct dispatcher	*disp = NULL;
#ifndef lint
	struct anoubis_msg m = {
		.length = len,
		.u = { .buf = buf }
	};
#else
	struct anoubis_msg m;
#endif
	int opcode;

	/* Return an error if the partner sent data over a dead channel. */
	if (!server->chan
	    || (server->connect_flags & (FLAG_GOTCLOSEACK|FLAG_ERROR)))
		return -EBADF;
	/* Protocol data stream corrupt! */
	if (!VERIFY_LENGTH(&m, 0) || !crc32_check(m.u.buf, m.length))
		return -EIO;
	/* Message too short. */
	if (!VERIFY_FIELD(&m, general, type))
		return -EIO;

	anoubis_dump(&m, "server process");

	opcode = get_value(m.u.general->type);
	token = 0;
	if (ANOUBIS_IS_NOTIFY(opcode)) {
		if (!VERIFY_FIELD(&m, token, token))
			return -EIO;
		token = m.u.token->token;
		if ((server->proto & ANOUBIS_PROTO_NOTIFY) == 0)
			return reply_invalid_token(server, token, opcode);
	}

	/* Check if opcode is applicable. */
	switch (opcode) {
	case ANOUBIS_P_REQUEST:
		/* This starts a request if POLICY_FLAG_START is set. */
		if (!VERIFY_FIELD(&m, policyrequest, flags))
			reply_invalid(server, opcode);
		flags = get_value(m.u.policyrequest->flags);
		if ((flags & POLICY_FLAG_START) == 0)
			break;
	case ANOUBIS_C_VERSEL:
	case ANOUBIS_C_AUTH:
	case ANOUBIS_C_AUTHDATA:
	case ANOUBIS_C_OPTREQ:
	case ANOUBIS_C_PROTOSEL:
	case ANOUBIS_N_REGISTER:
	case ANOUBIS_N_UNREGISTER:
	case ANOUBIS_P_VERSION:
		/*
		 * These start new requests. If we either got a closereq
		 * from the other end these are forbidden. If we sent a
		 * closereq they are allowed but we no longer accept them.
		 */
		if ((server->connect_flags & FLAG_GOTCLOSEREQ)
		    || (server->connect_flags & FLAG_SENTCLOSEREQ))
			return reply_invalid_token(server, token, opcode);
		break;
	case ANOUBIS_C_CLOSEREQ:
	case ANOUBIS_C_CLOSEACK:
		/* Always accepted even if they start a request. */
		break;
	case ANOUBIS_REPLY:
	case ANOUBIS_N_DELEGATE:
		/* These belong to ongoing requests. */
		break;
	case ANOUBIS_N_ASK:
	case ANOUBIS_N_NOTIFY:
	case ANOUBIS_N_LOGNOTIFY:
	case ANOUBIS_N_RESYOU:
	case ANOUBIS_N_RESOTHER:
	case ANOUBIS_N_POLICYCHANGE:
	case ANOUBIS_N_STATUSNOTIFY:
	case ANOUBIS_P_REPLY:
	case ANOUBIS_P_VERSIONREPLY:
		/* These are not allowed from the client. */
		return reply_invalid_token(server, token, opcode);
	default:
		/*
		 * Default is deny. Please fit new opcodes into the
		 * categories above.
		 */
		disp = anoubis_find_dispatcher(server, opcode);
		if (!disp)
			return reply_invalid_token(server, token, opcode);
		/*
		 * Dispatched events are ok, as long as the connection
		 * is not about to be closed.
		 */
		if ((server->connect_flags & FLAG_GOTCLOSEREQ)
		    || (server->connect_flags & FLAG_SENTCLOSEREQ))
			return reply_invalid_token(server, token, opcode);
	}

	/* Process message based on its opcode. */
	switch (opcode) {
	/* Single shot requests from the connect protocol */
	case ANOUBIS_C_VERSEL:
		return anoubis_process_versel(server, &m, opcode);
	case ANOUBIS_C_AUTH:
		return anoubis_process_auth(server, &m, opcode);
	case ANOUBIS_C_AUTHDATA:
		return anoubis_process_authdata(server, &m, opcode);
	case ANOUBIS_C_OPTREQ:
		return anoubis_process_optreq(server, &m, opcode);
	case ANOUBIS_C_PROTOSEL:
		return anoubis_process_protosel(server, &m, opcode);
	/* Close request handling. */
	case ANOUBIS_C_CLOSEREQ:
	case ANOUBIS_C_CLOSEACK:
		return anoubis_process_closereq(server, opcode);
	/* Single shot client requests from the notify protocol. */
	case ANOUBIS_N_REGISTER:
	case ANOUBIS_N_UNREGISTER:
		return anoubis_process_nregister(server, &m, opcode, token);
	case ANOUBIS_REPLY:
	case ANOUBIS_N_DELEGATE:
		if (!VERIFY_FIELD(&m, token, token))
			return -EIO;
		return anoubis_process_reply(server, &m, opcode,
		    m.u.token->token);
	case ANOUBIS_P_REQUEST: {
		struct anoubis_msg * tmp;
		if ((server->proto & ANOUBIS_PROTO_POLICY) == 0)
			return reply_invalid_token(server, token, opcode);
		tmp = anoubis_msg_clone(&m);
		if (!tmp)
			return -ENOMEM;
		/* This will ultimately free the copy of the message. */
		return anoubis_policy_comm_process(server->policy, tmp,
		    server->auth_uid, server->chan);
	}
	case ANOUBIS_P_VERSION:
		return anoubis_process_version(server, &m, opcode);
	default:
		return anoubis_process_dispatcher(server, disp, &m);
	}
	/* NOTREACHED */
	return -EINVAL;
}

int anoubis_server_eof(struct anoubis_server * server)
{
	return (server->connect_flags & FLAG_GOTCLOSEACK)
	    && (server->connect_flags & FLAG_SENTCLOSEACK);
}

struct anoubis_notify_group *
anoubis_server_getnotify(struct anoubis_server * server)
{
	return server->notify;
}

struct achat_channel *
anoubis_server_getchannel(struct anoubis_server *server)
{
	return server->chan;
}

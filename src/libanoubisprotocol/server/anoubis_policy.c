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

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <anoubischat.h>
#include <anoubis_msg.h>
#include <anoubis_policy.h>

#ifndef NEEDBSDCOMPAT
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#define	REQUEST_DATAEND		0x001UL
#define REPLY_DATASTART		0x002UL
#define REPLY_DATAEND		0x004UL

struct policy_request {
	u_int64_t token;
	LIST_ENTRY(policy_request) link;
	struct achat_channel * chan;
	u_int32_t uid;
	int flags;
};

struct anoubis_policy_comm {
	anoubis_policy_comm_dispatcher_t dispatch;
	/*@dependent@*/
	void *arg;
	u_int64_t nexttoken;
	LIST_HEAD(, policy_request) req;
};

/*
 * The following code utilizes list functions from BSD queue.h, which
 * cannot be reliably annotated. We therefore exclude the following
 * functions from memory checking.
 */

/*@-memchecks@*/
struct anoubis_policy_comm * anoubis_policy_comm_create(
    anoubis_policy_comm_dispatcher_t dispatch, void *arg)
{
	struct anoubis_policy_comm * ret;

	if (!dispatch)
		return NULL;
	ret = malloc(sizeof(struct anoubis_policy_comm));
	if (!ret)
		return NULL;
	ret->dispatch = dispatch;
	ret->arg = arg;
	ret->nexttoken = 0x10000000;
	LIST_INIT(&ret->req);
	return ret;
}

int anoubis_policy_comm_process(struct anoubis_policy_comm * comm,
    struct anoubis_msg * m, u_int32_t uid, struct achat_channel * chan)
{
	struct policy_request * req;
	size_t datalen;
	int ret, flags;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_PolicyRequestMessage))) {
		anoubis_msg_free(m);
		return -EINVAL;
	}
	datalen = m->length - CSUM_LEN - sizeof(Anoubis_PolicyRequestMessage);
	if (get_value(m->u.policyrequest->type) != ANOUBIS_P_REQUEST) {
		anoubis_msg_free(m);
		return -EINVAL;
	}
	LIST_FOREACH(req, &comm->req, link) {
		if (req->chan == chan)
			break;
	}
	flags = get_value(m->u.policyrequest->flags);
	if (req) {
		if (flags & POLICY_FLAG_START) 
			return -EBUSY;
		if (req->flags & REQUEST_DATAEND)
			return -EINVAL;
	} else {
		req = malloc(sizeof(struct policy_request));
		if (!req)
			return -ENOMEM;
		req->token = ++comm->nexttoken;
		req->uid = uid;
		req->chan = chan;
		req->flags = 0;
		LIST_INSERT_HEAD(&comm->req, req, link);
	}
	ret = comm->dispatch(comm, req->token, uid,
	    m->u.policyrequest->payload, datalen, comm->arg,
	    get_value(m->u.policyrequest->flags));
	if (ret < 0) {
		LIST_REMOVE(req, link);
		free(req);
	}
	if (flags & POLICY_FLAG_END)
		req->flags |= REQUEST_DATAEND;
	anoubis_msg_free(m);
	return ret;
}

int anoubis_policy_comm_answer(struct anoubis_policy_comm * comm,
    u_int64_t token, int error, void * data, int len, int end)
{
	struct policy_request * req;
	struct anoubis_msg * m;
	achat_rc ret;
	int flags = 0;

	LIST_FOREACH(req, &comm->req, link) {
		if (req->token == token)
			break;
	}
	if (!req)
		return -ESRCH;
	if (req->flags & REPLY_DATAEND)
		return -EINVAL;
	if ((req->flags & REPLY_DATASTART) == 0) {
		req->flags |= REPLY_DATASTART;
		flags |= POLICY_FLAG_START;
	}
	if (end) {
		req->flags |= REPLY_DATAEND;
		flags |= POLICY_FLAG_END;
		LIST_REMOVE(req, link);
		/*
		 * chan = NULL will be used to mark a connection as dead. This
		 * is not an error.
		 */
		if (!req->chan) {
			free(req);
			return 0;
		}
	}
	m = anoubis_msg_new(sizeof(Anoubis_PolicyReplyMessage) + len);
	if (!m) {
		if (end)
			free(req);
		return -ENOMEM;
	}
	set_value(m->u.policyreply->type, ANOUBIS_P_REPLY);
	set_value(m->u.policyreply->error, error);
	set_value(m->u.policyreply->flags, flags);
	if (len) {
		memcpy(m->u.policyreply->payload, data, len);
	}
	ret = anoubis_msg_send(req->chan, m);
	anoubis_msg_free(m);
	if (end)
		free(req);
	if (ret != ACHAT_RC_OK)
		return -EPIPE;
	return 0;
}

void anoubis_policy_comm_abort(struct anoubis_policy_comm * comm,
    struct achat_channel * chan)
{
	struct policy_request * req;
	LIST_FOREACH(req, &comm->req, link) {
		if (req->chan == chan)
			req->chan = NULL;
	}
}
/*@=memchecks@*/

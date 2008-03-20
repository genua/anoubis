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
#include <queue.h>

struct policy_request {
	u_int64_t token;
	LIST_ENTRY(policy_request) link;
	struct achat_channel * chan;
	struct anoubis_msg * m;
	u_int32_t uid;
};

struct anoubis_policy_comm {
	anoubis_policy_comm_dispatcher_t dispatch;
	u_int64_t nexttoken;
	LIST_HEAD(, policy_request) req;
};

struct anoubis_policy_comm * anoubis_policy_comm_create(
    anoubis_policy_comm_dispatcher_t dispatch)
{
	struct anoubis_policy_comm * ret;

	if (!dispatch)
		return NULL;
	ret = malloc(sizeof(struct anoubis_policy_comm));
	if (!ret)
		return NULL;
	ret->dispatch = dispatch;
	ret->nexttoken = 0x10000000;
	LIST_INIT(&ret->req);
	return ret;
}

int anoubis_policy_comm_process(struct anoubis_policy_comm * comm,
    struct anoubis_msg * m, u_int32_t uid, struct achat_channel * chan)
{
	struct policy_request * req;
	size_t datalen;
	int ret;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_PolicyRequestMessage)))
		return -EINVAL;
	datalen = m->length - CSUM_LEN - sizeof(Anoubis_PolicyRequestMessage);
	if (get_value(m->u.policyrequest->type) != ANOUBIS_P_REQUEST)
		return -EINVAL;
	LIST_FOREACH(req, &comm->req, link) {
		if (req->chan == chan)
			return -EBUSY;
	}
	req = malloc(sizeof(struct policy_request));
	if (!req)
		return -ENOMEM;
	req->m = m;
	req->token = ++comm->nexttoken;
	req->uid = uid;
	req->chan = chan;
	LIST_INSERT_HEAD(&comm->req, req, link);
	ret = comm->dispatch(comm, req->token, uid,
	    m->u.policyrequest->payload, datalen);
	if (ret < 0) {
		LIST_REMOVE(req, link);
		anoubis_msg_free(req->m);
		free(req);
	}
	return ret;
}

int anoubis_policy_comm_answer(struct anoubis_policy_comm * comm,
    u_int64_t token, int error, void * data, int len)
{
	struct policy_request * req;
	struct anoubis_msg * m;
	achat_rc ret;

	LIST_FOREACH(req, &comm->req, link) {
		if (req->token == token)
			break;
	}
	if (!req)
		return -ESRCH;
	LIST_REMOVE(req, link);
	anoubis_msg_free(req->m);
	/*
	 * chan = NULL will be used to mark a connection as dead. This
	 * is not an error.
	 */
	if (!req->chan) {
		free(req);
		return 0;
	}
	m = anoubis_msg_new(sizeof(Anoubis_PolicyReplyMessage) + len);
	if (!m) {
		free(req);
		return -ENOMEM;
	}
	set_value(m->u.policyreply->type, ANOUBIS_P_REPLY);
	set_value(m->u.policyreply->error, error);
	if (len) {
		memcpy(m->u.policyreply->payload, data, len);
	}
	ret = anoubis_msg_send(req->chan, m);
	anoubis_msg_free(m);
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

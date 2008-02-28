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
#include <sys/types.h>
#include <anoubischat.h>
#include <anoubis_msg.h>
#include <anoubis_notify.h>
#include <queue.h>

struct anoubis_notify_reg {
	LIST_ENTRY(anoubis_notify_reg) next;
	task_cookie_t task;
	u_int32_t ruleid;
	uid_t uid;
	u_int32_t subsystem;
};

#define FLAG_VERDICT	1
#define FLAG_REPLY	2
#define DROPMASK	(FLAG_VERDICT|FLAG_REPLY)


struct anoubis_notify_event {
	LIST_ENTRY(anoubis_notify_event) next;
	anoubis_token_t token;
	anoubis_notify_callback_t finish_callback;
	void * cbdata;
	unsigned int flags;
};

struct anoubis_notify_group {
	uid_t uid;	/* Authorized User-ID. */
	LIST_HEAD(, anoubis_notify_reg) regs;
	struct achat_channel * chan;
	LIST_HEAD(, anoubis_notify_event) pending;
	LIST_HEAD(, anoubis_notify_event) answered;
};

struct anoubis_notify_group * anoubis_notify_create(struct achat_channel * chan,
    uid_t uid)
{
	struct anoubis_notify_group * ret = malloc(sizeof(*ret));
	ret->chan = chan;
	ret->uid = uid;
	LIST_INIT(&ret->regs);
	LIST_INIT(&ret->pending);
	return ret;
}

void anoubis_notify_destroy(struct anoubis_notify_group * ng)
{
	struct anoubis_notify_reg * reg;
	while(!LIST_EMPTY(&ng->regs)) {
		reg = LIST_FIRST(&ng->regs);
		LIST_REMOVE(reg, next);
		free(reg);
	}
	free(ng);
}

int anoubis_notify_register(struct anoubis_notify_group * ng,
    uid_t uid, task_cookie_t task, u_int32_t ruleid, u_int32_t subsystem)
{
	struct anoubis_notify_reg * reg;

	/* XXX Check if the registration is valid!.  -- ceh 02/09 */
	/* XXX Limit maximum number of registrations!.  -- ceh 02/09 */
	reg = malloc(sizeof(struct anoubis_notify_reg));
	if (!reg)
		return -ENOMEM;
	reg->uid = uid;
	reg->task = task;
	reg->ruleid = ruleid;
	reg->subsystem = subsystem;
	LIST_INSERT_HEAD(&ng->regs, reg, next);
	return 0;
}

static int reg_match(struct anoubis_notify_reg * reg,
    uid_t uid, task_cookie_t task, u_int32_t ruleid, u_int32_t subsystem)
{
	return ((!reg->uid || reg->uid == uid)
	    && (!reg->task || reg->task == task)
	    && (!reg->ruleid || reg->ruleid == ruleid)
	    && (!reg->subsystem || reg->subsystem == subsystem));
}

static int reg_match_all(struct anoubis_notify_group * ng,
    uid_t uid, task_cookie_t task, u_int32_t ruleid, u_int32_t subsystem)
{
	struct anoubis_notify_reg * reg;
	LIST_FOREACH(reg, &ng->regs, next) {
		 if (reg_match(reg, uid, task, ruleid, subsystem))
			return 1;
	}
	return 0;
}


int anoubis_notify_unregister(struct anoubis_notify_group * ng,
    uid_t uid, task_cookie_t task, u_int32_t ruleid, u_int32_t subsystem)
{
	struct anoubis_notify_reg * reg;
	LIST_FOREACH(reg, &ng->regs, next) {
		 if (reg_match(reg, uid, task, ruleid, subsystem)) {
			LIST_REMOVE(reg, next);
			free(reg);
			return 0;
		 }
	}
	return -ESRCH;
}

/*
 * Returns:
 *  - negative errno on error.
 *  - zero if session is not registered for the message
 *  - positive if notify message was sent.
 */

int anoubis_notify(struct anoubis_notify_group * ng, task_cookie_t task,
    struct anoubis_msg * m, anoubis_notify_callback_t finish, void * cbdata)
{
	struct anoubis_notify_event * nev;
	int ret;
	int opcode;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_NotifyMessage)))
		return -EINVAL;
	opcode = get_value(m->u.general->type);
	if (opcode != ANOUBIS_N_ASK && opcode != ANOUBIS_N_NOTIFY)
		return -EINVAL;
	uid_t uid = get_value(m->u.notify->uid);
	pid_t pid = get_value(m->u.notify->pid);
	u_int32_t ruleid = get_value(m->u.notify->rule_id);
	u_int32_t subsystem = get_value(m->u.notify->subsystem);
	anoubis_token_t token = m->u.notify->token;

	if (token == 0)
		return -EINVAL;
	if (pid != /* Convert task id to pid. */ task)
		return -EINVAL;
	if (!reg_match_all(ng, uid, task, ruleid, subsystem))
		return 0;
	if (opcode == ANOUBIS_N_NOTIFY) {
		ret = anoubis_msg_send(ng->chan, m);
		if (ret < 0)
			return ret;
		return 1;
	}
	LIST_FOREACH(nev, &ng->pending, next) {
		if (nev->token == token)
			return -EEXIST;
	}
	nev = malloc(sizeof(struct anoubis_notify_event));
	if (!nev)
		return -ENOMEM;
	nev->token = token;
	nev->finish_callback = finish;
	nev->cbdata = cbdata;
	nev->flags = 0;
	LIST_INSERT_HEAD(&ng->pending, nev, next);
	ret = anoubis_msg_send(ng->chan, m);
	if (ret < 0) {
		LIST_REMOVE(nev, next);
		free(nev);
		return ret;
	}
	return 1;
}

static void drop_event(struct anoubis_notify_group * ng __attribute__((unused)),
    struct anoubis_notify_event * nev, unsigned int flag)
{
	nev->flags |= flag;
	if ((flag & DROPMASK) == DROPMASK) {
		LIST_REMOVE(nev, next);
		free(nev);
	}
}

/*
 * This function processes the answer received over the wire.
 * It does NOT send answers to the remote end because it is not clear
 * if the remote end should receive ANOUBIS_N_RESYOU or ANOUBIS_N_RESOTHER.
 * Instead the caller must explicitly generate answers by calling
 * anoubis_notify_end.
 */
int anoubis_notify_answer(struct anoubis_notify_group * ng,
    anoubis_token_t token, int verdict, int delegate)
{
	struct anoubis_notify_event * nev;
	anoubis_notify_callback_t finish;
	void * data;

	LIST_FOREACH(nev, &ng->pending, next) {
		if (nev->token == token)
			break;
	}
	/*
	 * IF we got no mesage with this token if we already received an
	 * answer for this token from this channel, this is no longer
	 * allowed.
	 */
	if (!nev || nev->flags & FLAG_VERDICT)
		return -ESRCH;
	/* Save the callback info, because drop might free @nev. */
	finish = nev->finish_callback;
	data = nev->cbdata;
	drop_event(ng, nev, FLAG_VERDICT);
	/*
	 * If FLAG_REPLY is set, this already went through @anoubis_notify_end.
	 * In this case there is no need to call the callback. The callback
	 * function must still be prepared to receive these callback for
	 * events that are in the process of being answered!
	 */
	if (nev->flags & FLAG_REPLY)
		finish(data, verdict, delegate);
	return 0;
}

/*
 * This function must be called in reaction to the event callback function
 * exactly once for each anoubis_notify_group the sent the event to a
 * client, i.e it returned a positive value from anoubis_notify.
 */
int anoubis_notify_end(struct anoubis_notify_group * ng,
    anoubis_token_t token, int verdict, uid_t uid, int you)
{
	struct anoubis_msg * m;
	struct anoubis_notify_event * nev;
	int ret;

	LIST_FOREACH(nev, &ng->pending, next) {
		if (nev->token == token)
			break;
	}
	if (!token)
		return -ESRCH;
	m = anoubis_msg_new(sizeof(Anoubis_NotifyResultMessage));
	if (!m)
		return -ENOMEM;
	LIST_REMOVE(nev, next);
	if (you)
		set_value(m->u.notifyresult->type, ANOUBIS_N_RESYOU);
	else
		set_value(m->u.notifyresult->type, ANOUBIS_N_RESOTHER);
	m->u.notifyresult->token = token;
	set_value(m->u.notifyresult->uid, uid);
	set_value(m->u.notifyresult->error, verdict);
	ret = anoubis_msg_send(ng->chan, m);
	anoubis_msg_free(m);
	if (ret < 0) {
		LIST_INSERT_HEAD(&ng->pending, nev, next);
		return ret;
	}
	drop_event(ng, nev, FLAG_REPLY);
	return 0;
}

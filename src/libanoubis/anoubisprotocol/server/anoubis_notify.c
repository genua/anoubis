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
#include <anoubis_chat.h>
#include <anoubis_msg.h>
#include <anoubis_notify.h>
#include <anoubis_errno.h>

#include <sys/queue.h>

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

struct anoubis_notify_reg {
	LIST_ENTRY(anoubis_notify_reg) next;
	u_int32_t ruleid;
	uid_t uid;
	u_int32_t subsystem;
};

struct anoubis_notify_head {
	LIST_HEAD(, anoubis_notify_event) events;
	struct anoubis_msg * m;
	int verdict;
	int eventcount;
	anoubis_notify_callback_t finish;
	void * cbdata;
	void * you;
	uid_t uid;
	int reply;
};

/*
 * FLAG_VERDICT: This peer will not send a verdict (because it
 *    already did so, because it died or because it delegated the event)
 * FLAG_REPLY: A reply has been sent to this peer and we are disconnected
 *    from the head.
 */

#define FLAG_VERDICT	1
#define FLAG_REPLY	2
#define DROPMASK	(FLAG_VERDICT|FLAG_REPLY)

struct anoubis_notify_event {
	LIST_ENTRY(anoubis_notify_event) next;
	LIST_ENTRY(anoubis_notify_event) nextgroup;
	struct anoubis_notify_group * grp;
	struct anoubis_notify_head * head;
	unsigned int flags;
	anoubis_token_t token;
};

struct anoubis_notify_group {
	uid_t uid;	/* Authorized User-ID. */
	LIST_HEAD(, anoubis_notify_reg) regs;
	/*@dependent@*/
	struct achat_channel * chan;
	LIST_HEAD(, anoubis_notify_event) pending;
};

/*
 * The following code utilizes list functions from BSD queue.h, which
 * cannot be reliably annotated. We therefore exclude the following
 * functions from memory checking.
 */
/*@-memchecks@*/
struct anoubis_notify_group * anoubis_notify_create(struct achat_channel * chan,
    uid_t uid)
{
	struct anoubis_notify_group * ret;

	if (NULL == chan)
		return NULL;

	ret = malloc(sizeof(*ret));
	if (!ret)
		return NULL;

	ret->chan = chan;
	ret->uid = uid;
	LIST_INIT(&ret->regs);
	LIST_INIT(&ret->pending);
	return ret;
}

static void drop_event(struct anoubis_notify_event * ev, int flags);

void anoubis_notify_destroy(struct anoubis_notify_group * ng)
{
	struct anoubis_notify_reg * reg;

	while(!LIST_EMPTY(&ng->regs)) {
		reg = LIST_FIRST(&ng->regs);
		LIST_REMOVE(reg, next);
		free(reg);
	}
	while(!LIST_EMPTY(&ng->pending)) {
		struct anoubis_notify_event * nev;
		struct anoubis_notify_head * head;
		nev = LIST_FIRST(&ng->pending);
		head = nev->head;
		drop_event(nev, FLAG_VERDICT|FLAG_REPLY);
		if (!head || head->eventcount)
			continue;
		/*
		 * We just dropped the last event that might have
		 * answered this without getting an answer.
		 */
		anoubis_notify_sendreply(head, head->verdict, head->you,
		    head->uid);
	}
	free(ng);
}

/*
 * Drop pending events and free the head. When this function is called
 * the list should be empty. We still drop the events to be sure.
 */
void anoubis_notify_destroy_head(struct anoubis_notify_head * head)
{
	anoubis_msg_free(head->m);
	while(!LIST_EMPTY(&head->events)) {
		struct anoubis_notify_event * event;
		event = LIST_FIRST(&head->events);
		drop_event(event, FLAG_REPLY);
	}
	free(head);
}

static int anoubis_register_ok(struct anoubis_notify_group * ng,
    uid_t uid, u_int32_t ruleid, u_int32_t subsystem)
{
	/* Registering for stat messages is allowed if all other ids are 0 */
	if (subsystem == ANOUBIS_SOURCE_STAT)
		return (uid == 0 && ruleid == 0);
	/*
	 * Registering for other events is only allowed if the uid and
	 * the authorized uid match or the user is root.
	 * uid -1 is a special case, which means that (unfortunately)
	 * this user can't monitor events.
	 */
	if (ng->uid == 0 /* XXX root */ ||
	   ((ng->uid == uid) && (ng->uid != (uid_t) -1)))
		return 1;
	return 0;
}

int anoubis_notify_register(struct anoubis_notify_group * ng,
    uid_t uid, u_int32_t ruleid, u_int32_t subsystem)
{
	struct anoubis_notify_reg * reg;

	if (!anoubis_register_ok(ng, uid, ruleid, subsystem))
		return -EPERM;
	reg = malloc(sizeof(struct anoubis_notify_reg));
	if (!reg)
		return -ENOMEM;
	reg->uid = uid;
	reg->ruleid = ruleid;
	reg->subsystem = subsystem;
	LIST_INSERT_HEAD(&ng->regs, reg, next);
	return 0;
}

static int reg_match(struct anoubis_notify_reg * reg,
    uid_t uid, u_int32_t ruleid, u_int32_t subsystem)
{
	/*
	 * XXX root can use uid -1 to request all notifications
	 */
	return ((reg->uid == (uid_t) -1 || reg->uid == uid)
	    && (!reg->ruleid || reg->ruleid == ruleid)
	    && (!reg->subsystem || reg->subsystem == subsystem));
}

static int reg_match_all(struct anoubis_notify_group * ng,
    uid_t uid, u_int32_t ruleid, u_int32_t subsystem)
{
	struct anoubis_notify_reg * reg;
	LIST_FOREACH(reg, &ng->regs, next) {
		 if (reg_match(reg, uid, ruleid, subsystem))
			return 1;
	}
	return 0;
}

static void drop_event(struct anoubis_notify_event * ev, int flags)
{
	if ((flags & FLAG_VERDICT) && (ev->flags & FLAG_VERDICT) == 0) {
		ev->flags |= FLAG_VERDICT;
		/*
		 * No need to track the eventcount if a reply has already
		 * been sent. Moreover, ev->head is NULL in this case anyway.
		 */
		if ((ev->flags & FLAG_REPLY) == 0)
			ev->head->eventcount--;
	}
	if ((flags & FLAG_REPLY) && (ev->flags & FLAG_REPLY) == 0) {
		LIST_REMOVE(ev, nextgroup);
		ev->head = NULL;
		ev->flags |= FLAG_REPLY;
	}
	if ((ev->flags & DROPMASK) == DROPMASK) {
		LIST_REMOVE(ev, next);
		free(ev);
	}
}


int anoubis_notify_unregister(struct anoubis_notify_group * ng,
    uid_t uid, u_int32_t ruleid, u_int32_t subsystem)
{
	struct anoubis_notify_reg * reg;
	LIST_FOREACH(reg, &ng->regs, next) {
		 if (reg_match(reg, uid, ruleid, subsystem)) {
			LIST_REMOVE(reg, next);
			free(reg);
			return 0;
		 }
	}
	return -ESRCH;
}

struct anoubis_notify_head *
anoubis_notify_create_head(struct anoubis_msg * m,
    anoubis_notify_callback_t finish, void * cbdata)
{
	struct anoubis_notify_head * head;
	int opcode;

	opcode = get_value(m->u.general->type);
	switch(opcode) {
	case ANOUBIS_N_POLICYCHANGE:
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_PolicyChangeMessage)))
			return NULL;
		break;
	case ANOUBIS_N_STATUSNOTIFY:
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_StatusNotifyMessage)))
			return NULL;
		break;
	case ANOUBIS_N_NOTIFY:
	case ANOUBIS_N_ASK:
	case ANOUBIS_N_LOGNOTIFY:
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_NotifyMessage)))
			return NULL;
		if (opcode != ANOUBIS_N_LOGNOTIFY) {
			set_value(m->u.notify->error, 0);
			set_value(m->u.notify->loglevel, 0);
		}
		break;
	default:
		return NULL;
	}
	head = malloc(sizeof(struct anoubis_notify_head));
	if (!head)
		return NULL;
	LIST_INIT(&head->events);
	head->m = m;
	head->verdict = ANOUBIS_E_IO;
	head->eventcount = 0;
	head->finish = finish;
	head->cbdata = cbdata;
	head->reply = 0;
	head->uid = 0;
	head->you = NULL;
	return head;
}

/*
 * Returns:
 *  - negative errno on error.
 *  - zero if session is not registered for the message
 *  - positive if notify message was sent.
 */

int anoubis_notify(struct anoubis_notify_group * ng,
    struct anoubis_notify_head * head)
{
	struct anoubis_notify_event * nev;
	struct anoubis_msg * m = head->m;
	int ret, opcode;
	u_int32_t uid, ruleid, subsystem;
	anoubis_token_t token;

	opcode = get_value(m->u.general->type);
	switch(opcode) {
	case ANOUBIS_N_POLICYCHANGE:
		uid = get_value(m->u.policychange->uid);
		subsystem = ANOUBIS_SOURCE_STAT;
		ruleid = 0;
		token = 0;
		/*
		 * Only root receives policy change messages for other
		 * users. XXX CEH: This check should be somewhere else!
		 */
		if (ng->uid != 0 && ng->uid != uid)
			return 0;
		break;
	case ANOUBIS_N_STATUSNOTIFY:
		subsystem = ANOUBIS_SOURCE_STAT;
		uid = 0;
		ruleid = 0;
		token = 0;
		break;
	default:
		uid = get_value(m->u.notify->uid);
		ruleid = get_value(m->u.notify->rule_id);
		subsystem = get_value(m->u.notify->subsystem);
		token = m->u.notify->token;
		if (token == 0)
			return -EINVAL;
	}

	if (!reg_match_all(ng, uid, ruleid, subsystem))
		return 0;
	LIST_FOREACH(nev, &ng->pending, next) {
		if (nev->token == token)
			return -EEXIST;
	}
	/* In these cases there is no need to wait for a reply */
	if (opcode == ANOUBIS_N_NOTIFY || opcode == ANOUBIS_N_LOGNOTIFY
	    || opcode == ANOUBIS_N_POLICYCHANGE
	    || opcode == ANOUBIS_N_STATUSNOTIFY) {
		ret = anoubis_msg_send(ng->chan, m);
		if (ret < 0)
			return ret;
		return 1;
	}
	nev = malloc(sizeof(struct anoubis_notify_event));
	if (!nev)
		return -ENOMEM;
	nev->token = token;
	nev->flags = 0;
	nev->head = head;
	nev->grp = ng;
	ret = anoubis_msg_send(ng->chan, m);
	if (ret < 0) {
		free(nev);
		return ret;
	}
	LIST_INSERT_HEAD(&ng->pending, nev, next);
	LIST_INSERT_HEAD(&head->events, nev, nextgroup);
	head->eventcount++;
	return 1;
}

int anoubis_notify_sendreply(struct anoubis_notify_head * head,
    int verdict, void * you, uid_t uid)
{
	struct anoubis_msg * m;
	anoubis_notify_callback_t finish;
	void * cbdata;

	if (verdict < 0)
		return -EINVAL;
	if (head->reply)
		return 0;
	head->reply = 1;
	m = anoubis_msg_new(sizeof(Anoubis_NotifyResultMessage));
	if (!m)
		return -ENOMEM;
	head->verdict = verdict;
	head->uid = uid;
	set_value(m->u.notifyresult->uid, uid);
	set_value(m->u.notifyresult->error, verdict);
	while(!LIST_EMPTY(&head->events)) {
		int ret;
		struct anoubis_notify_event * nev;
		nev = LIST_FIRST(&head->events);
		if (you == nev)
			set_value(m->u.notifyresult->type, ANOUBIS_N_RESYOU);
		else
			set_value(m->u.notifyresult->type, ANOUBIS_N_RESOTHER);
		m->u.notifyresult->token = nev->token;
		ret = anoubis_msg_send(nev->grp->chan, m);
		if (ret < 0)
			drop_event(nev, FLAG_VERDICT);
		drop_event(nev, FLAG_REPLY);
	}
	anoubis_msg_free(m);

	/* Call the callback function. This might leagally free head! */
	finish = head->finish;
	cbdata = head->cbdata;
	if (finish)
		finish(head, head->verdict, cbdata);
	return 0;
}

int anoubis_notify_answer(struct anoubis_notify_group * ng,
    anoubis_token_t token, int verdict, int delegate)
{
	struct anoubis_notify_event * nev;
	struct anoubis_notify_head * head;

	LIST_FOREACH(nev, &ng->pending, next) {
		if (nev->token == token)
			break;
	}
	/*
	 * If we got no mesage with this token or if we already received an
	 * answer for this token from this channel, this is no longer
	 * allowed.
	 */
	if (!nev || (nev->flags & FLAG_VERDICT))
		return -ESRCH;
	/*
	 * Event has already been answered. This is just the reply from
	 * the peer. In this case we only drop the event. We must not access
	 * nev->head!
	 */
	if (nev->flags & FLAG_REPLY) {
		drop_event(nev, FLAG_VERDICT);
		return 0;
	}
	/*
	 * If this is a delegation we remove the event from its group but
	 * leave it in the head (if it is still there). If dropping the
	 * event removes the last event from head take this as a verict
	 * with -EIO.
	 */
	head = nev->head;
	drop_event(nev, FLAG_VERDICT);
	/*
	 * Return if this is a delegation and answers from other
	 * peers are still outstanding just remember the verdict
	 * and return.
	 */
	head->verdict = verdict;
	head->uid = nev->grp->uid;
	head->you = nev;
	if (delegate) {
		if (head->eventcount)
			return 0;
	}
	/*
	 * Either this is the first reply or the last delegation.
	 * In either case send the replies now. In case of a delegation
	 * use the verdict provided together with the delegation!
	 */
	anoubis_notify_sendreply(head, head->verdict, head->you, head->uid);
	return 0;
}
/*@=memchecks@*/

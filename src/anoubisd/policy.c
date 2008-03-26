/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#include <sys/param.h>
#include <sys/time.h>

#include <err.h>
#include <errno.h>
#ifdef LINUX
#include <grp.h>
#endif
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

/* on glibc 2.6+, event.h uses non C89 types :/ */
#include <event.h>
#include "anoubisd.h"
#include "aqueue.h"
#include "amsg.h"

static void	policy_sighandler(int, short, void *);
static anoubisd_msg_t *reply_factory(void);
static void	dispatch_timer(int, short, void *);
static void	dispatch_m2p(int, short, void *);
static void	dispatch_p2m(int, short, void *);
static void	dispatch_s2p(int, short, void *);
static void	dispatch_p2s(int, short, void *);

static Queue	eventq_p2m;
static Queue	eventq_p2s;
static Queue	eventq_s2p;

/*
 * Keep track of messages which have been forwarded to the
 * session process. They will be timed out after a configured
 * timeout and a configured default reply will be generated.
 */
struct wait_reply {
	time_t time;
	struct eventdev_hdr	*msg;
};

static Queue	replyq;

struct event_info_policy {
	struct event	*ev_p2m, *ev_p2s, *ev_s2p;
	struct event	*ev_timer;
	struct timeval	*tv;
};

/* ARGSUSED */
static void
policy_sighandler(int sig, short event, void *arg)
{
	switch (sig) {
	case SIGTERM:
	case SIGINT:
	case SIGQUIT:
		(void)event_loopexit(NULL);
		/* FALLTRHOUGH */
	}
}

pid_t
policy_main(struct anoubisd_config *conf, int pipe_m2s[2], int pipe_m2p[2],
    int pipe_s2p[2])
{
	struct event	 ev_sigterm, ev_sigint, ev_sigquit;
	struct event	 ev_m2p, ev_s2p;
	struct event	 ev_p2m, ev_p2s;
	struct event	 ev_timer;
	struct timeval	 tv;
	struct event_info_policy ev_info;
	struct passwd	*pw;
	sigset_t	 mask;
	pid_t		 pid;

	switch (pid = fork()) {
	case -1:
		fatal("fork");
		/* NOTREACHED */
	case 0:
		break;
	default:
		return (pid);
	}

	anoubisd_process = PROC_POLICY;

	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		fatal("getpwnam");

	if (chroot(pw->pw_dir) == -1)
		fatal("chroot");
	if (chdir("/") == -1)
		fatal("chdir");

#ifdef OPENBSD
	setproctitle("policy engine");
#endif

	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		fatal("can't drop privileges");

	/* From now on, this is an unprivileged child process. */

	log_info("policy started");

	(void)event_init();

	/* We catch or block signals rather than ignoring them. */
	signal_set(&ev_sigterm, SIGTERM, policy_sighandler, NULL);
	signal_set(&ev_sigint, SIGINT, policy_sighandler, NULL);
	signal_set(&ev_sigquit, SIGQUIT, policy_sighandler, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);

	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGQUIT);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	close(pipe_m2p[0]);
	close(pipe_s2p[0]);
	close(pipe_m2s[0]);
	close(pipe_m2s[1]);

	queue_init(eventq_p2m);
	queue_init(eventq_p2s);
	queue_init(eventq_s2p);

	queue_init(replyq);

	/* init msg_bufs - keep track of outgoing ev_info */
	msg_init(pipe_m2p[1], &ev_m2p, "m2p");
	msg_init(pipe_s2p[1], &ev_s2p, "s2p");

	/* master process */
	event_set(&ev_m2p, pipe_m2p[1], EV_READ | EV_PERSIST, dispatch_m2p,
	    &ev_info);
	event_add(&ev_m2p, NULL);

	event_set(&ev_p2m, pipe_m2p[1], EV_WRITE, dispatch_p2m,
	    &ev_info);

	/* session process */
	event_set(&ev_s2p, pipe_s2p[1], EV_READ | EV_PERSIST, dispatch_s2p,
	    &ev_info);
	event_add(&ev_s2p, NULL);

	event_set(&ev_p2s, pipe_s2p[1], EV_WRITE, dispatch_p2s,
	    &ev_info);

	ev_info.ev_p2m = &ev_p2m;
	ev_info.ev_p2s = &ev_p2s;
	ev_info.ev_p2s = &ev_s2p;

	/* Five second timer for statistics ioctl */
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	ev_info.ev_timer = &ev_timer;
	ev_info.tv = &tv;
	evtimer_set(&ev_timer, &dispatch_timer, &ev_info);
	event_add(&ev_timer, &tv);

	DEBUG(DBG_TRACE, "policy event loop");
	if (event_dispatch() == -1)
		fatal("policy_main: event_dispatch");

	_exit(0);
}

static anoubisd_msg_t *
reply_factory(void)
{
	anoubisd_msg_t *msg_reply;

	if ((msg_reply = malloc(sizeof(anoubisd_msg_t) +
	    sizeof(struct eventdev_reply))) == NULL) {
		log_warn("reply_factory: cannot allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}
	bzero(msg_reply, sizeof(anoubisd_msg_t) +
	    sizeof(struct eventdev_reply));

	msg_reply->size = sizeof(anoubisd_msg_t) +
	    sizeof(struct eventdev_reply);
	msg_reply->mtype = ANOUBISD_MSG_EVENTREPLY;
	return msg_reply;
}

int
token_cmp(const void *msg1, const void *msg2)
{
	if (msg1 == NULL || msg2 == NULL) {
		DEBUG(DBG_TRACE, "token_cmp: null msg pointer");
		return 0;
	}
	if (((anoubisd_msg_t *)msg1)->mtype != ANOUBISD_MSG_EVENTDEV ||
	    ((anoubisd_msg_t *)msg2)->mtype != ANOUBISD_MSG_EVENTDEV) {
		DEBUG(DBG_TRACE, "token_cmp: bad mtype");
		return 0;
	}
	if (((struct eventdev_hdr *)((anoubisd_msg_t *)msg1)->msg)->msg_token ==
	    ((struct eventdev_hdr *)((anoubisd_msg_t *)msg2)->msg)->msg_token)
		return 1;
	return 0;
}

static void
dispatch_timer(int sig, short event, void *arg)
{
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;
	anoubisd_msg_t *msg, *msg_reply;
	Qentryp qep_cur, qep_next;
	struct eventdev_hdr *hdr;
	struct eventdev_reply *rep;
	time_t now = time(NULL);

	DEBUG(DBG_TRACE, ">dispatch_timer");

	if ((msg = queue_peek(&replyq)) == NULL) {
		event_add(ev_info->ev_timer, ev_info->tv);
		DEBUG(DBG_TRACE, "<dispatch_timer (no msg)");
		return;
	}

	qep_cur = queue_head(&replyq);
	do {
		qep_next = queue_walk(&replyq, qep_cur);

		/* check for timed out requests */
		msg = qep_cur->entry;
		if (now > msg->starttime + msg->timeout) {

			/* generate a neg reply and remove from the list */
			hdr = (struct eventdev_hdr *)msg->msg;
			msg_reply = reply_factory();
			rep = (struct eventdev_reply *)msg_reply->msg;
			rep->msg_token = hdr->msg_token;
			rep->reply = EPERM;

			queue_delete(&replyq, msg);
			DEBUG(DBG_QUEUE, " <replyq: %x", hdr->msg_token);

			enqueue(&eventq_p2m, msg_reply);
			DEBUG(DBG_QUEUE, " >eventq_p2m: %x", rep->msg_token);
			event_add(ev_info->ev_p2m, NULL);

			/* queue a cancel message to session */
			msg->mtype = ANOUBISD_MSG_EVENTCANCEL;
			enqueue(&eventq_p2s, msg);
			DEBUG(DBG_QUEUE, " >eventq_p2s: %x", rep->msg_token);
			event_add(ev_info->ev_p2s, NULL);
		}

	} while ((qep_cur = qep_next));

	event_add(ev_info->ev_timer, ev_info->tv);

	DEBUG(DBG_TRACE, "<dispatch_timer");
}

static void
dispatch_m2p(int fd, short sig, void *arg)
{
	anoubisd_msg_t *msg, *msg2, *msg_reply;
	anoubisd_reply_t reply;
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;
	struct eventdev_hdr *hdr;
	struct eventdev_reply *rep;

	DEBUG(DBG_TRACE, ">dispatch_m2p");

	if ((msg = get_msg(fd)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_m2p (no msg)");
		return;
	}

	/* this should be a eventdev_hdr message, needing a reply */
	if (msg->mtype != ANOUBISD_MSG_EVENTDEV) {
		free(msg);
		DEBUG(DBG_TRACE, "<dispatch_m2p (bad type %d)", msg->mtype);
		return;
	}
	hdr = (struct eventdev_hdr *)msg->msg;
	if ((hdr->msg_flags & EVENTDEV_NEED_REPLY) == 0) {
		free(msg);
		DEBUG(DBG_TRACE, "<dispatch_m2p (not NEED_REPLY)");
		return;
	}

	DEBUG(DBG_QUEUE, " >m2p: %x", hdr->msg_token);

	/* ask the policy_engine what to do */
	policy_engine(hdr, &reply);

	if (reply.ask) {

		time(&msg->starttime);
		msg->timeout = reply.timeout;

		/* save on the expected reply queue, for timeouts */
		if ((msg2 = malloc(msg->size)) == NULL) {
			free(msg);
			log_warn("dispatch_m2p: cannot allocate memory");
			master_terminate(ENOMEM);
			return;
		}
		bcopy(msg, msg2, msg->size);
		enqueue(&replyq, msg2);
		DEBUG(DBG_QUEUE, " >replyq: %x", hdr->msg_token);

		/* send it to the session */
		enqueue(&eventq_p2s, msg);
		DEBUG(DBG_QUEUE, " >eventq_p2s: %x", hdr->msg_token);
		event_add(ev_info->ev_p2s, NULL);

	} else {

		msg_reply = reply_factory();
		rep = (struct eventdev_reply *)msg_reply->msg;
		rep->msg_token = hdr->msg_token;
		rep->reply = reply.reply;

		free(msg);

		enqueue(&eventq_p2m, msg_reply);
		DEBUG(DBG_QUEUE, " >eventq_p2m: %x", hdr->msg_token);
		event_add(ev_info->ev_p2m, NULL);
	}

	DEBUG(DBG_TRACE, "<dispatch_m2p");
}

static void
dispatch_p2m(int fd, short sig, void *arg)
{
	anoubisd_msg_t *msg;
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;

	DEBUG(DBG_TRACE, ">dispatch_p2m");
	if ((msg = queue_peek(&eventq_p2m)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_p2m (no msg)");
		return;
	}

	if (send_msg(fd, msg)) {
		msg = dequeue(&eventq_p2m);
		DEBUG(DBG_QUEUE, " <eventq_p2m: %x",
		    ((struct eventdev_reply *)msg->msg)->msg_token);
		free(msg);
	}

	/* If the queue is not empty, we want to be called again */
	if (queue_peek(&eventq_p2m))
		event_add(ev_info->ev_p2m, NULL);
	DEBUG(DBG_TRACE, "<dispatch_p2m");
}

static void
dispatch_s2p(int fd, short sig, void *arg)
{
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;
	char buf[sizeof(anoubisd_msg_t)+sizeof(struct eventdev_hdr)];
	anoubisd_msg_t *msg, *msgto, *msgtmp;
	struct eventdev_hdr *hdr, *hdrtmp;

	DEBUG(DBG_TRACE, ">dispatch_s2p");
	if ((msg = queue_peek(&eventq_s2p)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_s2p (no msg)");
		return;
	}

	switch (msg->mtype) {

	case ANOUBISD_MSG_SESSION_REG:

/* XXX RD - handle session registration */
		msg = dequeue(&eventq_s2p);
		DEBUG(DBG_QUEUE, " <eventq_s2p");
		free(msg);
		break;

	case ANOUBISD_MSG_EVENTREPLY:

		hdr = (struct eventdev_hdr *)msg->msg;

		/* create temporary message, for searching */
		bzero(buf, sizeof(buf));
		msgtmp = (anoubisd_msg_t *)buf;
		msgtmp->mtype = ANOUBISD_MSG_EVENTDEV;
		hdrtmp = (struct eventdev_hdr *)msgtmp->msg;
		hdrtmp->msg_token = hdr->msg_token;

		if ((msgto = queue_find(&replyq, msgtmp, token_cmp)) != NULL) {

			if (send_msg(fd, msg)) {

				queue_delete(&replyq, msgto);
				DEBUG(DBG_QUEUE, " <replyq: %x",
				    ((struct eventdev_hdr *)msg->msg)->
					msg_token);

				msg = dequeue(&eventq_s2p);
				DEBUG(DBG_QUEUE, " <eventq_s2p: %x",
				    ((struct eventdev_reply *)msg->msg)->
					msg_token);
				free(msg);
			}
		} else {
			msg = dequeue(&eventq_s2p);
			DEBUG(DBG_QUEUE, " <eventq_s2p: %x",
			    ((struct eventdev_hdr *)msg->msg)->msg_token);
			free(msg);
		}
		break;

	default:

		DEBUG(DBG_TRACE, "dispatch_s2p: msg type %d", msg->mtype);
		msg = dequeue(&eventq_s2p);
		DEBUG(DBG_QUEUE, " <eventq_s2p: ?");
		free(msg);
		break;
	}

	/* If the queue is not empty, we want to be called again */
	if (queue_peek(&eventq_s2p))
		event_add(ev_info->ev_s2p, NULL);

	DEBUG(DBG_TRACE, "<dispatch_s2p");
}

static void
dispatch_p2s(int fd, short sig, void *arg)
{
	anoubisd_msg_t *msg;
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;

	DEBUG(DBG_TRACE, ">dispatch_p2s");

	if ((msg = queue_peek(&eventq_p2s)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_p2s (no msg)");
		return;
	}

	if (send_msg(fd, msg)) {
		msg = dequeue(&eventq_p2s);
		DEBUG(DBG_QUEUE, " <eventq_p2s: %x",
		    ((struct eventdev_hdr *)msg->msg)->msg_token);
		free(msg);
	}

	/* If the queue is not empty, we want to be called again */
	if (queue_peek(&eventq_p2s))
		event_add(ev_info->ev_p2s, NULL);
	DEBUG(DBG_TRACE, "<dispatch_p2s");
}

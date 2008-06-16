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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <err.h>
#include <errno.h>
#ifdef LINUX
#include <grp.h>
#include <openssl/sha.h>
#endif
#ifdef OPENBSD
#include <sha2.h>
#endif
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* on glibc 2.6+, event.h uses non C89 types :/ */
#include <event.h>
#include "anoubisd.h"
#include "aqueue.h"
#include "amsg.h"

#include <anoubis_protocol.h>

static void	policy_sighandler(int, short, void *);
static void	dispatch_timer(int, short, void *);
static void	dispatch_m2p(int, short, void *);
static void	dispatch_p2m(int, short, void *);
static void	dispatch_s2p(int, short, void *);
static void	dispatch_p2s(int, short, void *);

static Queue	eventq_p2m;
static Queue	eventq_p2s;

/*
 * Keep track of messages which have been forwarded to the
 * session process. They will be timed out after a configured
 * timeout and a configured default reply will be generated.
 */
struct reply_wait {
	eventdev_token	token;
	time_t	starttime;
	time_t	timeout;
};
static Queue	replyq;

struct event_info_policy {
	/*@dependent@*/
	struct event	*ev_p2m, *ev_p2s, *ev_s2p;
	/*@dependent@*/
	struct event	*ev_timer;
	/*@null@*/ /*@temp@*/
	struct timeval	*tv;
};

/* ARGSUSED */
static void
policy_sighandler(int sig, short event, void *arg)
{
	switch (sig) {
	case SIGUSR1:
		pe_dump();
		break;
	case SIGHUP:
		pe_reconfigure();
		break;
	case SIGTERM:
	case SIGINT:
	case SIGQUIT:
		(void)event_loopexit(NULL);
		/* FALLTRHOUGH */
	}
}

pid_t
policy_main(struct anoubisd_config *conf, int pipe_m2s[2], int pipe_m2p[2],
    int pipe_s2p[2], int loggers[3])
/*@globals undef eventq_p2m, undef eventq_p2s, undef replyq@*/
{
	struct event	 ev_sigterm, ev_sigint, ev_sigquit, ev_sigusr1,
			     ev_sighup;
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
		/*NOTREACHED*/
	case 0:
		break;
	default:
		/*
		 * eventq_p2m and eventq_p2s do not need to
		 * be defined in the child process.
		 */
		/*@i@*/return (pid);
	}

	anoubisd_process = PROC_POLICY;

	(void)event_init();

	log_init(loggers[1]);
	close(loggers[0]);
	close(loggers[2]);

	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		fatal("getpwnam");

	if (chroot(ANOUBISD_POLICYDIR) == -1)
		fatal("chroot");
	if (chdir(ANOUBISD_POLICYCHROOT) == -1)
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

	/* We catch or block signals rather than ignoring them. */
	signal_set(&ev_sigterm, SIGTERM, policy_sighandler, NULL);
	signal_set(&ev_sigint, SIGINT, policy_sighandler, NULL);
	signal_set(&ev_sigquit, SIGQUIT, policy_sighandler, NULL);
	signal_set(&ev_sigusr1, SIGUSR1, policy_sighandler, NULL);
	signal_set(&ev_sighup, SIGHUP, policy_sighandler, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);
	signal_add(&ev_sigusr1, NULL);
	signal_add(&ev_sighup, NULL);

	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGQUIT);
	sigdelset(&mask, SIGUSR1);
	sigdelset(&mask, SIGHUP);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	close(pipe_m2p[0]);
	close(pipe_s2p[0]);
	close(pipe_m2s[0]);
	close(pipe_m2s[1]);

	queue_init(eventq_p2m);
	queue_init(eventq_p2s);

	queue_init(replyq);

	/* init msg_bufs - keep track of outgoing ev_info */
	msg_init(pipe_m2p[1], "m2p");
	msg_init(pipe_s2p[1], "s2p");

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
	ev_info.ev_s2p = &ev_s2p;

	/* Five second timer for statistics ioctl */
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	ev_info.ev_timer = &ev_timer;
	ev_info.tv = &tv;
	evtimer_set(&ev_timer, &dispatch_timer, &ev_info);
	event_add(&ev_timer, &tv);

	/* Start policy engine */
	pe_init();

	DEBUG(DBG_TRACE, "policy event loop");
	if (event_dispatch() == -1)
		fatal("policy_main: event_dispatch");

	/* Shutdown policy engine */
	pe_shutdown();
	_exit(0);
}

static void
dispatch_timer(int sig, short event, void *arg)
{
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;
	struct reply_wait *msg_wait;
	anoubisd_msg_t *msg;
	eventdev_token *tk;
	/*@dependent@*/
	Qentryp qep_cur;
	/*@dependent@*/
	Qentryp qep_next;
	struct eventdev_reply *rep;
	time_t now = time(NULL);

	DEBUG(DBG_TRACE, ">dispatch_timer");

	if ((msg_wait = queue_peek(&replyq)) == NULL) {
		event_add(ev_info->ev_timer, ev_info->tv);
		DEBUG(DBG_TRACE, "<dispatch_timer");
		return;
	}

	qep_cur = queue_head(&replyq);
	if (qep_cur == NULL)
		return;

	do {
		qep_next = queue_walk(&replyq, qep_cur);

		msg_wait = qep_cur->entry;
		if (now > (msg_wait->starttime + msg_wait->timeout)) {

			msg = msg_factory(ANOUBISD_MSG_EVENTREPLY,
			    sizeof(struct eventdev_reply));
			rep = (struct eventdev_reply *)msg->msg;
			rep->msg_token = msg_wait->token;
			rep->reply = EPERM;
			enqueue(&eventq_p2m, msg);
			DEBUG(DBG_QUEUE, " >eventq_p2m: %x", rep->msg_token);
			event_add(ev_info->ev_p2m, NULL);

			msg = msg_factory(ANOUBISD_MSG_EVENTCANCEL,
			    sizeof(eventdev_token));
			tk = (eventdev_token *)msg->msg;
			*tk = msg_wait->token;
			enqueue(&eventq_p2s, msg);
			DEBUG(DBG_QUEUE, " >eventq_p2s: %x", msg_wait->token);
			event_add(ev_info->ev_p2s, NULL);

			DEBUG(DBG_QUEUE, " <replyq: %x", msg_wait->token);
			queue_delete(&replyq, msg_wait);
			free(msg_wait);
		}

	} while ((qep_cur = qep_next));

	event_add(ev_info->ev_timer, ev_info->tv);

	DEBUG(DBG_TRACE, "<dispatch_timer");
}

static void
dispatch_m2p(int fd, short sig, void *arg)
{
	struct reply_wait *msg_wait;
	anoubisd_msg_t *msg, *msg_reply;
	anoubisd_reply_t *reply;
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;
	/*@dependent@*/
	struct eventdev_hdr *hdr;
	struct eventdev_reply *rep;
	anoubisd_msg_sfsopen_t *sfsmsg = NULL;

	DEBUG(DBG_TRACE, ">dispatch_m2p");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL) {
			DEBUG(DBG_TRACE, "<dispatch_m2p (no msg)");
			return;
		}
		DEBUG(DBG_QUEUE, " >m2p: %x",
		    ((struct eventdev_hdr *)msg->msg)->msg_token);

		switch(msg->mtype) {
		case ANOUBISD_MSG_EVENTDEV:
			hdr = (struct eventdev_hdr *)msg->msg;
			break;
		case ANOUBISD_MSG_SFSOPEN:
			sfsmsg = (anoubisd_msg_sfsopen_t *)msg->msg;
			hdr = (struct eventdev_hdr *)&sfsmsg->hdr;
			break;
		default:
			DEBUG(DBG_TRACE, "<dispatch_m2p (bad type %d)",
			    msg->mtype);
			free(msg);
			continue;
		}

		DEBUG(DBG_PE, "dispatch_m2p: src=%d pid=%d", hdr->msg_source,
		    hdr->msg_pid);
		if (((hdr->msg_flags & EVENTDEV_NEED_REPLY) == 0) &&
		    (hdr->msg_source != ANOUBIS_SOURCE_PROCESS &&
		    hdr->msg_source != ANOUBIS_SOURCE_SFSEXEC)) {
			free(msg);
			DEBUG(DBG_TRACE, "<dispatch_m2p (not NEED_REPLY)");
			continue;
		}
		reply = policy_engine(msg);
		if (reply == NULL) {
			DEBUG(DBG_TRACE, "<dispatch_m2p (no reply)");
			/*
			 * Just in case messages have been queued. E.g.
			 * via send_notify
			 */
			event_add(ev_info->ev_p2s, NULL);
			continue;
		}

		if (reply->ask) {
			switch(msg->mtype) {
			case ANOUBISD_MSG_SFSOPEN:
				sfsmsg->rule_id = reply->rule_id;
				break;
			case ANOUBISD_MSG_EVENTDEV: {
				anoubisd_msg_t *nmsg;
				anoubisd_msg_eventask_t *eventask;
				int extra = hdr->msg_size;
				int plen = 0;
				int cslen = 0;

				if (reply->path)
					plen = 1+strlen(reply->path);
				if (reply->csum)
					cslen = SHA256_DIGEST_LENGTH;
				nmsg = msg_factory(ANOUBISD_MSG_EVENTASK,
				    sizeof(anoubisd_msg_eventask_t)
				    + extra + plen + cslen);
				eventask =
				    (anoubisd_msg_eventask_t *)nmsg->msg;
				eventask->rule_id = reply->rule_id;
				eventask->evoff = 0;
				eventask->evlen = extra;
				bcopy(hdr, eventask->payload, hdr->msg_size);
				eventask->csumoff = extra;
				eventask->csumlen = cslen;
				bcopy(reply->csum, eventask->payload+extra,
				    cslen);
				eventask->pathoff = extra+cslen;
				eventask->pathlen = plen;
				bcopy(reply->path,
				    eventask->payload+extra+cslen, plen);
				hdr = (struct eventdev_hdr *)eventask->payload;
				free(msg);
				msg = nmsg;
				break;
			}
			}
			if ((msg_wait = malloc(sizeof(struct reply_wait))) ==
			    NULL) {
				free(msg);
				log_warn("dispatch_m2p: can't allocate memory");
				master_terminate(ENOMEM);
				continue;
			}

			msg_wait->token = hdr->msg_token;
			if (time(&msg_wait->starttime) == -1) {
				free(msg);
				log_warn("dispatch_m2p: failed to get time");
				master_terminate(EIO);
				continue;
			}
			msg_wait->timeout = reply->timeout;

			enqueue(&replyq, msg_wait);
			DEBUG(DBG_QUEUE, " >replyq: %x", msg_wait->token);

			/* send msg to the session */
			enqueue(&eventq_p2s, msg);
			DEBUG(DBG_QUEUE, " >eventq_p2s: %x", hdr->msg_token);
			event_add(ev_info->ev_p2s, NULL);

		} else {

			msg_reply = msg_factory(ANOUBISD_MSG_EVENTREPLY,
			    sizeof(struct eventdev_reply));
			rep = (struct eventdev_reply *)msg_reply->msg;
			rep->msg_token = hdr->msg_token;
			rep->reply = reply->reply;

			free(msg);

			enqueue(&eventq_p2m, msg_reply);
			DEBUG(DBG_QUEUE, " >eventq_p2m: %x", hdr->msg_token);
			event_add(ev_info->ev_p2m, NULL);
		}

		free(reply);

		DEBUG(DBG_TRACE, "<dispatch_m2p (loop)");
	}
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

	/* msg was checked for non-nullness just above */
	/*@-nullderef@*/ /*@-nullpass@*/
	if (send_msg(fd, msg)) {
		msg = dequeue(&eventq_p2m);
		DEBUG(DBG_QUEUE, " <eventq_p2m: %x",
		    ((struct eventdev_reply *)msg->msg)->msg_token);
		free(msg);
	}
	/*@=nullderef@*/ /*@=nullpass@*/

	/* If the queue is not empty, we want to be called again */
	if (queue_peek(&eventq_p2m) || msg_pending(fd))
		event_add(ev_info->ev_p2m, NULL);

	DEBUG(DBG_TRACE, "<dispatch_p2m");
}

int
token_cmp(void *msg1, void *msg2)
{
	if (msg1 == NULL || msg2 == NULL) {
		DEBUG(DBG_TRACE, "token_cmp: null msg pointer");
		return 0;
	}
	if (((struct reply_wait *)msg1)->token ==
	    ((struct reply_wait *)msg2)->token)
		return 1;
	return 0;
}

void
send_lognotify(struct eventdev_hdr *hdr, u_int32_t error, u_int32_t loglevel,
    u_int32_t rule_id)
{
	int size;
	anoubisd_msg_t * msg;
	struct anoubisd_msg_logrequest * req;

	size = sizeof(anoubisd_msg_t)
	    + sizeof(struct anoubisd_msg_logrequest)
	    + hdr->msg_size
	    - sizeof(struct eventdev_hdr);
	msg = malloc(size);
	if (!msg) {
		log_warn("send_lognotify: can't allocate memory");
		return;
	}
	msg->size = size;
	msg->mtype = ANOUBISD_MSG_LOGREQUEST;
	req = (struct anoubisd_msg_logrequest *)msg->msg;
	req->error = error;
	req->loglevel = loglevel;
	req->rule_id = rule_id;
	bcopy(hdr, &req->hdr, hdr->msg_size);
	enqueue(&eventq_p2s, msg);
}

int send_policy_data(u_int64_t token, int fd)
{
	anoubisd_msg_t * msg;
	/*@observer@*/
	struct anoubisd_reply * comm;
	int flags = POLICY_FLAG_START;
	int size = sizeof(anoubisd_msg_t) + sizeof(struct anoubisd_reply)
	    + 3000;

	while(1) {
		msg = malloc(size);
		if (!msg)
			goto oom;
		msg = msg_factory(ANOUBISD_MSG_POLREPLY, size);
		comm = (struct anoubisd_reply *)msg->msg;
		comm->token = token;
		comm->reply = 0;
		comm->flags = flags;
		comm->len = read(fd, comm->msg, 3000);
		if (comm->len < 0) {
			int ret = -errno;
			free(msg);
			return ret;
		}
		if (comm->len == 0) {
			comm->flags |= POLICY_FLAG_END;
			enqueue(&eventq_p2s, msg);
			break;
		}
		enqueue(&eventq_p2s, msg);
		flags = 0;
	}
	return 0;
oom:
	log_warn("send_policy_data: can't allocate memory");
	master_terminate(ENOMEM);
	return -ENOMEM;
}

static void
dispatch_s2p(int fd, short sig, void *arg)
{
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;
	struct reply_wait rep_tmp, *rep_wait;
	/*@observer@*/
	struct eventdev_reply * evrep;
	anoubisd_msg_t *msg, *msg_rep;
	/*@observer@*/
	anoubisd_reply_t *reply, *rep;

	DEBUG(DBG_TRACE, ">dispatch_s2p");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL) {
			DEBUG(DBG_TRACE, "<dispatch_s2p (no msg)");
			return;
		}

		switch (msg->mtype) {

		case ANOUBISD_MSG_SESSION_REG:

			/* XXX RD - handle session registration */

			DEBUG(DBG_QUEUE, " <eventq_s2p");
			free(msg);
			break;

		case ANOUBISD_MSG_POLREQUEST:
			reply = policy_engine(msg);
			if (!reply) {
				/*
				 * Messages have been queued via
				 * send_policy_data
				 */
				event_add(ev_info->ev_p2s, NULL);
				break;
			}
			msg_rep = msg_factory(ANOUBISD_MSG_POLREPLY,
			    sizeof(anoubisd_reply_t) + reply->len);
			rep = (anoubisd_reply_t *)msg_rep->msg;
			bcopy(reply, rep, sizeof(anoubisd_reply_t)+reply->len);
			free(reply);
			enqueue(&eventq_p2s, msg_rep);
			DEBUG(DBG_QUEUE, " >eventq_p2s: %x", rep->token);
			event_add(ev_info->ev_p2s, NULL);
			free(msg);
			break;

		case ANOUBISD_MSG_EVENTREPLY:

			evrep = (struct eventdev_reply *)msg->msg;
			rep_tmp.token = evrep->msg_token;

			if ((rep_wait = queue_find(&replyq, &rep_tmp,
			    token_cmp)) != 0) {
				/*
				 * Only send message if still in queue. It
				 * might have already been replied to by a
				 * timeout or other GUI
				 */
				queue_delete(&replyq, rep_wait);
				DEBUG(DBG_QUEUE, " <replyq: %x",
				    rep_wait->token);
				enqueue(&eventq_p2m, msg);
				DEBUG(DBG_QUEUE, " >eventq_p2m: %x",
				    ((struct eventdev_reply *)msg->msg)
					->msg_token);
				event_add(ev_info->ev_p2m, NULL);
			} else {
				free(msg);
			}
			break;

		default:

			DEBUG(DBG_TRACE, "dispatch_s2p: msg type %d",
			    msg->mtype);
			free(msg);
			break;
		}
	}
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

	/* msg was checked for non-nullness just above */
	/*@-nullderef@*/ /*@-nullpass@*/
	if (send_msg(fd, msg)) {
		msg = dequeue(&eventq_p2s);
		DEBUG(DBG_QUEUE, " <eventq_p2s: %x",
		    ((struct eventdev_hdr *)msg->msg)->msg_token);
		free(msg);
	}
	/*@=nullderef@*/ /*@=nullpass@*/

	/* If the queue is not empty, we want to be called again */
	if (queue_peek(&eventq_p2s) || msg_pending(fd))
		event_add(ev_info->ev_p2s, NULL);

	DEBUG(DBG_TRACE, "<dispatch_p2s");
}

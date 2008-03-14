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
#include <event.h>
#ifdef LINUX
#include <grp.h>
#endif
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "anoubisd.h"
#include "aqueue.h"
#include "amsg.h"

static void	policy_sighandler(int, short, void *);
static void	dispatch_m2p(int, short, void *);
static void	dispatch_p2m(int, short, void *);
static void	dispatch_s2p(int, short, void *);
static void	dispatch_p2s(int, short, void *);

static Queue eventq_p2m;
static Queue eventq_p2s;

struct event_info_policy {
	struct event	*ev_p2m, *ev_p2s;
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

	if (event_dispatch() == -1)
		fatal("policy_main: event_dispatch");

	_exit(0);
}

static void
dispatch_m2p(int fd, short sig, void *arg)
{
	anoubisd_msg_t *msg;
	anoubisd_msg_t *msg_reply;
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;
	struct eventdev_hdr *hdr;
	struct eventdev_reply *rep;

	DEBUG(DBG_TRACE, ">dispatch_m2p");
	if ((msg = get_msg(fd)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_m2p (no msg)");
		return;
	}

	/* this should be a eventdev_hdr message, needing a reply */
/* XXX RD Check message type */

	hdr = (struct eventdev_hdr *)msg->msg;

	if (hdr->msg_flags & EVENTDEV_NEED_REPLY) {
		if ((msg_reply = malloc(sizeof(anoubisd_msg_t) +
		    sizeof(struct eventdev_reply))) == NULL) {
/* XXX RD probably fatal */
		}
		bzero(msg_reply, sizeof(anoubisd_msg_t) +
		    sizeof(struct eventdev_reply));

		msg_reply->size = sizeof(anoubisd_msg_t) +
		    sizeof(struct eventdev_reply);
		rep = (struct eventdev_reply *)msg_reply->msg;
		rep->msg_token = hdr->msg_token;
		rep->reply = 0;		/* allow */

		enqueue(&eventq_p2m, msg_reply);
		event_add(ev_info->ev_p2m, NULL);
	}

	free(msg);
	DEBUG(DBG_TRACE, "<dispatch_m2p");
}

static void
dispatch_p2m(int fd, short sig, void *arg)
{
	anoubisd_msg_t *msg;
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;

	DEBUG(DBG_TRACE, ">dispatch_p2m");
	if ((msg = queue_next(&eventq_p2m)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_p2m (no msg)");
		return;
	}

	if (send_msg(fd, msg)) {
		msg = dequeue(&eventq_p2m);
		free(msg);
	}

	/* If the queue is not empty, we want to be called again */
	if (queue_next(&eventq_p2m))
		event_add(ev_info->ev_p2m, NULL);
	DEBUG(DBG_TRACE, "<dispatch_p2m");
}

static void
dispatch_s2p(int fd, short sig, void *arg)
{
	DEBUG(DBG_TRACE, ">dispatch_s2p");
	/* XXX HJH: Todo */
	DEBUG(DBG_TRACE, "<dispatch_s2p");
}

static void
dispatch_p2s(int fd, short sig, void *arg)
{
	DEBUG(DBG_TRACE, ">dispatch_p2s");
	/* XXX MG: Todo */
	DEBUG(DBG_TRACE, "<dispatch_p2s");
}

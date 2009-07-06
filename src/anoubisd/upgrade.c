/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <event.h>
#ifdef LINUX
#include <grp.h>
#include <bsdcompat.h>
#endif
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* on glibc 2.6+, event.h uses non C89 types :/ */
#include <event.h>

#include "anoubisd.h"
#include "amsg.h"
#include "aqueue.h"

static void	dispatch_m2u(int, short, void *);
static void	dispatch_u2m(int, short, void *);

static int	terminate = 0;
static Queue    eventq_u2m;

struct event_info_policy {
	/*@dependent@*/
	struct event	*ev_u2m, *ev_m2u;
	struct event	*ev_sigs[10];
};


static void
upgrade_sighandler(int sig, short event __used, void *arg)
{
	int		  i;
	sigset_t	  mask;
	struct event	**sigs;

	switch (sig) {
	case SIGINT:
	case SIGTERM:
		terminate = 1;
		sigfillset(&mask);
		sigprocmask(SIG_SETMASK, &mask, NULL);
		sigs = arg;
		for (i=0; sigs[i]; ++i) {
			signal_del(sigs[i]);
		}
		break;
	case SIGQUIT:
		(void)event_loopexit(NULL);
		/* FALLTRHOUGH */
	}
}

pid_t
upgrade_main(struct anoubisd_config *conf __used, int pipe_m2u[2],
    int pipe_m2p[2], int pipe_s2p[2], int pipe_m2s[2], int loggers[4])
{
	pid_t		pid;
	sigset_t	mask;
	struct event	ev_m2u, ev_u2m;
	struct event	ev_sigterm, ev_sigint, ev_sigquit;
	static struct event	*sigs[10];
	struct event_info_policy ev_info;

	pid = fork();
	if (pid < 0) {
		fatal("fork");
		/*NOTREACHED*/
	}
	if (pid) {
		return (pid);
	}
	anoubisd_process = PROC_UPGRADE;

	(void)event_init();

	log_init(loggers[3]);
	close(loggers[0]);
	close(loggers[1]);
	close(loggers[2]);

	close(pipe_m2u[0]);

	setproctitle("upgrade");
	log_info("upgrade started (pid %d)", getpid());

	signal_set(&ev_sigterm, SIGTERM, upgrade_sighandler, sigs);
	signal_set(&ev_sigint,  SIGINT,  upgrade_sighandler, sigs);
	signal_set(&ev_sigquit, SIGQUIT, upgrade_sighandler, sigs);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint,  NULL);
	signal_add(&ev_sigquit, NULL);
	sigs[0] = &ev_sigterm;
	sigs[1] = &ev_sigint;
	sigs[2] = &ev_sigquit;
	sigs[3] = NULL;

	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGQUIT);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	close(pipe_m2p[0]);
	close(pipe_m2p[1]);
	close(pipe_s2p[0]);
	close(pipe_s2p[1]);
	close(pipe_m2s[0]);
	close(pipe_m2s[1]);

	queue_init(eventq_u2m);
	msg_init(pipe_m2u[1], "m2u");

	/* master process */
	event_set(&ev_m2u, pipe_m2u[1], EV_READ | EV_PERSIST, dispatch_m2u,
	    &ev_info);
	event_add(&ev_m2u, NULL);

	event_set(&ev_u2m, pipe_m2u[1], EV_WRITE, dispatch_u2m,
	    &ev_info);

	ev_info.ev_u2m = &ev_u2m;
	ev_info.ev_m2u = &ev_m2u;

	if (event_dispatch() == -1) {
		fatal("upgrade_main: event_dispatch");
	}

	_exit(0);
}

static void
dispatch_m2u(int fd, short sig __used, void *arg)
{
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;

	DEBUG(DBG_TRACE, ">dispatch_m2u");

	if (msg_eof(fd)) {
		if (terminate < 2)
			terminate = 2;
		event_del(ev_info->ev_m2u);
	}

	DEBUG(DBG_TRACE, "<dispatch_m2u");
}

static void
dispatch_u2m(int fd, short sig __used, void *arg)
{
	struct event_info_policy *ev_info = (struct event_info_policy*)arg;

	DEBUG(DBG_TRACE, ">dispatch_u2m");

	/* If the queue is not empty, we want to be called again */
	if (queue_peek(&eventq_u2m) || msg_pending(fd))
		event_add(ev_info->ev_u2m, NULL);

	DEBUG(DBG_TRACE, "<dispatch_u2m");
}

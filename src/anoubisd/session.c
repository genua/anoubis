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

static void	session_sighandler(int, short, void *);
static void	m2s_dispatch(int, short, void *);
static void	s2m_dispatch(int, short, void *);
static void	s2p_dispatch(int, short, void *);
static void	p2s_dispatch(int, short, void *);

struct event_info_session {
	struct event	*ev_s2m, *ev_s2p;
};

static void
session_sighandler(int sig, short event, void *arg)
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
session_main(struct anoubisd_config *conf, int pipe_m2s[2], int pipe_m2p[2],
    int pipe_s2p[2])
{
	struct event	 ev_sigterm, ev_sigint, ev_sigquit;
	struct event	 ev_m2s, ev_p2s;
	struct event	 ev_s2m, ev_s2p;
	struct event_info_session	ev_info;
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

	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		fatal("getpwnam");

	if (chroot(pw->pw_dir) == -1)
		fatal("chroot");
	if (chdir("/") == -1)
		fatal("chdir");

#ifdef OPENBSD
	setproctitle("session engine");
#endif
	anoubisd_process = PROC_SESSION;

	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		fatal("can't drop privileges");

	/* From now on, this is an unprivileged child process. */

	(void)event_init();

	/* We catch or block signals rather than ignoring them. */
	signal_set(&ev_sigterm, SIGTERM, session_sighandler, NULL);
	signal_set(&ev_sigint, SIGINT, session_sighandler, NULL);
	signal_set(&ev_sigquit, SIGQUIT, session_sighandler, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);

	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGQUIT);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	close(pipe_m2s[0]);
	close(pipe_s2p[1]);
	close(pipe_m2p[0]);
	close(pipe_m2p[1]);

	event_set(&ev_m2s, pipe_m2s[1], EV_READ | EV_PERSIST, m2s_dispatch,
	    &ev_info);
	event_add(&ev_m2s, NULL);

	event_set(&ev_s2m, pipe_m2s[1], EV_WRITE, s2m_dispatch,
	    &ev_info);

	event_set(&ev_p2s, pipe_s2p[1], EV_READ | EV_PERSIST, p2s_dispatch,
	    &ev_info);
	event_add(&ev_p2s, NULL);

	event_set(&ev_s2p, pipe_s2p[1], EV_WRITE, s2p_dispatch,
	    &ev_info);

	ev_info.ev_s2m = &ev_s2m;
	ev_info.ev_s2p = &ev_s2p;

	if (event_dispatch() == -1)
		fatal("session_main: event_dispatch");

	_exit(0);
}

static void
m2s_dispatch(int fd, short sig, void *arg)
{
	/* XXX HJH: Todo */
}

static void
s2m_dispatch(int fd, short sig, void *arg)
{
	/* XXX MG: Todo */
}

static void
s2p_dispatch(int fd, short sig, void *arg)
{
	/* XXX HJH: Todo */
}

static void
p2s_dispatch(int fd, short sig, void *arg)
{
	/* XXX MG: Todo */
}

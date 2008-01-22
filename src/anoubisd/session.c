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

#include <err.h>
#include <errno.h>
#ifdef LINUX
#include <grp.h>
#endif
#include <poll.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "anoubisd.h"

void	session_sighandler(int);

volatile sig_atomic_t	session_quit = 0;

void
session_sighandler(int sig)
{
	switch (sig) {
	case SIGTERM:
	case SIGINT:
	case SIGQUIT:
		session_quit = 1;
		break;
	}
}

#define PFD_PIPE_MASTER	0
#define PFD_PIPE_POLICY	1
#define PFD_MAX		2
#define POLL_TIMEOUT	(3600 * 1000)

pid_t
session_main(struct anoubisd_config *conf, int pipe_m2s[2], int pipe_m2p[2],
    int pipe_s2p[2])
{
	struct passwd	*pw;
	struct sigaction sa;
	struct pollfd	 pfd[PFD_MAX];
	sigset_t	 mask;
	pid_t		 pid;
	int		 nfds;

	switch (pid = fork()) {
	case -1:
		err(1, "fork");
		/* NOTREACHED */
	case 0:
		break;
	default:
		return (pid);
	}

	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		err(1, "getpwnam");

	if (chroot(pw->pw_dir) == -1)
		err(1, "chroot");
	if (chdir("/") == -1)
		err(1, "chdir");

#ifdef OPENBSD
	setproctitle("session engine");
#endif

	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		err(1, "can't drop privileges");

	/* We catch or block signals rather than ignoring them. */
	sigfillset(&mask);

	bzero(&sa, sizeof(sa));
	sa.sa_flags |= SA_RESTART;
	sa.sa_handler = session_sighandler;

	if (sigaction(SIGTERM, &sa, NULL) == -1)
		err(1, "sigaction");
	sigdelset(&mask, SIGTERM);

	if (sigaction(SIGINT, &sa, NULL) == -1)
		err(1, "sigaction");
	sigdelset(&mask, SIGINT);

	if (sigaction(SIGQUIT, &sa, NULL) == -1)
		err(1, "sigaction");
	sigdelset(&mask, SIGQUIT);

	sigprocmask(SIG_SETMASK, &mask, NULL);

	close(pipe_m2s[0]);
	close(pipe_s2p[1]);
	close(pipe_m2p[0]);
	close(pipe_m2p[1]);

	while (session_quit == 0) {
		bzero(pfd, sizeof(pfd));
		pfd[PFD_PIPE_MASTER].fd = pipe_m2s[1];
		pfd[PFD_PIPE_MASTER].events = POLLIN;
		pfd[PFD_PIPE_POLICY].fd = pipe_s2p[0];
		pfd[PFD_PIPE_POLICY].events = POLLIN;

		if ((nfds = poll(pfd, PFD_MAX, POLL_TIMEOUT)) == -1) {
			if (errno != EINTR)
				err(1, "poll");
		}

		if (nfds > 0 && pfd[PFD_PIPE_MASTER].revents & POLLIN)
			/* XXX HSH:  todo */
			;

		if (nfds > 0 && pfd[PFD_PIPE_MASTER].revents & POLLOUT)
			/* XXX HSH:  todo */
			;

		if (nfds > 0 && pfd[PFD_PIPE_POLICY].revents & POLLIN)
			/* XXX HSH:  todo */
			;

		if (nfds > 0 && pfd[PFD_PIPE_POLICY].revents & POLLOUT)
			/* XXX HSH:  todo */
			;
	}

	_exit(0);
}

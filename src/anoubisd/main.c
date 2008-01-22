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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <poll.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#endif

#include "anoubisd.h"
#include "anoubis_alf.h"

void	usage(void) __dead;
void	sighandler(int);
int	check_child(pid_t, const char *);
void	sanitise_stdfd(void);

volatile sig_atomic_t	quit = 0;
volatile sig_atomic_t	sigchld = 0;
volatile sig_atomic_t	reconfig = 0;

__dead void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-dnv]\n", __progname);
	exit(1);
}

#define PFD_PIPE_SESSION	0
#define PFD_PIPE_POLICY		1
#define PFD_DEVICE_ALF		2
#define PFD_MAX			3
#define POLL_TIMEOUT		(3600 * 1000)

void
sighandler(int sig)
{
	switch (sig) {
	case SIGTERM:
	case SIGINT:
	case SIGQUIT:
		quit = 1;
		break;
	case SIGHUP:
		reconfig = 1;
		break;
	case SIGCHLD:
		sigchld = 1;
		break;
	}
}

int
main(int argc, char *argv[])
{
	struct anoubisd_config	conf;
	struct sigaction	sa;
	struct pollfd		pfd[PFD_MAX];
	pid_t			pid;
	pid_t			se_pid = 0;
	pid_t			policy_pid = 0;
	sigset_t		mask;
	int			debug = 0;
	int			ch, nfds;
	int			pipe_m2s[2];
	int			pipe_m2p[2];
	int			pipe_s2p[2];
	int			eventfds[2];

	/* Ensure that fds 0, 1 and 2 are open or directed to /dev/null */
	sanitise_stdfd();

	bzero(&conf, sizeof(conf));

	while ((ch = getopt(argc, argv, "dnv")) != -1) {
		switch (ch) {
		case 'd':
			debug = 1;
			break;
		case 'n':
			conf.opts |= ANOUBISD_OPT_NOACTION;
			break;
		case 'v':
			if (conf.opts & ANOUBISD_OPT_VERBOSE)
				conf.opts |= ANOUBISD_OPT_VERBOSE2;
			conf.opts |= ANOUBISD_OPT_VERBOSE;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}

	/*
	 * XXX HSH: Configfile will be parsed here.
	 */

	if (conf.opts & ANOUBISD_OPT_NOACTION)
		exit(0);

	if (geteuid() != 0)
		errx(1, "need root privileges");

	if (getpwnam(ANOUBISD_USER) == NULL)
		errx(1, "unkown user %s", ANOUBISD_USER);

	openlog("anoubisd", LOG_CONS|LOG_ODELAY, LOG_AUTH);

	if (!debug)
		daemon(1, 0);

	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_m2s) == -1)
		err(1, "socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_m2p) == -1)
		err(1, "socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_s2p) == -1)
		err(1, "socketpair");

	se_pid = session_main(&conf, pipe_m2s, pipe_m2p, pipe_s2p);
	policy_pid = policy_main(&conf, pipe_m2s, pipe_m2p, pipe_s2p);

#ifdef OPENBSD
	setproctitle("master");
#endif

	/* We catch or block signals rather than ignore them. */
	sigfillset(&mask);

	bzero(&sa, sizeof(sa));
	sa.sa_flags |=  SA_RESTART;
	sa.sa_handler = sighandler;

	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		warn("sigaction");
		goto cleanup;
	}
	sigdelset(&mask, SIGTERM);

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		warn("sigaction");
		goto cleanup;
	}
	sigdelset(&mask, SIGINT);

	if (sigaction(SIGQUIT, &sa, NULL) == -1) {
		warn("sigaction");
		goto cleanup;
	}
	sigdelset(&mask, SIGQUIT);

	if (sigaction(SIGHUP, &sa, NULL) == -1) {
		warn("sigaction");
		goto cleanup;
	}
	sigdelset(&mask, SIGHUP);

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		warn("sigaction");
		goto cleanup;
	}
	sigdelset(&mask, SIGCHLD);

	sigprocmask(SIG_SETMASK, &mask, NULL);

	close(pipe_m2s[1]);
	close(pipe_m2p[1]);
	close(pipe_s2p[0]);
	close(pipe_s2p[1]);

	/*
	 * Open eventdevices to communicate with the kernel.
	 */
	eventfds[0] = open("/dev/eventdev", O_RDWR);
	if (eventfds[0] < 0) {
		warn("open(eventdev)");
		goto cleanup;
	}
	eventfds[1] = open("/dev/anoubis", O_RDWR);
	if (eventfds[1] < 0) {
		warn("open(alf/anoubis)");
		goto cleanup;
	}
	if (ioctl(eventfds[1], ANOUBIS_DECLARE_FD, eventfds[0]) < 0) {
		warn("ioctl");
		goto cleanup;
	}
	close(eventfds[1]);

	/*
	 * Daemon mainloop
	 */
	while (quit == 0) {
		bzero(pfd, sizeof(pfd));
		pfd[PFD_PIPE_SESSION].fd = pipe_m2s[0];
		pfd[PFD_PIPE_SESSION].events = POLLIN;
		pfd[PFD_PIPE_POLICY].fd = pipe_m2p[0];
		pfd[PFD_PIPE_POLICY].events = POLLIN;
		pfd[PFD_DEVICE_ALF].fd = eventfds[0];
		pfd[PFD_DEVICE_ALF].events = POLLIN;

		if ((nfds = poll(pfd, PFD_MAX, POLL_TIMEOUT)) == -1) {
			if (errno != EINTR) {
				warn("poll");
				quit = 1;
			}
		}

		if ((nfds > 0) && (pfd[PFD_PIPE_SESSION].revents & POLLOUT))
			/* XXX HSH: todo */
			;

		if ((nfds > 0) && (pfd[PFD_PIPE_SESSION].revents & POLLIN))
			/* XXX HSH: todo */
			;

		if ((nfds > 0) && (pfd[PFD_PIPE_POLICY].revents & POLLOUT))
			/* XXX HSH: todo */
			;

		if ((nfds > 0) && (pfd[PFD_PIPE_POLICY].revents & POLLIN))
			/* XXX HSH: todo */
			;

		if ((nfds > 0) && (pfd[PFD_DEVICE_ALF].revents & POLLIN)) {
			struct alf_event event;
			struct eventdev_reply rep;
			int ret;

			if (read(pfd[PFD_DEVICE_ALF].fd, &event,
			    sizeof(struct alf_event)) < sizeof(event))
				continue;

			rep.reply = 0; /* Allow everything */
			rep.msg_token = event.hdr.msg_token;

			if ((ret = write(pfd[PFD_DEVICE_ALF].fd, &rep,
			    sizeof(rep))) < sizeof(rep)) {
				continue;
			}
		}

		if (reconfig) {
			reconfig = 0;

			/* XXX HSH: todo */
		}

		if (sigchld) {
			sigchld = 0;
			if (check_child(se_pid, "session engine")) {
				quit = 1;
				se_pid = 0;
			}
			if (check_child(policy_pid, "policy engine")) {
				quit = 1;
				policy_pid = 0;
			}
		}
	}

cleanup:
	/*
	 * Ignoring SIGCHLD changes the behaviour of wait(2) et al. in that
	 * way, that a call to wait(2) will block until all of the calling
	 * processes child processes terminate, and then returns a value
	 * of -1 with errno set to ECHILD.  Moreover, an exited child will
	 * not turn into a zombie.  See signal(3) and sigaction(2).
	 */
	bzero(&sa, sizeof(sa));
	sa.sa_flags |= SA_RESTART;
	sa.sa_handler = SIG_IGN;

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		/* Just warn and continue cleaning up the kids. */
		warn("sigaction");
	}

	if (se_pid)
		kill(se_pid, SIGTERM);
	if (policy_pid)
		kill(policy_pid, SIGTERM);

	/* XXX HSH: Do all cleanup here. */
	close(eventfds[0]);
	close(eventfds[1]);

	do {
		if ((pid = wait(NULL)) == -1 &&
		    errno != EINTR && errno != ECHILD)
			err(1, "wait");
	} while (pid != -1 || (pid == -1 && errno == EINTR));

	return (0);
}

int
check_child(pid_t pid, const char *pname)
{
	int	status;

	if (waitpid(pid, &status, WNOHANG) > 0) {
		if (WIFEXITED(status)) {
			warnx("Lost child: %s exited", pname);
			return (1);
		}
		if (WIFSIGNALED(status)) {
			warnx("Lost child: %s terminated; signal %d", pname,
			    WTERMSIG(status));
			return (1);
		}
	}

	return (0);
}

void
sanitise_stdfd(void)
{
	int nullfd, dupfd;

	if ((nullfd = dupfd = open("/dev/null", O_RDWR)) == -1)
		err(1, "open");

	while (++dupfd <= 2) {
		/* Only clobber closed fds */
		if (fcntl(dupfd, F_GETFL, 0) >= 0)
			continue;
		if (dup2(nullfd, dupfd) == -1) {
			err(1, "dup2");
		}
	}
	if (nullfd > 2)
	close(nullfd);
}

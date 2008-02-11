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
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

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

#include <err.h>
#include <errno.h>
#include <event.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "anoubisd.h"
#include "anoubis_alf.h"

static void	usage(void) __dead;
static void	sighandler(int, short, void *);
static void	main_cleanup(void);
static void	main_shutdown(void) __dead;
static int	check_child(pid_t, const char *);
static void	sanitise_stdfd(void);
static void	reconfigure(void);
static void	dispatch_m2s(int, short, void *);
static void	dispatch_m2p(int, short, void *);
static void	dispatch_alf(int, short, void *);

__dead static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-dnv]\n", __progname);
	exit(1);
}

pid_t		se_pid = 0;
pid_t		policy_pid = 0;

/*
 * Note:  Signalhandler managed by libevent are _not_ run in signal
 * context, thus any C-function can be used.
 */
static void
sighandler(int sig, short event, void *arg)
{
	int	die = 0;

	switch (sig) {
	case SIGTERM:
	case SIGINT:
	case SIGQUIT:
		die = 1;
	case SIGCHLD:
		if (check_child(se_pid, "session engine")) {
			se_pid = 0;
			die = 1;
		}
		if (check_child(policy_pid, "policy engine")) {
			policy_pid = 0;
			die = 1;
		}
		if (die) {
			main_shutdown();
			/* NOTREACHED */
		}
		break;
	case SIGHUP:
		reconfigure();
		break;
	}
}

int
main(int argc, char *argv[])
{
	struct event		ev_sigterm, ev_sigint, ev_sigquit, ev_sighup,
				    ev_sigchld;
	struct event		ev_m2s, ev_m2p, ev_alf;
	struct anoubisd_config	conf;
	sigset_t		mask;
	int			debug = 0;
	int			ch;
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

	(void)event_init();

	/* We catch or block signals rather than ignore them. */
	signal_set(&ev_sigterm, SIGTERM, sighandler, NULL);
	signal_set(&ev_sigint, SIGINT, sighandler, NULL);
	signal_set(&ev_sigquit, SIGQUIT, sighandler, NULL);
	signal_set(&ev_sighup, SIGHUP, sighandler, NULL);
	signal_set(&ev_sigchld, SIGCHLD, sighandler, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);
	signal_add(&ev_sighup, NULL);
	signal_add(&ev_sigchld, NULL);

	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGQUIT);
	sigdelset(&mask, SIGHUP);
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
		main_cleanup();
		err(1, "open(/dev/eventdev)");
	}
	eventfds[1] = open("/dev/anoubis", O_RDWR);
	if (eventfds[1] < 0) {
		close(eventfds[0]);
		main_cleanup();
		err(1, "open(/dev/anoubis)");
	}
	if (ioctl(eventfds[1], ANOUBIS_DECLARE_FD, eventfds[0]) < 0) {
		close(eventfds[0]);
		close(eventfds[1]);
		main_cleanup();
		err(1, "ioctl");
	}
	close(eventfds[1]);

	event_set(&ev_m2s, pipe_m2s[0], EV_READ | EV_PERSIST, dispatch_m2s,
	    NULL);
	event_add(&ev_m2s, NULL);
	event_set(&ev_m2p, pipe_m2p[0], EV_READ | EV_PERSIST, dispatch_m2p,
	    NULL);
	event_add(&ev_m2p, NULL);
	event_set(&ev_alf, eventfds[0], EV_READ | EV_PERSIST, dispatch_alf,
	    NULL);
	event_add(&ev_alf, NULL);

	if (event_dispatch() == -1)
		warn("main: event_dispatch");

	main_cleanup();
	exit(0);
}

static void
main_cleanup(void)
{
	struct sigaction	sa;
	pid_t			pid;

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

	do {
		if ((pid = wait(NULL)) == -1 &&
		    errno != EINTR && errno != ECHILD)
			err(1, "wait");
	} while (pid != -1 || (pid == -1 && errno == EINTR));
}

__dead static void
main_shutdown(void)
{
	main_cleanup();
	exit(0);
}

static int
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

static void
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

static void
reconfigure(void)
{
	/* XXX HJH: Todo */
}

static void
dispatch_m2s(int fd, short event, void *arg)
{
	/* XXX HJH: Todo */
}

static void
dispatch_m2p(int fd, short event, void *arg)
{
	/* XXX HJH: Todo */
}

static void
dispatch_alf(int fd, short event, void *arg)
{
	struct alf_event	alf_event;
	struct eventdev_reply	rep;

	if (read(fd, &alf_event, sizeof(alf_event)) < sizeof(alf_event)) {
		warn("short read");
		return;
	}

	rep.reply = 0;	/* Allow everything */
	rep.msg_token = alf_event.hdr.msg_token;

	if (write(fd, &rep, sizeof(rep)) < sizeof(rep)) {
		warn("short write");
		return;
	}

	return;
}

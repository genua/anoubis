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

static void	usage(void) __dead;
static void	sighandler(int, short, void *);
static void	main_cleanup(void);
static void	main_shutdown(int) __dead;
static int	check_child(pid_t, const char *);
static void	sanitise_stdfd(void);
static void	reconfigure(void);
static void	dispatch_m2s(int, short, void *);
static void	dispatch_s2m(int, short, void *);
static void	dispatch_m2p(int, short, void *);
static void	dispatch_p2m(int, short, void *);
static void	dispatch_m2dev(int, short, void *);
static void	dispatch_dev2m(int, short, void *);

__dead static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [ -dnv ] [ -s socket ]\n", __progname);
	exit(1);
}

pid_t		se_pid = 0;
pid_t		policy_pid = 0;

static TAILQ_HEAD(head_m2p, anoubisd_event_in) eventq_m2p =
    TAILQ_HEAD_INITIALIZER(eventq_m2p);
static TAILQ_HEAD(head_m2s, anoubisd_event_in) eventq_m2s =
    TAILQ_HEAD_INITIALIZER(eventq_m2s);
static TAILQ_HEAD(head_m2dev, anoubisd_event_out) eventq_m2dev =
    TAILQ_HEAD_INITIALIZER(eventq_m2dev);

struct event_info_main {
	struct event	*ev_m2s, *ev_m2p, *ev_m2dev;
	struct event	*ev_s2m, *ev_p2m;
};

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
			main_shutdown(0);
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
	struct event		ev_s2m, ev_p2m, ev_dev2m;
	struct event		ev_m2s, ev_m2p, ev_m2dev;
	struct event_info_main	ev_info;
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

	anoubisd_process = PROC_MAIN;
	log_init(1);		/* Log to stderr until we're daemonized. */

	bzero(&conf, sizeof(conf));

	while ((ch = getopt(argc, argv, "dnvs:")) != -1) {
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
		case 's':
			if (conf.unixsocket)
				usage();
			conf.unixsocket = strdup(optarg);
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}

	/* Defaults. */
	if (conf.unixsocket == NULL)
		conf.unixsocket = ANOUBISD_SOCKETNAME;

	/*
	 * XXX HSH: Configfile will be parsed here.
	 */

	if (conf.opts & ANOUBISD_OPT_NOACTION)
		exit(0);

	if (geteuid() != 0)
		errx(1, "need root privileges");

	if (getpwnam(ANOUBISD_USER) == NULL)
		errx(1, "unkown user %s", ANOUBISD_USER);

	log_init(debug);

	if (!debug) {
		if (daemon(1, 0) !=0)
			fatal("daemon");
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_m2s) == -1)
		fatal("socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_m2p) == -1)
		fatal("socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_s2p) == -1)
		fatal("socketpair");

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
		log_warn("open(/dev/eventdev)");
		main_shutdown(1);
	}
	eventfds[1] = open("/dev/anoubis", O_RDWR);
	if (eventfds[1] < 0) {
		log_warn("open(/dev/anoubis)");
		close(eventfds[0]);
		main_shutdown(1);
	}
	if (ioctl(eventfds[1], ANOUBIS_DECLARE_LISTENER, 0) < 0) {
		log_warn("ioctl");
		close(eventfds[0]);
		close(eventfds[1]);
		main_shutdown(1);
	}
	if (ioctl(eventfds[1], ANOUBIS_DECLARE_FD, eventfds[0]) < 0) {
		log_warn("ioctl");
		close(eventfds[0]);
		close(eventfds[1]);
		main_shutdown(1);
	}
	close(eventfds[1]);

	/* session process */
	event_set(&ev_m2s, pipe_m2s[0], EV_WRITE, dispatch_m2s,
	    &ev_info);

	ev_info.ev_s2m = &ev_s2m;
	event_set(&ev_s2m, pipe_m2s[0], EV_READ | EV_PERSIST, dispatch_s2m,
	    &ev_info);
	event_add(&ev_s2m, NULL);

	/* policy process */
	event_set(&ev_m2p, pipe_m2p[0], EV_WRITE, dispatch_m2p,
	    &ev_info);

	ev_info.ev_p2m = &ev_p2m;
	event_set(&ev_p2m, pipe_m2p[0], EV_READ | EV_PERSIST, dispatch_p2m,
	    &ev_info);
	event_add(&ev_p2m, NULL);

	/* event device */
	event_set(&ev_dev2m, eventfds[0], EV_READ | EV_PERSIST, dispatch_dev2m,
	    &ev_info);
	event_add(&ev_dev2m, NULL);

	event_set(&ev_m2dev, eventfds[0], EV_WRITE, dispatch_m2dev,
	    &ev_info);

	ev_info.ev_m2s = &ev_m2s;
	ev_info.ev_m2p = &ev_m2p;
	ev_info.ev_m2dev = &ev_m2dev;

	TAILQ_INIT(&eventq_m2p);
	TAILQ_INIT(&eventq_m2s);
	TAILQ_INIT(&eventq_m2dev);

	if (event_dispatch() == -1)
		log_warn("main: event_dispatch");

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
		log_warn("sigaction");
	}

	if (se_pid)
		kill(se_pid, SIGTERM);
	if (policy_pid)
		kill(policy_pid, SIGTERM);

	/* XXX HSH: Do all cleanup here. */

	do {
		if ((pid = wait(NULL)) == -1 &&
	    errno != EINTR && errno != ECHILD)
		fatal("wait");
	} while (pid != -1 || (pid == -1 && errno == EINTR));
}

__dead static void
main_shutdown(int error)
{
	main_cleanup();
	exit(error);
}

static int
check_child(pid_t pid, const char *pname)
{
	int	status;

	if (waitpid(pid, &status, WNOHANG) > 0) {
		if (WIFEXITED(status)) {
			log_warnx("Lost child: %s exited", pname);
			return (1);
	}
	if (WIFSIGNALED(status)) {
		log_warnx("Lost child: %s terminated; signal %d",
		    pname, WTERMSIG(status));
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
	struct anoubisd_event_in *ev;
	struct event_info_main *ev_info = (struct event_info_main*)arg;
	int len;

	if (TAILQ_EMPTY(&eventq_m2s))
		return;

	ev = TAILQ_FIRST(&eventq_m2s);

	len = write(fd, ev, ev->event_size);

	if (len < 0) {
		log_warn("write error");
		return;
	}

	if (len < ev->event_size) {
		log_warn("short write");
		return;
	}

	TAILQ_REMOVE(&eventq_m2s, ev, events);
	free(ev);

	/* If the queue is not empty, we want to be called again */
	if (!TAILQ_EMPTY(&eventq_m2s))
		event_add(ev_info->ev_m2s, NULL);
}

static void
dispatch_s2m(int fd, short event, void *arg)
{
	struct event_info_main *ev_info = (struct event_info_main*)arg;
	char buf[100];
	int len;

	/*
	 * XXX HJH for now just make sure we can get an EOF and don't
	 * XXX HJH bother about short reads for now.
	 */
	len = read(fd, buf, sizeof(buf));
	if (len == -1) {
		log_warn("read error");
		return;
	}
	if (len == 0) {
		event_del(ev_info->ev_s2m);
		event_loopexit(NULL);
		return;
	}
	
	/* XXX MG: Todo */
}

static void
dispatch_m2p(int fd, short event, void *arg)
{
	struct anoubisd_event_in *ev;
	struct event_info_main *ev_info = (struct event_info_main*)arg;
	int len;

	if (TAILQ_EMPTY(&eventq_m2p))
		return;

	ev = TAILQ_FIRST(&eventq_m2p);

	len = write(fd, ev, ev->event_size);

	if (len < 0) {
		log_warn("write error");
		return;
	}

	if (len < ev->event_size) {
		log_warn("short write");
		return;
	}

	TAILQ_REMOVE(&eventq_m2p, ev, events);
	free(ev);

	/* If the queue is not empty, we want to be called again */
	if (!TAILQ_EMPTY(&eventq_m2p))
		event_add(ev_info->ev_m2p, NULL);
}

static void
dispatch_p2m(int fd, short event, void *arg)
{
	int len;
	struct anoubisd_event_out *ev;
	struct event_info_main *ev_info = (struct event_info_main*)arg;

	if ((ev = malloc(sizeof(struct anoubisd_event_out))) == NULL) {
		log_warn("can't allocate memory");
		return;
	}

	len = read(fd, ev, sizeof(struct anoubisd_event_out));

	if (len < 0) {
		log_warn("read error");
		return;
	}
	if (len == 0) {
		event_del(ev_info->ev_p2m);
		event_loopexit(NULL);
		return;
	}
	if (len < sizeof(struct anoubisd_event_out)) {
		log_warn("short read");
		return;
	}

	TAILQ_INSERT_TAIL(&eventq_m2dev, ev, events);
	event_add(ev_info->ev_m2dev, NULL);
}

static void
dispatch_m2dev(int fd, short event, void *arg)
{
	struct anoubisd_event_out *ev;
	struct event_info_main *ev_info = (struct event_info_main*)arg;

	if (TAILQ_EMPTY(&eventq_m2dev))
		return;

	ev = TAILQ_FIRST(&eventq_m2dev);

	if (write(fd, &(ev->reply), sizeof(ev->reply)) < sizeof(ev->reply)) {
		log_warn("short write");
		return;
	}

	TAILQ_REMOVE(&eventq_m2dev, ev, events);
	free(ev);

	if (!TAILQ_EMPTY(&eventq_m2dev))
		event_add(ev_info->ev_m2dev, NULL);
}

static void
dispatch_dev2m(int fd, short event, void *arg)
{
	struct eventdev_hdr * hdr;
	struct eventdev_reply	rep;
	struct anoubisd_event_in *ev;
	struct event_info_main *ev_info = (struct event_info_main*)arg;

	int len;
	int size;
#define BUFSIZE 8192
	char buf[BUFSIZE];

	len = read(fd, buf, sizeof(buf));
	if (len < 0) {
		log_warn("read error");
		return;
	}
	if (len < sizeof (struct eventdev_hdr)) {
		log_warn("short read");
		return;
	}
	hdr = (struct eventdev_hdr *)buf;

	/* we shortcut and ack events for our own children */
	if ((hdr->msg_flags & EVENTDEV_NEED_REPLY) &&
	    (hdr->msg_pid == se_pid || hdr->msg_pid == policy_pid)) {

		bzero(&rep, sizeof(rep));

		rep.reply = 0;
		rep.msg_token = hdr->msg_token;

		if (write(fd, &rep, sizeof(rep)) < sizeof(rep)) {
			log_warn("short write");
			return;
		}
		return;
	}

	size = sizeof(struct anoubisd_event_in) + hdr->msg_size - sizeof(*hdr);

	if ((ev = malloc(size)) == NULL) {
		log_warn("can't allocate memory");

		if (hdr->msg_flags & EVENTDEV_NEED_REPLY) {
			rep.reply = 1;	/* Fail close */
			rep.msg_token = hdr->msg_token;

			if (write(fd, &rep, sizeof(rep)) < sizeof(rep)) {
				log_warn("short write");
				return;
			}
		}

		return;
	}

	ev->event_size = size;
	ev->hdr = *hdr;
	memcpy(ev->msg, (char *)hdr + sizeof(struct eventdev_hdr),
	    hdr->msg_size - sizeof(*hdr));

	TAILQ_INSERT_TAIL(&eventq_m2p, ev, events);

	/* Wait for fd to get ready */
	event_add(ev_info->ev_m2p, NULL);

	/*
	 * Copy events to the session process for notifications. It is not
	 * fatal if malloc fails here, the notification will just not arrive
	 * at the session process
	 */
	if ((ev = malloc(size)) != NULL) {
		ev->event_size = size;
		ev->hdr = *hdr;
		memcpy(ev->msg, (char *)hdr + sizeof(struct eventdev_hdr),
		    hdr->msg_size - sizeof(*hdr));

		TAILQ_INSERT_TAIL(&eventq_m2s, ev, events);

		event_add(ev_info->ev_m2s, NULL);
	}

	return;
}

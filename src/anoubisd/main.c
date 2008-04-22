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
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
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

/*@noreturn@*/
static void	usage(void) __dead;

static int	eventfds[2];
static void	main_cleanup(void);
/*@noreturn@*/
static void	main_shutdown(int) __dead;
static void	sanitise_stdfd(void);
static void	reconfigure(void);

static void	sighandler(int, short, void *);
static int	check_child(pid_t, const char *);
static void	dispatch_m2s(int, short, void *);
static void	dispatch_s2m(int, short, void *);
static void	dispatch_m2p(int, short, void *);
static void	dispatch_p2m(int, short, void *);
static void	dispatch_m2dev(int, short, void *);
static void	dispatch_dev2m(int, short, void *);

pid_t		master_pid = 0;
pid_t		se_pid = 0;
pid_t		policy_pid = 0;

static Queue eventq_m2p;
static Queue eventq_m2s;
static Queue eventq_m2dev;

struct event_info_main {
	/*@dependent@*/
	struct event	*ev_m2s, *ev_m2p, *ev_m2dev;
	/*@dependent@*/
	struct event	*ev_timer;
	/*@null@*/
	struct timeval	*tv;
	int anoubisfd;
};

static char *pid_file_name = ANOUBISD_PIDFILENAME;

__dead static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-D <flags>] [-dnv] [-s <socket> ]\n",
	    __progname);
	exit(1);
}

/*
 * Note:  Signalhandler managed by libevent are _not_ run in signal
 * context, thus any C-function can be used.
 */
static void
sighandler(int sig, short event, void *arg)
{
	int	die = 0;

	DEBUG(DBG_TRACE, ">sighandler: %d", sig);

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
			/*NOTREACHED*/
		}
		break;
	case SIGHUP:
		reconfigure();
		break;
	}

	DEBUG(DBG_TRACE, "<sighandler: %d", sig);
}

static int
check_pid(void)
{
	FILE *fp;
	pid_t pid;
	int scans;

	fp = fopen(pid_file_name, "r");
	if (fp == NULL)
		return 0;
	scans = fscanf(fp, "%d", &pid);
	fclose(fp);
	if (scans != 1)
		return 0;

	if (pid <= 1)
		return 0;

	if (kill(pid, 0) == 0)
		return 1;

	return 0;
}

static void
save_pid(pid_t pid)
{
	FILE *fp;
	int error = 0;

	fp = fopen(pid_file_name, "w");
	if (fp == NULL) {
		log_warn(pid_file_name);
		error = 1;
	}
	fprintf(fp, "%d\n", pid);
	if (fclose(fp)) {
		log_warn(pid_file_name);
		error = 1;
	}
	if (error)
		fatal("cannot write pid file");
}

static void
dispatch_timer(int sig, short event, /*@dependent@*/ void * arg)
{
	struct event_info_main	*ev_info = arg;

	DEBUG(DBG_TRACE, ">dispatch_timer");

	ioctl(ev_info->anoubisfd, ANOUBIS_REQUEST_STATS, 0);
	event_add(ev_info->ev_timer, ev_info->tv);

	DEBUG(DBG_TRACE, "<dispatch_timer");
}

int
main(int argc, char *argv[])
/*@globals undef eventq_m2p, undef eventq_m2s, undef eventq_m2dev@*/
{
	struct event		ev_sigterm, ev_sigint, ev_sigquit, ev_sighup,
				    ev_sigchld;
	struct event		ev_s2m, ev_p2m, ev_dev2m;
	struct event		ev_m2s, ev_m2p, ev_m2dev;
	struct event		ev_timer;
	/*@observer@*/
	struct event_info_main	ev_info;
	struct anoubisd_config	conf;
	sigset_t		mask;
	int			ch;
	int			pipe_m2s[2];
	int			pipe_m2p[2];
	int			pipe_s2p[2];
	struct timeval		tv;
	char		       *endptr;

	/* Ensure that fds 0, 1 and 2 are open or directed to /dev/null */
	sanitise_stdfd();

	anoubisd_process = PROC_MAIN;

	bzero(&conf, sizeof(conf));

	while ((ch = getopt(argc, argv, "dD:nvs:")) != -1) {
		switch (ch) {
		case 'd':
			debug_stderr = 1;
			break;
		case 'D':
			errno = 0;    /* To distinguish success/failure */
			debug_flags = strtol(optarg, &endptr, 0);

			if ((errno == ERANGE &&
			    (debug_flags == LONG_MAX ||
			     debug_flags == LONG_MIN)) ||
			    (errno != 0 && debug_flags == 0)) {
				perror("strtol");
				usage();
			}
			if (endptr == optarg) {
				fprintf(stderr, "No digits were found\n");
				usage();
			}
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
			/*NOTREACHED*/
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

	if (check_pid())
		errx(1, "anoubisd is already running");

	log_init();

	if (debug_stderr == 0)
		if (daemon(1, 0) !=0)
			fatal("daemon");

	log_info("master start");
	DEBUG(DBG_TRACE, "debug=%x", debug_flags);
	master_pid = getpid();
	DEBUG(DBG_TRACE, "master_pid=%d", master_pid);
	save_pid(master_pid);

	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_m2s) == -1)
		fatal("socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_m2p) == -1)
		fatal("socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_s2p) == -1)
		fatal("socketpair");

	se_pid = session_main(&conf, pipe_m2s, pipe_m2p, pipe_s2p);
	DEBUG(DBG_TRACE, "session_pid=%d", se_pid);
	policy_pid = policy_main(&conf, pipe_m2s, pipe_m2p, pipe_s2p);
	DEBUG(DBG_TRACE, "policy_pid=%d", policy_pid);

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

	queue_init(eventq_m2p);
	queue_init(eventq_m2s);
	queue_init(eventq_m2dev);

	/* init msg_bufs - keep track of outgoing ev_info */
	msg_init(pipe_m2s[0], "m2s");
	msg_init(pipe_m2p[0], "m2p");

	/*
	 * Open eventdev to communicate with the kernel.
	 * This needs to be done before the event_add for it,
	 * because that causes an event.
	 */
	eventfds[0] = open("/dev/eventdev", O_RDWR);
	if (eventfds[0] < 0) {
		log_warn("open(/dev/eventdev)");
		main_shutdown(1);
	}
	msg_init(eventfds[0], "m2dev");

	/* session process */
	event_set(&ev_m2s, pipe_m2s[0], EV_WRITE, dispatch_m2s,
	    &ev_info);

	event_set(&ev_s2m, pipe_m2s[0], EV_READ | EV_PERSIST, dispatch_s2m,
	    &ev_info);
	event_add(&ev_s2m, NULL);

	/* policy process */
	event_set(&ev_m2p, pipe_m2p[0], EV_WRITE, dispatch_m2p,
	    &ev_info);

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

	/*
	 * Open event device to communicate with the kernel.
	 */
	eventfds[1] = open("/dev/anoubis", O_RDWR);
	if (eventfds[1] < 0) {
		log_warn("open(/dev/anoubis)");
		close(eventfds[0]);
		main_shutdown(1);
	}
	/*
	 * ioctl is declared with varadic parameters, which is
	 * impossible to annotate properly. We therefore temporarily
	 * disable checking for passed NULL pointers.
	 */
	/*@-nullpass@*/
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
	/*@=nullpass@*/
	/* Note that we keep /dev/anoubis open for subsequent ioctls. */

	/* Five second timer for statistics ioctl */
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	ev_info.anoubisfd = eventfds[1];
	ev_info.ev_timer = &ev_timer;
	ev_info.tv = &tv;
	evtimer_set(&ev_timer, &dispatch_timer, &ev_info);
	event_add(&ev_timer, &tv);

	DEBUG(DBG_TRACE, "master event loop");
	if (event_dispatch() == -1)
		log_warn("main: event_dispatch");
	DEBUG(DBG_TRACE, "master event loop ended");

	main_cleanup();

	exit(0);
}

static void
main_cleanup(void)
{
	struct sigaction	sa;
	pid_t			pid;

	DEBUG(DBG_TRACE, ">main_cleanup");

	close(eventfds[1]);
	close(eventfds[0]);

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

	unlink(pid_file_name);
}

/*@noreturn@*/
__dead static void
main_shutdown(int error)
{
	main_cleanup();
	log_info("shutting down");
	exit(error);
}

/*@noreturn@*/
__dead void
master_terminate(int error)
{
	DEBUG(DBG_TRACE, ">master_terminate");

	if (master_pid) {
		if (getpid() == master_pid)
			main_shutdown(error);
		else
			kill(master_pid, SIGTERM);
	}
/*	sleep(10); XXX RD - probably not needed */
	_exit(error);
}

static int
check_child(pid_t pid, const char *pname)
{
	int	status;

	DEBUG(DBG_TRACE, ">check_child");

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

	DEBUG(DBG_TRACE, "<check_child");

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

anoubisd_msg_t *
msg_factory(int mtype, int size)
{
	anoubisd_msg_t *msg;

	if ((msg = malloc(sizeof(anoubisd_msg_t) + size)) == NULL) {
		log_warn("msg_factory: cannot allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}
	bzero(msg, sizeof(anoubisd_msg_t) + size);
	msg->mtype = mtype;
	msg->size = sizeof(anoubisd_msg_t) + size;
	return msg;
}

static void
dispatch_m2s(int fd, short event, /*@dependent@*/ void *arg)
{
	/*@dependent@*/
	anoubisd_msg_t *msg;
	struct event_info_main *ev_info = (struct event_info_main*)arg;

	DEBUG(DBG_TRACE, ">dispatch_m2s");

	if ((msg = queue_peek(&eventq_m2s)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_m2s (no msg) ");
		return;
	}

	if (send_msg(fd, msg)) {
		msg = dequeue(&eventq_m2s);
		DEBUG(DBG_QUEUE, " <eventq_m2s: %x",
		    ((struct eventdev_hdr *)msg->msg)->msg_token);
		free(msg);
	}

	/* If the queue is not empty, to be called again */
	if (queue_peek(&eventq_m2s) || msg_pending(fd))
		event_add(ev_info->ev_m2s, NULL);

	DEBUG(DBG_TRACE, "<dispatch_m2s");
}

static void
dispatch_s2m(int fd, short event, void *arg)
{
	/*@dependent@*/
	anoubisd_msg_t *msg;

	DEBUG(DBG_TRACE, ">dispatch_s2m");

	for (;;) {

		if ((msg = get_msg(fd)) == NULL) {
			DEBUG(DBG_TRACE, "<dispatch_s2m");
			return;
		}
		DEBUG(DBG_QUEUE, " >s2m: %x",
		    ((struct eventdev_hdr *)msg->msg)->msg_token);

		/* This should be a sessionid registration message */

/* XXX RD Session registration - optimization */

		free(msg);

		DEBUG(DBG_TRACE, "<dispatch_s2m (loop)");
	}
}

static void
dispatch_m2p(int fd, short event, /*@dependent@*/ void *arg)
{
	/*@dependent@*/
	anoubisd_msg_t *msg;
	struct event_info_main *ev_info = (struct event_info_main*)arg;

	DEBUG(DBG_TRACE, ">dispatch_m2p");

	if ((msg = queue_peek(&eventq_m2p)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_m2p (no msg)");
		return;
	}

	if (send_msg(fd, msg)) {
		msg = dequeue(&eventq_m2p);
		DEBUG(DBG_QUEUE, " <eventq_m2p: %x",
		    ((struct eventdev_hdr *)msg->msg)->msg_token);
		free(msg);
	}

	/* If the queue is not empty, we want to be called again */
	if (queue_peek(&eventq_m2p) || msg_pending(fd))
		event_add(ev_info->ev_m2p, NULL);

	DEBUG(DBG_TRACE, "<dispatch_m2p");
}

static void
dispatch_p2m(int fd, short event, /*@dependent@*/ void *arg)
{
	/*@dependent@*/
	struct event_info_main *ev_info = (struct event_info_main*)arg;
	anoubisd_msg_t *msg;

	DEBUG(DBG_TRACE, ">dispatch_p2m");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL) {
			DEBUG(DBG_TRACE, "<dispatch_p2m");
			return;
		}
		if (msg->mtype != ANOUBISD_MSG_EVENTREPLY) {
			DEBUG(DBG_TRACE, "<dispatch_p2m (bad msg)");
			continue;
		}
		DEBUG(DBG_QUEUE, " >p2m: %x",
		    ((struct eventdev_reply *)msg->msg)->msg_token);

		enqueue(&eventq_m2dev, msg);
		DEBUG(DBG_QUEUE, " >eventq_m2dev: %x",
		    ((struct eventdev_reply *)msg->msg)->msg_token);
		event_add(ev_info->ev_m2dev, NULL);

		DEBUG(DBG_TRACE, "<dispatch_p2m (loop)");
	}
}

static void
dispatch_m2dev(int fd, short event, /*@dependent@*/ void *arg)
{
	/*@dependent@*/
	anoubisd_msg_t *msg;
	struct event_info_main *ev_info = (struct event_info_main*)arg;

	DEBUG(DBG_TRACE, ">dispatch_m2dev");

	if ((msg = queue_peek(&eventq_m2dev)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_m2dev (no msg)");
		return;
	}

	if (send_reply(fd, msg)) {
		msg = dequeue(&eventq_m2dev);
		DEBUG(DBG_QUEUE, " <eventq_m2dev: %x",
		    ((struct eventdev_reply *)msg->msg)->msg_token);
		free(msg);
	}

	if (queue_peek(&eventq_m2dev) || msg_pending(fd))
		event_add(ev_info->ev_m2dev, NULL);

	DEBUG(DBG_TRACE, "<dispatch_m2dev");
}

static void
dispatch_dev2m(int fd, short event, void *arg)
{
	struct eventdev_hdr * hdr;
	/*@dependent@*/
	anoubisd_msg_t *msg;
	/*@dependent@*/
	anoubisd_msg_t *msg_reply;
	struct eventdev_reply *rep;
	struct event_info_main *ev_info = (struct event_info_main*)arg;

	DEBUG(DBG_TRACE, ">dispatch_dev2m");

	for (;;) {
		if ((msg = get_event(fd)) == NULL) {
			DEBUG(DBG_TRACE, "<dispatch_dev2m");
			return;
		}
		hdr = (struct eventdev_hdr *)msg->msg;

		DEBUG(DBG_QUEUE, " >dev2m: %x %c", hdr->msg_token,
		    (hdr->msg_flags & EVENTDEV_NEED_REPLY)  ? 'R' : 'N');

		/* we shortcut and ack events for our own children */
		if ((hdr->msg_flags & EVENTDEV_NEED_REPLY) &&
		    (hdr->msg_pid == se_pid || hdr->msg_pid == policy_pid)) {

			msg_reply = msg_factory(ANOUBISD_MSG_EVENTREPLY,
			    sizeof(struct eventdev_reply));
			rep = (struct eventdev_reply *)msg_reply->msg;
			rep->msg_token = hdr->msg_token;
			rep->reply = 0;

			/* this should be queued, so as to not get lost */
			enqueue(&eventq_m2dev, msg_reply);
			DEBUG(DBG_QUEUE, " >eventq_m2dev: %x",
			    ((struct eventdev_reply *)msg_reply->msg)->
				msg_token);
			event_add(ev_info->ev_m2dev, NULL);

			free(msg);

			DEBUG(DBG_TRACE, "<dispatch_dev2m (self)");
			continue;

		}

		DEBUG(DBG_TRACE, " token %x pid %d", hdr->msg_token,
		    hdr->msg_pid);

/* XXX RD syslog? */

		if ((hdr->msg_flags & EVENTDEV_NEED_REPLY) ||
		    (hdr->msg_source == ANOUBIS_SOURCE_PROCESS) ||
		    (hdr->msg_source == ANOUBIS_SOURCE_SFSEXEC)) {

			/* Send event to policy process for handling. */
			enqueue(&eventq_m2p, msg);
			DEBUG(DBG_QUEUE, " >eventq_m2p: %x", hdr->msg_token);
			event_add(ev_info->ev_m2p, NULL);

		} else {

			/* Send event to session process for notifications. */
			enqueue(&eventq_m2s, msg);
			DEBUG(DBG_QUEUE, " >eventq_m2s: %x", hdr->msg_token);
			event_add(ev_info->ev_m2s, NULL);
		}

		DEBUG(DBG_TRACE, "<dispatch_dev2m (loop)");
	}
}

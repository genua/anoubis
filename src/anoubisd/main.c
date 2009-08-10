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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#ifdef LINUX
#include <bsdcompat.h>
#include <linux/anoubis_sfs.h>
#include <openssl/sha.h>
#else
#include <dev/anoubis_sfs.h>
#include <sha2.h>
#endif

#ifndef OPENBSD
#include <ucontext.h>
#endif

/* on glibc 2.6+, event.h uses non C89 types :/ */
#include <event.h>
#include <anoubis_msg.h>
#include "anoubisd.h"
#include "aqueue.h"
#include "amsg.h"
#include "sfs.h"
#include "cert.h"
#include "cfg.h"

static int	terminate = 0;
static int	eventfds[2];
static void	main_cleanup(void);
/*@noreturn@*/
static void	main_shutdown(int) __dead;
static void	sanitise_stdfd(void);

static void	sighandler(int, short, void *);
static int	check_child(pid_t, const char *);
static void	dispatch_m2s(int, short, void *);
static void	dispatch_s2m(int, short, void *);
static void	dispatch_m2p(int, short, void *);
static void	dispatch_p2m(int, short, void *);
static void	dispatch_m2dev(int, short, void *);
static void	dispatch_dev2m(int, short, void *);
static void	dispatch_m2u(int, short, void *);
static void	dispatch_u2m(int, short, void *);

struct anoubisd_config	anoubisd_config;

pid_t		master_pid = 0;
pid_t		se_pid = 0;
pid_t		logger_pid = 0;
pid_t		policy_pid = 0;
pid_t		upgrade_pid = 0;

u_int32_t	debug_flags = 0;
u_int32_t	debug_stderr = 0;
gid_t		anoubisd_gid;

extern char	*__progname;
char		*logname;
unsigned long	 version;

static Queue eventq_m2p;
static Queue eventq_m2s;
static Queue eventq_m2u;
static Queue eventq_m2dev;

struct event_info_main {
	/*@dependent@*/
	struct event	*ev_m2s, *ev_m2p, *ev_m2u, *ev_m2dev;
	struct event	*ev_s2m, *ev_p2m, *ev_u2m, *ev_dev2m;
	struct event	*ev_sigs[10];
	/*@dependent@*/
	struct event	*ev_timer;
	/*@null@*/
	struct timeval	*tv;
	int anoubisfd;
};

struct upgrade_com {
	int	pipe_m2u[2];
	int	pipe_u2l[2];
	struct event ev_m2u;
	struct event ev_u2m;
};

static struct upgrade_com upgrade_pipes;

static void	_send_sfs_list(u_int64_t token, const char *path,
    u_int8_t *keyid, int idlen, uid_t uid, struct event_info_main *ev_info,
    int search_for_uid, int op, int search_all_ids);
static void	send_id_list(u_int64_t token, const char *entry,
    struct event_info_main *ev_info, int op);
static void	send_entry_list(u_int64_t token, const char *path,
    u_int8_t *keyid, int idlen, uid_t uid, struct event_info_main *ev_info,
    int op, int for_all_ids);
static void	reconfigure(struct event_info_main *);

static char *pid_file_name = PACKAGE_PIDFILE;
#ifdef LINUX
static char *omit_pid_file = "/var/run/sendsigs.omit.d/anoubisd";
#endif

/*
 * SEGV-Handler: Send some useful information to syslog.
 */
void
segvhandler(int sig __used, siginfo_t *info, void *uc)
{
	char	 buf[2048];
	char	*p = buf;
	int	 i, n, nreg;
	void	**ucp = uc;


	sprintf(p, "anoubisd: SEGFAULT at %p ucontext:%n",
	    info?info->si_addr:(void*)-1, &n);
	p += n;
	nreg = sizeof(ucontext_t)/sizeof(void*);
	if (nreg > 100)
		nreg = 100;
	if (ucp) {
		for (i=0; i<nreg; ++i) {
			sprintf(p, " %p%n", ucp[i], &n);
			p += n;
		}
		sprintf(p, "\n");
	} else {
		sprintf(p, " (none)\n");
	}
	/* Avoid unused return value warning by using "i = write(...). */
	i = write(1, buf, strlen(buf));
	syslog(LOG_ALERT, "%s", buf);
	exit(126);
}


/*
 * Note:  Signalhandler managed by libevent are _not_ run in signal
 * context, thus any C-function can be used.
 */
static void
sighandler(int sig, short event __used, void *arg __used)
{
	int			 die = 0;
	struct event_info_main	*info;

	DEBUG(DBG_TRACE, ">sighandler: %d", sig);

	info = arg;

	switch (sig) {
	case SIGTERM:
	case SIGINT: {
		int			 i;
		sigset_t		 mask;

		if (terminate < 1)
			terminate = 1;
		log_warnx("anoubisd: Shutdown requested by signal");
		if (ioctl(eventfds[1], ANOUBIS_UNDECLARE_FD,
		    eventfds[0]) == 0) {
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 10000;
			event_del(info->ev_timer);
			event_del(info->ev_dev2m);
			event_add(info->ev_dev2m, &tv);
			sigfillset(&mask);
			sigdelset(&mask, SIGCHLD);
			sigprocmask(SIG_SETMASK, &mask, NULL);
			if (upgrade_pid)
				kill(upgrade_pid, SIGTERM);
			kill(se_pid, SIGTERM);
			kill(policy_pid, SIGTERM);
			kill(logger_pid, SIGTERM);
			for (i=0; info->ev_sigs[i]; ++i)
				signal_del(info->ev_sigs[i]);
			break;
		}
		log_warn("Cannot undeclare eventdev queue");
		/* FALLTHROUGH */
	}
	case SIGQUIT:
		die = 1;
	case SIGCHLD:
		if (check_child(se_pid, "session engine")) {
			se_pid = 0;
			if (!terminate)
				die = 1;
		}
		if (check_child(policy_pid, "policy engine")) {
			policy_pid = 0;
			if (!terminate)
				die = 1;
		}
		if (check_child(logger_pid, "anoubis logger")) {
			logger_pid = 0;
			if (!terminate)
				die = 1;
		}
		if (check_child(upgrade_pid, "anoubis upgrade")) {
			upgrade_pid = 0;
			/* XXX ch: shall we really terminate or die? */
			if (!terminate)
				die = 1;
		}
		if (terminate && sig == SIGCHLD)
			signal_del(arg);
		if (die) {
			main_shutdown(0);
			/*NOTREACHED*/
		}
		break;
	case SIGHUP:
		reconfigure(info);
		break;
	}

	DEBUG(DBG_TRACE, "<sighandler: %d", sig);
}

FILE *
check_pid(void)
{
	FILE *fp;

	fp = fopen(pid_file_name, "a+");
	if (fp == NULL) {
		log_warn("%s", pid_file_name);
		return NULL;
	}

	if (flock(fileno(fp), LOCK_EX|LOCK_NB) != 0) {
		int errno_save = errno;
		fclose(fp);
		errno = errno_save;
		return NULL;
	}
	return fp;
}

static void
save_pid(FILE *fp, pid_t pid)
{
	rewind(fp);
	if (ftruncate(fileno(fp), 0) != 0) {
		log_warn("%s", pid_file_name);
		fatal("cannot clear pid file");
	}

	fprintf(fp, "%d\n", pid);
	fflush(fp);
}

static void
dispatch_timer(int sig __used, short event __used, /*@dependent@*/ void * arg)
{
	struct event_info_main	*ev_info = arg;
	static int		 first = 1;

	DEBUG(DBG_TRACE, ">dispatch_timer");

	/*
	 * Simulate a SIGCHLD at the first timer event. This will catch
	 * cases where one of the child process exists before we hit enter
	 * the event loop in main.
	 */
	if (first) {
		first = 0;
		sighandler(SIGCHLD, 0, NULL);
	}
	/* ioctls cannot be sensibly annotated because of varadic args */
	/*@i@*/ioctl(ev_info->anoubisfd, ANOUBIS_REQUEST_STATS, 0);
	event_add(ev_info->ev_timer, ev_info->tv);

	DEBUG(DBG_TRACE, "<dispatch_timer");
}

enum anoubisd_process_type	anoubisd_process;

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
	sigset_t		mask;
	int			pipe_m2s[2];
	int			pipe_m2p[2];
	int			pipe_s2p[2];
	int			pipe_m2l[2];
	int			pipe_s2l[2];
	int			pipe_p2l[2];
	int			loggers[4];
	struct timeval		tv;
	struct passwd		*pw;
	FILE			*pidfp;
	struct sigaction	act;
#ifdef LINUX
	FILE			*omitfp;
#endif

	/* Ensure that fds 0, 1 and 2 are open or directed to /dev/null */
	sanitise_stdfd();

	anoubisd_process = PROC_MAIN;

	cfg_initialize(argc, argv);
	if (cfg_read() != 0) {
		fatal("Unable to read configuration");
	}

#ifndef HAVE_SETPROCTITLE
	/* Prepare for later setproctitle emulation */
	compat_init_setproctitle(argc, argv);
#endif

	if ((logname = strdup(__progname)) == NULL) {
		logname = __progname;
		fatal("can't copy progname");
	}

	if (anoubisd_config.opts & ANOUBISD_OPT_NOACTION) {
		/* Dump configuration if requested */
		if (anoubisd_config.opts & ANOUBISD_OPT_VERBOSE) {
			cfg_dump(stdout);
		}
	}

	/*
	 * Get ANOUBISCORE Version early because this is inherited by the
	 * child processes. Close the file descriptior again because we
	 * do not want it to be inherited by child processes.
	 */
	{
		int	fd;
		fd = open("/dev/anoubis", O_RDWR);
		if (fd < 0)
			early_err(2, "Could not open /dev/anoubis");

		if (ioctl(fd, ANOUBIS_GETVERSION, &version) < 0)
			early_err(3, "ANOUBIS_GETVERSION: Cannot retrieve "
			    "version number");

		if (version < ANOUBISCORE_MIN_VERSION
		    || version > ANOUBISCORE_VERSION) {
			char *msg;
			if (asprintf(&msg, "Anoubis Version mismatch: real=%lx "
			    "expected=%lx min=%lx", version,
			    ANOUBISCORE_VERSION, ANOUBISCORE_MIN_VERSION) < 0 )
				msg = NULL;
			early_errx(4, msg);
		}
		close(fd);
	}

	if (anoubisd_config.opts & ANOUBISD_OPT_NOACTION) {
		/* Exit at this point, no further action required */
		exit(0);
	}

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = segvhandler;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGSEGV, &act, NULL) < 0)
		early_errx(1, "Sigaction failed");

	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		early_errx(1, "User " ANOUBISD_USER " does not exist");
	if (pw->pw_gid == 0)
		early_errx(1, "Group ID of " ANOUBISD_USER " must not be 0");
	anoubisd_gid = pw->pw_gid;

	if (geteuid() != 0)
		early_errx(1, "need root privileges");

	if (getpwnam(ANOUBISD_USER) == NULL)
		early_errx(1, "unkown user " ANOUBISD_USER);

	if ((pidfp = check_pid()) == NULL) {
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			early_errx(1, "anoubisd is already running");
		else
			early_errx(1, "cannot write pid file");
	}
	if (debug_stderr == 0)
		if (daemon(1, 0) !=0)
			early_errx(1, "daemonize failed");

	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_m2l) == -1)
		early_errx(1, "socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_p2l) == -1)
		early_errx(1, "socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_s2l) == -1)
		early_errx(1, "socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC,
	    upgrade_pipes.pipe_u2l) == -1)
		early_errx(1, "socketpair");
	logger_pid = logger_main(pipe_m2l, pipe_p2l, pipe_s2l,
	    upgrade_pipes.pipe_u2l);
	close(pipe_m2l[0]);
	close(pipe_p2l[0]);
	close(pipe_s2l[0]);
	close(upgrade_pipes.pipe_u2l[0]);
	loggers[0] = pipe_m2l[1];
	loggers[1] = pipe_p2l[1];
	loggers[2] = pipe_s2l[1];
	loggers[3] = upgrade_pipes.pipe_u2l[1];

	(void)event_init();

	log_init(loggers[0]);
	DEBUG(DBG_TRACE, "logger_pid=%d", logger_pid);

	log_info("master start");
	DEBUG(DBG_TRACE, "debug=%x", debug_flags);
	master_pid = getpid();
	DEBUG(DBG_TRACE, "master_pid=%d", master_pid);
	save_pid(pidfp, master_pid);

	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_m2s) == -1)
		fatal("socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_m2p) == -1)
		fatal("socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_s2p) == -1)
		fatal("socketpair");
	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC,
	    upgrade_pipes.pipe_m2u) == -1)
		fatal("socketpair");

	se_pid = session_main(pipe_m2s, pipe_m2p, pipe_s2p,
	    upgrade_pipes.pipe_m2u, loggers);
	DEBUG(DBG_TRACE, "session_pid=%d", se_pid);
	policy_pid = policy_main(pipe_m2s, pipe_m2p, pipe_s2p,
	    upgrade_pipes.pipe_m2u, loggers);
	DEBUG(DBG_TRACE, "policy_pid=%d", policy_pid);
	upgrade_pid = upgrade_main(upgrade_pipes.pipe_m2u, pipe_m2p,
	    pipe_s2p, pipe_m2s, loggers);
	DEBUG(DBG_TRACE, "upgrade_pid=%d", upgrade_pid);
	close(loggers[1]);
	close(loggers[2]);
	close(loggers[3]);

	close(pipe_m2s[1]);
	close(pipe_m2p[1]);
	close(pipe_s2p[0]);
	close(pipe_s2p[1]);
	close(upgrade_pipes.pipe_m2u[1]);

	/* Load Public Keys */
	cert_init(0);

	setproctitle("master");

#ifdef LINUX
	if ((omitfp = fopen(omit_pid_file, "w"))) {
		fprintf(omitfp, "%d\n%d\n%d\n%d\n%d\n",
		    master_pid, se_pid, policy_pid, logger_pid, upgrade_pid);
		fclose(omitfp);
	}
#endif

	/* We catch or block signals rather than ignore them. */
	signal_set(&ev_sigterm, SIGTERM, sighandler, &ev_info);
	signal_set(&ev_sigint, SIGINT, sighandler, &ev_info);
	signal_set(&ev_sigquit, SIGQUIT, sighandler, NULL);
	signal_set(&ev_sighup, SIGHUP, sighandler, &ev_info);
	signal_set(&ev_sigchld, SIGCHLD, sighandler, &ev_sigchld);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);
	signal_add(&ev_sighup, NULL);
	signal_add(&ev_sigchld, NULL);
	ev_info.ev_sigs[0] = &ev_sigterm;
	ev_info.ev_sigs[1] = &ev_sigint;
	ev_info.ev_sigs[2] = &ev_sigquit;
	ev_info.ev_sigs[3] = &ev_sighup;
	ev_info.ev_sigs[4] = NULL;

	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGQUIT);
	sigdelset(&mask, SIGHUP);
	sigdelset(&mask, SIGCHLD);
	sigdelset(&mask, SIGSEGV);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	queue_init(eventq_m2p);
	queue_init(eventq_m2s);
	queue_init(eventq_m2dev);
	queue_init(eventq_m2u);

	/* init msg_bufs - keep track of outgoing ev_info */
	msg_init(pipe_m2s[0], "m2s");
	msg_init(pipe_m2p[0], "m2p");
	msg_init(upgrade_pipes.pipe_m2u[0], "m2u");

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

	/* upgrade process */
	event_set(&upgrade_pipes.ev_m2u, upgrade_pipes.pipe_m2u[0], EV_WRITE,
	    dispatch_m2u, &ev_info);

	event_set(&upgrade_pipes.ev_u2m, upgrade_pipes.pipe_m2u[0],
	    EV_READ | EV_PERSIST, dispatch_u2m, &ev_info);
	event_add(&upgrade_pipes.ev_u2m, NULL);

	/* event device */
	event_set(&ev_dev2m, eventfds[0], EV_READ | EV_PERSIST, dispatch_dev2m,
	    &ev_info);
	event_add(&ev_dev2m, NULL);

	event_set(&ev_m2dev, eventfds[0], EV_WRITE, dispatch_m2dev,
	    &ev_info);

	ev_info.ev_m2s = &ev_m2s;
	ev_info.ev_m2p = &ev_m2p;
	ev_info.ev_m2dev = &ev_m2dev;
	ev_info.ev_dev2m = &ev_dev2m;
	ev_info.ev_p2m = &ev_p2m;
	ev_info.ev_s2m = &ev_s2m;
	ev_info.ev_m2u = &upgrade_pipes.ev_m2u;
	ev_info.ev_u2m = &upgrade_pipes.ev_u2m;

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
	if (ioctl(eventfds[1], ANOUBIS_DECLARE_FD, eventfds[0]) < 0) {
		log_warn("ioctl");
		close(eventfds[0]);
		close(eventfds[1]);
		main_shutdown(1);
	}
	if (ioctl(eventfds[1], ANOUBIS_DECLARE_LISTENER, eventfds[0]) < 0) {
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
	/*
	 * We only reach this point in case of a graceful shutdown. Thus
	 * must of the cleanup is already done. In particular we must
	 * not wait for our children: session and policy already exited
	 * and logger will terminate after us.
	 */
	unlink(pid_file_name);
#ifdef LINUX
	unlink(omit_pid_file);
#endif
	log_warnx("anoubisd shutdown");
	flush_log_queue();

	exit(0);
}

static void
main_cleanup(void)
{
	pid_t			pid;

	DEBUG(DBG_TRACE, ">main_cleanup");

	close(eventfds[1]);
	close(eventfds[0]);

	if (upgrade_pid)
		kill(upgrade_pid, SIGQUIT);
	if (se_pid)
		kill(se_pid, SIGQUIT);
	if (policy_pid)
		kill(policy_pid, SIGQUIT);
	if (logger_pid)
		kill(logger_pid, SIGQUIT);

	/*
	 * Do NOT wait for the logger. The logger process will terminate
	 * once we exit and close our pipe to the logger.
	 */
	while (se_pid && policy_pid && upgrade_pid) {
		if ((pid = wait(NULL)) == -1 &&
		    errno != EINTR && errno != ECHILD)
			fatal("wait");
		if (pid == upgrade_pid)
			upgrade_pid = 0;
		if (pid == se_pid)
			se_pid = 0;
		if (pid == policy_pid)
			policy_pid = 0;
		if (pid == logger_pid)
			logger_pid = 0;
	}

	log_warnx("anoubisd is terminating");
	flush_log_queue();
	unlink(pid_file_name);
#ifdef LINUX
	unlink(omit_pid_file);
#endif
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
	}
	/*
	 * This will send a SIGCHLD to the master which will then terminate.
	 * It is not possible to just kill(2) the master because we switched
	 * UIDs and are no longer allowed to do this.
	 */
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
		early_err(1, "open");

	while (++dupfd <= 2) {
		/* Only clobber closed fds */
		if (fcntl(dupfd, F_GETFL, 0) >= 0)
			continue;
		if (dup2(nullfd, dupfd) == -1) {
			early_err(1, "dup2");
		}
	}
	if (nullfd > 2)
		close(nullfd);
}

static void
reconfigure(struct event_info_main *info)
{
	anoubisd_msg_t	*msg;

	/* Reload Public Keys */
	cert_reconfigure(0);

	if (cfg_reread() != 0) {
		log_warn("re-read of configuration failed.");
	} else {
		msg = cfg_msg_create();

		if (msg != NULL) {
			enqueue(&eventq_m2s, msg);
			event_add(info->ev_m2s, NULL);
		}

		msg = cfg_msg_create();
		if (msg != NULL) {
			enqueue(&eventq_m2p, msg);
			event_add(info->ev_m2p, NULL);
		}
	}
}

static void
dispatch_m2s(int fd, short event __used, /*@dependent@*/ void *arg)
{
	anoubisd_msg_t			*msg;
	struct event_info_main		*ev_info = arg;
	int				 ret;
	eventdev_token			 token = 0;

	DEBUG(DBG_TRACE, ">dispatch_m2s");

	msg = queue_peek(&eventq_m2s);
	if (msg)
		token = ((struct eventdev_hdr *)msg->msg)->msg_token;
	ret = send_msg(fd, msg);
	if (msg) {
		if (ret < 0) {
			/* Error: Drop message */
			dequeue(&eventq_m2s);
			DEBUG(DBG_QUEUE, " <eventq_m2s: dropping %x", token);
			free(msg);
		} else if (ret > 0) {
			/* Success: send_msg will free the message. */
			dequeue(&eventq_m2s);
			DEBUG(DBG_QUEUE, " <eventq_m2s: sent %x", token);
		}
	}
	/* Write was not successful: Check if we lost one of our childs. */
	if (ret <= 0)
		sighandler(SIGCHLD, 0, NULL);
	/* If the queue is not empty, to be called again */
	if (queue_peek(&eventq_m2s) || msg_pending(fd))
		event_add(ev_info->ev_m2s, NULL);
	else if (terminate >= 2) {
		shutdown(fd, SHUT_WR);
	}

	DEBUG(DBG_TRACE, "<dispatch_m2s");
}

/*
 * Search in entries of the shadowtree for uids/keyid that registered a
 * checksum/signautre of the entry.
 */
static void
send_id_list(u_int64_t token, const char *entry,
    struct event_info_main *ev_info, int op)
{
	_send_sfs_list(token, entry, NULL, 0, 0, ev_info, 1, op, 0);
}

/*
 * Send registered entries of a directory. Also send directorys which
 * contain further entries.
 */
static void
send_entry_list(u_int64_t token, const char *path, u_int8_t *keyid,
    int idlen, uid_t uid, struct event_info_main *ev_info, int op,
    int for_all_ids)
{
	if (keyid)
		if (idlen <= 0)
			return;

	_send_sfs_list(token, path, keyid, idlen, uid, ev_info, 0, op,
	    for_all_ids);
}

static int
check_for_entry(const char *path, const char *entry, unsigned char *keyid,
    int idlen, uid_t uid)
{
	char *sigfile = NULL,
	     *tmp = NULL;
	int i, ret = -1;

	if (!entry)
		return -1;

	if (keyid != NULL) {
		sigfile = calloc((2*idlen)+1, sizeof(char));
		if (sigfile == NULL) {
			log_warn("check_for_entry: can't allocate memory");
			master_terminate(ENOMEM);
			return -1;
		}
		for (i = 0; i < idlen; i++)
			sprintf(&sigfile[2*i], "%2.2x", keyid[i]);
		sigfile[2*i] = '\0';
		ret = asprintf(&tmp, "%s/%s/k%s", path, entry, sigfile);
		if (ret < 0)
			return -1;
		ret = check_if_exist(tmp);
		free(tmp);
		free(sigfile);
	} else {
		ret = asprintf(&tmp, "%s/%s/%d", path, entry, uid);
		if (ret < 0) {
			log_warn("check_for_entry: can't allocate memory");
			master_terminate(ENOMEM);
			return -1;
		}
		ret = check_if_exist(tmp);
		free(tmp);
	}
	return ret;
}

static void
_send_sfs_list(u_int64_t token, const char *path, u_int8_t *keyid,
    int idlen, uid_t uid, struct event_info_main *ev_info, int search_for_id,
    int op, int for_all_ids)
{
	/* Since only pathname components are transferred, it is sufficient
	 * that the payload size (3000) is larger than NAME_MAX (255),
	 * it does not need to be larger than PATH_MAX (4096)
	 */
	DIR			*sfs_dir;
	struct dirent		*sfs_ent = NULL;
	anoubisd_reply_t	*reply;
	anoubisd_msg_t		*msg;
	char			*tmp = NULL;
	int			 flags = POLICY_FLAG_START;
	int			 size = sizeof(struct anoubisd_reply) + 3000;
	int			 cnt = 0, error = 0, len = 0, stars, i, ret = 0;

	sfs_dir = opendir(path);
	if (!sfs_dir) {
		if (errno == ENOENT)
			error = 0;
		else
			error = errno;
		goto err;
	}

	sfs_ent = readdir(sfs_dir);
	while (1) {
		msg = msg_factory(ANOUBISD_MSG_POLREPLY, size);
		if (!msg) {
			log_warn("send_checksum_list: can't allocate memory");
			master_terminate(ENOMEM);
			return;
		}
		reply = (anoubisd_reply_t*)msg->msg;
		reply->flags = flags;
		reply->token = token;
		reply->timeout = 0;
		reply->reply = 0;

		while (sfs_ent != NULL) {
			if ((strcmp(sfs_ent->d_name, ".") == 0) ||
			    (strcmp(sfs_ent->d_name, "..") == 0)) {
			    sfs_ent = readdir(sfs_dir);
				continue;
			}
			len = strlen(sfs_ent->d_name);
			for (i = 0, stars = 0; i < len; i++) {
				if (sfs_ent->d_name[i] == '*')
					stars++;
			}
			/* If the ammount of stars is odd its an sfsentry
			 * if its even its an directory. If the ammount of
			 * stars is 0 we and we are checking for
			 * uids/keyids we are going to the next if.
			 */
			if (stars%2 && !search_for_id) {
				if (op == ANOUBIS_CHECKSUM_OP_LIST_ALL) {
					/* The user root try to export for
					 * all users so we don't check for
					 * correctness
					 */
					if (for_all_ids)
						goto out;
					/* Check the keyid */
					if (keyid) {
						ret = check_for_entry(path,
						    sfs_ent->d_name,
						    keyid, idlen, 0);
					}
					if (ret == 0) {
						ret = check_for_entry(path,
						    sfs_ent->d_name,
						    NULL, 0, uid);
					}
				} else {
					ret = check_for_entry(path,
					    sfs_ent->d_name, keyid, idlen, uid);
				}
				/* It is not our searched entry */
				if (ret == 0) {
					sfs_ent = readdir(sfs_dir);
					continue;
				/* An error occured */
				} else if (ret < 0) {
					goto err;
				}
				/* We found our entry */
			}
out:
			if (search_for_id) {
				if (op == ANOUBIS_CHECKSUM_OP_KEYID_LIST) {
					if (sfs_ent->d_name[0] != 'k') {
						sfs_ent = readdir(sfs_dir);
						continue;
					}
					tmp = strdup(&sfs_ent->d_name[1]);
				} else {
					if (sfs_ent->d_name[0] == 'k') {
						sfs_ent = readdir(sfs_dir);
						continue;
					}
					tmp = strdup(sfs_ent->d_name);
				}
			} else {
				tmp = remove_escape_seq(sfs_ent->d_name);
			}
			if (!tmp) {
				log_warn("send_checksum_list: "
				    "can't allocate memory");
				master_terminate(ENOMEM);
				return;
			}

			/* Now we pack and ship */
			len = strlen(tmp) + 1;
			cnt += len;
			if (cnt > 3000) {
				cnt -= len;
				free(tmp);
				break;
			}
			memcpy(&reply->msg[cnt - len], tmp, len);
			free(tmp);
			sfs_ent = readdir(sfs_dir);
		}

		reply->len = cnt;
		msg_shrink(msg, sizeof(anoubisd_reply_t) + cnt);
		cnt = 0;

		if (sfs_ent == NULL)
			reply->flags |= POLICY_FLAG_END;

		enqueue(&eventq_m2s, msg);
		event_add(ev_info->ev_m2s, NULL);
		flags = 0;

		if (sfs_ent == NULL)
			break;
	}
	closedir(sfs_dir);
	return;

err:
	msg = msg_factory(ANOUBISD_MSG_POLREPLY, sizeof(anoubisd_reply_t));
	if (!msg) {
		log_warn("send_checksum_list: can't allocate memory");
		master_terminate(ENOMEM);
	}
	reply = (anoubisd_reply_t*)msg->msg;
	reply->token = token;
	reply->timeout = 0;
	reply->flags = flags;
	reply->reply = error;
	reply->flags |= POLICY_FLAG_END;
	reply->len = 0;
	enqueue(&eventq_m2s, msg);
	event_add(ev_info->ev_m2s, NULL);
}

static void
send_sfscache_invalidate_uid(const char *path, uid_t uid,
    struct event_info_main *ev_info)
{
	struct anoubisd_msg			*msg;
	struct anoubisd_sfscache_invalidate	*invmsg;
	msg = msg_factory(ANOUBISD_MSG_SFSCACHE_INVALIDATE,
	    sizeof(struct anoubisd_sfscache_invalidate) + strlen(path) + 1);
	if (!msg) {
		master_terminate(ENOMEM);
		return;
	}
	invmsg = (struct anoubisd_sfscache_invalidate*)msg->msg;
	invmsg->uid = uid;
	invmsg->plen = strlen(path)+1;
	invmsg->keylen = 0;
	memcpy(invmsg->payload, path, invmsg->plen);
	enqueue(&eventq_m2p, msg);
	event_add(ev_info->ev_m2p, NULL);
}


static void
send_sfscache_invalidate_key(const char *path, u_int8_t *keyid, int keylen,
    struct event_info_main *ev_info)
{
	struct anoubisd_msg			*msg;
	struct anoubisd_sfscache_invalidate	*invmsg;
	int					 i, j;

	msg = msg_factory(ANOUBISD_MSG_SFSCACHE_INVALIDATE,
	    sizeof(struct anoubisd_sfscache_invalidate) + strlen(path) + 1
	    + 2*keylen+1);
	if (!msg) {
		master_terminate(ENOMEM);
		return;
	}
	invmsg = (struct anoubisd_sfscache_invalidate*)msg->msg;
	invmsg->uid = (uid_t)-1;
	invmsg->plen = strlen(path)+1;
	invmsg->keylen = 2*keylen+1;
	memcpy(invmsg->payload, path, invmsg->plen);
	i = invmsg->plen;
	for (j=0; j<keylen; ++j) {
		sprintf(invmsg->payload + i, "%2.2x", keyid[j]);
		i += 2;
	}
	invmsg->payload[i] = 0;
	enqueue(&eventq_m2p, msg);
	event_add(ev_info->ev_m2p, NULL);

}

/* If a message arrives regarding a signature the payload of the message
 * should be including at least the keyid and the regarding path.
 *	---------------------------------
 *	|	keyid	|	path	|
 *	---------------------------------
 * futhermore should the message include the signature and the checksum
 * if the requested operation is ADDSIG
 *	-----------------------------------------------------------------
 *	|	keyid	|	csum	|	sigbuf	|	path	|
 *	-----------------------------------------------------------------
 * idlen is in this the length of the keyid. rawmsg.length is the total
 * length of the message excluding the path length.
 */

static int
sfs_op_is_add(int op)
{
	return (op == ANOUBIS_CHECKSUM_OP_ADDSUM
	    || op == ANOUBIS_CHECKSUM_OP_ADDSIG);
}

static int
parse_sfs_checksumop(struct sfs_checksumop *dst, struct anoubis_msg *m,
    uid_t auth_uid)
{
	char		*payload = NULL;
	int		 curlen, plen, error;
	unsigned int	 flags;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_ChecksumRequestMessage)))
		return -EINVAL;
	memset(dst, 0, sizeof(struct sfs_checksumop));
	dst->op = get_value(m->u.checksumrequest->operation);
	flags = get_value(m->u.checksumrequest->flags);
	dst->auth_uid = auth_uid;
	if (flags & ANOUBIS_CSUM_UID)
		dst->uid = get_value(m->u.checksumrequest->uid);
	else
		dst->uid = auth_uid;
	dst->idlen = get_value(m->u.checksumrequest->idlen);
	if (sfs_op_is_add(dst->op)) {
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_ChecksumAddMessage)))
			return -EINVAL;
		payload = (void*)m->u.checksumadd->payload;
		dst->siglen = get_value(m->u.checksumadd->cslen);
		dst->sigdata = (u_int8_t*)payload + dst->idlen;
	} else {
		payload = (void*)m->u.checksumrequest->payload;
	}
	if (dst->idlen) {
		dst->keyid = (void*)payload;
		/*
		 * ANOUBIS_CSUM_UID is not permitted together with a key ID.
		 * Exception (*sigh*): ANOUBIS_CHECKSUM_OP_LIST_ALL
		 */
		if (flags & ANOUBIS_CSUM_UID
		    && dst->op != ANOUBIS_CHECKSUM_OP_LIST_ALL)
			return -EINVAL;
	}
	dst->path = payload + dst->idlen + dst->siglen;
	curlen = (char*)payload - (char*)m->u.buf;
	if (curlen < 0 || curlen > (int)m->length - CSUM_LEN)
		return -EFAULT;
	plen = m->length - curlen - CSUM_LEN;
	/* XXX CEH: We should reject any flags that we do not handle. */
	if (dst->op == ANOUBIS_CHECKSUM_OP_LIST_ALL)
		dst->allflag = !!(flags & ANOUBIS_CSUM_ALL);

	error = -EINVAL;
	for (curlen = 0; curlen < plen; ++curlen) {
		if (dst->path[curlen] == 0) {
			error = 0;
			break;
		}
	}
	return error;
}

static int
validate_sfs_checksumop(struct sfs_checksumop *csop)
{
	struct cert		*cert = NULL;

	if (!csop->path)
		return -EINVAL;
	if (csop->allflag && csop->op != ANOUBIS_CHECKSUM_OP_LIST_ALL)
		return -EINVAL;
	/* Requests for another user's uid are only allowed for root. */
	if (csop->uid != csop->auth_uid && csop->auth_uid != 0)
		return -EPERM;
	if (csop->keyid) {
		cert = cert_get_by_keyid(csop->keyid, csop->idlen);
		if (cert == NULL)
			return -EPERM;
		/*
		 * If the requesting user is not root, the certificate
		 * must belong to the authenticated user.
		 */
		if (cert->uid != csop->auth_uid && csop->auth_uid != 0)
			return -EPERM;
		/*
		 * If the requested uid is different from the authenticated
		 * user it must be the same as the uid in the cert.
		 */
		if (csop->auth_uid != csop->uid && csop->uid != cert->uid)
			return -EPERM;
	}

	switch(csop->op) {
	case ANOUBIS_CHECKSUM_OP_DEL:
	case ANOUBIS_CHECKSUM_OP_GET:
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
		if (cert != NULL)
			return -EINVAL;
		if (csop->op == ANOUBIS_CHECKSUM_OP_ADDSUM) {
			if (!csop->sigdata || csop->siglen != ANOUBIS_CS_LEN)
				return -EINVAL;
		} else {
			if (csop->sigdata)
				return -EINVAL;
		}
		break;
	case ANOUBIS_CHECKSUM_OP_GETSIG:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
		if (cert == NULL)
			return -EPERM;
		if (csop->op == ANOUBIS_CHECKSUM_OP_ADDSIG) {
			if (!csop->sigdata || csop->siglen < ANOUBIS_CS_LEN)
				return -EINVAL;
		} else {
			if (csop->sigdata)
				return -EINVAL;
		}
		break;
	case ANOUBIS_CHECKSUM_OP_LIST:
		if (csop->sigdata)
			return -EINVAL;
		break;
	case ANOUBIS_CHECKSUM_OP_UID_LIST:
	case ANOUBIS_CHECKSUM_OP_KEYID_LIST:
		if (csop->auth_uid != 0)
			return -EPERM;
		if (csop->auth_uid != csop->uid) {
			/*
			 * XXX CEH: We should return an error but sfssig
			 * XXX CEH: sends request like this.
			 */
			csop->uid = 0;
		}
		if (csop->keyid)
			return -EINVAL;
		if (csop->sigdata)
			return -EINVAL;
		break;
	case ANOUBIS_CHECKSUM_OP_LIST_ALL:
		if (csop->sigdata)
			return -EINVAL;
		if (csop->allflag && csop->auth_uid != 0)
			return -EPERM;
		break;
	case ANOUBIS_CHECKSUM_OP_VALIDATE:
		/*
		 * This operation is only used by sfssig internally and
		 * should go away. The daemon does not handle validate
		 * requests at all.
		 */
		return -EINVAL;
	default:
		return -EINVAL;
	}
	return 0;
}

static void
dispatch_checksumop(anoubisd_msg_t *msg, struct event_info_main *ev_info)
{
	struct anoubis_msg rawmsg;
	anoubisd_msg_checksum_op_t *msg_comm =
	    (anoubisd_msg_checksum_op_t *)msg->msg;
	char * path = NULL;
	char *result = NULL;
	int	flags;
	int	cslen = 0,
		l_all = 0,
		idlen = 0,		/* Length of KeyID		*/
		siglen = 0;		/* Length of Signature and csum */
	uid_t	req_uid;
	int err = -EFAULT, op = 0, extra = 0;
	anoubisd_reply_t * reply;
	u_int8_t *digest = NULL;
	u_int8_t *key = NULL;
	u_int8_t *sign = NULL;
	int i, plen;
	struct sfs_checksumop	csop;

	rawmsg.length = msg_comm->len;
	rawmsg.u.buf = msg_comm->msg;
	if (parse_sfs_checksumop(&csop, &rawmsg, msg_comm->uid) < 0) {
		err = -EINVAL;
		goto out;
	}
	if (validate_sfs_checksumop(&csop) < 0) {
		/*
		 * XXX CEH: These log messages are temporary and should
		 * XXX CEH: go away once the normal user land no longer
		 * XXX CEH: sends invalid message.
		 */
		log_warnx("CHECKSUM OP VALIDATION FAILED");
		log_warnx("checksumop: op=%d, allflag=%d uid=%d auth_uid=%d",
		    csop.op, csop.allflag, csop.uid, csop.auth_uid);
		log_warnx("checksumop: idlen=%d siglen=%d", csop.idlen,
		    csop.siglen);
		log_warnx("checksumop: path=%s", csop.path);
		err = -EINVAL;
		goto out;
	}


	if (!VERIFY_LENGTH(&rawmsg, sizeof(Anoubis_ChecksumRequestMessage)))
		goto out;
	op = get_value(rawmsg.u.checksumrequest->operation);
	req_uid = get_value(rawmsg.u.checksumrequest->uid);
	flags = get_value(rawmsg.u.checksumrequest->flags);
	idlen = get_value(rawmsg.u.checksumrequest->idlen);

	if (!(flags & ANOUBIS_CSUM_UID))
		req_uid = msg_comm->uid;
	l_all = (flags & ANOUBIS_CSUM_ALL);

	switch (op) {
	case ANOUBIS_CHECKSUM_OP_LIST:
	case ANOUBIS_CHECKSUM_OP_LIST_ALL:
	case ANOUBIS_CHECKSUM_OP_UID_LIST:
	case ANOUBIS_CHECKSUM_OP_KEYID_LIST:
		plen = rawmsg.length - CSUM_LEN
		    - sizeof(Anoubis_ChecksumRequestMessage);
		path = rawmsg.u.checksumrequest->payload + idlen;
		if (idlen > 0 && ((op == ANOUBIS_CHECKSUM_OP_LIST) ||
		    (op == ANOUBIS_CHECKSUM_OP_LIST_ALL))) {
			if ((key = calloc(idlen, sizeof(u_int8_t))) == NULL)
				goto out;
			memcpy(key, rawmsg.u.checksumrequest->payload, idlen);
		}
		for (i=0; i<plen; ++i) {
			if (path[i] == 0)
				break;
		}
		if (i >= plen)
			goto out;

		if ((op == ANOUBIS_CHECKSUM_OP_LIST) ||
		    (op == ANOUBIS_CHECKSUM_OP_LIST_ALL)) {
			err = convert_user_path(path, &result, 1);
			if (err < 0)
				goto out;
			send_entry_list(msg_comm->token, result, key,
			    idlen, req_uid, ev_info, op, l_all);
		} else {
			err = convert_user_path(path, &result, 0);
			if (err < 0) {
				log_warn("Error while converting user path");
				goto out;
			}
			send_id_list(msg_comm->token, result, ev_info, op);
		}
		if (result)
			free(result);
		if (key)
			free(key);
		return;
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
		if (!VERIFY_LENGTH(&rawmsg, sizeof(Anoubis_ChecksumAddMessage)))
			goto out;
		cslen = get_value(rawmsg.u.checksumadd->cslen);
		if ((cslen != SHA256_DIGEST_LENGTH) &&
		    (op == ANOUBIS_CHECKSUM_OP_ADDSUM)) {
			err = -EINVAL;
			goto out;
		}
		if (!VERIFY_LENGTH(&rawmsg,
		    sizeof(Anoubis_ChecksumAddMessage) + idlen + cslen))
			goto out;
		digest = (u_int8_t *)calloc(idlen + cslen, sizeof(u_int8_t));
		if (!digest) {
			err = -ENOMEM;
			goto out;
		}
		memcpy(digest, rawmsg.u.checksumadd->payload, idlen + cslen);
		path = rawmsg.u.checksumadd->payload + cslen + idlen;
		plen = rawmsg.length - CSUM_LEN - cslen - idlen
		    - sizeof(Anoubis_ChecksumAddMessage);
		break;
	case ANOUBIS_CHECKSUM_OP_GETSIG:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
		idlen = get_value(rawmsg.u.checksumrequest->idlen);
		cslen = 0;
		path = rawmsg.u.checksumrequest->payload + idlen;
		plen = rawmsg.length - CSUM_LEN - idlen
		    - sizeof(Anoubis_ChecksumRequestMessage);
		digest = (u_int8_t *)calloc(idlen, sizeof(u_int8_t));
		if (!digest) {
			err = -ENOMEM;
			goto out;
		}
		memcpy(digest, rawmsg.u.checksumrequest->payload, idlen);
		break;
	default:
		digest = (u_int8_t *)calloc(SHA256_DIGEST_LENGTH,
		    sizeof(u_int8_t));
		if (!digest) {
			err = -ENOMEM;
			goto out;
		}
		cslen = SHA256_DIGEST_LENGTH;
		path = rawmsg.u.checksumrequest->payload;
		plen = rawmsg.length - CSUM_LEN
		    - sizeof(Anoubis_ChecksumRequestMessage);
		break;
	}

	if (plen < 0)
		goto out;

	for (i=0; i<plen; ++i) {
		if (path[i] == 0)
			break;
	}
	if (i >= plen)
		goto out;

	err = sfs_checksumop(path, op, req_uid, digest, &sign, &siglen, cslen,
	    idlen);
	extra = 0;
	if (err == 0) {
		switch (op) {
		case ANOUBIS_CHECKSUM_OP_GET:
			extra = SHA256_DIGEST_LENGTH;
			break;
		case ANOUBIS_CHECKSUM_OP_GETSIG:
			/* siglen is the length of signature and the
			 * regarding checksum */
			extra = siglen;
			break;
		case ANOUBIS_CHECKSUM_OP_ADDSUM:
		case ANOUBIS_CHECKSUM_OP_DEL:
			extra = 0;
			send_sfscache_invalidate_uid(path, req_uid, ev_info);
			break;
		case ANOUBIS_CHECKSUM_OP_ADDSIG:
		case ANOUBIS_CHECKSUM_OP_DELSIG:
			extra = 0;
			send_sfscache_invalidate_key(path, digest, idlen,
			    ev_info);
		default:
			extra = 0;
			break;
		}
	}
out:
	msg = msg_factory(ANOUBISD_MSG_POLREPLY,
	    sizeof(anoubisd_reply_t) + extra);
	if (!msg) {
		master_terminate(ENOMEM);
		return;
	}
	reply = (anoubisd_reply_t*)msg->msg;
	reply->token = msg_comm->token;
	reply->timeout = 0;
	reply->reply = -err;
	reply->flags = POLICY_FLAG_START | POLICY_FLAG_END;
	reply->len = extra;
	if (sign) {
		memcpy(reply->msg, sign, extra);
		free(sign);
	} else if (extra)
		memcpy(reply->msg, digest, extra);
	if (digest)
		free(digest);
	enqueue(&eventq_m2s, msg);
	DEBUG(DBG_QUEUE, " >eventq_m2s: %llx",
	    (unsigned long long)reply->token);
	event_add(ev_info->ev_m2s, NULL);
}

static void
dispatch_s2m(int fd, short event __used, void *arg)
{
	/*@dependent@*/
	anoubisd_msg_t *msg;
	struct event_info_main *ev_info = (struct event_info_main*)arg;

	DEBUG(DBG_TRACE, ">dispatch_s2m");

	for (;;) {

		if ((msg = get_msg(fd)) == NULL)
			break;
		switch(msg->mtype) {
		case ANOUBISD_MSG_CHECKSUM_OP:
			dispatch_checksumop(msg, ev_info);
			break;
		default:
			DEBUG(DBG_QUEUE, " >s2m: %x",
			    ((struct eventdev_hdr *)msg->msg)->msg_token);

			/* This should be a sessionid registration message */
			log_warn("dispatch_s2m: bad mtype %d", msg->mtype);
			break;
		}

/* XXX RD Session registration - optimization */

		free(msg);

		DEBUG(DBG_TRACE, "<dispatch_s2m (loop)");
	}
	if (msg_eof(fd))
		event_del(ev_info->ev_s2m);
	DEBUG(DBG_TRACE, "<dispatch_s2m");
}

static void
dispatch_m2p(int fd, short event __used, /*@dependent@*/ void *arg)
{
	anoubisd_msg_t			*msg;
	struct event_info_main		*ev_info = arg;
	int				 ret;
	eventdev_token			 token = 0;

	DEBUG(DBG_TRACE, ">dispatch_m2p");

	msg = queue_peek(&eventq_m2p);
	if (msg)
		token = ((struct eventdev_hdr *)msg->msg)->msg_token;
	ret = send_msg(fd, msg);
	if (msg) {
		if (ret < 0) {
			/* Error: Drop message */
			dequeue(&eventq_m2p);
			DEBUG(DBG_QUEUE, " <eventq_m2p: dropping %x", token);
			free(msg);
		} else if (ret > 0) {
			/* Success: send_msg will free the message. */
			dequeue(&eventq_m2p);
			DEBUG(DBG_QUEUE, " <eventq_m2p: sent %x", token);
		}
	}

	/* Write was not successful. See if we lost one of our childs. */
	if (ret <= 0)
		sighandler(SIGCHLD, 0, NULL);

	/* If the queue is not empty, we want to be called again */
	if (queue_peek(&eventq_m2p) || msg_pending(fd))
		event_add(ev_info->ev_m2p, NULL);
	else if (terminate >= 2) {
		shutdown(fd, SHUT_WR);
	}

	DEBUG(DBG_TRACE, "<dispatch_m2p");
}

static void
dispatch_p2m(int fd, short event __used, /*@dependent@*/ void *arg)
{
	/*@dependent@*/
	struct event_info_main	*ev_info = arg;
	anoubisd_msg_t		*msg;

	DEBUG(DBG_TRACE, ">dispatch_p2m");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL)
			break;
		switch(msg->mtype) {
		case ANOUBISD_MSG_EVENTREPLY:
			DEBUG(DBG_QUEUE, " >p2m: eventdev msg token=%x",
			    ((struct eventdev_reply *)msg->msg)->msg_token);
			enqueue(&eventq_m2dev, msg);
			DEBUG(DBG_QUEUE, " >eventq_m2dev: %x",
			    ((struct eventdev_reply *)msg->msg)->msg_token);
			event_add(ev_info->ev_m2dev, NULL);
			break;
		case ANOUBISD_MSG_UPGRADE:
			DEBUG(DBG_QUEUE, " >p2m: upgrade msg");
			enqueue(&eventq_m2u, msg);
			event_add(ev_info->ev_m2u, NULL);
			break;
		default:
			DEBUG(DBG_TRACE, "<dispatch_p2m (bad msg)");
		}

		DEBUG(DBG_TRACE, "<dispatch_p2m (loop)");
	}
	if (msg_eof(fd))
		event_del(ev_info->ev_p2m);
	DEBUG(DBG_TRACE, "<dispatch_p2m");
}

static void
dispatch_m2dev(int fd, short event __used, /*@dependent@*/ void *arg)
{
	/*@dependent@*/
	anoubisd_msg_t *msg;
	struct event_info_main *ev_info = (struct event_info_main*)arg;
	ssize_t			ret;

	DEBUG(DBG_TRACE, ">dispatch_m2dev");

	/* msg was checked for non-nullness just above */
	/*@-nullderef@*/ /*@-nullpass@*/
	if ((msg = queue_peek(&eventq_m2dev)) == NULL) {
		DEBUG(DBG_TRACE, "<dispatch_m2dev (no msg)");
		return;
	}
	/*@=nullderef@*/ /*@=nullpass@*/

	switch(msg->mtype) {
		case ANOUBISD_MSG_EVENTREPLY:
			ret = write(fd, msg->msg,
			    sizeof(struct eventdev_reply));

			/*
			 * ESRCH/EINVAL returns are events which
			 * have been cancelled before anoubisd replied.
			 * These should be dequeued.
			 */
			if (ret > 0 ||
			    (ret < 0 && (errno == EINVAL || errno == ESRCH))) {
				msg = dequeue(&eventq_m2dev);
				DEBUG(DBG_QUEUE, " <eventq_m2dev: %x%s",
				    ((struct eventdev_reply *)
				    msg->msg)->msg_token,
				    (ret < 0)? " (bad reply)" : "");
				free(msg);
			}
			break;
		default:
			DEBUG(DBG_TRACE, "<dispatch_m2dev (bad msg)");
			msg = dequeue(&eventq_m2dev);
			free(msg);
			break;
	}

	if (queue_peek(&eventq_m2dev) || msg_pending(fd))
		event_add(ev_info->ev_m2dev, NULL);

	DEBUG(DBG_TRACE, "<dispatch_m2dev");
}

static void
dispatch_dev2m(int fd, short event __used, void *arg)
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
		if ((msg = get_event(fd)) == NULL)
			break;
		hdr = (struct eventdev_hdr *)msg->msg;

		DEBUG(DBG_QUEUE, " >dev2m: %x %c", hdr->msg_token,
		    (hdr->msg_flags & EVENTDEV_NEED_REPLY)  ? 'R' : 'N');

		/* we shortcut and ack events for our own children */
		if ((hdr->msg_flags & EVENTDEV_NEED_REPLY) &&
		    (hdr->msg_pid == (u_int32_t)se_pid ||
		     hdr->msg_pid == (u_int32_t)policy_pid
		     || hdr->msg_pid == (u_int32_t)logger_pid)) {

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

		if ((hdr->msg_flags & EVENTDEV_NEED_REPLY) ||
		    (hdr->msg_source == ANOUBIS_SOURCE_PROCESS) ||
		    (hdr->msg_source == ANOUBIS_SOURCE_SFSEXEC) ||
		    (hdr->msg_source == ANOUBIS_SOURCE_IPC)) {

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
	if (terminate) {
		event_del(ev_info->ev_dev2m);
		ev_info->ev_dev2m = NULL;
		if (terminate < 2)
			terminate = 2;
		event_add(ev_info->ev_m2p, NULL);
		event_add(ev_info->ev_m2s, NULL);
		event_add(ev_info->ev_m2u, NULL);
	}
	DEBUG(DBG_TRACE, "<dispatch_dev2m");
}

static void
dispatch_m2u(int fd, short event __used, void *arg)
{
	anoubisd_msg_t			*msg;
	struct event_info_main		*ev_info = arg;
	int				 ret;
	int				 type = 0;

	DEBUG(DBG_TRACE, ">dispatch_m2u");

	msg = queue_peek(&eventq_m2u);
	if (msg)
		type = msg->mtype;
	ret = send_msg(fd, msg);

	if (msg && ret != 0) {
		DEBUG(DBG_QUEUE, " eventq_m2u: Message type %d", type);
		dequeue(&eventq_m2u);
		/* In case of success send_msg freed the message. */
		if (ret < 0) {
			log_warnx(" eventq_m2u: Dropping Message");
			free(msg);
		}
	}
	/* Write was not successful: Check if we lost one of our childs. */
	if (ret <= 0)
		sighandler(SIGCHLD, 0, NULL);

	/* If the queue is not empty, to be called again */
	if (queue_peek(&eventq_m2u) || msg_pending(fd))
		event_add(ev_info->ev_m2u, NULL);
	else if (terminate >= 2) {
		shutdown(fd, SHUT_WR);
	}

	DEBUG(DBG_TRACE, "<dispatch_m2u");
}

static void
dispatch_sfs_update_all(anoubisd_msg_t *msg)
{
	anoubisd_sfs_update_all_t	*umsg;
	size_t				 plen;
	char				*path;
	int				 ret;

	/*
	 * Validate the message format and extract path/checksum. The
	 * Pathname must have space for at least one byte.
	 */
	if ((size_t)msg->size < sizeof(*msg) + sizeof(*umsg)
	    + ANOUBIS_CS_LEN + 1)
		goto bad;
	umsg = (anoubisd_sfs_update_all_t *)msg->msg;
	if (umsg->cslen != ANOUBIS_CS_LEN)
		goto bad;
	plen = msg->size - sizeof(*msg) - sizeof(*umsg) - ANOUBIS_CS_LEN;
	path = umsg->payload + umsg->cslen;
	/* Forcfully NUL terminate the path name. */
	path[plen-1] = 0;

	/*
	 * Update checksums on disk.
	 */
	ret = sfs_update_all(path, (u_int8_t*)umsg->payload, ANOUBIS_CS_LEN);
	if (ret < 0)
		log_warnx("Cannot update checksums during update");
	return;
bad:
	log_warnx(" dispatch_sfs_update_all: Malformed message");
	return;
}

static void
dispatch_u2m(int fd, short event __used, void *arg)
{
	struct event_info_main		*ev_info = arg;
	anoubisd_msg_t			*msg;

	DEBUG(DBG_TRACE, ">dispatch_u2m");
	for (;;) {
		if ((msg = get_msg(fd)) == NULL)
			break;
		DEBUG(DBG_QUEUE, " >u2m: msg type=%d", msg->mtype);
		switch(msg->mtype) {
		case ANOUBISD_MSG_UPGRADE:
			enqueue(&eventq_m2p, msg);
			event_add(ev_info->ev_m2p, NULL);
			break;
		case ANOUBISD_MSG_SFS_UPDATE_ALL:
			dispatch_sfs_update_all(msg);
			free(msg);
			break;
		default:
			free(msg);
			DEBUG(DBG_TRACE, "<dispatch_p2m (bad msg)");
		}

		DEBUG(DBG_TRACE, "<dispatch_p2m (loop)");
	}
	if (msg_eof(fd))
		event_del(ev_info->ev_u2m);
	DEBUG(DBG_TRACE, "<dispatch_u2m");
}

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
#include "version.h"

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
#include <ctype.h>
#include <unistd.h>
#include <paths.h>

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
#include <anoubis_sig.h>
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

static unsigned long	upgraded_files = 0;

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

static void	reconfigure(struct event_info_main *);
static void	init_root_key(char *, struct event_info_main *);

FILE	    *pidfp;
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

static int
read_sfsversion(void)
{
	FILE	*fp;
	int ret, err, version;

	fp = fopen(ANOUBISD_SFS_TREE_VERSIONFILE, "r");
	if (!fp) {
		if (errno == ENOENT)
			return 0;
		return -errno;
	}

	ret = fscanf(fp, "%d\n", &version);
	err = errno;
	/* The policy engine needs access to this file. */
	if (fchown(fileno(fp), 0, anoubisd_gid) < 0
	    || fchmod(fileno(fp), 0640) < 0) {
		log_warn("Cannot modify owner/permissions on "
		    ANOUBISD_SFS_TREE_VERSIONFILE);
	}
	fclose(fp);
	if (ret != 1)
		return -err;
	if (version < 0)
		return -EINVAL;
	return version;
}

static int
is_sfstree_empty()
{
	struct dirent	*dirent;
	int		 empty = 1;
	DIR		*dirh  = opendir(SFS_CHECKSUMROOT);
	if (dirh == NULL) {
		if (errno == ENOENT)
			return 1;
		else
			return -errno;
	}
	while ((dirent = readdir(dirh)) != NULL) {
		if (strcmp(dirent->d_name, ".")  != 0 ||
		    strcmp(dirent->d_name, "..") != 0) {
			empty = 0;
			break;
		}
	}
	closedir(dirh);
	return empty;
}

static int
write_sfsversion()
{
	FILE *fp = fopen(ANOUBISD_SFS_TREE_VERSIONFILE, "w");
	if (!fp ||
	    fprintf(fp, "%d\n", ANOUBISD_SFS_TREE_FORMAT_VERSION) < 0 ||
	    fclose(fp) == EOF) {
		return -errno;
	}
	/* The policy engine needs access to this file. */
	if (fchown(fileno(fp), 0, anoubisd_gid) < 0
	    || fchmod(fileno(fp), 0640) < 0) {
		log_warn("Cannot modify owner/permissions on "
		    ANOUBISD_SFS_TREE_VERSIONFILE);
	}
	return ANOUBISD_SFS_TREE_FORMAT_VERSION;
}

int
main(int argc, char *argv[])
/*@globals undef eventq_m2p, undef eventq_m2s, undef eventq_m2dev@*/
{

	int			pipes[PIPE_MAX * 2];
	int			loggers[PROC_LOGGER * 2];
	int			logfd, policyfd, sessionfd, upgradefd;
	struct event		ev_sigterm, ev_sigint, ev_sigquit, ev_sighup,
				    ev_sigchld;
	struct event		ev_s2m, ev_p2m, ev_u2m, ev_dev2m;
	struct event		ev_m2s, ev_m2p, ev_m2u, ev_m2dev;
	struct event		ev_timer;
	/*@observer@*/
	struct event_info_main	ev_info;
	int			p;
	sigset_t		mask;
	struct timeval		tv;
	struct passwd		*pw;
	int			sfsversion;
	struct sigaction	act;
#ifdef LINUX
	int			 dazukofd;
	FILE			*omitfp;
#endif

	/* Ensure that fds 0, 1 and 2 are open or directed to null */
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
		fd = open(_PATH_DEV "anoubis", O_RDWR);
		if (fd < 0)
			early_err(2, "Could not open " _PATH_DEV "anoubis");

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

	/* Get anoubisd gid early. This is used by read/write_sfsversion */
	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		early_errx(1, "User " ANOUBISD_USER " does not exist");
	if (pw->pw_gid == 0)
		early_errx(1, "Group ID of " ANOUBISD_USER " must not be 0");
	anoubisd_gid = pw->pw_gid;

	sfsversion = read_sfsversion();
	if (sfsversion == 0) {
		/*
		 * check if sfs tree is empty. If yes, everything is ok and
		 * we can just set the sfs.version. If no, the upgrade script
		 * needs to run.
		 */
		int empty = is_sfstree_empty();
		if (empty < 0) {
			early_err(5, "Could not read " SFS_CHECKSUMROOT);
		}
		if (empty) {
			if (anoubisd_config.opts & ANOUBISD_OPT_NOACTION) {
				/* No further action required */
				exit(0);
			}
			sfsversion = write_sfsversion();
			if (sfsversion < 0) {
				early_err(5, "Could not write "
				    ANOUBISD_SFS_TREE_VERSIONFILE);
			}
		}
	}

	if (sfsversion < 0) {
		early_err(5, "Could not read " ANOUBISD_SFS_TREE_VERSIONFILE);
	} else if (sfsversion > ANOUBISD_SFS_TREE_FORMAT_VERSION) {
		char *msg;
		if (asprintf(&msg, "Layout version of " SFS_CHECKSUMROOT
		    " is %i, but we only support %i.\n"
		    "You need to upgrade anoubisd.",
		    sfsversion, ANOUBISD_SFS_TREE_FORMAT_VERSION) == -1)
			msg = NULL;
		early_errx(6, msg);
	} else if (sfsversion < ANOUBISD_SFS_TREE_FORMAT_VERSION) {
		char *msg;
		if (asprintf(&msg, "Layout version of " SFS_CHECKSUMROOT
		    " is %i, but we only support %i.\n"
		    "You need to run the upgrade script.",
		    sfsversion, ANOUBISD_SFS_TREE_FORMAT_VERSION) == -1)
			msg = NULL;
		early_errx(6, msg);
	}

	if (anoubisd_config.opts & ANOUBISD_OPT_NOACTION) {
		/* Exit at this point, no further action required.
		 * Note that there is another exit(0) above in the
		 * case of an empty sfs tree.
		 * */
		exit(0);
	}

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = segvhandler;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGSEGV, &act, NULL) < 0)
		early_errx(1, "Sigaction failed");

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

	for (p = 0; p < PROC_LOGGER; p += 2) {
		if (socketpair(AF_UNIX, SOCK_STREAM,
		    PF_UNSPEC, &loggers[p]) == -1)
			early_errx(1, "socketpair");
	}

	for (p = 0; p < PIPE_MAX; p += 2) {
		if (socketpair(AF_UNIX, SOCK_STREAM,
		    PF_UNSPEC, &pipes[p]) == -1)
			early_errx(1, "socketpair");
	}

	logger_pid = logger_main(pipes, loggers);

	(void)event_init();

	log_init(loggers[anoubisd_process]);
	DEBUG(DBG_TRACE, "logger_pid=%d", logger_pid);

	log_info("master started (pid %d)", getpid());
	log_info("Package: " PACKAGE_VERSION " (build " PACKAGE_BUILD ")");
	DEBUG(DBG_TRACE, "debug=%x", debug_flags);
	master_pid = getpid();
	DEBUG(DBG_TRACE, "master_pid=%d", master_pid);
	save_pid(pidfp, master_pid);

	se_pid = session_main(pipes, loggers);
	DEBUG(DBG_TRACE, "session_pid=%d", se_pid);
	policy_pid = policy_main(pipes, loggers);
	DEBUG(DBG_TRACE, "policy_pid=%d", policy_pid);
	upgrade_pid = upgrade_main(pipes, loggers);
	DEBUG(DBG_TRACE, "upgrade_pid=%d", upgrade_pid);

	policyfd = sessionfd = upgradefd = logfd = -1;
	SWAP(policyfd, pipes[PIPE_MAIN_POLICY]);
	SWAP(sessionfd, pipes[PIPE_MAIN_SESSION]);
	SWAP(upgradefd, pipes[PIPE_MAIN_UPGRADE]);
	SWAP(logfd, loggers[anoubisd_process]);
	cleanup_fds(pipes, loggers);

	/* Load Public Keys */
	cert_init(0);

	setproctitle("master");
#ifdef LINUX
	dazukofd = dazukofs_ignore();

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
	msg_init(sessionfd, "m2s");
	msg_init(policyfd, "m2p");
	msg_init(upgradefd, "m2u");

	/*
	 * Open eventdev to communicate with the kernel.
	 * This needs to be done before the event_add for it,
	 * because that causes an event.
	 */
	eventfds[0] = open(_PATH_DEV "eventdev", O_RDWR);
	if (eventfds[0] < 0) {
		log_warn("Could not open " _PATH_DEV "eventdev");
		main_shutdown(1);
	}
	msg_init(eventfds[0], "m2dev");

	/* session process */
	event_set(&ev_m2s, sessionfd, EV_WRITE, dispatch_m2s, &ev_info);
	event_set(&ev_s2m, sessionfd, EV_READ | EV_PERSIST, dispatch_s2m,
	    &ev_info);
	event_add(&ev_s2m, NULL);

	/* policy process */
	event_set(&ev_m2p, policyfd, EV_WRITE, dispatch_m2p, &ev_info);
	event_set(&ev_p2m, policyfd, EV_READ | EV_PERSIST, dispatch_p2m,
	    &ev_info);
	event_add(&ev_p2m, NULL);

	/* upgrade process */
	event_set(&ev_m2u, upgradefd, EV_WRITE, dispatch_m2u, &ev_info);
	event_set(&ev_u2m, upgradefd, EV_READ | EV_PERSIST, dispatch_u2m,
	    &ev_info);
	event_add(&ev_u2m, NULL);

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
	ev_info.ev_m2u = &ev_m2u;
	ev_info.ev_u2m = &ev_u2m;

	/*
	 * Open event device to communicate with the kernel.
	 */
	eventfds[1] = open(_PATH_DEV "anoubis", O_RDWR);
	if (eventfds[1] < 0) {
		log_warn("Could not open " _PATH_DEV "anoubis");
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
	/* Note that we keep the anoubis device open for subsequent ioctls. */

	/* Five second timer for statistics ioctl */
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	ev_info.anoubisfd = eventfds[1];
	ev_info.ev_timer = &ev_timer;
	ev_info.tv = &tv;
	evtimer_set(&ev_timer, &dispatch_timer, &ev_info);
	event_add(&ev_timer, &tv);

	init_root_key(NULL, &ev_info);

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

#ifdef LINUX
int
dazukofs_ignore(void)
{
	int fd = -1;

	fd = open(_PATH_DEV "dazukofs.ign", O_RDONLY);
	if ((fd == -1) && (errno != ENOENT))
		log_warn("Could not open dazukofs: %s", strerror(errno));
	return(fd);
}
#endif

static void
sanitise_stdfd(void)
{
	int nullfd, dupfd;

	if ((nullfd = dupfd = open(_PATH_DEVNULL, O_RDWR)) == -1)
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

void
cleanup_fds(int pipes[], int loggers[])
{
	unsigned int p;

	/* only the main process should lock the pidfile */
	if (anoubisd_process != PROC_MAIN)
		fclose(pidfp);

	/* close all the remaining valid loggers and pipes */
	for (p=0; p < PIPE_MAX; p++) {
		if (pipes[p] == -1)
			continue;
		msg_release(pipes[p]);
		close(pipes[p]);
	}

	for (p=0; p < PROC_LOGGER; p++) {
		if (loggers[p] == -1)
			continue;
		msg_release(loggers[p]);
		close(loggers[p]);
	}
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
		init_root_key(NULL, info);
	}
}

static void
dispatch_m2s(int fd, short event __used, /*@dependent@*/ void *arg)
{
	anoubisd_msg_t			*msg;
	struct event_info_main		*ev_info = arg;
	struct eventdev_hdr		*ev_hdr;
	int				 ret;
	eventdev_token			 token = 0;

	DEBUG(DBG_TRACE, ">dispatch_m2s");

	msg = queue_peek(&eventq_m2s);
	if (msg) {
		ev_hdr = (struct eventdev_hdr *)msg->msg;
		token = ev_hdr->msg_token;
	}
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
send_sfscache_invalidate_key(const char *path, const u_int8_t *keyid,
    int keylen, struct event_info_main *ev_info)
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

static void
send_upgrade_ok(int value, struct event_info_main *ev_info)
{
	struct anoubisd_msg		*msg;
	struct anoubisd_msg_upgrade	*upg;

	msg = msg_factory(ANOUBISD_MSG_UPGRADE,
	    sizeof(struct anoubisd_msg_upgrade) + 1);
	if (!msg) {
		master_terminate(ENOMEM);
		return;
	}
	upg = (struct anoubisd_msg_upgrade*)msg->msg;
	upg->upgradetype = ANOUBISD_UPGRADE_OK;
	upg->chunksize = 1;
	upg->chunk[0] = !!value;
	enqueue(&eventq_m2p, msg);
	event_add(ev_info->ev_m2p, NULL);
}

static void
send_upgrade_notification(struct event_info_main *ev_info)
{
	struct anoubisd_msg		*msg;
	struct anoubisd_msg_upgrade	*upg;

	msg = msg_factory(ANOUBISD_MSG_UPGRADE,
	    sizeof(struct anoubisd_msg_upgrade));
	if (!msg) {
		master_terminate(ENOMEM);
		return;
	}
	upg = (struct anoubisd_msg_upgrade*)msg->msg;
	upg->upgradetype = ANOUBISD_UPGRADE_NOTIFY;
	upg->chunksize = upgraded_files;
	enqueue(&eventq_m2s, msg);
	event_add(ev_info->ev_m2s, NULL);
}

static void
init_root_key(char *passphrase, struct event_info_main *ev_info)
{
	struct cert	*cert;

	cert = cert_get_by_uid(0);
	DEBUG(DBG_UPGRADE, ">init_root_key: passphrase: %s cert: %p",
	    passphrase?"yes":"no", cert);
	/*
	 * Log error conditions if a passphrase is given, i.e. we deal
	 * with a direct request from the user.
	 */
	if (passphrase) {
		if (!cert) {
			log_warnx("Got Passphrase but no public rootkey "
			    "is configured");
		} else if (anoubisd_config.rootkey == NULL) {
			log_warnx("Passphrase but no private rootkey "
			    "is configured");
		}
	}
	/*
	 * If no rootkey is configured, upgrades are allowed but no
	 * signature upgrades will be performed. Any previously configured
	 * private key must be delted.
	 */
	if (anoubisd_config.rootkey == NULL) {
		/* Clear existing rootkey if any. */
		if (cert)
			cert_load_priv_key(cert, NULL, NULL);
		/* Update is allowed but will not do signature upgrades */
		send_upgrade_ok(1, ev_info);
		return;
	}
	/*
	 * If a rootkey is configured but root does not have a public key
	 * we ignore the root key and allow upgrades.
	 */
	if (!cert) {
		log_warnx("Ignoring rootkey because root has no public key.");
		send_upgrade_ok(1, ev_info);
		DEBUG(DBG_UPGRADE, "<init_root_key: no rootkey cert: %p %p",
		    cert, cert?cert->privkey:NULL);
		return;
	}
	/*
	 * Try to load the new private key into the root certificate. If
	 * a rootkey is required but it failed to load upgrades are denied.
	 */
	cert_load_priv_key(cert, anoubisd_config.rootkey, passphrase);
	if (anoubisd_config.rootkey_required && cert->pubkey == NULL) {
		log_warnx("Upgrades will be denied due to missing root key");
		send_upgrade_ok(0, ev_info);
		return;
	}
	/* Upgrades are allowed. */
	send_upgrade_ok(1, ev_info);
	DEBUG(DBG_UPGRADE, "<init_root_key: cert: %p %p", cert,
	    cert?cert->privkey:NULL);
}

static anoubisd_msg_t *
create_polreply_msg(u_int64_t token, int payloadlen)
{
	anoubisd_msg_t		*msg;
	anoubisd_reply_t	*reply;
	msg = msg_factory(ANOUBISD_MSG_POLREPLY,
	    sizeof(anoubisd_reply_t) + payloadlen);
	if (!msg) {
		master_terminate(ENOMEM);
		return 0;
	}
	reply = (anoubisd_reply_t *)msg->msg;
	memset(reply, 0, sizeof(anoubisd_reply_t));
	reply->flags = 0;
	reply->len = payloadlen;
	reply->token = token;
	return msg;
}

static void
sfs_checksumop_list(struct sfs_checksumop *csop, u_int64_t token,
    struct event_info_main *ev_info)
{
	char			*sfs_path = NULL;
	int			 error = EINVAL;
	anoubisd_msg_t		*msg = NULL;
	anoubisd_reply_t	*reply = NULL;
	DIR			*sfs_dir = NULL;
	struct dirent		*entry;
	int			 offset;
	int			 found = 0;

	if (csop->op != ANOUBIS_CHECKSUM_OP_GENERIC_LIST) {
		error = EINVAL;
		goto out;
	}
	error = -convert_user_path(csop->path, &sfs_path,
	    !(csop->listflags & ANOUBIS_CSUM_WANTIDS));
	if (error)
		goto out;
	sfs_dir = opendir(sfs_path);
	if (sfs_dir == NULL) {
		error = errno;
		if (error == ENOENT || error == ENOTDIR)
			error = 0;
		goto out;
	}
	msg = create_polreply_msg(token, 8000);
	reply = (anoubisd_reply_t *)msg->msg;
	reply->flags |= POLICY_FLAG_START;
	offset = 0;

	while((entry = readdir(sfs_dir)) != NULL) {
		char		*tmp;
		int		 tmplen;
		if (!sfs_wantentry(csop, sfs_path, entry->d_name))
			continue;
		found = 1;
		tmp = remove_escape_seq(entry->d_name);
		tmplen = strlen(tmp) + 1;
		if (offset + tmplen > reply->len) {
			reply->len = offset;
			msg_shrink(msg, sizeof(anoubisd_reply_t) + offset);
			enqueue(&eventq_m2s, msg);
			event_add(ev_info->ev_m2s, NULL);
			msg = create_polreply_msg(token, 8000);
			reply = (anoubisd_reply_t*)msg->msg;
			offset = 0;
		}
		if (offset + tmplen > reply->len) {
			free(tmp);
			error = ENAMETOOLONG;
			goto out;
		}
		memcpy(reply->msg + offset, tmp, tmplen);
		offset += tmplen;
		free(tmp);
	}
	closedir(sfs_dir);
	reply->len = offset;
	reply->flags |= POLICY_FLAG_END;
	msg_shrink(msg, sizeof(anoubisd_reply_t) + offset);
	enqueue(&eventq_m2s, msg);
	event_add(ev_info->ev_m2s, NULL);
	if (!found) {
		sfs_remove_index(sfs_path, csop);
	}
	free(sfs_path);
	return;
out:
	if (sfs_dir)
		closedir(sfs_dir);
	if (sfs_path)
		free(sfs_path);
	if (msg) {
		msg_shrink(msg, sizeof(anoubisd_reply_t));
		reply->len = 0;
		reply->flags |= POLICY_FLAG_END;
	} else {
		msg = create_polreply_msg(token, 0);
		reply = (anoubisd_reply_t *)msg->msg;
		reply->flags = (POLICY_FLAG_START | POLICY_FLAG_END);
	}
	reply->reply = error;
	enqueue(&eventq_m2s, msg);
	event_add(ev_info->ev_m2s, NULL);
}

static void
dispatch_checksumop(anoubisd_msg_t *msg, struct event_info_main *ev_info)
{
	struct anoubis_msg		 rawmsg;
	anoubisd_msg_checksum_op_t	*msg_comm;
	struct sfs_checksumop		 csop;
	anoubisd_reply_t		*reply;
	void				*sigbuf = NULL;
	int				 siglen = 0;
	int				 err = -EFAULT;

	msg_comm = (anoubisd_msg_checksum_op_t *)msg->msg;
	rawmsg.length = msg_comm->len;
	rawmsg.u.buf = msg_comm->msg;
	if (sfs_parse_checksumop(&csop, &rawmsg, msg_comm->uid) < 0) {
		err = -EINVAL;
		goto out;
	}

	switch (csop.op) {
	case _ANOUBIS_CHECKSUM_OP_LIST:
	case _ANOUBIS_CHECKSUM_OP_LIST_ALL:
	case _ANOUBIS_CHECKSUM_OP_UID_LIST:
	case _ANOUBIS_CHECKSUM_OP_KEYID_LIST:
	case ANOUBIS_CHECKSUM_OP_GENERIC_LIST:
		sfs_checksumop_list(&csop, msg_comm->token, ev_info);
		return;
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case _ANOUBIS_CHECKSUM_OP_GETSIG_OLD:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
	case _ANOUBIS_CHECKSUM_OP_GET_OLD:
	case ANOUBIS_CHECKSUM_OP_GET2:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
	case ANOUBIS_CHECKSUM_OP_DEL:
		err = sfs_checksumop(&csop, &sigbuf, &siglen);
		if (err < 0)
			goto out;
		break;
	default:
		err = -EINVAL;
		goto out;
	}

	switch (csop.op) {
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case ANOUBIS_CHECKSUM_OP_DEL:
		send_sfscache_invalidate_uid(csop.path, csop.uid, ev_info);
		break;
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
		send_sfscache_invalidate_key(csop.path, csop.keyid,
		    csop.idlen, ev_info);
	}
out:
	msg = msg_factory(ANOUBISD_MSG_POLREPLY,
	    sizeof(anoubisd_reply_t) + siglen);
	if (!msg) {
		master_terminate(ENOMEM);
		return;
	}
	reply = (anoubisd_reply_t*)msg->msg;
	reply->token = msg_comm->token;
	reply->timeout = 0;
	reply->reply = -err;
	reply->flags = POLICY_FLAG_START | POLICY_FLAG_END;
	reply->len = siglen;
	if (siglen) {
		memcpy(reply->msg, sigbuf, siglen);
		free(sigbuf);
	}
	enqueue(&eventq_m2s, msg);
	DEBUG(DBG_QUEUE, " >eventq_m2s: %llx",
	    (unsigned long long)reply->token);
	event_add(ev_info->ev_m2s, NULL);
}

static void
dispatch_passphrase(anoubisd_msg_t *msg, struct event_info_main *ev_info)
{
	anoubisd_msg_passphrase_t	*pass;
	int				 len;

	pass = (anoubisd_msg_passphrase_t *)msg->msg;
	len = msg->size - sizeof(anoubisd_msg_t);
	if (len < (int)sizeof(anoubisd_msg_passphrase_t) + 1)
		return;
	pass->payload[len-1] = 0;
	init_root_key(pass->payload, ev_info);
}

static void
dispatch_s2m(int fd, short event __used, void *arg)
{
	/*@dependent@*/
	anoubisd_msg_t *msg;
	struct event_info_main *ev_info = (struct event_info_main*)arg;
	struct eventdev_hdr *ev_hdr;

	DEBUG(DBG_TRACE, ">dispatch_s2m");

	for (;;) {

		if ((msg = get_msg(fd)) == NULL)
			break;
		switch(msg->mtype) {
		case ANOUBISD_MSG_CHECKSUM_OP:
			dispatch_checksumop(msg, ev_info);
			break;
		case ANOUBISD_MSG_PASSPHRASE:
			dispatch_passphrase(msg, ev_info);
			break;
		default:
			ev_hdr = (struct eventdev_hdr *)msg->msg;
			DEBUG(DBG_QUEUE, " >s2m: %x", ev_hdr->msg_token);

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
	struct eventdev_hdr		*ev_hdr;
	int				 ret;
	eventdev_token			 token = 0;

	DEBUG(DBG_TRACE, ">dispatch_m2p");

	msg = queue_peek(&eventq_m2p);
	if (msg) {
		ev_hdr = (struct eventdev_hdr *)msg->msg;
		token = ev_hdr->msg_token;
	}
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
	struct eventdev_reply	*ev_rep;
	anoubisd_msg_t		*msg;

	DEBUG(DBG_TRACE, ">dispatch_p2m");

	for (;;) {
		if ((msg = get_msg(fd)) == NULL)
			break;
		switch(msg->mtype) {
		case ANOUBISD_MSG_EVENTREPLY:
			ev_rep = (struct eventdev_reply *)msg->msg;
			DEBUG(DBG_QUEUE, " >p2m: eventdev msg token=%x",
			    ev_rep->msg_token);
			enqueue(&eventq_m2dev, msg);
			DEBUG(DBG_QUEUE, " >eventq_m2dev: %x",
			    ev_rep->msg_token);
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
	struct eventdev_reply *ev_rep;
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
				ev_rep = (struct eventdev_reply *)msg->msg;
				DEBUG(DBG_QUEUE, " <eventq_m2dev: %x%s",
				    ev_rep->msg_token,
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
	struct eventdev_reply * rep;
	/*@dependent@*/
	anoubisd_msg_t *msg;
	/*@dependent@*/
	anoubisd_msg_t *msg_reply;
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
			rep = (struct eventdev_reply *)msg_reply->msg;
			DEBUG(DBG_QUEUE, " >eventq_m2dev: %x",
				rep->msg_token);
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
dispatch_sfs_update_all(anoubisd_msg_t *msg, struct event_info_main *ev_info)
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
	if (anoubisd_config.rootkey) {
		struct cert	*cert = cert_get_by_uid(0);

		DEBUG(DBG_UPGRADE, " signature update for %s, cert %p %p",
		    path, cert, cert?cert->privkey:NULL);
		if (cert && cert->privkey) {
			ret = sfs_update_signature(path, cert,
			    (u_int8_t*)umsg->payload, ANOUBIS_CS_LEN);
			if (ret < 0)
				log_warnx("Cannot update root signature "
				    "during update");
		} else if (anoubisd_config.rootkey_required) {
			log_warnx("ERROR: Upgrade in progress but rootkey "
			    "is not available");
			send_upgrade_ok(0, ev_info);
		}
	}
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
		case ANOUBISD_MSG_UPGRADE: {
			struct anoubisd_msg_upgrade	*umsg;

			umsg = (struct anoubisd_msg_upgrade *)msg->msg;
			if (umsg->upgradetype == ANOUBISD_UPGRADE_END) {
				send_upgrade_notification(ev_info);
				upgraded_files = 0;
			}
			enqueue(&eventq_m2p, msg);
			event_add(ev_info->ev_m2p, NULL);
			break;
		}
		case ANOUBISD_MSG_SFS_UPDATE_ALL:
			dispatch_sfs_update_all(msg, ev_info);
			upgraded_files++;
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

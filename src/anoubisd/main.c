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
#include <sys/resource.h>

#include <arpa/inet.h>
#include <netinet/in.h>

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
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <openssl/rand.h>

#ifdef LINUX
#include <bsdcompat.h>
#include <linux/anoubis_sfs.h>
#include <openssl/sha.h>
#include <attr/xattr.h>
#else
#include <dev/anoubis_sfs.h>
#include <sha2.h>
#endif

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

#ifndef OPENBSD
#include <ucontext.h>
#endif

/* on glibc 2.6+, event.h uses non C89 types :/ */
#include <event.h>
#include <anoubis_msg.h>
#include <anoubis_sig.h>
#include <anoubis_errno.h>
#include "anoubisd.h"
#include "aqueue.h"
#include "amsg.h"
#include "sfs.h"
#include "cert.h"
#include "cfg.h"
#include <anoubis_alloc.h>

/* Prototypes. */
static void	main_shutdown(int) __dead;
static void	sanitise_stdfd(void);

static void	sighandler(int, short, void *);
static void	dispatch_m2s(int, short, void *);
static void	dispatch_s2m(int, short, void *);
static void	dispatch_m2p(int, short, void *);
static void	dispatch_p2m(int, short, void *);
static void	dispatch_m2dev(int, short, void *);
static void	dispatch_dev2m(int, short, void *);
static void	dispatch_m2u(int, short, void *);
static void	dispatch_u2m(int, short, void *);
static void	reconfigure(void);
static void	init_root_key(char *);

/**
 * This variable identifies the anoubisd daemon process.
 */
enum anoubisd_process_type	anoubisd_process;

/**
 * The current state of the termination. Things to do during termination
 * are:
 * - Tell the kernel that no more events events should be sent.
 * - Remove all signal handlers from the event loop.
 * - Send signals to the child process (all except logger).
 * - Set terminate to 1.
 * Once terminate is set to 1:
 * - Read remaining events from the kernel and dispatch them to child
 *   processes.
 * - Set terminate to 2 once EOF is received from the kernel event device.
 * Once terminate is set to 2:
 * - Close write ends of pipes to child processes once the queue for
 *   these processes is empty and remove associated events.
 * - Wait for the peer processes to close their ends of the pipes. Once
 *   EOF is received while reading from one of these pipes, close them
 *   and remove the remaining events.
 * The process terminates when there is no remaining event in the event
 * loop.
 */
static int	terminate = 0;

/**
 * File descriptors for the anoubis devices.
 * Index zero is a handle to our eventdev queue.
 * Index one is a handle to the anoubis device.
 */
static int	eventfds[2];

/**
 * The anoubis daemon configuration.
 */
struct anoubisd_config	anoubisd_config;

pid_t		master_pid = 0;		/** The pid of the daemon master */
pid_t		se_pid = 0;		/** The pid of the session engine */
pid_t		logger_pid = 0;		/** The pid of the logger process */
pid_t		policy_pid = 0;		/** The pid of the policy engine */
pid_t		upgrade_pid = 0;	/** The pid of the upgrade process */

u_int32_t	debug_flags = 0;	/** Current debug flags */
u_int32_t	debug_stderr = 0;	/** True if debugging is on stderr */
gid_t		anoubisd_gid;		/** Group ID of the anoubisd user */

/**
 * The name of the binary executed by the current process.
 */
extern char	*__progname;

/**
 * The string that is stored here will be prepended to log and debug
 * messages. It identifies the process that sends a paricular message.
 */
char		*logname;

/**
 * The interface version of the anoubis kernel module as returned by
 * the ANOUBIS_GETVERSION ioctl.
 */
unsigned long	 version;

/**
 * The total number of files that were upgraded in the current
 * upgrade run.
 */
static unsigned long	upgraded_files = 0;

/**
 * Message queue for writes of the master to the policy engine.
 */
static Queue eventq_m2p;

/**
 * Message queue for writes of from the master to the session engine.
 */
static Queue eventq_m2s;

/**
 * Message queue for writes of the master to the upgrade process.
 */
static Queue eventq_m2u;

/**
 * Message queue for write of the master to the eventdev device.
 */
static Queue eventq_m2dev;


/**
 * A file handle to the PID file. This file handle remains open
 * while the process runs and it holds a lock on the pid file. This
 * allows a subsequent anoubisd process to determine if the lock file
 * is stale.
 */
FILE	    *pidfp;

#ifdef LINUX

/**
 * Candidate files for the omit pid logic of killall5. A process can
 * use these files to tell killall5 that certain processes should not
 * be killed during shutdown. Each of these locations is tried in turn
 * and the first location that is valid is actually used.
 */
static char *omit_pid_files[] = {
	"/lib/init/rw/sendsigs.omit.d/" PACKAGE_DAEMON,
	"/var/run/sendsigs.omit.d/" PACKAGE_DAEMON,
	NULL
};

/**
 * The actual omit pid file for killall5.
 */
static char *omit_pid_file = NULL;

#endif

/**
 * Signal handler for segmentation faults. As opposed to other "signal"
 * handlers, this handler is not called from normal process context by
 * libevent but directly as a signal handler.
 * It sends some useful information to syslog and tries to write a
 * crash log and exits.
 *
 * @param sig The signal number.
 * @param info The signal info if any.
 * @param uc The context of the call. This can be used to generate a
 *     readable backtrace.
 * @return None.
 */
void
segvhandler(int sig, siginfo_t *info, void *uc)
{
	char	 buf[2048];
	char	*p = buf;
	int	 i, n, nreg;
	void	**ucp = uc;
	int	 fd;
#if HAVE_BACKTRACE
	void	*btbuf[30];
	char	**btsymbols;
	int c;
#endif

	/*
	 * We don't have syslog in all processes.
	 * Try to write a crash log, too.
	 */
	fd = open(PACKAGE_POLICYDIR "/crash.log", O_WRONLY|O_APPEND|O_CREAT,
	    S_IRUSR|S_IWUSR|S_IRGRP);
	if (fd == -1)
		fd = open("/crash.log", O_WRONLY|O_APPEND|O_CREAT,
		    S_IRUSR|S_IWUSR|S_IRGRP);
	if (fd == -1)
		fd = 0;

	sprintf(p, PACKAGE_DAEMON "[%d]: SIGNAL %d at %p ucontext:%n",
	    getpid(), sig, info?info->si_addr:(void*)-1, &n);
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
	if (fd)
		i = write(fd, buf, strlen(buf));
	syslog(LOG_ALERT, "%s", buf);

#ifdef HAVE_BACKTRACE
	n = backtrace(btbuf, sizeof(btbuf));
	btsymbols = backtrace_symbols(btbuf, n);
	for (i = 0; i < n ; i++) {
		c = snprintf(buf, sizeof(buf), "%p (%s)\n", btbuf[i],
		    btsymbols ? btsymbols[i] : "(unknown)");
		syslog(LOG_ALERT, "%s", buf);
		if (fd)
		    c = write(fd, buf, c);
	}
#endif

	if (fd)
		close(fd);
	exit(126);
}


/**
 * This is the signal handler for all signals except SIGSEGV which
 * has its own handler. This is not a real signal handler. Instead
 * libevent catches signals and generates libevent events for each
 * signal received. This handler is called in normal process context
 * from libevent as a response to these events.
 *
 * @param sig The signal.
 * @param event The event type (see libevent).
 * @param The callback argument of for the event handler. This is either
 *     NULL or a struct event_info_main.
 * @return None.
 *
 * NOTE: This handler is sometimes called manually with arg == NULL.
 */
static void
sighandler(int sig, short event __used, void *arg)
{
	int			 die = 0;
	struct event_info_main	*info = arg;

	DEBUG(DBG_TRACE, ">sighandler: %d", sig);

	switch (sig) {
	case SIGTERM:
	case SIGINT: {
		int			 i;
		sigset_t		 mask;

		if (terminate < 1)
			terminate = 1;
		anoubisd_scanners_detach();
		log_warnx(PACKAGE_DAEMON ": Shutdown requested by signal");
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
			if (se_pid)
				kill(se_pid, SIGTERM);
			if (policy_pid)
				kill(policy_pid, SIGTERM);
			if (logger_pid)
				kill(logger_pid, SIGTERM);
			for (i=0; info->ev_sigs[i]; ++i)
				signal_del(info->ev_sigs[i]);
			break;
		}
		log_warnx("Cannot undeclare eventdev queue");
		/* FALLTHROUGH */
	}
	case SIGQUIT:
		die = 1;
	case SIGCHLD: {
		pid_t			 pid;
		int			 status;
		const char		*childname = NULL;

		while (1) {
			childname = NULL;
			pid = waitpid(-1, &status, WNOHANG);
			if (pid < 0 && errno == EINTR)
				continue;
			if (pid <= 0)
				break;
			if (se_pid == pid) {
				se_pid = 0;
				childname = "session engine";
			}
			if (pid == policy_pid) {
				policy_pid = 0;
				childname = "policy engine";
			}
			if (pid == upgrade_pid) {
				upgrade_pid = 0;
				childname = "anoubis upgrade";
			}
			if (pid == logger_pid) {
				logger_pid = 0;
				childname = "anoubis logger";
			}
			if (childname == NULL) {
				if (info)
					anoubisd_scanproc_exit(pid, status,
					    info, &eventq_m2p);
				continue;
			}
			if (WIFEXITED(status))
				log_warnx("Lost child: %s exited", childname);
			if (WIFSIGNALED(status))
				log_warnx("Lost child: %s terminated with "
				    "signal %d", childname, WTERMSIG(status));
			if (!terminate)
				die = 1;
		}
		if (terminate && sig == SIGCHLD && info)
			signal_del(info->ev_sigchld);
		if (die) {
			main_shutdown(0);
			/*NOTREACHED*/
		}
		break;
	}
	case SIGHUP:
		reconfigure();
		break;
	}

	DEBUG(DBG_TRACE, "<sighandler: %d", sig);
}

/**
 * Initialize a singal set for sigation with all signals blocked exepct
 * for a few fatal one that we can handle.
 *
 * @param A pointer to the signal set that should be initialized.
 * @return None.
 */
void anoubisd_defaultsigset(sigset_t *mask)
{
	sigfillset(mask);
	sigdelset(mask, SIGTERM);
	sigdelset(mask, SIGINT);
	sigdelset(mask, SIGQUIT);
	sigdelset(mask, SIGSEGV);
	sigdelset(mask, SIGILL);
	sigdelset(mask, SIGABRT);
	sigdelset(mask, SIGFPE);
	sigdelset(mask, SIGBUS);
}

/**
 * Create and flock the pid file. If this fails the daemon is probably
 * already running.
 *
 * @param None.
 * @return A file handle to the pid file. The caller should write its
 *     pid to the file and keep the file handle open. The flock  lock will
 *     be dropped at exit time.
 */
FILE *
check_pid(void)
{
	FILE *fp;

	fp = fopen(PACKAGE_PIDFILE, "a+");
	if (fp == NULL) {
		log_warn("%s", PACKAGE_PIDFILE);
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

/**
 * Write the given pid to the the file. The file is truncated in the process.
 *
 * @param fp The file handle.
 * @param pid The pid to write. It is formated as a decimal number followed
 *     by a newline.
 * @return None.
 */
static void
save_pid(FILE *fp, pid_t pid)
{
	rewind(fp);
	if (ftruncate(fileno(fp), 0) != 0) {
		log_warn("%s", PACKAGE_PIDFILE);
		fatal("cannot clear pid file");
	}

	fprintf(fp, "%d\n", pid);
	fflush(fp);
}

/**
 * This is the event handler for the timer event. The timer fires every
 * five seconds and reuquest a statistics message from the kernel.
 *
 * @param sig The signal number (see libevent).
 * @param event The type of event that occured (see libevent).
 * @param The event callback data. This is a pointer to struct event_info_main.
 * @return None.
 */
static void
dispatch_timer(int sig __used, short event __used, void * arg)
{
	struct event_info_main	*ev_info = arg;
	static int		 first = 1;
	static int		 warned = 0;

	DEBUG(DBG_TRACE, ">dispatch_timer");

	/*
	 * Simulate a SIGCHLD at the first timer event. This will catch
	 * cases where one of the child process exits before we enter
	 * the event loop in main.
	 */
	if (first) {
		first = 0;
		sighandler(SIGCHLD, 0, NULL);
	}
	if (ioctl(ev_info->anoubisfd, ANOUBIS_REQUEST_STATS, 0) < 0) {
		if (!warned)
			log_warn("Failed to request stats from the kernel");
		warned = 1;
	} else {
		warned = 0;
	}
	event_add(ev_info->ev_timer, ev_info->tv);

	DEBUG(DBG_TRACE, "<dispatch_timer");
}

/**
 * Read the version number of the sfs tree's on disk file format.
 *
 * @param  None.
 * @return The version number (positive) or an error code (negative).
 */
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

/**
 * Check if the on disk sfs tree is empty.
 *
 * @param None.
 * @return Possible return values are:
 *     Positive: The sfs tree does not contain any files.
 *     Zero: The sfs tree exists and contains at least one file.
 *     Negative: An error occured, the return value is a negative error code.
 */
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

/**
 * Write the sfs tree version currently supported by this implementation
 * of the anoubis daemon to the version file. This function overwrites
 * previous contents of the file.
 *
 * @param None.
 * @return The sfs tree version (positive) or a negative error code.
 */
static int
write_sfsversion()
{
	FILE	*fp = fopen(ANOUBISD_SFS_TREE_VERSIONFILE, "w");

	if (fp == NULL)
		return -errno;
	fprintf(fp, "%d\n", ANOUBISD_SFS_TREE_FORMAT_VERSION);
	fflush(fp);
	/* The policy engine needs access to this file. */
	if (fchown(fileno(fp), 0, anoubisd_gid) < 0
	    || fchmod(fileno(fp), 0640) < 0) {
		log_warn("Cannot modify owner/permissions on "
		    ANOUBISD_SFS_TREE_VERSIONFILE);
	}
	fclose(fp);
	return ANOUBISD_SFS_TREE_FORMAT_VERSION;
}

#ifdef LINUX

/**
 * Tell the kernel's out of memory killer that this processes should never
 * be a victim of out of memory killing.
 *
 * @param pid The pid of the process.
 * @return None. This is a best effort function.
 */
static void
deactivate_oom_kill(int pid)
{
	FILE			*fp;
	char			*filename;

	if (asprintf(&filename, "/proc/%d/oom_adj", pid) < 0)
		return;
	fp = fopen(filename, "w");
	free(filename);
	if (!fp)
		return;

	/* -17 means disable oom killer for this process */
	fprintf(fp, "-17");
	fclose(fp);
}

#endif

/**
 * This is the anoubis daemon main entry point. This function handles
 * startup of the anoubis daemon and then enters the master's main event loop.
 * Anoubis daemon terminates once the main event loop exists. Depending
 * on the command line options this function daemonizes and runs the
 * main loop in a child process.
 *
 * @param argc the number of command line arguments given.
 * @param argv The command line arguments.
 * @return The exit code. Successful exit only indicates that the
 *     process demonized successfully.
 */
int
main(int argc, char *argv[])
{

	int			pipes[PIPE_MAX * 2];
	int			loggers[PROC_LOGGER * 2];
	int			logfd, policyfd, sessionfd, upgradefd;
	struct event		ev_sigterm, ev_sigint, ev_sigquit, ev_sighup,
				    ev_sigchld;
	struct event		ev_s2m, ev_p2m, ev_u2m, ev_dev2m;
	struct event		ev_m2s, ev_m2p, ev_m2u, ev_m2dev;
	struct event		ev_timer;
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

	sanitise_stdfd();
	anoubisd_process = PROC_MAIN;

	if (!cfg_initialize(argc, argv) || !cfg_read())
		fatal("Unable to read configuration");

#ifndef HAVE_SETPROCTITLE
	/* Prepare for later setproctitle emulation */
	compat_init_setproctitle(argc, argv);
#endif

	if ((logname = strdup(__progname)) == NULL) {
		logname = __progname;
		fatal("can't copy progname");
	}

	if (anoubisd_config.opts & ANOUBISD_OPT_NOACTION)
		cfg_dump(stdout);

	/*
	 * Get ANOUBISCORE Version early because this is inherited by the
	 * child processes. Close the file descriptor again because we
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
		if (empty < 0)
			early_err(5, "Could not read " SFS_CHECKSUMROOT);
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

	if (anoubisd_config.allow_coredumps) {
		struct rlimit limits = { RLIM_INFINITY, RLIM_INFINITY };
		int ret = setrlimit(RLIMIT_CORE, &limits);
		if (ret < 0)
			early_err(1, "setrlmit RLIMIT_CORE failed");
	} else {
		act.sa_flags = SA_SIGINFO;
		act.sa_sigaction = segvhandler;
		sigemptyset(&act.sa_mask);
		if (sigaction(SIGSEGV, &act, NULL) < 0 ||
		    sigaction(SIGILL,  &act, NULL) < 0 ||
		    sigaction(SIGFPE,  &act, NULL) < 0 ||
		    sigaction(SIGABRT, &act, NULL) < 0 ||
		    sigaction(SIGBUS,  &act, NULL) < 0
		    ) {
			early_errx(1, "Sigaction failed");
		}
	}

	if (geteuid() != 0)
		early_errx(1, "need root privileges");

	if (getpwnam(ANOUBISD_USER) == NULL)
		early_errx(1, "unkown user " ANOUBISD_USER);

	if ((pidfp = check_pid()) == NULL) {
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			early_errx(1, PACKAGE_DAEMON " is already running");
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

	if (anoubisd_config.pg_scanners)
		log_info("%d playground scanner(s) configured",
		    anoubisd_config.pg_scanners);
#ifdef LINUX
	dazukofd = dazukofs_ignore();

	for (p=0; omit_pid_files[p]; p++) {
		if ((omitfp = fopen(omit_pid_files[p], "w")) == NULL)
			continue;
		fprintf(omitfp, "%d\n%d\n%d\n%d\n%d\n",
		    master_pid, se_pid, policy_pid, logger_pid, upgrade_pid);
		fclose(omitfp);
		omit_pid_file = omit_pid_files[p];
		break;
	}
	deactivate_oom_kill(master_pid);
	deactivate_oom_kill(se_pid);
	deactivate_oom_kill(policy_pid);
	deactivate_oom_kill(logger_pid);
	deactivate_oom_kill(upgrade_pid);
#endif

	/* We catch or block signals rather than ignore them. */
	signal_set(&ev_sigterm, SIGTERM, sighandler, &ev_info);
	signal_set(&ev_sigint, SIGINT, sighandler, &ev_info);
	signal_set(&ev_sigquit, SIGQUIT, sighandler, NULL);
	signal_set(&ev_sighup, SIGHUP, sighandler, &ev_info);
	signal_set(&ev_sigchld, SIGCHLD, sighandler, &ev_info);
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
	ev_info.ev_sigchld = &ev_sigchld;

	anoubisd_defaultsigset(&mask);
	sigdelset(&mask, SIGCHLD);
	sigdelset(&mask, SIGHUP);
	/* Unblock SIGALRM. This is required by the scanner children. */
	sigdelset(&mask, SIGALRM);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	/* init msg_bufs - keep track of outgoing ev_info */
	msg_init(sessionfd);
	msg_init(policyfd);
	msg_init(upgradefd);

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
	msg_init(eventfds[0]);

	/* session process */
	event_set(&ev_m2s, sessionfd, EV_WRITE, dispatch_m2s, NULL);
	event_set(&ev_s2m, sessionfd, EV_READ | EV_PERSIST, dispatch_s2m,
	    &ev_info);
	event_add(&ev_s2m, NULL);

	/* policy process */
	event_set(&ev_m2p, policyfd, EV_WRITE, dispatch_m2p, NULL);
	event_set(&ev_p2m, policyfd, EV_READ | EV_PERSIST, dispatch_p2m,
	    &ev_info);
	event_add(&ev_p2m, NULL);

	/* upgrade process */
	event_set(&ev_m2u, upgradefd, EV_WRITE, dispatch_m2u, NULL);
	event_set(&ev_u2m, upgradefd, EV_READ | EV_PERSIST, dispatch_u2m,
	    &ev_info);
	event_add(&ev_u2m, NULL);

	/* event device */
	event_set(&ev_dev2m, eventfds[0], EV_READ | EV_PERSIST, dispatch_dev2m,
	    &ev_info);
	event_add(&ev_dev2m, NULL);

	event_set(&ev_m2dev, eventfds[0], EV_WRITE, dispatch_m2dev, NULL);

	queue_init(&eventq_m2p, &ev_m2p);
	queue_init(&eventq_m2s, &ev_m2s);
	queue_init(&eventq_m2dev, &ev_m2dev);
	queue_init(&eventq_m2u, &ev_m2u);

	ev_info.ev_dev2m = &ev_dev2m;
	ev_info.ev_p2m = &ev_p2m;
	ev_info.ev_s2m = &ev_s2m;
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
	pe_playground_initpgid(eventfds[1], eventfds[0]);
	/* Note that we keep the anoubis device open for subsequent ioctls. */

	/* Five second timer for statistics ioctl */
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	ev_info.anoubisfd = eventfds[1];
	ev_info.ev_timer = &ev_timer;
	ev_info.tv = &tv;
	evtimer_set(&ev_timer, &dispatch_timer, &ev_info);
	event_add(&ev_timer, &tv);

	init_root_key(NULL);

	setproctitle("master");

	DEBUG(DBG_TRACE, "master event loop");
	if (event_dispatch() == -1)
		log_warnx("main: event_dispatch");
	DEBUG(DBG_TRACE, "master event loop ended");
	/*
	 * We only reach this point in case of a graceful shutdown. Thus
	 * must of the cleanup is already done. In particular we must
	 * not wait for our children: session and policy already exited
	 * and logger will terminate after us.
	 */
	unlink(PACKAGE_PIDFILE);
#ifdef LINUX
	if (omit_pid_file)
		unlink(omit_pid_file);
#endif
	log_warnx(PACKAGE_DAEMON " shutdown");
	flush_log_queue();

	exit(0);
}

/**
 * Force a shutdown of the anoubis daemon master. This is not the
 * regular shutdown process it is only used during emergencies. It
 * forcefully terminates child processes (except for the logger process)
 * using SIGQUIT, cleans up open files and exits.
 *
 * @param error The exit code to use.
 * @return This function never returns.
 */
__dead static void
main_shutdown(int error)
{
	pid_t			pid;

	DEBUG(DBG_TRACE, ">main_cleanup");

	close(eventfds[1]);
	close(eventfds[0]);

	anoubisd_scanners_detach();
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

	log_warnx(PACKAGE_DAEMON " is terminating");
	flush_log_queue();
	unlink(PACKAGE_PIDFILE);
#ifdef LINUX
	if (omit_pid_file)
		unlink(omit_pid_file);
#endif
	log_info("shutting down");
	exit(error);
}

/**
 * This function can be used by all permantly active anoubis daemon processes
 * it initiate a forceful termination of the anoubis daemon. Non master
 * processes do this simply by exiting. The master process calls main_shutdown.
 *
 * @param The exit code to use for the calling process.
 * @return This function never returns.
 */
__dead void
master_terminate(int error)
{
	DEBUG(DBG_TRACE, ">master_terminate: error=%d", error);

	if (master_pid && getpid() == master_pid)
		main_shutdown(error);
	/*
	 * This will send a SIGCHLD to the master which will then terminate.
	 * It is not possible to just kill(2) the master because we switched
	 * UIDs and are no longer allowed to do this.
	 */
	_exit(error);
}

#ifdef LINUX

/**
 * Tell dazukofs that it should not try to check file system accesses
 * done by the anoubis daemon master.
 *
 * @param None.
 * @return An open file descriptior. The caller must keep this file
 *     descriptor open as long as it wants to dazukofs to ignore us.
 *     If something goes wrong (usually because dazukofs is not active)
 *     -1 is returned.
 */
int
dazukofs_ignore(void)
{
	int fd = -1;

	fd = open(_PATH_DEV "dazukofs.ign", O_RDONLY);
	if ((fd == -1) && (errno != ENOENT))
		log_warn("Could not open dazukofs: %s",
				anoubis_strerror(errno));
	return(fd);
}

#endif

/**
 * Ensure that fds 0, 1 and 2 are open or redirected to /dev/null.
 * If this function fails to setup fd in this way it will exit with an
 * early error.
 *
 * @param None.
 * @return None.
 */
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

/**
 * Close all file descriptors in the pipes and loggers array. The
 * caller should set file descriptors that it wants to keep opern to -1.
 * Additionally, all processes except the master also close the anoubis
 * daemon pid file.
 *
 * @param The pipes array with PIPE_MAX elements.
 * @param loggers The loggers array with PROC_LOGGER elements.
 * @return None.
 */
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

/**
 * Reconfigure the anoubis daemon master process (i.e. re-read its config
 * file). Config options that are relevant for child processes are stored
 * in a configuration message and sent to the child processes for their
 * reconfiguration.
 *
 * @param None.
 * @return None.
 */
static void
reconfigure(void)
{
	struct anoubisd_msg	*msg;

	/* Reload Public Keys */
	cert_reconfigure(0);

	if (!cfg_reread()) {
		log_warnx("re-read of configuration failed.");
	} else {
		msg = cfg_msg_create();

		if (msg != NULL)
			enqueue(&eventq_m2s, msg);

		msg = cfg_msg_create();
		if (msg != NULL)
			enqueue(&eventq_m2p, msg);
		init_root_key(NULL);
	}
}

/**
 * This is the event handler for messages that are sent from the master to
 * the session process. It is called when the master to session queue is not
 * empty and the master to session pipe is ready for writing. The handler
 * must make sure that it re-adds the write event as long as the queue is
 * not empty. This is handled by dispatch_write_queue.
 *
 * @param fd The file descriptor of the pipe to the session engine.
 * @param event The event type (see libevent, unused).
 * @param arg The callback argument (see libevent, unused).
 */
static void
dispatch_m2s(int fd, short event __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_m2s");

	if (dispatch_write_queue(&eventq_m2s, fd) <= 0) {
		/* Write was not successful: Check if we lost a child. */
		sighandler(SIGCHLD, 0, NULL);
	}
	if (terminate >= 2 && !queue_peek(&eventq_m2s) && !msg_pending(fd))
		shutdown(fd, SHUT_WR);

	DEBUG(DBG_TRACE, "<dispatch_m2s");
}

/**
 * Tell the policy engine that it should invalidate any cached sfs tree
 * entries for the given path name and user ID. This must be called after the
 * master modifies the sfs tree entry on disk.
 *
 * @param path The path name of the entry to invalidate.
 * @param uid The user ID of the entry to invalidate.
 * @return None.
 */
static void
send_sfscache_invalidate_uid(const char *path, uid_t uid)
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
}

/**
 * Tell the policy engine that it should invalidate any cached sfs tree
 * entries for the given path name and key ID. This must be called after
 * the master modifies the sfs tree entry on disk.
 *
 * @param path The path name of the entry to invalidate.
 * @param keyid The key ID of the entry to invalidate.
 * @return None.
 */
static void
send_sfscache_invalidate_key(const char *path, const struct abuf_buffer keyid)
{
	struct anoubisd_msg			*msg;
	struct anoubisd_sfscache_invalidate	*invmsg;
	char					*keyidstr = NULL;

	keyidstr = abuf_convert_tohexstr(keyid);
	if (keyidstr == NULL) {
		master_terminate(ENOMEM);
		return;
	}
	msg = msg_factory(ANOUBISD_MSG_SFSCACHE_INVALIDATE,
	    sizeof(struct anoubisd_sfscache_invalidate) + strlen(path) + 1
	    + strlen(keyidstr) + 1);
	if (!msg) {
		free(keyidstr);
		master_terminate(ENOMEM);
		return;
	}
	invmsg = (struct anoubisd_sfscache_invalidate*)msg->msg;
	invmsg->uid = (uid_t)-1;
	invmsg->plen = strlen(path)+1;
	invmsg->keylen = strlen(keyidstr)+1;
	memcpy(invmsg->payload, path, invmsg->plen);
	memcpy(invmsg->payload + invmsg->plen, keyidstr, invmsg->keylen);
	free(keyidstr);
	enqueue(&eventq_m2p, msg);
}

/**
 * Tell the policy engine that it should invalidate any cached sfs tree
 * entries for the entry that was just modified by the checksum request.
 * This function extracts path name and user or key ID from the checksum op
 * and calls the appropriate invalidate function.
 *
 * @param The checksum operation.
 * @return None.
 */
static void
send_sfscache_invalidate(struct sfs_checksumop *csop)
{
	switch (csop->op) {
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case ANOUBIS_CHECKSUM_OP_DEL:
		send_sfscache_invalidate_uid(csop->path, csop->uid);
		break;
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
		send_sfscache_invalidate_key(csop->path, csop->keyid);
	}
}

/**
 * Send a message to the policy engine that tells the policy engine
 * if it is ok to track upgrades.
 *
 * @param True if it is ok to track update, false otherwise.
 * @return None.
 */
static void
send_upgrade_ok(int value)
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
}

/**
 * Notfiy the session engine about a finished upgrade. The message
 * contains the number of upgraded files in chunksize.
 *
 * @param None.
 * @return None.
 */
static void
send_upgrade_notification(void)
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
}

/**
 * Initialize root's private key for signature updates during upgrade.
 * This function reads the root private key as specified in the configuration
 * file and loads it. If a valid root key is present, it will be used to
 * update signatures of root during an upgrade.
 *
 * @param passphrase The passphrase required to unlock the key.
 * return None.
 */
static void
init_root_key(char *passphrase)
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
	 * private key must be deleted.
	 */
	if (anoubisd_config.rootkey == NULL) {
		/* Clear existing rootkey if any. */
		if (cert)
			cert_load_priv_key(cert, NULL, NULL);
		/* Update is allowed but will not do signature upgrades */
		send_upgrade_ok(1);
		return;
	}
	/*
	 * If a rootkey is configured but root does not have a public key
	 * we ignore the root key and allow upgrades.
	 */
	if (!cert) {
		log_warnx("Ignoring rootkey because root has no public key.");
		send_upgrade_ok(1);
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
		send_upgrade_ok(0);
		return;
	}
	/* Upgrades are allowed. */
	send_upgrade_ok(1);
	DEBUG(DBG_UPGRADE, "<init_root_key: cert: %p %p", cert,
	    cert?cert->privkey:NULL);
}

/**
 * Create a checksum reply message and initialize it. The memory for the
 * message is allocated via msg_factory and must be freed by the caller.
 *
 * @param type The message type of the new message.
 * @param token The token of the new message is initialized to this
 *     value.
 * @param payloadlen This is the amout of memory that is reserved for
 *     message payload. This memory is allocated but remains uninitialized.
 * @return The message or NULL if memory allocation failed.
 */
static struct anoubisd_msg *
create_checksumreply_msg(int type, u_int64_t token, int payloadlen)
{
	struct anoubisd_msg		*msg;
	struct anoubisd_msg_csumreply	*reply;

	msg = msg_factory(type, sizeof(struct anoubisd_msg_csumreply)
	    + payloadlen);
	if (!msg) {
		master_terminate(ENOMEM);
		return 0;
	}
	reply = (struct anoubisd_msg_csumreply *)msg->msg;
	memset(reply, 0, sizeof(struct anoubisd_msg_csumreply));
	reply->flags = 0;
	reply->len = payloadlen;
	reply->token = token;
	reply->reply = 0;
	return msg;
}

/**
 * Process a checksum list request. The details of the request are given
 * by the csop parameter. As a result of this function one or more
 * checksum reply message are sent to the session engine. These messages
 * contain the result of the list request.
 *
 * @param csop The details of the checksum list operation.
 * @param token The token to use for the reply message.
 * @return None.
 */
static void
sfs_checksumop_list(struct sfs_checksumop *csop, u_int64_t token)
{
	char				*sfs_path = NULL;
	int				 error = EINVAL;
	struct anoubisd_msg		*msg = NULL;
	struct anoubisd_msg_csumreply	*reply = NULL;
	DIR				*sfs_dir = NULL;
	struct dirent			*entry;
	int				 offset;
	int				 found = 0, wantids;

	wantids = (csop->listflags & ANOUBIS_CSUM_WANTIDS);
	if (csop->op != ANOUBIS_CHECKSUM_OP_GENERIC_LIST) {
		error = EINVAL;
		goto out;
	}
	error = -convert_user_path(csop->path, &sfs_path, !wantids);
	if (error)
		goto out;
	sfs_dir = opendir(sfs_path);
	if (sfs_dir == NULL) {
		error = errno;
		if (error == ENOENT || error == ENOTDIR)
			error = 0;
		goto out;
	}
	msg = create_checksumreply_msg(ANOUBISD_MSG_CHECKSUMREPLY,
	    token, 8000);
	reply = (struct anoubisd_msg_csumreply *)msg->msg;
	reply->flags |= POLICY_FLAG_START;
	offset = 0;

	while((entry = readdir(sfs_dir)) != NULL) {
		char		*tmp;
		int		 tmplen;
		if (!sfs_wantentry(csop, sfs_path, entry->d_name))
			continue;
		found = 1;
		if (wantids) {
			tmp = strdup(entry->d_name);
		} else {
			tmp = remove_escape_seq(entry->d_name);
		}
		tmplen = strlen(tmp) + 1;
		if (offset + tmplen > reply->len) {
			reply->len = offset;
			msg_shrink(msg,
			    sizeof(struct anoubisd_msg_csumreply) + offset);
			enqueue(&eventq_m2s, msg);
			msg = create_checksumreply_msg(
			    ANOUBISD_MSG_CHECKSUMREPLY, token, 8000);
			reply = (struct anoubisd_msg_csumreply *)msg->msg;
			offset = 0;
		}
		if (offset + tmplen > reply->len) {
			free(tmp);
			error = ENAMETOOLONG;
			goto out;
		}
		memcpy(reply->data + offset, tmp, tmplen);
		offset += tmplen;
		free(tmp);
	}
	closedir(sfs_dir);
	reply->len = offset;
	reply->flags |= POLICY_FLAG_END;
	msg_shrink(msg, sizeof(struct anoubisd_msg_csumreply) + offset);
	enqueue(&eventq_m2s, msg);
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
		msg_shrink(msg, sizeof(struct anoubisd_msg_csumreply));
		reply->len = 0;
		reply->flags |= POLICY_FLAG_END;
	} else {
		msg = create_checksumreply_msg(ANOUBISD_MSG_CHECKSUMREPLY,
		    token, 0);
		reply = (struct anoubisd_msg_csumreply *)msg->msg;
		reply->flags = (POLICY_FLAG_START | POLICY_FLAG_END);
	}
	reply->reply = error;
	enqueue(&eventq_m2s, msg);
}

/**
 * Process a CSMULTI style checksum request. The core property of a
 * CSMULTI request is that it can contain serveral path names in a single
 * request message. This function sends one ore more checksum reply message
 * with the result of the checksum operation to the session engine.
 * Individual checksum operations in the CSMULTI request are handled by
 * sfs_checksumop.
 *
 * @param csop The checksum operation.
 * @param token The token to use for the reply messages.
 * @return Zero if the request was processed and no further action of the
 *     caller is neccessary. A negative error code if an error occured.
 *     In case of an error the caller must notify the session engine
 *     of the error.
 *
 * NOTE: This function cannot be used to process list requests.
 */
static int
sfs_process_csmulti(struct sfs_checksumop *csop, u_int64_t token)
{
	int				 get = 0;
	unsigned int			 totallen = 0;
	int				*errors = NULL;
	struct abuf_buffer		*sigbufs = NULL;
	struct abuf_buffer		 rbuf;
	unsigned int			 nrec = csop->nrec;
	unsigned int			 i;
	struct anoubisd_msg		*msg = NULL;
	struct anoubisd_msg_csumreply	*reply;
	Anoubis_CSMultiReplyMessage	*rep;
	Anoubis_CSMultiReplyRecord	*r;
	unsigned int			 off;

	DEBUG(DBG_TRACE, " >sfs_process_csmulti");
	if (csop->op == ANOUBIS_CHECKSUM_OP_GET2
	    || csop->op == ANOUBIS_CHECKSUM_OP_GETSIG2)
		get = 1;
	if (nrec > 1000)
		nrec = 1000;
	errors = malloc(nrec * sizeof(int));
	if (!errors) {
		DEBUG(DBG_TRACE, " <sfs_process_csmulti (ENOMEM)");
		return -ENOMEM;
	}
	if (get) {
		sigbufs = malloc(nrec * sizeof(struct abuf_buffer));
		if (sigbufs == NULL) {
			free(errors);
			DEBUG(DBG_TRACE, " <sfs_process_csmulti (ENOMEM)");
			return -ENOMEM;
		}
		for (i=0; i<nrec; ++i)
			sigbufs[i] = ABUF_EMPTY;
	}
	totallen = sizeof(Anoubis_CSMultiReplyMessage);
	DEBUG(DBG_CSUM, " sfs_process_csmulti: nrec=%d", nrec);
	for (i=0; i<nrec; ++i) {
		struct sfs_csmulti_record	*rec;

		rec = &csmultiarr_access(csop->csmulti, i);
		csop->path = rec->path;
		csop->sigbuf = rec->csdata;
		if (get)
			errors[i] = -sfs_checksumop(csop, &sigbufs[i]);
		else
			errors[i] = -sfs_checksumop(csop, NULL);
		totallen += sizeof(Anoubis_CSMultiReplyRecord);
		if (errors[i]) {
			if (get) {
				abuf_free(sigbufs[i]);
				sigbufs[i] = ABUF_EMPTY;
			}
		} else {
			if (get)
				totallen += abuf_length(sigbufs[i]);
			else
				send_sfscache_invalidate(csop);
		}
		if (totallen > 16000) {
			nrec = i+1;
			break;
		}
	}
	totallen += sizeof(Anoubis_CSMultiReplyRecord);
	msg = create_checksumreply_msg(ANOUBISD_MSG_CSMULTIREPLY, token,
	    totallen);
	if (!msg)
		goto nomem;

	reply = (struct anoubisd_msg_csumreply *)msg->msg;
	reply->flags = POLICY_FLAG_START | POLICY_FLAG_END;
	reply->reply = 0;
	rbuf = abuf_open_frommem(reply->data, totallen);
	rep = abuf_cast(rbuf, Anoubis_CSMultiReplyMessage);
	set_value(rep->type, ANOUBIS_P_CSMULTIREPLY);
	set_value(rep->operation, csop->op);
	set_value(rep->error, 0);
	off = offsetof(Anoubis_CSMultiReplyMessage, payload);
	for (i=0; i<nrec; ++i) {
		unsigned int			 length;

		length = sizeof(Anoubis_CSMultiReplyRecord);
		if (get)
			length += abuf_length(sigbufs[i]);
		r = abuf_cast_off(rbuf, off, Anoubis_CSMultiReplyRecord);
		if (!r)
			goto nomem;
		set_value(r->length, length);
		set_value(r->index, csmultiarr_access(csop->csmulti, i).index);
		set_value(r->error, errors[i]);
		if (get && abuf_length(sigbufs[i])) {
			unsigned int cp;
			unsigned int off2;

			off2 = off
			    + offsetof(Anoubis_CSMultiReplyRecord, payload);
			cp = abuf_copy_part(rbuf, off2, sigbufs[i], 0,
			    abuf_length(sigbufs[i]));
			if (cp != abuf_length(sigbufs[i]))
				goto nomem;
			abuf_free(sigbufs[i]);
			sigbufs[i] = ABUF_EMPTY;
		}
		off += length;
	}
	r = abuf_cast_off(rbuf, off, Anoubis_CSMultiReplyRecord);
	if (!r)
		goto nomem;
	set_value(r->length, 0);
	set_value(r->index, 0);
	set_value(r->error, 0);
	free(errors);
	if (sigbufs)
		free(sigbufs);

	enqueue(&eventq_m2s, msg);
	DEBUG(DBG_QUEUE, " >eventq_m2s: %" PRIx64, reply->token);
	DEBUG(DBG_TRACE, " <sfs_process_csmulti (success)");

	return 0;

nomem:
	if (msg)
		free(msg);
	if (errors)
		free(errors);
	if (sigbufs) {
		for (i=0; i<nrec; ++i)
			abuf_free(sigbufs[i]);
		free(sigbufs);
	}
	DEBUG(DBG_TRACE, " <sfs_process_csmulti (error, ENOMEM)");
	return -ENOMEM;
}

/**
 * Handle a checksum request received from the session engine. This
 * function parses the request message into a checksum operation structure
 * and calls the appropriate function (either sfs_checksumop_list or
 * sfs_checksumop directly) to handle the request. CSMULTI style requests
 * are not handled by this function.
 *
 * @param msg The checksum request message. The message ist not freed by
 *     this function.
 * @return None.
 */
static void
dispatch_checksumop(struct anoubisd_msg *msg)
{
	struct anoubis_msg		 rawmsg;
	struct anoubisd_msg_csumop	*msg_csum;
	struct sfs_checksumop		 csop;
	struct anoubisd_msg_csumreply	*reply;
	struct abuf_buffer		 sigbuf = ABUF_EMPTY;
	int				 err = -EFAULT;

	msg_csum = (struct anoubisd_msg_csumop *)msg->msg;
	rawmsg.length = msg_csum->len;
	rawmsg.u.buf = msg_csum->msg;
	err = sfs_parse_checksumop(&csop, &rawmsg, msg_csum->uid);
	if (err < 0)
		goto out;

	switch (csop.op) {
	case ANOUBIS_CHECKSUM_OP_GENERIC_LIST:
		sfs_checksumop_list(&csop, msg_csum->token);
		return;
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
	case ANOUBIS_CHECKSUM_OP_GET2:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
	case ANOUBIS_CHECKSUM_OP_DEL:
		err = sfs_checksumop(&csop, &sigbuf);
		if (err < 0)
			goto out;
		break;
	default:
		err = -EINVAL;
		goto out;
	}

	send_sfscache_invalidate(&csop);
out:
	msg = create_checksumreply_msg(ANOUBISD_MSG_CHECKSUMREPLY,
	    msg_csum->token, abuf_length(sigbuf));
	reply = (struct anoubisd_msg_csumreply *)msg->msg;
	reply->reply = -err;
	reply->flags = POLICY_FLAG_START | POLICY_FLAG_END;
	if (abuf_length(sigbuf)) {
		abuf_copy_frombuf(reply->data, sigbuf, abuf_length(sigbuf));
	}
	abuf_free(sigbuf);
	enqueue(&eventq_m2s, msg);
	DEBUG(DBG_QUEUE, " >eventq_m2s: %" PRIx64, reply->token);
}

/**
 * Handle a CSMULTI checksum request message received from the session
 * engine. This function parses the request message into a checksum
 * operation structure and calls sfs_process_csmulti to handle the request.
 *
 * @param msg The request message.
 * @return None.
 */
static void
dispatch_csmulti_request(struct anoubisd_msg *msg)
{
	struct anoubisd_msg_csumop	*msg_csum;
	struct sfs_checksumop		 csop;
	struct anoubisd_msg_csumreply	*reply;
	struct abuf_buffer		 mbuf;
	int				 err;

	DEBUG(DBG_TRACE, " >dispatch_csmulti_request");
	msg_csum = (struct anoubisd_msg_csumop *)msg->msg;
	mbuf = abuf_open_frommem(msg_csum->msg, msg_csum->len);
	err = sfs_parse_csmulti(&csop, mbuf, msg_csum->uid);
	if (err < 0)
		goto err;
	err = sfs_process_csmulti(&csop, msg_csum->token);
	csmultiarr_free(csop.csmulti);
	if (err)
		goto err;
	DEBUG(DBG_TRACE, " <dispatch_csmulti_request (success)");
	return;

err:
	msg = create_checksumreply_msg(ANOUBISD_MSG_CSMULTIREPLY,
	    msg_csum->token, 0);
	reply = (struct anoubisd_msg_csumreply *)msg->msg;
	reply->reply = -err;
	reply->flags = POLICY_FLAG_START | POLICY_FLAG_END;
	enqueue(&eventq_m2s, msg);
	DEBUG(DBG_QUEUE, " >eventq_m2s %" PRIx64, reply->token);
	DEBUG(DBG_TRACE, " <dispatch_csmulti_request (error)");
}

/**
 * Handle a message from the session engine that contains the passphrase
 * of root's private key. The function init_root_key is called with the
 * passphrase to unlock roots private key.
 * NOTE: See the anoubisctl manual page for security considerations
 * regarding this feature.
 *
 * @param msg The request message.
 * @return None.
 */
static void
dispatch_passphrase(struct anoubisd_msg *msg)
{
	anoubisd_msg_passphrase_t	*pass;
	int				 len;

	pass = (anoubisd_msg_passphrase_t *)msg->msg;
	len = msg->size - sizeof(struct anoubisd_msg);
	if (len < (int)sizeof(anoubisd_msg_passphrase_t) + 1)
		return;
	pass->payload[len-1] = 0;
	init_root_key(pass->payload);
}

/**
 * The number of random bytes used for the authentication challenge.
 */
#define		RANDOM_CHALLENGE_BYTES	16

/**
 * Handle a user request for authentication. Depending on the authentication
 * mode this function sends an authentication challenge or a success report
 * (an empty challenge) to the session engine. The callenge (if any) must be
 * signed by the user interface with the user's key and sent back to the
 * master for verification.
 *
 * @param The authentication request message.
 * @return None.
 *
 * NOTE: The master process does not keep track of the authentication
 * challenges it generated. The session engine is responsible for this.
 */
static void
dispatch_auth_request(struct anoubisd_msg *msg)
{
	struct anoubisd_msg			*rep_msg;
	struct anoubisd_msg_authchallenge	*challenge;
	struct anoubisd_msg_authrequest		*auth;
	struct cert				*cert;
	int					 need_challenge = 0;
	int					 error = 0;
	static int				 rnd_initialized = 0;

	struct {
		struct timeval		time;
		unsigned char		rnd[RANDOM_CHALLENGE_BYTES];
	} cdata;

	auth = (struct anoubisd_msg_authrequest *)msg->msg;
	cert = cert_get_by_uid_ignored(auth->auth_uid);
	switch(anoubisd_config.auth_mode) {
	case ANOUBISD_AUTH_MODE_ENABLED:
		need_challenge = 1;
		break;
	case ANOUBISD_AUTH_MODE_OPTIONAL:
		if (cert && cert->ignore == 0)
			need_challenge = 1;
		else if (cert && cert->ignore == 1)
			error = EPERM;
		break;
	case ANOUBISD_AUTH_MODE_OFF:
		break;
	default:
		error = EACCES;
		need_challenge = 0;
	}
	if (need_challenge && rnd_initialized == 0) {
		if (RAND_status()) {
			rnd_initialized = 1;
		} else {
			/* openssl RAND will automatically seed from
			 * /dev/urandom if available. If it isn't, we
			 * have a problem.
			 */
			log_warnx("ERROR: No entropy available. "
			    "/dev/urandom missing?");
			error = EACCES;
		}
	}
	if (need_challenge && cert && abuf_length(cert->keyid)
	    && rnd_initialized) {
		int ret;
		/*
		 * This doesn't need to be good random data, the same value
		 * just must not be used more than once.
		 */
		RAND_pseudo_bytes(&cdata.rnd[0], sizeof(cdata.rnd));
		ret = gettimeofday(&cdata.time, NULL);
		if (ret < 0) {
			error = errno;
		} else {
			/*
			 * let's assume the micro second timestamp contains
			 * 4 bits of entropy
			 */
			RAND_add(&cdata.time, sizeof(cdata.time), 0.5);
		}
	}
	if (error || cert == NULL || need_challenge == 0
	    || abuf_length(cert->keyid) == 0) {
		rep_msg = msg_factory(ANOUBISD_MSG_AUTH_CHALLENGE,
		    sizeof(struct anoubisd_msg_authchallenge));
		if (rep_msg == NULL) {
			master_terminate(ENOMEM);
			return;
		}
		challenge = (struct anoubisd_msg_authchallenge *)rep_msg->msg;
		challenge->token = auth->token;
		challenge->auth_uid = auth->auth_uid;
		if (need_challenge)
			challenge->error = EPERM;
		else if (error)
			challenge->error = error;
		else
			challenge->error = 0;
		challenge->idlen = challenge->challengelen = 0;
	} else {
		/* We do need a challenge. Create it. */
		int	payload = abuf_length(cert->keyid) + sizeof(cdata);

		rep_msg = msg_factory(ANOUBISD_MSG_AUTH_CHALLENGE,
		    sizeof(struct anoubisd_msg_authchallenge) + payload);
		if (rep_msg == NULL) {
			master_terminate(ENOMEM);
			return;
		}
		challenge = (struct anoubisd_msg_authchallenge *)rep_msg->msg;
		challenge->token = auth->token;
		challenge->auth_uid = auth->auth_uid;
		challenge->idlen = abuf_length(cert->keyid);
		challenge->challengelen = sizeof(cdata);
		challenge->error = 0;
		memcpy(challenge->payload, &cdata, sizeof(cdata));
		abuf_copy_frombuf(challenge->payload + sizeof(cdata),
		    cert->keyid, abuf_length(cert->keyid));
	}
	enqueue(&eventq_m2s, rep_msg);
	DEBUG(DBG_QUEUE, " >eventq_m2s: challenge %" PRIx64, auth->token);
	return;
}

/**
 * Process a request from the session engine to verify an authentication
 * request. The message contains an authentication challenge and the
 * signature. The result is an authenticaton result message that informs
 * the session engine whether the signature was correct.
 *
 * @param imsg The message with the authentication request received from
 *     the session engine.
 * @return None.
 */
static void
dispatch_auth_verify(struct anoubisd_msg *imsg)
{
	struct anoubisd_msg			*omsg;
	struct anoubisd_msg_authverify		*verify;
	struct anoubisd_msg_authresult		*authresult;
	struct cert				*cert;
	void					*data, *sig;
	int					 datalen, siglen;

	verify = (struct anoubisd_msg_authverify *)imsg->msg;
	cert = cert_get_by_uid(verify->auth_uid);
	data = verify->payload;
	datalen = verify->datalen;
	sig = verify->payload + datalen;
	siglen = verify->siglen;
	omsg = msg_factory(ANOUBISD_MSG_AUTH_RESULT,
	    sizeof(struct anoubisd_msg_authresult));
	if (omsg == NULL) {
		master_terminate(ENOMEM);
		return;
	}
	authresult = (struct anoubisd_msg_authresult *)omsg->msg;
	authresult->token = verify->token;
	authresult->auth_uid = verify->auth_uid;
	if (cert == NULL || cert->pubkey == NULL || anoubis_sig_verify_buffer(
	    data, datalen, sig, siglen, cert->pubkey) != 0)
		authresult->error = EPERM;
	else
		authresult->error = 0;
	enqueue(&eventq_m2s, omsg);
	DEBUG(DBG_QUEUE, " >eventq_m2s: auth result token=%" PRIx64 " error=%d",
	    authresult->token, authresult->error);
}

/**
 * This is the event handler for incoming messages from the session engine.
 * The corrensponding libevent event is always active. This function is
 * called as soon as data becomes ready on the file descriptor. Note that
 * there may be incomplete messages, get_msg will return NULL in this case.
 * Additionally, there may be multiple messages.
 *
 * @param fd The file descriptor to read from.
 * @param event The type of the event (see libevent, unused)
 * @param arg The event callback data. This is an event_info_main
 *     structure.
 * @return None.
 */
static void
dispatch_s2m(int fd, short event __used, void *arg)
{
	struct anoubisd_msg		*msg;
	struct event_info_main		*ev_info = arg;

	DEBUG(DBG_TRACE, ">dispatch_s2m");

	for (;;) {

		if ((msg = get_msg(fd)) == NULL)
			break;
		switch(msg->mtype) {
		case ANOUBISD_MSG_CHECKSUM_OP:
			dispatch_checksumop(msg);
			break;
		case ANOUBISD_MSG_CSMULTIREQUEST:
			dispatch_csmulti_request(msg);
			break;
		case ANOUBISD_MSG_PASSPHRASE:
			dispatch_passphrase(msg);
			break;
		case ANOUBISD_MSG_AUTH_REQUEST:
			dispatch_auth_request(msg);
			break;
		case ANOUBISD_MSG_AUTH_VERIFY:
			dispatch_auth_verify(msg);
			break;
		default:
			log_warnx("dispatch_s2m: bad mtype %d", msg->mtype);
			break;
		}
		free(msg);
		DEBUG(DBG_TRACE, "<dispatch_s2m (loop)");
	}
	if (msg_eof(fd))
		event_del(ev_info->ev_s2m);
	DEBUG(DBG_TRACE, "<dispatch_s2m");
}

/**
 * This is the event handler for messages that are sent from the master to
 * the policy process. It is called when the master to policy queue is not
 * empty and the master to policy pipe is ready for writing. The handler
 * must make sure that it re-adds the write event as long as the queue is
 * not empty. This is handled by dispatch_write_queue.
 *
 * @param fd The file descriptor of the pipe to the session engine.
 * @param event The event type (see libevent, unused).
 * @param arg The callback argument (see libevent, unused).
 */
static void
dispatch_m2p(int fd, short event __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_m2p");

	if (dispatch_write_queue(&eventq_m2p, fd) <= 0) {
		/* Write not successful: Check if we lost a child. */
		sighandler(SIGCHLD, 0, NULL);
	}
	if (terminate >= 2 && !queue_peek(&eventq_m2p) && !msg_pending(fd))
		shutdown(fd, SHUT_WR);

	DEBUG(DBG_TRACE, "<dispatch_m2p");
}

#ifdef LINUX

/**
 * Convert a device number as received from stat to the format that
 * is reported by the kernel.
 *
 * @param dev The stat style device number.
 * @return The kernel style device number.
 */
static inline uint64_t
expand_dev(dev_t dev)
{
	uint64_t	major = (dev & 0xfff00) >> 8;
	uint64_t	minor = (dev & 0xff) | ((dev >> 12) & 0xfff00);

	return (major << 20) | minor;
}

#else

static inline uint64_t
expand_dev(dev_t dev)
{
	return dev;
}

#endif

/**
 * Handle a playground commit request received from the policy engine.
 * This function opens the file to scan and calls anoubisd_scan_start
 * to start a scanner child process. In case of an error the error is
 * reported directly. Otherwise the result of the scanner child process
 * will be reported.
 *
 * @param msg The request message from the policy engine.
 * @param evinfo The event information of the main thread.
 * @return None.
 */
static void
dispatch_pgcommit(struct anoubisd_msg *msg,
    struct event_info_main *evinfo __used)
{
	struct anoubisd_msg_pgcommit		*pgmsg;
	struct anoubisd_msg			*rmsg;
	struct anoubisd_msg_pgcommit_reply	*pgrep;
	struct stat				 statbuf;
	int					 fd = -1;
	int					 err = ENOSYS;

	pgmsg = (struct anoubisd_msg_pgcommit *)msg->msg;
	DEBUG(DBG_TRACE, ">dispatch_pgcommit token %" PRId64, pgmsg->token);
	/*
	 * XXX CEH: We should not try to open arbitrary user provided files
	 * XXX CEH: in the master process. This can lead to denial of
	 * XXX CEH: service. We should try to move the open to a child process.
	 */
	fd = open(pgmsg->path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
	if (fd < 0) {
		err = errno;
		goto error;
	}
	if (fstat(fd, &statbuf) < 0) {
		err = errno;
		goto error;
	}
	err = EPERM;
	if (expand_dev(statbuf.st_dev) != pgmsg->dev
	    || statbuf.st_ino != pgmsg->ino)
		goto error;
#ifdef LINUX
	if (ioctl(evinfo->anoubisfd, ANOUBIS_SCAN_STARTED, fd) < 0) {
		err = errno;
		goto error;
	}
#else
	err = -ENOSYS;
	goto error;
#endif
	err = -anoubisd_scan_start(pgmsg->token, fd, pgmsg->auth_uid,
	    pgmsg->ignore_recommended_scanners);
	if (err)
		goto error;
	return;
error:
	if (fd >= 0)
		close(fd);
	rmsg = msg_factory(ANOUBISD_MSG_PGCOMMIT_REPLY,
	    sizeof(struct anoubisd_msg_pgcommit_reply));
	pgrep = (struct anoubisd_msg_pgcommit_reply *)rmsg->msg;
	pgrep->error = err;
	pgrep->len = 0;
	pgrep->token = pgmsg->token;
	enqueue(&eventq_m2p, rmsg);
	DEBUG(DBG_QUEUE, " >eventq_m2p: commit reply error=%d token=%" PRId64,
	    pgrep->error, pgrep->token);
	DEBUG(DBG_TRACE, "<dispatch_pgcommit token %" PRId64, pgmsg->token);
}

/**
 * This is the event handler for incoming messages from the policy engine.
 * The corrensponding libevent event is always active. This function is
 * called as soon as data becomes ready on the file descriptor. Note that
 * there may be incomplete messages. This is handled by the get_msg code.
 * Additionally, there may be multiple messages.
 *
 * @param fd The file descriptor to read from.
 * @param event The type of the event (see libevent, unused)
 * @param arg The event callback data. This is an event_info_main
 *     structure.
 * @return None.
 */
static void
dispatch_p2m(int fd, short event __used, /*@dependent@*/ void *arg)
{
	struct event_info_main		*ev_info = arg;
	struct eventdev_reply		*ev_rep;
	struct anoubisd_msg		*msg;

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
			break;
		case ANOUBISD_MSG_UPGRADE:
			DEBUG(DBG_QUEUE, " >p2m: upgrade msg");
			enqueue(&eventq_m2u, msg);
			break;
		case ANOUBISD_MSG_PGCOMMIT:
			DEBUG(DBG_QUEUE, " >p2m: pgcommit msg");
			dispatch_pgcommit(msg, ev_info);
			free(msg);
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

/**
 * This is the event handler for messages that are sent from the master to
 * the kernel device. It is called when the master to kernel queue is not
 * empty and the eventdev device is ready for writing. The handler
 * must make sure that it re-adds the write event as long as the queue is
 * not empty.
 *
 * @param fd The file descriptor of the pipe to the session engine.
 * @param event The event type (see libevent, unused).
 * @param arg The callback argument (see libevent, unused).
 */
static void
dispatch_m2dev(int fd, short event __used, void *arg __used)
{
	struct anoubisd_msg		*msg;
	struct eventdev_reply		*ev_rep;
	ssize_t				 ret;

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
		event_add(eventq_m2dev.ev, NULL);

	DEBUG(DBG_TRACE, "<dispatch_m2dev");
}

/**
 * This is the event handler for kernel events. The corresponding libevent
 * event is always active. This function is called as soon as data becomes
 * ready on the file descriptor. Kernel events received from the eventdev
 * queue are dispatched to the policy engine or directly to the session
 * engine depending on the message's source and the value of the NEED_REPLY
 * flag in the event.
 *
 * @param fd The file descriptor to read from.
 * @param event The type of the event (see libevent, unused)
 * @param arg The event callback data. This is an event_info_main
 *     structure.
 * @return None.
 */
static void
dispatch_dev2m(int fd, short event __used, void *arg)
{
	struct eventdev_hdr		*hdr;
	struct eventdev_reply		*rep;
	struct anoubisd_msg		*msg;
	struct anoubisd_msg		*msg_reply;
	struct event_info_main		*ev_info = arg;

	DEBUG(DBG_TRACE, ">dispatch_dev2m");

	for (;;) {
		if ((msg = get_event(fd)) == NULL)
			break;
		hdr = (struct eventdev_hdr *)msg->msg;

		DEBUG(DBG_QUEUE, " >dev2m: %x %c source=%d", hdr->msg_token,
		    (hdr->msg_flags & EVENTDEV_NEED_REPLY)  ? 'R' : 'N',
		    hdr->msg_source);

		/* we shortcut and ack events for our own children */
		if ((hdr->msg_flags & EVENTDEV_NEED_REPLY) &&
		    (hdr->msg_pid == (u_int32_t)se_pid
		     || hdr->msg_pid == (u_int32_t)policy_pid
		     || hdr->msg_pid == (u_int32_t)logger_pid)) {
			msg_reply = msg_factory(ANOUBISD_MSG_EVENTREPLY,
			    sizeof(struct eventdev_reply));
			if (msg_reply == NULL) {
				master_terminate(ENOMEM);
				return;
			}
			rep = (struct eventdev_reply *)msg_reply->msg;
			rep->msg_token = hdr->msg_token;
			rep->reply = 0;
			if (hdr->msg_source == ANOUBIS_SOURCE_PLAYGROUND)
				rep->reply = EPERM;

			/* this should be queued, so as to not get lost */
			enqueue(&eventq_m2dev, msg_reply);
			rep = (struct eventdev_reply *)msg_reply->msg;
			DEBUG(DBG_QUEUE, " >eventq_m2dev: %x", rep->msg_token);

			free(msg);

			DEBUG(DBG_TRACE, "<dispatch_dev2m (self)");
			continue;

		}

		DEBUG(DBG_TRACE, " token %x pid %d", hdr->msg_token,
		    hdr->msg_pid);

		if ((hdr->msg_flags & EVENTDEV_NEED_REPLY) ||
		    (hdr->msg_source == ANOUBIS_SOURCE_PROCESS) ||
		    (hdr->msg_source == ANOUBIS_SOURCE_SFSEXEC) ||
		    (hdr->msg_source == ANOUBIS_SOURCE_IPC) ||
		    (hdr->msg_source == ANOUBIS_SOURCE_PLAYGROUNDPROC) ||
		    (hdr->msg_source == ANOUBIS_SOURCE_PLAYGROUNDFILE)) {
			/* Send event to policy process for handling. */
			enqueue(&eventq_m2p, msg);
			DEBUG(DBG_QUEUE, " >eventq_m2p: %x source=%d",
			    hdr->msg_token, hdr->msg_source);
		} else {
			/* Send event to session process for notifications. */
			enqueue(&eventq_m2s, msg);
			DEBUG(DBG_QUEUE, " >eventq_m2s: %x", hdr->msg_token);
		}

		DEBUG(DBG_TRACE, "<dispatch_dev2m (loop)");
	}
	if (terminate) {
		event_del(ev_info->ev_dev2m);
		ev_info->ev_dev2m = NULL;
		if (terminate < 2)
			terminate = 2;
		event_add(eventq_m2p.ev, NULL);
		event_add(eventq_m2s.ev, NULL);
		event_add(eventq_m2u.ev, NULL);
	}
	DEBUG(DBG_TRACE, "<dispatch_dev2m");
}

/**
 * This is the event handler for messages that are sent from the master to
 * the upgrade process. It is called when the master to upgrade queue is not
 * empty and the master to upgrade pipe is ready for writing. The handler
 * must make sure that it re-adds the write event as long as the queue is
 * not empty. This is handled by dispatch_write_queue.
 *
 * @param fd The file descriptor of the pipe to the session engine.
 * @param event The event type (see libevent, unused).
 * @param arg The callback argument (see libevent, unused).
 */
static void
dispatch_m2u(int fd, short event __used, void *arg __used)
{
	DEBUG(DBG_TRACE, ">dispatch_m2u");

	if (dispatch_write_queue(&eventq_m2u, fd) <= 0) {
		/* Write not successful. Check if we lost a child. */
		sighandler(SIGCHLD, 0, NULL);
	}
	if (terminate >= 2 && !queue_peek(&eventq_m2u) && !msg_pending(fd))
		shutdown(fd, SHUT_WR);

	DEBUG(DBG_TRACE, "<dispatch_m2u");
}

/**
 * Update everyones checksums and signatures (where possible) on a
 * particular file. This is called after an upgrade completes.
 *
 * @param The request message received from the upgrade process that
 *     contains the path name.
 * @return None.
 *
 * NOTE: This function does not invalidate the sfs tree cache in the
 * policy engine. The entire cache will be dropped at the end of the
 * upgrade.
 */
static void
dispatch_sfs_update_all(struct anoubisd_msg *msg)
{
	anoubisd_sfs_update_all_t	*umsg;
	size_t				 plen;
	char				*path;
	int				 ret;

	if (anoubisd_config.upgrade_mode == ANOUBISD_UPGRADE_MODE_OFF) {
		log_warnx("Dropping SFS_UPDATE_ALL message. "
		    "Upgrade mode is OFF.");
		return;
	}
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
	/* Forcefully NUL terminate the path name. */
	path[plen-1] = 0;

	/*
	 * Update checksums on disk.
	 */
	ret = sfs_update_all(path,
	    abuf_open_frommem(umsg->payload, ANOUBIS_CS_LEN));
	if (ret < 0)
		log_warnx("Cannot update checksums during update");
	if (anoubisd_config.rootkey) {
		struct cert	*cert = cert_get_by_uid(0);

		DEBUG(DBG_UPGRADE, " signature update for %s, cert %p %p",
		    path, cert, cert?cert->privkey:NULL);
		if (cert && cert->privkey) {
			ret = sfs_update_signature(path, cert,
			    abuf_open_frommem(umsg->payload, ANOUBIS_CS_LEN));
			if (ret < 0)
				log_warnx("Cannot update root signature "
				    "during update");
		} else if (anoubisd_config.rootkey_required) {
			log_warnx("ERROR: Upgrade in progress but rootkey "
			    "is not available");
			send_upgrade_ok(0);
		}
	}
	return;
bad:
	log_warnx(" dispatch_sfs_update_all: Malformed message");
	return;
}

/**
 * Event handler function for events coming from the update daemon.
 * This function is called from libevent's main event handling loop.
 *
 * @param fd The file descriptor to read data from.
 * @param event The type of event the occured (not useded).
 * @arg The callback argument. This is a pointer to a struct event_info_main.
 * @return None.
 */
static void
dispatch_u2m(int fd, short event __used, void *arg)
{
	struct anoubisd_msg		*msg;
	struct event_info_main		*ev_info = arg;

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
				send_upgrade_notification();
				upgraded_files = 0;
			}
			enqueue(&eventq_m2p, msg);
			break;
		}
		case ANOUBISD_MSG_SFS_UPDATE_ALL:
			dispatch_sfs_update_all(msg);
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

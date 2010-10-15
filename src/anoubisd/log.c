/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "config.h"

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <err.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <event.h>
#ifdef LINUX
#include <grp.h>
#include <bsdcompat.h>
#endif

#include <anoubis_errno.h>
#include "anoubisd.h"
#include "amsg.h"
#include "aqueue.h"

/**
 * \file
 * This file provides functions and mechanisms for logging to the other
 * anoubis daemon processes. Additionally, it contains the implementation
 * of the logger process.
 *
 * Each anoubis daemon process sends its log messages to the logger process
 * which will forward them to syslog stderr. This is neccessary because syslog
 * can block and this is not a good idea e.g. in the policy engine. If
 * debugging is to stderr, the logger process is basically unused.
 *
 * The logging file descriptor and other logging state is maintained in
 * static variables that are defined in this file below. Each anoubis daemon
 * process starts its life by calling log_init which will set up logging and
 * initialize these global variabels.
 *
 * The logger process itself is started by calling logger_main.
 */

/**
 * This array maps numerical process names to strings. The process name
 * is used to prefix each log message with a message source.
 */
static const char * const procnames[] = {
	[ PROC_MAIN ] = "main",
	[ PROC_POLICY ] = "policy",
	[ PROC_SESSION ] = "session",
	[ PROC_UPGRADE ] = "upgrade",
};

/**
 * Log message file descriptor. This variable is set up by log_init and
 * stores the file descriptor that log messages are sent to in other
 * anoubis daemon processes.
 */
static int		__log_fd = -1;

/**
 * The log write event. This event is initialized by log_init and is added
 * to  the event loop once a log message was added to the log write queue
 * of an anoubis daemon process. Each anoubis daemon process has its own
 * instance of this event.
 */
static struct event	__log_event;

/**
 * This is the outgoing log queue. Each anoubis daemon process has its own
 * instance of this queue.
 */
static Queue		__eventq_log;

/**
 * This variable is used to avoid recurive calls to the logging functions
 * if an error occurs while we are in the process of logging another message.
 * If it is set to true, log messages will simply be dropped.
 */
static int		__logging = 0;

/**
 * This array is used by the logger process to store the event structures of
 * its signal events. It is used in the signal handler to remove all signal
 * events from the event queue.
 */
static struct event	*sigs[10];

/**
 * This is the write dispatcher function for anoubis daemon processes
 * other than the logger. It is called once the log file desciptor becomes
 * ready.
 *
 * @param fd Allways equal to __log_fd. The file descriptor that is ready
 *     for write.
 * @param event Unused. The type of event that occured.
 * @param arg Unused. The callback argument of the event.
 * @return None.
 */
static void
dispatch_log_write(int fd __used, short event __used, void *arg __used)
{
	__logging = 1;
	dispatch_write_queue(&__eventq_log, __log_fd);
	__logging = 0;
}

/**
 * Try to forward messages in the outgoing log queue to syslog directly.
 * This is used to make pending log messages available if the logger process
 * terminates unexpectedly. Note that this might not work for chrooted
 * processes.
 */
void
flush_log_queue(void)
{
	struct anoubisd_msg		*msg;
	openlog(logname, LOG_PID | LOG_NDELAY, LOG_DAEMON);
	while ((msg = dequeue(&__eventq_log))) {
		struct anoubisd_msg_logit	*lmsg;
		lmsg = (struct anoubisd_msg_logit *)msg->msg;
		syslog(lmsg->prio, "%s", lmsg->msg);
		free(msg);
	}
}

/**
 * Initialize logging for an anoubis daemon process. The caller must
 * provide the write end of a file descriptor that is connected to the
 * logger process.
 *
 * @param fd Log messages are sent to this file descriptor.
 * @return None.
 *
 * NOTE: This function should be called exactly once for each anoubis
 *     daemon process.
 */
void
log_init(int fd)
{
	msg_init(fd);
	__log_fd = fd;
	__logging = 0;
	event_set(&__log_event, __log_fd, EV_WRITE, &dispatch_log_write, NULL);
	queue_init(&__eventq_log, &__log_event);
}

/**
 * Log a formatted message. The message is first formatted in a printf
 * like fashion and then either written to stderr or sent to the logger
 * process. This is the core function that handles all logging request
 *
 * @param pri The logging priority. This is ultimately passed as the
 *     first parameter of syslog(3).
 * @param fmt The format string for the message.
 * @param ap The arguments for the format string. The variable length
 *     argument list was already parsed into a va_list. See stdarg.h for
 *     details regarding this issue.
 * @return None.
 */
static void
vlog(int pri, const char *fmt, va_list ap)
{
	char	*nfmt;

	if (__logging)
		return;
	__logging = 1;
	if (debug_stderr) {
		/* best effort in out of mem situations */
		if (asprintf(&nfmt, "%s\n", fmt) == -1) {
			vfprintf(stderr, fmt, ap); /* Flawfinder: ignore */
			fprintf(stderr, "\n");
		} else {
			vfprintf(stderr, nfmt, ap); /* Flawfinder: ignore */
			free(nfmt);
		}
		fflush(stderr);
	} else {
		if (__log_fd >= 0) {
			struct anoubisd_msg	*msg;
			struct anoubisd_msg_logit	*lmsg;
			if (vasprintf(&nfmt, fmt, ap) == -1)
				master_terminate();
			msg = msg_factory(ANOUBISD_MSG_LOGIT,
			    sizeof(struct anoubisd_msg_logit) + strlen(nfmt)+1);
			if (!msg) {
				free(nfmt);
				master_terminate();
			}
			lmsg = (struct anoubisd_msg_logit *)msg->msg;
			lmsg->prio = pri;
			strlcpy(lmsg->msg, nfmt, strlen(nfmt)+1);
			free(nfmt);
			enqueue(&__eventq_log, msg);
		} else {
			vsyslog(pri, fmt, ap);
		}
	}
	__logging = 0;
}

/**
 * Format and send a log message. This is a wrapper around vlog that
 * does not require a parsed va_list but accepts a normal variable length
 * argument list instead.
 *
 * @param pri The log priority.
 * @param fmt The format string for the message.
 * @param ... The format arguments.
 * @return None.
 */
static void
logit(int pri, const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vlog(pri, fmt, ap);
	va_end(ap);
}

/**
 * Create and format a warning message with an error message derived from
 * the current value of errno. The message created is logged as LOG_CRIT.
 * It consists of the message format is specified followed by the error
 * string that corresponds to the current value of errno(3).
 *
 * @param emsg The format string for the message.
 * @param ... The format arguments.
 * @return None.
 */
void
log_warn(const char *emsg, ...)
{
	char	*nfmt;
	va_list	 ap;

	/* best effort to even work in out of memory situations */
	if (emsg == NULL)
		logit(LOG_CRIT, "%s", anoubis_strerror(errno));
	else {
		va_start(ap, emsg);

		if (asprintf(&nfmt, "%s: %s", emsg,
			anoubis_strerror(errno)) == -1) {
			/* we tried it... */
			vlog(LOG_CRIT, emsg, ap);
			logit(LOG_CRIT, "%s", anoubis_strerror(errno));
		} else {
			vlog(LOG_CRIT, nfmt, ap);
			free(nfmt);
		}
		va_end(ap);
	}
}

/**
 * Create and format a warning message. Use this function instead of
 * log_warn if errno is not set to something useful. The message is
 * logged with priority LOG_CRIT.
 *
 * @param emsg The format string for the message.
 * @param ... The format arguments.
 * @return None.
 */
void
log_warnx(const char *emsg, ...)
{
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_CRIT, emsg, ap);
	va_end(ap);
}

/**
 * Create and format an informational message. This is the same as
 * log_warnx except that the priority is only LOG_INFO.
 *
 * @param emsg The format string for the message.
 * @param ... The format arguments.
 * @return None.
 */
void
log_info(const char *emsg, ...)
{
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_INFO, emsg, ap);
	va_end(ap);
}

/**
 * Format and log a debug message. This should not be used directly. Use
 * the DEBUG macro instead. The message is logged with priority LOG_DEBUG.
 *
 * @param emsg The format string for the message.
 * @param ... The format arguments.
 * @return None.
 */
void
log_debug(const char *emsg, ...)
{
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_DEBUG, emsg, ap);
	va_end(ap);
}

/**
 * Log a critical error message, flush pending log messages and exit.
 * The log message is not formatted, the string is logged as is. Consider
 * using log_warnx plus master_terminate() instead of this function.
 *
 * @param emsg The message to log.
 * @return This function never returns.
 */
__dead void
fatal(const char *emsg)
{
	if (emsg == NULL)
		logit(LOG_CRIT, "fatal in %s: %s", procnames[anoubisd_process],
		    anoubis_strerror(errno));
	else
		if (errno)
			logit(LOG_CRIT, "fatal in %s: %s: %s",
			    procnames[anoubisd_process], emsg,
			    anoubis_strerror(errno));
		else
			logit(LOG_CRIT, "fatal in %s: %s",
			    procnames[anoubisd_process], emsg);
	flush_log_queue();
	exit(1);
}

/**
 * Print an error message early in the anoubis daemon startup. This
 * functions unconditionally prints the message to stderr and appends
 * an error string derived from errno. Finally the function exits with
 * exit code 1.
 *
 * @param emsg The error message string. No format characters are allowed.
 * @return This function never returns.
 */
__dead void
early_err(const char *emsg)
{
	if (emsg == NULL) {
		fprintf(stderr, "%s: %s\n", logname, anoubis_strerror(errno));
	 } else {
		if (errno) {
			fprintf(stderr, "%s: %s: %s\n", logname, emsg,
			    anoubis_strerror(errno));
		} else {
			fprintf(stderr, "%s: %s\n", logname, emsg);
		}
	}
	exit(1);
}

/**
 * Print an error message early in the anoubis daemon startup. This
 * functions unconditionally prints the message to stderr and exits
 * with exit code 1.
 *
 * @param emsg The error message string. No format characters are allowed.
 * @return This function never returns.
 */
__dead void
early_errx(const char *emsg)
{
	errno = 0;
	early_err(emsg);
}

/**
 * This is the signal handler used inside the logger process. Once a
 * fatal signal is received it blocks all further signals and removes
 * the signal events from the event loop. This will cause the event loop
 * to exit once all of the logger pipes from the other anoubis daemon
 * processes are closed.
 *
 * @param sig The signal (unused).
 * @param event The type of the event (unused).
 * @param arg The callback argument of the event (unused).
 * @return None.
 */
static void
logger_sighandler(int sig __used, short event __used, void *arg __used)
{
	int		  i;
	sigset_t	  mask;

	/*
	 * Logger will not terminate until all other processes have closed
	 * their log file handles.
	 */
	sigfillset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);
	for (i=0; sigs[i]; ++i)
		signal_del(sigs[i]);
}

/**
 * Dispatch a read event on one of the log file descriptors inside the
 * logger process. This is the event handler for all read events in the
 * logger. Messages are read from the fd and logged to syslog. If EOF
 * is detected on one of the file descriptors a TERM signal is simulated
 * by calling the signal handler manually.
 *
 * @param fd The log file descriptor to read from.
 * @param sig The event type (unused).
 * @param arg The event callback. This is the event itself. We use this
 *     to remove the event from the event queue in case of EOF. The event
 *     is dynamically allocated and must be freed in this case.
 * @return None.
 */
void
dispatch_log_read(int fd, short sig __used, void *arg)
{
	struct anoubisd_msg	*msg;
	__logging = 1;
	while(1) {
		msg = get_msg(fd);
		if (msg == NULL)
			break;
		if (msg->mtype == ANOUBISD_MSG_LOGIT) {
			struct anoubisd_msg_logit	*lmsg;
			lmsg = (struct anoubisd_msg_logit *)msg->msg;
			syslog(lmsg->prio, "%s", lmsg->msg);
		} else {
			syslog(LOG_CRIT, "Bad message type %d in logger",
			    msg->mtype);
		}
		free(msg);
	}
	if (msg_eof(fd)) {
		logger_sighandler(SIGTERM, 0, NULL);
		event_del(arg);
		free(arg);
	}
	__logging = 0;
}

/**
 * Spawn the logger process and return its process ID. This functions
 * sets up signal handling and the event loop in the logger process.
 * The parent process returns immediately.
 *
 * @param pipes The pipes between the different anoubis daemon processes.
 *     This array should have PIPE_MAX elements. All of the file descriptors
 *     in this array will be closed in the child.
 * @param loggers The file descriptors of the pipes between the anoubisd
 *     daemon processes and the logger. The ends at the odd indices belong
 *     to the logger process and will be used to read log messages while
 *     the ends at the even indices will be closed in the logger child.
 *     This array should have PROC_LOGGER elements.
 * @return This function returns the process ID of the logger process in
 *     the parent.
 */
pid_t
logger_main(int pipes[], int loggers[])
{
	int		 pp;
	pid_t		 pid;
	struct event	*ev_logger;
	struct event	 ev_sigterm, ev_sigint, ev_sigquit;
	struct passwd	*pw;
	sigset_t	 mask;
	static int	 logpipeidx[] = { PROC_MAIN+1, PROC_POLICY+1,
			     PROC_SESSION+1, PROC_UPGRADE+1, -1 };

	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		fatal("getpwnam");

	pid = fork();
	if (pid == -1)
		fatal("fork");
		/*NOTREACHED*/
	if (pid)
		return pid;

	(void)event_init();

	anoubisd_process = PROC_LOGGER;

#ifdef LINUX
	dazukofs_ignore();
#endif

	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		fatal("can't drop privileges");

	openlog(logname, LOG_PID | LOG_NDELAY, LOG_DAEMON);
	tzset();
	__log_fd = -1;
	log_info("logger started (pid %d)", getpid());

	signal_set(&ev_sigterm, SIGTERM, logger_sighandler, NULL);
	signal_set(&ev_sigint, SIGINT, logger_sighandler, NULL);
	signal_set(&ev_sigquit, SIGQUIT, logger_sighandler, NULL);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);
	sigs[0] = &ev_sigterm;
	sigs[1] = &ev_sigint;
	sigs[2] = &ev_sigquit;
	sigs[3] = NULL;

	anoubisd_defaultsigset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	for (pp = 0; logpipeidx[pp] >= 0; pp ++) {
		int		idx = logpipeidx[pp];

		if ((ev_logger = malloc(sizeof(struct event))) == NULL)
			fatal("logger_main: event malloc");
		msg_init(loggers[idx]);
		event_set(ev_logger, loggers[idx], EV_READ|EV_PERSIST,
		    &dispatch_log_read, ev_logger);
		event_add(ev_logger, NULL);
		loggers[idx] = -1;
	}
	cleanup_fds(pipes, loggers);

	setproctitle("logger");

	if (event_dispatch() == -1)
		fatal("logger_main: event_dispatch");
	_exit(0);
}

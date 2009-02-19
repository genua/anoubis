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
#include <errno.h>
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

#include "anoubisd.h"
#include "amsg.h"
#include "aqueue.h"

static const char * const procnames[] = {
	"parent",
	"policy",
	"session"
};

static void	logit(int, const char *, ...);
static void	vlog(int, const char *, va_list);

static int		__log_fd = -1;
static struct event	__log_event;
static Queue		__eventq_log;
static int		__logging = 0;

extern char	*logname;
static int	 terminate = 0;

static void
dispatch_log_write(int fd __used, short event __used, void *arg __used)
{
	anoubisd_msg_t		*msg;
	int			 ret;

	__logging = 1;
	if ((msg = queue_peek(&__eventq_log)) == NULL) {
		__logging = 0;
		return;
	}
	ret = send_msg(__log_fd, msg);
	if (ret != 0) {
		msg = dequeue(&__eventq_log);
		free(msg);
	}
	/*
	 * Tricky:  First dequeue pending message, then log and
	 * thus queue a new message.
	 */
	if (ret == -1)
		log_warnx("dispatch_log_write: dropping message %p", msg);

	if (queue_peek(&__eventq_log) || msg_pending(__log_fd))
		event_add(&__log_event, NULL);
	__logging = 0;
}

void
flush_log_queue(void)
{
	anoubisd_msg_t		*msg;
	openlog(logname, LOG_PID | LOG_NDELAY, LOG_DAEMON);
	while ((msg = dequeue(&__eventq_log))) {
		syslog(msg->mtype, "%s", msg->msg);
		free(msg);
	}
}

void
log_init(int fd)
{
	msg_init(fd, "logger");
	__log_fd = fd;
	__logging = 0;
	queue_init(__eventq_log);
	event_set(&__log_event, __log_fd, EV_WRITE, &dispatch_log_write, NULL);
}

static void
logit(int pri, const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vlog(pri, fmt, ap);
	va_end(ap);
}

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
			anoubisd_msg_t	*msg;
			if (vasprintf(&nfmt, fmt, ap) == -1) {
				master_terminate(ENOMEM);
				return;
			}
			msg = msg_factory(pri, strlen(nfmt)+1);
			if (!msg) {
				free(nfmt);
				master_terminate(ENOMEM);
				return;
			}
			strlcpy(msg->msg, nfmt, strlen(nfmt)+1);
			free(nfmt);
			enqueue(&__eventq_log, msg);
			event_add(&__log_event, NULL);
		} else {
			vsyslog(pri, fmt, ap);
		}
	}
	__logging = 0;
}

void
log_warn(const char *emsg, ...)
{
	char	*nfmt;
	va_list	 ap;

	/* best effort to even work in out of memory situations */
	if (emsg == NULL)
		logit(LOG_CRIT, "%s", strerror(errno));
	else {
		va_start(ap, emsg);

		if (asprintf(&nfmt, "%s: %s", emsg, strerror(errno)) == -1) {
			/* we tried it... */
			vlog(LOG_CRIT, emsg, ap);
			logit(LOG_CRIT, "%s", strerror(errno));
		} else {
			vlog(LOG_CRIT, nfmt, ap);
			free(nfmt);
		}
		va_end(ap);
	}
}

void
log_warnx(const char *emsg, ...)
{
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_CRIT, emsg, ap);
	va_end(ap);
}

void
log_info(const char *emsg, ...)
{
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_INFO, emsg, ap);
	va_end(ap);
}

void
log_debug(const char *emsg, ...)
{
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_DEBUG, emsg, ap);
	va_end(ap);
}

__dead void
fatal(const char *emsg)
{
	if (emsg == NULL)
		logit(LOG_CRIT, "fatal in %s: %s", procnames[anoubisd_process],
		    strerror(errno));
	else
		if (errno)
			logit(LOG_CRIT, "fatal in %s: %s: %s",
			    procnames[anoubisd_process], emsg, strerror(errno));
		else
			logit(LOG_CRIT, "fatal in %s: %s",
			    procnames[anoubisd_process], emsg);
	flush_log_queue();
	if (anoubisd_process == PROC_MAIN)
		exit(1);
	else				/* parent copes via SIGCHLD */
		_exit(1);
}

__dead void
fatalx(const char *emsg)
{
	errno = 0;
	fatal(emsg);
}

void
early_err(int eval, const char *emsg)
{
	if (emsg == NULL) {
		fprintf(stderr, "%s: %s\n", logname, strerror(errno));
	 } else {
		if (errno) {
			fprintf(stderr, "%s: %s: %s\n", logname, emsg,
			    strerror(errno));
		} else {
			fprintf(stderr, "%s: %s\n", logname, emsg);
		}
	}

	exit(eval);
}

void
early_errx(int eval, const char *emsg)
{
	errno = 0;
	early_err(eval, emsg);
}

void dispatch_log_read(int fd, short sig __used, void *arg)
{
	anoubisd_msg_t * msg;
	__logging = 1;
	while(1) {
		msg = get_msg(fd);
		if (msg == NULL)
			break;
		syslog(msg->mtype, "%s", msg->msg);
		free(msg);
	}
	if (msg_eof(fd))
		event_del(arg);
	__logging = 0;
}

static void
logger_sighandler(int sig __used, short event __used, void *arg)
{
	int		  i;
	struct event	**sigs;
	sigset_t	  mask;

	/*
	 * Logger will not terminate until all other processes have closed
	 * their log file handles.
	 */
	terminate = 1;
	sigfillset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);
	sigs = arg;
	for (i=0; sigs[i]; ++i)
		signal_del(sigs[i]);
}

pid_t
logger_main(struct anoubisd_config *conf __used, int pipe_m2l[2],
    int pipe_p2l[2],
    int pipe_s2l[2])
{
	pid_t pid;
	struct event	 ev_m2l, ev_p2l, ev_s2l;
	struct event	 ev_sigterm, ev_sigint, ev_sigquit;
	struct passwd	*pw;
	sigset_t	 mask;
	static struct event *	sigs[10];

	if ((pw = getpwnam(ANOUBISD_USER)) == NULL)
		fatal("getpwnam");

	pid = fork();
	if (pid < 0)
		fatal("fork");
		/*NOTREACHED*/
	if (pid)
		return pid;
	anoubisd_process = PROC_LOGGER;
	setproctitle("logger");

	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		fatal("can't drop privileges");

	openlog(logname, LOG_PID | LOG_NDELAY, LOG_DAEMON);
	tzset();
	__log_fd = -1;
	log_info("logger started (pid %d)", getpid());

	(void)event_init();

	signal_set(&ev_sigterm, SIGTERM, logger_sighandler, sigs);
	signal_set(&ev_sigint, SIGINT, logger_sighandler, sigs);
	signal_set(&ev_sigquit, SIGQUIT, logger_sighandler, sigs);
	signal_add(&ev_sigterm, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigquit, NULL);
	sigs[0] = &ev_sigterm;
	sigs[1] = &ev_sigint;
	sigs[2] = &ev_sigquit;
	sigs[3] = NULL;

	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGQUIT);
	sigprocmask(SIG_SETMASK, &mask, NULL);

	close(pipe_m2l[1]);
	close(pipe_p2l[1]);
	close(pipe_s2l[1]);

	msg_init(pipe_m2l[0], "m2l");
	msg_init(pipe_p2l[0], "p2l");
	msg_init(pipe_s2l[0], "s2l");

	event_set(&ev_m2l, pipe_m2l[0], EV_READ | EV_PERSIST,
	    &dispatch_log_read, &ev_m2l);
	event_add(&ev_m2l, NULL);
	event_set(&ev_p2l, pipe_p2l[0], EV_READ | EV_PERSIST,
	    &dispatch_log_read, &ev_p2l);
	event_add(&ev_p2l, NULL);
	event_set(&ev_s2l, pipe_s2l[0], EV_READ | EV_PERSIST,
	    &dispatch_log_read, &ev_s2l);
	event_add(&ev_s2l, NULL);

	if (event_dispatch() == -1)
		fatal("logger_main: event_dispatch");
	_exit(0);
}

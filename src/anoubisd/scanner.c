/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include <config.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <paths.h>

#include <stdlib.h>
#include <stdio.h>
#include <event.h>
#include <pwd.h>
#include <syslog.h>
#include <inttypes.h>

#ifdef LINUX
#include "linux/anoubis.h"
#include "linux/anoubis_playground.h"
#include <attr/xattr.h>
#include <bsdcompat.h>
#else
#include "dev/anoubis.h"
#endif

#include <anoubis_alloc.h>
#include <anoubisd.h>
#include <amsg.h>
#include <aqueue.h>

/**
 * This structure is used to describe one active scanner child.
 * All active scanner children are maintained in a linked list.
 */
struct scanproc {
	/**
	 * The link to the next structure in the global scanner list
	 * (only used in the master process).
	 */
	LIST_ENTRY(scanproc)		 next;

	/**
	 * The token of the scan request that is handled by this scanner
	 * child.
	 */
	uint64_t			 token;

	/**
	 * An open file descriptor of the file that is being scanned.
	 */
	int				 scanfile;

	/**
	 * The pipe between the scanner child and the anoubis daemon master.
	 * The read end is held open by the master, the write end by the scanner
	 * child.
	 */
	int				 pipe[2];

	/**
	 * The user ID of the user requesting the scan. There can be at most
	 * one active scanner per user.
	 */
	uint64_t			 uid;

	/**
	 * The process ID of the scanner child process (only valid in the
	 * master process).
	 */
	pid_t				 childpid;

	/**
	 * The libevent read event used to read messages from the scanner
	 * child in in the master process (only valid in the master process).
	 */
	struct event			 event;

	/**
	 * The scanner reply message is stored in this message buffer.
	 * The buffer contains at most one anoubisd_msg (only used in the
	 * master process).
	 */
	struct abuf_buffer		 msgbuf;

	/**
	 * The offset into the msgbuf buffer where the next byte received
	 * from the scanner child must be stored (only used in the master
	 * process).
	 */
	unsigned int			 msgoff;
};

/**
 * The global list of active scanner child processes.
 */
LIST_HEAD(, scanproc)	scanprocs = LIST_HEAD_INITIALIZER(scanproc);

/**
 * This structure encapsulates the result of a single scanner. It is
 * only used internally in scanner_run and scanner_main.
 */
struct scanresult {
	/**
	 * The next element in the scanner result list.
	 */
	CIRCLEQ_ENTRY(scanresult)	 next;

	/**
	 * A buffer that contains the error text of this scanner.
	 */
	struct abuf_buffer		 text;
	int				 off;

	/**
	 * The exit code of the scanner sub process.
	 */
	int				 exitcode;

	/**
	 * A pointer back to the scanner structure. This is ok because
	 * the scanner list is read only in the scanner sub-process. Only
	 * the parent process (anoubisd master) reconfigures the scanner list.
	 */
	struct anoubisd_pg_scanner	*scanner;
};

static char *envp[] = {
	"PATH=" _PATH_STDPATH,
	NULL, /* will be set to HOME */
	NULL
};

/**
 * Search the scanner list for an entry with the given user ID and return
 * this entry.
 *
 * @param uid The used ID to look for.
 * @return NULL if there is no active scanner of this user, a pointer to
 *     the scanproc structure if there is.
 */
static struct scanproc *
scanproc_by_uid(uid_t uid)
{
	struct scanproc		*sp;

	LIST_FOREACH(sp, &scanprocs, next) {
		if (sp->uid == uid)
			return sp;
	}
	return NULL;
}

/**
 * Set to one if the parent requested the termination of this process
 * by sending SIGTERM or SIGINT.
 */
static volatile sig_atomic_t		terminate;

/**
 * Dummy handler for signals. The handler does nothing. It is used here
 * instead of SIG_IGN to make sure that the signal interrupts system calls.
 * The interrupt is used to check for timeouts.
 */
static void
sighandler(int sig)
{
	if (sig == SIGTERM || sig == SIGINT)
		terminate = 1;
}

/**
 * This is the read event handler that reads data received from the
 * scanner child into the appropriate buffer. This function will read
 * up to one result message from the file descriptor and store it in
 * the message buffer of the scanproc structure. Once the message is
 * complete the event is removed from the event loop and the pipe is
 * closed. At this point sp->pipe[0] in the scanproc structure will be
 * set to -1.
 *
 * @param fd The file descriptor to read from.
 * @param event The event type (see libevent, unused).
 * @param arg The callback data of the event. This is a pointer to
 *     the scanproc structure for this child.
 * @return None.
 *
 * NOTE: This function is called implicitly from the libevent event loop
 * if data becomes ready on the file descriptor. However, it is called
 * explicitly if the scanner process exits, too.
 */
static void
dispatch_scand2m(int fd, short event __used, void *arg)
{
	struct scanproc		*sp = arg;

	if (sp->pipe[0] < 0)
		return;
	if (sp->pipe[0] != fd) {
		log_warnx("Bad file descriptior in dispatch_scand2m");
		master_terminate();
	}
	DEBUG(DBG_TRACE, ">dispatch_scand2m");
	while (1) {
		int		 ret;
		unsigned int	 len;
		void		*ptr;

		DEBUG(DBG_PG, " dispatch_scand2m: msgoff=%d buflen=%ld",
		    sp->msgoff, abuf_length(sp->msgbuf));
		if (sp->msgoff < sizeof(struct anoubisd_msg)) {
			len = sizeof(struct anoubisd_msg);
		} else {
			struct anoubisd_msg		*msg;

			msg = abuf_cast(sp->msgbuf, struct anoubisd_msg);
			if (!msg)
				master_terminate();
			if (msg->size > 8000)
				msg->size = 8000;
			len = msg->size;
		}
		if (sp->msgoff >= len)
			break;
		if (len > abuf_length(sp->msgbuf))
			sp->msgbuf = abuf_realloc(sp->msgbuf, len);
		ptr = abuf_toptr(sp->msgbuf, sp->msgoff, len-sp->msgoff);
		ret = read(fd, ptr, len-sp->msgoff);
		if (ret < 0) {
			DEBUG(DBG_TRACE, "<dispatch_scand2m: err=%d", errno);
			return;
		}
		if (ret == 0)
			break;
		sp->msgoff += ret;
	}
	event_del(&sp->event);
	close(sp->pipe[0]);
	sp->pipe[0] = -1;
	DEBUG(DBG_TRACE, "<dispatch_scand2m: done");
}

/**
 * This function handles the exit of a scanner child. It first calls
 * the read event handler until EOF is received or the reply message is
 * complete. It then processes the scan result and sends an appropriate
 * message to the policy engine. If the scan was successful, the security
 * label is removed, too.
 *
 * @param pid The pid of the process that exited.
 * @param status The exit status of the child.
 * @param evinfo The event information of the master process.
 * @param queue The event queue that the reply message should be sent to.
 * @return True if the pid matched a scanner child.
 */
int
anoubisd_scanproc_exit(pid_t pid, int status,
    struct event_info_main *evinfo __used, Queue *queue)
{
	struct scanproc				*sp;
	int					 err = -EFAULT, extra;
	struct anoubisd_msg			*msg;
	struct anoubisd_msg_pgcommit_reply	*pgrep, *scanreply = NULL;

	DEBUG(DBG_PG, " scanproc exit: pid=%d status=%x", pid, status);
	LIST_FOREACH(sp, &scanprocs, next) {
		if (sp->childpid && sp->childpid == pid)
			break;
	}
	if (sp == NULL)
		return 0;
	DEBUG(DBG_PG, " scanproc exit: pid=%d status=%x found", pid, status);
	/* Handle negative exist status. */
	if (!WIFEXITED(status) || WEXITSTATUS(status)) {
		log_warnx("Scanner sub processes exited with code %d\n",
		    status);
		event_del(&sp->event);
		close(sp->pipe[0]);
		sp->pipe[0] = -1;
		err = -EINTR;
		goto out;
	} else {
		while(sp->pipe[0] >= 0)
			dispatch_scand2m(sp->pipe[0], 0, sp);
	}
	err = -EFAULT;
	if (sp->msgoff < sizeof(struct anoubisd_msg))
		goto out;
	msg = abuf_cast(sp->msgbuf,  struct anoubisd_msg);
	if (!msg)
		goto out;
	if (msg->mtype != ANOUBISD_MSG_PGCOMMIT_REPLY)
		goto out;
	if ((int)abuf_length(sp->msgbuf) != msg->size)
		goto out;
	if (!amsg_verify_nonfatal(msg))
		goto out;
	scanreply = (struct anoubisd_msg_pgcommit_reply *)(msg->msg);
	err = 0;
	if (scanreply->error)
		goto out;
	log_info("scanning request of user %" PRId64 " successful", sp->uid);
#ifdef LINUX
	if (ioctl(evinfo->anoubisfd, ANOUBIS_SCAN_SUCCESS, sp->scanfile) < 0) {
		err = -errno;
		goto out;
	}
	if (fremovexattr(sp->scanfile, "security.anoubis_pg") < 0) {
		err = -errno;
		goto out;
	}
#else
	err = -ENOSYS;
#endif
out:
	/*
	 * if err != 0 we send a simply error message without scanner data.
	 * If err == 0 we take the error code from scanreply->error and
	 * copy any data that might be present in scanreply.
	 */
	extra = 0;
	if (err == 0)
		extra = scanreply->len;
	msg = msg_factory(ANOUBISD_MSG_PGCOMMIT_REPLY,
	    sizeof(struct anoubisd_msg_pgcommit_reply) + extra);
	pgrep = (struct anoubisd_msg_pgcommit_reply *)msg->msg;
	if (err == 0) {
		pgrep->error = scanreply->error;
	} else {
		pgrep->error = -err;
	}
	pgrep->len = extra;
	pgrep->token = sp->token;
	if (extra)
		memcpy(pgrep->payload, scanreply->payload, extra);
	enqueue(queue, msg);
	DEBUG(DBG_QUEUE, " >eventq_m2p: commit reply error=%d token=%" PRId64,
	    pgrep->error, pgrep->token);
	close(sp->scanfile);
	LIST_REMOVE(sp, next);
	abuf_free(sp->msgbuf);
	abuf_free_type(sp, struct scanproc);
	return 1;
}

/**
 * Run a single scanner synchronously in a sub process and report the
 * result of the scan. The output of the scanner is limited to 1000 bytes,
 * the rest of the output will be ignored.
 *
 * The file to scan is provided to the scanner on stdin, the output
 * of the scanner is collected on stdout and stderr. All of the standard
 * file descriptors must be open before this function is called.
 *
 * Two special scanners are supported: "allow" and "deny". These scanners
 * return success and failure, respectively. They do not spwan another child
 * process.
 *
 * NOTE: Memory management is not important here because this all
 * happens in sub-processes that will exit after the scan.
 *
 * @param scanner The description of the scanner to run.
 * @param fd The file descriptor to scan.
 * @param toclose A list of file desciptors (terminated by -1) that will
 *     be closed in the child process. The child process always closes
 * @return  The result of the scan.
 */
static struct scanresult *
scanner_run(struct anoubisd_pg_scanner *scanner, int fd, int toclose[])
{
	struct scanresult	*result;
	pid_t			 pid;
	int			 fds[2];
	int			 status;
	time_t			 starttime, now;
	time_t			 timeout = anoubisd_config.scanner_timeout;
	const char		*str;

	result = abuf_alloc_type(struct scanresult);
	if (result == NULL)
		return NULL;
	result->exitcode = 2;
	result->off = 0;
	result->text = ABUF_EMPTY;
	result->scanner = scanner;

	/* Handle special scanners "allow" and "deny". */
	if (strcasecmp(scanner->path, "allow") == 0) {
		result->exitcode = 0;
		return result;
	}
	if (strcasecmp(scanner->path, "deny") == 0) {
		result->exitcode = 1;
		return result;
	}

	result->text = abuf_alloc(1000);
	if (time(&starttime) < 0)
		goto err;
	if (lseek(fd, 0, SEEK_SET) < 0)
		goto err;

	/*
	 * Create the pipe for the scanner output. The write end
	 * will be mapped to stdout and stderr of the scanner.
	 */
	if (pipe(fds) < 0)
		goto err;

	pid = fork();
	if (pid < 0) {
		close(fds[0]);
		close(fds[1]);
		goto err;
	}
	if (pid == 0) {
		int		 i;
		char		*argv[2];

		/* Child process that runs the actual scanner. */
		for (i=0; toclose[i] >= 0; ++i)
			close(toclose[i]);
		close(0);
		if (dup(fd) != 0)
			_exit(2);
		close(fd);
		close(1);
		close(2);
		if (dup(fds[1]) != 1)
			_exit(2);
		if (dup(fds[1]) != 2)
			_exit(2);
		close(fds[0]);
		close(fds[1]);

		argv[0] = scanner->path;
		argv[1] = NULL;
		execve(scanner->path, argv, envp);
		_exit(2);
	}
	/*
	 * Parent process:
	 * Read scanner output and store it in the result. This is done
	 * synchronously. Note that this is one of the very few instances
	 * in the anoubis daemon where the file descriptor that we read from
	 * is delibarately set to be blocking.
	 */
	close(fds[1]);
	while (1) {
		int		 ret, len;
		char		*ptr;

		len = abuf_length(result->text) - result->off;
		if (len <= 0)
			break;
		ptr = abuf_toptr(result->text, result->off, len);
		ret = read(fds[0], ptr, len);
		if (ret == 0)
			break;
		if (ret < 0 && errno != EAGAIN && errno != EINTR) {
			terminate = 1;
			break;
		}
		/* Stop at the first NUL byte in the scanner output. */
		for (len=0; len<ret; ++len)
			if (ptr[len] == 0)
				break;
		result->off += len;
		if (len < ret)
			break;
		/* Zero means: No timeout. */
		if (timeout && (time(&now) < 0 || now - starttime > timeout))
			terminate = 1;
		/* Can be set by the signal handler, too. */
		if (terminate)
			break;
	}
	close(fds[0]);
	if (terminate)
		kill(pid, SIGTERM);
	abuf_limit(&result->text, result->off);
	while (wait(&status) != pid) {
		if (terminate) {
			kill(pid, SIGKILL);
		} else if (timeout) {
			if (time(&now) < 0 || now - starttime > timeout) {
				kill(pid, SIGTERM);
				terminate = 1;
			}
		}
	}
	if (WIFEXITED(status))
		result->exitcode = WEXITSTATUS(status);
	/* Forceful termination can  never result in a successful scan. */
	if (terminate && result->exitcode == 0)
		result->exitcode = 2;
	return result;
err:
	result->exitcode = 2;
	str = strerror(errno);
	result->off = strlen(str);
	if (result->off > (int)abuf_length(result->text))
		result->off = abuf_length(result->text);
	abuf_copy_tobuf(result->text, str, result->off);
	abuf_limit(&result->text, result->off);
	return result;
}

/**
 * The main loop of the scanner. This function spawns a child process
 * for each scanner that is configured and collects all scanner results.
 * The scanner results are then combined into a single reply message with
 * a payload of at most 8000 bytes. Each scanner that wants to report an
 * error can contribute proportionally to these 8000 bytes.
 *
 * The format of the result is a sequence of strings. There are two
 * strings per scanner, the first is the scanner's description from the
 * config file and the second is the error output of the scanner. Only
 * outpt of failed scanners is reported.
 *
 * Scanners that are not required (i.e. recommended) are only run if flags
 * is zero. An empty list of scanners means that the file must not be
 * committed.
 *
 * NOTE: This function runs in a sub process of the anoubis daemon master
 * that exits immediately after the scan. This means that we need not care
 * about freeing of memory. We allocate only a few kilobytes per scanner.
 *
 * @param sp The description of this scanner process.
 * @param flags If flags is non-zero only recommended scanners are run.
 * @return This function never returns. It must call _exit instead.
 *
 * NOTE: It is not allowed to use functions that use the anoubis daemon
 * NOTE: internal logger process from within scanner_main. Use syslog
 * NOTE: directly instead. The list of forbidden functions includes
 * DEBUG(...), log_* and fatal.
 */
__dead static void
scanner_main(struct scanproc *sp, int flags)
{
	int					 i, off;
	struct anoubisd_msg			*msg;
	struct anoubisd_msg_pgcommit_reply	*pgrep;
	void					*buf;
	int					 err = 0;
	CIRCLEQ_HEAD(, scanresult)		 results;
	struct scanresult			*result;
	struct anoubisd_pg_scanner		*scanner;
	int					 toclose[2];
	struct abuf_buffer			 resbuf = ABUF_EMPTY;
	int					 nresults = 0, limit = 0;
	struct sigaction			 action;
	struct itimerval			 timer;

	CIRCLEQ_INIT(&results);
	/*
	 * NOTE: _SC_OPEN_MAX is the maximum number of open _files_ not
	 * NOTE: file descriptors. However, file descriptors are handed
	 * NOTE: out sequentially except for the dup2 case. As we do not
	 * NOTE: use dup2 with larger numbers we should be safe here.
	 */
	for (i=3; i<sysconf(_SC_OPEN_MAX); ++i) {
		if (i == sp->pipe[1])
			continue;
		if (i == sp->scanfile)
			continue;
		close(i);
	}

	/* Setup signals and timeouts. */
	action.sa_flags = 0;		/* No SA_RESTART! */
	action.sa_handler = sighandler;
	sigemptyset(&action.sa_mask);
	if (sigaction(SIGINT, &action, NULL) < 0
	    || sigaction(SIGTERM, &action, NULL) < 0
	    || sigaction(SIGALRM, &action, NULL) < 0)
		_exit(2);

	/* Fire every ten seconds. */
	timer.it_interval.tv_sec = 10;
	timer.it_interval.tv_usec = 0;
	timer.it_value = timer.it_interval;
	if (setitimer(ITIMER_REAL, &timer, NULL) < 0)
		_exit(2);

	toclose[0] = sp->pipe[1];
	toclose[1] = -1;

	setproctitle("scanner uid=%d", (int)sp->uid);

	err = -EPERM;
	if (CIRCLEQ_EMPTY(&anoubisd_config.pg_scanner))
		goto out;
	err = -ENOMEM;
	CIRCLEQ_FOREACH(scanner, &anoubisd_config.pg_scanner, link) {
		/* Skip non-required scanners if flags are non-zero. */
		if (flags && !scanner->required)
			continue;
		result = scanner_run(scanner, sp->scanfile, toclose);
		if (result == NULL)
			goto out;
		CIRCLEQ_INSERT_TAIL(&results, result, next);
		if (result->exitcode != 0)
			nresults++;
		if (terminate) {
			err = -EINTR;
			goto out;
		}
	}
	err = -EFAULT;
	if (nresults) {
		resbuf = abuf_alloc(8000);
		limit = abuf_length(resbuf) / nresults;
	}
	if (nresults && limit < 100)
		goto out;
	err = 0;
	off = 0;
	CIRCLEQ_FOREACH(result, &results, next) {
		int		 dlen;
		char		*ptr;

		scanner = result->scanner;
		if (result->exitcode == 0)
			continue;
		if (scanner->required) {
			err = -EPERM;
		} else {
			if (err == 0)
				err = -EAGAIN;
		}
		dlen = strlen(scanner->description) + 1;
		if (dlen > limit/2)
			dlen = limit/2;
		ptr = abuf_toptr(resbuf, off, dlen);
		memcpy(ptr, scanner->description, dlen);
		ptr[dlen-1] = 0;
		off += dlen;
		dlen = limit - dlen - 1;
		if (dlen > (int)abuf_length(result->text))
			dlen = abuf_length(result->text);
		abuf_copy_part(resbuf, off, result->text, 0, dlen);
		off += dlen;
		/* Append a NUL byte. */
		abuf_copy_tobuf(abuf_open(resbuf, off), "", 1);
		off++;
	}
	abuf_limit(&resbuf, off);

out:
	msg = msg_factory(ANOUBISD_MSG_PGCOMMIT_REPLY,
	    sizeof(struct anoubisd_msg_pgcommit_reply) + abuf_length(resbuf));
	if (msg == NULL)
		exit(2);
	pgrep = (struct anoubisd_msg_pgcommit_reply *)msg->msg;
	pgrep->error = -err;
	pgrep->len = abuf_length(resbuf);
	pgrep->token = 0;
	abuf_copy_frombuf(pgrep->payload, resbuf, abuf_length(resbuf));
	buf = msg;
	off = 0;
	while (off < msg->size) {
		int ret = write(sp->pipe[1], buf+off, msg->size-off);
		if (ret < 0 && errno != EAGAIN && errno != EINTR)
			_exit(2);
		off += ret;
	}
	_exit(0);
}

/**
 * Start a new scanner child for a given file that is alread open.
 * This function spawns a scanner child and allocates and fills the
 * scanproc structure.
 *
 * @param token The token to use for the reply to this scan requst.
 * @param fd The file to scan.
 * @param auth_uid The user ID of the user requesting this scan.
 * @param flags Request flags. Currently unused. This will be used to
 *     tell the scanner child if recommended scanners should be run.
 * @return Zero if the scanner child was started, a negative error code
 *     otherwise. The caller should create an answer for the scan request
 *     if the function returns an error.
 */
int
anoubisd_scan_start(uint64_t token, int fd, uint64_t auth_uid, int flags)
{
	struct scanproc		*sp = scanproc_by_uid(auth_uid);
	int			 ret;

	/* Scan already in progress */
	if (sp)
		return -EBUSY;
	sp = abuf_alloc_type(struct scanproc);
	if (sp == NULL)
		return -ENOMEM;
	sp->msgbuf = ABUF_EMPTY;
	sp->msgoff = 0;
	sp->token = token;
	sp->scanfile = fd;
	sp->uid = auth_uid;
	if (pipe(sp->pipe) < 0 || fcntl(sp->pipe[0], F_SETFL, O_NONBLOCK) < 0) {
		ret = -errno;
		abuf_free(sp->msgbuf);
		abuf_free_type(sp, struct scanproc);
		return ret;
	}
	sp->childpid = fork();
	if (sp->childpid < 0) {
		ret = -errno;
		close(sp->pipe[0]);
		close(sp->pipe[1]);
		abuf_free(sp->msgbuf);
		abuf_free_type(sp, struct scanproc);
		return ret;
	}
	if (sp->childpid == 0) {
		struct passwd	*pw;

		/* Drop privileges */
		if ((pw = getpwnam(SCAN_USER)) == NULL) {
			syslog(LOG_CRIT, "getpwnam failed for %s: %s",
			    SCAN_USER, strerror(errno));
			_exit(1);
		}
		if (setgroups(1, &pw->pw_gid) ||
		    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
		    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid)) {
			syslog(LOG_CRIT, "can't drop privileges: %s",
			    strerror(errno));
			_exit(1);
		}

		close(sp->pipe[0]);
		terminate = 0;

		if (chdir(pw->pw_dir) != 0) {
			syslog(LOG_CRIT, "can't change to new HOME %s: %s",
			    pw->pw_dir, strerror(errno));
			_exit(1);
		}
		if (asprintf(&envp[1], "HOME=%s", pw->pw_dir) == -1) {
			syslog(LOG_CRIT, "out of memory");
			_exit(1);
		}

		scanner_main(sp, flags);
		_exit(126);
	}
	close(sp->pipe[1]);
	event_set(&sp->event, sp->scanfile, EV_READ|EV_PERSIST,
	    dispatch_scand2m, sp);
	event_add(&sp->event, NULL);
	LIST_INSERT_HEAD(&scanprocs, sp, next);
	DEBUG(DBG_PG, " scanproc stared: pid=%d", sp->childpid);
	return 0;
}

/**
 * Detach from all scanner children and send them a SIGTERM. This
 * should case the children to terminate in time. As this is only called
 * during daemon termination, we do not wait for the termination of the
 * scanner children. This is left to the child process run by scanner_main.
 *
 * @param None.
 * @return None.
 */
void
anoubisd_scanners_detach(void)
{
	struct scanproc		*sp;

	while (!LIST_EMPTY(&scanprocs)) {
		sp = LIST_FIRST(&scanprocs);
		LIST_REMOVE(sp, next);
		if (sp->pipe[0] >= 0) {
			event_del(&sp->event);
			close(sp->pipe[0]);
			sp->pipe[0] = -1;
		}
		kill(sp->childpid, SIGTERM);
		close(sp->scanfile);
		abuf_free(sp->msgbuf);
		abuf_free_type(sp, struct scanproc);
	}
}

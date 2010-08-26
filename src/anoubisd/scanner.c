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
#include <inttypes.h>
#include <sys/ioctl.h>
#include <event.h>

#ifdef LINUX
#include "linux/anoubis.h"
#include "linux/anoubis_playground.h"
#include <attr/xattr.h>
#else
#include "dev/anoubis.h"
#endif

#include <anoubis_alloc.h>
#include <anoubisd.h>
#include <amsg.h>
#include <aqueue.h>

/**
 * This structure is used to describe one active scanner child.
 * All active scanner children are maintained in a linked list. Fields are:
 *
 * next: The link to the next structure in the global scanner list
 *     (only used in the master process).
 * token: The token of the scan request that is handled by this scanner
 *     child.
 * scanfile: An open file descriptor of the file that is being scanned.
 * pipe: The pipe between the scanner child and the anoubis daemon master.
 *     The read end is held open by the master, the write end by the scanner
 *     child.
 * uid: The user ID of the user requesting the scan. There can be at most
 *     one active scanner per user.
 * childpid: The process ID of the scanner child process (only valid in the
 *     master process).
 * event: The libevent read event used to read messages from the scanner
 *     child in in the master process (only valid in the master process).
 * msgbuf: The scanner reply message is stored in this message buffer.
 *     The buffer contains at most one anoubisd_msg (only used in the
 *     master process).
 * msgoff: The offset into the msgbuf buffer where the next byte received
 *     from the scanner child must be stored (only used in the master
 *     process).
 */
struct scanproc {
	LIST_ENTRY(scanproc)		 next;
	uint64_t			 token;
	int				 scanfile;
	int				 pipe[2];
	uint64_t			 uid;
	pid_t				 childpid;
	struct event			 event;
	struct abuf_buffer		 msgbuf;
	unsigned int			 msgoff;
};

/**
 * The global list of active scanner child processes.
 */
LIST_HEAD(, scanproc)	scanprocs = LIST_HEAD_INITIALIZER(scanproc);

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
		master_terminate(EINVAL);
		return;
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
			if (!msg) {
				master_terminate(ENOMEM);
				return;
			}
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

	LIST_FOREACH(sp, &scanprocs, next) {
		if (sp->childpid && sp->childpid == pid)
			break;
	}
	if (sp == NULL)
		return 0;
	/* Handle negative exist status. */
	if (!WIFEXITED(status) || WEXITSTATUS(status)) {
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
		DEBUG(DBG_PG, " XXX %d %d", evinfo->anoubisfd, sp->scanfile);
		err = -errno;
		goto out;
	}
	if (fremovexattr(sp->scanfile, "security.anoubis_pg") < 0) {
		DEBUG(DBG_PG, " XXX YYY %dd", sp->scanfile);
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
 * The main loop of the scanner. This is a dummy implementation.
 *
 * @return This function never returns. It must call _exit instead.
 */
__dead static void
scanner_main(struct scanproc *sp)
{
	int					 i, off;
	struct anoubisd_msg			*msg;
	struct anoubisd_msg_pgcommit_reply	*pgrep;
	void					*buf;
	int					 err = 0;

	/* XXX CEH: How do we find out the upper limit here? */
	for (i=0; i<1024;  ++i) {
		if (i == sp->pipe[1])
			continue;
		if (i == sp->scanfile)
			continue;
		close(i);
	}
	msg = msg_factory(ANOUBISD_MSG_PGCOMMIT_REPLY,
	    sizeof(struct anoubisd_msg_pgcommit_reply));
	if (msg == NULL)
		exit(2);
	pgrep = (struct anoubisd_msg_pgcommit_reply *)msg->msg;
	pgrep->error = -err;
	pgrep->len = 0;
	pgrep->token = 0;
	buf = msg;
	off = 0;
	while (off < msg->size) {
		int ret = write(sp->pipe[1], buf+off, msg->size-off);
		if (ret < 0 && errno != EAGAIN)
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
anoubisd_scan_start(uint64_t token, int fd, uint64_t auth_uid,
    int flags __attribute__((unused)))
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
		close(sp->pipe[0]);
		scanner_main(sp);
		log_warnx("scanner_main returned!");
		_exit(1);
	}
	close(sp->pipe[1]);
	event_set(&sp->event, sp->scanfile, EV_READ|EV_PERSIST,
	    dispatch_scand2m, sp);
	event_add(&sp->event, NULL);
	LIST_INSERT_HEAD(&scanprocs, sp, next);
	return 0;
}

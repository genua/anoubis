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

LIST_HEAD(, scanproc)	scanprocs = LIST_HEAD_INITIALIZER(scanproc);

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

static int
scanproc_exit(pid_t pid, int status, struct event_info_main *evinfo __used,
    Queue *queue)
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

/*
 * NOTE: Currently we cannot use log_* functions or DEBUG in scanner main!
 * XXX CEH: This is a dummy implementation that always returns success.
 */
static void
scanner_main(struct scanproc *sp)
{
	int					 i, off;
	struct anoubisd_msg			*msg;
	struct anoubisd_msg_pgcommit_reply	*pgrep;
	void					*buf;
	int					 err = 0;

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

int
anoubisd_scanner_exit(struct event_info_main *evinfo, Queue *queue)
{
	int	handled = 0;

	while (1) {
		int	status;
		pid_t	ret = waitpid(-1, &status, WNOHANG);

		if (ret == 0)
			break;
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		/* We got a process.  */
		if (scanproc_exit(ret, status, evinfo, queue))
			handled = 1;
	}
	return handled;
}

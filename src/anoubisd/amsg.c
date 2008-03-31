/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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
/* on glibc 2.6+, event.h uses non C89 types :/ */
#include "splint-includes.h"
#endif /* S_SPLINT_S */

#include <sys/types.h>
#include <errno.h>
#include <event.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include "anoubisd.h"
#include "amsg.h"

/* simple buffered file access, remember the event */
void
msg_init(int fd, struct event *ev, char *name)
{
	int idx;

	/* already initialized? */
	for (idx=0; idx < MSG_BUFS; idx++)
		if (fds[idx].fd == fd)
			return;

	if (fcntl(fd, F_SETFL, O_NONBLOCK))
		log_warn("O_NONBLOCK not set");

	for (idx=0; idx < MSG_BUFS; idx++) {
		if (fds[idx].rbufp == NULL) {

			if ((fds[idx].rbufp = malloc(MSG_BUF_SIZE*2)) == NULL) {
				log_warn("msg_init: can't allocate memory");
				master_terminate(ENOMEM);
				return;
			}
			fds[idx].rheadp = fds[idx].rtailp = fds[idx].rbufp;

			if ((fds[idx].wbufp = malloc(MSG_BUF_SIZE)) == NULL) {
				free(fds[idx].rbufp);
				log_warn("msg_init: can't allocate memory");
				master_terminate(ENOMEM);
				return;
			}
			fds[idx].wheadp = fds[idx].wtailp = fds[idx].wbufp;

			fds[idx].fd = fd;
			fds[idx].ev = ev;
			fds[idx].name = name;
			DEBUG(DBG_MSG_FD, "msg_init: name=%s fd:%d idx:%d",
			    name, fd, idx);

			return;
		}
	}
}

static struct msg_buf *
_get_mbp(int fd)
{
	int idx;

	for (idx=0; idx < MSG_BUFS; idx++)
		if (fds[idx].fd == fd) {
			DEBUG(DBG_MSG_FD, "_get_mbp: fd:%d idx:%d", fd, idx);
			return &fds[idx];
		}

	log_warn("buffer not found");
	return NULL;
}

static void
_fill_buf(struct msg_buf *mbp)
{
	int size;

	/* if empty, point to start */
	if (mbp->rheadp == mbp->rtailp)
		mbp->rheadp = mbp->rtailp = mbp->rbufp;

	/* if not at start, move to start */
	if (mbp->rheadp != mbp->rbufp) {
		size = mbp->rheadp - mbp->rbufp;
		bcopy(mbp->rheadp, mbp->rbufp, size);
		mbp->rheadp -= size;
		mbp->rtailp -= size;
	}

	size = read(mbp->fd, mbp->rtailp, MSG_BUF_SIZE);
	if (size < 0) {
		if (errno != EAGAIN)
			log_warn("read error");
		return;
	}
	if (size == 0) {
		log_warn("read error (closed)");
		event_del(mbp->ev);
		event_loopexit(NULL);
		return;
	}
	mbp->rtailp += size;
	DEBUG(DBG_MSG_RECV, "_fill_buf: fd:%d size:%d", mbp->fd, size);
}

static void
_flush_buf(struct msg_buf *mbp)
{
	int size;

	if (mbp->wheadp == mbp->wtailp)
		return;

	size = write(mbp->fd, mbp->wheadp, mbp->wtailp - mbp->wheadp);
	if (size < 0) {
		log_warn("write error");
		return;
	}
	mbp->wheadp += size;
	DEBUG(DBG_MSG_SEND, "_flush_buf: fd:%d size:%d", mbp->fd, size);

	/* if empty, point to start */
	if (mbp->wheadp == mbp->wtailp)
		mbp->wheadp = mbp->wtailp = mbp->wbufp;
}

/*
 * This returns a 'malloc'ed msg on success.
 */
anoubisd_msg_t *
get_msg(int fd)
{
	struct msg_buf *mbp;
	anoubisd_msg_t *msg;
	anoubisd_msg_t *msg_r;


	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warn("msg_buf not initialized");
		return NULL;
	}

	/* we need at least a short (check for an int) in the buffer */
	if (mbp->rtailp - mbp->rheadp < sizeof(int))
		_fill_buf(mbp);
	if (mbp->rtailp - mbp->rheadp < sizeof(int))
		return NULL;

	/* check for a complete message */
	msg = (anoubisd_msg_t *)mbp->rheadp;
	if (mbp->rtailp - mbp->rheadp < msg->size)
		_fill_buf(mbp);
	if (mbp->rtailp - mbp->rheadp < msg->size)
		return NULL;

	if ((msg_r = malloc(msg->size)) == NULL) {
		log_warn("get_msg: can't allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}

	bcopy(msg, msg_r, msg->size);
	mbp->rheadp += msg->size;
	DEBUG(DBG_MSG_RECV, "get_msg: fd:%d size:%d", mbp->fd, msg->size);
	return msg_r;
}

/*
 * This returns a 'malloc'ed msg on success.
 */
anoubisd_msg_t *
get_event(int fd)
{
	struct msg_buf *mbp;
	struct eventdev_hdr *evt;
	anoubisd_msg_t *msg_r;

	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warn("msg_buf not initialized");
		master_terminate(ENOENT);
		return NULL;
	}

	/* we need at least a short (check for an int) in the buffer */
	if (mbp->rtailp - mbp->rheadp < sizeof(int))
		_fill_buf(mbp);
	if (mbp->rtailp - mbp->rheadp < sizeof(int))
		return NULL;

	/* check for a complete message */
	evt = (struct eventdev_hdr *)mbp->rheadp;
	if ((mbp->rtailp - mbp->rheadp) < evt->msg_size)
		_fill_buf(mbp);
	if ((mbp->rtailp - mbp->rheadp) < evt->msg_size)
		return NULL;

	if ((msg_r = malloc(evt->msg_size + sizeof(anoubisd_msg_t))) == NULL) {
		log_warn("get_event: can't allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}

	msg_r->size = evt->msg_size + sizeof(anoubisd_msg_t);
	msg_r->mtype = ANOUBISD_MSG_EVENTDEV;
	bcopy(evt, msg_r->msg, evt->msg_size);
	mbp->rheadp += evt->msg_size;
	DEBUG(DBG_MSG_RECV, "get_event: fd:%d size:%d", mbp->fd, evt->msg_size);
	return msg_r;
}

int
send_msg(int fd, anoubisd_msg_t *msg)
{
	struct msg_buf *mbp;

	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warn("msg_buf not initialized");
		return 0;
	}
	if (mbp->wtailp != mbp->wheadp) {
		_flush_buf(mbp);
/* XXX [RD] event_add()? */
		return 0;
	}

	bcopy(msg, mbp->wtailp, msg->size);
	mbp->wtailp += msg->size;
	DEBUG(DBG_MSG_RECV, "send_msg: fd:%d size:%d", mbp->fd, msg->size);
	_flush_buf(mbp);
	return 1;
}

int
send_reply(int fd, anoubisd_msg_t *msg)
{
	struct msg_buf *mbp;

	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warn("msg_buf not initialized");
		return 0;
	}
	if (mbp->wtailp != mbp->wheadp) {
		_flush_buf(mbp);
/* XXX [RD] event_add()? */
		return 0;
	}

	bcopy(msg->msg, mbp->wtailp, sizeof(struct eventdev_reply));
	mbp->wtailp += sizeof(struct eventdev_reply);
	DEBUG(DBG_MSG_RECV, "send_reply: fd:%d size:%d", mbp->fd, msg->size);
	_flush_buf(mbp);
	return 1;
}

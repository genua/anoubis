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
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "anoubisd.h"
#include "amsg.h"

#define MSG_BUFS 100
struct msg_buf {
	int		 fd;
	size_t		 rmsgoff;
	size_t		 woff;
	char		*name;
	void		*rbufp, *rheadp, *rtailp;
	anoubisd_msg_t	*rmsg;
	anoubisd_msg_t	*wmsg;
};

/*@reldef@*/
static struct msg_buf fds[MSG_BUFS];

/* simple buffered file access, remember the event */
void
msg_init(int fd, char *name)
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

			/*
			 * We use MSG_BUF_SIZE*2, so we can always read
			 * up to MSG_BUF_SIZE bytes!
			 */
			if ((fds[idx].rbufp = malloc(MSG_BUF_SIZE*2)) == NULL) {
				log_warn("msg_init: can't allocate memory");
				master_terminate(ENOMEM);
				return;
			}
			fds[idx].rheadp = fds[idx].rtailp = fds[idx].rbufp;
			fds[idx].rmsg = NULL;
			fds[idx].rmsgoff = 0;

			fds[idx].wmsg = NULL;
			fds[idx].woff = 0;

			fds[idx].fd = fd;
			fds[idx].name = name;
			DEBUG(DBG_MSG_FD, "msg_init: name=%s fd:%d idx:%d",
			    name, fd, idx);

			return;
		}
	}
	log_warn("msg_init: No unused msg_bufs found");
}

/*@exposed@*/ /*@null@*/
static struct msg_buf *
_get_mbp(int fd)
{
	int idx;

	for (idx=0; idx < MSG_BUFS; idx++)
		if (fds[idx].fd == fd) {
			DEBUG(DBG_MSG_FD, "_get_mbp: fd:%d idx:%d", fd, idx);
			return &fds[idx];
		}

	DEBUG(DBG_MSG_FD, "buffer not found: %d", fd);
	return NULL;
}

void
msg_release(int fd)
{
	struct msg_buf	*buf = _get_mbp(fd);
	if (!buf)
		return;
	free(buf->rbufp);
	if (buf->wmsg)
		free(buf->wmsg);
	if (buf->rmsg)
		free(buf->rmsg);
	buf->rmsg = NULL;
	buf->rmsgoff = 0;
	buf->wmsg = NULL;
	buf->woff = 0;
	buf->rbufp = NULL;
	buf->fd = -1;
	buf->name = NULL;
}

/*
 * Return true if more data can be expected from the fd, zero if
 * EOF was detected.
 */
static int
_fill_buf(struct msg_buf *mbp)
{
	int size, space;

	DEBUG(DBG_MSG_RECV, " _fill_buf: rmsg=%p rmsgoff=%d rheadp=%p "
	    "rtailp=%p msgsize=%d", mbp->rmsg, (int)mbp->rmsgoff, mbp->rheadp,
	    mbp->rtailp, (mbp->rmsg)?(int)mbp->rmsg->size:0);
	if (mbp->rmsg) {
		/* Message already complete. */
		if ((int)mbp->rmsgoff == mbp->rmsg->size)
			return 1;
		size = read(mbp->fd, (void*)(mbp->rmsg) + mbp->rmsgoff,
		    mbp->rmsg->size - mbp->rmsgoff);
		if (size <= 0)
			goto err;
		mbp->rmsgoff += size;
		return 1;
	}
	/* if empty, point to start */
	if (mbp->rheadp == mbp->rtailp)
		mbp->rheadp = mbp->rtailp = mbp->rbufp;

	/* if not at start, move to start */
	if (mbp->rheadp != mbp->rbufp) {
		size = mbp->rtailp - mbp->rheadp;
		memmove(mbp->rbufp, mbp->rheadp, size);
		mbp->rheadp = mbp->rbufp;
		mbp->rtailp = mbp->rheadp + size;
	}
	space = 2*MSG_BUF_SIZE - (mbp->rtailp - mbp->rbufp);
	if (space <= 0)
		return 1;
	size = read(mbp->fd, mbp->rtailp, space);
	if (size <= 0)
		goto err;
	mbp->rtailp += size;
	DEBUG(DBG_MSG_RECV, "_fill_buf: fd:%d size:%d", mbp->fd, size);
	return 1;
err:
	DEBUG(DBG_MSG_RECV, "_fill_buf: fd:%d size:%d", mbp->fd, size);
	if (size < 0 && errno == EAGAIN)
		return 1;
	if (size < 0)
		log_warn("read error");
	return 0;
}

int msg_eof(int fd)
{
	struct msg_buf		*mbp;

	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warn("msg_buf not initialized");
		return 1;
	}
	if (mbp->rmsg || (mbp->rheadp != mbp->rtailp))
		return 0;
	if (_fill_buf(mbp))
		return 0;
	return 1;
}

static void
_flush_buf(struct msg_buf *mbp)
{
	int size;

	if (mbp->wmsg == NULL)
		return;
	if ((int)mbp->woff == mbp->wmsg->size) {
		free(mbp->wmsg);
		mbp->wmsg = NULL;
		mbp->woff = 0;
		return;
	}
	size = write(mbp->fd, ((void*)mbp->wmsg)+mbp->woff,
	    mbp->wmsg->size - mbp->woff);
	if (size < 0) {
		log_warn("write error");
		return;
	}
	mbp->woff += size;
	DEBUG(DBG_MSG_SEND, "_flush_buf: fd:%d size:%d", mbp->fd, size);

	/* if empty, point to start */
	if ((int)mbp->woff == mbp->wmsg->size) {
		free(mbp->wmsg);
		mbp->woff = 0;
		mbp->wmsg = NULL;
	}
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
	int		copy;


	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warn("msg_buf not initialized");
		return NULL;
	}

	if (mbp->rmsg) {
		if ((int)mbp->rmsgoff != mbp->rmsg->size)
			if (!_fill_buf(mbp))
				goto eof;
		if ((int)mbp->rmsgoff == mbp->rmsg->size) {
			msg_r = mbp->rmsg;
			mbp->rmsg = NULL;
			mbp->rmsgoff = 0;
			goto message;
		}
		return NULL;
	}
	/* we need at least the anoubisd_msg_t structure. */
	if ((unsigned int)(mbp->rtailp - mbp->rheadp) < sizeof(anoubisd_msg_t))
		if (!_fill_buf(mbp))
			goto eof;
	if ((unsigned int)(mbp->rtailp - mbp->rheadp) < sizeof(anoubisd_msg_t))
		return NULL;

	/* check for a complete message */
	msg = (anoubisd_msg_t *)mbp->rheadp;
	if (msg->size > MSG_SIZE_LIMIT
	    || msg->size < (int)sizeof(anoubisd_msg_t)) {
		log_warnx("get_msg: Bad message size %d", msg->size);
		master_terminate(EINVAL);
		return NULL;
	}
	if ((msg_r = malloc(msg->size)) == NULL) {
		log_warn("get_msg: can't allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}
	copy = mbp->rtailp - mbp->rheadp;
	if (copy > msg->size)
		copy = msg->size;
	bcopy(msg, msg_r, copy);
	mbp->rheadp += copy;
	if (msg_r->size != copy) {
		mbp->rmsg = msg_r;
		mbp->rmsgoff = copy;
		return NULL;
	}
message:
	amsg_verify(msg_r);
	DEBUG(DBG_MSG_RECV, "get_msg: fd:%d size:%d", mbp->fd, msg_r->size);
	return msg_r;
eof:
	return NULL;
}


/*
 * This returns a 'malloc'ed msg on success.
 */
anoubisd_msg_t *
get_event(int fd)
{
	struct msg_buf		*mbp;
	struct eventdev_hdr	*evt;
	anoubisd_msg_t		*msg_r;
	int			 size;

	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warn("msg_buf not initialized");
		master_terminate(ENOENT);
		return NULL;
	}

	/*
	 * If we can call get_event on this fd, it cannot have
	 * a pending message because get_event only returns a single
	 * complete message.
	 */
	if (mbp->rheadp != mbp->rtailp || mbp->rmsg != NULL) {
		log_warnx(" get_event: Invalid buffer state");
		return NULL;
	}
	size = read(fd, mbp->rbufp, 2*MSG_BUF_SIZE);
	if (size <= 0) {
		if (size < 0 && errno != EAGAIN)
			log_warn(" get_event: read error");
		return NULL;
	}
	evt = (struct eventdev_hdr *)mbp->rbufp;
	if (size < (int)sizeof(int) || evt->msg_size != size) {
		log_warnx(" Bad eventdev message length %d", size);
		return NULL;
	}
	msg_r = msg_factory(ANOUBISD_MSG_EVENTDEV, evt->msg_size);
	if (msg_r == NULL) {
		log_warn("get_event: can't allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}
	bcopy(evt, msg_r->msg, evt->msg_size);
	if (version < ANOUBISCORE_VERSION) {
		msg_r = compat_get_event(msg_r, version);
		if (!msg_r) {
			log_warnx("Cannot convert Message from version %ld",
			    version);
			master_terminate(ENOMEM);
		}
		evt = (struct eventdev_hdr *)msg_r->msg;
	}
	if (eventdev_hdr_size(msg_r->msg, evt->msg_size) < 0) {
		log_warnx("Dropping malformed kernel event, src=%d, size=%d",
		    evt->msg_source, evt->msg_size);
		free(msg_r);
		return NULL;
	}
	DEBUG(DBG_MSG_RECV, "get_event: fd:%d size:%d", mbp->fd, evt->msg_size);
	amsg_verify(msg_r);
	return msg_r;
}

/*
 * Returns:
 *  - negative errno number in case of error
 *  - Zero on EOF
 *  - One otherwise. *msgp is NULL if message is incomplete
 */
int
get_client_msg(int fd, struct anoubis_msg **msgp)
{
	struct msg_buf		*mbp;
	u_int32_t		 len;
	struct anoubis_msg	*m;
	*msgp = NULL;

	if ((mbp = _get_mbp(fd)) == NULL)
		return 0;
	if (mbp->rmsg) {
		log_warnx("get_client_msg: Bad buffer state");
		return 0;
	}
	if ((unsigned int)(mbp->rtailp - mbp->rheadp) < sizeof(len))
		if (!_fill_buf(mbp))
			return 0;
	if ((unsigned int)(mbp->rtailp - mbp->rheadp) < sizeof(len))
		return 1;
	len = ntohl(*(u_int32_t*)mbp->rheadp);
	if ((unsigned int)(mbp->rtailp - mbp->rheadp) < len)
		if (!_fill_buf(mbp))
			return 0;
	if ((unsigned int)(mbp->rtailp - mbp->rheadp) < len)
		return 1;
	m = anoubis_msg_new(len - sizeof(len) - CSUM_LEN);
	if (!m)
		return -ENOMEM;
	bcopy(mbp->rheadp+sizeof(len), m->u.buf, m->length);
	mbp->rheadp += len;
	*msgp = m;
	return 1;
}

/* Just flush the buffer if msg == NULL. */
int
send_msg(int fd, anoubisd_msg_t *msg)
{
	struct msg_buf *mbp;

	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warnx("msg_buf not initialized");
		return -1;
	}
	if (mbp->wmsg) {
		_flush_buf(mbp);
		if (mbp->wmsg)
			return 0;
	}
	if (!msg)
		return 1;
	if (msg->size > MSG_SIZE_LIMIT) {
		log_warnx("send_msg: message %p to large %d", msg, msg->size);
		return -1;
	}
	amsg_verify(msg);
	mbp->wmsg = msg;
	mbp->woff = 0;
	DEBUG(DBG_MSG_RECV, "send_msg: fd:%d size:%d", mbp->fd, msg->size);
	_flush_buf(mbp);
	return 1;
}

int
msg_pending(int fd)
{
	struct msg_buf * mbp;

	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warn("msg_buf not initialized");
		return 0;
	}
	return mbp->wmsg != NULL;
}

struct anoubisd_msg *
msg_factory(int mtype, int size)
{
	struct anoubisd_msg *msg;

	size += sizeof(anoubisd_msg_t);
	if (size < (int)sizeof(anoubisd_msg_t) || size > MSG_SIZE_LIMIT) {
		log_warnx("msg_factory: Bad message size %d", size);
		return NULL;
	}
	if ((msg = malloc(size)) == NULL) {
		log_warn("msg_factory: cannot allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}
	bzero(msg, size);
	msg->mtype = mtype;
	msg->size = size;
	return msg;
}

void
msg_shrink(struct anoubisd_msg *msg, int nsize)
{
	nsize += sizeof(anoubisd_msg_t);
	if (nsize < (int)sizeof(anoubisd_msg_t) || nsize > MSG_SIZE_LIMIT) {
		log_warnx("msg_shrink: Bad message size %d", nsize);
		return;
	}
	if (nsize < msg->size)
		msg->size = nsize;
}

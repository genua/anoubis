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

/**
 * The maximum number of message buffers. These buffers are used for
 * client connections in session.c, too. Thus we use a rather large value.
 */
#define MSG_BUFS 1000

/**
 * This structure handles buffered reading of variable lenght messages
 * from pipes that connect anoubis daemon process with each other and
 * the kernel.
 *
 */
struct msg_buf {
	/**
	 * The file descriptor of this buffer.
	 */
	int			 fd;

	/**
	 * The number of bytes that are already stored in the buffer
	 * pointed to by rmsg. Should be zero if the rmsg field is NULL.
	 */
	size_t			 rmsgoff;

	/**
	 * An offset into the message that is currently written to the pipe.
	 * The first woff bytes of the message have been written the rest must
	 * still be written.
	 */
	size_t			 woff;

	/**
	 * Points to a buffer used for reading data. This is where input
	 * data is first stored. It is only transfered to rmsg if the current
	 * message turns out to be incomplete. This pointer always points to
	 * the start of the buffer which need not be the start of valid data.
	 */
	void			*rbufp;

	/**
	 * A pointer into the read buffer. This pointer points to the start
	 * of the unprocessed data in the buffer.
	 */
	void			*rheadp;

	/**
	 * A pointer into the read bufffer. This pointer points one byte
	 * after the end of the unprocessed data in the buffer.
	 */
	void			*rtailp;

	/**
	 * If an incomplete message is found at the end of the read buffer,
	 * the new message is allocated and stored here. The partial data is
	 * copied into the message and removed from the read buffer. Subsequent
	 * data is read directly into this message until the message is
	 * complete. Once complete the message is returned to the caller.
	 */
	struct anoubisd_msg	*rmsg;

	/**
	 * This is the message that is currently being written to the pipe.
	 * The message will be freed after it is written, i.e. the caller
	 * can forget about it.
	 */
	struct anoubisd_msg	*wmsg;

	/**
	 * True if a previous call to read returned zero. This is used to
	 *     implement a side effect free end of file test.
	 */
	int			 iseof;
};

/**
 * Message buffers for active file descriptors are stored here. This
 * storage is local to this file. The callers only deal with the file
 * descriptors themselves. A message buffer is in use iff its rbufp
 * pointer is not zero.
 */
static struct msg_buf fds[MSG_BUFS];

/**
 * Associate a message buffer with a file descriptor and initialize it.
 * The file descriptor is set no non-blocking.
 *
 * @param fd The file descriptor.
 * @return None.
 */
void
msg_init(int fd)
{
	int idx;

	if (fd < 0) {
		log_warnx("msg_init with negative fd");
		master_terminate();
	}
	/* already initialized? */
	for (idx=0; idx < MSG_BUFS; idx++)
		if (fds[idx].fd == fd)
			return;

	if (fcntl(fd, F_SETFL, O_NONBLOCK))
		log_warn("O_NONBLOCK not set");

	/*
	 * Try to use idx=fd if possible. This is a heuristic that
	 * will speed up the search for a message buffer.
	 */
	idx = fd;
	if (idx >= MSG_BUFS || fds[idx].rbufp != NULL) {
		for (idx=MSG_BUFS-1; idx >= 0; idx--) {
			if (fds[idx].rbufp == NULL)
				break;
		}
	}
	if (idx < 0) {
		log_warnx("msg_init: No unused msg_bufs found");
		return;
	}


	/*
	 * We use MSG_BUF_SIZE*2, so we can always read
	 * up to MSG_BUF_SIZE bytes!
	 */
	if ((fds[idx].rbufp = malloc(MSG_BUF_SIZE*2)) == NULL) {
		log_warn("msg_init: can't allocate memory");
		master_terminate();
	}
	fds[idx].rheadp = fds[idx].rtailp = fds[idx].rbufp;
	fds[idx].rmsg = NULL;
	fds[idx].rmsgoff = 0;

	fds[idx].wmsg = NULL;
	fds[idx].woff = 0;

	fds[idx].fd = fd;
	fds[idx].iseof = 0;
	DEBUG(DBG_MSG_FD, "msg_init: fd:%d idx:%d", fd, idx);
}

/**
 * Return a pointer to the message buffer for a given file descriptor.
 * We check the index that is equal to the file descriptor first. msg_init
 * perferrably stores the buffer at this index.
 *
 * @param fd The file descriptor.
 * @return The message buffer associated with the file descriptor.
 */
static struct msg_buf *
_get_mbp(int fd)
{
	int idx;

	if (fd < 0)
		return NULL;
	idx = fd;
	if (idx >= MSG_BUFS || fds[idx].rbufp == NULL || fds[idx].fd != fd) {
		for (idx=MSG_BUFS-1; idx >= 0; idx--) {
			if (fds[idx].rbufp && fds[idx].fd == fd)
				break;
		}
	}
	if (idx < 0) {
		DEBUG(DBG_MSG_FD, "buffer not found: %d", fd);
		return NULL;
	}
	DEBUG(DBG_MSG_FD, "_get_mbp: fd:%d idx:%d", fd, idx);
	return &fds[idx];
}

/**
 * Release a message buffer before closing a file descriptor. This is
 * mainly useful in child processes of the master that want to get rid
 * of message buffers for file desciptors that must be closed.
 *
 * @param fd The file descriptor.
 * @return None.
 */
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
	buf->iseof = 0;
}

/**
 * This function reads data and stores it in the appropriate buffer.
 * If there is an incomplete message, read data is stored in this
 * message. Otherwise data is stored in the read buffer.
 *
 * @param mbp The message buffer.
 * @return True if more data can be expected from the file descriptor.
 *     Zero if end of file is returned.
 * NOTE: Zero is only returned if the file descriptor will never have
 * additional data ready. A temorary shortage of data (i.e. an empty pipe
 * buffer) does not cause this function to return zero.
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
		if (size == 0)
			mbp->iseof = 1;
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
	if (size == 0)
		mbp->iseof = 1;
	if (size <= 0)
		goto err;
	mbp->rtailp += size;
	DEBUG(DBG_MSG_RECV, "_fill_buf: fd:%d size:%d", mbp->fd, size);
	return 1;
err:
	DEBUG(DBG_MSG_RECV, "_fill_buf: fd:%d size:%d", mbp->fd, size);
	if (size < 0 && (errno == EAGAIN || errno == EINTR))
		return 1;
	if (size < 0)
		log_warn("read error");
	return 0;
}

/**
 * Return true if end of file was detected on the file descriptor in
 * a previous read and no more (complete) messages can be read from
 * the file descriptor. This function is side effect free wrt. the
 * file descriptor, i.e. it does not (try to) read from it. If end of
 * file is reached on the file descriptor but there is still some data
 * in the buffer, it is expecgted that the caller uses something like
 * get_msg to read the data.
 *
 * @param fd The file descriptor.
 * @return True if end of file is reached.
 *
 * NOTE: This function used to call _fill_buf which is bogus because
 *     it can cause the file descriptor to become blocking with a
 *     complete message in the receive buffer. Depending on the time
 *     that msg_eof is called in the receive handler this might cause
 *     the message to get stuck in the receive buffer until more data
 *     is received from the other end.
 * NOTE2: The final incomplete message the will never be completed
 *     will not prevent this function from returning true.
 */
int msg_eof(int fd)
{
	struct msg_buf		*mbp;

	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warnx("msg_buf not initialized");
		return 1;
	}
	if (!mbp->iseof)
		return 0;
	/*
	 * If mbp->rmsg is not NULL the read buffer must be empty.
	 * As no more data can be read from the fd, return true (EOF)
	 * if the message is incomplete, false if it is complete.
	 */
	if (mbp->rmsg) {
		if (mbp->rmsg->size == (int)mbp->rmsgoff)
			return 0;
		return 1;
	}
	/*
	 * If the message buffer does not contain enough data for at least
	 * a message header, this is certainly not a complete message, i.e.
	 * we return EOF. Otherwise the next call to get_msg will either
	 * empty the buffer or move the (rest of the) buffer contents to
	 * mbp->rmsg.
	 */
	if (mbp->rtailp - mbp->rheadp < (int)sizeof(struct anoubisd_msg))
		return 1;
	return 0;
}

/**
 * Write pending data from current outgoing message to the file
 * descriptor. This function tries to write data at most once.
 *
 * @param mbp The message buffer.
 */
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
		switch(errno) {
		case EAGAIN: case EINTR:
			return;
		case EPIPE: case EINVAL: case EBADF: case EIO: case EFAULT:
			/*
			 * Non recoverable errors. Drop data to prevent an
			 * endless loop during shutdown. Note that we cannot
			 * just terminate because we might want to use this
			 * for user connections, too.
			 */
			log_warn("write error dropping %sdata",
			    (mbp->woff ? "incomplete " : ""));
			free(mbp->wmsg);
			mbp->wmsg = NULL;
			mbp->woff = 0;
			return;
		}
		log_warn("write error");
		return;
	}
	mbp->woff += size;
	DEBUG(DBG_MSG_SEND, "_flush_buf: fd:%d size:%d", mbp->fd, size);

	if ((int)mbp->woff == mbp->wmsg->size) {
		free(mbp->wmsg);
		mbp->woff = 0;
		mbp->wmsg = NULL;
	}
}

/**
 * Try to retrieve data from the buffer associate with the file descriptor.
 * The read buffer is filled if required. If a complete message is available
 * it is returned. The message is passed through amsg_verify before it is
 * returned.
 *
 * @param fd The file descriptor to read a message from.
 * @return A complete message in malloced memory or NULL if there is no
 *     data or only an incomplete message. If this function returns NULL,
 *     it is guaranteed that data must be read from the file descriptor
 *     before a complete message can be returned. It is not guaranteed
 *     that all available data from the file descriptor was actuall read!
 */
struct anoubisd_msg *
get_msg(int fd)
{
	struct msg_buf		*mbp;
	struct anoubisd_msg	*msg;
	struct anoubisd_msg	*msg_r;
	int			 copy;

	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warnx("msg_buf not initialized");
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
	/* we need at least the struct anoubisd_msg structure. */
	if (mbp->rtailp - mbp->rheadp < (int)sizeof(struct anoubisd_msg))
		if (!_fill_buf(mbp))
			goto eof;
	if (mbp->rtailp - mbp->rheadp < (int)sizeof(struct anoubisd_msg))
		return NULL;

	/* check for a complete message */
	msg = (struct anoubisd_msg *)mbp->rheadp;
	if (msg->size > MSG_SIZE_LIMIT
	    || msg->size < (int)sizeof(struct anoubisd_msg)) {
		log_warnx("get_msg: Bad message size %d", msg->size);
		master_terminate();
	}
	if ((msg_r = malloc(msg->size)) == NULL) {
		log_warn("get_msg: can't allocate memory");
		master_terminate();
	}
	copy = mbp->rtailp - mbp->rheadp;
	if (copy > msg->size)
		copy = msg->size;
	memcpy(msg_r, msg, copy);
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


/**
 * Try to read a kernel message (i.e. a struct eventdev_hdr followed by
 * payload data) from the file descriptor (or from data in its associated
 * buffer). This is similar to get_msg but takes into account that
 * the kernel will not let us read partial messages from the eventdev
 * device.
 *
 * @param fd The file descriptor to read from.
 * @return A kernel event encapsulated in a malloced struct anoubis_msg
 *     of type ANOUBISD_MSG_EVENTDEV or NULL if no message is available.
 *     The message format of the mesage is converted if we are running
 *     on an older kernel. The resulting message is passed through
 *     amsg_verify before returning it.
 *
 * NOTE: The converted kernel event is checked for correctness and the
 * whole message is dropped if this check fails. This happens before
 * amsg_verify to prevent the daemon from crashing on bogus kernel messages.
 */
struct anoubisd_msg *
get_event(int fd)
{
	struct msg_buf			*mbp;
	struct eventdev_hdr		*evt;
	struct anoubisd_msg		*msg_r;
	int				 size;

	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warnx("msg_buf not initialized");
		master_terminate();
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
		if (size == 0)
			mbp->iseof = 1;
		if (size < 0 && errno != EAGAIN && errno != EINTR)
			log_warn(" get_event: read error");
		return NULL;
	}
	evt = (struct eventdev_hdr *)mbp->rbufp;
	if (size < (int)sizeof(struct eventdev_hdr) || evt->msg_size != size) {
		log_warnx(" Bad eventdev message length %d", size);
		return NULL;
	}
	msg_r = msg_factory(ANOUBISD_MSG_EVENTDEV, evt->msg_size);
	if (msg_r == NULL) {
		log_warn("get_event: can't allocate memory");
		master_terminate();
	}
	memcpy(msg_r->msg, evt, evt->msg_size);
	if (version < ANOUBISCORE_VERSION) {
		msg_r = compat_get_event(msg_r, version);
		if (!msg_r) {
			log_warnx("Cannot convert Message from version %ld",
			    version);
			master_terminate();
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

/**
 * Read an anoubis protocol message from a client connected to the session
 * engine. A client message on the wire consists of a 32 bit length in
 * network byte order followed by the actual message data. The length
 * given includes the 32 length itself and the 32 bit checksum at the
 * end of the message.
 *
 * @param fd The file descriptor to read from.
 * @param msgp A complete message (if any) is stored here. This value
 *     must never be NULL. The pointer pointed to by msgp is either set
 *     to NULL or is set to point to the result message. The result message
 *     is allocated using anoubis_msg_new and must be freed by the caller
 *     with anoubis_msg_free.
 * @return Zero if EOF is encountered. A negative error code if an error
 *     occured or a positive value in case of success.
 *     NOTE: Success does not mean that there is a complete message
 *     available. The message may still be incomplete. In this case
 *     NULL is stored in *msgp.
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
	/*
	 * anoubis_msg_new expects the size of the message without
	 * the message length and without the trainling checksum (i.e.
	 * payload data only). However, m->length of the resulting
	 * message includes the checksum but not the message length.
	 * This is the reason for the somewhat surprising length calculations.
	 */
	m = anoubis_msg_new(len - sizeof(len) - CSUM_LEN);
	if (!m)
		return -ENOMEM;
	memcpy(m->u.buf, mbp->rheadp+sizeof(len), m->length);
	mbp->rheadp += len;
	*msgp = m;
	return 1;
}

/**
 * Write a message to the file descriptor. The message is buffered
 * if it cannot be written immediately. At most one message can be
 * buffered at a time. If the function accepts the message it makes
 * sure that the message is freed. Otherwise the caller is responsible
 * for the message. Note that is quite possible that the function rejects
 * a message. This happens in particular, if another message is still in
 * the output buffer. The message is checked using amsg_verify before it
 * is sent.
 *
 * @param fd The file descriptor.
 * @param msg The message to send. Use NULL to just flush the message
 *     buffer.
 * @return Positive if the message was accepted, zero if the message
 *     was not accepted due to a write buffer and negative if the
 *     message cannot be sent permanently (e.g. because it exceeds the
 *     message size limit or the buffer is not initialized). If the
 *     message to send is NULL a positive return value implies that the
 *     buffer was flushed completely.
 *
 * @note This function adds the message to the output buffer and tries
 *     to flush the output buffer. However, if the output buffer is
 *     not empty after the function returns, it is the responsibility of
 *     the caller to flush the output buffer at a later point in time.
 */
int
send_msg(int fd, struct anoubisd_msg *msg)
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

/**
 * Return true if there is data sitting in the output buffer for this
 * file descriptor.
 *
 * @param fd The file descriptor.
 * @return True if there is data in the output buffer.
 */
int
msg_pending(int fd)
{
	struct msg_buf * mbp;

	if ((mbp = _get_mbp(fd)) == NULL) {
		log_warnx("msg_buf not initialized");
		return 0;
	}
	return mbp->wmsg != NULL;
}

/**
 * Allocate a new struct anoubis_msg with the given type and size.
 * The message size does not include the space required for the
 * struct anoubisd_msg. The total message size must not exceed MSG_SIZE_LIMIT.
 *
 * @param mtype The message type.
 * @param size The size of the message payload, i.e. the message data
 *     without the struct anoubisd_msg.
 * @return A message allocated via malloc. The caller is responsible for
 *     freeing it. NULL if we are out of memory.
 */
struct anoubisd_msg *
msg_factory(int mtype, int size)
{
	struct anoubisd_msg *msg;

	size += sizeof(struct anoubisd_msg);
	if (size < (int)sizeof(struct anoubisd_msg) || size > MSG_SIZE_LIMIT) {
		log_warnx("msg_factory: Bad message size %d", size);
		return NULL;
	}
	if ((msg = malloc(size)) == NULL) {
		log_warn("msg_factory: cannot allocate memory");
		master_terminate();
	}
	bzero(msg, size);
	msg->mtype = mtype;
	msg->size = size;
	return msg;
}

/**
 * Reduce the message payload size of a message. This function does not
 * actually free memory and it never increases the message size. However,
 * calling this function can be used to control which part of message is
 * sent to other anoubis daemon processes.
 *
 * @param msg The message to shrink.
 * @param nsize The new size of the message payload. The size of a
 *     struct anoubisd_msg is automatically added to this value and the
 *     message size only changes if the new size is smaller than the old
 *     size.
 */
void
msg_shrink(struct anoubisd_msg *msg, int nsize)
{
	nsize += sizeof(struct anoubisd_msg);
	if (nsize < (int)sizeof(struct anoubisd_msg)
	    || nsize > MSG_SIZE_LIMIT) {
		log_warnx("msg_shrink: Bad message size %d", nsize);
		return;
	}
	if (nsize < msg->size)
		msg->size = nsize;
}

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

#include <sys/types.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <event.h>

#include "anoubis_chat.h"
#include "accbuffer.h"
#include "accutils.h"

/**
 * Reads data from a filedescriptor.
 *
 * Reads up to <code>nbyte</code> bytes from the filedescriptor <code>fd</code>
 * and stores the data in <code>buf</code>. The function stops reading, if no
 * more data are available in <code>fd</code>.
 *
 * @param fd The filedescriptor to read from
 * @param buf The destination-buffer where the data are stored
 * @param nbyte Maximum number of bytes to read from the filedescriptor
 * @return Number of bytes received from the filedescriptor. On error -1 is
 *         returned.
 */
static ssize_t
acc_read(int fd, void *buf, ssize_t nbyte)
{
	ssize_t num = 0;

	if (nbyte < 0)
		return -1;
	while (num < nbyte) {
		ssize_t		result;

		result = read(fd, buf + num, nbyte - num);

		if (result < 0) {
			if (errno == EINTR)
				continue;
			else
				return (num == 0) ? -1 : num;
		}

		num += result;

		if (result == 0) /* No more data available for reading */
			return num;
	}

	return num;
}

/**
 * Writes data into a filedescriptor.
 *
 * @param fd The destination filedescriptor
 * @param buf The source-buffer
 * @param nbyte Number of bytes to write
 * @return Number of bytes written into the filedescriptor. On error -1 is
 *         returned.
 */
static ssize_t
acc_write(int fd, void *buf, ssize_t nbyte)
{
	ssize_t num = 0;

	if (nbyte < 0)
		return -1;
	while (num < nbyte) {
		int result;

		result = write(fd, buf + num, nbyte - num);

		if (result < 0) {
			if (errno == EINTR)
				continue;
			else
				return (num == 0) ? -1 : num;
		}

		num += result;
	}

	return num;
}

/**
 * Flushes the content of the output-buffer.
 * The content of achat_channel::sendbuffer is written into achat_channel::fd.
 *
 * @param chan The channel
 * @return If the complete content of the output-buffer was written into the
 *         filedescriptor achat_rc::ACHAT_RC_OK is returned. When there are
 *         still some pending data left, achat_rc::ACHAT_RC_PENDING is returned.
 */
achat_rc
acc_flush(struct achat_channel *chan)
{
	void		*buf;
	ssize_t		 bwritten, bsize;
	achat_rc	 ret;
	int		 error;

	ACC_CHKPARAM(chan != NULL);

	buf = acc_bufferptr(chan->sendbuffer);
	bsize = acc_bufferlen(chan->sendbuffer);

	if (bsize <= 0) {
		/* Nothing to do */
		return ACHAT_RC_OK;
	}

	bwritten = acc_write(chan->fd, buf, bsize);
	error = errno;

	if (bwritten > 0) {
		achat_rc rc;

		/* Remove chunk of data from buffer */
		rc = acc_bufferconsume(chan->sendbuffer, bwritten);

		if (rc != ACHAT_RC_OK)
			return ACHAT_RC_ERROR;
	}

	if (bwritten < 0)
		ret = (error == EAGAIN) ? ACHAT_RC_PENDING : ACHAT_RC_ERROR;
	else /* bwritten > 0 */
		ret = (bwritten == bsize) ? ACHAT_RC_OK : ACHAT_RC_PENDING;
	if (ret == ACHAT_RC_PENDING && chan->event) {
		event_add(chan->event, NULL);
		ret = ACHAT_RC_OK;
	}
	return ret;
}

/**
 * Appends data to the output-buffer and flushes (at least a part of) it.
 * Data are appended at achat_channel::sendbuffer and acc_flush() is called.
 *
 * @param acc The channel
 * @param msg Data to be appended
 * @param size Number of bytes to be appended
 * @return If all data are written by acc_flush() achat_rc::ACHAT_RC_OK is
 *         returned. When there are still some pending data left,
 *         achat_rc::ACHAT_RC_PENDING is returned.
 */
achat_rc
acc_sendmsg(struct achat_channel *acc, const char *msg, size_t size)
{
	uint32_t	pkgsize;
	achat_rc	rc;

	ACC_CHKPARAM(acc  != NULL);
	ACC_CHKPARAM(msg  != NULL);
	ACC_CHKPARAM(0 < size && size <= ACHAT_MAX_MSGSIZE);

	/* Can only send a message, if you have an open socket */
	if (acc->fd < 0)
		return (ACHAT_RC_ERROR);

	/* (1) Size of following message (in network byte order!) */
	pkgsize = htonl(sizeof(pkgsize) + size);
	rc = acc_bufferappend(acc->sendbuffer, &pkgsize, sizeof(pkgsize));
	if (rc != ACHAT_RC_OK)
		return rc;

	/* (2) Append the message */
	rc = acc_bufferappend(acc->sendbuffer, msg, size);
	if (rc != ACHAT_RC_OK) {
		/* Remove the size from the buffer to keep it consistent */
		acc_buffertrunc(acc->sendbuffer, sizeof(pkgsize));
		return rc;
	}

	/* Flush (at least a part of) the message */
	return acc_flush(acc);
}

/**
 * Appends data at the input-buffer of the channel.
 * The expected number of bytes is passed to the function. Data are read
 * from chat_channel::fd and inserted into achat_channel::recvbuffer.
 *
 * @param acc The channel
 * @param size Number of bytes top read from the filedescriptor
 * @return If the number of expected bytes were read from the filedescriptor
 *         achat_rc::ACHAT_RC_OK is returned. If the number of bytes read from
 *         the filedescriptor is less than <code>size</code>
 *         achat_rc::ACHAT_RC_PENDING is returned.
 */
static achat_rc
acc_fillrecvbuffer(struct achat_channel *acc, size_t size)
{
	const size_t	bsize = acc_bufferlen(acc->recvbuffer);
	ssize_t		nread;
	char		*readbuf;
	int		mincnt = size;
	int		needcnt;
	int		error;

	if (acc->blocking == ACC_NON_BLOCKING) {
		/* Read at least 4k */
		if (mincnt < 4096)
			mincnt = 4096;
	}

	ACC_CHKPARAM(mincnt <= ACHAT_MAX_MSGSIZE);
	ACC_CHKPARAM(mincnt >= 0);

	needcnt = mincnt - bsize;
	if (needcnt <= 0) {
		/* You have enough data in your buffer */
		return (ACHAT_RC_OK);
	}

	/* Make space in receive-buffer */
	readbuf = acc_bufferappend_space(acc->recvbuffer, needcnt);
	if (readbuf == NULL)
		return (ACHAT_RC_ERROR);

	nread = acc_read(acc->fd, readbuf, needcnt);
	error = errno;

	if (nread < 0) {
		/* Buffer not filled, remove allocated space again */
		achat_rc rc = acc_buffertrunc(acc->recvbuffer, needcnt);
		if (rc != ACHAT_RC_OK)
			return (ACHAT_RC_ERROR);

		return (error == EAGAIN) ? ACHAT_RC_PENDING : ACHAT_RC_ERROR;
	}

	if (nread < needcnt) {
		/* nread might be < needcnt, truncate the buffer to have the */
		/* correct buffer size */
		achat_rc rc = acc_buffertrunc(acc->recvbuffer, needcnt - nread);
		if (rc != ACHAT_RC_OK)
			return (ACHAT_RC_ERROR);
	}

	return (nread > 0) ? ACHAT_RC_OK : ACHAT_RC_EOF;
}

/**
 * Reads a complete message from the channel.
 *
 * The message starts with a two-byte-sequence, which contains the size of
 * the following datagram. Next, the message is read from the channel.
 *
 * @param acc The channel
 * @param msg Additionally, the functions copies the message into this buffer.
 * @param size First, points to an integer, which contains the size of
 *        <code>msg</code>. Then, the function writes the number of bytes read
 *        into the integer.
 * @return If the complete message was read, achat_rc::ACHAT_RC_OK is returned.
 *        If you still read some more data to complete the message
 *        achat_rc::ACHAT_RC_PENDING is returned.
 */
achat_rc
acc_receivemsg(struct achat_channel *acc, char *msg, size_t *size)
{
	size_t		bsize;
	uint32_t	pkgsize = 0;

	ACC_CHKPARAM(acc  != NULL);
	ACC_CHKPARAM(msg  != NULL);
	ACC_CHKPARAM(0 < *size && *size <=  ACHAT_MAX_MSGSIZE);

	/* Can only receive a message, if you have an open socket */
	if (acc->fd < 0)
		return (ACHAT_RC_ERROR);

	bsize =  acc_bufferlen(acc->recvbuffer);

	if (bsize < sizeof(pkgsize)) {
		/* Don't have enough data to receive size of message */
		achat_rc rc = acc_fillrecvbuffer(acc, sizeof(pkgsize));

		if (rc != ACHAT_RC_OK) /* error, eof, pending */
			return (rc);
	}

	bsize = acc_bufferlen(acc->recvbuffer);

	if (bsize >= sizeof(pkgsize)) {
		/* Enough data received to build up package size */
		memcpy(&pkgsize, acc_bufferptr(acc->recvbuffer),
			sizeof(pkgsize));
		pkgsize = ntohl(pkgsize); /* Convert to host byte order */

		if (pkgsize == sizeof(pkgsize)) { /* Empty body */
			*size = 0;
			return (ACHAT_RC_OK);
		}
		if (pkgsize < sizeof(pkgsize)) /* Corrupted buffer */
			return (ACHAT_RC_ERROR);
	}
	else {
		/* Not enough data received */
		return ACHAT_RC_PENDING;
	}

	if (bsize < pkgsize) {
		/* Complete message not buffered, re-read */
		achat_rc rc = acc_fillrecvbuffer(acc, pkgsize);

		if (rc != ACHAT_RC_OK) /* error, eof, pending */
			return (rc);
	}

	bsize = acc_bufferlen(acc->recvbuffer);
	if (bsize < pkgsize) {
		/* Don't have enough data to return complete message */
		return ACHAT_RC_PENDING;
	}

	/* Complete message available */
	if (*size >= pkgsize - sizeof(pkgsize)) {
		*size = pkgsize - sizeof(pkgsize);
		memcpy(msg, acc_bufferptr(acc->recvbuffer) + sizeof(pkgsize),
			*size);

		/* Remove message from buffer */
		return acc_bufferconsume(acc->recvbuffer, pkgsize);
	}
	else /* msg is not big enough to hold the complete message */
		return ACHAT_RC_NOSPACE;
}

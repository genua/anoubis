/* $OpenBSD: buffer.c,v 1.31 2006/08/03 03:34:41 deraadt Exp $ */
/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Functions for manipulating fifo buffers (that can grow if needed).
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#include <sys/param.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "accbuffer.h"
#include "accutils.h"

/* Initializes the buffer structure. */
achat_rc
acc_bufferinit(achat_buffer *buffer)
{
	const u_int len = ACHAT_BUFFER_DEFAULTSZ;

	ACC_CHKPARAM(buffer != NULL);
	ACC_CHKPARAM(len <= UINT_MAX);

	buffer->alloc = 0;
	buffer->offset = 0;
	buffer->end = 0;
	buffer->buf = malloc(len);

	if (buffer->buf == NULL)
		return (ACHAT_RC_OOMEM);

	buffer->alloc = len;

	return (ACHAT_RC_OK);
}

/* Frees any memory used for the buffer. */
achat_rc
acc_bufferfree(achat_buffer *buffer)
{
	ACC_CHKPARAM(buffer != NULL);

	if (buffer->alloc > 0) {
		if (buffer->buf == NULL)
			return (ACHAT_RC_ERROR);
		bzero(buffer->buf, buffer->alloc);
		free(buffer->buf);
		bzero(buffer, sizeof(struct achat_buffer));
	}

	return (ACHAT_RC_OK);
}

/*
 * Clears any data from the buffer, making it empty.  This does not actually
 * zero the memory.
 */
achat_rc
acc_bufferclear(achat_buffer *buffer)
{
	ACC_CHKPARAM(buffer != NULL);

	buffer->offset = 0;
	buffer->end = 0;

	return (ACHAT_RC_OK);
}

/* Appends data to the buffer, expanding it if necessary. */
achat_rc
acc_bufferappend(achat_buffer *buffer, const void *data, u_int len)
{
	void *p;

	ACC_CHKPARAM(buffer != NULL);
	ACC_CHKPARAM(data   != NULL);
	ACC_CHKPARAM(len    >  0);

	p = acc_bufferappend_space(buffer, len);
	if (p == NULL)
		return (ACHAT_RC_ERROR);

	memcpy(p, data, len);

	return (ACHAT_RC_OK);
}

static int
accbuffer_compact(achat_buffer *buffer)
{
	/*
	 * If the buffer is quite empty, but all data is at the end, move the
	 * data to the beginning.
	 */
	if (buffer->offset > MIN(buffer->alloc, ACHAT_BUFFER_MAX_CHUNK)) {
		memmove(buffer->buf, buffer->buf + buffer->offset,
			buffer->end - buffer->offset);
		buffer->end -= buffer->offset;
		buffer->offset = 0;
		return (1);
	}
	return (0);
}

/*
 * Appends space to the buffer, expanding the buffer if necessary. This does
 * not actually copy the data into the buffer, but instead returns a pointer
 * to the allocated region.
 */
void *
acc_bufferappend_space(achat_buffer *buffer, u_int len)
{
	u_int newlen;
	void *p;

	if ((buffer == NULL) || (len <= 0) || (len > ACHAT_BUFFER_MAX_CHUNK))
		return (NULL);

	/* If the buffer is empty, start using it from the beginning. */
	if (buffer->offset == buffer->end) {
		buffer->offset = 0;
		buffer->end = 0;
	}

restart:
	/* If there is enough space to store all data, store it now. */
	if (buffer->end + len < buffer->alloc) {
		p = buffer->buf + buffer->end;
		buffer->end += len;
		return (p);
	}

	/* Compact data back to the start of the buffer if necessary */
	if (accbuffer_compact(buffer))
		goto restart;

	/* Increase the size of the buffer and retry. */
	newlen = roundup(buffer->alloc + len, ACHAT_BUFFER_ALLOCSZ);
	if ((newlen == 0) || (newlen > UINT_MAX) ||
	    (newlen > ACHAT_BUFFER_MAX_LEN))
		return (NULL);

	buffer->buf = realloc(buffer->buf, newlen);
	if (buffer->buf == NULL)
		return (NULL);

	buffer->alloc = newlen;
	goto restart;
	/* NOTREACHED */
}

/* Returns the number of bytes of data in the buffer. */
size_t
acc_bufferlen(achat_buffer *buffer)
{
	return (buffer->end - buffer->offset);
}

/* Returns a pointer to the first used byte in the buffer. */
void *
acc_bufferptr(achat_buffer *buffer)
{
	return (buffer->buf + buffer->offset);
}

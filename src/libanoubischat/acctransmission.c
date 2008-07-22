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

#include "anoubischat.h"
#include "accbuffer.h"
#include "accutils.h"

achat_rc
acc_flush(struct achat_channel *chan)
{
	struct achat_buffer	*buf = chan->sendbuffer;
	unsigned int		 len;

	if (!chan->sendbuffer)
		return ACHAT_RC_OK;
	while((len = acc_bufferlen(buf))) {
		int ret = write(chan->fd, acc_bufferptr(buf), len);
		if (ret < 0) {
			if (errno == EAGAIN) {
				if (chan->event)
					event_add(chan->event, NULL);
				return ACHAT_RC_OK;
			}
			return  ACHAT_RC_ERROR;
		}
		buf->offset += ret;
	}
	acc_bufferfree(buf);
	free(chan->sendbuffer);
	chan->sendbuffer = NULL;
	return ACHAT_RC_OK;
}

achat_rc
acc_sendmsg(struct achat_channel *acc, const char *msg, size_t size)
{
	u_int32_t	 pkgsizenet = 0;
	achat_rc	 rc = ACHAT_RC_ERROR;
	char		*sendptr;

	ACC_CHKPARAM(acc  != NULL);
	ACC_CHKPARAM(msg  != NULL);
	ACC_CHKPARAM(0 < size && size <= ACHAT_MAX_MSGSIZE);

	/* Can only send a message, if you have an open socket */
	if (acc->fd == -1)
		return (ACHAT_RC_ERROR);

	if (acc->sendbuffer) {
		rc = acc_flush(acc);
		if (rc != ACHAT_RC_OK)
			return rc;
	}
	/* We already have a buffer and were unable to flush it. */
	if (acc->sendbuffer)
		return ACHAT_RC_ERROR;
	acc->sendbuffer = malloc(sizeof(struct achat_buffer));
	if (!acc->sendbuffer)
		return ACHAT_RC_OOMEM;
	rc = acc_bufferinit(acc->sendbuffer);
	if (rc != ACHAT_RC_OK)
		/* no memory leak in OOM situations */
		/*@i1*/return (rc);

	pkgsizenet = htonl(sizeof(pkgsizenet) + size);
	rc = acc_bufferappend(acc->sendbuffer, &pkgsizenet,
	    sizeof(pkgsizenet));
	if (rc != ACHAT_RC_OK)
		goto release_buffer;
	rc = acc_bufferappend(acc->sendbuffer, msg, size);
	if (rc != ACHAT_RC_OK)
		goto release_buffer;
	sendptr = acc_bufferptr(acc->sendbuffer);

	rc = acc_flush(acc);

	return rc;
release_buffer:
	acc_bufferfree(acc->sendbuffer);
	free(acc->sendbuffer);
	acc->sendbuffer = NULL;
	return rc;
}

achat_rc
acc_receivemsg(struct achat_channel *acc, char *msg, size_t *size)
{
	achat_buffer	 pkgbuffer;
	u_int32_t	 pkgsizenet = 0;
	size_t		 len;
	achat_rc	 rc;
	char		*receiveptr;

	ACC_CHKPARAM(acc  != NULL);
	ACC_CHKPARAM(msg  != NULL);
	ACC_CHKPARAM(0 < *size && *size <=  ACHAT_MAX_MSGSIZE);

	/* Can only receive a message, if you have an open socket */
	if (acc->fd == -1)
		return (ACHAT_RC_ERROR);

	rc = acc_bufferinit(&pkgbuffer);
	if (rc != ACHAT_RC_OK) {
		/* no memory leak in OOM situations */
		/*@i1*/return rc;
	}

	len = sizeof(pkgsizenet);
	rc = acc_io(acc, read, (char*)&pkgsizenet, &len);
	if (rc != ACHAT_RC_OK)
		goto out;

	pkgsizenet = ntohl(pkgsizenet);
	if (pkgsizenet <= sizeof(pkgsizenet)) {
		rc = ACHAT_RC_ERROR;
		goto out;
	}

	len = pkgsizenet - sizeof(pkgsizenet);
	if (*size < len || len > ACHAT_MAX_MSGSIZE) {
		rc = ACHAT_RC_ERROR;
		goto out;
	}

	(void)acc_bufferappend_space(&pkgbuffer, len);
	receiveptr = acc_bufferptr(&pkgbuffer);

	rc = acc_io(acc, read, receiveptr, &len);
	if (rc != ACHAT_RC_OK)
		goto out;

	memcpy(msg, receiveptr, len);
	*size = len;

out:
	acc_bufferfree(&pkgbuffer);

	return (rc);
}

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

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "accbuffer.h"
#include "accutils.h"
#include "anoubischat.h"

#include <stdio.h>
achat_rc
acc_sendmsg(struct achat_channel *acc, const char *msg, size_t size)
{
	achat_buffer	 pkgbuffer;
	u_int32_t	 pkgsizenet = 0;
	achat_rc	 rc = ACHAT_RC_ERROR;
	char		*sendptr;

	ACC_CHKPARAM(acc  != NULL);
	ACC_CHKPARAM(msg  != NULL);
	ACC_CHKPARAM(size >  0);
	ACC_CHKSTATE(acc, ACC_STATE_ESTABLISHED);

	acc_bufferinit(&pkgbuffer);
	pkgsizenet = htonl(sizeof(pkgsizenet) + size);

	rc = acc_bufferappend(&pkgbuffer, (const void *)&pkgsizenet,
	    sizeof(pkgsizenet));
	if (rc != ACHAT_RC_OK) {
		acc_bufferfree(&pkgbuffer);
		return (rc);
	}

	rc = acc_bufferappend(&pkgbuffer, msg, size);
	if (rc != ACHAT_RC_OK) {
		acc_bufferfree(&pkgbuffer);
		return (rc);
	}

	sendptr = acc_bufferptr(&pkgbuffer);
	acc->state = ACC_STATE_TRANSFERING;

	rc = acc_io(acc, accwrite, sendptr, ntohl(pkgsizenet));

	acc->state = ACC_STATE_ESTABLISHED;
	acc_bufferfree(&pkgbuffer);

	return (rc);
}

achat_rc
acc_receivemsg(struct achat_channel *acc, char *msg, size_t size)
{
	achat_buffer	 pkgbuffer;
	u_int32_t	 pkgsizenet = 0;
	achat_rc	 rc = ACHAT_RC_OK;
	char		*receiveptr;

	ACC_CHKPARAM(acc  != NULL);
	ACC_CHKPARAM(msg  != NULL);
	ACC_CHKPARAM(size >  0);
	ACC_CHKSTATE(acc, ACC_STATE_ESTABLISHED);

	acc_bufferinit(&pkgbuffer);
	acc->state = ACC_STATE_TRANSFERING;

	rc = acc_io(acc, read, (char*)&pkgsizenet, sizeof(pkgsizenet));
	if (rc != ACHAT_RC_OK) {
		/*
		 * It seems we ost syncronisation. Closing the
		 * connection would be wise ...
		 */
		acc->state = ACC_STATE_ESTABLISHED;
		acc_close(acc);
		return (ACHAT_RC_ERROR);
	}

	acc_bufferappend_space(&pkgbuffer, ntohl(pkgsizenet));
	receiveptr = acc_bufferptr(&pkgbuffer);

	rc = acc_io(acc, read, receiveptr,
	    ntohl(pkgsizenet) - sizeof(pkgsizenet));
	if (rc != ACHAT_RC_OK) {
		/*
		 * It seems we ost syncronisation. Closing the
		 * connection would be wise ...
		 */
		acc->state = ACC_STATE_ESTABLISHED;
		acc_close(acc);
		return (ACHAT_RC_ERROR);
	}

	memcpy(msg, receiveptr, size);

	acc->state = ACC_STATE_ESTABLISHED;
	acc_bufferfree(&pkgbuffer);

	return (rc);
}

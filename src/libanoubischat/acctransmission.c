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

achat_rc
acc_sendmsg(struct achat_channel *acc, const char *msg, size_t size)
{
	achat_buffer	 pkgbuffer;
	u_int32_t	 pkgsizenet = 0;
	achat_rc	 rc = ACHAT_RC_ERROR;
	size_t		 len;
	char		*sendptr;

	ACC_CHKPARAM(acc  != NULL);
	ACC_CHKPARAM(msg  != NULL);
	ACC_CHKPARAM(0 < size && size <= ACHAT_MAX_MSGSIZE);
	ACC_CHKSTATE(acc, ACC_STATE_ESTABLISHED);

	rc = acc_bufferinit(&pkgbuffer);
	if (rc != ACHAT_RC_OK)
		return (rc);		/* XXX HJH close? */
	pkgsizenet = htonl(sizeof(pkgsizenet) + size);

	rc = acc_bufferappend(&pkgbuffer, (const void *)&pkgsizenet,
	    sizeof(pkgsizenet));
	if (rc != ACHAT_RC_OK) {
		acc_bufferfree(&pkgbuffer);
		return (rc);		/* XXX HJH close? */
	}

	rc = acc_bufferappend(&pkgbuffer, msg, size);
	if (rc != ACHAT_RC_OK) {
		acc_bufferfree(&pkgbuffer);
		return (rc);		/* XXX HJH close? */
	}

	sendptr = acc_bufferptr(&pkgbuffer);
	acc->state = ACC_STATE_TRANSFERING;

	len = ntohl(pkgsizenet);
	rc = acc_io(acc, accwrite, sendptr, &len);

	acc->state = ACC_STATE_ESTABLISHED;
	acc_bufferfree(&pkgbuffer);

	return (rc);
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
	ACC_CHKSTATE(acc, ACC_STATE_ESTABLISHED);

	rc = acc_bufferinit(&pkgbuffer);
	if (rc != ACHAT_RC_OK) {
		return rc;
	}

	acc->state = ACC_STATE_TRANSFERING;

	len = sizeof(pkgsizenet);
	rc = acc_io(acc, read, (char*)&pkgsizenet, &len);
	if (rc != ACHAT_RC_OK) {
		/*
		 * XXX HJH It might be better that the caller has to
		 * XXX HJH take care of closeing the channel.  Moreover
		 * XXX HJH the ACC_CHK* do not close the channel on failure,
		 * XXX HJH consistency?  When does the caller have to take
		 * XXX HJH care of cleaning up the channel?
		 */
		acc_close(acc);
		goto out;
	}

	pkgsizenet = ntohl(pkgsizenet);
	if (pkgsizenet <= sizeof(pkgsizenet)) {
		rc = ACHAT_RC_ERROR;
		acc_close(acc);		/* XXX HJH */
		goto out;
	}

	len = pkgsizenet - sizeof(pkgsizenet);
	if (*size < len || len > ACHAT_MAX_MSGSIZE) {
		rc = ACHAT_RC_ERROR;
		acc_close(acc);		/* XXX HJH */
		goto out;
	}

	(void)acc_bufferappend_space(&pkgbuffer, len);
	receiveptr = acc_bufferptr(&pkgbuffer);

	rc = acc_io(acc, read, receiveptr, &len);
	if (rc != ACHAT_RC_OK) {
		acc_close(acc);		/* XXX HJH */
		goto out;
	}

	memcpy(msg, receiveptr, len);
	*size = len;

out:
	acc->state = ACC_STATE_ESTABLISHED;
	acc_bufferfree(&pkgbuffer);

	return (rc);
}

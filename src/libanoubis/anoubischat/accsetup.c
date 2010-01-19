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
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#include "accutils.h"
#include "anoubis_chat.h"
#include "accbuffer.h"

struct achat_channel *
acc_create(void)
{
	struct achat_channel	*c;
	struct achat_buffer	*sb, *rb;

	/* Create storage for send/recv-buffer */
	sb = malloc(sizeof(struct achat_buffer));
	if (sb == NULL)
		return NULL;

	rb = malloc(sizeof(struct achat_buffer));
	if (rb == NULL) {
		free(sb);
		return NULL;
	}

	/* Initialize send/recv-buffer */
	if (acc_bufferinit(sb) != ACHAT_RC_OK) {
		free(sb);
		free(rb);

		return NULL;
	}

	if (acc_bufferinit(rb) != ACHAT_RC_OK) {
		acc_bufferfree(sb);
		free(sb);
		free(rb);

		return NULL;
	}

	/* Create the channel, assign send/recv-buffer */
	c = (struct achat_channel*)malloc(sizeof(struct achat_channel));
	if (c != NULL) {
		c->sendbuffer = sb;
		c->recvbuffer = rb;

		acc_clear(c);
	}
	else {
		acc_bufferfree(sb);
		acc_bufferfree(rb);
		free(sb);
		free(rb);
	}

	return (c);
}

achat_rc
acc_destroy(struct achat_channel *acc)
{
	achat_rc rc = ACHAT_RC_OK;

	ACC_CHKPARAM(acc != NULL);

	if (acc->fd != -1)
		rc = acc_close(acc);

	if (acc->sendbuffer != NULL) {
		acc_bufferfree(acc->sendbuffer);
		free(acc->sendbuffer);
		acc->sendbuffer = NULL;
	}
	if (acc->recvbuffer != NULL) {
		acc_bufferfree(acc->recvbuffer);
		free(acc->recvbuffer);
		acc->recvbuffer = NULL;
	}

	free((void*)acc);

	return (rc);
}

achat_rc
acc_clear(struct achat_channel *acc)
{
	ACC_CHKPARAM(acc != NULL);

	acc->sslmode = ACC_SSLMODE_NONE;
	acc->tail = ACC_TAIL_NONE;
	acc->blocking = ACC_BLOCKING;
	memset(&acc->addr, 0, sizeof(struct sockaddr_storage));
	acc->addrsize = 0;
	acc->fd = -1;
	acc->euid = -1;
	acc->egid = -1;
	acc_bufferclear(acc->sendbuffer);
	acc_bufferclear(acc->recvbuffer);
	acc->event = NULL;

	return (ACHAT_RC_OK);
}

achat_rc
acc_settail(struct achat_channel *acc, enum acc_tail newtail)
{
	ACC_CHKPARAM(acc != NULL);
	ACC_CHKPARAM(newtail < 3);

	/* Operation only allowed, if you have no open connection */
	if (acc->fd != -1)
		return (ACHAT_RC_ERROR);

	acc->tail = newtail;
	return (ACHAT_RC_OK);
}

achat_rc
acc_setsslmode(struct achat_channel *acc, enum acc_sslmode newsslmode)
{
	ACC_CHKPARAM(acc != NULL);
	ACC_CHKPARAM(newsslmode < 3);

	/* Operation only allowed, if you have no open connection */
	if (acc->fd != -1)
		return (ACHAT_RC_ERROR);

	/* currently no encryption is implemented */
	if (newsslmode == ACC_SSLMODE_ENCIPHERED)
		return (ACHAT_RC_NYI);

	acc->sslmode = newsslmode;
	return (ACHAT_RC_OK);
}

achat_rc
acc_setblockingmode(struct achat_channel *acc, enum acc_blockingmode newmode)
{
	ACC_CHKPARAM(acc != NULL);
	ACC_CHKPARAM(newmode < 2);

	/* Operation only allowed, if you have no open connection */
	if (acc->fd != -1)
		return (ACHAT_RC_ERROR);

	acc->blocking = newmode;
	return (ACHAT_RC_OK);
}

achat_rc
acc_setaddr(struct achat_channel *acc, struct sockaddr_storage *newsa,
	size_t addrsize)
{
	ACC_CHKPARAM(acc   != NULL);
	ACC_CHKPARAM(newsa != NULL);

	/* Operation only allowed, if you have no open connection */
	if (acc->fd != -1)
		return (ACHAT_RC_ERROR);

	bzero(&acc->addr, sizeof(acc->addr));
	bcopy(newsa, &acc->addr, sizeof(acc->addr));
	acc->addrsize = addrsize;

	return (ACHAT_RC_OK);
}

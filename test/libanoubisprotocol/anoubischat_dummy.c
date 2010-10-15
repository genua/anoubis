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

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include <sys/queue.h>

#define __used __attribute__((unused))

#include <anoubischat_dummy.h>

struct mymsgbuf {
	TAILQ_ENTRY(mymsgbuf) next;
	size_t len;
	char buf[0];
};

struct achat_channel {
	TAILQ_HEAD(, mymsgbuf) rd;
	TAILQ_HEAD(, mymsgbuf) wr;
};

struct achat_channel *acc_create(void)
{
	struct achat_channel * ret = malloc(sizeof(*ret));
	// cppcheck-suppress uninitdata
	TAILQ_INIT(&ret->rd);
	// cppcheck-suppress uninitdata
	TAILQ_INIT(&ret->wr);
	return ret;
};

achat_rc acc_destroy(struct achat_channel * chan)
{
	while(!TAILQ_EMPTY(&chan->rd)) {
		struct mymsgbuf * cur = TAILQ_FIRST(&chan->rd);
		TAILQ_REMOVE(&chan->rd, cur, next);
		free(cur);
	}
	while(!TAILQ_EMPTY(&chan->wr)) {
		struct mymsgbuf * cur = TAILQ_FIRST(&chan->wr);
		TAILQ_REMOVE(&chan->wr, cur, next);
		free(cur);
	}
	free(chan);
	return ACHAT_RC_OK;
}

achat_rc acc_clear(struct achat_channel * chan __used)
{
	return ACHAT_RC_OK;
}

achat_rc acc_settail(struct achat_channel * chan __used,
		enum acc_tail tail __used)
{
	return ACHAT_RC_OK;
}

achat_rc acc_setsslmode(struct achat_channel * chan __used,
		enum acc_sslmode mode __used)
{
	return ACHAT_RC_OK;
}

achat_rc acc_setaddr(struct achat_channel * chan __used,
		struct sockaddr_storage * ss __used)
{
	return ACHAT_RC_OK;
}

/* Subsystem Connect */
achat_rc acc_prepare(struct achat_channel * chan __used)
{
	return ACHAT_RC_OK;
}

achat_rc acc_open(struct achat_channel * chan __used)
{
	return ACHAT_RC_OK;
}

struct achat_channel *acc_opendup(struct achat_channel * chan __used)
{
	return ACHAT_RC_OK;
}

achat_rc acc_close(struct achat_channel * chan __used)
{
	return ACHAT_RC_OK;
}

achat_rc acc_getpeerids(struct achat_channel * chan __used)
{
	return ACHAT_RC_NYI;
}

/* Subsystem Transmission */
achat_rc __acc_sendmsg(struct achat_channel * chan, const char * buf,
    size_t len, int operator)
{
	struct mymsgbuf * msg = malloc(sizeof(*msg)+len);
	if (!msg)
		return ACHAT_RC_OOMEM;
	msg->len = len;
	memcpy(msg->buf, buf, len);
	if (!operator)
		TAILQ_INSERT_TAIL(&chan->wr, msg, next);
	else
		TAILQ_INSERT_TAIL(&chan->rd, msg, next);
	return ACHAT_RC_OK;
}

achat_rc acc_sendmsg(struct achat_channel * chan, const char * buf, size_t len)
{
	return __acc_sendmsg(chan, buf, len, 0);
}

achat_rc acc_sendmsg_operator(struct achat_channel * chan, const char * buf,
    size_t len)
{
	return __acc_sendmsg(chan, buf, len, 1);
}
achat_rc __acc_receivemsg(struct achat_channel * chan, char * buf,
    size_t * len, int operator)
{
	struct mymsgbuf * msg = NULL;
	if (!operator) {
		msg = TAILQ_FIRST(&chan->rd);
	} else {
		msg = TAILQ_FIRST(&chan->wr);
	}
	if (msg == NULL)
		return ACHAT_RC_EOF;
	if (msg->len > *len)
		return ACHAT_RC_ERROR;
	*len = msg->len;
	if (!operator) {
		TAILQ_REMOVE(&chan->rd, msg, next);
	} else {
		TAILQ_REMOVE(&chan->wr, msg, next);
	}
	memcpy(buf, msg->buf, *len);
	free(msg);
	return ACHAT_RC_OK;
}

achat_rc acc_receivemsg(struct achat_channel * chan, char * buf, size_t * len)
{
	return __acc_receivemsg(chan, buf, len, 0);
}

achat_rc acc_receivemsg_operator(struct achat_channel * chan,
    char * buf, size_t * len)
{
	return __acc_receivemsg(chan, buf, len, 1);
}

/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include "amsg_list.h"

/**
 * Initialize an amsg_list_context structure. This function assumes
 * that the list context is not yet initialized, it does not free
 * any memory. The context structure itself must be pre-allocated
 * by the caller. The message within the context is initialized except
 * for the flags. It does not have the error field set and it does not
 * contain any records.
 *
 * @param ctx The message context to fill (pre-allocated by the caller).
 * @param token The token for the list reply message. The session engine uses
 *     this value to demultiplex the reply to the correct client.
 * @param listtype The type of the records that will be added to the message.
 * @return Zero in case of success, a negative error code (-ENOMEM) if
 *     an error occured.
 */
int
amsg_list_init(struct amsg_list_context *ctx, uint64_t token,
    uint16_t listtype)
{
	static const int		 msgsize = 8000;

	ctx->msg = msg_factory(ANOUBISD_MSG_LISTREPLY, msgsize);
	if (!ctx->msg)
		return -ENOMEM;
	ctx->buf = abuf_open_frommem(ctx->msg->msg, 8000);
	ctx->dmsg = abuf_cast(ctx->buf, struct anoubisd_msg_listreply);
	if (!ctx->dmsg) {
		free(ctx->msg);
		return -ENOMEM;
	}
	ctx->dmsg->token = token;
	ctx->dmsg->flags = 0;
	ctx->dmsg->len = sizeof(Anoubis_ListMessage);
	ctx->buf = abuf_open(ctx->buf,
	    offsetof(struct anoubisd_msg_listreply, data));
	ctx->pmsg = abuf_cast(ctx->buf, Anoubis_ListMessage);
	if (!ctx->pmsg) {
		free(ctx->msg);
		return -ENOMEM;
	}
	set_value(ctx->pmsg->type, ANOUBIS_P_LISTREP);
	set_value(ctx->pmsg->flags, 0);
	set_value(ctx->pmsg->error, 0);
	set_value(ctx->pmsg->nrec, 0);
	set_value(ctx->pmsg->rectype, listtype);
	ctx->buf = abuf_open(ctx->buf,
	    offsetof(Anoubis_ListMessage, payload));
	ctx->curlen = sizeof(struct anoubisd_msg_listreply)
	    + sizeof(Anoubis_ListMessage);
	ctx->nrec = 0;
	ctx->flags = POLICY_FLAG_START;
	return 0;
}

/**
 * Send the message in the list context to the given queue.
 * This function sets the flags fields and the nrec field in the message.
 * The message is shrinked in size according to the curlen field in the
 * context.
 *
 * @param ctx The message context.
 * @param q The queue for the message.
 * @return None.
 */
void
amsg_list_send(struct amsg_list_context *ctx, Queue *q)
{
	ctx->dmsg->flags = ctx->flags;
	set_value(ctx->pmsg->nrec, ctx->nrec);
	set_value(ctx->pmsg->flags, ctx->flags);
	msg_shrink(ctx->msg, ctx->curlen);
	enqueue(q, ctx->msg);
	ctx->msg = NULL;
	ctx->flags = 0;
}

/**
 * Update the list context fields after a single record has been
 * added to the context's payload.
 *
 * @param ctx The reply context.
 * @param size The size of the new record.
 * @return None.
 */
void
amsg_list_addrecord(struct amsg_list_context *ctx, int size)
{
	ctx->buf = abuf_open(ctx->buf, size);
	ctx->curlen += size;
	ctx->nrec++;
	ctx->dmsg->len += size;
}

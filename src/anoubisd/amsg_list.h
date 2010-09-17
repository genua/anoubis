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

#ifndef _AMSG_LIST_H_
#define _AMSG_LIST_H_

#include "config.h"

#include <anoubis_protocol.h>
#include <anoubisd.h>
#include <anoubis_alloc.h>

/**
 * This structure is used to build reply messages for user list
 * requests. It contains all the information that is needed to build
 * a reply message. Description of individual fields:
 *
 * msg The anoubisd_msg structure that is currently under construction.
 * dmsg A pointer to the anoubis daemon header within the reply message. This
 *     header is a anoubisd_msg_listreply structure.
 * pmsg A pointer to the protocol header within the reply message. This
 *     header is of type Anoubis_ListMessage.
 * buf This buffer represents the message payload at the end of the
 *     message. The caller can use the memory in this buffer for individual
 *     reply records. Once a record is added the buffer should be modified
 *     such that it only represents the rest of the payload.
 * curlen The current length of the message without the (rest of) the payload.
 *     This is initially set to the size of the daemon header plus the size
 *     of the protocol header. The caller should adjust this value if
 *     payload data is added.
 * nrec The number of records in this message. This is initialized to zero
 *     and the caller must make sure that this value is adjusted as more
 *     records are added in the payload area.
 * flags The flags to use for the next reply. This is initialized to
 *     POLICY_FLAG_START and is cleared upon a pe_playground_send.
 */
struct amsg_list_context {
	struct anoubisd_msg			*msg;
	struct anoubisd_msg_listreply		*dmsg;
	Anoubis_ListMessage			*pmsg;
	struct abuf_buffer			 buf;
	int					 curlen;
	int					 nrec;
	int					 flags;
};

extern int	amsg_list_init(struct amsg_list_context *ctx, uint64_t token,
		    uint16_t listtype);
extern void	amsg_list_send(struct amsg_list_context *ctx, Queue *q);
extern void	amsg_list_addrecord(struct amsg_list_context *ctx, int size);

#endif	/* _AMSG_LIST_H_ */

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

#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

#include "anoubisd.h"

anoubisd_reply_t *
policy_engine(int mtype, void *request)
{
	anoubisd_reply_t *reply;
	struct eventdev_hdr *hdr = NULL;
	struct anoubisd_msg_comm *comm = NULL;

	DEBUG(DBG_TRACE, ">policy_engine");


	if (mtype == ANOUBISD_MSG_EVENTDEV)
		hdr = (struct eventdev_hdr *)request;

	if (mtype == ANOUBISD_MSG_POLREQUEST)
		comm = (anoubisd_msg_comm_t *)request;


	if ((reply = malloc(sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("policy_engine: cannot allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}
	bzero(reply, sizeof(anoubisd_reply_t));

	reply->ask = 0;		/* false */
	reply->reply = 0;	/* allow */
	reply->timeout = (time_t)0;
	reply->len = 0;

	if (mtype == ANOUBISD_MSG_POLREQUEST)
		reply->token = comm->token;

	DEBUG(DBG_TRACE, "<policy_engine");

	return reply;
}

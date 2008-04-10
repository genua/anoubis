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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

#ifdef LINUX
#include <linux/anoubis_sfs.h>
#include <linux/anoubis_alf.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis_alf.h>
#endif

#include "anoubisd.h"

anoubisd_reply_t	*policy_engine(int, void *);
anoubisd_reply_t	*pe_dispatch_event(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_process(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_alf(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_sfs(struct eventdev_hdr *);
anoubisd_reply_t	*pe_dispatch_policy(struct anoubisd_msg_comm *);

anoubisd_reply_t *
policy_engine(int mtype, void *request)
{
	anoubisd_reply_t *reply;
	struct eventdev_hdr *hdr = NULL;
	struct anoubisd_msg_comm *comm = NULL;

	DEBUG(DBG_TRACE, ">policy_engine");

	switch (mtype) {
	case ANOUBISD_MSG_EVENTDEV:
		hdr = (struct eventdev_hdr *)request;
		reply = pe_dispatch_event(hdr);
		break;

	case ANOUBISD_MSG_POLREQUEST:
		comm = (anoubisd_msg_comm_t *)request;
		reply = pe_dispatch_policy(comm);
		break;

	default:
		reply = NULL;
		break;
	}

	DEBUG(DBG_TRACE, "<policy_engine");

	return reply;
}

anoubisd_reply_t *
pe_dispatch_event(struct eventdev_hdr *hdr)
{
	anoubisd_reply_t	*reply = NULL;

	DEBUG(DBG_PE, "pe_dispatch_event: pid %u uid %u token %x %d",
	    hdr->msg_pid, hdr->msg_uid, hdr->msg_token, hdr->msg_source);

	switch (hdr->msg_source) {
	case ANOUBIS_SOURCE_PROCESS:
		reply = pe_handle_process(hdr);
		break;

	case ANOUBIS_SOURCE_SFS:
		reply = pe_handle_sfs(hdr);
		break;

	case ANOUBIS_SOURCE_ALF:
		reply = pe_handle_alf(hdr);
		break;

	default:
		break;
	}

	return reply;
}

anoubisd_reply_t *
pe_handle_process(struct eventdev_hdr *hdr)
{
#ifndef OPENBSD
	struct ac_process_message	*msg;

	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct ac_process_message))) {
		log_warn("pe_handle_process: short message");
		return NULL;
	}

	msg = (struct ac_process_message *)(hdr + 1);

	DEBUG(DBG_PE_PROC, "pe_handle_process: pid %u uid %u op %d path \"%s\"",
	    hdr->msg_pid, hdr->msg_uid, msg->op,
	    msg->op == ANOUBIS_PROCESS_OP_EXEC ? msg->pathhint : "");
#endif
	return NULL;
}

anoubisd_reply_t *
pe_handle_alf(struct eventdev_hdr *hdr)
{
	struct alf_event	*msg;
	anoubisd_reply_t	*reply = NULL;

	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct alf_event))) {
		log_warn("pe_handle_alf: short message");
		return NULL;
	}
		
	msg = (struct alf_event *)(hdr + 1);

	DEBUG(DBG_PE_ALF, "pe_hanlde_alf: pid %u uid %u token %x",
	    hdr->msg_pid, hdr->msg_uid, hdr->msg_token);

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_handle_alf: cannot allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}

	/* XXX HSH ALF events are always allowed */
	reply->ask = 0;		/* false */
	reply->reply = 0;	/* allow */
	reply->timeout = (time_t)0;
	reply->len = 0;

	DEBUG(DBG_TRACE, "<policy_engine");
	return reply;
}

anoubisd_reply_t *
pe_handle_sfs(struct eventdev_hdr *hdr)
{
	anoubisd_reply_t	*reply = NULL;
#ifdef LINUX	/* XXX not fully implemented on OpenBSD, yet. */
	struct sfs_open_message	*msg;

	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct sfs_open_message))) {
		log_warn("pe_handle_sfs: short message");
		return NULL;
	}

	msg = (struct sfs_open_message *)(hdr + 1);

	DEBUG(DBG_PE_SFS, "pe_hanlde_sfs: pid %u uid %u ino %llu dev 0x%llx "
	    "flags %lx \"%s\"", hdr->msg_pid, hdr->msg_uid,
	    msg->ino, msg->dev, msg->flags, msg->pathhint ? msg->pathhint : "");
#endif	/* LINUX */

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_handle_sfs: cannot allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}

	/* XXX HSH SFS events are always allowed */
	reply->ask = 0;		/* false */
	reply->reply = 0;	/* allow */
	reply->timeout = (time_t)0;
	reply->len = 0;

	return reply;
}


anoubisd_reply_t *
pe_dispatch_policy(struct anoubisd_msg_comm *comm)
{
	anoubisd_reply_t	*reply = NULL;

	DEBUG(DBG_PE_POLICY, "pe_dispatch_policy: token %x", comm->token);

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_dispatch_policy: cannot allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}

	reply->token = comm->token;

	return reply;
}

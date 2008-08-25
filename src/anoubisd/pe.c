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
#ifdef OPENBSD
#include <sys/limits.h>
#endif

#include <apn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef LINUX
#include <queue.h>
#include <bsdcompat.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis_sfs.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/queue.h>
#include <dev/anoubis_alf.h>
#include <dev/anoubis_sfs.h>
#include <dev/anoubis.h>
#endif

#include "anoubisd.h"
#include "pe.h"

anoubisd_reply_t	*policy_engine(anoubisd_msg_t *);
anoubisd_reply_t	*pe_dispatch_event(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_process(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_sfsexec(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_alf(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_sfs(anoubisd_msg_sfsopen_t *);

void
pe_init(void)
{
	pe_proc_init();
	pe_pubkey_init();
	pe_user_init();
}

void
pe_shutdown(void)
{
	pe_proc_flush();
	pe_user_flush_db(NULL);
	pe_pubkey_flush_db(NULL);
}

void
pe_reconfigure(void)
{
	pe_pubkey_reconfigure();
	pe_user_reconfigure();
}

anoubisd_reply_t *
policy_engine(anoubisd_msg_t *request)
{
	anoubisd_reply_t *reply;

	DEBUG(DBG_TRACE, ">policy_engine");

	switch (request->mtype) {
	case ANOUBISD_MSG_EVENTDEV: {
		struct eventdev_hdr *hdr = (struct eventdev_hdr *)request->msg;
		reply = pe_dispatch_event(hdr);
		break;
	} case ANOUBISD_MSG_SFSOPEN: {
		anoubisd_msg_sfsopen_t *sfsmsg;
		sfsmsg = (anoubisd_msg_sfsopen_t *)request->msg;
		reply = pe_handle_sfs(sfsmsg);
		break;
	} case ANOUBISD_MSG_POLREQUEST: {
		anoubisd_msg_comm_t *comm = (anoubisd_msg_comm_t *)request->msg;
		reply = pe_dispatch_policy(comm);
		break;
	} default:
		reply = NULL;
		break;
	}

	DEBUG(DBG_TRACE, "<policy_engine");

	return (reply);
}

anoubisd_reply_t *
pe_dispatch_event(struct eventdev_hdr *hdr)
{
	anoubisd_reply_t	*reply = NULL;

	DEBUG(DBG_PE, "pe_dispatch_event: pid %u uid %u token %x %d",
	    hdr->msg_pid, hdr->msg_uid, hdr->msg_token, hdr->msg_source);

	if (hdr == NULL) {
		log_warnx("pe_dispatch_event: empty header");
		return (NULL);
	}

	switch (hdr->msg_source) {
	case ANOUBIS_SOURCE_PROCESS:
		reply = pe_handle_process(hdr);
		break;

	case ANOUBIS_SOURCE_SFSEXEC:
		reply = pe_handle_sfsexec(hdr);
		break;

	case ANOUBIS_SOURCE_ALF:
		reply = pe_handle_alf(hdr);
		break;

	default:
		log_warnx("pe_dispatch_event: unknown message source %d",
		    hdr->msg_source);
		break;
	}

	return (reply);
}

anoubisd_reply_t *
pe_handle_sfsexec(struct eventdev_hdr *hdr)
{
	struct sfs_open_message		*msg;

	if (hdr == NULL) {
		log_warnx("pe_handle_sfsexec: empty header");
		return (NULL);
	}
	if ((unsigned short)hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct sfs_open_message))) {
		log_warnx("pe_handle_sfsexec: short message");
		return (NULL);
	}
	msg = (struct sfs_open_message *)(hdr+1);
	pe_proc_exec(msg->common.task_cookie,
	    hdr->msg_uid, hdr->msg_pid,
	    (msg->flags & ANOUBIS_OPEN_FLAG_CSUM) ? msg->csum : NULL,
	    (msg->flags & ANOUBIS_OPEN_FLAG_PATHHINT) ? msg->pathhint : NULL);

	return (NULL);
}

anoubisd_reply_t *
pe_handle_process(struct eventdev_hdr *hdr)
{
	struct ac_process_message	*msg;

	if (hdr == NULL) {
		log_warnx("pe_handle_process: empty header");
		return (NULL);
	}
	if ((unsigned short)hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct ac_process_message))) {
		log_warnx("pe_handle_process: short message");
		return (NULL);
	}
	msg = (struct ac_process_message *)(hdr + 1);

	switch (msg->op) {
	case ANOUBIS_PROCESS_OP_FORK:
		/* Use cookie of new process */
		pe_proc_fork(hdr->msg_uid, msg->task_cookie,
		    msg->common.task_cookie);
		break;
	case ANOUBIS_PROCESS_OP_EXIT:
		/* NOTE: Do NOT use msg->common.task_cookie here! */
		pe_proc_exit(msg->task_cookie);
		break;
	default:
		log_warnx("pe_handle_process: undefined operation %d", msg->op);
		break;
	}
	return (NULL);
}

anoubisd_reply_t *
pe_handle_alf(struct eventdev_hdr *hdr)
{
	struct alf_event	*msg;
	anoubisd_reply_t	*reply = NULL;
	struct pe_proc		*proc;

	if (hdr == NULL) {
		log_warnx("pe_handle_alf: empty header");
		return (NULL);
	}
	if ((unsigned short)hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct alf_event))) {
		log_warnx("pe_handle_alf: short message");
		return (NULL);
	}
	msg = (struct alf_event *)(hdr + 1);

	/* get process from tracker list */
	if ((proc = pe_proc_get(msg->common.task_cookie)) == NULL) {
		/*
		 * Untracked process: Do not insert it here because
		 * we do not have proper csum/path data for this process.
		 */
		DEBUG(DBG_PE_ALF, "pe_handle_alf: untrackted process %u",
		    hdr->msg_pid);
	} else {
		if (pe_proc_get_pid(proc) == -1)
			pe_proc_set_pid(proc, hdr->msg_pid);
	}
	reply = pe_decide_alf(proc, hdr);
	pe_proc_put(proc);
	DEBUG(DBG_TRACE, "<policy_engine");
	return (reply);
}

anoubisd_reply_t *
pe_handle_sfs(anoubisd_msg_sfsopen_t *sfsmsg)
{
	anoubisd_reply_t		*reply = NULL;
	struct eventdev_hdr		*hdr;
	time_t				 now;

	if (sfsmsg == NULL) {
		log_warnx("pe_handle_sfs: empty message");
		return (NULL);
	}
	hdr = (struct eventdev_hdr *)&sfsmsg->hdr;
	if ((unsigned short)hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct sfs_open_message))) {
		log_warnx("pe_handle_sfs: short message");
		return (NULL);
	}

	if (time(&now) == (time_t)-1) {
		int code = errno;
		log_warn("pe_handle_sfs: Cannot get time");
		master_terminate(code);
		return (NULL);
	}
	reply = pe_decide_sfs(hdr->msg_uid, sfsmsg, now);

	return (reply);
}

void
pe_dump(void)
{
	pe_proc_dump();
	pe_user_dump();
}

int
pe_in_scope(struct apn_scope *scope, struct anoubis_event_common *common,
    time_t now)
{
	if (!scope)
		return 1;
	if (scope->timeout && now > scope->timeout)
		return 0;
	if (scope->task && common->task_cookie != scope->task)
		return 0;
	return 1;
}

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
#include <stddef.h>

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

#include <anoubis_protocol.h>
#include "anoubisd.h"
#include "pe.h"
#include "sfs.h"

static anoubisd_reply_t	*pe_dispatch_event(struct eventdev_hdr *);
static anoubisd_reply_t	*pe_handle_process(struct eventdev_hdr *);
static anoubisd_reply_t	*pe_handle_sfsexec(struct eventdev_hdr *);
static anoubisd_reply_t	*pe_handle_alf(struct eventdev_hdr *);
static anoubisd_reply_t	*pe_handle_ipc(struct eventdev_hdr *);
static anoubisd_reply_t	*pe_handle_sfs(struct eventdev_hdr *);

void
pe_init(void)
{
	sfshash_init();
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
	sfshash_flush();
	pe_pubkey_reconfigure();
	pe_user_reconfigure();
}

/*
 * XXX CEH: If we want to take privilege separation seriously, we
 * XXX CEH: must check the length of each request and make sure that
 * XXX CEH: it is sufficiently large for the payload it is expected to
 * XXX CEH: contain.
 */
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
	} case ANOUBISD_MSG_POLREQUEST: {
		anoubisd_msg_comm_t *comm = (anoubisd_msg_comm_t *)request->msg;
		reply = pe_dispatch_policy(comm);
		break;
	} case ANOUBISD_MSG_SFSDISABLE: {
		anoubisd_msg_sfsdisable_t *sfsdisable;
		sfsdisable = (anoubisd_msg_sfsdisable_t *)request->msg;
		reply = calloc(1, sizeof(struct anoubisd_reply));
		if (reply == NULL)
			break;
		reply->token = sfsdisable->token;
		reply->ask = 0;
		reply->rule_id = 0;
		reply->prio = 0;
		reply->len = 0;
		reply->flags = POLICY_FLAG_START|POLICY_FLAG_END;
		reply->timeout = 0;
		reply->reply = EPERM;
		if (pe_proc_set_sfsdisable(sfsdisable->pid, sfsdisable->uid))
			reply->reply = 0;
		break;
	} default:
		reply = NULL;
		break;
	}

	DEBUG(DBG_TRACE, "<policy_engine");

	return (reply);
}

static anoubisd_reply_t *
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

	case ANOUBIS_SOURCE_IPC:
		reply = pe_handle_ipc(hdr);
		break;

	case ANOUBIS_SOURCE_SFS:
		reply = pe_handle_sfs(hdr);
		break;

	default:
		log_warnx("pe_dispatch_event: unknown message source %d",
		    hdr->msg_source);
		break;
	}

	return (reply);
}

static anoubisd_reply_t *
pe_handle_sfsexec(struct eventdev_hdr *hdr)
{
	struct sfs_open_message		*msg;

	if (hdr == NULL) {
		log_warnx("pe_handle_sfsexec: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
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

static anoubisd_reply_t *
pe_handle_process(struct eventdev_hdr *hdr)
{
	struct ac_process_message	*msg;

	if (hdr == NULL) {
		log_warnx("pe_handle_process: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
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
		log_warnx("pe_handle_process: undefined operation %ld",
		    msg->op);
		break;
	}
	return (NULL);
}

static anoubisd_reply_t *
pe_handle_alf(struct eventdev_hdr *hdr)
{
	struct alf_event	*msg;
	anoubisd_reply_t	*reply = NULL;
	struct pe_proc		*proc;

	if (hdr == NULL) {
		log_warnx("pe_handle_alf: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
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

static anoubisd_reply_t *
pe_handle_ipc(struct eventdev_hdr *hdr)
{
	struct ac_ipc_message	*msg;

	if (hdr == NULL) {
		log_warnx("pe_handle_ipc: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct ac_ipc_message))) {
		log_warnx("pe_handle_ipc: short message");
		return (NULL);
	}
	msg = (struct ac_ipc_message *)(hdr + 1);

	switch (msg->op) {
	case ANOUBIS_SOCKET_OP_CONNECT:
		pe_ipc_connect(msg);
		break;
	case ANOUBIS_SOCKET_OP_DESTROY:
		pe_ipc_destroy(msg);
		break;
	default:
		log_warnx("pe_handle_ipc: undefined operation %d", msg->op);
		break;
	}

	return (NULL);
}

static anoubisd_reply_t *
reply_merge(anoubisd_reply_t *r1, anoubisd_reply_t *r2)
{
	if (!r1)
		return r2;
	if (!r2)
		return r1;
	if (r1->ask == 0 && r1->reply)
		goto use1;
	if (r2->ask == 0 && r2->reply)
		goto use2;
	if (r1->ask)
		goto use1;
	if (r2->ask)
		goto use2;
use1:
	free(r2);
	return r1;
use2:
	free(r1);
	return r2;
}

static anoubisd_reply_t *
pe_handle_sfs(struct eventdev_hdr *hdr)
{
	anoubisd_reply_t		*reply = NULL, *reply2 = NULL;
	struct pe_file_event		*fevent;
	struct pe_proc			*proc;

	if (hdr == NULL) {
		log_warnx("pe_handle_sfs: empty message");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct sfs_open_message))) {
		log_warnx("pe_handle_sfs: short message");
		return (NULL);
	}
	if ((fevent = pe_parse_file_event(hdr)) == NULL) {
		log_warnx("Cannot parse sfsopen message");
		return NULL;
	}

	proc = pe_proc_get(fevent->cookie);
	if (proc && pe_proc_get_pid(proc) == -1)
		pe_proc_set_pid(proc, hdr->msg_pid);

	pe_context_open(proc, hdr);
	reply = pe_decide_sfs(proc, fevent, hdr);
	reply2 = pe_decide_sandbox(proc, fevent, hdr);

	pe_proc_put(proc);
	free(fevent);

	/* XXX CEH: This might need more thought. */
	return reply_merge(reply, reply2);
}

void
pe_dump(void)
{
	pe_proc_dump();
	pe_user_dump();
}

int
pe_in_scope(struct apn_scope *scope, anoubis_cookie_t task,
    time_t now)
{
	if (!scope)
		return 1;
	if (scope->timeout && now > scope->timeout)
		return 0;
	if (scope->task && task != scope->task)
		return 0;
	return 1;
}

struct pe_file_event *
pe_parse_file_event(struct eventdev_hdr *hdr)
{
	struct pe_file_event	*ret = NULL;
	struct sfs_open_message	*kernmsg;
	int			 sfslen, pathlen, i;

	if (!hdr)
		return NULL;
	kernmsg = (struct sfs_open_message *)(hdr+1);
	if (hdr->msg_size < sizeof(struct eventdev_hdr))
		goto err;
	sfslen = hdr->msg_size - sizeof(struct eventdev_hdr);
	if (sfslen < (int)sizeof(struct sfs_open_message))
		goto err;
	if (kernmsg->flags & ANOUBIS_OPEN_FLAG_PATHHINT) {
		pathlen = sfslen - offsetof(struct sfs_open_message, pathhint);
		for (i=pathlen-1; i >= 0; --i)
			if (kernmsg->pathhint[i] == 0)
				break;
		if (i < 0)
			goto err;
	} else {
		kernmsg->pathhint[0] = 0;
	}
	ret = malloc(sizeof(struct pe_file_event));
	if (!ret)
		return NULL;
	ret->cookie = kernmsg->common.task_cookie;
	if (kernmsg->pathhint[0])
		ret->path = strdup(kernmsg->pathhint);
	else
		ret->path = NULL;
	ret->cslen = 0;
	if (kernmsg->flags & ANOUBIS_OPEN_FLAG_CSUM) {
		ret->cslen = ANOUBIS_CS_LEN;
		bcopy(kernmsg->csum, ret->cs, ANOUBIS_CS_LEN);
	}
	ret->amask = 0;
	/* Treat FOLLOW as a read event. */
	if (kernmsg->flags & (ANOUBIS_OPEN_FLAG_READ|ANOUBIS_OPEN_FLAG_FOLLOW))
		ret->amask |= APN_SBA_READ;
	if (kernmsg->flags & ANOUBIS_OPEN_FLAG_WRITE)
		ret->amask |= APN_SBA_WRITE;
	if (kernmsg->flags & ANOUBIS_OPEN_FLAG_EXEC)
		ret->amask |= APN_SBA_EXEC;
	ret->uid = hdr->msg_uid;
	return ret;
err:
	if (ret)
		free(ret);
	return NULL;
}

int
pe_build_prefixhash(struct apn_rule *block)
{
	int			 cnt = 0, idx = 1;
	struct apn_rule		*rule;

	TAILQ_FOREACH(rule, &block->rule.chain, entry)
		cnt++;
	block->userdata = pe_prefixhash_create(cnt);
	if (!block->userdata)
		return -ENOMEM;
	DEBUG(DBG_TRACE, ">pe_build_prefixhash");
	TAILQ_FOREACH(rule, &block->rule.chain, entry) {
		int		 ret;
		const char	*prefix;
		switch (rule->apn_type) {
		case APN_SFS_ACCESS:
			prefix = rule->rule.sfsaccess.path;
			break;
		case APN_SFS_DEFAULT:
			prefix = rule->rule.sfsdefault.path;
			break;
		case APN_DEFAULT:
			prefix = NULL;
			break;
		case APN_SB_ACCESS:
			prefix = rule->rule.sbaccess.path;
			break;
		default:
			log_warnx("pe_build_prefixhash: Invalid rule "
			    "type %u in SFS rule %lu",
			    rule->apn_type, rule->apn_id);
			pe_prefixhash_destroy(block->userdata);
			block->userdata = NULL;
			DEBUG(DBG_TRACE,
			    "<pe_build_prefixhash: invalid type");
			return -EINVAL;
		}
		ret = pe_prefixhash_add(block->userdata, prefix, rule, idx);
		if (ret < 0) {
			pe_prefixhash_destroy(block->userdata);
			block->userdata = NULL;
			DEBUG(DBG_TRACE, "<pe_build_prefixhash: "
			    "add failed with %d", ret);
			return  ret;
		}
		DEBUG(DBG_TRACE, " pe_build_prefixhash: added %p", rule);
		idx++;
	}
	DEBUG(DBG_TRACE, "<pe_build_prefixhash");
	return 0;
}

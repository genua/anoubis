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
#include <sys/socket.h>
#include <sys/stat.h>
#ifdef OPENBSD
#include <sys/limits.h>
#endif

#include <arpa/inet.h>
#include <netinet/in.h>

#include <apn.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <openssl/pem.h>

#ifdef OPENBSD
#ifndef s6_addr32
#define s6_addr32 __u6_addr.__u6_addr32
#endif
#endif

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
#include "sfs.h"
#include "kernelcache.h"
#include "pe.h"

struct pe_pubkey {
	TAILQ_ENTRY(pe_pubkey)	 entry;
	uid_t			 uid;
	EVP_PKEY		*pubkey;
};
TAILQ_HEAD(pubkeys, pe_pubkey) *pubkeys;

struct pe_context {
	int			 refcount;
	struct apn_rule		*rule;
	struct apn_context	*ctx;
	struct apn_ruleset	*ruleset;
};

anoubisd_reply_t	*policy_engine(anoubisd_msg_t *);
anoubisd_reply_t	*pe_dispatch_event(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_process(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_sfsexec(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_alf(struct eventdev_hdr *);
anoubisd_reply_t	*pe_decide_alf(struct pe_proc *, struct eventdev_hdr *);
int			 pe_alf_evaluate(struct pe_proc *, struct pe_context *,
			     struct alf_event *, int *, u_int32_t *);
int			 pe_decide_alffilt(struct pe_proc *, struct apn_rule *,
			     struct alf_event *, int *, u_int32_t *, time_t);
int			 pe_decide_alfcap(struct apn_rule *, struct
			     alf_event *, int *, u_int32_t *, time_t);
int			 pe_decide_alfdflt(struct apn_rule *, struct
			     alf_event *, int *, u_int32_t *, time_t);
void			 pe_kcache_alf(struct apn_alfrule *, int,
			     struct pe_proc *, struct alf_event *);
char			*pe_dump_alfmsg(struct alf_event *,
			     struct eventdev_hdr *, int);
char			*pe_dump_ctx(struct eventdev_hdr *, struct pe_proc *,
			     int);
anoubisd_reply_t	*pe_handle_sfs(anoubisd_msg_sfsopen_t *);
anoubisd_reply_t	*pe_decide_sfs(uid_t, anoubisd_msg_sfsopen_t*,
			     time_t now);
int			 pe_decide_sfscheck(struct apn_rule *, struct
			     sfs_open_message *, int *, u_int32_t *, time_t);
int			 pe_decide_sfsdflt(struct apn_rule *, struct
			     sfs_open_message *, int *, u_int32_t *, time_t);
char			*pe_dump_sfsmsg(struct sfs_open_message *, int);

void			 pe_inherit_ctx(struct pe_proc *);
struct pe_context	*pe_search_ctx(struct apn_ruleset *, const u_int8_t *,
			     const char *);
struct pe_context	*pe_alloc_ctx(struct apn_rule *, struct apn_ruleset *);
int			 pe_decide_ctx(struct pe_proc *, const u_int8_t *,
			     const char *);
void			 pe_dump_dbgctx(const char *, struct pe_proc *);
int			 pe_addrmatch_out(struct alf_event *, struct
			     apn_alfrule *);
int			 pe_addrmatch_in(struct alf_event *, struct
			     apn_alfrule *);
int			 pe_addrmatch_host(struct apn_host *, void *,
			     unsigned short);
int			 pe_addrmatch_port(struct apn_port *, void *,
			     unsigned short);
static int		 pe_in_scope(struct apn_scope *,
			     struct anoubis_event_common *, time_t);
int			 pe_load_pubkeys(const char *, struct pubkeys *);
void			 pe_flush_pubkeys(struct pubkeys *);
int			 pe_verify_sig(const char *, uid_t);
EVP_PKEY		*pe_get_pubkey(uid_t);

/*
 * The following code utilizes list functions from BSD queue.h, which
 * cannot be reliably annotated. We therefore exclude the following
 * functions from memory checking.
 */
/*@-memchecks@*/
void
pe_init(void)
{
	struct pubkeys	*pk;

	pe_proc_init();
	if ((pk = calloc(1, sizeof(struct pubkeys))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);	/* XXX HSH */
	}
	TAILQ_INIT(pk);

	/* We die gracefully if loading of public keys fails. */
	if (pe_load_pubkeys(ANOUBISD_PUBKEYDIR, pk) == -1)
		fatal("pe_init: failed to load policy keys");
	pubkeys = pk;
	pe_user_init();
}

void
pe_shutdown(void)
{
	pe_proc_flush();
	pe_user_flush_db(NULL);
	pe_flush_pubkeys(pubkeys);
}

void
pe_reconfigure(void)
{
	struct pubkeys	*newpk, *oldpk;

	if ((newpk = calloc(1, sizeof(struct pubkeys))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);       /* XXX HSH */
	}
	TAILQ_INIT(newpk);

	if (pe_load_pubkeys(ANOUBISD_PUBKEYDIR, newpk) == -1) {
		log_warnx("pe_reconfigure: could not load public keys");
		free(newpk);
		return;
	}

	/*
	 * We switch pubkeys right now, so newly loaded policies can
	 * be verfied using the most recent keys.
	 */
	oldpk = pubkeys;
	pubkeys = newpk;
	pe_flush_pubkeys(oldpk);

	pe_user_reconfigure();
}

/*
 * Update a context.
 * If the members rule and ctx of struct pe_context are set, they
 * reference rules in the old pdb.  If similar rules are available in
 * the new pdb (ie. checksum of the application can be found), update
 * the references. Otherwise, reset them to NULL.
 *
 * If pdb is NULL the currently active policy database is used.
 */
int
pe_update_ctx(struct pe_proc *pproc, struct pe_context **newctx, int prio,
    struct pe_policy_db *pdb)
{
	struct apn_ruleset	*newrules;
	struct pe_context	*context;

	if (pproc == NULL) {
		log_warnx("pe_update_ctx: empty process");
		return (-1);
	}
	if (newctx == NULL) {
		log_warnx("pe_update_ctx: invalid new context pointer");
		return (-1);
	}
	if (prio < 0 || prio >= PE_PRIO_MAX) {
		log_warnx("pe_update_ctx: illegal priority %d", prio);
		return (-1);
	}

	/*
	 * Try to get policies for our uid from the new pdb.  If the
	 * new pdb does not provide policies for our uid, try to get the
	 * default policies.
	 */

	DEBUG(DBG_TRACE, ">pe_update_ctx");
	context = NULL;
	newrules = pe_user_get_ruleset(pe_proc_get_uid(pproc), prio, pdb);
	DEBUG(DBG_TRACE, " pe_update_ctx: newrules = %x", newrules);
	if (newrules) {
		u_int8_t		*csum = NULL;
		char			*pathhint = NULL;
		struct apn_rule		*oldrule;
		struct apn_app		*oldapp;
		struct pe_context	*ctx;

		/*
		 * XXX CEH: In some cases we might want to look for
		 * XXX CEH: a new any rule if the old context for the priority
		 * XXX CEH: or its associated ruleset is NULL.
		 */
		ctx = pe_proc_get_context(pproc, prio);
		if (ctx == NULL)
			goto out;
		oldrule = ctx->rule;
		if (!oldrule)
			goto out;
		oldapp = oldrule->app;
		if (oldapp) {
			if (oldapp->hashtype == APN_HASH_SHA256)
				csum = oldapp->hashvalue;
			pathhint = oldapp->name;
		}
		context = pe_search_ctx(newrules, csum, pathhint);
	}
out:

	DEBUG(DBG_PE_POLICY, "pe_update_ctx: context %p rule %p ctx %p",
	    context, context ? context->rule : NULL, context ? context->ctx :
	    NULL);

	*newctx = context;

	return (0);
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
	struct pe_proc			*proc = NULL;
	struct pe_context		*ctx0, *ctx1;
	struct pe_proc_ident		*pident;

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
	proc = pe_proc_get(msg->common.task_cookie);
	if (proc == NULL) {
		DEBUG(DBG_PE_PROC, "pe_handle_process: untracked "
		    "process %u 0x%08llx execs", hdr->msg_pid,
		    msg->common.task_cookie);
		return (NULL);
	}
	/* fill in checksum and pathhint */
	pident = pe_proc_ident(proc);
	if (msg->flags & ANOUBIS_OPEN_FLAG_CSUM) {
		if (pident->csum == NULL) {
			pident->csum = calloc(1, sizeof(msg->csum));
			if (pident->csum == NULL) {
				log_warn("calloc");
				master_terminate(ENOMEM);	/* XXX HSH */
			}
		}
		bcopy(msg->csum, pident->csum, sizeof(msg->csum));
	} else {
		if (pident->csum) {
			free(pident->csum);
			pident->csum = NULL;
		}
	}
	if (pident->pathhint) {
		free(pident->pathhint);
		pident->pathhint = NULL;
	}
	if (msg->flags & ANOUBIS_OPEN_FLAG_PATHHINT) {
		if ((pident->pathhint = strdup(msg->pathhint)) == NULL) {
			log_warn("strdup");
			master_terminate(ENOMEM);	/* XXX HSH */
		}
	}
	pe_proc_set_pid(proc, hdr->msg_pid);
	pe_set_ctx(proc, hdr->msg_uid, pident->csum, pident->pathhint);

	ctx0 = pe_proc_get_context(proc, 0);
	ctx1 = pe_proc_get_context(proc, 1);
	/* Get our policy */
	DEBUG(DBG_PE_PROC, "pe_handle_sfsexec: using policies %p %p "
	    "for %s csum 0x%08lx...",
	    ctx0 ? ctx0->rule : NULL, ctx1 ? ctx1->rule : NULL,
	    pident->pathhint ? pident->pathhint : "",
	    pident->csum ? htonl(*(unsigned long *)pident->csum) : 0);
	pe_proc_put(proc);
	return (NULL);
}

anoubisd_reply_t *
pe_handle_process(struct eventdev_hdr *hdr)
{
	struct ac_process_message	*msg;
	struct pe_proc			*proc = NULL;

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
		proc = pe_proc_alloc(hdr->msg_uid, msg->task_cookie,
		    msg->common.task_cookie);
		pe_inherit_ctx(proc);
		pe_proc_track(proc);
		break;
	case ANOUBIS_PROCESS_OP_EXIT:
		/* NOTE: Do NOT use msg->common.task_cookie here! */
		proc = pe_proc_get(msg->task_cookie);
		pe_proc_untrack(proc);
		break;
	default:
		log_warnx("pe_handle_process: undefined operation %d", msg->op);
		break;
	}

	if (proc) {
		struct pe_proc_ident *pident = pe_proc_ident(proc);
		DEBUG(DBG_PE_PROC, "pe_handle_process: token 0x%08llx pid %d "
		    "uid %u op %d proc %p csum 0x%08x... parent "
		    "token 0x%08llx",
		    pe_proc_task_cookie(proc), pe_proc_get_pid(proc),
		    hdr->msg_uid, msg->op,
		    proc,
		    pident->csum ? htonl(*(unsigned long *)pident->csum) : 0,
		    pe_proc_task_cookie(pe_proc_get_parent(proc)));
		pe_proc_put(proc);
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
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct alf_event))) {
		log_warnx("pe_handle_alf: short message");
		return (NULL);
	}
	msg = (struct alf_event *)(hdr + 1);

	/* get process from tracker list */
	if ((proc = pe_proc_get(msg->common.task_cookie)) == NULL) {
		DEBUG(DBG_PE_ALF, "pe_handle_alf: untrackted process %u",
		    hdr->msg_pid);
		/* Untracked process: create it, set context and track it. */
		proc = pe_proc_alloc(hdr->msg_uid, msg->common.task_cookie, 0);
		pe_set_ctx(proc, hdr->msg_uid, NULL, NULL);
		pe_proc_track(proc);
	}

	if (pe_proc_get_pid(proc) == -1)
		pe_proc_set_pid(proc, hdr->msg_pid);

	reply = pe_decide_alf(proc, hdr);
	pe_proc_put(proc);

	DEBUG(DBG_TRACE, "<policy_engine");
	return (reply);
}

anoubisd_reply_t *
pe_decide_alf(struct pe_proc *proc, struct eventdev_hdr *hdr)
{
	static char		 prefix[] = "ALF";
	static char		*verdict[3] = { "allowed", "denied", "asked" };
	struct alf_event	*msg;
	anoubisd_reply_t	*reply;
	int			 i, ret, decision, log, prio;
	u_int32_t		 rule_id = 0;
	char			*dump = NULL;
	char			*context = NULL;

	if (hdr == NULL) {
		log_warnx("pe_decide_alf: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct alf_event))) {
		log_warnx("pe_decide_alf: short message");
		return (NULL);
	}
	msg = (struct alf_event *)(hdr + 1);

	log = 0;
	prio = -1;
	rule_id = 0;
	decision = -1;

	for (i = 0; i < PE_PRIO_MAX; i++) {
		DEBUG(DBG_PE_DECALF, "pe_decide_alf: prio %d context %p", i,
		    pe_proc_get_context(proc, i));

		ret = pe_alf_evaluate(proc, pe_proc_get_context(proc, i),
		    msg, &log, &rule_id);

		if (ret != -1) {
			decision = ret;
			prio = i;
		}
		if (ret == POLICY_DENY) {
			break;
		}
	}
	DEBUG(DBG_PE_DECALF, "pe_decide_alf: decision %d", decision);

	/* If no default rule matched, decide on deny */
	if (decision == -1) {
		decision = POLICY_DENY;
		log = APN_LOG_ALERT;
	}

	if (log != APN_LOG_NONE) {
		context = pe_dump_ctx(hdr, proc, prio);
		dump = pe_dump_alfmsg(msg, hdr, ANOUBISD_LOG_APN);
	}

	/* Logging */
	switch (log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		log_info("%s prio %d rule %d %s %s (%s)", prefix, prio,
		    rule_id, verdict[decision], dump, context);
		send_lognotify(hdr, decision, log, rule_id);
		break;
	case APN_LOG_ALERT:
		log_warnx("%s prio %d rule %d %s %s (%s)", prefix, prio,
		    rule_id, verdict[decision], dump, context);
		send_lognotify(hdr, decision, log, rule_id);
		break;
	default:
		log_warnx("pe_decide_alf: unknown log type %d", log);
	}

	if (dump)
		free(dump);
	if (context)
		free(context);

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_decide_alf: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}

	reply->ask = 0;
	reply->rule_id = rule_id;
	reply->timeout = (time_t)0;
	if (decision == POLICY_ASK) {
		struct pe_proc_ident *pident = pe_proc_ident(proc);
		reply->ask = 1;
		reply->timeout = 300;	/* XXX 5 Minutes for now. */
		reply->csum = pident->csum;
		reply->path = pident->pathhint;
	}
	reply->reply = decision;
	reply->len = 0;

	return (reply);
}

int
pe_alf_evaluate(struct pe_proc *proc, struct pe_context *context,
    struct alf_event *msg, int *log, u_int32_t *rule_id)
{
	struct apn_rule	*rule;
	int		 decision;
	time_t		 t;

	if (context == NULL)
		return (-1);
	if ((rule = context->rule) == NULL) {
		log_warnx("pe_alf_evaluate: empty rule");
		return (-1);
	}
	if (msg == NULL) {
		log_warnx("pe_alf_evaluate:: empty alf event");
		return (-1);
	}

	if (time(&t) == (time_t)-1) {
		int code = errno;
		log_warn("Cannot get current time");
		master_terminate(code);
		return -1;
	}
	decision = pe_decide_alffilt(proc, rule, msg, log, rule_id, t);
	if (decision == -1)
		decision = pe_decide_alfcap(rule, msg, log, rule_id, t);
	if (decision == -1)
		decision = pe_decide_alfdflt(rule, msg, log, rule_id, t);

	DEBUG(DBG_PE_DECALF, "pe_alf_evaluate: decision %d rule %p", decision,
	    rule);

	return (decision);
}

/*
 * ALF filter logic.
 */
int
pe_decide_alffilt(struct pe_proc *proc, struct apn_rule *rule,
    struct alf_event *msg, int *log, u_int32_t *rule_id, time_t now)
{
	struct apn_alfrule	*hp;
	int			 decision;

	if (rule == NULL) {
		log_warnx("pe_decide_alffilt: empty rule");
		return (-1);
	}
	if (msg == NULL) {
		log_warnx("pe_decide_alffilt: empty alf event");
		return (-1);
	}

	/* We only decide on UDP and TCP. */
	if (msg->protocol != IPPROTO_UDP && msg->protocol != IPPROTO_TCP)
		return (-1);

	/* For TCP, we only decide on ACCEPT/CONNECT but allow SEND/RECVMSG */
	if ((msg->op == ALF_SENDMSG || msg->op == ALF_RECVMSG) &&
	    msg->protocol == IPPROTO_TCP) {
		if (log)
			*log = APN_LOG_NONE;
		return POLICY_ALLOW;
	}


	decision = -1;
	for (hp = rule->rule.alf; hp; hp = hp->next) {
		/*
		 * Skip non-filter rules.
		 */
		if (hp->type != APN_ALF_FILTER
		    || !pe_in_scope(hp->scope, &msg->common, now))
			continue;

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: family %d == %d/%p?",
		    msg->family, hp->rule.afilt.filtspec.af, hp);
		/*
		 * Address family:
		 * If the rule does not specify a certain family (ie.
		 * AF_UNSPEC), we accept any address family.  Otherwise,
		 * families have to match.
		 */
		if (msg->family != hp->rule.afilt.filtspec.af &&
		    hp->rule.afilt.filtspec.af != AF_UNSPEC)
			continue;

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: operation %d",
		    msg->op);
		/*
		 * Operation
		 */
		switch (msg->op) {
		case ALF_ANY:
			/* XXX HSH ? */
			continue;

		case ALF_CONNECT:
			if (hp->rule.afilt.filtspec.netaccess != APN_CONNECT &&
			    hp->rule.afilt.filtspec.netaccess != APN_SEND &&
			    hp->rule.afilt.filtspec.netaccess != APN_BOTH)
				continue;
			break;

		case ALF_ACCEPT:
			if (hp->rule.afilt.filtspec.netaccess != APN_ACCEPT &&
			    hp->rule.afilt.filtspec.netaccess != APN_RECEIVE &&
			    hp->rule.afilt.filtspec.netaccess != APN_BOTH)
				continue;
			break;

		case ALF_SENDMSG:
		case ALF_RECVMSG:
			break;

		default:
			continue;
		}

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: socket type %d",
		   msg->type);
		/*
		 * Socket type
		 */
		if (msg->type != SOCK_STREAM && msg->type != SOCK_DGRAM)
			continue;

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: protocol %d",
		    msg->protocol);
		/*
		 * Protocol
		 */
		if (msg->protocol != hp->rule.afilt.filtspec.proto)
			continue;

		/*
		 * Check addresses.
		 */
		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: address check op %d",
		    msg->op);
		switch (msg->protocol) {
		case IPPROTO_TCP:
			if (msg->op == ALF_CONNECT) {
				if (pe_addrmatch_out(msg, hp) == 0)
					continue;
			} else if (msg->op == ALF_ACCEPT) {
				if (pe_addrmatch_in(msg, hp) == 0)
					continue;
			} else if (msg->op == ALF_SENDMSG || msg->op ==
			    ALF_RECVMSG) {
				/*
				 * this case is now handled above, this code
				 * is no longer in use
				 */
				if (pe_addrmatch_out(msg, hp) == 0 &&
				    pe_addrmatch_in(msg, hp) == 0)
					continue;
			} else
				continue;
			break;

		case IPPROTO_UDP:
			if (msg->op == ALF_CONNECT || msg->op == ALF_SENDMSG) {
				if (pe_addrmatch_out(msg, hp) == 0)
					continue;
			} else if (msg->op == ALF_ACCEPT || msg->op ==
			    ALF_RECVMSG) {
				if (pe_addrmatch_in(msg, hp) == 0)
					continue;
			} else
				continue;
			break;

		default:
			continue;
		}

		/*
		 * Decide.
		 */
		switch (hp->rule.afilt.action) {
		case APN_ACTION_ALLOW:
			decision = POLICY_ALLOW;
			pe_kcache_alf(hp, decision, proc, msg);
			break;
		case APN_ACTION_DENY:
			decision = POLICY_DENY;
			pe_kcache_alf(hp, decision, proc, msg);
			break;
		case APN_ACTION_ASK:
			decision = POLICY_ASK;
			break;
		default:
			log_warnx("pe_decide_alffilt: invalid action %d",
			    hp->rule.afilt.action);
			continue;
		}

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: decision %d",
		    decision);

		if (log)
			*log = hp->rule.afilt.filtspec.log;
		if (rule_id)
			*rule_id = hp->id;
		break;
	}

	return (decision);
}

void
pe_kcache_alf(struct apn_alfrule *rule, int decision, struct pe_proc *proc,
    struct alf_event *msg)
{
	struct anoubis_kernel_policy *policy;
	struct alf_rule *alfrule;

	if (msg->op != ALF_SENDMSG && msg->op != ALF_RECVMSG)
		return;

	policy = malloc(sizeof(struct anoubis_kernel_policy) +
	    sizeof(struct alf_rule));
	if (policy == NULL)
		return;

	policy->anoubis_source = ANOUBIS_SOURCE_ALF;
	policy->rule_len = sizeof(struct alf_rule);
	policy->decision = decision;
	if (rule->rule.afilt.filtspec.statetimeout > 0) {
		policy->expire = rule->rule.afilt.filtspec.statetimeout +
		    time(NULL);
	} else {
		policy->expire = 0;
	}

	alfrule = (struct alf_rule*)(policy->rule);

	alfrule->family = msg->family;
	alfrule->protocol = msg->protocol;
	alfrule->type = msg->type;
	alfrule->op = msg->op;

	switch(alfrule->family) {
	case AF_INET:
		alfrule->local.port_min = msg->local.in_addr.sin_port;
		alfrule->local.port_max = msg->local.in_addr.sin_port;
		alfrule->peer.port_min = msg->peer.in_addr.sin_port;
		alfrule->peer.port_max = msg->peer.in_addr.sin_port;
		alfrule->local.addr.in_addr =
		    msg->local.in_addr.sin_addr;
		alfrule->peer.addr.in_addr =
		    msg->peer.in_addr.sin_addr;
		break;

	case AF_INET6:
		alfrule->local.port_min = msg->local.in6_addr.sin6_port;
		alfrule->local.port_max = msg->local.in6_addr.sin6_port;
		alfrule->peer.port_min = msg->peer.in6_addr.sin6_port;
		alfrule->peer.port_max = msg->peer.in6_addr.sin6_port;
		alfrule->local.addr.in6_addr =
		    msg->local.in6_addr.sin6_addr;
		alfrule->peer.addr.in6_addr =
		    msg->peer.in6_addr.sin6_addr;
		break;
	}

	pe_proc_kcache_add(proc, policy);

	free(policy);
}

/*
 * ALF capability logic.
 */
int
pe_decide_alfcap(struct apn_rule *rule, struct alf_event *msg, int *log,
    u_int32_t *rule_id, time_t now)
{
	int			 decision;
	struct apn_alfrule	*hp;

	if (rule == NULL) {
		log_warnx("pe_decide_alfcap: empty rule");
		return (POLICY_DENY);
	}
	if (msg == NULL) {
		log_warnx("pe_decide_alfcap: empty alf event");
		return (POLICY_DENY);
	}

	/* We decide on everything except UDP and TCP */
	if (msg->protocol == IPPROTO_UDP || msg->protocol == IPPROTO_TCP)
		return (-1);

	decision = -1;
	hp = rule->rule.alf;
	while (hp) {
		/* Skip non-capability rules. */
		if (hp->type != APN_ALF_CAPABILITY ||
		    !pe_in_scope(hp->scope, &msg->common, now)) {
			hp = hp->next;
			continue;
		}
		/*
		 * We have three types of capabilities:
		 *
		 * APN_ALF_CAPRAW: We allow packets with socket type SOCK_RAW
		 * and SOCK_PACKET (ie. ping, dhclient)
		 *
		 * APN_ALF_CAPOTHER: We allow everything else.
		 *
		 * APN_ALF_CAPALL: Allow anything.
		 */
		switch (hp->rule.acap.capability) {
		case APN_ALF_CAPRAW:
			if (msg->type == SOCK_RAW)
				decision = POLICY_ALLOW;
#ifdef LINUX
			if (msg->type == SOCK_PACKET)
				decision = POLICY_ALLOW;
#endif
			break;
		case APN_ALF_CAPOTHER:
#ifdef LINUX
			if (msg->type != SOCK_RAW && msg->type != SOCK_PACKET)
#else
			if (msg->type != SOCK_RAW)
#endif
				decision = POLICY_ALLOW;
			break;
		case APN_ALF_CAPALL:
			decision = POLICY_ALLOW;
			break;
		default:
			log_warnx("pe_decide_alfcap: unknown capability %d",
			    hp->rule.acap.capability);
			decision = -1;
		}

		if (decision != -1) {
			if (log)
				*log = hp->rule.acap.log;
			if (rule_id)
				*rule_id = hp->id;
			break;
		}
		hp = hp->next;
	}

	return (decision);
}

/*
 * ALF default logic.
 */
int
pe_decide_alfdflt(struct apn_rule *rule, struct alf_event *msg, int *log,
    u_int32_t *rule_id, time_t now)
{
	struct apn_alfrule	*hp;
	int			 decision;

	if (rule == NULL) {
		log_warnx("pe_decide_alfdflt: empty rule");
		return (-1);
	}

	decision = -1;
	hp = rule->rule.alf;
	while (hp) {
		/* Skip non-default rules. */
		if (hp->type != APN_ALF_DEFAULT
		    || !pe_in_scope(hp->scope, &msg->common, now)) {
			hp = hp->next;
			continue;
		}
		/*
		 * Default rules are easy, just pick the specified action.
		 */
		switch (hp->rule.apndefault.action) {
		case APN_ACTION_ALLOW:
			decision = POLICY_ALLOW;
			break;
		case APN_ACTION_DENY:
			decision = POLICY_DENY;
			break;
		case APN_ACTION_ASK:
			decision = POLICY_ASK;
			break;
		default:
		log_warnx("pe_decide_alfdflt: unknown action %d",
			    hp->rule.apndefault.action);
			decision = -1;
		}

		if (decision != -1) {
			if (log)
				*log = hp->rule.apndefault.log;
			if (rule_id)
				*rule_id = hp->id;
			break;
		}
		hp = hp->next;
	}

	return (decision);
}

/*
 * Decode an ALF message into a printable string.  This strings is
 * allocated and needs to be freed by the caller.
 */
char *
pe_dump_alfmsg(struct alf_event *msg, struct eventdev_hdr *hdr, int format)
{
	unsigned short	 lport, pport;
	char		*op, *af, *type, *proto, *dump;

	/*
	 * v4 address: 4 * 3 digits + 3 dots + 1 \0 = 16
	 * v6 address: 16 * 4 digits + 15 colons + 1 \0 = 79
	 *
	 * v6 address might also have some leading colons, etc. so 128
	 * bytes are more than sufficient and safe.
	 */
	char		 local[128], peer[128];

	if (msg == NULL)
		return (NULL);

	pport = lport = 0;
	snprintf(local, 128, "<unknown>");
	snprintf(peer, 128, "<unknown>");

	switch (msg->op) {
	case ALF_ANY:
		op = "any";
		break;
	case ALF_CONNECT:
		op = "connect";
		break;
	case ALF_ACCEPT:
		op = "accept";
		break;
	case ALF_SENDMSG:
		op = "sendmsg";
		break;
	case ALF_RECVMSG:
		op = "recvmsg";
		break;
	default:
		op = "<unknown>";
	}

	switch(msg->family) {
	case AF_INET:
		af = "inet";
		lport = ntohs(msg->local.in_addr.sin_port);
		pport = ntohs(msg->peer.in_addr.sin_port);

		if (inet_ntop(msg->family, &msg->local.in_addr.sin_addr,
		    local, sizeof(local)) == NULL)
			snprintf(local, 128, "<unknown>");
		if (inet_ntop(msg->family, &msg->peer.in_addr.sin_addr, peer,
		    sizeof(peer)) == NULL)
			snprintf(peer, 128, "<unknown>");
		break;

	case AF_INET6:
		af = "inet6";
		lport = ntohs(msg->local.in6_addr.sin6_port);
		pport = ntohs(msg->peer.in6_addr.sin6_port);

		if (inet_ntop(msg->family, &msg->local.in6_addr.sin6_addr,
		    local, sizeof(local)))
			snprintf(local, 128, "<unknown>");
		if (inet_ntop(msg->family, &msg->peer.in6_addr.sin6_addr, peer,
		    sizeof(peer)) == NULL)
			snprintf(peer, 128, "<unknown>");
		break;
#ifdef LINUX
	case AF_PACKET:
		af = "packet";
		break;
#endif
	case AF_UNSPEC:
		af = "unspec";
		break;
	default:
		af = "<unknown>";
	}
	switch(msg->type) {
	case SOCK_STREAM:
		type = "stream";
		break;
	case SOCK_DGRAM:
		type = "dgram";
		break;
	case SOCK_RAW:
		type = "raw";
		break;
	case SOCK_RDM:
		type = "rdm";
		break;
	case SOCK_SEQPACKET:
		type = "seqpacket";
		break;
#ifdef LINUX
	case SOCK_PACKET:
		type = "packet";
		break;
#endif
	default:
		type = "<unknown>";
	}
	switch (msg->protocol) {
	case IPPROTO_ICMP:
		proto = "icmp";
		break;
	case IPPROTO_UDP:
		proto = "udp";
		break;
	case IPPROTO_TCP:
		proto = "tcp";
		break;
	default:
		proto = "<unknown>";
	}

	switch (format) {
	case ANOUBISD_LOG_RAW:
		if (asprintf(&dump, "%s (%u): uid %u pid %u %s (%u) %s (%u) "
		    "%s (%u) local %s:%hu peer %s:%hu", op, msg->op,
		    hdr->msg_uid, hdr->msg_pid, type, msg->type, proto,
		    msg->protocol, af, msg->family, local, lport, peer, pport)
		    == -1) {
			dump = NULL;
		}
		break;

	case ANOUBISD_LOG_APN:
		if (asprintf(&dump, "%s %s %s from %s port %hu to %s port %hu",
		    op, af, proto, local, lport, peer, pport) == -1) {
			dump = NULL;
		}
		break;

	default:
		log_warnx("pe_dump_alfmsg: illegal logging format %d", format);
		dump = NULL;
	}

	return (dump);
}

/*
 * Decode a context in a printable string.  This strings is allocated
 * and needs to be freed by the caller.
 */
char *
pe_dump_ctx(struct eventdev_hdr *hdr, struct pe_proc *proc, int prio)
{
	struct pe_context	*ctx;
	unsigned long		 csumctx;
	char			*dump, *progctx;
	struct pe_proc_ident	*pident;

	/* hdr must be non-NULL, proc might be NULL */
	if (hdr == NULL)
		return (NULL);

	csumctx = 0;
	progctx = "<none>";

	if (proc && 0 <= prio && prio <= PE_PRIO_MAX) {
		ctx = pe_proc_get_context(proc, prio);

		if (ctx && ctx->rule) {
			if (ctx->rule->app) {
				csumctx = htonl(*(unsigned long *)
				    ctx->rule->app->hashvalue);
				progctx = ctx->rule->app->name;
			} else
				progctx = "any";
		}
	}

	pident = pe_proc_ident(proc);
	if (asprintf(&dump, "uid %hu pid %hu program %s checksum 0x%08x... "
	    "context program %s checksum 0x%08lx...",
	    hdr->msg_uid, hdr->msg_pid,
	    (pident && pident->pathhint) ? pident->pathhint : "<none>",
	    (pident && pident->csum) ?
	    htonl(*(unsigned long *)pident->csum) : 0,
	    progctx, csumctx) == -1) {
		dump = NULL;
	}

	return (dump);
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
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
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

anoubisd_reply_t *
pe_decide_sfs(uid_t uid, anoubisd_msg_sfsopen_t *sfsmsg, time_t now)
{
	static char		 prefix[] = "SFS";
	static char		*verdict[3] = { "allowed", "denied", "asked" };
	anoubisd_reply_t	*reply = NULL;
	struct eventdev_hdr	*hdr;
	struct sfs_open_message	*msg;
	int			 ret, log, i, decision, prio;
	u_int32_t		 rule_id = 0;
	char			*dump = NULL;
	char			*context = NULL;

	hdr = &sfsmsg->hdr;
	msg = (struct sfs_open_message *)(hdr+1);

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_handle_sfs: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}

	decision = -1;
	prio = -1;
	rule_id = 0;
	log = 0;

	if (msg->flags & ANOUBIS_OPEN_FLAG_CSUM) {
		for (i = 0; i < PE_PRIO_MAX; i++) {
			struct apn_ruleset	*rs;
			struct apn_rule		*rule;

			rs = pe_user_get_ruleset(uid, i, NULL);
			if (rs == NULL)
				continue;

			if (TAILQ_EMPTY(&rs->sfs_queue))
				continue;

				TAILQ_FOREACH(rule, &rs->sfs_queue, entry) {
				ret = pe_decide_sfscheck(rule, msg,
				    &log, &rule_id, now);

				if (ret == -1)
					ret = pe_decide_sfsdflt(rule,
					    msg, &log, &rule_id, now);

				if (ret != -1) {
					decision = ret;
					prio = i;
				}
				if (ret == POLICY_DENY)
					break;
			}
			if (decision == POLICY_DENY)
				break;
		}
		/* Look into checksum from /var/lib/sfs */
		if ((decision == -1) &&
		    (sfsmsg->anoubisd_csum_set != ANOUBISD_CSUM_NONE)) {
			if (memcmp(msg->csum, sfsmsg->anoubisd_csum,
			    ANOUBIS_SFS_CS_LEN))
				decision = POLICY_DENY;
		}
	} else {
		if (sfsmsg->anoubisd_csum_set != ANOUBISD_CSUM_NONE)
			decision = POLICY_DENY;
	}

	if (decision == -1)
		decision = POLICY_ALLOW;

	if (log != APN_LOG_NONE) {
		context = pe_dump_ctx(hdr, pe_proc_get(msg->common.task_cookie),
		    prio);
		dump = pe_dump_sfsmsg(msg, ANOUBISD_LOG_APN);
	}

	/* Logging */
	switch (log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		log_info("%s prio %d rule %d %s %s (%s)", prefix, prio,
		    rule_id, verdict[decision], dump, context);
		send_lognotify(hdr, decision, log, rule_id);
		break;
	case APN_LOG_ALERT:
		log_warnx("%s %s %s (%s)", prefix, prio, rule_id,
		    verdict[decision], dump, context);
		send_lognotify(hdr, decision, log, rule_id);
		break;
	default:
		log_warnx("pe_decide_sfs: unknown log type %d", log);
	}

	if (dump)
	    free(dump);
	if (context)
		free(context);

	reply->reply = decision;
	reply->ask = 0;		/* false */
	reply->rule_id = rule_id;
	reply->timeout = (time_t)0;
	reply->len = 0;

	return (reply);
}

int
pe_decide_sfscheck(struct apn_rule *rule, struct sfs_open_message *msg,
    int *log, u_int32_t *rule_id, time_t now)
{
	struct apn_sfsrule	*hp;
	int			 decision;

	if (rule == NULL) {
		log_warnx("pe_decide_sfscheck: empty rule");
		return (-1);
	}
	if (msg == NULL) {
		log_warnx("pe_decide_sfscheck: empty sfs event");
		return (-1);
	}

	decision = -1;
	for (hp = rule->rule.sfs; hp; hp = hp->next) {
		/*
		 * skip non-check rules
		 */
		if (hp->type != APN_SFS_CHECK
		    || !pe_in_scope(hp->scope, &msg->common, now))
			continue;

		if (strcmp(hp->rule.sfscheck.app->name, msg->pathhint))
			continue;

		decision = POLICY_ALLOW;
		/*
		 * If a SFS rule exists for a file, the decision is always
		 * deny if the checksum does not match.
		 */
		if (memcmp(msg->csum, hp->rule.sfscheck.app->hashvalue,
		    ANOUBIS_SFS_CS_LEN))
			decision = POLICY_DENY;

		if (log)
			*log = hp->rule.sfscheck.log;
		if (rule_id)
			*rule_id = hp->id;

		break;
	}

	return decision;
}

int
pe_decide_sfsdflt(struct apn_rule *rule, struct sfs_open_message *msg, int *log,
    u_int32_t *rule_id, time_t now)
{
	struct apn_sfsrule	*hp;
	int			 decision;

	if (rule == NULL) {
		log_warnx("pe_decide_sfsdflt: empty rule");
		return (-1);
	}

	decision = -1;
	hp = rule->rule.sfs;
	while (hp) {
		/* Skip non-default rules. */
		if (hp->type != APN_SFS_DEFAULT
		    || !pe_in_scope(hp->scope, &msg->common, now)) {
			hp = hp->next;
			continue;
		}
		/*
		 * Default rules are easy, just pick the specified action.
		 */
		switch (hp->rule.apndefault.action) {
		case APN_ACTION_ALLOW:
			decision = POLICY_ALLOW;
			break;
		case APN_ACTION_DENY:
			decision = POLICY_DENY;
			break;
		case APN_ACTION_ASK:
			decision = POLICY_ASK;
			break;
		default:
		log_warnx("pe_decide_sfsdflt: unknown action %d",
			    hp->rule.apndefault.action);
			decision = -1;
		}

		if (decision != -1) {
			if (log)
				*log = hp->rule.apndefault.log;
			if (rule_id)
				*rule_id = hp->id;
			break;
		}
		hp = hp->next;
	}

	return (decision);
}

/*
 * Decode a SFS message into a printable string.  The strings is allocated
 * and needs to be freed by the caller.
 *
 * "format" is specified for symmetry with pe_dump_alfmsg().
 */
char *
pe_dump_sfsmsg(struct sfs_open_message *msg, int format)
{
	char	*dump, *access;

	if (msg == NULL)
		return (NULL);

	if (msg->flags & ANOUBIS_OPEN_FLAG_READ)
		access = "read";
	else if (msg->flags & ANOUBIS_OPEN_FLAG_WRITE)
		access = "write";
	else
		access = "<unknown access>";

	if (asprintf(&dump, "%s %s", access,
	    msg->flags & ANOUBIS_OPEN_FLAG_PATHHINT ? msg->pathhint : "<none>")
	    == -1) {
		dump = NULL;
	}

	return (dump);
}

/*
 * Inherit the parent process context. This might not necessarily be
 * the direct parent process, but some grand parent.  This happens on
 * fork(2).  Moreover, we set a reference to that particular parent.
 *
 * If our parent was never tracked, we get a new context.
 */
void
pe_inherit_ctx(struct pe_proc *proc)
{
	struct pe_proc	*parent;
	int		 i;

	if (proc == NULL) {
		log_warnx("pe_inherit_ctx: empty process");
		return;
	}

	/* Get parents context */
	parent = pe_proc_get_parent(proc);
	DEBUG(DBG_PE_CTX, "pe_inherit_ctx: parent %p 0x%08llx", parent,
	    pe_proc_task_cookie(parent));

	if (parent && !pe_proc_valid_context(parent)) {
		DEBUG(DBG_PE_CTX, "pe_inherit_ctx: parent %p 0x%08llx has no "
		    "context", parent, pe_proc_task_cookie(parent));
	}

	if (parent && pe_proc_valid_context(parent)) {
		for (i = 0; i < PE_PRIO_MAX; i++) {
			struct pe_context *ctx = pe_proc_get_context(parent, i);
			pe_proc_set_context(proc, i, ctx);
		}
		pe_proc_set_parent(proc, parent);

		pe_dump_dbgctx("pe_inherit_ctx", proc);
	} else {
		/*
		 * No parent available, derive new context:  We have
		 * neither an UID, nor pathname and checksum.  Thus the only
		 * possible context will be derived from the admin/default
		 * rule set.  If this is not available, the process will
		 * get no context and susequent policy decisions will not
		 * yield a valid result (ie. != -1).  In that case,
		 * the policy engine will enforce the decision POLICY_DENY.
		 */
		pe_set_ctx(proc, -1, NULL, NULL);
	}
}

/*
 * Set our context.  If we never inherited one, search for an apropriate
 * application rule and use that one as our context from now on.
 */
void
pe_set_ctx(struct pe_proc *proc, uid_t uid, const u_int8_t *csum,
    const char *pathhint)
{
	int			 i;
	struct pe_context	*tmpctx;

	/* We can both handle csum and pathhint being NULL. */
	if (proc == NULL) {
		log_warnx("pe_set_ctx: empty process");
		return;
	}

	/*
	 * If we have inherited a context, check if it has context rule
	 * that allows us to switch context.  If no, we keep the context and
	 * just return.  Otherwise, we continue and search our new context.
	 */
	if (pe_proc_valid_context(proc)) {
		tmpctx = pe_proc_get_context(proc, 0);
		DEBUG(DBG_PE_CTX, "pe_set_ctx: proc %p 0x%08llx has context "
		    "%p rule %p ctx %p", proc, pe_proc_task_cookie(proc),
		    tmpctx, tmpctx ? tmpctx->rule : NULL,
		    tmpctx ? tmpctx->ctx : NULL);

		if (pe_decide_ctx(proc, csum, pathhint) != 1) {
			DEBUG(DBG_PE_CTX, "pe_set_ctx: keeping context");
			return;
		}
		DEBUG(DBG_PE_CTX, "pe_set_ctx: switching context");
	}

	/*
	 * If we are not tracked (which actually should not happen), or we
	 * are allowed to switch context we use the first matching
	 * application rule as new context.
	 * Note that pe_search_ctx will get a reference to the new context
	 * and pe_proc_set_context will get another one. We need to drop one
	 * of them.
	 */
	for (i = 0; i < PE_PRIO_MAX; i++) {
		struct apn_ruleset *ruleset;
		ruleset = pe_user_get_ruleset(uid, i, NULL);
		tmpctx = pe_search_ctx(ruleset, csum, pathhint);
		pe_proc_set_context(proc, i, tmpctx);
		pe_put_ctx(tmpctx);
	}
	pe_proc_set_parent(proc, proc);
	if (uid >= 0)
		pe_proc_set_uid(proc, uid);

	tmpctx = pe_proc_get_context(proc, 0);
	DEBUG(DBG_PE_CTX, "pe_set_ctx: proc %p 0x%08llx got context "
	    "%p rule %p ctx %p", proc, pe_proc_task_cookie(proc),
	    tmpctx, tmpctx ? tmpctx->rule : NULL, tmpctx ? tmpctx->ctx : NULL);
}

struct pe_context *
pe_search_ctx(struct apn_ruleset *rs, const u_int8_t *csum,
    const char *pathhint)
{
	struct pe_context	*context;
	struct apn_rule		*prule, *rule;
	struct apn_alfrule	*alfrule;
	struct apn_app		*hp;

	DEBUG(DBG_PE_CTX, "pe_search_ctx: ruleset %p csum 0x%08x...", rs,
	    csum ? htonl(*(unsigned long *)csum) : 0);

	/*
	 * We do not check csum and pathhint for being NULL, as we
	 * handle empty checksums and pathhint is actually not used, yet.
	 * The point is, that a process without a checksum can match
	 * a rule specifying "any" as application (see below).
	 */
	if (rs == NULL)
		return (NULL);

	/*
	 * XXX HSH Right now only ALF rules provide a context.  This is
	 * XXX HSH likely to change when sandboxing is implemented.
	 */
	if (TAILQ_EMPTY(&rs->alf_queue))
		return (NULL);

	rule = NULL;
	TAILQ_FOREACH(prule, &rs->alf_queue, entry) {
		/*
		 * A rule without application will always match.  This is
		 * especially important, when no checksum is available,
		 * as we still have to walk the full tailq.  Thus we use
		 * "continue" below
		 */
		if (prule->app == NULL) {
			rule = prule;
			break;
		}
		if (csum == NULL)
			continue;

		/*
		 * Otherwise walk chain of applications and check
		 * against hashvalue.
		 */
		for (hp = prule->app; hp && rule == NULL; hp = hp->next) {
			if (bcmp(hp->hashvalue, csum, sizeof(hp->hashvalue)))
				continue;
			rule = prule;
		}
		if (rule)
			break;
	}
	if (rule == NULL) {
		DEBUG(DBG_PE_CTX, "pe_search_ctx: no rule found");
		return (NULL);
	}
	context = pe_alloc_ctx(rule, rs);
	if (!context) {
		master_terminate(ENOMEM);
		return NULL;
	}

	/*
	 * Now we have a rule chain, search for a context rule.  It is
	 * ok, if we do not find a context rule.  This will mean, that
	 * we have to stay in the current context on exec(2).
	 */
	for (alfrule = rule->rule.alf; alfrule; alfrule = alfrule->next) {
		if (alfrule->type != APN_ALF_CTX)
			continue;
		context->ctx = &alfrule->rule.apncontext;
		break;
	}

	DEBUG(DBG_PE_CTX, "pe_search_ctx: context %p rule %p %s", context,
	    rule, context->ctx ? (context->ctx->application ?
	    context->ctx->application->name : "any") : "<none>");

	return (context);
}

/*
 * NOTE: rs must not be NULL.
 */
struct pe_context *
pe_alloc_ctx(struct apn_rule *rule, struct apn_ruleset *rs)
{
	struct pe_context	*ctx;

	if (!rs) {
		log_warnx("NULL rulset in pe_alloc_ctx?");
		return NULL;
	}
	if ((ctx = calloc(1, sizeof(struct pe_context))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);	/* XXX HSH */
		return NULL;
	}
	ctx->rule = rule;
	ctx->ruleset = rs;
	ctx->refcount = 1;

	return (ctx);
}

void
pe_put_ctx(struct pe_context *ctx)
{
	if (!ctx || --(ctx->refcount))
		return;

	free(ctx);
}

void
pe_reference_ctx(struct pe_context *ctx)
{
	if (!ctx)
		return;

	ctx->refcount++;
}

/*
 * Decide, if it is ok to switch context for an application specified
 * by csum and pathhint.  Right now, pathhint is not used, only csum.
 *
 * Returns 1 if switching is ok, 0 if not and -1 if no decision can
 * be made.
 *
 * NOTE: bcmp returns 0 on match, otherwise non-zero.  Do not confuse this...
 */
int
pe_decide_ctx(struct pe_proc *proc, const u_int8_t *csum, const char *pathhint)
{
	struct apn_app	*hp;
	int		 cmp, decision, i;

	/*
	 * NOTE: Once we actually use the pathhint in policiy decision
	 * NOTE: this shortcut will no longer be valid.
	 */
	if (!csum)
		return (0);

	/*
	 * Iterate over all priorities, starting with the highest.
	 * If a priority does not provide a context, we just continue.
	 *
	 * I f a priority provides a context, we have to evaluate that one.
	 * If switiching is not allowed we stop return the decision.
	 * Other wise we continue with the evaluation.
	 */
	decision = -1;
	for (i = 0; i < PE_PRIO_MAX; i++) {
		struct pe_context *ctx = pe_proc_get_context(proc, i);
		/* No context means, no decision, just go on. */
		if (ctx == NULL) {
			continue;
		}
		/* Context without rule means, not switching. */
		if (ctx->ctx == NULL) {
			decision = 0;
			break;
		}
		/*
		 * If the rule says "context new any", application will be
		 * empty.  In that case, initialize the compare result cmp to 0
		 * (ie. match).
		 */
		hp = ctx->ctx->application;
		if (hp == NULL)
			cmp = 0;
		while (hp) {
			cmp = bcmp(hp->hashvalue, csum, sizeof(hp->hashvalue));
			if (cmp == 0)
				break;
			hp = hp->next;
		}

		/*
		 * If cmp is 0, there was a matching rule, ie. switching
		 * is allowed.
		 */
		if (cmp == 0) {
			decision = 1;

			DEBUG(DBG_PE_CTX,
			    "pe_decide_ctx: found \"%s\" csm 0x%08x... for "
			    "priority %d", (hp && hp->name) ? hp->name : "any"
			    , (hp && hp->hashvalue) ?
			    htonl(*(unsigned long *)hp->hashvalue) : 0, i);
		} else
			decision = 0;
	}

	return (decision);
}

void
pe_dump(void)
{
	pe_proc_dump();
	pe_user_dump();
}

void
pe_dump_dbgctx(const char *prefix, struct pe_proc *proc)
{
	int	i;

	if (proc == NULL) {
		DEBUG(DBG_PE_CTX, "%s: prio %d rule %p ctx %p", prefix, -1,
		    NULL, NULL);
		return;
	}

	for (i = 0; i < PE_PRIO_MAX; i++) {
		struct pe_context *ctx = pe_proc_get_context(proc, i);
		DEBUG(DBG_PE_CTX, "%s: prio %d rule %p ctx %p", prefix, i,
		    ctx ? ctx->rule : NULL, ctx ? ctx->ctx : NULL);
	}
}

/*
 * Check outgoing connection. 1 on match, 0 otherwise.
 */
int
pe_addrmatch_out(struct alf_event *msg, struct apn_alfrule *rule)
{
	struct apn_host	*fromhost, *tohost;
	struct apn_port	*fromport, *toport;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_out: msg %p rule %p", msg, rule);

	if (rule == NULL) {
		log_warnx("pe_addrmatch_out: empty rule");
		return (0);
	}
	if (msg == NULL) {
		log_warnx("pe_addrmatch_out: empty event");
		return (0);
	}

	fromhost = rule->rule.afilt.filtspec.fromhost;
	fromport = rule->rule.afilt.filtspec.fromport;;
	tohost = rule->rule.afilt.filtspec.tohost;
	toport = rule->rule.afilt.filtspec.toport;

	/*
	 * fromhost == local?
	 */
	if (pe_addrmatch_host(fromhost, (void *)&msg->local, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(fromport, (void *)&msg->local, msg->family) == 0)
		return (0);

	/*
	 * tohost == peer?
	 */
	if (pe_addrmatch_host(tohost, (void *)&msg->peer, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(toport, (void *)&msg->peer, msg->family) == 0)
		return (0);

	return (1);
}

/*
 * Check incoming connection. 1 on match, 0 otherwise.
 */
int
pe_addrmatch_in(struct alf_event *msg, struct apn_alfrule *rule)
{
	struct apn_host	*fromhost, *tohost;
	struct apn_port	*fromport, *toport;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_in: msg %p rule %p", msg, rule);

	if (rule == NULL) {
		log_warnx("pe_addrmatch_out: empty rule");
		return (0);
	}
	if (msg == NULL) {
		log_warnx("pe_addrmatch_out: empty event");
		return (0);
	}

	fromhost = rule->rule.afilt.filtspec.fromhost;
	fromport = rule->rule.afilt.filtspec.fromport;;
	tohost = rule->rule.afilt.filtspec.tohost;
	toport = rule->rule.afilt.filtspec.toport;

	/* fromhost == peer? */
	if (pe_addrmatch_host(fromhost, (void *)&msg->peer, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(fromport, (void *)&msg->peer, msg->family) == 0)
		return (0);

	/* tohost == local? */
	if (pe_addrmatch_host(tohost, (void *)&msg->local, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(toport, (void *)&msg->local, msg->family) == 0)
		return (0);

	return (1);
}

/*
 * compare addr1 and addr2 with netmask of length len
 */
static int
compare_ip6_mask(struct in6_addr *addr1, struct in6_addr *addr2, int len)
{
	int		 bits, match, i;
	struct in6_addr	 mask;
	u_int32_t	*pmask;

	if (len > 128)
		len = 128;

	if (len == 128)
		return !bcmp(addr1, addr2, sizeof(struct in6_addr));

	if (len < 0)
		len = 0;

	bits = len;
	bzero(&mask, sizeof(struct in6_addr));
	pmask = &mask.s6_addr32[0];
	while (bits >= 32) {
		*pmask = 0xffffffff;
		pmask++;
		bits -= 32;
	}
	if (bits > 0) {
		*pmask = htonl(0xffffffff << (32 - bits));
	}

	match = 1;
	for (i=0; i <=3 ; i++) {
		if ((addr1->s6_addr32[i] & mask.s6_addr32[i]) !=
		    (addr2->s6_addr32[i] & mask.s6_addr32[i])) {
			match = 0;
			break;
		}
	}

	return match;
}

/*
 * Compare addresses. 1 on match, 0 otherwise.
 */
int
pe_addrmatch_host(struct apn_host *host, void *addr, unsigned short af)
{
	struct apn_host		*hp;
	struct sockaddr_in	*in;
	struct sockaddr_in6	*in6;
	in_addr_t		 mask;
	int			 match;
	int			 negate;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_host: host %p addr %p af %d", host,
	    addr, af);

	/* Empty host means "any", ie. match always. */
	if (host == NULL)
		return (1);

	if (addr == NULL) {
		log_warnx("pe_addrmatch_host: no address specified");
		return (0);
	}

	if (host->addr.af != af)
		return (0);

	hp = host;
	while (hp) {
		negate = hp->negate;
		switch (af) {
		case AF_INET:
			in = (struct sockaddr_in *)addr;
			mask = htonl(~0UL << (32 - hp->addr.len));
			match = ((in->sin_addr.s_addr & mask) ==
			    (hp->addr.apa.v4.s_addr & mask));
			break;

		case AF_INET6:
			in6 = (struct sockaddr_in6 *)addr;
			match = compare_ip6_mask(&in6->sin6_addr,
			    &hp->addr.apa.v6, hp->addr.len);
			break;

		default:
			log_warnx("pe_addrmatch_host: unknown address family "
			    "%d", af);
			match = negate = 0;
		}
		if ((!negate && match) || (negate && !match))
			break;
		hp = hp->next;
	}

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_host: match %d, negate %d", match,
	    negate);

	if (negate)
	    return (!match);
	else
	    return (match);
}

int
pe_addrmatch_port(struct apn_port *port, void *addr, unsigned short af)
{
	struct apn_port		*pp;
	struct sockaddr_in	*in;
	struct sockaddr_in6	*in6;
	int			 match;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_port: port %p addr %p af %d", port,
	    addr, af);

	/* Empty port means "any", ie. match always. */
	if (port == NULL)
		return (1);

	if (addr == NULL) {
		log_warnx("pe_addrmatch_port: no address specified");
		return (0);
	}

	pp = port;
	while (pp) {
		switch (af) {
		case AF_INET:
			in = (struct sockaddr_in *)addr;
			if (pp->port2) {
				match = (in->sin_port >= pp->port &&
					in->sin_port <= pp->port2);
			}
			else
				match = (in->sin_port == pp->port);
			break;

		case AF_INET6:
			in6 = (struct sockaddr_in6 *)addr;
			if (pp->port2) {
				match = (in6->sin6_port >= pp->port &&
					in6->sin6_port <= pp->port2);
			}
			else
				match = (in6->sin6_port == pp->port);
			break;

		default:
			log_warnx("pe_addrmatch_port: unknown address "
			    "family %d", af);
			match = 0;
		}
		if (match)
			break;
		pp = pp->next;
	}

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_port: match %d", match);

	return (match);
}

static int
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

int
pe_load_pubkeys(const char *dirname, struct pubkeys *pubkeys)
{
	DIR			*dir;
	FILE			*fp;
	struct dirent		*dp;
	struct pe_pubkey	*pk;
	uid_t			 uid;
	int			 count;
	const char		*errstr;
	char			*filename;

	DEBUG(DBG_PE_POLICY, "pe_load_pubkeys: %s %p", dirname, pubkeys);

	if (pubkeys == NULL) {
		log_warnx("pe_load_pubkeys: illegal pubkey queue");
		return (0);
	}

	if ((dir = opendir(dirname)) == NULL) {
		log_warn("opendir");
		return (0);
	}

	count = 0;

	/* iterate over directory entries */
	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_type != DT_REG)
			continue;
		/*
		 * Validate the file name: It has to be a numeric uid.
		 */
		uid = strtonum(dp->d_name, 0, UID_MAX, &errstr);
		if (errstr) {
			log_warnx("pe_load_pubkeys: filename \"%s/%s\" %s",
			    dirname, dp->d_name, errstr);
			continue;
		}
		if (asprintf(&filename, "%s/%s", dirname, dp->d_name) == -1) {
			log_warnx("asprintf: Out of memory");
			continue;
		}
		if ((pk = calloc(1, sizeof(struct pe_pubkey))) == NULL) {
			log_warnx("calloc: Out or memory");
			free(filename);
			continue;
		}
		if ((fp = fopen(filename, "r")) == NULL) {
			log_warn("fopen");
			free(pk);
			free(filename);
			continue;
		}
		free(filename);

		if ((pk->pubkey = PEM_read_PUBKEY(fp, NULL, NULL, NULL))
		    == NULL) {
			log_warn("PEM_read_PUBKEY");
			free(pk);
			fclose(fp);
			continue;
		}
		pk->uid = uid;
		fclose(fp);
		TAILQ_INSERT_TAIL(pubkeys, pk, entry);
		count++;
	}

	if (closedir(dir) == -1)
		log_warn("closedir");

	DEBUG(DBG_PE_POLICY, "pe_load_pubkeys: %d public keys loaded", count);

	return (count);
}

void
pe_flush_pubkeys(struct pubkeys *pk)
{
	struct pe_pubkey	*p, *next;

	for (p = TAILQ_FIRST(pk); p != TAILQ_END(pk); p = next) {
		next = TAILQ_NEXT(p, entry);
		TAILQ_REMOVE(pk, p, entry);

		EVP_PKEY_free(p->pubkey);
		free(p);
	}
}

int
pe_verify_sig(const char *filename, uid_t uid)
{
	char		 buffer[1024];
	EVP_PKEY	*sigkey;
	EVP_MD_CTX	 ctx;
	unsigned char	*sigbuf;
	char		*sigfile;
	int		 fd, n, siglen, result;

	if (asprintf(&sigfile, "%s.sig", filename) == -1) {
		log_warnx("asprintf: Out of memory");
		return (0);	/* XXX policy will not be loaded... */
	}

	/* If no signature file is available, return successfully */
	if ((fd = open(sigfile, O_RDONLY)) == -1) {
		free(sigfile);
		return (1);
	}
	free(sigfile);

	/* Fail when we have no public key for that uid */
	if ((sigkey = pe_get_pubkey(uid)) == NULL) {
		log_warnx("pe_verify_sig: no key for uid %lu available",
		    (unsigned long)uid);
		close(fd);
		return (0);
	}

	/* Read in signature */
	siglen = EVP_PKEY_size(sigkey);
	if ((sigbuf = calloc(siglen, sizeof(unsigned char))) == NULL) {
		close(fd);
		return (0);
	}
	if (read(fd, sigbuf, siglen) != siglen) {
		log_warnx("pe_verify_sig: error reading signature file %s",
		    sigfile);
		close(fd);
		free(sigbuf);
		return (0);
	}
	close(fd);

	EVP_MD_CTX_init(&ctx);
	if (EVP_VerifyInit(&ctx, EVP_sha1()) == 0) {
		log_warnx("pe_verify_sig: could not verify signature");
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return (0);
	}

	/* Open policy file */
	if ((fd = open(filename, O_RDONLY)) == -1) {
		log_warnx("pe_verify_sig: could not read policy %s", filename);
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return (0);
	}

	while ((n = read(fd, buffer, sizeof(buffer))) > 0)
		EVP_VerifyUpdate(&ctx, buffer, n);
	if (n == -1) {
		log_warn("read");
		close(fd);
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return (0);
	}
	close(fd);

	/* Verify */
	if ((result = EVP_VerifyFinal(&ctx, sigbuf, siglen, sigkey)) == -1)
		log_warnx("pe_verify_sig: could not verify signature");

	EVP_MD_CTX_cleanup(&ctx);
	free(sigbuf);

	return (result);
}

EVP_PKEY *
pe_get_pubkey(uid_t uid)
{
	struct pe_pubkey	*p;

	TAILQ_FOREACH(p, pubkeys, entry) {
		if (p->uid == uid)
			return (p->pubkey);
	}

	return (NULL);
}

int pe_context_uses_rs(struct pe_context *ctx, struct apn_ruleset *rs)
{
	return ctx && rs && (ctx->ruleset == rs);
}

struct apn_rule *
pe_context_get_rule(struct pe_context *ctx)
{
	if (!ctx)
		return NULL;
	return ctx->rule;
}

/*@=memchecks@*/

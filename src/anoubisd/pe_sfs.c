/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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
#include <bsdcompat.h>
#include <linux/anoubis_sfs.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis_sfs.h>
#include <dev/anoubis.h>
#endif

#include <sys/queue.h>

#include <anoubis_protocol.h>

#include "anoubisd.h"
#include "sfs.h"
#include "cert.h"
#include "pe.h"

static char		*pe_dump_sfsmsg(struct pe_file_event *);

static struct apn_default *
pe_sfs_match_one(struct apn_rule *rule, struct pe_file_event *fevent,
    time_t now, int *matchp)
{
	/* XXX CEH: This static variable is a hack! */
	static struct apn_default	 tmpresult;
	char				*prefix = NULL;
	struct apn_subject		*subject = NULL;
	u_int8_t			 csum[ANOUBIS_CS_LEN];
	u_int8_t			*cs = NULL;
	int				 len, ret;

	if (!pe_in_scope(rule->scope, fevent->cookie, now))
		return NULL;
	switch (rule->apn_type) {
	case APN_SFS_ACCESS:
		prefix = rule->rule.sfsaccess.path;
		subject = &rule->rule.sfsaccess.subject;
		break;
	case APN_SFS_DEFAULT:
		tmpresult.log = rule->rule.sfsdefault.log;
		tmpresult.action = rule->rule.sfsdefault.action;
		prefix = rule->rule.sfsdefault.path;
		break;
	default:
		log_warnx("Invalid rule type %u in SFS rule %lu",
		    rule->apn_type, rule->apn_id);
		return NULL;
	}
	if (prefix != NULL) {
		if (fevent->path == NULL)
			return NULL;
		len = strlen(prefix);
		/* Allow trailing slashes on the prefix. Important for "/" */
		while(len && prefix[len-1] == '/')
			len--;
		if (strncmp(fevent->path, prefix, len) != 0)
			return NULL;
		if (fevent->path[len] && fevent->path[len] != '/')
			return NULL;
	}
	if (rule->apn_type == APN_SFS_DEFAULT) {
		(*matchp) = ANOUBIS_SFS_DEFAULT;
		return &tmpresult;
	}

	cs = NULL;
	switch (subject->type) {
	case APN_CS_UID:
		ret = sfshash_get_uid(fevent->path, subject->value.uid, csum);
		if (ret == 0)
			cs = csum;
		break;
	case APN_CS_KEY:
		ret = sfshash_get_key(fevent->path, subject->value.keyid, csum);
		if (ret == 0)
			cs = csum;
		break;
	case APN_CS_UID_SELF:
		ret = sfshash_get_uid(fevent->path, fevent->uid, csum);
		if (ret == 0)
			cs = csum;
		break;
	case APN_CS_KEY_SELF: {
		char		*keyid;
		keyid = cert_keyid_for_uid(fevent->uid);
		if (!keyid)
			break;
		ret = sfshash_get_key(fevent->path, keyid, csum);
		free(keyid);
		if (ret == 0)
			cs = csum;
		break;
	}
	default:
		break;
	}
	if (cs == NULL) {
		(*matchp) = ANOUBIS_SFS_UNKNOWN;
		return &rule->rule.sfsaccess.unknown;
	}
	/*
	 * Special case checksum handling during an upgrade:
	 * - If a checksum with the correct length is present
	 *   assume that it matches.
	 * - Additionally, assume a checksum match if the process is
	 *   an upgrader (PE_UPGRADE_WRITEOK).
	 * In all other cases continue with normal processing.
	 */
	if (fevent->upgrade_flags & PE_UPGRADE_TOUCHED) {
		if ((fevent->cslen == ANOUBIS_CS_LEN)
		    || (fevent->upgrade_flags & PE_UPGRADE_WRITEOK)) {
			(*matchp) = ANOUBIS_SFS_VALID;
			return &rule->rule.sfsaccess.valid;
		}
	}
	if ((fevent->cslen != ANOUBIS_CS_LEN) ||
	    (memcmp(cs, fevent->cs, ANOUBIS_CS_LEN) != 0)) {
		(*matchp) = ANOUBIS_SFS_INVALID;
		return &rule->rule.sfsaccess.invalid;
	}
	(*matchp) = ANOUBIS_SFS_VALID;
	return &rule->rule.sfsaccess.valid;
}

int
pe_sfs_getrules(uid_t uid, int prio, const char *path,
	struct apn_rule ***rulelist)
{
	struct apn_ruleset	*rs;
	struct apn_rule		*sfsrules;
	int			 rulecnt, error;

	rs = pe_user_get_ruleset(uid, prio, NULL);
	if (rs == NULL)
		return 0;
	if (TAILQ_EMPTY(&rs->sfs_queue))
		return 0;
	sfsrules = TAILQ_FIRST(&rs->sfs_queue);

	DEBUG(DBG_PE_SFS, " pe_sfs_getrules: rules prio %d rules %p",
				prio, sfsrules);

	if (TAILQ_EMPTY(&sfsrules->rule.chain))
		return 0;
	if (sfsrules->userdata == NULL) {
		error = pe_build_prefixhash(sfsrules);
		if (error < 0)
			return error;
	}

	error = pe_prefixhash_getrules(sfsrules->userdata, path,
	    rulelist, &rulecnt);
	if (error < 0)
		return error;

	if (rulecnt == 0)
		free(*rulelist);

	return rulecnt;
}

anoubisd_reply_t *
pe_decide_sfs(struct pe_proc *proc, struct pe_file_event *fevent,
    struct eventdev_hdr *hdr)
{
	static const char	*verdict[3] = { "allowed", "denied", "asked" };
	anoubisd_reply_t	*reply = NULL;
	int			 secure = 0;
	int			 decision = -1;
	int			 sfsmatch = ANOUBIS_SFS_NONE;
	int			 rule_id = 0;
	int			 i, prio = 0;
	int			 log = APN_LOG_NONE;
	char			*context = NULL, *dump = NULL;
	time_t			 now;
	char			 cstext[2*ANOUBIS_CS_LEN+1];

	DEBUG(DBG_PE_SFS, ">pe_decide_sfs");
	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_handle_sfs: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}
	if (proc && pe_proc_is_secure(proc))
		secure = 1;
	if (time(&now) == (time_t)-1) {
		log_warn("pe_decide_sfs: Cannot get current time");
		goto err;
	}

	for (i = 0; i < PE_PRIO_MAX; i++) {
		struct apn_rule		**rulelist = NULL;
		int			  rulecnt, r;

		if (secure
		    && pe_context_is_nosfs(pe_proc_get_context(proc, i))) {
			/* NOSFS enforced by the context. Do nothing. */
			DEBUG(DBG_PE_SFS," pe_decide_sfs: nosfs enabled for prio %d", i);
			continue;
		}

		rulecnt = pe_sfs_getrules(fevent->uid, i, fevent->path,
					&rulelist);

		if (rulecnt == 0)
			continue;
		else if (rulecnt < 0) {
			decision = APN_ACTION_DENY;
			break;
		}

		DEBUG(DBG_PE_SFS," pe_decide_sfs: %d rules from hash", rulecnt);
		for (r=0; r<rulecnt; ++r) {
			struct	apn_default	*res;
			struct apn_rule		*rule = rulelist[r];
			int			 match = ANOUBIS_SFS_NONE;

			res = pe_sfs_match_one(rule, fevent, now, &match);
			if (res == NULL)
				continue;
			/*
			 * XXX CEH: This does NOT honour a LOG directive
			 * XXX CEH: on a CONTINUE action.
			 */
			if (res->action == APN_ACTION_CONTINUE)
				continue;
			rule_id = rule->apn_id;
			log = res->log;
			prio = i;
			switch (res->action) {
			case APN_ACTION_ALLOW:
			case APN_ACTION_DENY:
			case APN_ACTION_ASK:
				decision = res->action;
				sfsmatch = match;
				break;
			default:
				log_warnx("Invalid action %d in rule %lu",
				    res->action, rule->apn_id);
				decision = APN_ACTION_DENY;
				sfsmatch = match;
				break;
			}
			DEBUG(DBG_PE_SFS," pe_decide_sfs: decision = %d rule %d prio %d",
			    decision, rule_id, prio);
			break;
		}
		free(rulelist);
		if (decision != -1 && decision != APN_ACTION_ALLOW)
			break;
	}

	if (decision == -1)
		decision = APN_ACTION_ALLOW;

	cstext[0] = 0;
	if (log != APN_LOG_NONE) {
		int i;
		context = pe_context_dump(hdr, proc, prio);
		dump = pe_dump_sfsmsg(fevent);
		if (fevent->cslen == ANOUBIS_CS_LEN) {
			for (i=0; i<ANOUBIS_CS_LEN; ++i)
				sprintf(&cstext[2*i], "%02x", fevent->cs[i]);
			cstext[2*ANOUBIS_CS_LEN] = 0;
		}
	}
	/* Logging */
	switch (log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		log_info("token %u: SFS prio %d rule %d %s %s csum=%s (%s)",
		    hdr->msg_token, prio, rule_id, verdict[decision], dump,
		    cstext, context);
		send_lognotify(hdr, decision, log, rule_id, prio,
		    sfsmatch);
		break;
	case APN_LOG_ALERT:
		log_warnx("token %u: SFS prio %d rule %d %s %s csum=%s (%s)",
		    hdr->msg_token, prio, rule_id, verdict[decision], dump,
		    cstext, context);
		send_lognotify(hdr, decision, log, rule_id, prio,
		    sfsmatch);
		break;
	default:
		log_warnx("pe_decide_sfs: unknown log type %d", log);
	}

	if (dump)
		free(dump);
	if (context)
		free(context);

	reply->ask = 0;
	reply->hold = 0;
	reply->log = log;
	reply->rule_id = rule_id;
	reply->prio = prio;
	reply->sfsmatch = sfsmatch;
	reply->timeout = (time_t)0;
	if (decision == APN_ACTION_DENY)
		reply->reply = EPERM;
	else
		reply->reply = 0;
	reply->len = 0;
	if (decision == APN_ACTION_ASK) {
		reply->ask = 1;
		reply->timeout = 300;
		reply->pident = pe_proc_ident(proc);
		reply->ctxident = pe_context_get_ident(
		    pe_proc_get_context(proc, reply->prio));
	}
	DEBUG(DBG_PE_SFS, "<pe_decide_sfs");
	return (reply);
err:
	reply->reply = EPERM;
	reply->rule_id = 0;
	reply->prio = 0;
	return reply;
}

/*
 * Decode a SFS message into a printable string.  The string is allocated
 * and needs to be freed by the caller.
 */
static char *
pe_dump_sfsmsg(struct pe_file_event *event)
{
	char	*dump, access[4];
	int	 k = 0;

	if (event == NULL)
		return (NULL);

	if (event->amask & APN_SBA_READ)
		access[k++] = 'r';
	if (event->amask & APN_SBA_WRITE)
		access[k++] = 'w';
	if (event->amask & APN_SBA_EXEC)
		access[k++] = 'x';
	access[k] = 0;
	if (asprintf(&dump, "%s %s", access, event->path) == -1)
		dump = NULL;
	return dump;
}

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

#include <apn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef LINUX
#include <queue.h>
#include <bsdcompat.h>
#include <linux/anoubis_sfs.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/queue.h>
#include <dev/anoubis_sfs.h>
#include <dev/anoubis.h>
#endif

#include <stddef.h>

#include <anoubis_protocol.h>

#include "anoubisd.h"
#include "sfs.h"
#include "cert.h"
#include "pe.h"

struct result {
	int log;
	int rule_id;
	int prio;
	int decision;
};

static void
pe_sb_evaluate(struct apn_rule **rulelist, int rulecnt,
    struct pe_file_event *sbevent, struct result *res, unsigned int atype,
    int prio)
{
	struct apn_rule		*match = NULL;
	time_t			 now;
	int			 i;

	if ((sbevent->amask & atype) == 0) {
		/* Do not touch rule_id, prio or log level. */
		res->decision = APN_ACTION_ALLOW;
		return;
	}
	if (time(&now) == (time_t)-1) {
		log_warn("pe_sb_evaluate: Cannot get current time");
		goto err;
	}
	for (i=0; i<rulecnt; ++i) {
		struct apn_rule	*sbrule = rulelist[i];
		char		*prefix;
		int		 cstype;
		u_int8_t	*cs;
		u_int8_t	 csum[ANOUBIS_CS_LEN];
		int		 ret;

		if (sbrule->apn_type == APN_DEFAULT) {
			if (!match)
				match = sbrule;
			continue;
		}
		if (sbrule->apn_type != APN_SB_ACCESS) {
			log_warn("Invalid rule type %d on sandbox queue",
			    sbrule->apn_type);
			continue;
		}
		if (!pe_in_scope(sbrule->scope, sbevent->cookie, now))
			continue;
		if ((sbrule->rule.sbaccess.amask & atype) == 0)
			continue;
		if ((prefix = sbrule->rule.sbaccess.path)) {
			int len = 0;
			if (!sbevent->path)
				continue;
			len = strlen(prefix);
			/* Allow trailing slashes in prefix. Important for / */
			while(len && prefix[len-1] == '/')
				len --;
			if (strncmp(sbevent->path, prefix, len) != 0)
				continue;
			if (sbevent->path[len] && sbevent->path[len] != '/')
				continue;
		}
		cstype = sbrule->rule.sbaccess.cs.type;
		if (cstype == APN_CS_NONE) {
			match = sbrule;
			goto have_match;
		}
		/*
		 * No match if checksum required but no checksum present
		 * in event.
		 */
		if (sbevent->cslen != ANOUBIS_CS_LEN)
			continue;
		cs = NULL;
		ret = 0;
		switch (cstype) {
		case APN_CS_CSUM:
			cs = sbrule->rule.sbaccess.cs.value.csum;
			if (!cs)
				goto err;
			break;
		case APN_CS_UID_SELF:
			ret = sfshash_get_uid(sbevent->path,
			    sbevent->uid, csum);
			if (ret == 0)
				cs = csum;
			break;
		case APN_CS_UID:
			ret = sfshash_get_uid(sbevent->path,
			    sbrule->rule.sbaccess.cs.value.uid, csum);
			if (ret == 0)
				cs = csum;
			break;
		case APN_CS_KEY_SELF: {
			char	*keyid;
			keyid = cert_keyid_for_uid(sbevent->uid);
			if (!keyid)
				break;
			ret = sfshash_get_key(sbevent->path, keyid, csum);
			free(keyid);
			if (ret == 0)
				cs = csum;
			break;
		}
		case APN_CS_KEY:
			ret = sfshash_get_key(sbevent->path,
			    sbrule->rule.sbaccess.cs.value.keyid, csum);
			if (ret == 0)
				cs = csum;
			break;
		}
		if (ret != 0 && ret != -ENOENT)
			log_warnx("sfshash_get: Error %d", -ret);
		if (!cs)
			continue;
		if (bcmp(cs, sbevent->cs, ANOUBIS_CS_LEN) != 0)
			continue;
		match = sbrule;
		goto have_match;
	}
	if (!match)
		return;
have_match:
	/*
	 * Matching rule is in @match. This might be a default rule
	 * or a real match.
	 */
	if (match->apn_type == APN_DEFAULT) {
		res->decision = match->rule.apndefault.action;
		if (match->rule.apndefault.log > res->log)
			res->log = match->rule.apndefault.log;
	} else if (match->apn_type == APN_SB_ACCESS) {
		res->decision = match->rule.sbaccess.action;
		if (match->rule.sbaccess.log > res->log)
			res->log = match->rule.sbaccess.log;
	} else {
		log_warnx("Inconsistent rule type");
		goto err;
	}
	res->rule_id = match->apn_id;
	res->prio = prio;
	DEBUG(DBG_SANDBOX, " pe_sb_evaluate: decision = %d rule %d prio %d",
	    res->decision, res->rule_id,  res->prio);
	return;
err:
	res->decision = APN_ACTION_DENY;
	res->rule_id = 0;
	res->prio = 0;
	res->log = APN_LOG_NONE;
}

int
pe_sb_getrules (struct pe_proc *proc, uid_t uid, int prio, const char *path,
    struct apn_rule	***rulelist)
{
	struct apn_rule	 *sbrules;
	int		  rulecnt;

	/*
	 * If we do not have a process, find the default rules
	 * for the given UID (if any) and try to apply these.
	 */
	if (proc) {
		sbrules = pe_context_get_sbrule(
		    pe_proc_get_context(proc, prio));
		DEBUG(DBG_SANDBOX, " pe_sb_getrules: context rules "
		    "prio %d rules %p", prio, sbrules);

	} else {
		struct apn_ruleset	*rs;
		rs = pe_user_get_ruleset(uid, prio, NULL);
		if (rs) {
			TAILQ_FOREACH(sbrules, &rs->sb_queue, entry)
				if (sbrules->app == NULL)
					break;
		} else {
			sbrules = NULL;
		}
		DEBUG(DBG_SANDBOX, " pe_sb_getrules: default rules "
		    "prio %d rules %p", prio, sbrules);
	}

	/*
	 * Give the next priority a chance if we do not have
	 * any policy. This is default allow.
	 */
	if (!sbrules)
		return (0);

	if (!sbrules->userdata) {
		if (pe_build_prefixhash(sbrules) < 0) {
			master_terminate(ENOMEM);
			return (-1);
		}
	}

	if (pe_prefixhash_getrules(sbrules->userdata, path,
				rulelist, &rulecnt) < 0) {
		master_terminate(ENOMEM);
		return (-1);
	}

	if (rulecnt == 0)
		free(*rulelist);

	return (rulecnt);
}

static char *
pe_sb_dumpevent(struct pe_file_event *sbevent)
{
	char	*ret = NULL;
	char	 mask[4];
	int	 k = 0;

	/* XXX Need to dump more info */
	if (sbevent->amask & APN_SBA_READ)
		mask[k++] = 'r';
	if (sbevent->amask & APN_SBA_WRITE)
		mask[k++] = 'w';
	if (sbevent->amask & APN_SBA_EXEC)
		mask[k++] = 'x';
	mask[k] = 0;
	if (asprintf(&ret, "path %s %s", sbevent->path, mask) == -1)
		ret = NULL;
	return ret;
}

/*
 * We use a default allow policy for the cases where no policy is
 * given. This can be changed be writing an explicit Policy with
 * a default deny rule.
 */
anoubisd_reply_t *
pe_decide_sandbox(struct pe_proc *proc, struct pe_file_event *sbevent,
    struct eventdev_hdr *hdr)
{
	struct result		 res[3];
	struct result		 final;
	struct anoubisd_reply	*reply;
	char			*dump = NULL, *context = NULL;
	int			 i, j;
	static const char	*verdict[3] = { "allowed", "denied", "asked" };

	if (!sbevent)
		return NULL;

	DEBUG(DBG_SANDBOX, ">pe_decide_sandbox");
	for (i=0; i<3; ++i) {
		res[i].rule_id = 0;
		res[i].prio = 0;
		res[i].decision = -1;
		res[i].log = APN_LOG_NONE;
	}
	final.decision = -1;
	for (i=0; i<PE_PRIO_MAX; ++i) {
		struct apn_rule		**rulelist = NULL;
		int			  rulecnt;

		rulecnt = pe_sb_getrules(proc, sbevent->uid, i, sbevent->path,
					&rulelist);

		if (rulecnt == 0)
			continue;
		else if (rulecnt < 0) {
			final.decision = APN_ACTION_DENY;
			break;
		}

		/*
		 * The three results may be pre-initialized from a higher
		 * priority. However, the only possibilities are
		 * uniniatilized (-1) or APN_ACTION_ALLOW.
		 */
		pe_sb_evaluate(rulelist, rulecnt, sbevent, &res[0],
		    APN_SBA_READ, i);
		pe_sb_evaluate(rulelist, rulecnt, sbevent, &res[1],
		    APN_SBA_WRITE, i);
		pe_sb_evaluate(rulelist, rulecnt, sbevent, &res[2],
		    APN_SBA_EXEC, i);
		free(rulelist);
		/*
		 * If any of the events results in DENY we are done here.
		 * If any of the events results in ASK we are done, too.
		 * However, a DENY result for another event at the same
		 * priority takes precedence.
		 */
		for (j=0; j<3; ++j) {
			if (res[j].decision == APN_ACTION_DENY) {
				final.decision = APN_ACTION_DENY;
				break;
			} else if (res[j].decision == APN_ACTION_ASK) {
				final.decision = APN_ACTION_ASK;
			} else if (final.decision == -1) {
				final.decision = APN_ACTION_ALLOW;
			}
		}
		/* If we have a verdict and it is not ALLOW we are done. */
		if (final.decision != -1 && final.decision != APN_ACTION_ALLOW)
			break;
	}
	DEBUG(DBG_SANDBOX, " pe_decide_sandbox: decision = %d, prio = %d",
	    final.decision, i);
	reply = calloc(1, sizeof(struct anoubisd_reply));
	if (!reply) {
		log_warn(" pe_decide_sandbox: cannot allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}
	/*
	 * If we still do not have a final verdict we do not have a log
	 * level or a rule to report either.
	 */
	if (final.decision == -1) {
		final.rule_id = 0;
		final.prio = 0;
		final.decision = APN_ACTION_ALLOW;
		final.log = APN_LOG_NONE;
	} else {
		/*
		 * We had at least a partial match. Report the
		 * rule IDs with the highest log level that we have.
		 */
		final.log = -1;
		final.rule_id = 0;
		final.prio = 0;
		for (j=0; j<3; ++j) {
			if (res[j].decision != final.decision)
				continue;
			if (res[j].log > final.log) {
				final.log = res[j].log;
				final.rule_id = res[j].rule_id;
				final.prio = res[j].prio;
			}
		}
	}

	/* Logging */
	if (final.log == -1)
		final.log = APN_LOG_NONE;
	if (final.log != APN_LOG_NONE) {
		context = pe_context_dump(hdr, proc, final.prio);
		dump = pe_sb_dumpevent(sbevent);
	}

	/* Logging */
	switch (final.log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		hdr->msg_source = ANOUBIS_SOURCE_SANDBOX;
		log_info("token %u: SANDBOX prio %d rule %d %s %s (%s)",
		    hdr->msg_token, final.prio, final.rule_id,
		    verdict[final.decision], dump, context);
		send_lognotify(hdr, final.decision, final.log, final.rule_id,
		    final.prio, ANOUBIS_SFS_NONE);
		break;
	case APN_LOG_ALERT:
		hdr->msg_source = ANOUBIS_SOURCE_SANDBOX;
		log_warnx("token %u: SANDBOX prio %d rule %d %s %s (%s)",
		    hdr->msg_token, final.prio, final.rule_id,
		    verdict[final.decision], dump, context);
		send_lognotify(hdr, final.decision, final.log, final.rule_id,
		    final.prio, ANOUBIS_SFS_NONE);
		break;
	default:
		log_warnx(" pe_decide_sandbox: unknown log type %d", final.log);
	}

	if (dump)
		free(dump);
	if (context)
		free(context);

	reply->ask = 0;
	reply->log = final.log;
	reply->rule_id = final.rule_id;
	reply->prio = final.prio;
	reply->timeout = (time_t)0;
	reply->sfsmatch = ANOUBIS_SFS_NONE;
	if (final.decision == APN_ACTION_DENY)
		reply->reply = EPERM;
	else
		reply->reply = 0;
	if (final.decision == APN_ACTION_ASK) {
		reply->ask = 1;
		reply->timeout = 300;
		reply->pident = pe_proc_ident(proc);
		reply->ctxident = pe_context_get_ident(
		    pe_proc_get_context(proc, reply->prio));
	}
	DEBUG(DBG_SANDBOX, "<pe_decide_sandbox");
	return reply;
}

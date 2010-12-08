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
#include <bsdcompat.h>
#include <linux/anoubis_sfs.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/anoubis_sfs.h>
#include <dev/anoubis.h>
#endif

#include <sys/queue.h>

#include <stddef.h>

#include <anoubis_protocol.h>

#include "anoubisd.h"
#include "sfs.h"
#include "cert.h"
#include "pe.h"

/**
 * Result of a sandbox rule evaluation. Use internally in pe_sandbox.c
 */
struct result {
	int log;	/** The log level */
	int rule_id;	/** The rule id that matched */
	int prio;	/** The priority of the rule that matched */
	int decision;	/** The decision */
};

/**
 * Evaluate a file event according to the list of sandbox rules.
 * The result of the evaluation is returned in the result structure.
 * Only one access type (read, write or execute) is evaluated at a time.
 *
 * @param rulelist The list of rules to evalute.
 * @param sbevent The sandbox event.
 * @param res The result of the evaluation is returned in this structure.
 *     As long as the verdict does not change, the log level is only
 *     increased. If a previously stored log level exists, it is retained.
 *     This is important for log allow rules, i.e. the user cannot suppress
 *     logging from an admin log allow rule by specifying an allow rule
 *     without log.
 * @param atype The access type.
 * @param prio The priority.
 */
static void
pe_sb_evaluate(struct apnarr_array rulelist, struct pe_file_event *sbevent,
    struct result *res, unsigned int atype, int prio)
{
	struct apn_rule		*match = NULL;
	time_t			 now;
	size_t			 i;

	if ((sbevent->amask & atype) == 0) {
		/* Do not touch rule_id, prio or log level. */
		res->decision = APN_ACTION_ALLOW;
		return;
	}
	DEBUG(DBG_SANDBOX, "pe_sandbox_evaluate: path %s, atype=%d",
	    sbevent->path, atype);
	if (time(&now) == (time_t)-1) {
		log_warn("pe_sb_evaluate: Cannot get current time");
		goto err;
	}
	for (i=0; i<apnarr_size(rulelist); ++i) {
		struct apn_rule		*sbrule;
		char			*prefix;
		int			 cstype;
		struct abuf_buffer	 csum = ABUF_EMPTY;
		int			 ret;

		sbrule = apnarr_access(rulelist, i);
		if (sbrule->apn_type == APN_DEFAULT) {
			if (!match)
				match = sbrule;
			continue;
		}
		if (sbrule->apn_type != APN_SB_ACCESS) {
			log_warnx("Invalid rule type %d on sandbox queue",
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
		 * in event. Special case during upgrade.
		 */
		if (abuf_length(sbevent->csum) != ANOUBIS_CS_LEN
		    && (sbevent->upgrade_flags & PE_UPGRADE_TOUCHED) == 0)
			continue;
		csum = ABUF_EMPTY;
		ret = 0;
		switch (cstype) {
		case APN_CS_UID_SELF:
			ret = sfshash_get_uid(sbevent->path,
			    sbevent->uid, &csum);
			break;
		case APN_CS_UID:
			ret = sfshash_get_uid(sbevent->path,
			    sbrule->rule.sbaccess.cs.value.uid, &csum);
			break;
		case APN_CS_KEY_SELF: {
			char	*keyid;
			keyid = cert_keyid_for_uid(sbevent->uid);
			if (!keyid)
				break;
			ret = sfshash_get_key(sbevent->path, keyid, &csum);
			free(keyid);
			break;
		}
		case APN_CS_KEY:
			ret = sfshash_get_key(sbevent->path,
			    sbrule->rule.sbaccess.cs.value.keyid, &csum);
			break;
		}
		if (ret != 0 && ret != -ENOENT)
			log_warnx("sfshash_get: Error %d", -ret);
		if (abuf_empty(csum))
			continue;
		/* Special upgrade handling. */
		if (sbevent->upgrade_flags & PE_UPGRADE_TOUCHED) {
			abuf_free(csum);
			if ((sbevent->upgrade_flags & PE_UPGRADE_WRITEOK)
			    || abuf_length(sbevent->csum) == ANOUBIS_CS_LEN) {
				match = sbrule;
				goto have_match;
			}
			continue;
		}
		if (!abuf_equal(csum, sbevent->csum)) {
			abuf_free(csum);
			continue;
		}
		abuf_free(csum);
		match = sbrule;
		goto have_match;
	}
	if (!match) {
		DEBUG(DBG_SANDBOX, "pe_sandbox_evaluate: no match");
		return;
	}
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

/**
 * Return a list of candidate sandbox rules of the given  process that
 * might match the given path. The prefix hash is used to find candidate
 * rules. It is not guaranteed that all candidate rules in fact match.
 * This must be verified by the caller.
 *
 * @param proc The process
 * @param uid The uid of the process.
 * @param prio The rule priority.
 * @param path The path to find candidates for.
 * @param rulelist The rule list is returned here.
 * @return Zero in case of success, a negative error code in case of
 *     an error.
 */
int
pe_sb_getrules(struct pe_proc *proc, uid_t uid, int prio, const char *path,
    struct apnarr_array *rulelist)
{
	struct apn_rule	*sbrules;
	int		 error;
	int		 ispg = (pe_proc_get_playgroundid(proc) != 0);

	/*
	 * If we do not have a process, find the default rules
	 * for the given UID (if any) and try to apply these.
	 */
	if (proc && pe_proc_get_uid(proc) == uid) {
		sbrules = pe_context_get_sbrule(
		    pe_proc_get_context(proc, prio));
		DEBUG(DBG_SANDBOX, " pe_sb_getrules: context rules "
		    "prio %d rules %p", prio, sbrules);

	} else {
		struct apn_ruleset	*rs;
		rs = pe_user_get_ruleset(uid, prio, NULL);
		if (rs) {
			TAILQ_FOREACH(sbrules, &rs->sb_queue, entry) {
				if (!ispg && (sbrules->flags & APN_RULE_PGONLY))
					continue;
				if (sbrules->app == NULL)
					break;
			}
		} else {
			sbrules = NULL;
		}
		DEBUG(DBG_SANDBOX, " pe_sb_getrules: default rules "
		    "prio %d rules %p", prio, sbrules);
	}

	(*rulelist) = apnarr_EMPTY;
	/*
	 * Give the next priority a chance if we do not have
	 * any policy. This is default allow.
	 */
	if (!sbrules)
		return (0);

	if (!sbrules->userdata) {
		error = pe_build_prefixhash(sbrules);
		if (error < 0)
			return error;
	}
	return pe_prefixhash_getrules(sbrules->userdata, path, rulelist);
}

/**
 * Return a string version of a sandbox event.
 *
 * @param sbevent The sandbox event.
 * @return A readable representation of the event. The string is
 *     allocated dynamically and must be freed by the caller.
 */
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

/**
 * Decide a sandbox event. We use a default allow policy for the cases
 * where no policy is given. This can be changed by writing an explicit
 * Policy with a default deny rule.
 *
 * Merging of the different replies is tricky. There is a total of up
 * to six different replies. One for each priority and one for each
 * access mode. We report highest log level of those replies that have
 * the same verdict as the final result.
 *
 * @param proc The process that triggered the event.
 * @param sbevent The event itself.
 * @return An anoubis reply structure. The structure must be freed
 *     by the caller.
 */
struct anoubisd_reply *
pe_decide_sandbox(struct pe_proc *proc, struct pe_file_event *sbevent)
{
	struct result		 res[3];
	struct result		 final;
	struct anoubisd_reply	*reply;
	char			*dump = NULL, *context = NULL;
	int			 i, j;
	static const char	*verdict[3] = { "allowed", "denied", "asked" };

	if (!sbevent)
		return NULL;

	DEBUG(DBG_SANDBOX, ">pe_decide_sandbox: %s", sbevent->path);
	for (i=0; i<3; ++i) {
		res[i].rule_id = 0;
		res[i].prio = 0;
		res[i].decision = -1;
		res[i].log = APN_LOG_NONE;
	}
	final.decision = -1;
	for (i=0; i<PE_PRIO_MAX; ++i) {
		struct apnarr_array	rulelist;
		int			error;

		error = pe_sb_getrules(proc, sbevent->uid, i, sbevent->path,
		    &rulelist);
		if (error < 0) {
			final.decision = APN_ACTION_DENY;
			break;
		}
		if (apnarr_size(rulelist) == 0)
			continue;

		/*
		 * The three results may be pre-initialized from a higher
		 * priority. However, the only possibilities are
		 * uniniatilized (-1) or APN_ACTION_ALLOW.
		 */
		pe_sb_evaluate(rulelist, sbevent, &res[0], APN_SBA_READ, i);
		pe_sb_evaluate(rulelist, sbevent, &res[1], APN_SBA_WRITE, i);
		pe_sb_evaluate(rulelist, sbevent, &res[2], APN_SBA_EXEC, i);
		apnarr_free(rulelist);

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
		master_terminate();
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
		final.log = APN_LOG_NONE;
		final.rule_id = 0;
		final.prio = 0;
		for (j=0; j<3; ++j) {
			/*
			 * Always use the highest log level. It need not
			 * match with the rule ID.
			 */
			if (res[j].log > final.log)
				final.log = res[j].log;
			/*
			 * Do not report the rule ID and prio if the partial
			 * verdict is not equal to the final verdict.
			 */
			if (res[j].decision != final.decision)
				continue;
			final.rule_id = res[j].rule_id;
			final.prio = res[j].prio;
		}
	}

	/* Logging */
	if (final.log != APN_LOG_NONE) {
		context = pe_context_dump(sbevent->rawhdr, proc, final.prio);
		dump = pe_sb_dumpevent(sbevent);
	}

	/* Logging */
	switch (final.log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		sbevent->rawhdr->msg_source = ANOUBIS_SOURCE_SANDBOX;
		log_info("token %u: SANDBOX prio %d rule %d %s %s (%s)",
		    sbevent->rawhdr->msg_token, final.prio, final.rule_id,
		    verdict[final.decision], dump, context);
		send_lognotify(proc, sbevent->rawhdr, final.decision,
		    final.log, final.rule_id, final.prio, ANOUBIS_SFS_NONE);
		break;
	case APN_LOG_ALERT:
		sbevent->rawhdr->msg_source = ANOUBIS_SOURCE_SANDBOX;
		log_warnx("token %u: SANDBOX prio %d rule %d %s %s (%s)",
		    sbevent->rawhdr->msg_token, final.prio, final.rule_id,
		    verdict[final.decision], dump, context);
		send_lognotify(proc, sbevent->rawhdr, final.decision,
		    final.log, final.rule_id, final.prio, ANOUBIS_SFS_NONE);
		break;
	default:
		log_warnx(" pe_decide_sandbox: unknown log type %d", final.log);
	}

	if (dump)
		free(dump);
	if (context)
		free(context);

	reply->ask = 0;
	reply->hold = 0;
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
	DEBUG(DBG_SANDBOX, "<pe_decide_sandbox: reply=%d ask=%d", reply->reply,
	    reply->ask);
	return reply;
}

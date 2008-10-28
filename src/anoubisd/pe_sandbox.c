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
#include "pe.h"

struct sandbox_event {
	anoubis_cookie_t	 cookie;
	char			*path;
	u_int8_t		 cs[ANOUBIS_CS_LEN];
	unsigned int		 amask;
	int			 cslen;
	int			 uid;
	void			*sigdata; /* XXX CEH: This needs more work */
};

struct result {
	int log;
	int rule_id;
	int decision;
};

static struct sandbox_event *
pe_sb_parse_sfsmsg(struct eventdev_hdr *hdr)
{
	struct sandbox_event	*ret = NULL;
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
	ret = malloc(sizeof(struct sandbox_event));
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
	ret->sigdata = NULL;
	return ret;
err:
	if (ret)
		free(ret);
	return NULL;
}

static void
pe_sb_evaluate(struct apn_rule *sbrules, struct sandbox_event *sbevent,
    struct result *res, unsigned int atype)
{
	struct apn_rule		*sbrule;
	struct apn_rule		*match = NULL;
	time_t			 now;

	if ((sbevent->amask & atype) == 0) {
		/* Do not touch rule_id or log level. */
		res->decision = POLICY_ALLOW;
		return;
	}
	if (sbrules->apn_type != APN_SB) {
		log_warnx("pe_sb_evaluate: Non sandbox Policy found");
		goto err;
	}
	if (time(&now) == (time_t)-1) {
		log_warn("pe_sb_evaluate: Cannot get current time");
		goto err;
	}
	TAILQ_FOREACH(sbrule, &sbrules->rule.chain, entry) {
		char	*prefix;
		int	 cstype;
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
			if (strncmp(sbevent->path, prefix, len) != 0)
				continue;
			if (sbevent->path[len] && sbevent->path[len] != '/')
				continue;
		}
		cstype = sbrule->rule.sbaccess.cstype;
		if (cstype != SBCS_NONE) {
			u_int8_t	*cs = NULL;
			/* XXX CEH: Check signature and csum matches. */
			if (cstype == SBCS_CSUM) {
				cs = sbrule->rule.sbaccess.cs.csum;
				if (!cs)
					goto err;
			}
			if (!cs)
				continue;
			if (bcmp(cs, sbevent->cs, ANOUBIS_CS_LEN) != 0)
				continue;
		}
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
	DEBUG(DBG_SANDBOX, " pe_sb_evaluate: decision = %d rule %d",
	    res->decision, res->rule_id);
	return;
err:
	res->decision = POLICY_DENY;
	res->rule_id = 0;
	res->log = APN_LOG_NONE;
}

static char *
pe_sb_dumpevent(struct sandbox_event *sbevent)
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
pe_decide_sandbox(struct pe_proc *proc, struct eventdev_hdr *hdr)
{
	struct sandbox_event	*sbevent = pe_sb_parse_sfsmsg(hdr);
	struct result		 res[3];
	struct result		 final;
	struct anoubisd_reply	*reply;
	char			*dump = NULL, *context = NULL;
	int			 i, j;
	static const char	*verdict[3] = { "allowed", "denied", "asked" };

	if (!sbevent)
		return NULL;

	DEBUG(DBG_SANDBOX, " pe_decide_sandbox");
	for (i=0; i<3; ++i) {
		res[i].rule_id = 0;
		res[i].decision = -1;
		res[i].log = APN_LOG_NONE;
	}
	final.decision = -1;
	for (i=0; i<PE_PRIO_MAX; ++i) {
		struct apn_rule		*sbrules;

		/*
		 * If we do not have a process, find the default rules
		 * for the given UID (if any) and try to apply these.
		 */
		if (proc) {
			sbrules = pe_context_get_sbrule(
			    pe_proc_get_context(proc, i));
			DEBUG(DBG_SANDBOX, " pe_decide_sandbox: context rules "
			    "prio %d rules %p", i, sbrules);

		} else {
			struct apn_ruleset	*rs;
			rs = pe_user_get_ruleset(sbevent->uid, i, NULL);
			if (rs) {
				TAILQ_FOREACH(sbrules, &rs->sb_queue, entry)
					if (sbrules->app == NULL)
						break;
			} else {
				sbrules = NULL;
			}
			DEBUG(DBG_SANDBOX, " pe_decide_sandbox: default rules "
			    "prio %d rules %p", i, sbrules);
		}
		/*
		 * Give the next priority a chance if we do not have
		 * any policy. This is default allow.
		 */
		if (!sbrules)
			continue;
		/*
		 * The three results may be pre-initilized from a higher
		 * priority. However, the only possibilities are
		 * uninitilized (-1) or POLICY_ALLOW.
		 */
		pe_sb_evaluate(sbrules, sbevent, &res[0], APN_SBA_READ);
		pe_sb_evaluate(sbrules, sbevent, &res[1], APN_SBA_WRITE);
		pe_sb_evaluate(sbrules, sbevent, &res[2], APN_SBA_EXEC);
		/*
		 * If any of the events results in DENY we are done here.
		 * If any of the events results in ASK we are done, too.
		 * However, a DENY result for another event at the same
		 * priority takes precedence.
		 */
		for (j=0; j<3; ++j) {
			if (res[j].decision == POLICY_DENY) {
				final.decision = POLICY_DENY;
				break;
			} else if (res[j].decision == POLICY_ASK) {
				final.decision = POLICY_ASK;
			} else if (final.decision == -1) {
				final.decision = POLICY_ALLOW;
			}
		}
		/* If we have a verdict and it is not ALLOW we are done. */
		if (final.decision != -1 && final.decision != POLICY_ALLOW)
			break;
	}
	DEBUG(DBG_SANDBOX, ">pe_decide_sandbox: decision = %d, prio = %d",
	    final.decision, i);
	reply = calloc(1, sizeof(struct anoubisd_reply));
	if (!reply) {
		log_warn("pe_decide_sb: cannot allocate memory");
		master_terminate(ENOMEM);
		return NULL;
	}
	/*
	 * If we still do not have a final verdict we do not have a log
	 * level or a rule to report either.
	 */
	if (final.decision == -1) {
		final.rule_id = 0;
		final.decision = POLICY_ALLOW;
		final.log = APN_LOG_NONE;
	} else {
		/*
		 * We had at least a partial match. Report the
		 * rule IDs with the highest log level that we have.
		 */
		final.log = -1;
		final.rule_id = 0;
		for (j=0; j<3; ++j) {
			if (res[j].decision != final.decision)
				continue;
			if (res[j].log > final.log) {
				final.log = res[j].log;
				final.rule_id = res[j].rule_id;
			}
		}
	}

	/* Logging */
	if (final.log == -1)
		final.log = APN_LOG_NONE;
	if (final.log != APN_LOG_NONE) {
		if (i < PE_PRIO_MAX)
			context = pe_context_dump(hdr, proc, i);
		dump = pe_sb_dumpevent(sbevent);
	}

	/* Logging */
	switch (final.log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		log_info("SANDBOX prio %d rule %d %s %s (%s)", i,
		    final.rule_id, verdict[final.decision], dump, context);
		send_lognotify(hdr, final.decision, final.log, final.rule_id);
		break;
	case APN_LOG_ALERT:
		log_warnx("SANDBOX prio %d rule %d %s %s (%s)", i,
		    final.rule_id, verdict[final.decision], dump, context);
		send_lognotify(hdr, final.decision, final.log, final.rule_id);
		break;
	default:
		log_warnx("pe_decide_sandbox: unknown log type %d", final.log);
	}

	if (dump)
		free(dump);
	if (context)
		free(context);
	free(sbevent);

	reply->ask = 0;
	reply->rule_id = final.rule_id;
	reply->timeout = (time_t)0;
	if (final.decision == POLICY_DENY)
		reply->reply = EPERM;
	else
		reply->reply = 0;
	if (final.decision == POLICY_ASK) {
		struct pe_proc_ident	*pident = pe_proc_ident(proc);
		reply->ask = 1;
		reply->timeout = 300;
		reply->csum = pident->csum;
		reply->path = pident->pathhint;
	}
	DEBUG(DBG_SANDBOX, "<pe_decide_sandbox");
	return reply;
}

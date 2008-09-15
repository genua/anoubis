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

#include <anoubis_protocol.h>

#include "anoubisd.h"
#include "sfs.h"
#include "pe.h"

static int		 pe_decide_sfscheck(struct apn_rule *, struct
			     sfs_open_message *, int *, u_int32_t *, time_t);
static int		 pe_decide_sfsdflt(struct apn_rule *, struct
			     sfs_open_message *, int *, u_int32_t *, time_t);
static char		*pe_dump_sfsmsg(struct sfs_open_message *, int);


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
	struct pe_proc		*proc = NULL;
	int			 do_disable = 0;

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

	proc = pe_proc_get(msg->common.task_cookie);
	if (proc && pe_proc_is_sfsdisable(proc, hdr->msg_uid)) {
		do_disable = 1;
		pe_proc_put(proc);
	}

	if (msg->flags & ANOUBIS_OPEN_FLAG_CSUM) {
		for (i = 0; i < PE_PRIO_MAX; i++) {
			struct apn_ruleset	*rs;
			struct apn_rule		*rule;

			if (do_disable && (i == PE_PRIO_USER1))
				continue;
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
		context = pe_context_dump(hdr,
		    pe_proc_get(msg->common.task_cookie), prio);
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

	if (decision == POLICY_DENY)
		reply->reply = EPERM;
	else
		reply->reply = 0;
	reply->ask = 0;		/* false */
	reply->rule_id = rule_id;
	reply->timeout = (time_t)0;
	reply->len = 0;

	return (reply);
}

static int
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

static int
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
static char *
pe_dump_sfsmsg(struct sfs_open_message *msg, int format __used)
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

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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef LINUX
#include <queue.h>
#include <bsdcompat.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/queue.h>
#include <dev/anoubis.h>
#endif

#include <apn.h>
#include "anoubisd.h"
#include "pe.h"

struct pe_context {
	int			 refcount;
	struct apn_rule		*rule;
	struct apn_context	*ctx;
	struct apn_ruleset	*ruleset;
};

static void			 pe_dump_dbgctx(const char *, struct pe_proc *);
static struct pe_context	*pe_context_search(struct apn_ruleset *,
				     struct pe_proc_ident *);
static int			 pe_context_decide(struct pe_proc *,
				     struct pe_proc_ident *);
static struct pe_context	*pe_context_alloc(struct apn_rule *,
				     struct apn_ruleset *);

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
pe_context_update(struct pe_proc *pproc, struct pe_context **newctx, int prio,
    struct pe_policy_db *pdb)
{
	struct apn_ruleset	*newrules;
	struct pe_context	*context;

	if (pproc == NULL) {
		log_warnx("pe_update_ctx: empty process");
		return (-1);
	}
	if (newctx == NULL) {
		log_warnx("pe_context_update: invalid new context pointer");
		return (-1);
	}
	if (prio < 0 || prio >= PE_PRIO_MAX) {
		log_warnx("pe_context_update: illegal priority %d", prio);
		return (-1);
	}

	/*
	 * Try to get policies for our uid from the new pdb.  If the
	 * new pdb does not provide policies for our uid, try to get the
	 * default policies.
	 */

	DEBUG(DBG_TRACE, ">pe_context_update");
	context = NULL;
	newrules = pe_user_get_ruleset(pe_proc_get_uid(pproc), prio, pdb);
	DEBUG(DBG_TRACE, " pe_context_update: newrules = %x", newrules);
	if (newrules) {
		u_int8_t		*csum = NULL;
		char			*pathhint = NULL;
		struct apn_rule		*oldrule;
		struct apn_app		*oldapp;
		struct pe_context	*ctx;
		struct pe_proc_ident	tmpident;

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
		/*
		 * XXX CEH: This tmpident will go away once we have
		 * XXX CEH: proper idents in the old context.
		 */
		tmpident.csum = csum;
		tmpident.pathhint = pathhint;
		context = pe_context_search(newrules, &tmpident);
	}
out:

	DEBUG(DBG_PE_POLICY, "pe_context_update: context %p rule %p ctx %p",
	    context, context ? context->rule : NULL, context ? context->ctx :
	    NULL);

	*newctx = context;

	return (0);
}

/*
 * Decode a context in a printable string.  This strings is allocated
 * and needs to be freed by the caller.
 */
char *
pe_context_dump(struct eventdev_hdr *hdr, struct pe_proc *proc, int prio)
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

/*
 * Inherit the parent process context. This might not necessarily be
 * the direct parent process, but some grand parent.  This happens on
 * fork(2).  Moreover, we set a reference to that particular parent.
 *
 * If our parent was never tracked, we get a new context.
 */
void
pe_context_inherit(struct pe_proc *proc, struct pe_proc *parent)
{
	int		 i;

	if (proc == NULL) {
		log_warnx("pe_context_inherit: empty process");
		return;
	}

	DEBUG(DBG_PE_CTX, "pe_context_inherit: parent %p 0x%08llx", parent,
	    pe_proc_task_cookie(parent));

	if (parent && !pe_proc_valid_context(parent)) {
		DEBUG(DBG_PE_CTX, "pe_context_inherit: parent %p 0x%08llx "
		    "has no context", parent, pe_proc_task_cookie(parent));
	}

	if (parent && pe_proc_valid_context(parent)) {
		for (i = 0; i < PE_PRIO_MAX; i++) {
			struct pe_context *ctx = pe_proc_get_context(parent, i);
			pe_proc_set_context(proc, i, ctx);
		}
		pe_proc_set_parent(proc, parent);

		pe_dump_dbgctx("pe_context_inherit", proc);
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
		pe_context_set(proc, -1, NULL);
	}
}

/*
 * Set our context.  If we never inherited one, search for an apropriate
 * application rule and use that one as our context from now on.
 */
void
pe_context_set(struct pe_proc *proc, uid_t uid, struct pe_proc_ident *pident)
{
	int			 i;
	struct pe_context	*tmpctx;

	/* We can handle pident being NULL. */
	if (proc == NULL) {
		log_warnx("pe_context_set: empty process");
		return;
	}

	/*
	 * If we have inherited a context, check if it has context rule
	 * that allows us to switch context.  If no, we keep the context and
	 * just return.  Otherwise, we continue and search our new context.
	 */
	if (pe_proc_valid_context(proc)) {
		tmpctx = pe_proc_get_context(proc, 0);
		DEBUG(DBG_PE_CTX, "pe_context_set: proc %p 0x%08llx "
		    "has context %p rule %p ctx %p",
		    proc, pe_proc_task_cookie(proc),
		    tmpctx, tmpctx ? tmpctx->rule : NULL,
		    tmpctx ? tmpctx->ctx : NULL);

		if (pe_context_decide(proc, pident) != 1) {
			DEBUG(DBG_PE_CTX, "pe_context_set: keeping context");
			return;
		}
		DEBUG(DBG_PE_CTX, "pe_context_set: switching context");
	}

	/*
	 * If we are not tracked (which actually should not happen), or we
	 * are allowed to switch context we use the first matching
	 * application rule as new context.
	 * Note that pe_context_search will get a reference to the new context
	 * and pe_proc_set_context will get another one. We need to drop one
	 * of them.
	 */
	for (i = 0; i < PE_PRIO_MAX; i++) {
		struct apn_ruleset *ruleset;
		ruleset = pe_user_get_ruleset(uid, i, NULL);
		tmpctx = pe_context_search(ruleset, pident);
		pe_proc_set_context(proc, i, tmpctx);
		pe_context_put(tmpctx);
	}
	pe_proc_set_parent(proc, proc);
	if (uid != (uid_t)-1)
		pe_proc_set_uid(proc, uid);

	tmpctx = pe_proc_get_context(proc, 0);
	DEBUG(DBG_PE_CTX, "pe_context_set: proc %p 0x%08llx got context "
	    "%p rule %p ctx %p", proc, pe_proc_task_cookie(proc),
	    tmpctx, tmpctx ? tmpctx->rule : NULL, tmpctx ? tmpctx->ctx : NULL);
}

static struct pe_context *
pe_context_search(struct apn_ruleset *rs, struct pe_proc_ident *pident)
{
	struct pe_context	*context;
	struct apn_rule		*prule, *rule;
	struct apn_alfrule	*alfrule;
	struct apn_app		*hp;

	DEBUG(DBG_PE_CTX, "pe_context_search: ruleset %p csum 0x%08x...", rs,
	    (pident && pident->csum) ?
	    htonl(*(unsigned long *)pident->csum) : 0);

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
		if (!pident || pident->csum == NULL)
			continue;

		/*
		 * Otherwise walk chain of applications and check
		 * against hashvalue.
		 */
		for (hp = prule->app; hp && rule == NULL; hp = hp->next) {
			if (bcmp(hp->hashvalue, pident->csum,
			    sizeof(hp->hashvalue)) != 0)
				continue;
			rule = prule;
		}
		if (rule)
			break;
	}
	if (rule == NULL) {
		DEBUG(DBG_PE_CTX, "pe_context_search: no rule found");
		return (NULL);
	}
	context = pe_context_alloc(rule, rs);
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

	DEBUG(DBG_PE_CTX, "pe_context_search: context %p rule %p %s", context,
	    rule, context->ctx ? (context->ctx->application ?
	    context->ctx->application->name : "any") : "<none>");

	return (context);
}

/*
 * NOTE: rs must not be NULL.
 */
static struct pe_context *
pe_context_alloc(struct apn_rule *rule, struct apn_ruleset *rs)
{
	struct pe_context	*ctx;

	if (!rs) {
		log_warnx("NULL rulset in pe_context_alloc?");
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
pe_context_put(struct pe_context *ctx)
{
	if (!ctx || --(ctx->refcount))
		return;

	free(ctx);
}

void
pe_context_reference(struct pe_context *ctx)
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
static int
pe_context_decide(struct pe_proc *proc, struct pe_proc_ident *pident)
{
	struct apn_app	*hp;
	int		 cmp, decision, i;

	/*
	 * NOTE: Once we actually use the pathhint in policiy decision
	 * NOTE: this shortcut will no longer be valid.
	 */
	if (pident == NULL || pident->csum == NULL)
		return (0);

	/*
	 * Iterate over all priorities, starting with the highest.
	 * If a priority does not provide a context, we just continue.
	 *
	 * If a priority provides a context, we have to evaluate that one.
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
			cmp = bcmp(hp->hashvalue, pident->csum,
			    sizeof(hp->hashvalue));
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
			    "pe_context_decide: found \"%s\" csm 0x%08x... for "
			    "priority %d", (hp && hp->name) ? hp->name : "any"
			    , (hp && hp->hashvalue) ?
			    htonl(*(unsigned long *)hp->hashvalue) : 0, i);
		} else
			decision = 0;
	}

	return (decision);
}

static void
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

int
pe_context_uses_rs(struct pe_context *ctx, struct apn_ruleset *rs)
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

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

/*
 * CONTEXT CHANGE LOGIC
 *
 * Each process has two completely separate contexts:
 * - the Administrator context at priority 0 (PE_PRIO_ADMIN)   and
 * - the User context at priority 1 (PE_PRIO_USER).
 * Additionally each process known the pathname and checksum of the
 * file that it currently executes.
 *
 * Each context contains the following information:
 * - The ruleset that this context refers to.
 * - ALF rules from that ruleset.
 * - Context switching rules from that ruleset.
 * - The path and checksum that was used to search for the current
 *   ALF and context switching rules in the ruleset.
 *
 * Let us assume for the moment that all processes are properly tracked
 * (i.e. the Anoubis daemon know about them) and have ALF and context
 * switching rules assigned.
 *
 * There are three interesting operations. Each of these is applied to
 * the admin and the user context separately:
 * - FORK: In case of a fork the child process simply inherits the
 *         context of its parents. Additionally the process itself
 *         inherits the the pathname and checksum of the running binary
 *         from its parent.
 *         SPECIAL CASES:
 *         - The parent process is not yet tracked. (A)
 *         - The parent process is tracked but has no context. (B)
 * - EXEC: In case of an exec system call the process's pathname and checksum
 *         are always set to the values that correspond to the new binary.
 *         Additionally context switch might happen:
 *         If the current context switching rules do not permit a context
 *         switch to the new binary the old context including its rules
 *         is retained. Otherwise a new context is created. This context
 *         contains the current processes path and checksum and additionally
 *         the rules from the currently active ruleset that corresponds to
 *         them.
 *         SPECIAL CASES:
 *         - The process that execs is not yet tracked. (A)
 *         - The process is tracked but has a NULL context. (B)
 *         - The process has a context but this context contains no
 *           context switching rules. (D)
 * - RELOAD: In case of a rule reload, the path and checksum in the each
 *         context of each tracked process is used to search for a new
 *         context in the new rules.  This new context is used for the
 *         process from then on.
 *         SPECIAL CASES:
 *         - The process has no context. (A)
 *         - The process has a context but no new context is found in the
 *           new rules. (C)
 *
 * Dealing with special cases:
 * (A) Untracked processes: An untracked process must remain untracked until
 *     a proper pathname and checksum for the process is available. This only
 *     happens during exec. I.e.: An untracked process that forks generates
 *     an untracked child, an untracked process that execs will be tracked
 *     from then on.
 *     NOTE: Untracked processes originate from processes that were already
 *     running at the time the Anoubis Daemon was started.
 * (B) Processes without a context: If a process has no context, it never
 *     had one before. In case of a fork or a rule reload, this is not changed.
 *     In case of an exec there are two possibilities:
 *     - If there is no ruleset for that is applicable to the process and the
 *       given priority. Nothing is changed.
 *     - If there is a ruleset a new context for the process is created based
 *       on the checksum and pathname of the current process. This context
 *       is created even if no matching context is found in this ruleset.
 *       In the latter case all further context switching is forbidden. The
 *       process can only change its rules in case of a rule reload.
 * (C) Rule reload and no context is found: This case is handled in the
 *     same way as described above for exec: A new context is created
 *     anyway but that context contains no context switch rules and no
 *     ALF rules. The pathname and checksum are copied from the existing
 *     context.
 * (D) Finally if a process with a valid context execs and that context
 *     does not contain any context switching rules, context switching
 *     is not allowed. This has already been mentioned in case (B) above
 *     and applies also to case (C).
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
	struct pe_proc_ident	 ident;
};

static void			 pe_dump_dbgctx(const char *, struct pe_proc *);
static struct pe_context	*pe_context_search(struct apn_ruleset *,
				     struct pe_proc_ident *);
static int			 pe_context_decide(struct pe_proc *, int,
				     struct pe_proc_ident *);
static struct pe_context	*pe_context_alloc(struct apn_rule *,
				     struct apn_ruleset *,
				     struct pe_proc_ident *);
static void			 pe_context_switch(struct pe_proc *, int,
				     struct pe_proc_ident *, uid_t);
static void			 pe_context_norules(struct pe_proc *, int);


/*
 * Set the process context if no ruleset is present to search for
 * the context.
 * - If the process never had a context for the priority, leave it alone.
 * - Otherwise create a norules context with the pident from the old context
 *   but without actual rules.
 */
void
pe_context_norules(struct pe_proc *proc, int prio)
{
	struct pe_context *ctx = pe_proc_get_context(proc, prio);
	/* No old context => We're done. */
	if (!ctx)
		return;
	/* Old context already is a norules context => We're done. */
	if (ctx->ruleset == NULL && ctx->rule == NULL && ctx->ctx == NULL)
		return;
	ctx = pe_context_alloc(NULL, NULL, &ctx->ident);
	if (!ctx) {
		master_terminate(ENOMEM);
		return;
	}
	pe_proc_set_context(proc, prio, ctx);
}

/*
 * Update a context at a rules reload.
 * - If there are no rules a norules context is used.
 * - If there is an old context we search a new one based on the search
 *   parameters of that context.
 * - If there are rules but no old context we search for rules based on
 *   the processes identifier.
 * pdb contains the database that should be used to lookup rulesets. If pdb
 * is NULL the currently active policy database is used.
 */
void
pe_context_refresh(struct pe_proc *proc, int prio, struct pe_policy_db *pdb)
{
	struct apn_ruleset	*newrules;
	struct pe_context	*context, *oldctx;

	if (proc == NULL) {
		log_warnx("pe_update_ctx: empty process");
		return;
	}
	if (prio < 0 || prio >= PE_PRIO_MAX) {
		log_warnx("pe_context_refresh: illegal priority %d", prio);
		return;
	}

	DEBUG(DBG_TRACE, ">pe_context_refresh");
	context = NULL;
	newrules = pe_user_get_ruleset(pe_proc_get_uid(proc), prio, pdb);
	DEBUG(DBG_TRACE, " pe_context_refresh: newrules = %x", newrules);
	if (!newrules) {
		pe_context_norules(proc, prio);
		return;
	}
	oldctx = pe_proc_get_context(proc, prio);
	if (oldctx) {
		context = pe_context_search(newrules, &oldctx->ident);
	} else {
		context = pe_context_search(newrules, pe_proc_ident(proc));
	}
	DEBUG(DBG_PE_POLICY, "pe_context_refresh: context %p rule %p ctx %p",
	    context, context ? context->rule : NULL, context ? context->ctx :
	    NULL);
	pe_proc_set_context(proc, prio, context);
	pe_context_put(context);
	return;
}

/*
 * Decode a context in a printable string.  This string is allocated
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
 * Inherit the parent process context. This happens on fork(2).
 * - If our parent was never tracked, we simulate an exec.
 * - If the parent has a non NULL context for a prio. Use it.
 * - Otherwise if there are no rules for that uid/prio, use a norules
 *   context.
 * - If there are rules for the  process, use them to find a new context
 *   based on the current process's proc->ident.
 */
void
pe_context_fork(struct pe_proc *proc, struct pe_proc *parent)
{
	int	i;

	if (proc == NULL) {
		log_warnx("pe_context_fork: empty process");
		return;
	}

	DEBUG(DBG_PE_CTX, "pe_context_fork: parent %p 0x%08llx", parent,
	    pe_proc_task_cookie(parent));
	for (i = 0; i < PE_PRIO_MAX; i++) {
		struct pe_context	*ctx = pe_proc_get_context(parent, i);
		if (ctx) {
			pe_proc_set_context(proc, i, ctx);
			continue;
		}
		DEBUG(DBG_PE_CTX, "pe_context_fork: parent %p "
		    "0x%08llx has no context at prio %d",
		    parent, pe_proc_task_cookie(parent),  i);
		pe_context_switch(proc, i, pe_proc_ident(proc),
		    pe_proc_get_uid(proc));
	}
	pe_dump_dbgctx("pe_context_fork", proc);
}

/*
 * Select a new context for priority prio of the process proc based on the
 * pident and uid given as parameters.
 */
static void
pe_context_switch(struct pe_proc *proc, int prio,
    struct pe_proc_ident *pident, uid_t uid)
{
	struct pe_context	*tmpctx;
	struct apn_ruleset	*ruleset;

	ruleset = pe_user_get_ruleset(uid, prio, NULL);
	if (!ruleset) {
		pe_context_norules(proc, prio);
		return;
	}
	tmpctx = pe_context_search(ruleset, pident);
	pe_proc_set_context(proc, prio, tmpctx);
	DEBUG(DBG_PE_CTX, "pe_context_switch: proc %p 0x%08llx prio %d "
	    "got context %p rule %p ctx %p", proc, 
	    pe_proc_task_cookie(proc), prio,
	    tmpctx, tmpctx ? tmpctx->rule : NULL,
	    tmpctx ? tmpctx->ctx : NULL);
	/*
	 * pe_context_search got a reference to the new context and
	 * pe_proc_set_context got another one. Drop one of them.
	 */
	pe_context_put(tmpctx);
}

/*
 * Change a context at exec time.
 * - If pe_context_decide forbids changing the context. Do not change it.
 * - Otherwise the context at the prio either allowed the context switch
 *   or it is still invalid (NULL) which implicitly allows us to switch
 *   contexts. In that case search a new context using pe_context_switch.
 *   This will properly handle the case where the old context is NULL.
 */
void
pe_context_exec(struct pe_proc *proc, uid_t uid, struct pe_proc_ident *pident)
{
	int			 i;

	/* We can handle pident being NULL. */
	if (proc == NULL) {
		log_warnx("pe_context_exec: empty process");
		return;
	}

	/*
	 * Switch context if we are allowed to do so by our context rules
	 * or if we are not tracked. Actually, the latter should not happen.
	 */
	for (i = 0; i < PE_PRIO_MAX; i++) {
		/* Do not switch if are rules forbid it. */
		if (pe_context_decide(proc, i, pident) == 0)
			continue;
		DEBUG(DBG_PE_CTX,
		    "pe_context_exec: prio %d switching context", i);
		pe_context_switch(proc, i, pident, uid);
	}
	if (uid != (uid_t)-1)
		pe_proc_set_uid(proc, uid);
}

/*
 * Search for a context in the given ruleset. This function will only
 * return NULL if the ruleset is also NULL. Otherwise it will return
 * a context from the ruleset. If no such context can be found it will
 * return a non-NULL context which has no real rules but formally belongs
 * to the ruleset.
 */
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
	if (rule == NULL)
		DEBUG(DBG_PE_CTX, "pe_context_search: no rule found");
	context = pe_context_alloc(rule, rs, pident);
	if (!context) {
		master_terminate(ENOMEM);
		return NULL;
	}

	/*
	 * If we have a rule chain, search for a context rule.  It is
	 * ok, if we do not find a context rule.  This will mean, that
	 * we have to stay in the current context on exec(2).
	 */
	if (rule) {
		for (alfrule = rule->rule.alf; alfrule;
		    alfrule = alfrule->next) {
			if (alfrule->type != APN_ALF_CTX)
				continue;
			context->ctx = &alfrule->rule.apncontext;
			break;
		}
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
pe_context_alloc(struct apn_rule *rule, struct apn_ruleset *rs,
    struct pe_proc_ident *pident)
{
	struct pe_context	*ctx;

	if ((ctx = calloc(1, sizeof(struct pe_context))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);	/* XXX HSH */
		return NULL;
	}
	ctx->rule = rule;
	ctx->ruleset = rs;
	ctx->refcount = 1;
	ctx->ident.csum = NULL;
	ctx->ident.pathhint = NULL;
	if (pident)
		pe_proc_ident_set(&ctx->ident, pident->csum, pident->pathhint);

	return (ctx);
}

void
pe_context_put(struct pe_context *ctx)
{
	if (!ctx || --(ctx->refcount))
		return;
	pe_proc_ident_put(&ctx->ident);
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
 * Returns 1 if switching is ok, 0 if not.
 *
 * NOTE: bcmp returns 0 on match, otherwise non-zero.  Do not confuse this...
 */
static int
pe_context_decide(struct pe_proc *proc, int prio, struct pe_proc_ident *pident)
{
	struct apn_app		*hp;
	struct pe_context	*ctx;

	/*
	 * NOTE: Once we actually use the pathhint in policiy decision
	 * NOTE: this shortcut will no longer be valid.
	 */
	if (pident == NULL || pident->csum == NULL)
		return (0);

	ctx = pe_proc_get_context(proc, prio);
	/* No context: Allow a context switch. */
	if (ctx == NULL)
		return 1;
	/* Context without rule means, not switching. */
	if (ctx->ctx == NULL)
		return 0;
	/*
	 * If the rule says "context new any", application will be
	 * empty.  In that case, we must allow the switch.
	 */
	hp = ctx->ctx->application;
	if (hp) {
		while (hp) {
			if (bcmp(hp->hashvalue, pident->csum,
			    sizeof(hp->hashvalue)) == 0)
				break;
			hp = hp->next;
		}
		if (hp == NULL)
			return 0;
	}
	/* If we reach this point context switching is allowed. */
	DEBUG(DBG_PE_CTX,
	    "pe_context_decide: found \"%s\" csm 0x%08x... for "
	    "priority %d", (hp && hp->name) ? hp->name : "any",
	    (hp && hp->hashvalue) ?
	    htonl(*(unsigned long *)hp->hashvalue) : 0, prio);
	return 1;
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
	return ctx && rs && (ctx->ruleset == rs || ctx->ruleset == NULL);
}

struct apn_rule *
pe_context_get_rule(struct pe_context *ctx)
{
	if (!ctx)
		return NULL;
	return ctx->rule;
}

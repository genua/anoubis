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
 * Additionally each process knowns the pathname and checksum of the
 * file that it currently executes.
 *
 * Each context contains the following information:
 * - The ruleset that this context refers to.
 * - ALF rules from that ruleset.
 * - SANDBOX rules from that ruleset.
 * - Context switching rules from that ruleset.
 * - The path and checksum that was used to search for the current
 *   ALF and context switching rules in the ruleset.
 *
 * Let us assume for the moment that all processes are properly tracked
 * (i.e. the Anoubis daemon know about them) and have ALF and context
 * switching rules assigned.
 *
 * There are four interesting operations. Each of these is applied to
 * the admin and the user context separately:
 * - FORK: In case of a fork the child process simply inherits the
 *         context of its parents. Additionally, the process itself
 *         inherits the the pathname and checksum of the running binary
 *         from its parent.
 *         SPECIAL CASES:
 *         - The parent process is not yet tracked. (A)
 *         - The parent process is tracked but has no context. (B)
 * - EXEC: In case of an exec system call the process's pathname and checksum
 *         are always set to the values that correspond to the new binary.
 *         Additionally, a context switch might happen:
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
 * - OPEN: In case of an open system call the process might switch
 *         its context depending on context rules in its current policy.
 *	   SPECIAL CASES:
 *         - The process has no context. (A) (B)
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
 *     had one before. In case of a fork, rule reload or open, this is
 *     not changed. In case of an exec there are two possibilities:
 *     - If there is no ruleset that is applicable to the process and the
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

#include <stddef.h>

#include <apn.h>
#include "anoubisd.h"
#include "pe.h"
#include "sfs.h"
#include "cert.h"

struct pe_context {
	int			 refcount;
	struct apn_rule		*alfrule;
	struct apn_rule		*sbrule;
	struct apn_rule		*ctxrule;
	struct apn_ruleset	*ruleset;
	struct pe_proc_ident	 ident;
	anoubis_cookie_t	 conn_cookie;
};

static void			 pe_dump_dbgctx(const char *, struct pe_proc *);
static struct pe_context	*pe_context_search(struct apn_ruleset *,
				     struct pe_proc_ident *, uid_t uid);
static int			 pe_context_decide(struct pe_proc *, int, int,
				     struct pe_proc_ident *, uid_t uid);
static struct pe_context	*pe_context_alloc(struct apn_ruleset *,
				     struct pe_proc_ident *);
static void			 pe_context_switch(struct pe_proc *, int,
				     struct pe_proc_ident *, uid_t);
static void			 pe_context_norules(struct pe_proc *, int);
static void			 pe_savedctx_norules(struct pe_proc *, int);


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
	if (ctx->ruleset == NULL && ctx->alfrule == NULL
	    && ctx->sbrule == NULL && ctx->ctxrule == NULL)
		return;
	ctx = pe_context_alloc(NULL, &ctx->ident);
	if (!ctx) {
		master_terminate(ENOMEM);
		return;
	}
	pe_proc_set_context(proc, prio, ctx);
	pe_context_put(ctx);
}

/*
 * Set the saved process context if no ruleset is present to search for a
 * replacment saved context.
 * - If the process never saved a context for the priority, leave it alone.
 * - Otherwise create  norules context with the pident from the old context
 *   but without actual rules.
 */
void
pe_savedctx_norules(struct pe_proc *proc, int prio)
{
	struct pe_context *ctx = pe_proc_get_savedctx(proc, prio);

	/* No saved context => done */
	if (!ctx)
		return;
	/* Saved context has no rules => done */
	if (ctx->ruleset == NULL && ctx->alfrule == NULL
	    && ctx->sbrule == NULL && ctx->ctxrule == NULL)
		return;
	/* Create norules context. */
	ctx = pe_context_alloc(NULL, &ctx->ident);
	if (!ctx) {
		master_terminate(ENOMEM);
		return;
	}
	pe_proc_set_savedctx(proc, prio, ctx);
	pe_context_put(ctx);
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
	DEBUG(DBG_TRACE, " pe_context_refresh: newrules = %p", newrules);
	if (!newrules) {
		pe_context_norules(proc, prio);
		pe_savedctx_norules(proc, prio);
		return;
	}
	oldctx = pe_proc_get_context(proc, prio);
	if (oldctx) {
		context = pe_context_search(newrules, &oldctx->ident,
		    pe_proc_get_uid(proc));
	} else {
		context = pe_context_search(newrules, pe_proc_ident(proc),
		    pe_proc_get_uid(proc));
	}
	DEBUG(DBG_PE_POLICY, "pe_context_refresh: context %p alfrule %p "
	    "sbrule %p ctxrule %p", context, context ? context->alfrule : NULL,
	    context ? context->sbrule : NULL,
	    context ? context->ctxrule : NULL);
	pe_proc_set_context(proc, prio, context);
	pe_context_put(context);

	oldctx = pe_proc_get_savedctx(proc, prio);
	if (oldctx) {
		context = pe_context_search(newrules, &oldctx->ident,
		    pe_proc_get_uid(proc));
		pe_proc_set_savedctx(proc, prio, context);
		pe_context_put(context);
	}
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
	struct pe_proc_ident	*pident = NULL;
	long			 uidctx = -1;

	/* hdr must be non-NULL, proc might be NULL */
	if (hdr == NULL)
		return (NULL);

	csumctx = 0;
	progctx = "<none>";

	if (proc && 0 <= prio && prio <= PE_PRIO_MAX) {
		ctx = pe_proc_get_context(proc, prio);

		if (ctx) {
			if (ctx->ident.csum)
				csumctx = htonl(
				    *(unsigned long *)ctx->ident.csum);
			if (ctx->ident.pathhint)
				progctx = ctx->ident.pathhint;
		}
		pident = pe_proc_ident(proc);
		uidctx = pe_proc_get_uid(proc);
	}

	if (asprintf(&dump, "uid %hu pid %hu program %s checksum 0x%08x... "
	    "context uid %ld program %s checksum 0x%08lx...",
	    hdr->msg_uid, hdr->msg_pid,
	    (pident && pident->pathhint) ? pident->pathhint : "<none>",
	    (pident && pident->csum) ?
	    htonl(*(unsigned long *)pident->csum) : 0,
	    uidctx, progctx, csumctx) == -1) {
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
	    (unsigned long long)pe_proc_task_cookie(parent));
	for (i = 0; i < PE_PRIO_MAX; i++) {
		struct pe_context	*ctx = pe_proc_get_context(parent, i);
		if (ctx) {
			pe_proc_set_context(proc, i, ctx);
			continue;
		}
		DEBUG(DBG_PE_CTX, "pe_context_fork: parent %p "
		    "0x%08llx has no context at prio %d",
		    parent, (unsigned long long)pe_proc_task_cookie(parent),
		    i);
		pe_context_switch(proc, i, pe_proc_ident(proc),
		    pe_proc_get_uid(proc));
	}
	pe_dump_dbgctx("pe_context_fork", proc);
}

static int
pe_context_subject_match(const struct apn_app *app,
    const struct pe_proc_ident *pident, uid_t uid)
{
	const struct apn_subject	*subject;
	u_int8_t			*csptr = NULL;
	u_int8_t			 csbuf[ANOUBIS_CS_LEN];
	char				*keyid;
	int				 ret;
	
	if (!app)
		return 0;
	subject = &app->subject;
	if (uid == (uid_t)-1 && (subject->type == APN_CS_UID_SELF
	    || subject->type == APN_CS_KEY_SELF))
		return 0;
	switch(subject->type) {
	case APN_CS_NONE:
		return 1;
	case APN_CS_UID:
		uid = subject->value.uid;
		/* FALLTRHOUGH */
	case APN_CS_UID_SELF:
		if (uid == (uid_t)-1)
			return 0;
		if (!app->name)
			return 0;
		ret = sfshash_get_uid(app->name, uid, csbuf);
		if (ret != 0)
			return 0;
		csptr = csbuf;
		break;
	case APN_CS_KEY:
	case APN_CS_KEY_SELF:
		if (subject->type == APN_CS_KEY_SELF) {
			keyid = cert_keyid_for_uid(uid);
			if (!keyid)
				return 0;
		} else {
			keyid = subject->value.keyid;
		}
		ret = sfshash_get_key(app->name, keyid, csbuf);
		if (subject->type == APN_CS_KEY_SELF)
			free(keyid);
		if (ret != 0)
			return 0;
		csptr = csbuf;
		break;
	default:
		return 0;
	}
	if (!csptr)
		return 0;
	return (memcmp(csptr, pident->csum, ANOUBIS_CS_LEN) == 0);
}

static int
pe_context_app_match(const struct apn_app *app,
    const struct pe_proc_ident *pident, uid_t uid)
{
	if (!app || !app->name)
		return 1;
	/*
	 * Only compare the pathhint if no checksum is given. If a checksum
	 * is given we use app->name only to search for the checksum
	 */
	if (app->subject.type == APN_CS_NONE) {
		if (!pident->pathhint)
			return 0;
		if (strcmp(app->name, pident->pathhint) != 0)
			return 0;
	}
	return pe_context_subject_match(app, pident, uid);
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
	tmpctx = pe_context_search(ruleset, pident, uid);
	pe_proc_set_context(proc, prio, tmpctx);
	DEBUG(DBG_PE_CTX, "pe_context_switch: proc %p 0x%08llx prio %d "
	    "got context %p alfrule %p sbrule %p ctxrule %p", proc,
	    (unsigned long long)pe_proc_task_cookie(proc), prio, tmpctx,
	    tmpctx ? tmpctx->alfrule : NULL, tmpctx ? tmpctx->sbrule : NULL,
	    tmpctx ? tmpctx->ctxrule : NULL);
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
		if (pe_context_decide(proc, APN_CTX_NEW, i, pident, uid) == 0)
			continue;
		DEBUG(DBG_PE_CTX,
		    "pe_context_exec: prio %d switching context", i);
		pe_context_switch(proc, i, pident, uid);
	}
	if (uid != (uid_t)-1)
		pe_proc_set_uid(proc, uid);
}

/*
 * Change context on open(2):
 * - If pe_context_decide forbids changing the context, we don't.
 * - The logic is similar to pe_context_exec: Otherwise the context at the
 *   prio either allowed the context switch or it is still invalid (NULL)
 *   which implicitly allows us to switch contexts.  In that case search
 *   a new context using pe_context_switch.  Additionally the identity of the
 *   process is updated.
 */
void
pe_context_open(struct pe_proc *proc, struct eventdev_hdr *hdr)
{
	struct sfs_open_message	*sfsmsg;
	struct pe_proc_ident	 pident;
	int			 sfslen, pathlen, i;

	if (proc == NULL || hdr == NULL)
		return;

	sfsmsg = (struct sfs_open_message *)(hdr + 1);
	if (hdr->msg_size < sizeof(struct eventdev_hdr))
		return;
	sfslen = hdr->msg_size - sizeof(struct eventdev_hdr);
	if (sfslen < (int)sizeof(struct sfs_open_message))
		return;
	if (!(sfsmsg->flags & ANOUBIS_OPEN_FLAG_CSUM))
		return;
	if (sfsmsg->flags & ANOUBIS_OPEN_FLAG_PATHHINT) {
		pathlen = sfslen - offsetof(struct sfs_open_message, pathhint);
		for (i = pathlen - 1; i >= 0; --i) {
			if (sfsmsg->pathhint[i] == 0)
				break;
		}
		if (i < 0)
			return;
	} else {
		sfsmsg->pathhint[0] = 0;
	}

	pident.pathhint = sfsmsg->pathhint;
	pident.csum = sfsmsg->csum;

	for (i = 0; i < PE_PRIO_MAX; i++) {
		if (pe_context_decide(proc, APN_CTX_OPEN, i, &pident,
		    pe_proc_get_uid(proc)) == 0)
			continue;

		DEBUG(DBG_PE_CTX,
		    "pe_context_open: proc %p prio %d switching context to %s",
		    proc, i, pident.pathhint);

		pe_context_switch(proc, i, &pident, hdr->msg_uid);
	}
}

void
pe_context_borrow(struct pe_proc *proc, struct pe_proc *procp,
    anoubis_cookie_t cookie)
{
	struct pe_context	*ctx, *ctxp;
	int			 i;

	if (proc == NULL || procp == NULL)
		return;

	for (i = 0; i < PE_PRIO_MAX; i++) {
		ctx = pe_proc_get_context(proc, i);
		if (ctx == NULL)
			continue;
		if (pe_context_decide(procp, APN_CTX_BORROW, i, &ctx->ident,
		    pe_proc_get_uid(proc)) == 0)
			continue;
		ctxp = pe_proc_get_context(procp, i);
		if (ctx->ruleset != ctxp->ruleset)
			continue;

		DEBUG(DBG_PE_BORROW,
		    "pe_context_borrow: proc %p prio %d switching context to "
			"%p of proc %p", procp, i, ctx, proc);

		pe_proc_save_ctx(procp, i, cookie);
		pe_context_switch(procp, i, &ctx->ident,
		    pe_proc_get_uid(procp));
	}
}

void
pe_context_restore(struct pe_proc *proc, anoubis_cookie_t cookie)
{
	int	 i;

	if (proc == NULL)
		return;

	for (i = 0; i < PE_PRIO_MAX; i++)
		pe_proc_restore_ctx(proc, i, cookie);
}

static struct apn_rule *
pe_context_search_chain(struct apn_chain *chain, struct pe_proc_ident *pident,
    uid_t uid)
{
	struct apn_rule		*r;
	struct apn_app		*hp;
	TAILQ_FOREACH(r, chain, entry) {
		/*
		 * A rule without application will always match.  This is
		 * especially important, when no checksum is available,
		 * as we still have to walk the full tailq.  Thus we use
		 * "continue" below
		 */
		if (r->app == NULL)
			return r;
		if (!pident)
			continue;
		/*
		 * If we have a valid checksum walk chain of applications
		 * and check against hashvalue.
		 */
		for (hp = r->app; hp; hp = hp->next) {
			if (pe_context_app_match(hp, pident, uid))
				return r;
		}
	}
	return NULL;
}

/*
 * Search for a context in the given ruleset. This function will only
 * return NULL if the ruleset is also NULL. Otherwise it will return
 * a context from the ruleset. If no such context can be found it will
 * return a non-NULL context which has no real rules but formally belongs
 * to the ruleset.
 */
static struct pe_context *
pe_context_search(struct apn_ruleset *rs, struct pe_proc_ident *pident,
    uid_t uid)
{
	struct pe_context	*context;

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

	context = pe_context_alloc(rs, pident);
	if (!context) {
		master_terminate(ENOMEM);
		return NULL;
	}
	context->alfrule = pe_context_search_chain(&rs->alf_queue, pident, uid);
	context->sbrule = pe_context_search_chain(&rs->sb_queue, pident, uid);
	context->ctxrule = pe_context_search_chain(&rs->ctx_queue, pident, uid);

	DEBUG(DBG_PE_CTX, "pe_context_search: context %p alfrule %p sbrule %p "
	    "ctxrule %p", context, context->alfrule, context->sbrule,
	    context->ctxrule);

	return (context);
}

/*
 * NOTE: rs must not be NULL.
 */
static struct pe_context *
pe_context_alloc(struct apn_ruleset *rs, struct pe_proc_ident *pident)
{
	struct pe_context	*ctx;

	if ((ctx = calloc(1, sizeof(struct pe_context))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);
		return NULL;
	}
	ctx->alfrule = NULL;
	ctx->sbrule = NULL;
	ctx->ctxrule = NULL;
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
pe_context_decide(struct pe_proc *proc, int type, int prio,
    struct pe_proc_ident *pident, uid_t uid)
{
	struct apn_app		*hp = NULL;
	struct apn_rule		*rule;
	struct pe_context	*ctx;
	int			 ret;

	/*
	 * NOTE: Once we actually use the pathhint in policiy decision
	 * NOTE: this shortcut will no longer be valid.
	 */
	if (pident == NULL || pident->csum == NULL)
		return (0);

	ctx = pe_proc_get_context(proc, prio);
	/* No context: Allow a context switch. */
	if (ctx == NULL)
		return (type == APN_CTX_NEW);
	/* Context without rule means, not switching. */
	if (ctx->ctxrule == NULL)
		return 0;
	ret = 0;	/* deny by default */
	TAILQ_FOREACH(rule, &ctx->ctxrule->rule.chain, entry) {
		if (rule->apn_type != APN_CTX_RULE)
			continue;
		if (rule->rule.apncontext.type != type)
			continue;
		/*
		 * If the rule says "context new any", application will be
		 * empty.  In that case, we must allow the switch.
		 */
		hp = rule->rule.apncontext.application;
		if (!hp) {
			ret = 1;	/* allow */
			break;
		}
		while (hp) {
			if (pe_context_app_match(hp, pident, uid)) {
				ret = 1;
				break;
			}
			hp = hp->next;
		}
		if (ret)
			break;
	}
	if (!ret)
		return 0;
	/* If we reach this point context switching is allowed. */
	DEBUG(DBG_PE_CTX,
	    "pe_context_decide: found \"%s\" csum 0x%08x... for "
	    "type %d priority %d", (hp && hp->name) ? hp->name : "any",
	    (pident && pident->csum) ?
	    htonl(*(unsigned long *)pident->csum) : 0, type, prio);
	return 1;
}

static void
pe_dump_dbgctx(const char *prefix, struct pe_proc *proc)
{
	int	i;

	if (proc == NULL) {
		DEBUG(DBG_PE_CTX, "%s: prio %d rule (null) ctx (null)",
		    prefix, -1);
		return;
	}

	for (i = 0; i < PE_PRIO_MAX; i++) {
		struct pe_context *ctx = pe_proc_get_context(proc, i);
		DEBUG(DBG_PE_CTX, "%s: prio %d alfrule %p sbrule %p "
		    "ctxrule %p", prefix, i, ctx ? ctx->alfrule : NULL,
		    ctx ? ctx->sbrule : NULL, ctx ? ctx->ctxrule : NULL);
	}
}

int
pe_context_uses_rs(struct pe_context *ctx, struct apn_ruleset *rs)
{
	return ctx && rs && (ctx->ruleset == rs || ctx->ruleset == NULL);
}

struct apn_rule *
pe_context_get_alfrule(struct pe_context *ctx)
{
	if (!ctx)
		return NULL;
	return ctx->alfrule;
}

struct apn_rule *
pe_context_get_sbrule(struct pe_context *ctx)
{
	if (!ctx)
		return NULL;
	return ctx->sbrule;
}

struct pe_proc_ident *
pe_context_get_ident(struct pe_context *ctx)
{
	if (!ctx)
		return NULL;
	return &ctx->ident;
}

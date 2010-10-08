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

/**
 \file
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
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef LINUX
#include <bsdcompat.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include <sys/queue.h>

#include <stddef.h>

#include <apn.h>
#include "anoubisd.h"
#include "pe.h"
#include "sfs.h"
#include "cert.h"

/**
 * The context of a rule. Contexts can be shared and must thus be
 * reference counted.
 */
struct pe_context {
	/**
	 * The reference count of the context.
	 */
	int			 refcount;

	/**
	 * The alf rule block of the context.
	 */
	struct apn_rule		*alfrule;

	/**
	 * The sandbox rule block of the context.
	 */
	struct apn_rule		*sbrule;

	/**
	 * The context rule block of the rule.
	 */
	struct apn_rule		*ctxrule;

	/**
	 * The ruleset that the above rules point to.
	 */
	struct apn_ruleset	*ruleset;

	/**
	 * The path and checksum of the context. This data is used
	 * to refresh the context, e.g. in case of a rule reload.
	 */
	struct pe_proc_ident	 ident;
};

/* Prototypes */
static struct pe_context	*pe_context_search(struct apn_ruleset *,
				     struct pe_proc_ident *, uid_t uid, int pg);
static int			 pe_context_decide(struct pe_proc *, int, int,
				     struct pe_proc_ident *, uid_t uid);
static struct pe_context	*pe_context_alloc(struct apn_ruleset *,
				     struct pe_proc_ident *);
static void			 pe_context_switch(struct pe_proc *, int,
				     struct pe_proc_ident *, uid_t);
static void			 pe_context_norules(struct pe_proc *, int);
static void			 pe_savedctx_norules(struct pe_proc *, int);


/**
 * Set the process context in the case where no ruleset is present to
 * search for the context.
 * - If the process never had a context for the priority, leave it alone.
 * - Otherwise create a norules context with the pident from the old context
 *   but without actual rules.
 *
 * @param The process to create a context for.
 * @param prio The priority for of the new context.
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
	if (!ctx)
		master_terminate();
	pe_proc_set_context(proc, prio, ctx);
	pe_context_put(ctx);
}

/**
 * Set the saved process context if no ruleset is present to search for a
 * replacment saved context.
 * - If the process never saved a context for the priority, leave it alone.
 * - Otherwise create  norules context with the pident from the old context
 *   but without actual rules.
 *
 * @param proc The process to create a context for.
 * @param prio The priority of the new context.
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
	if (!ctx)
		master_terminate();
	pe_proc_set_savedctx(proc, prio, ctx);
	pe_context_put(ctx);
}

/**
 * Update a context at a rules reload.
 * - If there are no rules a norules context is used.
 * - If there is an old context we search a new one based on the search
 *   parameters of that context.
 * - If there are rules but no old context we search for rules based on
 *   the processes identifier.
 * pdb contains the database that should be used to lookup rulesets. If pdb
 * is NULL the currently active policy database is used.
 *
 * @param proc The process.
 * @param prio The priority.
 * @param pdb The policy data base to search.
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
		    pe_proc_get_uid(proc), pe_proc_get_playgroundid(proc) != 0);
	} else {
		context = pe_context_search(newrules, pe_proc_ident(proc),
		    pe_proc_get_uid(proc), pe_proc_get_playgroundid(proc) != 0);
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
		    pe_proc_get_uid(proc), pe_proc_get_playgroundid(proc) != 0);
		pe_proc_set_savedctx(proc, prio, context);
		pe_context_put(context);
	}
	return;
}

/**
 * Decode a context into a printable string. This string is allocated
 * dynamically and must be freed by the caller.
 *
 * @param hdr The eventdev event. uid and pid are extracted from here.
 * @param proc One context of this process should be dumped.
 * @param prio The priority of the context to dump.
 * @return The text representation of the context.
 */
char *
pe_context_dump(struct eventdev_hdr *hdr, struct pe_proc *proc, int prio)
{
	struct pe_context	*ctx;
	char			*dump, *progctx;
	struct pe_proc_ident	*pident = NULL;
	long			 uidctx = -1;

	/* hdr must be non-NULL, proc might be NULL */
	if (hdr == NULL)
		return (NULL);

	progctx = "<none>";

	if (proc && 0 <= prio && prio <= PE_PRIO_MAX) {
		ctx = pe_proc_get_context(proc, prio);

		if (ctx) {
			if (ctx->ident.pathhint)
				progctx = ctx->ident.pathhint;
		}
		pident = pe_proc_ident(proc);
		uidctx = pe_proc_get_uid(proc);
	}

	if (asprintf(&dump, "uid %hu pid %hu program %s "
	    "context uid %ld program %s",
	    hdr->msg_uid, hdr->msg_pid,
	    (pident && pident->pathhint) ? pident->pathhint : "<none>",
	    uidctx, progctx) == -1)
		dump = NULL;

	return dump;
}

/**
 * Inherit the parent process context. This happens on fork(2).
 * - If our parent was never tracked, we simulate an exec.
 * - If the parent's uid and our uid do not match we simulate an exec, too.
 * - If the parent has a non NULL context for a prio. Use it.
 * - Otherwise if there are no rules for that uid/prio, use a norules
 *   context.
 * - If there are rules for the  process, use them to find a new context
 *   based on the current process's proc->ident.
 *
 * @param proc The new child process.
 * @param parent The parent process to inherit from.
 */
void
pe_context_fork(struct pe_proc *proc, struct pe_proc *parent)
{
	int	i;

	if (proc == NULL) {
		log_warnx("pe_context_fork: empty process");
		return;
	}

	DEBUG(DBG_PE_CTX, "pe_context_fork: parent %p 0x%08" PRIx64, parent,
	    pe_proc_task_cookie(parent));
	for (i = 0; i < PE_PRIO_MAX; i++) {
		struct pe_context	*ctx;

		/*
		 * Keep the parent's context if it has one and both processes
		 * run with the same uid (aka ruleset).
		 */
		if (parent && pe_proc_get_uid(parent) == pe_proc_get_uid(proc)
		    && (ctx = pe_proc_get_context(parent, i)) != NULL) {
			pe_proc_set_context(proc, i, ctx);
			continue;
		}
		DEBUG(DBG_PE_CTX, "pe_context_fork: parent %p "
		    "0x%08" PRIx64 " has no context at prio %d",
		    parent, pe_proc_task_cookie(parent),
		    i);
		pe_context_switch(proc, i, pe_proc_ident(proc),
		    pe_proc_get_uid(proc));
		pe_proc_drop_saved_ctx(proc, i);
	}
}

/**
 * Return true if a process identification (path and checksum) matches
 * the subject in an apn_app from an application block. This function
 * evaluates a single application only. The caller must iterate over
 * the application list.
 *
 * @note: The path name in the pident is not evaluated by this function.
 *        It only compares checksums. The path in the application is
 *        used to retrieve the checksum from the sfs tree.
 *
 * @param app A list of application structure from an application rule.
 * @param pident The process identification. Only the checksum is used.
 * @param uid The user ID of the process. This is used to retrieve the
 *     checksum or key if the subject is of type UID_SELF or KEY_SELF.
 * @return True if the process and the subject in the application match.
 */
static int
pe_context_subject_match(const struct apn_app *app,
    const struct pe_proc_ident *pident, uid_t uid)
{
	const struct apn_subject	*subject;
	struct abuf_buffer		 csum = ABUF_EMPTY;
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
		ret = sfshash_get_uid(app->name, uid, &csum);
		if (ret != 0)
			return 0;
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
		ret = sfshash_get_key(app->name, keyid, &csum);
		if (subject->type == APN_CS_KEY_SELF)
			free(keyid);
		if (ret != 0)
			return 0;
		break;
	default:
		return 0;
	}
	ret = 0;
	if (!abuf_empty(csum) && !abuf_empty(pident->csum))
		ret = abuf_equal(csum, pident->csum);
	abuf_free(csum);
	return ret;
}

/**
 * Return true if the process given by the process identifier and the
 * given apn_app from an application rule match. If the subject in the
 * apn_app is of type NONE, this function compares path name. If a
 * subject is given, the subject is compared instead.
 *
 * @note: The caller must iterate over the list of applications. This
 *        function only checks a single application.
 *
 * @param app The apn_app structure.
 * @param pident The process identifier of the process.
 * @param uid The user ID of the process. This is used for subjects
 *     of type UID_SELF and KEY_SELF.
 * @return True if apn_app and pident match.
 */
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

/**
 * Select a new context for priority prio of the process proc based on the
 * pident and uid given as parameters.
 *
 * @param proc The process to create a new context for.
 * @param prio The priority of the new context.
 * @param pident The process identification to use for the new context.
 *     This is usually the identifier of the new binary at exec time.
 * @param uid The user ID of the process. This is used to search for
 *     the rule set.
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
	tmpctx = pe_context_search(ruleset, pident, uid,
	    pe_proc_get_playgroundid(proc) != 0);
	pe_proc_set_context(proc, prio, tmpctx);
	DEBUG(DBG_PE_CTX, "pe_context_switch: proc %p 0x%08" PRIx64 " prio %d "
	    "got context %p alfrule %p sbrule %p ctxrule %p", proc,
	    pe_proc_task_cookie(proc), prio, tmpctx,
	    tmpctx ? tmpctx->alfrule : NULL, tmpctx ? tmpctx->sbrule : NULL,
	    tmpctx ? tmpctx->ctxrule : NULL);
	/*
	 * pe_context_search got a reference to the new context and
	 * pe_proc_set_context got another one. Drop one of them.
	 */
	pe_context_put(tmpctx);
}

/**
 * Change a context at exec time.
 * - If pe_context_decide forbids changing the context. Do not change it.
 * - Otherwise the context at the prio either allowed the context switch
 *   or it is still invalid (NULL) which implicitly allows us to switch
 *   contexts. In that case search a new context using pe_context_switch.
 *   This will properly handle the case where the old context is NULL.
 *
 * @param proc The process that did the exec.
 * @param uid The new user ID. This is used to search for the rule set.
 * @param pident The process identifier of the binary that is executed.
 *     It will be used for the new context if switching contexts is allowed.
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
		/* Do not switch if rules forbid it. */
		if (pe_context_decide(proc, APN_CTX_NEW, i, pident, uid) == 0)
			continue;
		DEBUG(DBG_PE_CTX,
		    "pe_context_exec: prio %d switching context", i);
		pe_context_switch(proc, i, pident, uid);
		pe_proc_drop_saved_ctx(proc, i);
	}
	if (uid != (uid_t)-1)
		pe_proc_set_uid(proc, uid);
}

/**
 * Return true if the process will switch its context if it executes
 * a given binary.
 *
 * @param proc The process.
 * @param uid The user ID of the process.
 * @param pident The identification of the binary to exec.
 * @return True if the process will changes its context.
 */
int
pe_context_will_transition(struct pe_proc *proc, uid_t uid,
    struct pe_proc_ident *pident)
{
	int			 i;
	struct apn_ruleset	*ruleset;
	struct pe_context	*tmpctx, *curctx;

	if (proc == NULL)
		return  0;
	for (i = 0; i < PE_PRIO_MAX; ++i) {
		if (pe_context_decide(proc, APN_CTX_NEW, i, pident, uid) == 0)
			continue;
		ruleset = pe_user_get_ruleset(uid, i, NULL);
		/* No need for secure exec if there is no ruleset. */
		if (!ruleset)
			continue;
		curctx = pe_proc_get_context(proc, i);
		tmpctx = pe_context_search(ruleset, pident, uid,
		    pe_proc_get_playgroundid(proc) != 0);
		/* Both contexts NULL => no context switch */
		if (!tmpctx && !curctx)
			continue;
		/*
		 * If at least one of the rule blocks is different, we
		 * have a context switch.
		 */
		if (!tmpctx || !curctx || tmpctx->alfrule != curctx->alfrule
		    || tmpctx->sbrule != curctx->sbrule
		    || tmpctx->ctxrule != curctx->ctxrule) {
			pe_context_put(tmpctx);
			return 1;
		}
		pe_context_put(tmpctx);
	}
	return 0;
}

/**
 * Return true if the given process will be forced into a playground
 * if it executes the given binary.
 *
 * @param proc The process.
 * @param uid The user ID of the process.
 * @param pident The process identification of the new binary.
 * @param ruleidp The rule that forced the context switch is returned here.
 * @param priop The priority of the rule that forced the context switch
 *     is returned here.
 * @return True if the process will be forced into a playground.
 */
int
pe_context_will_pg(struct pe_proc *proc, uid_t uid,
    struct pe_proc_ident *pident, int *ruleidp, int *priop)
{
	int			 i;
	struct apn_ruleset	*ruleset;
	struct pe_context	*newctx, *curctx;

	if (proc == NULL)
		return  0;

	for (i = 0; i < PE_PRIO_MAX; ++i) {
		/* Do not switch if rules forbid it. */
		if (pe_context_decide(proc, APN_CTX_NEW, i, pident, uid) == 0)
			continue;

		ruleset = pe_user_get_ruleset(uid, i, NULL);
		/* No need for playground if there is no ruleset. */
		if (!ruleset)
			continue;

		curctx = pe_proc_get_context(proc, i);
		newctx = pe_context_search(ruleset, pident, uid,
		    pe_proc_get_playgroundid(proc) != 0);

		if (!newctx)
			continue;
		if (curctx && newctx->ctxrule == curctx->ctxrule) {
			pe_context_put(newctx);
			continue;
		}
		if (pe_context_is_pg(newctx)) {
			if (ruleidp)
				(*ruleidp) = newctx->ctxrule->apn_id;
			if (priop)
				(*priop) = i;
			pe_context_put(newctx);
			return 1;
		}
		pe_context_put(newctx);
	}
	return 0;
}

/**
 * Change context on open(2):
 * - If pe_context_decide forbids changing the context, we don't.
 * - The logic is similar to pe_context_exec: Otherwise the context at the
 *   prio either allowed the context switch or it is still invalid (NULL)
 *   which implicitly allows us to switch contexts.  In that case search
 *   a new context using pe_context_switch.  Additionally the identity of the
 *   process is updated.
 *
 * @param proc The process,.
 * @param hdr The eventdev event of the open.
 * @return None.
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
	pident.csum = abuf_open_frommem(sfsmsg->csum, sizeof(sfsmsg->csum));

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

/**
 * Switch contexts at a borrow event. The current context will be
 * saved and restored once the connection identified by the connection
 * cookie terminates. A context is only borrowed, if an appropriate
 * rule exists. This is checked using pe_context_decide.
 *
 * @param proc The process that just connected to another process.
 * @param procp The peer process to borrow from.
 * @param cookie The cookie of the connection. When the connection
 *     closes, it will be reported with this cookie.
 */
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

/**
 * Restore a context after a unix socket connection closes. If the
 * connection has been used to borrow a context, the saved context
 * will be restored, now.
 *
 * @param proc The process.
 * @param cookie The cookie of the terminated connection.
 */
void
pe_context_restore(struct pe_proc *proc, anoubis_cookie_t cookie)
{
	int	 i;

	if (proc == NULL)
		return;

	for (i = 0; i < PE_PRIO_MAX; i++)
		pe_proc_restore_ctx(proc, i, cookie);
}

/**
 * Search an apn_chain (i.e. a list of application blocks) for the
 * first block that matches the given process identification.
 *
 * @param chain The chain to search.
 * @param pident The process identification.
 * @param uid The user ID of the process.
 * @param ispg True if the process runs in a playground.
 * @return The first application rule block that matched. NULL if there is
 *     none.
 */
static struct apn_rule *
pe_context_search_chain(struct apn_chain *chain, struct pe_proc_ident *pident,
    uid_t uid, int ispg)
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
		if (!ispg && (r->flags & APN_RULE_PGONLY))
			continue;
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

/**
 * Search for a context in the given ruleset. This function will only
 * return NULL if the ruleset is also NULL. Otherwise it will return
 * a context from the ruleset. If no such context can be found it will
 * return a non-NULL context which has no real rules but formally belongs
 * to the ruleset.
 *
 * @param rs The ruleset.
 * @param pident The process identification that is used to search for
 *     a matching application rule.
 * @param uid The user ID of the process.
 * @param ispg True if the process runs in a playground.
 * @return A new context. The context is created and returned with one
 *     reference held. It is not yet assigned to any process.
 */
static struct pe_context *
pe_context_search(struct apn_ruleset *rs, struct pe_proc_ident *pident,
    uid_t uid, int ispg)
{
	struct pe_context	*context;

	DEBUG(DBG_PE_CTX, "pe_context_search: ruleset %p path %s", rs,
	    (pident && pident->pathhint) ? pident->pathhint : NULL);

	/*
	 * We do not check csum and pathhint for being NULL, as we
	 * handle empty checksums and pathhint is actually not used, yet.
	 * The point is, that a process without a checksum can match
	 * a rule specifying "any" as application (see below).
	 */
	if (rs == NULL)
		return (NULL);

	context = pe_context_alloc(rs, pident);
	if (!context)
		master_terminate();
	context->alfrule = pe_context_search_chain(&rs->alf_queue, pident,
	    uid, ispg);
	context->sbrule = pe_context_search_chain(&rs->sb_queue, pident,
	    uid, ispg);
	context->ctxrule = pe_context_search_chain(&rs->ctx_queue, pident,
	    uid, ispg);

	DEBUG(DBG_PE_CTX, "pe_context_search: context %p alfrule %p sbrule %p "
	    "ctxrule %p", context, context->alfrule, context->sbrule,
	    context->ctxrule);

	return (context);
}

/**
 * Allocate and initailize a new context. This function initializes
 * the ruleset and the process identification of the context. The reference
 * counter is set to one, everything else is set to NULL.
 *
 * @param rs The ruleset of the context.
 * @param pident The process identification.
 * @return The new context.
 */
static struct pe_context *
pe_context_alloc(struct apn_ruleset *rs, struct pe_proc_ident *pident)
{
	struct pe_context	*ctx;

	if ((ctx = calloc(1, sizeof(struct pe_context))) == NULL) {
		log_warn("calloc");
		master_terminate();
	}
	ctx->alfrule = NULL;
	ctx->sbrule = NULL;
	ctx->ctxrule = NULL;
	ctx->ruleset = rs;
	ctx->refcount = 1;
	ctx->ident.csum = ABUF_EMPTY;
	ctx->ident.pathhint = NULL;
	if (pident)
		pe_proc_ident_set(&ctx->ident, pident->csum, pident->pathhint);

	return (ctx);
}

/**
 * Drop one reference to a context. If the reference counter reaches
 * zero, the context is freed.
 *
 * @param ctx The context.
 */
void
pe_context_put(struct pe_context *ctx)
{
	if (!ctx || --(ctx->refcount))
		return;
	pe_proc_ident_put(&ctx->ident);
	free(ctx);
}

/**
 * Get a reference count to an already existing context.
 *
 * @param ctx The context.
 */
void
pe_context_reference(struct pe_context *ctx)
{
	if (!ctx)
		return;

	ctx->refcount++;
}

/**
 * Decide, if it is ok to switch context for an application specified
 * by csum and pathhint.
 *
 * @param proc The process that wants to switch contexts.
 * @param type The type of the context switch (APN_CTX_*, i.e. NEW, BORROW
 *     or OPEN).
 * @param prio The priority of the context.
 * @param pident The process identifier of the new context (if context
 *     switch is allowed).
 * @param uid The user ID of the process.
 * @return True if context switching is allowed. No context was
 *     allocated at this time.
 *
 */
static int
pe_context_decide(struct pe_proc *proc, int type, int prio,
    struct pe_proc_ident *pident, uid_t uid)
{
	struct apn_app		*hp = NULL;
	struct apn_rule		*rule;
	struct pe_context	*ctx;
	int			 ret;

	ctx = pe_proc_get_context(proc, prio);
	/* No context: Allow a context switch for exec (APN_CTX_NEW). */
	if (ctx == NULL)
		return (type == APN_CTX_NEW);
	/* Force a context switch if this  will change the user-ID, too. */
	if (type == APN_CTX_NEW
	    && (proc == NULL || pe_proc_get_uid(proc) != uid))
		return 1;
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
	    "pe_context_decide: found \"%s\" for type %d priority %d",
	    (hp && hp->name) ? hp->name : "any", type, prio);
	return 1;
}

/**
 * Return true if the given context uses the given ruleset. If the
 * ruleset of the context is NULL, this function returns true, too.
 * This function is used to determine if a context refresh is required
 * after a ruleset reload.
 *
 * @param ctx The context.
 * @param rs The ruleset.
 * @return True if a refresh is required.
 */
int
pe_context_uses_rs(struct pe_context *ctx, struct apn_ruleset *rs)
{
	return ctx && rs && (ctx->ruleset == rs || ctx->ruleset == NULL);
}

/**
 * Return the alf application rule block of the context.
 *
 * @param ctx The context.
 * @return The ALF application rule.
 */
struct apn_rule *
pe_context_get_alfrule(struct pe_context *ctx)
{
	if (!ctx)
		return NULL;
	return ctx->alfrule;
}

/**
 * Return the sandbox application rule block of the context.
 *
 * @param ctx The context.
 * @return The sandbox application rule.
 */
struct apn_rule *
pe_context_get_sbrule(struct pe_context *ctx)
{
	if (!ctx)
		return NULL;
	return ctx->sbrule;
}

/**
 * Return the context application rule block of the context.
 *
 * @param ctx The context.
 * @return The context application rule.
 */
struct apn_rule *
pe_context_get_ctxrule(struct pe_context *ctx)
{
	if (!ctx)
		return NULL;
	return ctx->ctxrule;
}

/**
 * Return the process identification used by the context.
 *
 * @param ctx The context.
 * @return The process identifcation.
 */
struct pe_proc_ident *
pe_context_get_ident(struct pe_context *ctx)
{
	if (!ctx)
		return NULL;
	return &ctx->ident;
}

/**
 * Return true if the context has the nosfs flag set.
 *
 * @param ctx The context.
 * @return  True if the nosfs flag is set.
 */
int
pe_context_is_nosfs(struct pe_context *ctx)
{
	if (ctx == NULL || ctx->ctxrule == NULL)
		return 0;
	return !!(ctx->ctxrule->flags & APN_RULE_NOSFS);
}

/**
 * Return true if the context has the pgforce flag set.
 *
 * @param ctx The context.
 * @return  True if the pgforce flag is set.
 */
int
pe_context_is_pg(struct pe_context *ctx)
{
	if (ctx == NULL || ctx->ctxrule == NULL)
		return 0;
	return !!(ctx->ctxrule->flags & APN_RULE_PGFORCE);
}

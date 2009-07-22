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

#include <apn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef LINUX
#include <queue.h>
#include <bsdcompat.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/queue.h>
#include <dev/anoubis.h>
#endif

#include "anoubisd.h"
#include "pe.h"

/*
 * The flag is true if we have seen at least one process create or
 * process destroy messsage. This affects the behaviour of proc_is_running.
 */
static int			 have_task_tracking = 0;

static void			 pe_proc_track(struct pe_proc *);
static void			 pe_proc_untrack(struct pe_proc *);
static struct pe_proc		*pe_proc_alloc(uid_t uid, anoubis_cookie_t,
				    struct pe_proc_ident *);
static inline unsigned int	 pe_proc_get_flag(struct pe_proc *,
				     unsigned int);
static inline void		 pe_proc_set_flag(struct pe_proc *,
				     unsigned int);
static inline void		 pe_proc_clr_flag(struct pe_proc *,
				     unsigned int);
static inline void		 pe_proc_upgrade_inherit(struct pe_proc *,
				     unsigned int);

#define _PE_PROC_INTERNALS_H_
#include "pe_proc_internals.h"

TAILQ_HEAD(tracker, pe_proc) tracker;

void
pe_proc_ident_set(struct pe_proc_ident *pident, const u_int8_t *csum,
    const char *pathhint)
{
	if (csum) {
		if (pident->csum == NULL) {
			pident->csum = calloc(1, ANOUBIS_SFS_CS_LEN);
			if (pident->csum == NULL)
				goto oom;
		}
		bcopy(csum, pident->csum, ANOUBIS_SFS_CS_LEN);
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
	if (pathhint) {
		if ((pident->pathhint = strdup(pathhint)) == NULL)
			goto oom;
	}
	return;
oom:
	log_warnx("pe_proc_ident_set: out of memory");
	master_terminate(ENOMEM);
}

void
pe_proc_ident_put(struct pe_proc_ident *pident)
{
	if (pident->csum) {
		free(pident->csum);
		pident->csum = NULL;
	}
	if (pident->pathhint) {
		free(pident->pathhint);
		pident->pathhint = NULL;
	}
}

void
pe_proc_init(void)
{
	TAILQ_INIT(&tracker);
}

void
pe_proc_flush(void)
{
	struct pe_proc	*p, *pnext;

	for (p = TAILQ_FIRST(&tracker); p != TAILQ_END(&tracker); p = pnext) {
		pnext = TAILQ_NEXT(p, entry);
		TAILQ_REMOVE(&tracker, p, entry);
		pe_proc_put(p);
	}
}

struct pe_proc *
pe_proc_get(anoubis_cookie_t cookie)
{
	struct pe_proc	*proc;

	TAILQ_FOREACH(proc, &tracker, entry) {
		if (proc->task_cookie == cookie)
			break;
	}
	if (proc) {
		DEBUG(DBG_PE_TRACKER, "pe_proc_get: proc %p pid %d cookie "
		    "0x%08llx", proc, (int)proc->pid,
		    (unsigned long long)proc->task_cookie);
		proc->refcount++;
	}
	return (proc);
}

void
pe_proc_put(struct pe_proc *proc)
{
	int	i;

	if (!proc || --(proc->refcount))
		return;
	pe_proc_ident_put(&proc->ident);

	for (i = 0; i < PE_PRIO_MAX; i++) {
		pe_context_put(proc->context[i]);
		pe_context_put(proc->saved_ctx[i]);
	}
	free(proc);
}

static struct pe_proc *
pe_proc_alloc(uid_t uid, anoubis_cookie_t cookie, struct pe_proc_ident *pident)
{
	struct pe_proc	*proc;

	if ((proc = calloc(1, sizeof(struct pe_proc))) == NULL)
		goto oom;
	proc->task_cookie = cookie;
	proc->borrow_cookie = 0;
	proc->pid = (pid_t)-1;
	proc->uid = uid;
	proc->sfsdisable_pid = (pid_t)-1;
	proc->sfsdisable_uid = (uid_t)-1;
	proc->flags = 0;
	proc->refcount = 1;
	proc->instances = 1;
#ifdef ANOUBIS_PROCESS_OP_CREATE
	proc->threads = 0;
#else
	proc->threads = 1;
#endif
	proc->ident.pathhint = NULL;
	proc->ident.csum = NULL;
	if (pident)
		pe_proc_ident_set(&proc->ident, pident->csum, pident->pathhint);
	DEBUG(DBG_PE_TRACKER, "pe_proc_alloc: proc %p uid %u cookie 0x%08llx ",
	    proc, uid, (unsigned long long)proc->task_cookie);

	return (proc);
oom:
	log_warn("pe_proc_alloc: cannot allocate memory");
	master_terminate(ENOMEM);
	return (NULL);	/* XXX HSH */
}

static void
pe_proc_track(struct pe_proc *proc)
{
	if (proc == NULL) {
		log_warnx("pe_proc_track: empty process");
		return;
	}
	DEBUG(DBG_PE_TRACKER, "pe_proc_track: proc %p cookie 0x%08llx",
	    proc, (unsigned long long)proc->task_cookie);
	proc->refcount++;
	TAILQ_INSERT_TAIL(&tracker, proc, entry);
}

static void
pe_proc_untrack(struct pe_proc *proc)
{
	if (!proc)
		return;
	TAILQ_REMOVE(&tracker, proc, entry);
	pe_proc_put(proc);
}

struct pe_context *
pe_proc_get_context(struct pe_proc *proc, int prio)
{
	if (!proc)
		return NULL;
	if (prio < 0 || prio >= PE_PRIO_MAX)
		return NULL;
	return proc->context[prio];
}

void
pe_proc_set_context(struct pe_proc *proc, int prio, struct pe_context *ctx)
{
	if (prio < 0 || prio >= PE_PRIO_MAX)
		return;

	if (ctx)
		pe_context_reference(ctx);
	if (proc->context[prio])
		pe_context_put(proc->context[prio]);
	proc->context[prio] = ctx;
}

uid_t
pe_proc_get_uid(struct pe_proc *proc)
{
	return proc->uid;
}

void
pe_proc_set_uid(struct pe_proc *proc, uid_t uid)
{
	proc->uid = uid;
}

anoubis_cookie_t
pe_proc_task_cookie(struct pe_proc *proc)
{
	if (!proc)
		return 0;
	return proc->task_cookie;
}

struct pe_proc_ident *
pe_proc_ident(struct pe_proc *proc)
{
	return proc ? &proc->ident : NULL;
}

pid_t
pe_proc_get_pid(struct pe_proc *proc)
{
	return proc->pid;
}

void pe_proc_set_pid(struct pe_proc *proc, pid_t pid)
{
	if (!proc)
		return;

	DEBUG(DBG_PE_PROC, "pe_proc_set_pid: proc %p pid %d -> %d "
	    "cookie 0x%08llx", proc, (int)proc->pid, (int)pid,
	    (unsigned long long)proc->task_cookie);

	proc->pid = pid;
	if (proc->pid != proc->sfsdisable_pid) {
		proc->sfsdisable_pid = (pid_t)-1;
		proc->sfsdisable_uid = (uid_t)-1;
	}
}

void
pe_proc_dump(void)
{
	struct pe_proc *proc;
	struct pe_context *ctx0, *ctx1;

	log_info("tracked processes:");
	TAILQ_FOREACH(proc, &tracker, entry) {
		ctx0 = proc->context[0];
		ctx1 = proc->context[1];
		log_info("proc %p token 0x%08llx borrow token 0x%08llx "
		    "pid %d csum 0x%08x pathhint \"%s\" ctx %p %p alfrules "
		    "%p %p sbrules %p %p flags 0x%x",
		    proc, (unsigned long long)proc->task_cookie,
		    (unsigned long long)proc->borrow_cookie, (int)proc->pid,
		    proc->ident.csum ?
		    htonl(*(unsigned long *)proc->ident.csum) : 0,
		    proc->ident.pathhint ? proc->ident.pathhint : "",
		    ctx0, ctx1,
		    pe_context_get_alfrule(ctx0), pe_context_get_alfrule(ctx1),
		    pe_context_get_sbrule(ctx0), pe_context_get_sbrule(ctx1),
		    proc->flags);
	}
}

/*
 * Insert a newly forked process and set its context.
 */
void
pe_proc_fork(uid_t uid, anoubis_cookie_t child, anoubis_cookie_t parent_cookie)
{
	struct pe_proc	*proc, *parent;

	parent = pe_proc_get(parent_cookie);
	proc = pe_proc_alloc(uid, child, pe_proc_ident(parent));
	if (!proc)
		return;
	pe_proc_track(proc);
	pe_context_fork(proc, parent);

	/* Hand mark of upgrade process down. */
	pe_proc_upgrade_inherit(proc, pe_proc_is_upgrade(parent));

	DEBUG(DBG_PE_PROC, "pe_proc_fork: token 0x%08llx pid %d "
	    "uid %u proc %p csum 0x%08x... parent token 0x%08llx flags 0x%x",
	    (unsigned long long)child, proc->pid, uid, proc,
	    proc->ident.csum ?
	    htonl(*(unsigned long *)proc->ident.csum) : 0,
	    (unsigned long long)parent_cookie, proc->flags);
	pe_proc_put(parent);
	pe_proc_put(proc);
}

/*
 * Add another credentials instance to this process.
 */
void pe_proc_addinstance(anoubis_cookie_t cookie)
{
	struct pe_proc *proc = pe_proc_get(cookie);
	if (proc) {
		proc->instances++;
		pe_proc_put(proc);
	}
}

/*
 * Remove a process upon exit.
 */
void pe_proc_exit(anoubis_cookie_t cookie)
{
	struct pe_proc	*proc;

	proc = pe_proc_get(cookie);
	if (!proc)
		return;
	if (--proc->instances)
		return;
	pe_proc_untrack(proc);
	pe_upgrade_end(proc);
	DEBUG(DBG_PE_PROC, "pe_proc_exit: token 0x%08llx pid %d "
	    "uid %u proc %p", (unsigned long long)cookie, proc->pid, proc->uid,
	    proc);
	pe_proc_put(proc);
}

/*
 * Return true if the process is in fact running.
 */
int
pe_proc_is_running(anoubis_cookie_t cookie)
{
	int ret;
	struct pe_proc	*proc = pe_proc_get(cookie);
	ret = proc && (proc->threads > 0 || have_task_tracking == 0);
	pe_proc_put(proc);
	return ret;
}

/*
 * Account for a new thread that is part of the process.
 */
void
pe_proc_add_thread(anoubis_cookie_t cookie)
{
	struct pe_proc	*proc = pe_proc_get(cookie);

	have_task_tracking = 1;
	if (proc) {
		proc->threads++;
		pe_proc_put(proc);
	}
}

/*
 * Account for a terminated thread that is part of the process.
 */
void
pe_proc_remove_thread(anoubis_cookie_t cookie)
{
	struct pe_proc	*proc = pe_proc_get(cookie);

	have_task_tracking = 1;
	if (proc && proc->threads > 0) {
		proc->threads--;
		if (proc->threads == 0)
			pe_upgrade_end(proc);
	}
	pe_proc_put(proc);
}

/*
 * Update process attributes after an exec system call.
 */
void pe_proc_exec(anoubis_cookie_t cookie, uid_t uid, pid_t pid,
    const u_int8_t *csum, const char *pathhint)
{
	struct pe_proc		*proc = pe_proc_get(cookie);
	struct pe_context	*ctx0, *ctx1;

	if (proc == NULL) {
		/* Previously untracked process. Track it. */
		DEBUG(DBG_PE_PROC, "pe_proc_exec: untracked "
		    "process %u 0x%08llx execs", pid,
		    (unsigned long long)cookie);
		proc = pe_proc_alloc(uid, cookie, NULL);
		pe_proc_track(proc);
	}
	/* fill in checksum and pathhint */
	pe_proc_ident_set(&proc->ident, csum, pathhint);
	pe_proc_set_pid(proc, pid);
	pe_context_exec(proc, uid, &proc->ident);

	ctx0 = pe_proc_get_context(proc, 0);
	ctx1 = pe_proc_get_context(proc, 1);
	/* Get our policy */
	DEBUG(DBG_PE_PROC, "pe_proc_exec: using alfrules %p %p sbrules %p %p "
	    "for %s csum 0x%08x...",
	    pe_context_get_alfrule(ctx0), pe_context_get_alfrule(ctx1),
	    pe_context_get_sbrule(ctx0), pe_context_get_sbrule(ctx1),
	    pathhint ? pathhint : "", csum ? htonl(*(unsigned long *)csum) : 0);
	pe_proc_put(proc);

	if (uid == 0 &&
	    anoubisd_config.upgrade_mode == ANOUBISD_UPGRADE_MODE_PROCESS) {
		struct anoubisd_upgrade_trigger_list *upgrade_trigger;
		struct anoubisd_upgrade_trigger *trigger;

		upgrade_trigger = &anoubisd_config.upgrade_trigger;

		LIST_FOREACH(trigger, upgrade_trigger, entries) {
			if (strcmp(pathhint, trigger->arg) == 0) {
				DEBUG(DBG_UPGRADE, "Enabling upgrade for %s",
				    pathhint);

				pe_upgrade_start(proc);
				break;
			}
		}
	}
}

void
pe_proc_update_db(struct pe_policy_db *newpdb)
{
	struct pe_proc		*proc;
	int			 i;

	if (newpdb == NULL) {
		log_warnx("pe_proc_update_db: empty database pointer");
		return;
	}

	TAILQ_FOREACH(proc, &tracker, entry) {
		for (i = 0; i < PE_PRIO_MAX; i++)
			pe_context_refresh(proc, i, newpdb);
	}
}

/*
 * This function is not allowed to fail! It must remove all references
 * to the old ruleset from tracked processes.
 * NOTE: If proc->context[prio] is not NULL then proc->context[prio]->ruleset
 *       cannot be NULL either. oldrs == NULL is allowed, however.
 *       In the latter case there was no policy before and we refresh all
 *       processes with the given uid.
 */
void
pe_proc_update_db_one(struct apn_ruleset *oldrs, int prio, uid_t uid)
{
	struct pe_context	*oldctx;
	struct pe_proc		*proc;

	DEBUG(DBG_TRACE, ">pe_proc_update_db_one");
	TAILQ_FOREACH(proc, &tracker, entry) {
		oldctx = pe_proc_get_context(proc, prio);
		if (oldctx == NULL)
			continue;
		if ((pe_context_uses_rs(oldctx, oldrs) != 0)
		    || (oldrs == NULL && pe_proc_get_uid(proc) == uid)) {
			pe_context_refresh(proc, prio, NULL);
		}
	}
	DEBUG(DBG_TRACE, "<pe_proc_update_db_one");
}

/*
 * HACK ALERT:
 * We do set sfsdisable_uid and sfsdisable_pid even if proc->pid == -1.
 * However, this will not disable sfs for this process/thread right now.
 * It will only do so once proc->pid has been set proc->sfsdisable_pid.
 */
int
pe_proc_set_sfsdisable(pid_t pid, uid_t uid)
{
	int found = 0;
	struct pe_proc	*proc;

	TAILQ_FOREACH(proc, &tracker, entry) {
		DEBUG(DBG_PE_TRACKER, "pe_proc_set_sfsdisable: Trying proc %p "
		    "pid %d cookie 0x%08llx", proc, (int)proc->pid,
		    (unsigned long long)proc->task_cookie);
		if (proc->uid != uid)
			continue;
		if (proc->pid != (pid_t)-1 && proc->pid != pid)
			continue;
		DEBUG(DBG_PE_TRACKER, "pe_proc_set_sfsdisable: proc %p pid %d "
		    "cookie 0x%08llx", proc, (int)proc->pid,
		    (unsigned long long)proc->task_cookie);
		proc->sfsdisable_pid = pid;
		proc->sfsdisable_uid = uid;
		found = 1;
	}
	return found;
}

int
pe_proc_is_sfsdisable(struct pe_proc *proc, uid_t uid)
{
	if (!proc)
		return 0;
	return (uid != (uid_t)-1) && (proc->pid != (pid_t)-1)
	    && (proc->sfsdisable_pid == proc->pid)
	    && (proc->sfsdisable_uid == uid);
}

void
pe_proc_save_ctx(struct pe_proc *proc, int prio, anoubis_cookie_t cookie)
{
	if (proc == NULL)
		return;
	if (prio < 0 || prio >= PE_PRIO_MAX)
		return;
	if (proc->saved_ctx[prio] != NULL)
		return;

	proc->saved_ctx[prio] = proc->context[prio];
	proc->context[prio] = NULL;
	proc->borrow_cookie = cookie;
}

void
pe_proc_restore_ctx(struct pe_proc *proc, int prio, anoubis_cookie_t cookie)
{
	struct pe_context *ctx;

	if (proc == NULL)
		return;
	if (prio < 0 || prio >= PE_PRIO_MAX)
		return;
	if (proc->saved_ctx[prio] == NULL)
		return;
	if (proc->borrow_cookie != cookie)
		return;

	ctx = proc->saved_ctx[prio];
	if (ctx) {
		pe_proc_set_context(proc, prio, ctx);
		/* pe_proc_set_context got a reference to this context. */
		pe_context_put(ctx);
		proc->saved_ctx[prio] = NULL;
	}
}

struct pe_context *
pe_proc_get_savedctx(struct pe_proc *proc, int prio)
{
	if (!proc)
		return NULL;
	if (prio < 0 || prio >= PE_PRIO_MAX)
		return NULL;
	return proc->saved_ctx[prio];
}

void
pe_proc_set_savedctx(struct pe_proc *proc, int prio, struct pe_context *ctx)
{
	if (prio < 0 || prio >= PE_PRIO_MAX)
		return;

	if (ctx)
		 pe_context_reference(ctx);
	if (proc->saved_ctx[prio])
		pe_context_put(proc->saved_ctx[prio]);
	proc->saved_ctx[prio] = ctx;
}

unsigned int
pe_proc_is_upgrade(struct pe_proc *proc)
{
	return (pe_proc_get_flag(proc, PE_PROC_FLAGS_UPGRADE));
}

unsigned int
pe_proc_is_upgrade_parent(struct pe_proc *proc)
{
	return (pe_proc_get_flag(proc, PE_PROC_FLAGS_UPGRADE_PARENT));
}

void
pe_proc_upgrade_addmark(struct pe_proc *proc)
{
	pe_proc_set_flag(proc, PE_PROC_FLAGS_UPGRADE);
	pe_proc_set_flag(proc, PE_PROC_FLAGS_UPGRADE_PARENT);
}

void
pe_proc_upgrade_clrmark(struct pe_proc *proc)
{
	pe_proc_clr_flag(proc, PE_PROC_FLAGS_UPGRADE);
	pe_proc_clr_flag(proc, PE_PROC_FLAGS_UPGRADE_PARENT);
}

static inline unsigned int
pe_proc_get_flag(struct pe_proc *proc, unsigned int flag)
{
	if (!proc) {
		return (0);
	}

	if (proc->flags & flag) {
		return (1);
	} else {
		return (0);
	}
}

static inline void
pe_proc_set_flag(struct pe_proc *proc, unsigned int flag)
{
	if (proc != NULL) {
		/* Set flag */
		proc->flags |= flag;
	}
}

static inline void
pe_proc_clr_flag(struct pe_proc *proc, unsigned int flag)
{
	if (proc != NULL) {
		/* Remove flag */
		proc->flags &= ~flag;
	}
}

static inline void
pe_proc_upgrade_inherit(struct pe_proc *proc, unsigned int flag)
{
	if (flag != 0) {
		pe_proc_set_flag(proc, PE_PROC_FLAGS_UPGRADE);
	} else {
		pe_proc_clr_flag(proc, PE_PROC_FLAGS_UPGRADE);
	}
}

void
pe_proc_upgrade_clrallmarks(void)
{
	struct pe_proc	*proc;

	TAILQ_FOREACH(proc, &tracker, entry) {
		pe_proc_upgrade_clrmark(proc);
	}
}

void
pe_proc_hold(struct pe_proc *proc)
{
	pe_proc_set_flag(proc, PE_PROC_FLAGS_HOLD);
}

int
pe_proc_is_hold(struct pe_proc *proc)
{
	return pe_proc_get_flag(proc, PE_PROC_FLAGS_HOLD);
}

void
pe_proc_release(void)
{
	struct pe_proc		*proc;

	TAILQ_FOREACH(proc, &tracker, entry) {
		if (!pe_proc_get_flag(proc, PE_PROC_FLAGS_HOLD))
			continue;
		pe_proc_clr_flag(proc, PE_PROC_FLAGS_HOLD);
		pe_upgrade_start(proc);
	}
}

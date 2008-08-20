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
#include "kernelcache.h"
#include "pe.h"

static void		 pe_proc_ident_set(struct pe_proc_ident *,
			     const u_int8_t *, const char *);
static void		 pe_proc_track(struct pe_proc *);
static void		 pe_proc_untrack(struct pe_proc *);
static struct pe_proc	*pe_proc_alloc(uid_t uid, anoubis_cookie_t,
			     anoubis_cookie_t);

struct pe_proc {
	TAILQ_ENTRY(pe_proc)	 entry;
	int			 refcount;
	pid_t			 pid;
	uid_t			 uid;

	struct pe_proc_ident	 ident;
	anoubis_cookie_t	 task_cookie;
	struct pe_proc		*parent;

	/* Per priority contexts */
	struct pe_context	*context[PE_PRIO_MAX];
	int			 valid_ctx;

	/* Pointer to kernel cache */
	struct anoubis_kernel_policy_header	*kcache;
};
TAILQ_HEAD(tracker, pe_proc) tracker;

static void
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
	struct pe_proc	*p, *proc;

	proc = NULL;
	TAILQ_FOREACH(p, &tracker, entry) {
		if (p->task_cookie == cookie) {
			proc = p;
			break;
		}
	}
	if (proc) {
		DEBUG(DBG_PE_TRACKER, "pe_proc_get: proc %p pid %d cookie "
		    "0x%08llx", proc, (int)proc->pid, proc->task_cookie);
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
	if (proc->parent != proc)
		pe_proc_put(proc->parent);
	if (proc->ident.csum)
		free(proc->ident.csum);
	if (proc->ident.pathhint)
		free(proc->ident.pathhint);

	for (i = 0; i < PE_PRIO_MAX; i++)
		pe_context_put(proc->context[i]);

	kernelcache_clear(proc->kcache);

	free(proc);
}

static struct pe_proc *
pe_proc_alloc(uid_t uid, anoubis_cookie_t cookie,
    anoubis_cookie_t parent_cookie)
{
	struct pe_proc	*proc, *parent;

	if ((proc = calloc(1, sizeof(struct pe_proc))) == NULL)
		goto oom;
	proc->task_cookie = cookie;
	parent = pe_proc_get(parent_cookie);
	proc->parent = parent;
	proc->pid = -1;
	proc->uid = uid;
	proc->refcount = 1;
	proc->kcache = NULL;
	proc->valid_ctx = 0;
	proc->ident.pathhint = NULL;
	proc->ident.csum = NULL;
	if (parent) {
		pe_proc_ident_set(&proc->ident, parent->ident.csum,
		    parent->ident.pathhint);
	}

	DEBUG(DBG_PE_TRACKER, "pe_proc_alloc: proc %p uid %u cookie 0x%08llx "
	    "parent cookie 0x%08llx", proc, uid, proc->task_cookie,
	    proc->parent ? parent_cookie : 0);

	return (proc);
oom:
	log_warn("pe_proc_alloc: cannot allocate memory");
	master_terminate(ENOMEM);
	return (NULL);	/* XXX HSH */
}

void
pe_proc_set_parent(struct pe_proc *proc, struct pe_proc *newparent)
{
	struct pe_proc *oldparent = proc->parent;

	proc->parent = newparent;
	if (proc != newparent)
		newparent->refcount++;
	if (oldparent && oldparent != proc)
		pe_proc_put(oldparent);
}

static void
pe_proc_track(struct pe_proc *proc)
{
	if (proc == NULL) {
		log_warnx("pe_track_proc: empty process");
		return;
	}
	DEBUG(DBG_PE_TRACKER, "pe_track_proc: proc %p cookie 0x%08llx",
	    proc, proc->task_cookie);
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

/*
 * NOTE (CEH): This function is deprecated. There should be no need to use
 * NOTE (CEH): it in new code. Either this will go away RSN or we will need
 * NOTE (CEH): a valid_context flag per priority.
 */
int
pe_proc_valid_context(struct pe_proc *proc)
{
	return proc->valid_ctx;
}

struct pe_context *
pe_proc_get_context(struct pe_proc *proc, int prio)
{
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
	proc->valid_ctx = 1;
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
	if (proc)
		proc->pid = pid;
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
		log_info("proc %p token 0x%08llx pproc %p pid %d csum 0x%08x "
		    "pathhint \"%s\" ctx %p %p rules %p %p", proc,
		    proc->task_cookie, proc->parent, (int)proc->pid,
		    proc->ident.csum ?
		    htonl(*(unsigned long *)proc->ident.csum) : 0,
		    proc->ident.pathhint ? proc->ident.pathhint : "",
		    ctx0, ctx1,
		    pe_context_get_rule(ctx0), pe_context_get_rule(ctx1));
	}
}

/*
 * Insert an existing but previously untracked process. If possible always
 * use pe_proc_fork instead. This is only for processes that were running
 * before the anoubisd started.
 */
struct pe_proc *
pe_proc_insert(anoubis_cookie_t cookie, uid_t uid)
{
	struct pe_proc *proc;

	proc = pe_proc_alloc(uid, cookie, 0);
	pe_context_set(proc, uid, NULL);
	pe_proc_track(proc);
	return proc;
}

/*
 * Insert a newly forked processes and set its context.
 */
void
pe_proc_fork(uid_t uid, anoubis_cookie_t child, anoubis_cookie_t parent)
{
	struct pe_proc	*proc;

	proc = pe_proc_alloc(uid, child, parent);
	if (!proc)
		return;
	pe_proc_track(proc);
	pe_context_inherit(proc, proc->parent);
	DEBUG(DBG_PE_PROC, "pe_proc_fork: token 0x%08llx pid %d "
	    "uid %u proc %p csum 0x%08x... parent token 0x%08llx",
	    child, proc->pid, uid, proc,
	    proc->ident.csum ?
	    htonl(*(unsigned long *)proc->ident.csum) : 0,
	    parent);
	pe_proc_put(proc);
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
	pe_proc_untrack(proc);
	DEBUG(DBG_PE_PROC, "pe_proc_exit: token 0x%08llx pid %d "
	    "uid %u proc %p", cookie, proc->pid, proc->uid, proc);
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
		    "process %u 0x%08llx execs", pid, cookie);
		proc = pe_proc_insert(cookie, uid);
	}
	/* fill in checksum and pathhint */
	pe_proc_ident_set(&proc->ident, csum, pathhint);
	pe_proc_set_pid(proc, pid);
	pe_context_set(proc, uid, &proc->ident);

	ctx0 = pe_proc_get_context(proc, 0);
	ctx1 = pe_proc_get_context(proc, 1);
	/* Get our policy */
	DEBUG(DBG_PE_PROC, "pe_proc_exec: using policies %p %p "
	    "for %s csum 0x%08lx...", pe_context_get_rule(ctx0),
	    pe_context_get_rule(ctx1), pathhint ? pathhint : "",
	    csum ? htonl(*(unsigned long *)csum) : 0);
	pe_proc_put(proc);
}

void
pe_context_refresh(struct pe_proc *proc, int prio, struct pe_policy_db *pdb)
{
	struct pe_context	*newctx;

	DEBUG(DBG_PE_POLICY, "pe_context_refresh: proc %p prio %d context %p",
	    proc, prio, pe_proc_get_context(proc, prio));
	if (pe_context_update(proc, &newctx, prio, pdb) == -1) {
		log_warnx("Failed to replace context");
		newctx = 0;
	}
	pe_proc_set_context(proc, prio, newctx);
	pe_context_put(newctx);
	pe_proc_kcache_clear(proc);
}

void
pe_proc_update_db(struct pe_policy_db *newpdb)
{
	struct pe_proc		*pproc;
	int			 i;

	if (newpdb == NULL) {
		log_warnx("pe_proc_update_db: empty database pointer");
		return;
	}

	TAILQ_FOREACH(pproc, &tracker, entry) {
		for (i = 0; i < PE_PRIO_MAX; i++)
			pe_context_refresh(pproc, i, newpdb);
	}
}

/*
 * This function is not allowed to fail! It must remove all references
 * to the old ruleset from tracked processes.
 * NOTE: If pproc->context[prio] is not NULL then pproc->context[prio]->ruleset
 *       cannot be NULL either. oldrs == NULL is allowed, however.
 */
void
pe_proc_update_db_one(struct apn_ruleset *oldrs, uid_t uid, int prio)
{
	struct pe_context	*oldctx;
	struct pe_proc		*pproc;
	struct pe_proc_ident	*pident;

	DEBUG(DBG_TRACE, ">pe_proc_update_db_one");
	TAILQ_FOREACH(pproc, &tracker, entry) {
		oldctx = pe_proc_get_context(pproc, prio);
		if (oldctx == NULL)
			continue;
		/*
		 * XXX CEH: For now we change the rules of a process if
		 * XXX CEH:  - it is running with rules from the ruleset that
		 * XXX CEH:    is being replaced   or
		 * XXX CEH:  - its registered user ID matches that of the
		 * XXX CEH:    user that is replacing its rules.
		 */
		if (pe_proc_get_uid(pproc) != uid &&
		    pe_context_uses_rs(oldctx, oldrs) == 0)
			continue;
		pe_context_refresh(pproc, prio, NULL);
		pident = pe_proc_ident(pproc);
		if (pident->csum && pident->pathhint) {
			pe_context_set(pproc, pe_proc_get_uid(pproc), pident);
		}
	}
	DEBUG(DBG_TRACE, "<pe_proc_update_db_one");
}

void
pe_proc_kcache_add(struct pe_proc *proc, struct anoubis_kernel_policy *policy)
{
	proc->kcache = kernelcache_add(proc->kcache, policy);
	kernelcache_send2master(proc->kcache, proc->pid);
}

void
pe_proc_kcache_clear(struct pe_proc *proc)
{
	if (proc->kcache) {
		proc->kcache = kernelcache_clear(proc->kcache);
		kernelcache_send2master(proc->kcache, proc->pid);
	}
}

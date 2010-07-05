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

#include "anoubisd.h"
#include "pe.h"
#include <anoubis_alloc.h>

/*
 * The flag is true if we have seen at least one process create or
 * process destroy messsage. This affects the behaviour of proc_is_running.
 */
static int			 have_task_tracking = 0;

static void			 pe_proc_track(struct pe_proc *);
static void			 pe_proc_untrack(struct pe_proc *);
static struct pe_proc		*pe_proc_alloc(uid_t uid, anoubis_cookie_t,
				    struct pe_proc_ident *, anoubis_cookie_t);
static inline unsigned int	 pe_proc_get_flag(struct pe_proc *,
				     unsigned int);
static inline void		 pe_proc_set_flag(struct pe_proc *,
				     unsigned int);
static inline void		 pe_proc_clr_flag(struct pe_proc *,
				     unsigned int);
static inline void		 pe_proc_upgrade_inherit(struct pe_proc *,
				     unsigned int);
static inline void		 pe_proc_secure_inherit(struct pe_proc *,
				     unsigned int);

#define _PE_PROC_INTERNALS_H_
#include "pe_proc_internals.h"

/**
 * A list of all processes known to the process management of anoubisd.
 */
TAILQ_HEAD(tracker, pe_proc) tracker;

/**
 * Fill the process identifier with a copy of the data given as parameters.
 * Any memory associated with the old process identifier is freed.
 *
 * @param A pointer to the new process identifier. The identifer must be
 *     valid and any memory allocated for the path or checksum data will
 *     be freed.
 * @param csum The new sha256 checksum. The memory will be copied.
 * @param pathhint The new pathhint. The memory will be copied.
 * @return None.
 */
void
pe_proc_ident_set(struct pe_proc_ident *pident, const struct abuf_buffer csum,
    const char *pathhint)
{
	if (!abuf_empty(csum)) {
		if (abuf_empty(pident->csum)) {
			pident->csum = abuf_alloc(ANOUBIS_SFS_CS_LEN);
			if (abuf_empty(pident->csum))
				goto oom;
		}
		abuf_copy(pident->csum, csum);
	} else {
		abuf_free(pident->csum);
		pident->csum = ABUF_EMPTY;
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

/**
 * Release all memory associated with a process identifier. This function
 * frees the checksum buffer and the pathint. The process identifier
 * will be initialized with an empty checksum buffer and pathhint.
 *
 * @param pident The process identifier.
 * @return None.
 */
void
pe_proc_ident_put(struct pe_proc_ident *pident)
{
	abuf_free(pident->csum);
	pident->csum = ABUF_EMPTY;
	if (pident->pathhint) {
		free(pident->pathhint);
		pident->pathhint = NULL;
	}
}

/**
 * Initailize the process tracking data structures.
 *
 * @param None.
 * @return None.
 */
void
pe_proc_init(void)
{
	TAILQ_INIT(&tracker);
}

/**
 * Free all memory allocated with the process tracking. This is
 * called in response to a HUP signal.
 *
 * @param None.
 * @return None.
 */
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

/**
 * Return the process structure for the given task cookie. If a process
 * with the given task cookie exists, a reference to its proc structure is
 * acquired and a pointer to the structure is returned.
 *
 * @param cookie The task cookie.
 * @return NULL if the process is not tracked or a pointer to the proc
 *     structure with the reference count increased.
 */
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
		    "0x%08" PRIx64, proc, (int)proc->pid, proc->task_cookie);
		proc->refcount++;
	}
	return (proc);
}

/**
 * Release one reference to the proc structure. If the reference count
 * reaches zero the structure is freed. Callers should use this function
 * to drop their reference to a process structure. Freeing the process
 * structure directly is not allowed.
 *
 * @param proc The proc structure.
 * @return None.
 */
void
pe_proc_put(struct pe_proc *proc)
{
	int	i;

	if (!proc || --(proc->refcount))
		return;
	pe_playground_delete(proc->pgid, proc);
	pe_proc_ident_put(&proc->ident);

	for (i = 0; i < PE_PRIO_MAX; i++) {
		pe_context_put(proc->context[i]);
		pe_context_put(proc->saved_ctx[i]);
	}
	abuf_free_type(proc, struct pe_proc);
}

/**
 * Set the playground ID of a process. This function make sure that the
 * playground management accounts for the new pgid of the process.
 *
 * @param proc The process that changes its playgroundid. The process
 *     may be new and not yet tracked.
 * @param pgid The new playground ID to set.
 * @return None.
 */
void
pe_proc_set_playgroundid(struct pe_proc *proc, anoubis_cookie_t pgid)
{
	if (proc && proc->pgid != pgid) {
		if (proc->pgid)
			pe_playground_delete(pgid, proc);
		proc->pgid = pgid;
		if (proc->pgid)
			pe_playground_add(pgid, proc);
	}
}

/**
 * Return the playground ID of the given process.
 *
 * @param The process.
 * @return The playground ID. This function returns zero if proc is NULL.
 */
anoubis_cookie_t
pe_proc_get_playgroundid(struct pe_proc *proc)
{
	if (!proc)
		return 0;
	return proc->pgid;
}

/**
 * Allocate a new process structure. The reference count of the new structure
 * is one, i.e. a single pe_proc_put will free the memory unless other
 * references are taken.
 *
 * @param uid The user-ID of the new process.
 * @param cookie The task cookie of the new process.
 * @param pident The process identifier of the new process. This function
 *     copies the data in pident, the memory allocated by the caller for the
 *     pident parameter is untouched.
 * @param pgid The playground ID of the new process.
 * @return A newly allocated process structure.. The structure is filled with
 *     the data from the parameters. If memory allocation fails, NULL is
 *     retunred.
 */
static struct pe_proc *
pe_proc_alloc(uid_t uid, anoubis_cookie_t cookie, struct pe_proc_ident *pident,
    anoubis_cookie_t pgid)
{
	struct pe_proc	*proc;

	if ((proc = abuf_zalloc_type(struct pe_proc)) == NULL)
		goto oom;
	proc->task_cookie = cookie;
	proc->borrow_cookie = 0;
	proc->pid = (pid_t)-1;
	proc->uid = uid;
	proc->flags = 0;
	proc->refcount = 1;
	proc->instances = 1;
	proc->pgid = 0;
#ifdef ANOUBIS_PROCESS_OP_CREATE
	proc->threads = 0;
#else
	proc->threads = 1;
#endif
	proc->ident.pathhint = NULL;
	proc->ident.csum = ABUF_EMPTY;
	if (pident)
		pe_proc_ident_set(&proc->ident, pident->csum, pident->pathhint);
	DEBUG(DBG_PE_TRACKER, "pe_proc_alloc: proc %p uid %u cookie 0x%08"
	    PRIx64 " ", proc, uid, proc->task_cookie);
	pe_proc_set_playgroundid(proc, pgid);

	return (proc);
oom:
	log_warn("pe_proc_alloc: cannot allocate memory");
	master_terminate(ENOMEM);
	return (NULL);	/* XXX HSH */
}

/**
 * Add the given process to the global list of tracked processes.
 * This function takes an additional reference count on the process that
 * will be released be pe_proc_untrack.
 *
 * @param The process.
 * @return None.
 */
static void
pe_proc_track(struct pe_proc *proc)
{
	if (proc == NULL) {
		log_warnx("pe_proc_track: empty process");
		return;
	}
	DEBUG(DBG_PE_TRACKER, "pe_proc_track: proc %p cookie 0x%08" PRIx64,
	    proc, proc->task_cookie);
	proc->refcount++;
	TAILQ_INSERT_TAIL(&tracker, proc, entry);
}

/**
 * Remove the process from the list of tracked processes. This function
 * drops the reference on the process that was obtained by pe_proc_track.
 *
 * @param proc The process.
 * @return None.
 */
static void
pe_proc_untrack(struct pe_proc *proc)
{
	if (!proc)
		return;
	TAILQ_REMOVE(&tracker, proc, entry);
	pe_proc_put(proc);
}

/**
 * Return the rules context with the given priority of the process.
 *
 * @param proc The process.
 * @param prio The rule priority (PE_PRIO_USER or PE_PRIO_ADMIN).
 * @return The rules context as requested. NULL if the process is NULL
 *     or no approriate context is defined.
 *
 * NOTE: The pointer returned by this function is invalidated if the
 *     rules context of the process changes or if the process is freed.
 *     I.e. the caller must either make sure that this cannot happen or
 *     take a reference of its own on the context before dropping its
 *     reference to the process.
 */
struct pe_context *
pe_proc_get_context(struct pe_proc *proc, int prio)
{
	if (!proc)
		return NULL;
	if (prio < 0 || prio >= PE_PRIO_MAX)
		return NULL;
	return proc->context[prio];
}

/**
 * Set the rules context of the given process to a new value.
 *
 * @param proc The process.
 * @param ctx The new rules context.
 * @return None.
 *
 * NOTE: The proc structure holds a reference to the current rules context.
 *     The reference on the old rules context (if any) is dropped and a
 *     new reference on the new context is acquired.
 */
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

/**
 * Return the current user-ID assigned to the process.
 *
 * @param proc The process.
 * @return The user ID is recorded in the process tracking.
 */
uid_t
pe_proc_get_uid(struct pe_proc *proc)
{
	return proc->uid;
}

/**
 * Change the user-ID of the process in the process tracking.
 *
 * @param proc The process.
 * @param uid The new user-ID.
 * @return None.
 */
void
pe_proc_set_uid(struct pe_proc *proc, uid_t uid)
{
	proc->uid = uid;
}

/**
 * Return the task cookie of the process.
 *
 * @param proc The process.
 * @return The task cookie.
 */
anoubis_cookie_t
pe_proc_task_cookie(struct pe_proc *proc)
{
	if (!proc)
		return 0;
	return proc->task_cookie;
}

/**
 * Return the process identifier of the process. The process identifier
 * encapsulates both pathname and checksum of the running binary.
 *
 * @param proc The process.
 * @return The process identifier.
 *
 * NOTE: This function returns a pointer to the process identifier that
 *     is already present in the struct pe_proc. No memory is copied and
 *     the life time of the pointer is limited by the life time of the
 *     surrounding proc structure.
 */
struct pe_proc_ident *
pe_proc_ident(struct pe_proc *proc)
{
	return proc ? &proc->ident : NULL;
}

/**
 * Return the process-ID of the process as recoreded in the process
 * management structure.
 *
 * @param The process.
 * @return The process ID.
 */
pid_t
pe_proc_get_pid(struct pe_proc *proc)
{
	return proc->pid;
}

/**
 * Set the process ID of the process in the process management structure.
 *
 * @param proc The process.
 * @param pid The new process ID.
 * @return None.
 */
void pe_proc_set_pid(struct pe_proc *proc, pid_t pid)
{
	if (!proc)
		return;

	DEBUG(DBG_PE_PROC, "pe_proc_set_pid: proc %p pid %d -> %d "
	    "cookie 0x%08" PRIx64, proc, (int)proc->pid, (int)pid,
	    proc->task_cookie);

	proc->pid = pid;
}

/**
 * Print a summary of all processes known to the process tracking.
 * The data is printed using log_info. Where this data ends up depends on
 * the command line arguments and potentially the syslog.conf file.
 *
 * @param None.
 * @return None.
 */
void
pe_proc_dump(void)
{
	struct pe_proc *proc;
	struct pe_context *ctx0, *ctx1;

	log_info("tracked processes:");
	TAILQ_FOREACH(proc, &tracker, entry) {
		ctx0 = proc->context[0];
		ctx1 = proc->context[1];
		log_info("proc %p token 0x%08" PRIx64 " borrow token 0x%08"
		    PRIx64 " pid %d pathhint \"%s\" ctx %p %p "
		    "alfrules %p %p sbrules %p %p flags 0x%x",
		    proc, proc->task_cookie,
		    proc->borrow_cookie, (int)proc->pid,
		    proc->ident.pathhint ? proc->ident.pathhint : "",
		    ctx0, ctx1,
		    pe_context_get_alfrule(ctx0), pe_context_get_alfrule(ctx1),
		    pe_context_get_sbrule(ctx0), pe_context_get_sbrule(ctx1),
		    proc->flags);
	}
}

/**
 * Insert a newly forked process and set its context. This function
 * creates a new process structure and fills it with the data from the
 * parameters. The process is inserted into the process tracking.
 *
 * @param uid The user-ID of the new process.
 * @param child The task cookie of the new process.
 * @param parent_cookie The task cookie of the parent process that is forking.
 * @param pgid The playground ID of the process.
 * @return None.
 */
void
pe_proc_fork(uid_t uid, anoubis_cookie_t child, anoubis_cookie_t parent_cookie,
    anoubis_cookie_t pgid)
{
	struct pe_proc	*proc, *parent;

	parent = pe_proc_get(parent_cookie);
	proc = pe_proc_alloc(uid, child, pe_proc_ident(parent), pgid);
	if (!proc) {
		pe_proc_put(parent);
		return;
	}
	pe_proc_track(proc);
	pe_context_fork(proc, parent);

	/* Hand mark of upgrade process down. */
	pe_proc_upgrade_inherit(proc, pe_proc_is_upgrade(parent));
	pe_proc_secure_inherit(proc, pe_proc_is_secure(parent));

	DEBUG(DBG_PE_PROC, "pe_proc_fork: token 0x%08" PRIx64 " pid %d "
	    "uid %u proc %p parent token 0x%08" PRIx64
	    " flags 0x%x", child, proc->pid, uid, proc,
	    parent_cookie, proc->flags);
	pe_proc_put(parent);
	pe_proc_put(proc);
}

/**
 * Add another credentials instance to this process. This function is
 * called if the kernel notifies the daemon that new credentials that were
 * reported as a new process previously, are now used for an already existing
 * process. The total number of credentials structures with the same
 * process ID are tracked in the instances field of the process structure.
 *
 * @param cookie The cookie of the process that gets a new instance.
 * @return None.
 *
 * NOTE: The caller should probably pair this function with a call to
 *     pe_proc_exit for the task cookie that was previously reported and
 *     got replaced, as this task cookie will never be used for a real
 *     process.
 */
void pe_proc_addinstance(anoubis_cookie_t cookie)
{
	struct pe_proc *proc = pe_proc_get(cookie);
	if (proc) {
		proc->instances++;
		pe_proc_put(proc);
	}
}

/**
 * Remove a process upon exit. This function is called when the kernel
 * frees credentials. We reduce the number of instances on the process and
 * once this number reaches zero the process is removed from the process
 * tracking.
 *
 * @param cookie The cookie of the process that lost an instance.
 * @return None.
 *
 * NOTE: This function calls pe_proc_untrack which will drop the reference
 *     to the process structure that was obtained by pe_proc_track.
 */
void pe_proc_exit(anoubis_cookie_t cookie)
{
	struct pe_proc	*proc;

	proc = pe_proc_get(cookie);
	if (!proc)
		return;
	if (--proc->instances) {
		pe_proc_put(proc);
		return;
	}
	pe_proc_untrack(proc);
	pe_upgrade_end(proc);
	DEBUG(DBG_PE_PROC, "pe_proc_exit: token 0x%08" PRIx64 " pid %d "
	    "uid %u proc %p", cookie, proc->pid, proc->uid,
	    proc);
	pe_proc_put(proc);
}

/**
 * Return true if the process is in fact running. A process is running if
 * it is still in the task tracking and has at least one thread assigned to
 * it.
 *
 * @param The cookie of the process to check.
 * @return True if the process is still running.
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

/**
 * Add a new (kernel based) thread to the process with the given
 * task cookie. This is called for threads, even for the master.
 * A process is alive as long as at least one of its threads is still
 * alive.
 *
 * @param cookie The task cookie.
 * @return None.
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

/**
 * Remove one thread that has exited or left the thread group from the
 * process. If the last thread of a process exits, an upgrade ends even
 * if the credentials of the process have been inherited by a file handle.
 *
 * @param cookie The task cookie of the terminating thread.
 * @return None.
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

/**
 * Update process attributes in the task tracking after an exec system call
 * according to the parameters.
 *
 * @param cookie The task cookie of the affected process.
 * @param uid The new user-ID of the process.
 * @param pid The process ID of the new process.
 * @param csum The checksum of the binary that is executed by the process.
 * @param pathhint The path of the binary that is executed by the process.
 * @param pgid The playground-ID of the new process.
 * @param secure True if the process performed a secure exec.
 * @return None.
 */
void pe_proc_exec(anoubis_cookie_t cookie, uid_t uid, pid_t pid,
    const struct abuf_buffer csum, const char *pathhint,
    anoubis_cookie_t pgid, int secure)
{
	struct pe_proc		*proc = pe_proc_get(cookie);
	struct pe_context	*ctx0, *ctx1;

	if (proc == NULL) {
		/* Previously untracked process. Track it. */
		DEBUG(DBG_PE_PROC, "pe_proc_exec: untracked "
		    "process %u 0x%08" PRIx64 " execs", pid,
		    cookie);
		proc = pe_proc_alloc(uid, cookie, NULL, pgid);
		pe_proc_track(proc);
	}
	/* fill in checksum and pathhint */
	pe_proc_ident_set(&proc->ident, csum, pathhint);
	pe_proc_set_pid(proc, pid);
	pe_context_exec(proc, uid, &proc->ident);
	if (secure) {
		pe_proc_set_flag(proc, PE_PROC_FLAGS_SECUREEXEC);
	} else {
		int i;
		pe_proc_clr_flag(proc, PE_PROC_FLAGS_SECUREEXEC);
		for (i = 0; i < PE_PRIO_MAX; i++) {
			if (pe_context_is_nosfs(pe_proc_get_context(proc, i))) {
				log_warnx("%s did not do a secure exec, "
				    "NOSFS flag will be ignored.", pathhint);
				break;
			}
		}
	}

	ctx0 = pe_proc_get_context(proc, 0);
	ctx1 = pe_proc_get_context(proc, 1);
	/* Get our policy */
	DEBUG(DBG_PE_PROC, "pe_proc_exec: using alfrules %p %p sbrules %p %p "
	    "for %s",
	    pe_context_get_alfrule(ctx0), pe_context_get_alfrule(ctx1),
	    pe_context_get_sbrule(ctx0), pe_context_get_sbrule(ctx1),
	    pathhint ? pathhint : "");

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
	pe_proc_put(proc);
}

/**
 * Return the flags that should be added to the return value in an
 * exec system call. This function checks if the process must be
 * forced into a playground and if the process must do a secure exec.
 *
 * @param cookie The task cookie of the process.
 * @param uid The user-ID of the new process.
 * @param csum The checksum of the new binary.
 * @param pathhint The path of the new binary.
 * @return None.
 */
int
pe_proc_flag_transition(anoubis_cookie_t cookie, uid_t uid,
    const struct abuf_buffer csum, const char *pathhint)
{
	struct pe_proc		*proc = pe_proc_get(cookie);
	struct pe_proc_ident	 pident = { ABUF_EMPTY, NULL };
	int			 ret = 0;

	if (!proc)
		return 0;
	pe_proc_ident_set(&pident, csum, pathhint);
#ifdef ANOUBIS_RET_NEED_SECUREEXEC
	if (pe_context_will_transition(proc, uid, &pident))
		ret |= ANOUBIS_RET_NEED_SECUREEXEC;
#endif
#ifdef ANOUBIS_RET_NEED_PLAYGROUND
	if (pe_context_will_pg(proc, uid, &pident))
		ret |= ANOUBIS_RET_NEED_PLAYGROUND;
#endif
	pe_proc_ident_put(&pident);
	pe_proc_put(proc);
	return ret;
}

/**
 * Refresh process contexts after a new policy database was loaded.
 * This iterates over all processes and creates new process contexts that
 * reference the newly loaded policies. No context switches happen.
 *
 * @param newpdb The new policy database.
 * @return None.
 */
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

/**
 * Replace all references to a given policy ruleset in all process contexts
 * with pointers to the new policy ruleset. This function must be called
 * after a new policy has been loaded.
 *
 * @param oldrs The old policy ruleset.
 * @param prio The ruleset priority.
 * @param uid The user ID that is affected by the policy load.
 * @return None.
 *
 * NOTE1: This function is not allowed to fail! It must remove all references
 *     to the old ruleset from tracked processes.
 * NOTE: If proc->context[prio] is not NULL then proc->context[prio]->ruleset
 *     cannot be NULL either. oldrs == NULL is allowed, however.
 *     In the latter case there was no policy before and we refresh all
 *     processes with the given uid.
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

/**
 * Save the currently active rules context away before it is replaced
 * with a context obtained via a connect rule.
 *
 * @param proc The process.
 * @param prio The priority of the rules context.
 * @param cookie The cookie of the task that we will borrow our new context
 *     from.
 * @return None.
 */
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

/**
 * Restore the saved context of a process after the connection to the
 * borrowing task is terminated.
 *
 * @param proc The process.
 * @param prio The priority of the affected context.
 * @param cookie The task cookie of the peer.
 * @return None.
 */
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

/**
 * Return the original (saved) context of the process. Don't use this
 * unless you know what you are doing. Use pe_proc_get_context instead.
 *
 * @param proc The process.
 * @param prio The priority of the rules context.
 * @return A pointer to the rules context. No reference to the context is
 *     obtained.
 */
struct pe_context *
pe_proc_get_savedctx(struct pe_proc *proc, int prio)
{
	if (!proc)
		return NULL;
	if (prio < 0 || prio >= PE_PRIO_MAX)
		return NULL;
	return proc->saved_ctx[prio];
}

/**
 * Modify the saved context of a process. This should only be used if the
 * ruleset changes. Do not use unless you know what you are doing.
 *
 * @param proc The process.
 * @param prio The prio of the affected rules context.
 * @param ctx The new rules context. A reference to the context will
 *     be acquired by the process structure.
 * @return None.
 */
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

/**
 * Return non-zero if the process is an upgrade process.
 *
 * @param proc The process.
 * @return Zero if the process is not an upgrade process, non-zero if it is.
 */
unsigned int
pe_proc_is_upgrade(struct pe_proc *proc)
{
	return (pe_proc_get_flag(proc, PE_PROC_FLAGS_UPGRADE));
}

/**
 * Return non-zero if the process is an upgrade parent process.
 *
 * @param proc The process.
 * @return Zero if the process is not an upgrade parent process, non-zero
 *     if it is.
 */
unsigned int
pe_proc_is_upgrade_parent(struct pe_proc *proc)
{
	return (pe_proc_get_flag(proc, PE_PROC_FLAGS_UPGRADE_PARENT));
}

/**
 * Mark the process as a new upgrade process. This makes the process both
 * an upgrade process and an upgrade parent process.
 *
 * @param proc The process.
 * @return None.
 */
void
pe_proc_upgrade_addmark(struct pe_proc *proc)
{
	pe_proc_set_flag(proc, PE_PROC_FLAGS_UPGRADE);
	pe_proc_set_flag(proc, PE_PROC_FLAGS_UPGRADE_PARENT);
}

/**
 * Clear the upgrade mark on a process. This removes all upgrade flags
 * from the process.
 *
 * @param proc The process.
 * @return None.
 */
void
pe_proc_upgrade_clrmark(struct pe_proc *proc)
{
	pe_proc_clr_flag(proc, PE_PROC_FLAGS_UPGRADE);
	pe_proc_clr_flag(proc, PE_PROC_FLAGS_UPGRADE_PARENT);
}

/**
 * Return 1 if a particular flag is set for the process.
 *
 * @param proc The process.
 * @param flag The flag.
 * @return Zero if the flag is not set, one if it is.
 */
static inline unsigned int
pe_proc_get_flag(struct pe_proc *proc, unsigned int flag)
{
	if (!proc)
		return 0;
	return !!(proc->flags & flag);
}

/**
 * Set the given process flags.
 *
 * @param proc The process.
 * @param flag The flag.
 * @return None.
 */
static inline void
pe_proc_set_flag(struct pe_proc *proc, unsigned int flag)
{
	if (proc != NULL) {
		/* Set flag */
		proc->flags |= flag;
	}
}

/**
 * Clear the given process flag.
 *
 * @param proc The process.
 * @param flag The flag.
 * @return None.
 */
static inline void
pe_proc_clr_flag(struct pe_proc *proc, unsigned int flag)
{
	if (proc != NULL) {
		/* Remove flag */
		proc->flags &= ~flag;
	}
}

/**
 * Set the upgrade flag of the process proc to the value given by flag.
 * I.e. the new process will be marked as an upgrade process iff flag is
 * non-zero.
 *
 * @param proc The process.
 * @param The desired value of the flag.
 * @return None.
 */
static inline void
pe_proc_upgrade_inherit(struct pe_proc *proc, unsigned int flag)
{
	if (flag != 0) {
		pe_proc_set_flag(proc, PE_PROC_FLAGS_UPGRADE);
	} else {
		pe_proc_clr_flag(proc, PE_PROC_FLAGS_UPGRADE);
	}
}

/**
 * Set the secure-exec flag of the process to the value given by flag.
 * I.e. the new process will be marked as a secure-exec process iff flag is
 * non-zero.
 *
 * @param proc The process.
 * @param flag The flag.
 * @return None.
 */
static inline void
pe_proc_secure_inherit(struct pe_proc *proc, unsigned int flag)
{
	if (flag) {
		pe_proc_set_flag(proc, PE_PROC_FLAGS_SECUREEXEC);
	} else {
		pe_proc_clr_flag(proc, PE_PROC_FLAGS_SECUREEXEC);
	}
}

/**
 * Clear the upgrade and upgrade parent markers on all processes. This
 * should be called once an upgrade end is detected.
 *
 * @param None.
 * @return None.
 */
void
pe_proc_upgrade_clrallmarks(void)
{
	struct pe_proc	*proc;

	TAILQ_FOREACH(proc, &tracker, entry) {
		pe_proc_upgrade_clrmark(proc);
	}
}

/**
 * Hold back the answer to all events of this process. This can be used
 * if the daemon did not finish processing of an upgrade and the same
 * process starts a new new upgrade.
 *
 * @param proc The process.
 * @return None.
 *
 * NOTE: This function only sets an approriate flag on the process. The
 *     event handling routines must check for this flag and act upon it.
 */
void
pe_proc_hold(struct pe_proc *proc)
{
	pe_proc_set_flag(proc, PE_PROC_FLAGS_HOLD);
}

/**
 * Return true if the process is marked with the hold flag (see pe_proc_hold).
 *
 * @param proc The process.
 * @return Non-zero if the hold flag is set, zero if it isn't.
 */
int
pe_proc_is_hold(struct pe_proc *proc)
{
	return pe_proc_get_flag(proc, PE_PROC_FLAGS_HOLD);
}

/**
 * Clear the hold flags on all processes and mark them as upgrade start
 * processes.
 *
 * @param None.
 * @return None.
 */
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

/**
 * Return true if the process did a secure exec.
 *
 * @param proc The process.
 * @return True if the process did a secure exec, false if it didn't.
 */
int
pe_proc_is_secure(struct pe_proc *proc)
{
	if (!proc)
		return 0;
	return pe_proc_get_flag(proc, PE_PROC_FLAGS_SECUREEXEC);
}

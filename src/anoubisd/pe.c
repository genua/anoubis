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
#include <sys/stat.h>
#include <limits.h>

#include <apn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>


#ifdef LINUX
#include <bsdcompat.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis_sfs.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis_alf.h>
#include <dev/anoubis_sfs.h>
#include <dev/anoubis.h>
#include <bsdcompat/dev/anoubis_playground.h>
#endif

#include <sys/queue.h>

#include <anoubis_protocol.h>
#include "anoubisd.h"
#include "pe.h"
#include "pe_filetree.h"
#include "sfs.h"
#include "cert.h"

/* Prototypes */
static struct anoubisd_reply	*pe_dispatch_event(struct eventdev_hdr *);
static struct anoubisd_reply	*pe_handle_process(struct eventdev_hdr *);
static struct anoubisd_reply	*pe_handle_sfsexec(struct eventdev_hdr *);
static struct anoubisd_reply	*pe_handle_alf(struct eventdev_hdr *);
static struct anoubisd_reply	*pe_handle_ipc(struct eventdev_hdr *);
static struct anoubisd_reply	*pe_handle_sfs(struct eventdev_hdr *);
static struct anoubisd_reply	*pe_handle_playgroundask(struct eventdev_hdr *);
static struct anoubisd_reply	*pe_handle_playgroundproc(
				     struct eventdev_hdr *);
static struct anoubisd_reply	*pe_handle_playgroundfile(
				     struct eventdev_hdr *);
static struct pe_file_event	*pe_parse_file_event(struct eventdev_hdr *hdr);
static int			 pe_compare(struct pe_proc *proc,
				     struct pe_path_event *event, time_t now);

#ifdef ANOUBIS_SOURCE_SFSPATH
static struct anoubisd_reply	*pe_handle_sfspath(struct eventdev_hdr *);
static struct pe_path_event	*pe_parse_path_event(struct eventdev_hdr *hdr);
#endif

/**
 * A file tree that stores all files that have at least one checksum and
 * were modified by the upgrade that is currently in progress.
 */
static struct pe_file_tree	*upgrade_tree = NULL;

/**
 * The total number of running upgrade parent processes, i.e. processes
 * that idependently lock the configured upgrade file. The upgrade is
 * considered to be in progress until the last of these processes
 * terminates its upgrade.
 */
static int			 upgrade_counter = 0;

/**
 * An iterator that points into the upgrade_tree. Requests from the
 * upgrade process for files that need checksum updates use this iterator.
 */
static struct pe_filetree_node	*upgrade_iterator = NULL;

/**
 * This variable is true if it is currently ok to start a new upgrade.
 * It is controlled by the master process via upgrade message of type
 * ANOUBISD_UPGRADE_OK and depends on configuration settings and internal
 * state in the master.
 */
static int			 upgrade_ok = 1;

/**
 * This file descriptor is used to open and lock the sfs version file
 * during an upgrade.  anoubisctl uses this to delay daemon restart
 * until the upgrade is over.
 */
static int			 sfsversionfd = -1;

/**
 * Initialize the policy engine.
 */
void
pe_init(void)
{
	sfshash_init();
	pe_proc_init();
	cert_init(1);
	pe_user_init();
}

/**
 * Shutdown the policy engine and free allocated memory.
 */
void
pe_shutdown(void)
{
	pe_user_flush_db(NULL);
	sfshash_flush();
	pe_proc_shutdown();
	cert_flush();
	pe_playground_shutdown();
}

/**
 * Reconfigure the policy engine. This is called in response to a
 * HUP signal. It reloads the policy and certificate database and
 * deletes all entries from the sfs hash.
 */
void
pe_reconfigure(void)
{
	sfshash_flush();
	cert_reconfigure(1);
	pe_user_reconfigure();
}

/**
 * Set the ugprade_ok variable the the given value.
 *
 * @param value The new value.
 * @return None.
 */
void
pe_set_upgrade_ok(int value)
{
	upgrade_ok = value;
}

/**
 * Try to start a new upgrade with the given process as the upgrade
 * parent. This function marks the current process as an upgrade parent,
 * initializes the upgrade file tree and locks the sfs version file
 * using sfsversionfd.
 *
 * Child processes will inherit the upgrade but not the upgrade parent
 * mark.
 *
 * If an old upgrade is complete but not all checksums have been updated
 * the task is put on hold and cannot start an upgrade right now.
 *
 * @param proc The new upgrade parent.
 * @return None.
 */
void
pe_upgrade_start(struct pe_proc *proc)
{
	/* Nothing to do if the process is already an upgrader. */
	if (pe_proc_is_upgrade_parent(proc))
		return;
	/* Initialize the tree if we currently have none. */
	if (upgrade_tree == NULL) {
		upgrade_tree = pe_filetree_create();
		if (upgrade_tree == NULL)
			return;
	} else if (upgrade_counter == 0) {
		pe_proc_hold(proc);
		DEBUG(DBG_UPGRADE, "Need to block upgrade start. "
		    "Old upgrade still in progress");
		return;
	}
	/* Mark the process as an upgrade parent and increase the counter. */
	pe_proc_upgrade_addmark(proc);
	if (sfsversionfd < 0) {
		sfsversionfd = open(ANOUBISD_SFS_TREE_VERSIONFILE_CHROOT,
		    O_RDONLY);
		if (sfsversionfd < 0) {
			log_warn("Cannot open "
			    ANOUBISD_SFS_TREE_VERSIONFILE_CHROOT);
		} else if (flock(sfsversionfd, LOCK_EX|LOCK_NB) < 0) {
			log_warn("Cannot flock "
			    ANOUBISD_SFS_TREE_VERSIONFILE_CHROOT);
			close(sfsversionfd);
			sfsversionfd = -1;
		}
	}
	upgrade_counter++;
	DEBUG(DBG_UPGRADE, "Upgrade started on task %" PRId64
	    ", counter now %d", pe_proc_task_cookie(proc), upgrade_counter);
}

/**
 * Remove the ugprade mark from a process. If this is not an upgrade
 * parent, or it is not the last upgrade partent to terminate its upgrade,
 * this is all that happens.
 *
 * If the upgrade actually ends now, the upgrade file tree is pruned of all
 * files that have been modified by processes that live longer than the
 * upgrade lasts and all remaining upgrade marks on upgrade child processes
 * are removed.
 *
 * Finally, the master is notified of the upgrade end. The upgrade process
 * will now start to request chunks of upgraded files' names and adjust
 * checksums and signatures as configured.
 *
 * @param proc The process that just ended its upgrade.
 * @return None.
 */
void
pe_upgrade_end(struct pe_proc *proc)
{
	struct pe_filetree_node	*it, *next;
	anoubis_cookie_t	 last_cookie = 0;
	int			 isparent = pe_proc_is_upgrade_parent(proc);

	/*
	 * Clear the upgrade mark. If the process is not an upgrade parent
	 * we are done.
	 */
	pe_proc_upgrade_clrmark(proc);
	if (!isparent)
		return;
	DEBUG(DBG_UPGRADE, "Upgrade end for task %" PRId64 ", counter now %d",
	    pe_proc_task_cookie(proc), upgrade_counter-1);
	/* Reduce the number of active upgrade parents. */
	if (--upgrade_counter > 0)
		return;
	/*
	 * If this was the last upgrade parent prune the tree. Files
	 * modified by an upgrade parent (it->task_cookie == 0) are kept
	 * unconditionally. Files modified by an upgrade child are kept if
	 * that child is no longer alive.
	 */
	for (it=pe_filetree_start(upgrade_tree); it; it = next) {
		next = pe_filetree_next(upgrade_tree, it);
		DEBUG(DBG_UPGRADE, "Upgraded file: %s", it->path);
		if (it->task_cookie == 0 || it->task_cookie == last_cookie)
			continue;
		if (pe_proc_is_running(it->task_cookie)) {
			DEBUG(DBG_UPGRADE, "Upgraded file: %s removed",
			    it->path);
			pe_filetree_remove(upgrade_tree, it);
		} else {
			last_cookie = it->task_cookie;
		}
	}
	pe_proc_upgrade_clrallmarks();
	upgrade_iterator = NULL;

	/* Trigger actual end of upgrade handling, i.e. checksum update. */
	send_upgrade_start();
}

/**
 * Initialize the upgrade iterator and let it point to the first
 * element of the upgrade file tree.
 */
void
pe_upgrade_filelist_start(void)
{
	if (!upgrade_tree || upgrade_counter) {
		upgrade_iterator = NULL;
	} else {
		upgrade_iterator = pe_filetree_start(upgrade_tree);
	}
}

/**
 * Advance the upgrade file list iterator to the next iterm in the
 * upgrade file tree.
 */
void
pe_upgrade_filelist_next(void)
{
	if (!upgrade_tree || upgrade_counter || !upgrade_iterator) {
		upgrade_iterator = NULL;
	} else {
		upgrade_iterator = pe_filetree_next(upgrade_tree,
		    upgrade_iterator);
	}
}

/**
 * Return a pointer to the current pe_filetree_node in the upgrade file tree.
 * The current node is determined by the upgrade file list iterator.
 *
 * @return A pointer to the current node. Must not be modified by the
 *     caller.
 */
struct pe_filetree_node *
pe_upgrade_filelist_get(void)
{
	if (!upgrade_tree || upgrade_counter)
		upgrade_iterator = NULL;
	return upgrade_iterator;
}

/**
 * This finishes an upgrade after all checksums have been updated.
 * It flushes the sfs checksum cache, frees the upgrade file list tree
 * and closes the sfsversionfd descriptor. However, it does not
 * release processes that have been put on hold. This must be done by
 * the caller.
 */
void
pe_upgrade_finish(void)
{
	if (upgrade_tree) {
		pe_filetree_destroy(upgrade_tree);
		upgrade_tree = NULL;
	}
	/*
	 * Checksums might have changed and the master does not send checksum
	 * updates for these files.
	 */
	sfshash_flush();
	upgrade_iterator = NULL;
	if (sfsversionfd >= 0) {
		/* Close releases the flock. */
		close(sfsversionfd);
		sfsversionfd = -1;
	}
}

/**
 * This is the main external entry point for the policy engine. All
 * kernel events that require a policy decision or other types of
 * attention from the policy engine go through this function.
 *
 * NOTE: Basic length verification has been done by amsg_verify.
 *
 * @param request The request message from the master (must be of type
 *     ANOUBISD_MSG_EVENTDEV).
 * @return An anoubisd_reply structure that describes the action to
 *     take for this event. The reply structure is allocated by this
 *     function and must be freed by the caller. The pointer fields in
 *     this structure point to memory owned by the current process and
 *     not by the anobuisd_reply structure. The caller should copy this
 *     data immediately if required.
 */
struct anoubisd_reply *
policy_engine(struct anoubisd_msg *request)
{
	struct anoubisd_reply	*reply;
	struct eventdev_hdr	*hdr;

	DEBUG(DBG_TRACE, ">policy_engine");

	if (request->mtype != ANOUBISD_MSG_EVENTDEV) {
		log_warnx(" policy_engine: Bad policy request type %d",
		    request->mtype);
		return NULL;
	}
	hdr = (struct eventdev_hdr *)request->msg;
	reply = pe_dispatch_event(hdr);

	DEBUG(DBG_TRACE, "<policy_engine");

	return reply;
}

/**
 * This is the internal function to handle the different types of kernel
 * events. It is used by policy_engine and dispatches different types
 * of kernel events to different dispatcher functions.
 *
 * @param hdr The eventdev header of the kernel event (followed by the
 *     event payload data.
 * @return An anoubisd_reply structure that tells the caller how to proceed
 *     with the event. The caller must make sure that the memory of this
 *     structure is freed.
 */
static struct anoubisd_reply *
pe_dispatch_event(struct eventdev_hdr *hdr)
{
	struct anoubisd_reply	*reply = NULL;

	DEBUG(DBG_PE, "pe_dispatch_event: pid %u uid %u token %x %d",
	    hdr->msg_pid, hdr->msg_uid, hdr->msg_token, hdr->msg_source);

	if (hdr == NULL) {
		log_warnx("pe_dispatch_event: empty header");
		return (NULL);
	}

	switch (hdr->msg_source) {
	case ANOUBIS_SOURCE_PROCESS:
		reply = pe_handle_process(hdr);
		break;

	case ANOUBIS_SOURCE_SFSEXEC:
		reply = pe_handle_sfsexec(hdr);
		break;

	case ANOUBIS_SOURCE_ALF:
		reply = pe_handle_alf(hdr);
		break;

	case ANOUBIS_SOURCE_IPC:
		reply = pe_handle_ipc(hdr);
		break;

	case ANOUBIS_SOURCE_SFS:
		reply = pe_handle_sfs(hdr);
		break;

#ifdef ANOUBIS_SOURCE_SFSPATH
	case ANOUBIS_SOURCE_SFSPATH:
		reply = pe_handle_sfspath(hdr);
		break;
#endif

	case ANOUBIS_SOURCE_PLAYGROUND:
		reply = pe_handle_playgroundask(hdr);
		break;
	case ANOUBIS_SOURCE_PLAYGROUNDPROC:
		reply = pe_handle_playgroundproc(hdr);
		break;
	case ANOUBIS_SOURCE_PLAYGROUNDFILE:
		reply = pe_handle_playgroundfile(hdr);
		break;

	default:
		log_warnx("pe_dispatch_event: unknown message source %d",
		    hdr->msg_source);
		break;
	}

	return (reply);
}

/**
 * Handle kernel events of type ANOUBIS_SOURCE_SFSEXEC. These events
 * are used to notify the anoubisd daemon of an exec. This function
 * extracs the new path name and checksum from the event and tells
 * process tracking about the exec.
 *
 * @param hdr The eventdev header of the event.
 * @return Always NULL. This type of events does not need a reply.
 */
static struct anoubisd_reply *
pe_handle_sfsexec(struct eventdev_hdr *hdr)
{
	struct sfs_open_message		*msg;
	struct abuf_buffer		 csum = ABUF_EMPTY;

	if (hdr == NULL) {
		log_warnx("pe_handle_sfsexec: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct sfs_open_message))) {
		log_warnx("pe_handle_sfsexec: short message");
		return (NULL);
	}
	msg = (struct sfs_open_message *)(hdr+1);
	if (msg->flags & ANOUBIS_OPEN_FLAG_CSUM)
		csum = abuf_open_frommem(msg->csum, sizeof(msg->csum));
	pe_proc_exec(msg->common.task_cookie,
	    hdr->msg_uid, hdr->msg_pid, csum,
	    (msg->flags & ANOUBIS_OPEN_FLAG_PATHHINT) ? msg->pathhint : NULL,
	    extract_pgid(&msg->common),
	    (msg->flags & ANOUBIS_OPEN_FLAG_SECUREEXEC));

	return (NULL);
}

/**
 * Handle kernel events of type ANOUBIS_SOURCE_PROCESS. The precise
 * action depends on the subtype of the event. Possible event types
 * depend on the operating system. Linux has some additional events,
 * as it in fact tracks credentials with cookies instead of processes:
 * ANOUBIS_PROCESS_OP_FORK: The tracked structure (process or credentials)
 *     was cloned (with an new task cookie). This is where inheritance
 *     of policies and contexts happens.
 * ANOUBIS_PROCESS_OP_EXIT: A creditials or process structure was freed.
 * ANOUBIS_PROCESS_OP_REPLACE: The credentials of a task have been replaced.
 *     In this process the cookie of the new credentials was replaced with
 *     the cookie of the victim. This means that cookie of the victim
 *     gains an instance while the (old) cookie of the new credentials
 *     loses an instance.
 * ANOUBIS_PROCESS_OP_CREATE: Credentials have been assigned to a process.
 *     The cookie in the credentials gains a thread.
 * ANOUBIS_PROCESS_OP_DESTROY: A thread with the given credentials terminated.
 *     The cookie of the cedentials loses a thread.
 * This function extracts the neccessary information from the kernel
 * events and forwards it to process tracking.
 *
 * @param hdr The event.
 * @return Always NULL. No further action required.
 */
static struct anoubisd_reply *
pe_handle_process(struct eventdev_hdr *hdr)
{
	struct ac_process_message	*msg;

	if (hdr == NULL) {
		log_warnx("pe_handle_process: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct ac_process_message))) {
		log_warnx("pe_handle_process: short message");
		return (NULL);
	}
	msg = (struct ac_process_message *)(hdr + 1);

	switch (msg->op) {
	case ANOUBIS_PROCESS_OP_FORK:
		/* Use cookie of new process */
		pe_proc_fork(hdr->msg_uid, msg->task_cookie,
		    msg->common.task_cookie, extract_pgid(&msg->common));
		break;
	case ANOUBIS_PROCESS_OP_EXIT:
		/* NOTE: Do NOT use msg->common.task_cookie here! */
		pe_proc_exit(msg->task_cookie);
		break;
#ifdef ANOUBIS_PROCESS_OP_REPLACE
	case ANOUBIS_PROCESS_OP_REPLACE:
		pe_proc_addinstance(msg->common.task_cookie);
		pe_proc_exit(msg->task_cookie);
		break;
#endif
#ifdef ANOUBIS_PROCESS_OP_CREATE
	case ANOUBIS_PROCESS_OP_CREATE:
		pe_proc_add_thread(msg->task_cookie);
		break;
	case ANOUBIS_PROCESS_OP_DESTROY:
		pe_proc_remove_thread(msg->task_cookie);
		break;
#endif
	default:
		log_warnx("pe_handle_process: undefined operation %ld",
		    msg->op);
		break;
	}
	return (NULL);
}

/**
 * Process an ALF escalation, i.e. a message of type ANOUBIS_SOURCE_ALF.
 * This functions finds the process responsible for the event (provided
 * that it is tracked) and calls pe_decide_alf to evaluate the policy.
 *
 * @param hdr The event.
 * @return The reply for the event.
 */
static struct anoubisd_reply *
pe_handle_alf(struct eventdev_hdr *hdr)
{
	struct alf_event	*msg;
	struct anoubisd_reply	*reply = NULL;
	struct pe_proc		*proc;

	if (hdr == NULL) {
		log_warnx("pe_handle_alf: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct alf_event))) {
		log_warnx("pe_handle_alf: short message");
		return (NULL);
	}
	msg = (struct alf_event *)(hdr + 1);

	/* get process from tracker list */
	if ((proc = pe_proc_get(msg->common.task_cookie)) == NULL) {
		/*
		 * Untracked process: Do not insert it here because
		 * we do not have proper csum/path data for this process.
		 */
		DEBUG(DBG_PE_ALF, "pe_handle_alf: untrackted process %u",
		    hdr->msg_pid);
	} else {
		if (pe_proc_get_pid(proc) == -1)
			pe_proc_set_pid(proc, hdr->msg_pid);
	}
	reply = pe_decide_alf(proc, hdr);
	pe_proc_put(proc);
	DEBUG(DBG_TRACE, "<policy_engine");
	return (reply);
}

/**
 * Handle events of type ANOUBIS_SOURCE_IPC. These are tracked by the
 * pe_ipc.c file and might generate context switches due to borrow rules.
 *
 * @param hdr The event.
 * @return Always NULL, no further action required.
 */
static struct anoubisd_reply *
pe_handle_ipc(struct eventdev_hdr *hdr)
{
	struct ac_ipc_message	*msg;

	if (hdr == NULL) {
		log_warnx("pe_handle_ipc: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct ac_ipc_message))) {
		log_warnx("pe_handle_ipc: short message");
		return (NULL);
	}
	msg = (struct ac_ipc_message *)(hdr + 1);

	switch (msg->op) {
	case ANOUBIS_SOCKET_OP_CONNECT:
		pe_ipc_connect(msg);
		break;
	case ANOUBIS_SOCKET_OP_DESTROY:
		pe_ipc_destroy(msg);
		break;
	default:
		log_warnx("pe_handle_ipc: undefined operation %d", msg->op);
		break;
	}

	return (NULL);
}

/**
 * Merge two replies one from an SFS rule and the other from a sandbox
 * rule for the same event into one. This makes sure that the user sees
 * only one escalation per file system access.
 *
 * Both replies must be malloced and will (conceptually) be freed.
 * The return value is again malloced and must be freed by the caller.
 *
 * Merge strategie:
 * - Either the sandbox or the sfs reply is used.
 * - If one reply is NULL, i.e. the module has no oppinon on the event,
 *   the other reply is used.
 * - If one reply out right denies the event, it is used.
 * - Otherwise if one event has the ask flag set it is used.
 * - If there is still a tie the SFS event is preferred.
 *
 * @param hdr The event in question. This is used to reset the event
 *     source that might have changed from SFS to SANDBOX and back.
 * @param sfs The SFS reply.
 * @param sb The sandbox reply.
 * @return The merged reply.
 */
static struct anoubisd_reply *
reply_merge(struct eventdev_hdr *hdr, struct anoubisd_reply *sfs,
    struct anoubisd_reply *sb)
{
	if (!sfs)
		goto use_sb;
	if (!sb)
		goto use_sfs;
	if (sfs->ask == 0 && sfs->reply)
		goto use_sfs;
	if (sb->ask == 0 && sb->reply)
		goto use_sb;
	if (sfs->ask)
		goto use_sfs;
	if (sb->ask)
		goto use_sb;
use_sfs:
	if (sb)
		free(sb);
	/*
	 * Due to logging rules this might have changed from SFS to SANDBOX,
	 * "You'd better change it back or we will both be sorry."
	 */
	hdr->msg_source = ANOUBIS_SOURCE_SFS;
	return sfs;
use_sb:
	if (sfs)
		free(sfs);
	hdr->msg_source = ANOUBIS_SOURCE_SANDBOX;
	return sb;
}

/**
 * Handle an event of type ANOUBIS_SOURCE_SFS. This type of kernel event
 * is used to implement both SFS and sandbox policies. Thus this function
 * passes the event to both the sfs and the sandbox policies and merges
 * the reply. Additionally, it is used to handle context switches
 * due to a context open rule and to track files modified by an upgrade.
 *
 * @param hdr The event.
 * @return The merged reply from both SFS and sandbox rules.
 */
static struct anoubisd_reply *
pe_handle_sfs(struct eventdev_hdr *hdr)
{
	struct anoubisd_reply		*reply = NULL, *reply2 = NULL;
	struct pe_file_event		*fevent;
	struct pe_proc			*proc;

	DEBUG(DBG_TRACE, ">pe_handle_sfs");
	if (hdr == NULL) {
		log_warnx("pe_handle_sfs: empty message");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct sfs_open_message))) {
		log_warnx("pe_handle_sfs: short message");
		return (NULL);
	}
	if ((fevent = pe_parse_file_event(hdr)) == NULL) {
		log_warnx("Cannot parse sfsopen message");
		return NULL;
	}
	proc = pe_proc_get(fevent->cookie);
	if (proc && pe_proc_get_pid(proc) == -1)
		pe_proc_set_pid(proc, hdr->msg_pid);

	pe_context_open(proc, hdr);
	if (pe_proc_is_upgrade(proc) && (fevent->amask & APN_SBA_WRITE)) {
		if (fevent->path && sfs_haschecksum_chroot(fevent->path) > 0) {
			anoubis_cookie_t	cookie = fevent->cookie;

			/*
			 * Use the special value 0 for the cookie if the
			 * modifying process is an upgrade parent. This
			 * makes sure that modifications of an upgrade parent
			 * are not pruned even if the upgrade parent still
			 * lives at the end of the upgrade.
			 */
			if (pe_proc_is_upgrade_parent(proc))
				cookie = 0;
			pe_filetree_insert(upgrade_tree, fevent->path, cookie);
			fevent->upgrade_flags |= PE_UPGRADE_TOUCHED;
		}
	} else {
		if (fevent->path
		    && pe_filetree_find(upgrade_tree, fevent->path))
			fevent->upgrade_flags |= PE_UPGRADE_TOUCHED;
	}
	if ((fevent->upgrade_flags & PE_UPGRADE_TOUCHED)
	    && pe_proc_is_upgrade(proc))
		fevent->upgrade_flags |= PE_UPGRADE_WRITEOK;

	reply = pe_decide_sfs(proc, fevent);
	reply2 = pe_decide_sandbox(proc, fevent);

	/* XXX CEH: This might need more thought. */
	reply = reply_merge(hdr, reply, reply2);

	/* Set exec flags if required. */
#ifdef ANOUBIS_RET_NEED_SECUREEXEC
	/*
	 * Starting with ANOUBISCORE_LOCK_VERSION the kernel understands
	 * flags in a reply. Only set exec flags if the reply is
	 * zero and the permissions include exec.
	 */
	if (fevent->amask & APN_SBA_EXEC && reply->reply == 0
	    && version >= ANOUBISCORE_LOCK_VERSION) {
		reply->reply = pe_proc_flag_transition(proc, fevent);
	}
#endif
	pe_proc_put(proc);

	if ((fevent->path) && (version >= ANOUBISCORE_LOCK_VERSION)) {
		anoubisd_upgrade_mode mode;
		struct anoubisd_upgrade_trigger_list *trigger_list;
		struct anoubisd_upgrade_trigger *trigger;

		mode = anoubisd_config.upgrade_mode;
		trigger_list = &anoubisd_config.upgrade_trigger;

		if (fevent->uid == 0 && reply->reply == 0 &&
		    (mode == ANOUBISD_UPGRADE_MODE_STRICT_LOCK ||
		     mode == ANOUBISD_UPGRADE_MODE_LOOSE_LOCK)) {

			LIST_FOREACH(trigger, trigger_list, entries) {
				if (strcmp(fevent->path, trigger->arg) == 0) {
					DEBUG(DBG_UPGRADE,
					    "Enabling lockwatch on %s",
					    fevent->path);
					reply->reply |=
					    ANOUBIS_RET_OPEN_LOCKWATCH;
					break;
				}
			}
		}
	}

	if (fevent->path)
		free(fevent->path);
	abuf_free(fevent->csum);
	free(fevent);

	DEBUG(DBG_TRACE, "<pe_handle_sfs");
	return(reply);
}

#ifdef ANOUBIS_SOURCE_SFSPATH

/**
 * Handle an event of type ANOUBIS_SOURCE_SFSPATH. This handles two
 * types of event:
 * - Renames and hardlinks are denied if they would move a file such that
 *   different path prefixes mentioned in the active rules would apply
 *   before and after the move.
 * - Additionally, this function tracks lock and unlock events for the
 *   upgrade handling.
 *
 * @param hdr The kernel event containing an operation and one or two
 *     path names.
 * @return An anoubis reply for the event.
 */
static struct anoubisd_reply *
pe_handle_sfspath(struct eventdev_hdr *hdr)
{
	struct anoubisd_reply		*reply = NULL;
	struct pe_path_event		*pevent;
	struct pe_proc			*proc;
	time_t				now;
	anoubisd_upgrade_mode		upgrade_mode;

	if (time(&now) == (time_t)-1) {
		log_warnx("pe_handle_sfspath: Cannot get current time");
		return (NULL);
	}

	if (hdr == NULL) {
		log_warnx("pe_handle_sfspath: empty message");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct sfs_path_message))) {
		log_warnx("pe_handle_sfspath: short message");
		return (NULL);
	}

	reply = malloc(sizeof(struct anoubisd_reply));
	if (reply == NULL)
		return(NULL);

	if ((pevent = pe_parse_path_event(hdr)) == NULL) {
		log_warnx("pe_handle_sfspath: cannot parse sfspath message");
		free(reply);
		return NULL;
	}

	reply->reply = 0;
	reply->ask = 0;
	reply->hold = 0;
	reply->rule_id = 0;
	reply->prio = 0;
	reply->timeout = (time_t)0;
	reply->sfsmatch = ANOUBIS_SFS_NONE;

	proc = pe_proc_get(pevent->cookie);
	if (proc && pe_proc_get_pid(proc) == -1)
		pe_proc_set_pid(proc, hdr->msg_pid);

	switch (pevent->op) {
		case ANOUBIS_PATH_OP_LINK:
		case ANOUBIS_PATH_OP_RENAME:
			reply->reply = -pe_compare(proc, pevent, now);
			break;
		case ANOUBIS_PATH_OP_LOCK:
			DEBUG(DBG_UPGRADE, "Lock event for task cookie %" PRIx64
			    " path %s", pe_proc_task_cookie(proc),
			    pevent->path[0]);

			upgrade_mode = anoubisd_config.upgrade_mode;
			if (hdr->msg_uid == 0 &&
			    (upgrade_mode == ANOUBISD_UPGRADE_MODE_STRICT_LOCK
			    || upgrade_mode ==
			    ANOUBISD_UPGRADE_MODE_LOOSE_LOCK)) {
				if (upgrade_ok || upgrade_counter) {
					pe_upgrade_start(proc);
				} else {
					log_warnx("Upgrade denied due to "
					    "missing root key");
					reply->reply = EPERM;
				}
			}
			break;
		case ANOUBIS_PATH_OP_UNLOCK:
			DEBUG(DBG_UPGRADE, "Unlock event for task cookie %"
			    PRIx64, pe_proc_task_cookie(proc));

			upgrade_mode = anoubisd_config.upgrade_mode;
			if (hdr->msg_uid == 0 &&
			    upgrade_mode == ANOUBISD_UPGRADE_MODE_STRICT_LOCK)
				pe_upgrade_end(proc);
			break;
	}
	if (pe_proc_is_hold(proc))
		reply->hold = 1;
	pe_proc_put(proc);
	free(pevent->path[0]);
	if (pevent->path[1])
		free(pevent->path[1]);
	free(pevent);

	return reply;
}
#endif

/**
 * Handle playground ask events. These events are generated if a
 * playground request access to the special file or tries to rename
 * a directory and this access cannot be done within the playground.
 *
 * @param hdr The eventdev header of the kernel event. Any playground
 *     specific payload data follows.
 * @return An anoubis_reply structure. Currently, we ask the user for
 *     permission unconditionally.
 */
static struct anoubisd_reply *
pe_handle_playgroundask(struct eventdev_hdr *hdr)
{

	struct anoubisd_reply		*reply = NULL;
	struct pg_open_message		*pgevent;
	struct pe_proc			*proc;

	if (hdr == NULL) {
		log_warnx("pe_handle_playgroundask: empty message");
		return NULL;
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct pg_open_message))) {
		log_warnx("pe_handle_playgroundask: short message");
		return NULL;
	}
	pgevent = (struct pg_open_message *)(hdr+1);

	reply = malloc(sizeof(struct anoubisd_reply));
	if (reply == NULL)
		return NULL;
	proc = pe_proc_get(pgevent->common.task_cookie);
	if (proc && pe_proc_get_pid(proc) == -1)
		pe_proc_set_pid(proc, hdr->msg_pid);

	reply->hold = 0;
	reply->log = APN_LOG_NONE; /* XXX CEH: This needs fixing. */
	reply->rule_id = 0;
	reply->prio = 1;
	reply->sfsmatch = ANOUBIS_SFS_NONE;
	reply->reply = 0;

	if (!proc) {
		reply->reply = EPERM;
		reply->ask = 0;
		reply->timeout = 0;
		return reply;
	}
	/*
	 * Suppress open ask events for sockets and fifos. There are too
	 * many of them.
	 */
	if (S_ISSOCK(pgevent->mode) || S_ISFIFO(pgevent->mode)) {
		reply->reply = 0;
		reply->ask = 0;
		reply->timeout = 0;
		pe_proc_put(proc);
		return reply;
	}
	reply->ask = 1;
	reply->timeout = 300;
	reply->pident = pe_proc_ident(proc);
	reply->ctxident = pe_context_get_ident(
	    pe_proc_get_context(proc, reply->prio));
	pe_proc_put(proc);
	return reply;
}

/**
 * Handle playground process events. These events are sent if the playground
 * ID of a process changes.
 *
 * @param hdr The eventdev header with the playground event.
 * @return This function always returns NULL.
 */
static struct anoubisd_reply *
pe_handle_playgroundproc(struct eventdev_hdr *hdr)
{
	struct pg_proc_message	*pg;
	struct pe_proc		*proc;

	if (!hdr || hdr->msg_size < sizeof(struct eventdev_hdr)
	    + sizeof(struct pg_proc_message))
		return  NULL;
	pg = (struct pg_proc_message *)(hdr+1);
	proc = pe_proc_get(pg->common.task_cookie);
	if (proc) {
		int		i;

		pe_proc_set_playgroundid(proc, extract_pgid(&pg->common));
		/* Policy might change due to a modified playground ID. */
		for (i=0; i<PE_PRIO_MAX; ++i)
			pe_context_refresh(proc, i, NULL);
		pe_proc_put(proc);
	}
	return NULL;
}

/**
 * Handle palyground file events. These events are sent if something
 * interesting happens with an inode that has a playground label.
 *
 * @param hdr The eventdev header with the playground event.
 * @return This function always returns NULL.
 */
static struct anoubisd_reply *
pe_handle_playgroundfile(struct eventdev_hdr *hdr)
{
	int				 plen;
	struct pg_file_message		*pg;

	if (!hdr || hdr->msg_size < sizeof(struct eventdev_hdr)
	    + sizeof(struct pg_file_message) + 1)
		return NULL;
	plen = hdr->msg_size - sizeof(struct eventdev_hdr)
	    - sizeof(struct pg_file_message);
	pg = (struct pg_file_message *)(hdr+1);
	/* Force NULL termination, just in case... */
	pg->path[plen-1] = 0;
	DEBUG(DBG_PG, "pe_handle_playgroundfile: Playground file %" PRIx64
	    ":%" PRIx64 ":%s", pg->dev, pg->ino, pg->path);
	switch (pg->op) {
	case ANOUBIS_PGFILE_INSTANTIATE:
		if (pg->path[0]) {
			pe_playground_file_instantiate(pg->pgid, pg->dev,
			    pg->ino, pg->path);
		} else {
			log_warnx("pe_handle_playgroundfile: "
			    "Empty file name length=%d", plen);
		}
		break;
	case ANOUBIS_PGFILE_DELETE:
		pe_playground_file_delete(pg->pgid, pg->dev, pg->ino);
		break;
	case ANOUBIS_PGFILE_SCAN: {
		struct pe_proc	*proc;

		if (pe_playground_file_scanrequest(pg->pgid, pg->dev,
		    pg->ino, pg->path, hdr->msg_uid) < 0) {
			log_warnx("illegal scan request for file %" PRIx64
			    ":%" PRIx64 " (%s) in playground %" PRIx64
			    " by user %d", pg->dev, pg->ino, pg->path,
			    pg->pgid, hdr->msg_uid);
			break;
		}
		proc = pe_proc_get(pg->common.task_cookie);
		log_info("scanning of file %" PRIx64 ":%" PRIx64 " (%s) in "
		    "playground %" PRIx64 " requested by user %d",
		    pg->dev, pg->ino, pg->path, pg->pgid, hdr->msg_uid);
		send_lognotify(proc, hdr, 0 /* error */, APN_LOG_NORMAL,
		    0 /* ruleid */, 0 /* prio */, 0 /* sfsmatch */);
		pe_proc_put(proc);
		break;
	}
	default:
		log_warnx("pe_handle_playgroundfile: Bad operation %d", pg->op);
	}
	return NULL;
}

/**
 * Dump the current state of the policy engine to the log. If this is
 * the system log or stderr depends on the command line options.
 */
void
pe_dump(void)
{
	pe_proc_dump();
	pe_user_dump();
	pe_playground_dump();
}

/**
 * Return true if the given scope includes the process given by the
 * task cookie. For scopes that include a timeout, it is assumed that
 * the current time is given by now.
 *
 * @param scope The scope.
 * @param task The cookie of the task in question.
 * @param now The current time (compared with the timeout in the scope.
 * @return True iff a rule with the given scope must be applied to the
 *     task.
 */
int
pe_in_scope(struct apn_scope *scope, anoubis_cookie_t task,
    time_t now)
{
	if (!scope)
		return 1;
	if (scope->timeout && now > scope->timeout)
		return 0;
	if (scope->task && task != scope->task)
		return 0;
	return 1;
}

/**
 * Analyse a raw kernel event of type ANOUBIS_SOURCE_SFS and store its
 * contents in a dynamically allocated structure of type pe_file_event.
 * The structure is allocated dynamically. The same applies to pointer fields
 * for the path name and the checksum within the structure. However,
 * the memory pointed to the rawhdr field will point back to the original
 * header.
 *
 * @param hdr The kernel event to parse.
 * @return The parsed event. See the description of the pe_file_event
 *     structure for further details.
 */
static struct pe_file_event *
pe_parse_file_event(struct eventdev_hdr *hdr)
{
	struct pe_file_event	*ret = NULL;
	struct sfs_open_message	*kernmsg;
	int			 sfslen, pathlen, i;

	if (!hdr)
		return NULL;
	DEBUG(DBG_TRACE, ">pe_parse_file_event");
	kernmsg = (struct sfs_open_message *)(hdr+1);
	if (hdr->msg_size < sizeof(struct eventdev_hdr))
		return NULL;
	sfslen = hdr->msg_size - sizeof(struct eventdev_hdr);
	if (sfslen < (int)sizeof(struct sfs_open_message))
		return NULL;
	if (kernmsg->flags & ANOUBIS_OPEN_FLAG_PATHHINT) {
		pathlen = sfslen - offsetof(struct sfs_open_message, pathhint);
		for (i=pathlen-1; i >= 0; --i)
			if (kernmsg->pathhint[i] == 0)
				break;
		if (i < 0)
			return NULL;
	} else {
		kernmsg->pathhint[0] = 0;
	}
	ret = malloc(sizeof(struct pe_file_event));
	if (!ret)
		return NULL;
	ret->rawhdr = hdr;
	ret->cookie = kernmsg->common.task_cookie;
	ret->path = NULL;
	ret->csum = ABUF_EMPTY;
	if (kernmsg->pathhint[0]) {
		ret->path = strdup(kernmsg->pathhint);
		if (ret->path == NULL) {
			free(ret);
			return NULL;
		}
	}
	if (kernmsg->flags & ANOUBIS_OPEN_FLAG_CSUM) {
		ret->csum = abuf_alloc(sizeof(kernmsg->csum));
		if (abuf_empty(ret->csum)) {
			free(ret);
			return NULL;
		}
		abuf_copy_tobuf(ret->csum, kernmsg->csum,
		    sizeof(kernmsg->csum));
	}
	ret->amask = 0;
	/* Treat FOLLOW as a read event. */
	if (kernmsg->flags & (ANOUBIS_OPEN_FLAG_READ|ANOUBIS_OPEN_FLAG_FOLLOW))
		ret->amask |= APN_SBA_READ;
	if (kernmsg->flags & ANOUBIS_OPEN_FLAG_WRITE)
		ret->amask |= APN_SBA_WRITE;
	if (kernmsg->flags & ANOUBIS_OPEN_FLAG_EXEC)
		ret->amask |= APN_SBA_EXEC;
	if (ret->amask == 0) {
		log_warnx("OOPS: zero amask for %s, flags is %lx\n",
		    ret->path, kernmsg->flags);
	}
	ret->uid = hdr->msg_uid;
	ret->upgrade_flags = 0;
	DEBUG(DBG_TRACE, "<pe_parse_file_event");
	return ret;
}

#ifdef ANOUBIS_SOURCE_SFSPATH

/**
 * Analyse a raw kernel event of type ANOUBIS_SOURCE_SFSPATH and store its
 * contents in a dynamically allocated structure of type pe_path_event.
 * The structure is allocated dynamically. The same applies to pointer fields
 * for the path names.
 *
 * @param hdr The kernel event to parse.
 * @return The parsed event. See the description of the pe_path_event
 *     structure for further details.
 */
static struct pe_path_event *
pe_parse_path_event(struct eventdev_hdr *hdr)
{
	struct pe_path_event	*ret = NULL;
	struct sfs_path_message	*kernmsg;
	unsigned int		 sfslen;
	const char		*pathptr;

	if (!hdr)
		return NULL;
	kernmsg = (struct sfs_path_message *)(hdr+1);
	if (hdr->msg_size < sizeof(struct eventdev_hdr))
		goto err;
	sfslen = hdr->msg_size - sizeof(struct eventdev_hdr);
	if (sfslen < sizeof(struct sfs_path_message))
		goto err;
	if (sfslen < (sizeof(struct sfs_path_message) +
		kernmsg->pathlen[0] + kernmsg->pathlen[1]))
		goto err;

	/* only support hardlink, rename and lock operations for now */
	switch (kernmsg->op) {
		case ANOUBIS_PATH_OP_LINK:
		case ANOUBIS_PATH_OP_RENAME:
			if (kernmsg->pathlen[1] == 0)
				goto err;
			break;
		case ANOUBIS_PATH_OP_LOCK:
		case ANOUBIS_PATH_OP_UNLOCK:
			if (kernmsg->pathlen[1] != 0)
				goto err;
			break;
		default:
			goto err;
	}
	if (kernmsg->pathlen[0] == 0)
		goto err;

	pathptr = kernmsg->paths;
	if (pathptr[kernmsg->pathlen[0]-1] != 0)
		goto err;
	pathptr += kernmsg->pathlen[0];
	if (kernmsg->pathlen[1] && pathptr[kernmsg->pathlen[1]-1] != 0)
		goto err;

	ret = malloc(sizeof(struct pe_path_event));
	if (!ret)
		return NULL;
	ret->path[0] = ret->path[1] = NULL;
	ret->cookie = kernmsg->common.task_cookie;
	ret->op = kernmsg->op;
	ret->uid = hdr->msg_uid;

	ret->path[0] = strdup(kernmsg->paths);
	if (ret->path[0] == NULL)
		goto err;

	if (kernmsg->pathlen[1]) {
		ret->path[1] = strdup((kernmsg->paths) + kernmsg->pathlen[0]);
		if (ret->path[1] == NULL)
			goto err;
	}
	return ret;

err:
	if (ret) {
		if (ret->path[0])
			free(ret->path[0]);
		if (ret->path[1])
			free(ret->path[1]);

		free(ret);
	}
	return NULL;
}
#endif

/**
 * Analyse the rules in the given rule block and store all path
 * prefixes in a prefix hash. The prefix hash is saved in the userdata
 * field of the rule and will be freed by the ruleset destructor.
 * It is useful to lookup matching rules for a given path name efficiently.
 *
 * @param block The rule block to analyse.
 * @return Zero in case of success, a negative error code if something
 *     went wrong.
 */
int
pe_build_prefixhash(struct apn_rule *block)
{
	int			 cnt = 0, idx = 1;
	struct apn_rule		*rule;

	TAILQ_FOREACH(rule, &block->rule.chain, entry)
		cnt++;
	block->userdata = pe_prefixhash_create(cnt);
	if (!block->userdata)
		return -ENOMEM;
	DEBUG(DBG_TRACE, ">pe_build_prefixhash");
	TAILQ_FOREACH(rule, &block->rule.chain, entry) {
		int		 ret;
		const char	*prefix;
		switch (rule->apn_type) {
		case APN_SFS_ACCESS:
			prefix = rule->rule.sfsaccess.path;
			break;
		case APN_SFS_DEFAULT:
			prefix = rule->rule.sfsdefault.path;
			break;
		case APN_DEFAULT:
			prefix = NULL;
			break;
		case APN_SB_ACCESS:
			prefix = rule->rule.sbaccess.path;
			break;
		default:
			log_warnx("pe_build_prefixhash: Invalid rule "
			    "type %u in SFS rule %lu",
			    rule->apn_type, rule->apn_id);
			pe_prefixhash_destroy(block->userdata);
			block->userdata = NULL;
			DEBUG(DBG_TRACE,
			    "<pe_build_prefixhash: invalid type");
			return -EINVAL;
		}
		ret = pe_prefixhash_add(block->userdata, prefix, rule, idx);
		if (ret < 0) {
			pe_prefixhash_destroy(block->userdata);
			block->userdata = NULL;
			DEBUG(DBG_TRACE, "<pe_build_prefixhash: "
			    "add failed with %d", ret);
			return  ret;
		}
		DEBUG(DBG_TRACE, " pe_build_prefixhash: added %p", rule);
		idx++;
	}
	DEBUG(DBG_TRACE, "<pe_build_prefixhash");
	return 0;
}

/**
 * Look at the path prefixes of all rules in the rulelist. If there is at
 * least one path prefix that applies to exactly one of the two paths in
 * the path event, an error is returned. This is used to check hardlink
 * and rename events.
 *
 * @param rulelist The rules to check.
 * @param pevent The path event containing the two paths that are part of
 *     the hardlink or rename operation.
 * @param now The current time. This is used to check if any the rules are
 *     in scope.
 * @return Zero if the event is allowed, a negative error code if at
 *     least one rule was found that matches exactly one of the two paths.
 */
static int
pe_compare_path(struct apnarr_array rulelist, struct pe_path_event *pevent,
    time_t now)
{
	size_t		i;

	DEBUG(DBG_PE, ">pe_compare_path");

	for (i = 0; i < apnarr_size(rulelist); i++) {
		struct apn_rule		*rule;
		char			*prefix = NULL;
		int			 len, p;
		int			 match[2] = { 0, 0 };

		rule = apnarr_access(rulelist, i);
		if (!pe_in_scope(rule->scope, pevent->cookie, now))
			continue;

		switch (rule->apn_type) {
		case APN_SFS_ACCESS:
			prefix = rule->rule.sbaccess.path;
			break;
		case APN_SFS_DEFAULT:
			prefix = rule->rule.sfsdefault.path;
			break;
		case APN_SB_ACCESS:
			prefix = rule->rule.sfsaccess.path;
			break;
		case APN_DEFAULT:
			prefix = NULL;
			break;
		default:
			log_warn("Invalid rule type %d on queue",
				rule->apn_type);
			prefix = NULL;
			break;
		}

		if (prefix == NULL)
			continue;
		len = strlen(prefix);

		/* Allow trailing slashes in prefix. Important for / */
		while(len && prefix[len-1] == '/')
			len--;

		for (p = 0; p < 2; p++) {
			if (strncmp(pevent->path[p], prefix, len) != 0) {
				match[p] = 0;
			} else if (pevent->path[p][len] &&
				   pevent->path[p][len] != '/') {
				match[p] = 0;
			} else {
				match[p] = 1;
			}
		}

		if (match[0] != match[1]) {
			DEBUG(DBG_PE, "<pe_compare_path");
			return -EXDEV;
		}
	}

	DEBUG(DBG_PE, "<pe_compare_path");
	return 0;
}

/**
 * This function is used to decide if a hardlink or rename operation
 * should be allow. It calls pe_compare_path with all applicable rule
 * lists, i.e. it checks:
 *  - admin and user rules
 *  - sandbox and sfs rules
 *  - matching rules are preselected based on each path in the event.
 *
 * @param proc The current process.
 * @param event The event containing both path names. The caller must
 *     verify that
 * @param now The current time for scope checks.
 * @return Zero if the event is allowed, a negative error code in case
 *     or an error.
 */
static int
pe_compare(struct pe_proc *proc, struct pe_path_event *event, time_t now)
{
	unsigned int		 x;
	static int		 types[2] = { APN_SB_ACCESS, APN_SFS_ACCESS };
	static int		 prios[PE_PRIO_MAX] = {
				     PE_PRIO_ADMIN,  PE_PRIO_USER1
				 };

	DEBUG(DBG_PE, ">pe_compare");
	if (!event)
		return -EPERM;
	/* NOTE: "!!" converts zero to zero and non-zero to 1. */
	for (x = 0; x < 8; x++) {
		int			type = types[!!(x & 4)];
		int			prio = prios[!!(x & 2)];
		int			pidx = (x & 1);
		struct apnarr_array	rulelist = apnarr_EMPTY;
		int			error = 0;

		if (type == APN_SFS_ACCESS) {
			error = pe_sfs_getrules(event->uid, prio,
			    event->path[pidx], &rulelist);
		} else if (type == APN_SB_ACCESS) {
			error = pe_sb_getrules(proc, event->uid, prio,
			    event->path[pidx], &rulelist);
		}

		DEBUG(DBG_PE, " pe_compare: prio %d rules %d for %s",
		    prio, (int)apnarr_size(rulelist), event->path[pidx]);
		if (error < 0)
			return error;
		if (apnarr_size(rulelist) == 0)
			continue;

		error = pe_compare_path(rulelist, event, now);
		apnarr_free(rulelist);
		if (error < 0)
			return error;
	}

	DEBUG(DBG_PE, "<pe_compare");
	return 0;
}

/*
 * Entry Points exported for the benefit of the policy engine unit tests.
 * DO NOT CALL THESE FUNCTIONS FROM NORMAL CODE.
 */
struct anoubisd_reply *
test_pe_handle_sfs(struct eventdev_hdr *hdr)
{
	return pe_handle_sfs(hdr);
}

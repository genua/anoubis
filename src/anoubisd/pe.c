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
#ifdef OPENBSD
#include <sys/limits.h>
#endif

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
#endif

#include <sys/queue.h>

#include <anoubis_protocol.h>
#include "anoubisd.h"
#include "pe.h"
#include "pe_filetree.h"
#include "sfs.h"
#include "cert.h"

static anoubisd_reply_t	*pe_dispatch_event(struct eventdev_hdr *);
static anoubisd_reply_t	*pe_handle_process(struct eventdev_hdr *);
static anoubisd_reply_t	*pe_handle_sfsexec(struct eventdev_hdr *);
static anoubisd_reply_t	*pe_handle_alf(struct eventdev_hdr *);
static anoubisd_reply_t	*pe_handle_ipc(struct eventdev_hdr *);
static anoubisd_reply_t	*pe_handle_sfs(struct eventdev_hdr *);
static anoubisd_reply_t *pe_handle_playgroundask(struct eventdev_hdr *);
static anoubisd_reply_t *pe_handle_playgroundproc(struct eventdev_hdr *);
static anoubisd_reply_t *pe_handle_playgroundfile(struct eventdev_hdr *);

static struct pe_file_event *pe_parse_file_event(struct eventdev_hdr *hdr);
#ifdef ANOUBIS_SOURCE_SFSPATH
static anoubisd_reply_t	*pe_handle_sfspath(struct eventdev_hdr *);
static struct pe_path_event *pe_parse_path_event(struct eventdev_hdr *hdr);
#endif

static struct pe_file_tree *upgrade_tree = NULL;
static int upgrade_counter = 0;
static struct pe_file_node *upgrade_iterator = NULL;
static int upgrade_ok = 1;

void
pe_init(void)
{
	sfshash_init();
	pe_proc_init();
	cert_init(1);
	pe_user_init();
}

void
pe_shutdown(void)
{
	pe_proc_flush();
	pe_user_flush_db(NULL);
	sfshash_flush();
}

void
pe_reconfigure(void)
{
	sfshash_flush();
	cert_reconfigure(1);
	pe_user_reconfigure();
}

void
pe_set_upgrade_ok(int value)
{
	upgrade_ok = value;
}

static int	sfsversionfd = -1;

void
pe_upgrade_start(struct pe_proc *proc)
{
	/* Nothing to do if the process is already an upgrader. */
	if (pe_proc_is_upgrade_parent(proc))
		return;
	/* Initialize the tree if we currently have none. */
	if (upgrade_tree == NULL) {
		upgrade_tree = pe_init_filetree();
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

void
pe_upgrade_end(struct pe_proc *proc)
{
	struct pe_file_node	*it, *next;
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
			pe_delete_node(upgrade_tree, it);
		} else {
			last_cookie = it->task_cookie;
		}
	}
	pe_proc_upgrade_clrallmarks();
	upgrade_iterator = NULL;

	/* Trigger actual end of upgrade handling, i.e. checksum update. */
	send_upgrade_start();
}

void
pe_upgrade_filelist_start(void)
{
	if (!upgrade_tree || upgrade_counter) {
		upgrade_iterator = NULL;
	} else {
		upgrade_iterator = pe_filetree_start(upgrade_tree);
	}
}

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

struct pe_file_node *
pe_upgrade_filelist_get(void)
{
	if (!upgrade_tree || upgrade_counter)
		upgrade_iterator = NULL;
	return upgrade_iterator;
}

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

/*
 * NOTE: Basic length verification has been done by amsg_verify.
 */
anoubisd_reply_t *
policy_engine(anoubisd_msg_t *request)
{
	anoubisd_reply_t	*reply;
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

static anoubisd_reply_t *
pe_dispatch_event(struct eventdev_hdr *hdr)
{
	anoubisd_reply_t	*reply = NULL;

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

static anoubisd_reply_t *
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

static anoubisd_reply_t *
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

static anoubisd_reply_t *
pe_handle_alf(struct eventdev_hdr *hdr)
{
	struct alf_event	*msg;
	anoubisd_reply_t	*reply = NULL;
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

static anoubisd_reply_t *
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

static anoubisd_reply_t *
reply_merge(struct eventdev_hdr *hdr, anoubisd_reply_t *sfs,
    anoubisd_reply_t *sb)
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

static anoubisd_reply_t *
pe_handle_sfs(struct eventdev_hdr *hdr)
{
	anoubisd_reply_t		*reply = NULL, *reply2 = NULL;
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
			pe_insert_node(upgrade_tree, fevent->path, cookie);
			fevent->upgrade_flags |= PE_UPGRADE_TOUCHED;
		}
	} else {
		if (fevent->path && pe_find_file(upgrade_tree, fevent->path))
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
static anoubisd_reply_t *
pe_handle_sfspath(struct eventdev_hdr *hdr)
{
	anoubisd_reply_t		*reply = NULL;
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

	reply = malloc(sizeof(anoubisd_reply_t));
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
			if (pe_compare(proc, pevent, APN_SFS_ACCESS, now))
				reply->reply = EXDEV;
			else if (pe_compare(proc, pevent, APN_SB_ACCESS, now))
				reply->reply = EXDEV;
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
static anoubisd_reply_t *
pe_handle_playgroundask(struct eventdev_hdr *hdr)
{

	anoubisd_reply_t		*reply = NULL;
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

	reply = malloc(sizeof(anoubisd_reply_t));
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
static anoubisd_reply_t *
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
		pe_proc_set_playgroundid(proc, extract_pgid(&pg->common));
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
static anoubisd_reply_t *
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
		struct pe_proc	*proc = pe_proc_get(pg->common.task_cookie);

		if (pe_playground_file_scanrequest(pg->pgid, pg->dev,
		    pg->ino, pg->path, hdr->msg_uid) < 0) {
			log_warnx("illegal scan request for file %" PRIx64
			    ":%" PRIx64 " (%s) in playground %" PRIx64
			    " by user %d", pg->dev, pg->ino, pg->path,
			    pg->pgid, hdr->msg_uid);
			break;
		}
		log_info("scanning of file %" PRIx64 ":%" PRIx64 " (%s) in "
		    "playground %" PRIx64 " requested by user %d",
		    pg->dev, pg->ino, pg->path, pg->pgid, hdr->msg_uid);
		send_lognotify(proc, hdr, 0 /* error */, APN_LOG_NORMAL,
		    0 /* ruleid */, 0 /* prio */, 0 /* sfsmatch */);
		break;
	}
	default:
		log_warnx("pe_handle_playgroundfile: Bad operation %d", pg->op);
	}
	return NULL;
}

void
pe_dump(void)
{
	pe_proc_dump();
	pe_user_dump();
	pe_playground_dump();
}

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
			return (-1);
		}
	}

	DEBUG(DBG_PE, "<pe_compare_path");
	return (0);
}

int
pe_compare(struct pe_proc *proc, struct pe_path_event *event, int type,
    time_t now)
{
	int			 decision = APN_ACTION_ALLOW;
	int			 i, p;

	DEBUG(DBG_PE, ">pe_compare");

	if (!event)
		return (APN_ACTION_DENY);

	if ((type != APN_SFS_ACCESS) && (type != APN_SB_ACCESS)) {
		log_warn("Called with unhandled rule type");
		return (-1);
	}

	for (i = 0; i < PE_PRIO_MAX; i++) {

		for (p = 0; p < 2; p++) {
			struct apnarr_array	rulelist = apnarr_EMPTY;
			int			error = 0;

			if (type == APN_SFS_ACCESS)
				error = pe_sfs_getrules(event->uid, i,
						event->path[p], &rulelist);
			else if (type == APN_SB_ACCESS)
				error = pe_sb_getrules(proc, event->uid, i,
						event->path[p], &rulelist);

			DEBUG(DBG_PE, " pe_compare: prio %d rules %d for %s",
			    i, (int)apnarr_size(rulelist), event->path[p]);
			if (error < 0) {
				decision = APN_ACTION_DENY;
				break;
			}
			if (apnarr_size(rulelist) == 0)
				continue;

			if (pe_compare_path(rulelist, event, now) < 0)
				decision = APN_ACTION_DENY;
			apnarr_free(rulelist);

			if (decision == APN_ACTION_DENY)
				break;
		}
		if (decision == APN_ACTION_DENY)
			break;
	}

	DEBUG(DBG_PE, "<pe_compare");
	return (decision);
}

/*
 * Entry Points exported for the benefit of the policy engine unit tests.
 * DO NOT CALL THESE FUNCTIONS FROM NORMAL CODE.
 */
anoubisd_reply_t
*test_pe_handle_sfs(struct eventdev_hdr *hdr)
{
	return pe_handle_sfs(hdr);
}

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
#include <sys/socket.h>
#ifdef OPENBSD
#include <sys/limits.h>
#endif

#include <arpa/inet.h>
#include <netinet/in.h>

#include <apn.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef LINUX
#include <queue.h>
#include <bsdcompat.h>
#include <linux/anoubis_sfs.h>
#include <linux/anoubis_alf.h>
#endif
#ifdef OPENBSD
#include <sys/queue.h>
#include <dev/anoubis_alf.h>
#endif

#ifdef LINUX
#include <linux/anoubis_sfs.h>
#include <linux/anoubis_alf.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis_alf.h>
#endif

#include "anoubisd.h"

struct pe_context {
	struct apn_rule		*rule;
	struct apn_context	*ctx;
};

struct pe_proc {
	TAILQ_ENTRY(pe_proc)	 entry;
	int			 refcount;
	pid_t			 pid;

	u_int8_t		*csum;
	char			*pathhint;
	anoubis_cookie_t	 task_cookie;
	struct pe_proc		*parent_proc;

	struct pe_context	*context;
	struct apn_rule		*policy;
};
TAILQ_HEAD(tracker, pe_proc) tracker;

struct pe_user {
	TAILQ_ENTRY(pe_user)	 entry;
	uid_t			 uid;

#define PE_PRIO_ADMIN	0
#define PE_PRIO_USER1	1
#define PE_PRIO_MAX	2
	struct apn_ruleset	*prio[PE_PRIO_MAX];
};
TAILQ_HEAD(policies, pe_user) pdb;

anoubisd_reply_t	*policy_engine(int, void *);
anoubisd_reply_t	*pe_dispatch_event(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_process(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_alf(struct eventdev_hdr *);
char			*pe_dump_alfmsg(struct alf_event *);
anoubisd_reply_t	*pe_handle_sfs(struct eventdev_hdr *);
anoubisd_reply_t	*pe_dispatch_policy(struct anoubisd_msg_comm *);

struct pe_proc		*pe_get_proc(anoubis_cookie_t);
void			 pe_put_proc(struct pe_proc * proc);
struct pe_proc		*pe_alloc_proc(anoubis_cookie_t, anoubis_cookie_t);
void			 pe_track_proc(struct pe_proc *);
void			 pe_untrack_proc(struct pe_proc *);
void			 pe_set_parent_proc(struct pe_proc * proc,
					    struct pe_proc * newparent);
int			 pe_load_db(void);
int			 pe_load_dir(const char *, int);
struct apn_ruleset	*pe_load_policy(const char *);
int			 pe_insert_rs(struct apn_ruleset *, uid_t, int);
struct pe_user		*pe_get_user(uid_t);
void			 pe_inherit_ctx(struct pe_proc *);
void			 pe_set_ctx(struct pe_proc *, uid_t, const u_int8_t *,
			     const char *);
struct pe_context	*pe_search_ctx(struct apn_ruleset *, const u_int8_t *,
			     const char *);
int			 pe_cmp_ctx(struct pe_proc *, const u_int8_t *,
			     const char *);

void
pe_init(void)
{
	int	count;

	TAILQ_INIT(&tracker);
	TAILQ_INIT(&pdb);

	count = pe_load_db();

	log_info("pe_init: %d policies loaded", count);
}

int
pe_load_db(void)
{
	int	count = 0;

	/* load admin policies */
	count = pe_load_dir(ANOUBISD_ADMINDIR, PE_PRIO_ADMIN);

	/* load user policies */
	count += pe_load_dir(ANOUBISD_USERDIR, PE_PRIO_USER1);

	return (count);
}

int
pe_load_dir(const char *dirname, int prio)
{
	DIR			*dir;
	struct dirent		*dp;
	struct apn_ruleset	*rs;
	int			 count;
	uid_t			 uid;
	const char		*errstr;
	char			*filename;

	DEBUG(DBG_PE_POLICY, "pe_load_dir: %s", dirname);

	if (prio < 0 || prio >= PE_PRIO_MAX) {
		log_warnx("pe_load_dir: illegal priority %d", prio);
		return (0);
	}

	if ((dir = opendir(dirname)) == NULL) {
		log_warn("opendir");
		return (0);
	}

	count = 0;

	/* iterate over directory entries */
	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_type != DT_REG)
			continue;

		/*
		 * Validate the file name: For PE_PRIO_ADMIN it has
		 * to be either ANOUBISD_DEFAULTNAME or a numeric uid,
		 * for other priorities only a numeric uid.
		 */
		if (prio == PE_PRIO_ADMIN && strcmp(dp->d_name,
		    ANOUBISD_DEFAULTNAME) == 0)
			uid = -1;
		else {
			uid = strtonum(dp->d_name, 0, UID_MAX, &errstr);
			if (errstr) {
				log_warnx("\"%s/%s\" %s", dirname, dp->d_name,
				    errstr);
				continue;
			}
		}
		if (asprintf(&filename, "%s/%s", dirname, dp->d_name) == -1) {
			log_warn("asprintf");
			continue;
		}
		rs = pe_load_policy(filename);
		free(filename);

		/* If parsing fails, we just continue */
		if (rs == NULL)
			continue;

		if (pe_insert_rs(rs, uid, prio) != 0) {
			apn_free_ruleset(rs);
			log_warnx("could not insert policy %s/%s", dirname,
			    dp->d_name);
			continue;
		}
		count++;
	}

	if (closedir(dir) == -1)
		log_warn("closedir");

	DEBUG(DBG_PE_POLICY, "pe_load_dir: %d policies inserted", count);

	return (count);
}

struct apn_ruleset *
pe_load_policy(const char *name)
{
	struct apn_ruleset	*rs;
	int			 ret;

	DEBUG(DBG_PE_POLICY, "pe_load_policy: %s", name);

	ret = apn_parse(name, &rs, 0);
	if (ret == -1) {
		log_warn("could not parse \"%s\"", name);
		return (NULL);
	}
	if (ret == 1) {
		log_warnx("could not parse \"%s\"", name);
		return (NULL);
	}

	return (rs);
}

int
pe_insert_rs(struct apn_ruleset *rs, uid_t uid, int prio)
{
	struct pe_user		*user;

	if (rs == NULL) {
		log_warnx("pe_insert_rs: empty ruleset");
		return (1);
	}
	if (prio < 0 || prio >= PE_PRIO_MAX) {
		log_warnx("pe_insert_rs: illegal priority %d", prio);
		return (1);
	}

	/* Find or create user */
	if ((user = pe_get_user(uid)) == NULL) {
		if ((user = calloc(1, sizeof(struct pe_user))) == NULL) {
			log_warn("calloc");
			master_terminate(ENOMEM);	/* XXX HSH */
		}
		user->uid = uid;
		TAILQ_INSERT_TAIL(&pdb, user, entry);
	} else if (user->prio[prio]) {
		DEBUG(DBG_PE_POLICY, "pe_insert_rs: freeing %p prio %d user %p",
		    user->prio[prio], prio, user);
		apn_free_ruleset(user->prio[prio]);
		user->prio[prio] = NULL;
	}

	user->prio[prio] = rs;

	DEBUG(DBG_PE_POLICY, "pe_insert_rs: uid %d (%p prio %p, %p)",
	    (int)uid, user, user->prio[0], user->prio[1]);

	return (0);
}

struct pe_user *
pe_get_user(uid_t uid)
{
	struct pe_user	*puser, *user;

	user = NULL;
	TAILQ_FOREACH(puser, &pdb, entry) {
		if (puser->uid != uid)
			continue;
		user = puser;
		break;
	}

	DEBUG(DBG_PE_POLICY, "pe_get_user: uid %d user %p", (int)uid, user);

	return (user);
}

anoubisd_reply_t *
policy_engine(int mtype, void *request)
{
	anoubisd_reply_t *reply;
	struct eventdev_hdr *hdr = NULL;
	struct anoubisd_msg_comm *comm = NULL;

	DEBUG(DBG_TRACE, ">policy_engine");

	switch (mtype) {
	case ANOUBISD_MSG_EVENTDEV:
		hdr = (struct eventdev_hdr *)request;
		reply = pe_dispatch_event(hdr);
		break;

	case ANOUBISD_MSG_POLREQUEST:
		comm = (anoubisd_msg_comm_t *)request;
		reply = pe_dispatch_policy(comm);
		break;

	default:
		reply = NULL;
		break;
	}

	DEBUG(DBG_TRACE, "<policy_engine");

	return (reply);
}

anoubisd_reply_t *
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

	case ANOUBIS_SOURCE_SFS:
		reply = pe_handle_sfs(hdr);
		break;

	case ANOUBIS_SOURCE_ALF:
		reply = pe_handle_alf(hdr);
		break;

	default:
		log_warnx("pe_dispatch_event: unknown message source %d",
		    hdr->msg_source);
		break;
	}

	return (reply);
}

anoubisd_reply_t *
pe_handle_process(struct eventdev_hdr *hdr)
{
#ifndef OPENBSD		/* XXX HSH */
	struct ac_process_message	*msg;
	struct pe_proc			*proc = NULL;

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
		proc = pe_alloc_proc(msg->task_cookie, msg->common.task_cookie);
		pe_inherit_ctx(proc);
		pe_track_proc(proc);
		break;
	case ANOUBIS_PROCESS_OP_EXEC:
		proc = pe_get_proc(msg->common.task_cookie);
		if (proc == NULL) {
			DEBUG(DBG_PE_PROC, "pe_handle_process: untracked "
			    "process %u 0x%08llx execs", hdr->msg_pid,
			    msg->common.task_cookie);
			break;
		}
		proc->pid = hdr->msg_pid;
		pe_set_ctx(proc, hdr->msg_uid, proc->csum, proc->pathhint);

		/* Get our policy */
		DEBUG(DBG_PE_PROC, "pe_handle_process: using policy %p",
		    proc->context ? proc->context->rule : NULL);
		break;
	case ANOUBIS_PROCESS_OP_EXIT:
		/* NOTE: Do NOT use msg->common.task_cookie here! */
		proc = pe_get_proc(msg->task_cookie);
		pe_untrack_proc(proc);
		break;
	default:
		log_warnx("pe_handle_process: undefined operation %d", msg->op);
		break;
	}

	if (proc) {
		DEBUG(DBG_PE_PROC, "pe_handle_process: token 0x%08llx pid %d "
		    "uid %u op %d path \"%s\" proc %p csum 0x%08x... parent "
		    "token 0x%08llx",
		    proc->task_cookie, proc->pid, hdr->msg_uid, msg->op,
		    msg->op == ANOUBIS_PROCESS_OP_EXEC ? msg->pathhint : "",
		    proc, proc->csum ? htonl(*(unsigned long *)proc->csum) : 0,
		    proc->parent_proc ? proc->parent_proc->task_cookie : 0);
		pe_put_proc(proc);
	}
#endif
	return (NULL);
}

anoubisd_reply_t *
pe_handle_alf(struct eventdev_hdr *hdr)
{
	struct alf_event	*msg;
	anoubisd_reply_t	*reply = NULL;

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

	DEBUG(DBG_PE_ALF, "pe_hanlde_alf: pid %u uid %u token %x",
	    hdr->msg_pid, hdr->msg_uid, hdr->msg_token);

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_handle_alf: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}

	/* XXX HSH ALF events are always allowed */
	reply->ask = 0;		/* false */
	reply->reply = 0;	/* allow */
	reply->timeout = (time_t)0;
	reply->len = 0;

	return (reply);
}

anoubisd_reply_t *
pe_handle_sfs(struct eventdev_hdr *hdr)
{
	anoubisd_reply_t	*reply = NULL;
#ifdef LINUX	/* XXX HSH */
	struct sfs_open_message	*msg;
	struct pe_proc		*proc;

	if (hdr == NULL) {
		log_warnx("pe_handle_sfs: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct sfs_open_message))) {
		log_warnx("pe_handle_sfs: short message");
		return (NULL);
	}
	msg = (struct sfs_open_message *)(hdr + 1);

	/* check tracker list */
	proc = pe_get_proc(msg->common.task_cookie);

	if (proc == NULL) {
		DEBUG(DBG_PE_SFS, "pe_handle_sfs: untracked process %u",
		    hdr->msg_pid);
	} else {
		/* if not yet set, fill in checksum and pathhint */
		if ((proc->csum == NULL) && (msg->flags &
		    ANOUBIS_OPEN_FLAG_CSUM)) {
			if ((proc->csum = calloc(sizeof(u_int8_t),
			    sizeof(msg->csum))) == NULL) {
				log_warn("calloc");
				master_terminate(ENOMEM);	/* XXX HSH */
			}
			bcopy(msg->csum, proc->csum, sizeof(msg->csum));
		}
		if ((proc->pathhint == NULL) &&
		    (msg->flags & ANOUBIS_OPEN_FLAG_PATHHINT)) {
			if ((proc->pathhint = strdup(msg->pathhint)) == NULL) {
				log_warn("strdup");
				master_terminate(ENOMEM);	/* XXX HSH */
			}
		}
		pe_put_proc(proc);
	}
	DEBUG(DBG_PE_SFS, "pe_handle_sfs: pid %u uid %u ino %llu dev 0x%llx "
	    "flags %lx \"%s\" csum 0x%08x...", hdr->msg_pid, hdr->msg_uid,
	    msg->ino, msg->dev, msg->flags, msg->pathhint ? msg->pathhint :
	    "", htonl(*(unsigned long *)msg->csum));

#endif	/* LINUX */

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_handle_sfs: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}

	/* XXX HSH SFS events are always allowed */
	reply->ask = 0;		/* false */
	reply->reply = 0;	/* allow */
	reply->timeout = (time_t)0;
	reply->len = 0;

	return (reply);
}

anoubisd_reply_t *
pe_dispatch_policy(struct anoubisd_msg_comm *comm)
{
	anoubisd_reply_t	*reply = NULL;

	if (comm == NULL) {
		log_warnx("pe_dispatch_policy: empty comm");
		return (NULL);
	}

	DEBUG(DBG_PE_POLICY, "pe_dispatch_policy: token %x", comm->token);

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_dispatch_policy: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}

	reply->token = comm->token;

	return (reply);
}

struct pe_proc *
pe_get_proc(anoubis_cookie_t cookie)
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
		DEBUG(DBG_PE_TRACKER, "pe_get_proc: proc %p pid %d cookie "
		    "0x%08llx", proc, (int)proc->pid, proc->task_cookie);
		proc->refcount++;
	}

	return (proc);
}

void
pe_put_proc(struct pe_proc * proc)
{
	if (!proc || --(proc->refcount))
		return;
	if (proc->parent_proc != proc)
		pe_put_proc(proc->parent_proc);
	if (proc->csum)
		free(proc->csum);
	if (proc->pathhint)
		free(proc->pathhint);
	/* XXX Free contents of proc structure. */
	free(proc);
}

struct pe_proc *
pe_alloc_proc(anoubis_cookie_t cookie, anoubis_cookie_t parent_cookie)
{
	struct pe_proc	*proc;

	if ((proc = calloc(1, sizeof(struct pe_proc))) == NULL) {
		log_warn("pe_findproc: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);	/* XXX HSH */
	}
	proc->task_cookie = cookie;
	proc->parent_proc = pe_get_proc(parent_cookie);
	proc->pid = -1;
	proc->refcount = 1;

	DEBUG(DBG_PE_TRACKER, "pe_alloc_proc: proc %p cookie 0x%08llx "
	    "parent cookie 0x%08llx", proc, proc->task_cookie,
	    proc->parent_proc ? parent_cookie : 0);

	return (proc);
}

void
pe_set_parent_proc(struct pe_proc * proc, struct pe_proc * newparent)
{
	struct pe_proc * oldparent = proc->parent_proc;
	proc->parent_proc = newparent;
	if (proc != newparent)
		newparent->refcount++;
	if (oldparent && oldparent != proc)
		pe_put_proc(oldparent);
}

void
pe_track_proc(struct pe_proc *proc)
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

void
pe_untrack_proc(struct pe_proc * proc)
{
	if (!proc)
		return;
	TAILQ_REMOVE(&tracker, proc, entry);
	pe_put_proc(proc);
}

/*
 * Inherit the parent process context. This might not necessarily be
 * the direct parent process, but some grand parent.  This happens on
 * fork(2).  Moreover, we set our parent cookie to that particular parent.
 *
 * If our parent was never tracked, we will not inherit a context,
 * thus we have to set one on exec(2).
 */
void
pe_inherit_ctx(struct pe_proc *proc)
{
	struct pe_proc	*parent;

	if (proc == NULL) {
		log_warnx("pe_inherit_ctx: empty process");
		return;
	}

	/* Get parents context */
	parent = proc->parent_proc;
	DEBUG(DBG_PE_CTX, "pe_inherit_ctx: parent %p 0x%08llx", parent,
	    parent ? parent->task_cookie : 0);

	if (parent && parent->context == NULL) {
		DEBUG(DBG_PE_CTX, "pe_inherit_ctx: parent %p 0x%08llx has no "
		    "context", parent, parent->task_cookie);
	}

	if (parent && parent->context) {
		DEBUG(DBG_PE_CTX, "pe_inherit_ctx: rule %p ctx %p",
		    parent->context->rule, parent->context->ctx);
		proc->context = parent->context;
		pe_set_parent_proc(proc, parent);
	}
}

/*
 * Set our context.  If we never inherited one, search for an apropriate
 * application rule and use that one as our context from now on.
 */
void
pe_set_ctx(struct pe_proc *proc, uid_t uid, const u_int8_t *csum,
    const char *pathhint)
{
	struct pe_context	*pctx, *ctx;
	struct pe_user		*user;
	int			 i;

	/* We can both handle csum and pathhint being NULL. */
	if (proc == NULL) {
		log_warnx("pe_set_ctx: empty process");
		return;
	}

	/*
	 * If we have inherited a context, check if it has context rule
	 * that allows us to switch context.  If no, we keep the context and
	 * just return.  Otherwise, we continue and search our new context.
	 */
	if (proc->context) {
		DEBUG(DBG_PE_CTX, "pe_set_ctx: proc %p 0x%08llx has context "
		    "%p rule %p ctx %p", proc, proc->task_cookie,
		    proc->context, proc->context ? proc->context->rule : NULL,
		    proc->context ? proc->context->ctx : NULL);

		if (pe_cmp_ctx(proc, csum, pathhint) != 0) {
			DEBUG(DBG_PE_CTX, "pe_set_ctx: keeping context");
			return;
		}
		DEBUG(DBG_PE_CTX, "pe_set_ctx: switching context");
	}

	/*
	 * If our parent is either not tracked or has no context (which
	 * actually should not happen), or we are allowed to switch context
	 * we use the first matching application rule as new context.
	 */
	if ((user = pe_get_user(uid)) == NULL &&
	    (user = pe_get_user(-1)) == NULL) {
		/*
		 * If we have not found any context bound to the UID
		 * or the default user (-1), we use an empty context,
		 * which will never allow us to leave it.
		 */
		ctx = NULL;
	} else {
		/*
		 * If we find no context rule on a priority queue,
		 * we have to stop immediately, as no rule means,
		 * context switching is not allowed.
		 */
		ctx = NULL;
		for (i = 0; i < PE_PRIO_MAX; i++) {
			pctx = pe_search_ctx(user->prio[i], csum, pathhint);
			if (pctx == NULL)
				break;
			ctx = pctx;
		}
	}
	proc->context = ctx;
	pe_set_parent_proc(proc, proc);

	DEBUG(DBG_PE_CTX, "pe_set_ctx: proc %p 0x%08llx got context "
	    "%p rule %p ctx %p", proc, proc->task_cookie,
	    proc->context, proc->context ? proc->context->rule : NULL,
	    proc->context ? proc->context->ctx : NULL);
}

struct pe_context *
pe_search_ctx(struct apn_ruleset *rs, const u_int8_t *csum,
    const char *pathhint)
{
	struct pe_context	*context;
	struct apn_rule		*prule, *rule;
	struct apn_alfrule	*alfrule;
	struct apn_app		*hp;

	DEBUG(DBG_PE_CTX, "pe_search_ctx: ruleset %p csum 0x%08x...", rs,
	    csum ?  htonl(*(unsigned long *)csum) : 0);

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
		if (csum == NULL)
			continue;

		/*
		 * Otherwise walk chain of applications and check
		 * against hashvalue.
		 */
		for (hp = prule->app; hp && rule == NULL; hp = hp->next) {
			if (bcmp(hp->hashvalue, csum, sizeof(hp->hashvalue)))
				continue;
			rule = prule;
		}
		if (rule)
			break;
	}
	if (rule == NULL) {
		/* XXX HSH apply a default rule */
		DEBUG(DBG_PE_CTX, "pe_search_ctx: no rule found");
		return (NULL);
	}

	if ((context = calloc(1, sizeof(struct pe_context))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);	/* XXX HSH */
	}
	context->rule = rule;

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

	DEBUG(DBG_PE_CTX, "pe_search_ctx: context %p rule %p %s", context,
	    rule, context->ctx ? (context->ctx->application ?
	    context->ctx->application->name : "any") : "<none>");

	return (context);
}

/*
 * Compare a context (in proc) against csum (and maybe one day against
 * path, too).  Return 0 on match, 1 otherwise.
 */
int
pe_cmp_ctx(struct pe_proc *proc, const u_int8_t *csum, const char *pathhint)
{
	struct apn_app	*hp;
	int		 cmp;

	if (proc->context == NULL || proc->context->ctx == NULL ||
	    csum == NULL || pathhint == NULL)
		return (1);

	/*
	 * If the rule says "context new any", application will be
	 * empty.  In that case, initialize the compare result cmp to 0
	 * (ie. match).
	 */
	hp = proc->context->ctx->application;
	if (hp == NULL)
		cmp = 0;
	else
		cmp = 1;
	while (hp) {
		cmp = bcmp(hp->hashvalue, csum, sizeof(hp->hashvalue));
		if (cmp == 0)
			break;
		hp = hp->next;
	}

	if (cmp == 0) {
		DEBUG(DBG_PE_CTX, "pe_cmp_ctx: found \"%s\" csm 0x%08x...",
		    (hp && hp->name) ? hp->name : "any" ,
		    (hp && hp->hashvalue) ?
		    htonl(*(unsigned long *)hp->hashvalue) : 0);
	}

	return (cmp);
}

void
pe_dump(void)
{
	struct pe_proc		*proc;
	struct pe_user		*user;
	struct apn_ruleset	*rs;
	struct apn_rule		*rp;
	int			 i;

	log_info("tracked processes:");
	TAILQ_FOREACH(proc, &tracker, entry) {
		log_info("proc %p token 0x%08llx pid %d csum 0x%08x "
		    "pathhint \"%s\" ctx %p rule %p policy %p", proc,
		    proc->task_cookie, (int)proc->pid, proc->csum ?
		    htonl(*(unsigned long *)proc->csum) : 0,
		    proc->pathhint ? proc->pathhint : "", proc->context,
		    proc->context ? proc->context->rule : NULL, proc->policy);
	}

	log_info("policies");
	TAILQ_FOREACH(user, &pdb, entry) {
		log_info("uid %d", (int)user->uid);
		for (i = 0; i < PE_PRIO_MAX; i++) {
			if (user->prio[i] == NULL)
				continue;
			rs = user->prio[i];
			log_info("\truleset %p at priority %d", rs, i);
			TAILQ_FOREACH(rp, &rs->alf_queue, entry)
				log_info("\t\talf rule %p for %s", rp,
				rp->app ? rp->app->name : "any");
		}
	}
}

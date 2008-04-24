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
#define POLICY_ALLOW	0
#define POLICY_DENY	1
#define POLICY_ASK	2
#endif

#include <anoubis_protocol.h>

#include "anoubisd.h"

struct pe_user {
	TAILQ_ENTRY(pe_user)	 entry;
	uid_t			 uid;

#define PE_PRIO_ADMIN	0
#define PE_PRIO_USER1	1
#define PE_PRIO_MAX	2
	struct apn_ruleset	*prio[PE_PRIO_MAX];
};
TAILQ_HEAD(policies, pe_user) pdb;

struct pe_context {
	int			 refcount;
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

	/* Per priority contexts */
	struct pe_context	*context[PE_PRIO_MAX];
	int			 valid_ctx;
};
TAILQ_HEAD(tracker, pe_proc) tracker;

static char * prio_to_string[PE_PRIO_MAX] = {
#ifndef lint
	[ PE_PRIO_ADMIN ] = ANOUBISD_ADMINDIR,
	[ PE_PRIO_USER1 ] = ANOUBISD_USERDIR,
#endif
};

struct policy_request {
	LIST_ENTRY(policy_request) next;
	u_int64_t token;
	u_int32_t ptype;
	u_int32_t authuid;
	int fd;
	char * tmpname;
	char * realname;
};
LIST_HEAD(, policy_request) preqs;

anoubisd_reply_t	*policy_engine(int, void *);
anoubisd_reply_t	*pe_dispatch_event(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_process(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_sfsexec(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_alf(struct eventdev_hdr *);
anoubisd_reply_t	*pe_decide_alf(struct pe_proc *, struct eventdev_hdr *);
int			 pe_alf_evaluate(struct pe_context *,
			     struct alf_event *, int *);
int			 pe_decide_alffilt(struct apn_rule *, struct
			     alf_event *, int *);
int			 pe_decide_alfcap(struct apn_rule *, struct
			     alf_event *, int *);
int			 pe_decide_alfdflt(struct apn_rule *, struct
			     alf_event *, int *);
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
void			 pe_flush_db(void);
void			 pe_flush_tracker(void);
int			 pe_load_dir(const char *, int);
struct apn_ruleset	*pe_load_policy(const char *);
int			 pe_insert_rs(struct apn_ruleset *, uid_t, int);
struct pe_user		*pe_get_user(uid_t);
void			 pe_inherit_ctx(struct pe_proc *);
void			 pe_set_ctx(struct pe_proc *, uid_t, const u_int8_t *,
			     const char *);
struct pe_context	*pe_search_ctx(struct apn_ruleset *, const u_int8_t *,
			     const char *);
struct pe_context	*pe_alloc_ctx(struct apn_rule *);
void			 pe_put_ctx(struct pe_context *);
void			 pe_reference_ctx(struct pe_context *);
int			 pe_decide_ctx(struct pe_proc *, const u_int8_t *,
			     const char *);
void			 pe_dump_ctx(const char *, struct pe_proc *);
int			 pe_addrmatch_out(struct alf_event *, struct
			     apn_alfrule *);
int			 pe_addrmatch_in(struct alf_event *, struct
			     apn_alfrule *);
int			 pe_addrmatch_host(struct apn_host *, void *,
			     unsigned short);
int			 pe_addrmatch_port(struct apn_port *, void *,
			     unsigned short);
static char		*pe_policy_file_name(uid_t uid, int prio);
static int		pe_open_policy_file(uid_t uid, int prio);

/*
 * The following code utilizes list functions from BSD queue.h, which
 * cannot be reliably annotated. We therefore exclude the following
 * functions from memory checking.
 */
/*@-memchecks@*/
void
pe_init(void)
{
	int	count;

	TAILQ_INIT(&tracker);
	TAILQ_INIT(&pdb);
	LIST_INIT(&preqs);

	count = pe_load_db();

	log_info("pe_init: %d policies loaded", count);
}

void
pe_shutdown(void)
{
	pe_flush_tracker();
	pe_flush_db();
}

void
pe_flush_db(void)
{
	struct pe_user	*p, *pnext;
	int		 i;

	for (p = TAILQ_FIRST(&pdb); p != TAILQ_END(&pdb); p = pnext) {
		pnext = TAILQ_NEXT(p, entry);
		TAILQ_REMOVE(&pdb, p, entry);

		for (i = 0; i < PE_PRIO_MAX; i++)
			apn_free_ruleset(p->prio[i]);
		free(p);
	}
}

void
pe_flush_tracker(void)
{
	struct pe_proc	*p, *pnext;

	for (p = TAILQ_FIRST(&tracker); p != TAILQ_END(&tracker); p = pnext) {
		pnext = TAILQ_NEXT(p, entry);
		TAILQ_REMOVE(&tracker, p, entry);
		pe_put_proc(p);
	}
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

static char *
pe_policy_file_name(uid_t uid, int prio)
{
	char * name;
	if (asprintf(&name, "/%s/%d", prio_to_string[prio], uid) == -1)
		return NULL;
	return name;
}

static int
pe_open_policy_file(uid_t uid, int prio)
{
	int err, fd;
	char * name = pe_policy_file_name(uid, prio);

	if (!name)
		return -ENOMEM;
	fd = open(name, O_RDONLY);
	err = errno;
	free(name);
	if (fd >= 0)
		return fd;
	if (err != ENOENT || prio != PE_PRIO_ADMIN)
		return -err;
	fd = open("/" ANOUBISD_ADMINDIR "/" ANOUBISD_DEFAULTNAME, O_RDONLY);
	if (fd >= 0)
		return fd;
	return -errno;
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

	case ANOUBIS_SOURCE_SFSEXEC:
		reply = pe_handle_sfsexec(hdr);
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
pe_handle_sfsexec(struct eventdev_hdr *hdr)
{
#ifndef OPENBSD
	struct sfs_open_message		*msg;
	struct pe_proc			*proc = NULL;

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
	proc = pe_get_proc(msg->common.task_cookie);
	if (proc == NULL) {
		DEBUG(DBG_PE_PROC, "pe_handle_process: untracked "
		    "process %u 0x%08llx execs", hdr->msg_pid,
		    msg->common.task_cookie);
		return (NULL);
	}
	/* if not yet set, fill in checksum and pathhint */
	if ((proc->csum == NULL) && (msg->flags & ANOUBIS_OPEN_FLAG_CSUM)) {
		if ((proc->csum = calloc(1, sizeof(msg->csum))) == NULL) {
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
	proc->pid = hdr->msg_pid;
	pe_set_ctx(proc, hdr->msg_uid, proc->csum, proc->pathhint);

	/* Get our policy */
	DEBUG(DBG_PE_PROC, "pe_handle_sfsexec: using policies %p %p "
	    "for %s csum 0x%08lx...",
	    proc->context[0] ? proc->context[0]->rule : NULL,
	    proc->context[1] ? proc->context[1]->rule : NULL,
	    proc->pathhint ? proc->pathhint : "",
	    proc->csum ? htonl(*(unsigned long *)proc->csum) : 0);
	pe_put_proc(proc);
#endif
	return (NULL);
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
		    "uid %u op %d proc %p csum 0x%08x... parent "
		    "token 0x%08llx",
		    proc->task_cookie, proc->pid, hdr->msg_uid, msg->op,
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
	if ((proc = pe_get_proc(msg->common.task_cookie)) == NULL) {
		DEBUG(DBG_PE_ALF, "pe_handle_alf: untrackted process %u",
		    hdr->msg_pid);
		/* Untracked process: create it, set context and track it. */
		proc = pe_alloc_proc(msg->common.task_cookie, 0);
		pe_set_ctx(proc, hdr->msg_uid, NULL, NULL);
		pe_track_proc(proc);
	}

	reply = pe_decide_alf(proc, hdr);
	pe_put_proc(proc);

	DEBUG(DBG_TRACE, "<policy_engine");
	return (reply);
}

anoubisd_reply_t *
pe_decide_alf(struct pe_proc *proc, struct eventdev_hdr *hdr)
{
#ifdef LINUX
	static char		*verdict[3] = { "allowed", "denied", "asked" };
	struct alf_event	*msg;
	anoubisd_reply_t	*reply;
	int			 i, ret, decision, log;
	char			*dump = NULL;

	if (hdr == NULL) {
		log_warnx("pe_decide_alf: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct alf_event))) {
		log_warnx("pe_decide_alf: short message");
		return (NULL);
	}
	msg = (struct alf_event *)(hdr + 1);

	log = 0;
	decision = -1;

	for (i = 0; proc->valid_ctx && i < PE_PRIO_MAX; i++) {
		DEBUG(DBG_PE_DECALF, "pe_decide_alf: prio %d context %p", i,
		    proc->context[i]);

		ret = pe_alf_evaluate(proc->context[i], msg, &log);

		if (ret != -1)
			decision = ret;
		if (ret == POLICY_DENY) {
			break;
		}
	}
	DEBUG(DBG_PE_DECALF, "pe_decide_alf: decision %d", decision);

	/* If no default rule matched, decide on deny */
	if (decision == -1)
		decision = POLICY_DENY;

	if (log != APN_LOG_NONE)
		dump = pe_dump_alfmsg(msg);

	/* Logging */
	switch (log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		log_info("%s %s", verdict[decision], dump);
		send_lognotify(hdr, decision, log);
		break;
	case APN_LOG_ALERT:
		log_warnx("%s %s", verdict[decision], dump);
		send_lognotify(hdr, decision, log);
		break;
	default:
		log_warnx("pe_decide_alf: unknown log type %d", log);
	}

	if (dump)
	    free(dump);

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_decide_alf: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}

	reply->ask = 0;			/* XXX HSH false */
	reply->reply = decision;
	reply->timeout = (time_t)0;
	reply->len = 0;
#else	/* LINUX */
	anoubisd_reply_t	*reply;

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_decide_alf: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}

	reply->ask = 0;
	reply->reply = 0;
	reply->timeout = (time_t)0;
	reply->len = 0;
#endif	/* LINUX */

	return (reply);
}

int
pe_alf_evaluate(struct pe_context *context, struct alf_event *msg, int *log)
{
	struct apn_rule	*rule;
	int		 decision;

	if (context == NULL)
		return (-1);
	if ((rule = context->rule) == NULL) {
		log_warnx("pe_alf_evaluate: empty rule");
		return (-1);
	}
	if (msg == NULL) {
		log_warnx("pe_alf_evaluate:: empty alf event");
		return (-1);
	}

	decision = pe_decide_alffilt(rule, msg, log);
	if (decision == -1)
		decision = pe_decide_alfcap(rule, msg, log);
	if (decision == -1)
		decision = pe_decide_alfdflt(rule, msg, log);

	DEBUG(DBG_PE_DECALF, "pe_alf_evaluate: decision %d rule %p", decision,
	    rule);

	return (decision);
}

/*
 * ALF filter logic.
 */
int
pe_decide_alffilt(struct apn_rule *rule, struct alf_event *msg, int *log)
{
	struct apn_alfrule	*hp;
	int			 decision;

	if (rule == NULL) {
		log_warnx("pe_decide_alffilt: empty rule");
		return (-1);
	}
	if (msg == NULL) {
		log_warnx("pe_decide_alffilt: empty alf event");
		return (-1);
	}

	/* We only decide on UDP and TCP. */
	if (msg->protocol != IPPROTO_UDP && msg->protocol != IPPROTO_TCP)
		return (-1);

	decision = -1;
	for (hp = rule->rule.alf; hp; hp = hp->next) {
		/*
		 * Skip non-filter rules.
		 */
		if (hp->type != APN_ALF_FILTER)
			continue;

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: family %d == %d/%p?",
		    msg->family, hp->rule.afilt.filtspec.af, hp);
		/*
		 * Address family:
		 * If the rule does not specify a certain family (ie.
		 * AF_UNSPEC), we accept any address family.  Otherwise,
		 * families have to match.
		 */
		if (msg->family != hp->rule.afilt.filtspec.af &&
		    hp->rule.afilt.filtspec.af != AF_UNSPEC)
			continue;

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: operation %d",
		    msg->op);
		/*
		 * Operation
		 */
		switch (msg->op) {
		case ALF_ANY:
			/* XXX HSH ? */
			continue;

		case ALF_CONNECT:
			if (hp->rule.afilt.filtspec.netaccess != APN_CONNECT &&
			    hp->rule.afilt.filtspec.netaccess != APN_INOUT)
				continue;
			break;

		case ALF_ACCEPT:
			if (hp->rule.afilt.filtspec.netaccess != APN_ACCEPT &&
			    hp->rule.afilt.filtspec.netaccess != APN_INOUT)
				continue;
			break;

		case ALF_SENDMSG:
		case ALF_RECVMSG:
			break;

		default:
			continue;
		}

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: socket type %d",
		   msg->type);
		/*
		 * Socket type
		 */
		if (msg->type != SOCK_STREAM && msg->type != SOCK_DGRAM)
			continue;

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: protocol %d",
		    msg->protocol);
		/*
		 * Protocol
		 */
		if (msg->protocol != hp->rule.afilt.filtspec.proto)
			continue;

		/*
		 * Check addresses.
		 */
		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: address check op %d",
		    msg->op);
		switch (msg->protocol) {
		case IPPROTO_TCP:
			if (msg->op == ALF_CONNECT) {
				if (pe_addrmatch_out(msg, hp) == 0)
					continue;
			} else if (msg->op == ALF_ACCEPT) {
				if (pe_addrmatch_in(msg, hp) == 0)
					continue;
			} else if (msg->op == ALF_SENDMSG || msg->op ==
			    ALF_RECVMSG) {
				if (pe_addrmatch_out(msg, hp) == 0 &&
				    pe_addrmatch_in(msg, hp) == 0)
					continue;
			} else
				continue;
			break;

		case IPPROTO_UDP:
			if (msg->op == ALF_CONNECT || msg->op == ALF_SENDMSG) {
				if (pe_addrmatch_out(msg, hp) == 0)
					continue;
			} else if (msg->op == ALF_ACCEPT || msg->op ==
			    ALF_RECVMSG) {
				if (pe_addrmatch_in(msg, hp) == 0)
					continue;
			} else
				continue;
			break;

		default:
			continue;
		}

		/*
		 * Decide.
		 */
		switch (hp->rule.afilt.action) {
		case APN_ACTION_ALLOW:
			decision = POLICY_ALLOW;
			break;
		case APN_ACTION_DENY:
			decision = POLICY_DENY;
			break;
		case APN_ACTION_ASK:
			decision = POLICY_ASK;
			break;
		default:
			log_warnx("pe_decide_alffilt: invalid action %d",
			    hp->rule.afilt.action);
			continue;
		}

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: decision %d",
		    decision);

		if (log)
			*log = hp->rule.afilt.filtspec.log;
		break;
	}

	return (decision);
}

/*
 * ALF capability logic.
 */
int
pe_decide_alfcap(struct apn_rule *rule, struct alf_event *msg, int *log)
{
	int			 decision;
#ifdef LINUX
	struct apn_alfrule	*hp;

	if (rule == NULL) {
		log_warnx("pe_decide_alfcap: empty rule");
		return (POLICY_DENY);
	}
	if (msg == NULL) {
		log_warnx("pe_decide_alfcap: empty alf event");
		return (POLICY_DENY);
	}

	/* We decide on everything except UDP and TCP */
	if (msg->protocol == IPPROTO_UDP || msg->protocol == IPPROTO_TCP)
		return (-1);

	decision = -1;
	hp = rule->rule.alf;
	while (hp) {
		/* Skip non-capability rules. */
		if (hp->type != APN_ALF_CAPABILITY) {
			hp = hp->next;
			continue;
		}
		/*
		 * We have three types of capabilities:
		 *
		 * APN_ALF_CAPRAW: We allow packets with socket type SOCK_RAW
		 * and SOCK_PACKET (ie. ping, dhclient)
		 *
		 * APN_ALF_CAPOTHER: We allow everything else.
		 *
		 * APN_ALF_CAPALL: Allow anything.
		 */
		switch (hp->rule.acap.capability) {
		case APN_ALF_CAPRAW:
			if (msg->type == SOCK_RAW || msg->type == SOCK_PACKET)
				decision = POLICY_ALLOW;
			break;
		case APN_ALF_CAPOTHER:
			if (msg->type != SOCK_RAW && msg->type != SOCK_PACKET)
				decision = POLICY_ALLOW;
			break;
		case APN_ALF_CAPALL:
			decision = POLICY_ALLOW;
			break;
		default:
			log_warnx("pe_decide_alfcap: unknown capability %d",
			    hp->rule.acap.capability);
			decision = -1;
		}

		if (decision != -1) {
			if (log)
				*log = hp->rule.acap.log;
			break;
		}
		hp = hp->next;
	}

#else	/* LINUX */
	decision = -1;
#endif	/* LINUX */

	return (decision);
}

/*
 * ALF default logic.
 */
int
pe_decide_alfdflt(struct apn_rule *rule, struct alf_event *msg, int *log)
{
	struct apn_alfrule	*hp;
	int			 decision;

	if (rule == NULL) {
		log_warnx("pe_decide_alfdflt: empty rule");
		return (-1);
	}

	decision = -1;
	hp = rule->rule.alf;
	while (hp) {
		/* Skip non-default rules. */
		if (hp->type != APN_ALF_DEFAULT) {
			hp = hp->next;
			continue;
		}
		/*
		 * Default rules are easy, just pick the specified action.
		 */
		switch (hp->rule.apndefault.action) {
		case APN_ACTION_ALLOW:
			decision = POLICY_ALLOW;
			break;
		case APN_ACTION_DENY:
			decision = POLICY_DENY;
			break;
		default:
		log_warnx("pe_decide_alfdflt: unknown action %d",
			    hp->rule.apndefault.action);
			decision = -1;
		}

		if (decision != -1) {
			if (log)
				*log = hp->rule.apndefault.log;
			break;
		}
		hp = hp->next;
	}

	return (decision);
}

/*
 * Decode an ALF message into a printable string.  This strings is
 * allocated and needs to be freed by the caller.
 */
char *
pe_dump_alfmsg(struct alf_event *msg)
{
#ifdef LINUX
	unsigned short	 lport, pport;
	char		*op, *af, *type, *proto, *dump;

	/*
	 * v4 address: 4 * 3 digits + 3 dots + 1 \0 = 16
	 * v6 address: 16 * 4 digits + 15 colons + 1 \0 = 79
	 *
	 * v6 address might also have some leading colons, etc. so 128
	 * bytes are more than sufficient and safe.
	 */
	char		 local[128], peer[128];

	if (msg == NULL)
		return (NULL);

	pport = lport = 0;
	sprintf(local, "<unknown>");
	sprintf(peer, "<unknown>");

	switch (msg->op) {
	case ALF_ANY:
		op = "any";
		break;
	case ALF_CONNECT:
		op = "connect";
		break;
	case ALF_ACCEPT:
		op = "accept";
		break;
	case ALF_SENDMSG:
		op = "sendmsg";
		break;
	case ALF_RECVMSG:
		op = "recvmsg";
		break;
	default:
		op = "<unknown>";
	}

	switch(msg->family) {
	case AF_INET:
		af = "inet";
		lport = ntohs(msg->local.in_addr.sin_port);
		pport = ntohs(msg->peer.in_addr.sin_port);

		if (inet_ntop(msg->family, &msg->local.in_addr.sin_addr,
		    local, sizeof(local)) == NULL)
			sprintf(local, "<unknown>");
		if (inet_ntop(msg->family, &msg->peer.in_addr.sin_addr, peer,
		    sizeof(peer)) == NULL)
			sprintf(peer, "<unknown>");
		break;

	case AF_INET6:
		af = "inet6";
		lport = ntohs(msg->local.in6_addr.sin6_port);
		pport = ntohs(msg->peer.in6_addr.sin6_port);

		if (inet_ntop(msg->family, &msg->local.in6_addr.sin6_addr,
		    local, sizeof(local)))
			sprintf(local, "<unknown>");
		if (inet_ntop(msg->family, &msg->peer.in6_addr.sin6_addr, peer,
		    sizeof(peer)) == NULL)
			sprintf(peer, "<unknown>");
		break;

	case AF_PACKET:
		af = "packet";
		break;
	case AF_UNSPEC:
		af = "unspec";
		break;
	default:
		af = "<unknown>";
	}
	switch(msg->type) {
	case SOCK_STREAM:
		type = "stream";
		break;
	case SOCK_DGRAM:
		type = "dgram";
		break;
	case SOCK_RAW:
		type = "raw";
		break;
	case SOCK_RDM:
		type = "rdm";
		break;
	case SOCK_SEQPACKET:
		type = "seqpacket";
		break;
	case SOCK_PACKET:
		type = "packet";
		break;
	default:
		type = "<unknown>";
	}
	switch (msg->protocol) {
	case IPPROTO_ICMP:
		proto = "icmp";
		break;
	case IPPROTO_UDP:
		proto = "udp";
		break;
	case IPPROTO_TCP:
		proto = "tcp";
		break;
	default:
		proto = "<unknown>";
	}

	if (asprintf(&dump, "%s (%d): uid %u pid %u %s (%d) %s (%d) %s (%d) "
	    "local %s:%u peer %s:%u", op, msg->op, msg->uid, msg->pid, type,
	    msg->type, proto, msg->protocol, af, msg->family, local, lport,
	    peer, pport) == -1) {
		dump = NULL;
	}
	return (dump);
#else	/* LINUX */
	return (NULL);
#endif	/* LINUX */
}

anoubisd_reply_t *
pe_handle_sfs(struct eventdev_hdr *hdr)
{
	anoubisd_reply_t	*reply = NULL;

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

static struct policy_request * policy_request_find(u_int64_t token)
{
	struct policy_request * req;
	LIST_FOREACH(req, &preqs, next) {
		if (req->token == token)
			return req;
	}
	return NULL;
}

static void put_request(struct policy_request *req)
{
	if (req->tmpname) {
		unlink(req->tmpname);
		free(req->tmpname);
	}
	if (req->realname) {
		free(req->realname);
	}
	if (req->fd >= 0)
		close(req->fd);
	free(req);
}

anoubisd_reply_t *
pe_dispatch_policy(struct anoubisd_msg_comm *comm)
{
	anoubisd_reply_t	*reply = NULL;
	Policy_Generic		*gen;
	Policy_GetByUid		*getbyuid;
	Policy_SetByUid		*setbyuid;
	u_int32_t		uid;
	u_int32_t		prio;
	int			error = 0;
	int			fd;
	struct policy_request	*req;
	char			*buf;
	int			len;

	if (comm == NULL) {
		log_warnx("pe_dispatch_policy: empty comm");
		return (NULL);
	}

	DEBUG(DBG_TRACE, ">pe_dispatch_policy");
	/*
	 * See if we have a policy request for this token. It is an error if
	 *  - there is no request and POLICY_FLAG_START is clear   or
	 *  - there is a request and POLICY_FLAG_START ist set.
	 */
	req = policy_request_find(comm->token);
	if ((req == NULL) == ((comm->flags & POLICY_FLAG_START) == 0)) {
		if (req) {
			error = EBUSY;
		} else {
			error = ESRCH;
		}
		goto err;
	}
	/* No request yet, start one. */
	if (req == NULL) {
		if (comm->len < sizeof(Policy_Generic)) {
			error = EINVAL;
			goto err;
		}
		gen = (Policy_Generic *)comm->msg;
		req = calloc(1, sizeof(struct policy_request));
		if (!req) {
			error = ENOMEM;
			goto err;
		}
		req->token = comm->token;
		req->ptype = get_value(gen->ptype);
		req->authuid = comm->uid;
		req->fd = -1;
		LIST_INSERT_HEAD(&preqs, req, next);
	}
	DEBUG(DBG_TRACE, " pe_dispatch_policy: ptype = %d, flags = %d "
	    "token = %lld", req->ptype, comm->flags, req->token);
	switch (req->ptype) {
	case ANOUBIS_PTYPE_GETBYUID:
		if ((comm->flags & POLICY_FLAG_END) == 0
		    || (comm->len < sizeof(Policy_GetByUid))) {
		    	error = EINVAL;
			goto err;
		}
		getbyuid = (Policy_GetByUid *)comm->msg;
		uid = get_value(getbyuid->uid);
		prio = get_value(getbyuid->prio);
		/*
		 * XXX CEH: Do more/better permission checks here!
		 * XXX CEH: Authorized user ID is in comm->uid.
		 * XXX CEH: Currently this assumes Admin == root == uid 0
		 */
		error = EPERM;
		if (comm->uid != uid && comm->uid != 0)
			goto err;
		fd = pe_open_policy_file(uid, prio);
		if (fd == -ENOENT) {
			error = 0;
			goto err;
		}
		error = EIO;
		if (fd < 0) {
			/* ENOENT indicates an empty policy and not an error. */
			if (fd == -ENOENT)
				error = 0;
			goto err;
		}
		error = - send_policy_data(comm->token, fd);
		close(fd);
		if (error)
			goto err;
		break;
	case ANOUBIS_PTYPE_SETBYUID:
		buf = comm->msg;
		len = comm->len;
		if (comm->flags & POLICY_FLAG_START) {
			if (comm->len < sizeof(Policy_SetByUid)) {
				error = EINVAL;
				goto err;
			}
			error = ENOMEM;
			setbyuid = (Policy_SetByUid *)comm->msg;
			uid = get_value(setbyuid->uid);
			prio = get_value(setbyuid->prio);
			/*
			 * XXX CEH: Do more/better permission checks here!
			 * XXX CEH: Authorized user ID is in comm->uid.
			 * XXX CEH: Currently this assumes Admin = root = uid 0
			 */
			if (uid != req->authuid && req->authuid != 0) {
				error = EPERM;
				goto err;
			}
			if (prio == PE_PRIO_ADMIN && req->authuid != 0) {
				error = EPERM;
				goto err;
			}
			req->realname = pe_policy_file_name(uid, prio);
			if (!req->realname)
				goto err;
			if (asprintf(&req->tmpname, "%s.%lld", req->realname,
			    req->token) == -1) {
			    	req->tmpname = NULL;
				goto err;
			}
			DEBUG(DBG_TRACE, "  open: %s", req->tmpname);
			req->fd = open(req->tmpname,
			    O_WRONLY|O_CREAT|O_EXCL, 0400);
			if (req->fd < 0) {
				error = errno;
				goto err;
			}
			DEBUG(DBG_TRACE, "  open: %s fd = %d", req->tmpname,
			    req->fd);
			buf += sizeof(Policy_SetByUid);
			len -= sizeof(Policy_SetByUid);
		}
		if (req->authuid != comm->uid) {
			error = EPERM;
			goto err;
		}
		while(len) {
			int ret = write(req->fd, buf, len);
			if (ret < 0) {
				DEBUG(DBG_TRACE, "  write error fd=%d",
				    req->fd);
				error = errno;
				goto err;
			}
			buf += ret;
			len -= ret;
		}
		if (comm->flags & POLICY_FLAG_END) {
			if (rename(req->tmpname, req->realname) < 0) {
				error = errno;
				goto err;
			}
			error = 0;
			goto err;
		}
		break;
	default:
		/* Unknown opcode. */
		error = EINVAL;
		goto err;
	}
	if (req && (comm->flags & POLICY_FLAG_END)) {
		LIST_REMOVE(req, next);
		put_request(req);
	}
	DEBUG(DBG_TRACE, "<pe_dispatch_policy (NULL)");
	return NULL;
err:
	if (req) {
		LIST_REMOVE(req, next);
		put_request(req);
	}
	reply = calloc(1, sizeof(struct anoubisd_reply));
	reply->token = comm->token;
	reply->ask = 0;
	reply->len = 0;
	reply->flags = POLICY_FLAG_START|POLICY_FLAG_END;
	reply->timeout = 0;
	reply->reply = error;
	DEBUG(DBG_TRACE, "<pe_dispatch_policy: %d", error);
	return reply;
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
pe_put_proc(struct pe_proc *proc)
{
	int	i;

	if (!proc || --(proc->refcount))
		return;
	if (proc->parent_proc != proc)
		pe_put_proc(proc->parent_proc);
	if (proc->csum)
		free(proc->csum);
	if (proc->pathhint)
		free(proc->pathhint);

	for (i = 0; i < PE_PRIO_MAX; i++)
		pe_put_ctx(proc->context[i]);

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
pe_set_parent_proc(struct pe_proc *proc, struct pe_proc *newparent)
{
	struct pe_proc *oldparent = proc->parent_proc;

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
pe_untrack_proc(struct pe_proc *proc)
{
	if (!proc)
		return;
	TAILQ_REMOVE(&tracker, proc, entry);
	pe_put_proc(proc);
}

/*
 * Inherit the parent process context. This might not necessarily be
 * the direct parent process, but some grand parent.  This happens on
 * fork(2).  Moreover, we set a reference to that particular parent.
 *
 * If our parent was never tracked, we will not inherit a context,
 * thus we have to set one later on exec(2).
 */
void
pe_inherit_ctx(struct pe_proc *proc)
{
	struct pe_proc	*parent;
	int		 i;

	if (proc == NULL) {
		log_warnx("pe_inherit_ctx: empty process");
		return;
	}

	/* Get parents context */
	parent = proc->parent_proc;
	DEBUG(DBG_PE_CTX, "pe_inherit_ctx: parent %p 0x%08llx", parent,
	    parent ? parent->task_cookie : 0);

	if (parent && !parent->valid_ctx) {
		DEBUG(DBG_PE_CTX, "pe_inherit_ctx: parent %p 0x%08llx has no "
		    "context", parent, parent->task_cookie);
	}

	if (parent && parent->valid_ctx) {
		for (i = 0; i < PE_PRIO_MAX; i++) {
			proc->context[i] = parent->context[i];
			pe_reference_ctx(proc->context[i]);
		}
		proc->valid_ctx = 1;
		pe_set_parent_proc(proc, parent);

		pe_dump_ctx("pe_inherit_ctx", proc);
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
	if (proc->valid_ctx) {
		DEBUG(DBG_PE_CTX, "pe_set_ctx: proc %p 0x%08llx has context "
		    "%p rule %p ctx %p", proc, proc->task_cookie,
		    proc->context[0],
		    proc->valid_ctx ? proc->context[0]->rule : NULL,
		    proc->valid_ctx ? proc->context[0]->ctx : NULL);

		if (pe_decide_ctx(proc, csum, pathhint) != 1) {
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
		for (i = 0; i < PE_PRIO_MAX; i++)
			proc->context[i] = NULL;
		proc->valid_ctx = 1;
	} else {
		for (i = 0; i < PE_PRIO_MAX; i++) {
			proc->context[i] = pe_search_ctx(user->prio[i], csum,
			    pathhint);
		}
		proc->valid_ctx = 1;
	}
	pe_set_parent_proc(proc, proc);

	DEBUG(DBG_PE_CTX, "pe_set_ctx: proc %p 0x%08llx got context "
	    "%p rule %p ctx %p", proc, proc->task_cookie,
	    proc->context[0], proc->context[0] ? proc->context[0]->rule : NULL,
	    proc->context[0] ? proc->context[0]->ctx : NULL);
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
		DEBUG(DBG_PE_CTX, "pe_search_ctx: no rule found");
		return (NULL);
	}
	context = pe_alloc_ctx(rule);

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

struct pe_context *
pe_alloc_ctx(struct apn_rule *rule)
{
	struct pe_context	*ctx;

	if ((ctx = calloc(1, sizeof(struct pe_context))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);	/* XXX HSH */
	}
	ctx->rule = rule;
	ctx->refcount = 1;

	return (ctx);
}

void
pe_put_ctx(struct pe_context *ctx)
{
	if (!ctx || --(ctx->refcount))
		return;

	free(ctx);
}

void
pe_reference_ctx(struct pe_context *ctx)
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
int
pe_decide_ctx(struct pe_proc *proc, const u_int8_t *csum, const char *pathhint)
{
	struct apn_app	*hp;
	int		 cmp, decision, i;

	if (!proc->valid_ctx || csum == NULL || pathhint == NULL)
		return (0);

	/*
	 * Iterate over all priorities, starting with the highest.
	 * If a priority does not provide a context, we just continue.
	 *
	 * I f a priority provides a context, we have to evaluate that one.
	 * If switiching is not allowed we stop return the decision.
	 * Other wise we continue with the evaluation.
	 */
	decision = -1;
	for (i = 0; i < PE_PRIO_MAX; i++) {
		/* No context means, no decision, just go on. */
		if (proc->context[i] == NULL) {
			continue;
		}
		/* Context without rule means, not switching. */
		if (proc->context[i]->ctx == NULL) {
			decision = 0;
			break;
		}
		/*
		 * If the rule says "context new any", application will be
		 * empty.  In that case, initialize the compare result cmp to 0
		 * (ie. match).
		 */
		hp = proc->context[i]->ctx->application;
		if (hp == NULL)
			cmp = 0;
		while (hp) {
			cmp = bcmp(hp->hashvalue, csum, sizeof(hp->hashvalue));
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
			    "pe_decide_ctx: found \"%s\" csm 0x%08x... for "
			    "priority %d", (hp && hp->name) ? hp->name : "any"
			    , (hp && hp->hashvalue) ?
			    htonl(*(unsigned long *)hp->hashvalue) : 0, i);
		} else
			decision = 0;
	}

	return (decision);
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
		    "pathhint \"%s\" ctx %p rules %p %p", proc,
		    proc->task_cookie, (int)proc->pid, proc->csum ?
		    htonl(*(unsigned long *)proc->csum) : 0,
		    proc->pathhint ? proc->pathhint : "", proc->context[0],
		    proc->context[0] ? proc->context[0]->rule : NULL,
		    proc->context[1] ? proc->context[1]->rule : NULL);
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

void
pe_dump_ctx(const char *prefix, struct pe_proc *proc)
{
	int	i;

	if (proc == NULL) {
		DEBUG(DBG_PE_CTX, "%s: prio %d rule %p ctx %p", prefix, -1,
		    NULL, NULL);
		return;
	}

	for (i = 0; i < PE_PRIO_MAX; i++) {
		DEBUG(DBG_PE_CTX, "%s: prio %d rule %p ctx %p", prefix, i,
		    proc->context[i] ? proc->context[i]->rule : NULL,
		    proc->context[i] ? proc->context[i]->ctx : NULL);
	}
}

/*
 * Check outgoing connection. 1 on match, 0 otherwise.
 */
int
pe_addrmatch_out(struct alf_event *msg, struct apn_alfrule *rule)
{
	struct apn_host	*fromhost, *tohost;
	struct apn_port	*fromport, *toport;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_out: msg %p rule %p", msg, rule);

	if (rule == NULL) {
		log_warnx("pe_addrmatch_out: empty rule");
		return (0);
	}
	if (msg == NULL) {
		log_warnx("pe_addrmatch_out: empty event");
		return (0);
	}

	fromhost = rule->rule.afilt.filtspec.fromhost;
	fromport = rule->rule.afilt.filtspec.fromport;;
	tohost = rule->rule.afilt.filtspec.tohost;
	toport = rule->rule.afilt.filtspec.toport;

	/*
	 * fromhost == local?
	 */
	if (pe_addrmatch_host(fromhost, (void *)&msg->local, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(fromport, (void *)&msg->local, msg->family) == 0)
		return (0);

	/*
	 * tohost == peer?
	 */
	if (pe_addrmatch_host(tohost, (void *)&msg->peer, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(toport, (void *)&msg->peer, msg->family) == 0)
		return (0);

	return (1);
}

/*
 * Check incoming connection. 1 on match, 0 otherwise.
 */
int
pe_addrmatch_in(struct alf_event *msg, struct apn_alfrule *rule)
{
	struct apn_host	*fromhost, *tohost;
	struct apn_port	*fromport, *toport;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_in: msg %p rule %p", msg, rule);

	if (rule == NULL) {
		log_warnx("pe_addrmatch_out: empty rule");
		return (0);
	}
	if (msg == NULL) {
		log_warnx("pe_addrmatch_out: empty event");
		return (0);
	}

	fromhost = rule->rule.afilt.filtspec.fromhost;
	fromport = rule->rule.afilt.filtspec.fromport;;
	tohost = rule->rule.afilt.filtspec.tohost;
	toport = rule->rule.afilt.filtspec.toport;

	/* fromhost == peer? */
	if (pe_addrmatch_host(fromhost, (void *)&msg->peer, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(fromport, (void *)&msg->peer, msg->family) == 0)
		return (0);

	/* tohost == local? */
	if (pe_addrmatch_host(tohost, (void *)&msg->local, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(toport, (void *)&msg->local, msg->family) == 0)
		return (0);

	return (1);
}

/*
 * Compare addresses. 1 on match, 0 otherwise.
 * XXX HSH masks for inet6
 */
int
pe_addrmatch_host(struct apn_host *host, void *addr, unsigned short af)
{
	struct apn_host		*hp;
	struct sockaddr_in	*in;
	struct sockaddr_in6	*in6;
	in_addr_t		 mask;
	int			 cmp;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_host: host %p addr %p af %d", host,
	    addr, af);

	/* Empty host means "any", ie. match always. */
	if (host == NULL)
		return (1);

	if (addr == NULL) {
		log_warnx("pe_addrmatch_host: no address specified");
		return (0);
	}

	if (host->addr.af != af)
		return (0);

	hp = host;
	while (hp) {
		switch (af) {
		case AF_INET:
			in = (struct sockaddr_in *)addr;
			mask = htonl(~0UL << (32 - hp->addr.len));
			cmp = ((in->sin_addr.s_addr & mask) ==
			    (hp->addr.apa.v4.s_addr & mask));
			break;

		case AF_INET6:
			/* XXX HSH no network addresses, yet! */
			in6 = (struct sockaddr_in6 *)addr;
			cmp = bcmp(&in6->sin6_addr, &hp->addr.apa.v6,
			    sizeof(struct in6_addr));
			break;

		default:
			log_warnx("pe_addrmatch_host: unknown address family "
			    "%d", af);
			cmp = 0;
		}
		hp = hp->next;
	}

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_host: cmp %d", cmp);

	return (cmp);
}

int
pe_addrmatch_port(struct apn_port *port, void *addr, unsigned short af)
{
	struct apn_port		*pp;
	struct sockaddr_in	*in;
	struct sockaddr_in6	*in6;
	int			 match;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_port: port %p addr %p af %d", port,
	    addr, af);

	/* Empty port means "any", ie. match always. */
	if (port == NULL)
		return (1);

	if (addr == NULL) {
		log_warnx("pe_addrmatch_port: no address specified");
		return (0);
	}

	pp = port;
	while (pp) {
		switch (af) {
		case AF_INET:
			in = (struct sockaddr_in *)addr;
			match = (in->sin_port == pp->port);
			break;

		case AF_INET6:
			in6 = (struct sockaddr_in6 *)addr;
			match = (in6->sin6_port == pp->port);
			break;

		default:
			log_warnx("pe_addrmatch_port: unknown address "
			    "family %d", af);
			match = 0;
		}
		pp = pp->next;
	}

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_port: match %d", match);

	return (match);
}
/*@=memchecks@*/

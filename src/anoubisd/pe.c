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
#include <sys/stat.h>
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

#include <openssl/pem.h>

#ifdef OPENBSD
#ifndef s6_addr32
#define s6_addr32 __u6_addr.__u6_addr32
#endif
#endif

#ifdef LINUX
#include <queue.h>
#include <bsdcompat.h>
#include <linux/anoubis_sfs.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/queue.h>
#include <dev/anoubis_alf.h>
#include <dev/anoubis_sfs.h>
#include <dev/anoubis.h>
#endif

#ifdef LINUX
#include <linux/anoubis_sfs.h>
#include <linux/anoubis_alf.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis_alf.h>
#include <dev/anoubis_sfs.h>
#define POLICY_ALLOW	0
#define POLICY_DENY	1
#define POLICY_ASK	2
#endif

#include <anoubis_protocol.h>

#include "anoubisd.h"
#include "sfs.h"
#include "kernelcache.h"

struct pe_user {
	TAILQ_ENTRY(pe_user)	 entry;
	uid_t			 uid;

#define PE_PRIO_ADMIN	0
#define PE_PRIO_USER1	1
#define PE_PRIO_MAX	2
	struct apn_ruleset	*prio[PE_PRIO_MAX];
};
TAILQ_HEAD(policies, pe_user) *pdb;

struct pe_pubkey {
	TAILQ_ENTRY(pe_pubkey)	 entry;
	uid_t			 uid;
	EVP_PKEY		*pubkey;
};
TAILQ_HEAD(pubkeys, pe_pubkey) *pubkeys;

struct pe_context {
	int			 refcount;
	struct apn_rule		*rule;
	struct apn_context	*ctx;
	struct apn_ruleset	*ruleset;
};

struct pe_proc {
	TAILQ_ENTRY(pe_proc)	 entry;
	int			 refcount;
	pid_t			 pid;
	uid_t			 uid;

	u_int8_t		*csum;
	char			*pathhint;
	anoubis_cookie_t	 task_cookie;
	struct pe_proc		*parent_proc;

	/* Per priority contexts */
	struct pe_context	*context[PE_PRIO_MAX];
	int			 valid_ctx;

	/* Pointer to kernel cache */
	struct anoubis_kernel_policy_header	*kcache;
};
TAILQ_HEAD(tracker, pe_proc) tracker;

static char * prio_to_string[PE_PRIO_MAX] = {
#ifndef lint
	[ PE_PRIO_ADMIN ] = ANOUBISD_POLICYCHROOT "/" ANOUBISD_ADMINDIR,
	[ PE_PRIO_USER1 ] = ANOUBISD_POLICYCHROOT "/" ANOUBISD_USERDIR,
#endif
};

struct policy_request {
	LIST_ENTRY(policy_request) next;
	u_int64_t	 token;
	u_int32_t	 ptype;
	u_int32_t	 authuid;
	int		 fd;
	int		 prio;
	uid_t		 uid;
	char		*tmpname;
	char		*realname;
};
LIST_HEAD(, policy_request) preqs;

anoubisd_reply_t	*policy_engine(anoubisd_msg_t *);
anoubisd_reply_t	*pe_dispatch_event(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_process(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_sfsexec(struct eventdev_hdr *);
anoubisd_reply_t	*pe_handle_alf(struct eventdev_hdr *);
anoubisd_reply_t	*pe_decide_alf(struct pe_proc *, struct eventdev_hdr *);
int			 pe_alf_evaluate(struct pe_proc *, struct pe_context *,
			     struct alf_event *, int *, u_int32_t *);
int			 pe_decide_alffilt(struct pe_proc *, struct apn_rule *,
			     struct alf_event *, int *, u_int32_t *, time_t);
int			 pe_decide_alfcap(struct apn_rule *, struct
			     alf_event *, int *, u_int32_t *, time_t);
int			 pe_decide_alfdflt(struct apn_rule *, struct
			     alf_event *, int *, u_int32_t *, time_t);
void			 pe_kcache_alf(struct apn_alfrule *, int,
			     struct pe_proc *, struct alf_event *);
char			*pe_dump_alfmsg(struct alf_event *,
			     struct eventdev_hdr *, int);
char			*pe_dump_ctx(struct eventdev_hdr *, struct pe_proc *,
			     int);
anoubisd_reply_t	*pe_handle_sfs(anoubisd_msg_sfsopen_t *);
anoubisd_reply_t	*pe_decide_sfs(struct pe_user *,
			    anoubisd_msg_sfsopen_t*, time_t now);
int			 pe_decide_sfscheck(struct apn_rule *, struct
			     sfs_open_message *, int *, u_int32_t *, time_t);
int			 pe_decide_sfsdflt(struct apn_rule *, struct
			     sfs_open_message *, int *, u_int32_t *, time_t);
anoubisd_reply_t	*pe_dispatch_policy(struct anoubisd_msg_comm *);
char			*pe_dump_sfsmsg(struct sfs_open_message *, int);

struct pe_proc		*pe_get_proc(anoubis_cookie_t);
void			 pe_put_proc(struct pe_proc * proc);
struct pe_proc		*pe_alloc_proc(uid_t, anoubis_cookie_t,
			     anoubis_cookie_t);
void			 pe_track_proc(struct pe_proc *);
void			 pe_untrack_proc(struct pe_proc *);
void			 pe_set_parent_proc(struct pe_proc * proc,
			     struct pe_proc * newparent);
int			 pe_load_db(struct policies *);
void			 pe_flush_db(struct policies *);
int			 pe_update_db(struct policies *, struct policies *);
void			 pe_update_db_one(struct apn_ruleset *, uid_t, int);
int			 pe_update_ctx(struct pe_proc *, struct pe_context **,
			     int, struct policies *);
int			 pe_replace_rs(struct apn_ruleset *, uid_t, int);
void			 pe_flush_tracker(void);
int			 pe_load_dir(const char *, int, struct policies *);
static int		 pe_clean_policy(const char *, const char *, int);
struct apn_ruleset	*pe_load_policy(const char *, int flags);
int			 pe_insert_rs(struct apn_ruleset *, uid_t, int,
			     struct policies *);
struct pe_user		*pe_get_user(uid_t, struct policies *);
struct apn_ruleset	*pe_get_ruleset(uid_t, int, struct policies *);
void			 pe_inherit_ctx(struct pe_proc *);
void			 pe_set_ctx(struct pe_proc *, uid_t, const u_int8_t *,
			     const char *);
struct pe_context	*pe_search_ctx(struct apn_ruleset *, const u_int8_t *,
			     const char *);
struct pe_context	*pe_alloc_ctx(struct apn_rule *, struct apn_ruleset *);
void			 pe_put_ctx(struct pe_context *);
void			 pe_reference_ctx(struct pe_context *);
int			 pe_decide_ctx(struct pe_proc *, const u_int8_t *,
			     const char *);
void			 pe_dump_dbgctx(const char *, struct pe_proc *);
int			 pe_addrmatch_out(struct alf_event *, struct
			     apn_alfrule *);
int			 pe_addrmatch_in(struct alf_event *, struct
			     apn_alfrule *);
int			 pe_addrmatch_host(struct apn_host *, void *,
			     unsigned short);
int			 pe_addrmatch_port(struct apn_port *, void *,
			     unsigned short);
static char		*pe_policy_file_name(uid_t uid, int prio);
static int		 pe_open_policy_file(uid_t uid, int prio);
static int		 pe_in_scope(struct apn_scope *,
			     struct anoubis_event_common *, time_t);
int			 pe_load_pubkeys(const char *, struct pubkeys *);
void			 pe_flush_pubkeys(struct pubkeys *);
int			 pe_verify_sig(const char *, uid_t);
EVP_PKEY		*pe_get_pubkey(uid_t);

/*
 * The following code utilizes list functions from BSD queue.h, which
 * cannot be reliably annotated. We therefore exclude the following
 * functions from memory checking.
 */
/*@-memchecks@*/
void
pe_init(void)
{
	struct policies	*pp;
	struct pubkeys	*pk;
	int		 count;

	TAILQ_INIT(&tracker);
	LIST_INIT(&preqs);

	if ((pp = calloc(1, sizeof(struct policies))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);	/* XXX HSH */
	}
	TAILQ_INIT(pp);
	if ((pk = calloc(1, sizeof(struct pubkeys))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);	/* XXX HSH */
	}
	TAILQ_INIT(pk);

	/* We die gracefully if loading of public keys fails. */
	if ((count = pe_load_pubkeys(ANOUBISD_PUBKEYDIR, pk)) == -1)
		fatal("pe_init: failed to load policy keys");
	pubkeys = pk;

	/* We die gracefully if loading fails. */
	if ((count = pe_load_db(pp)) == -1)
		fatal("pe_init: failed to initialize policy database");
	pdb = pp;

	log_info("pe_init: %d policies loaded to pdb %p", count, pp);
}

void
pe_shutdown(void)
{
	pe_flush_tracker();
	pe_flush_db(pdb);
	pe_flush_pubkeys(pubkeys);
}

void
pe_reconfigure(void)
{
	struct pubkeys	*newpk, *oldpk;
	struct policies	*newpdb, *oldpdb;
	int		 count;

	if ((newpk = calloc(1, sizeof(struct pubkeys))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);       /* XXX HSH */
	}
	TAILQ_INIT(newpk);

	if ((newpdb = calloc(1, sizeof(struct policies))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);       /* XXX HSH */
	}
	TAILQ_INIT(newpdb);

	if ((count = pe_load_pubkeys(ANOUBISD_PUBKEYDIR, newpk)) == -1) {
		log_warnx("pe_reconfigure: could not load public keys");
		free(newpk);
		free(newpdb);
		return;
	}

	/*
	 * We switch pubkeys right now, so newly loaded policies can
	 * be verfied using the most recent keys.
	 */
	oldpk = pubkeys;
	pubkeys = newpk;
	pe_flush_pubkeys(oldpk);

	if ((count = pe_load_db(newpdb)) == -1) {
		log_warnx("pe_reconfigure: could not reload policy database");
		free(newpdb);
		return;
	}

	/* Switch to new policy database */
	oldpdb = pdb;
	pdb = newpdb;

	if (pe_update_db(newpdb, oldpdb) == -1) {
		log_warnx("pe_reconfigure: database update failed");
		pe_flush_db(newpdb);
		return;
	}

	pe_flush_db(oldpdb);

	log_info("pe_reconfigure: loaded %d policies to new pdb %p, "
	    "flushed old pdb %p", count, newpdb, oldpdb);
}

void
pe_flush_db(struct policies *ppdb)
{
	struct pe_user	*p, *pnext;
	int		 i;

	for (p = TAILQ_FIRST(ppdb); p != TAILQ_END(ppdb); p = pnext) {
		pnext = TAILQ_NEXT(p, entry);
		TAILQ_REMOVE(ppdb, p, entry);

		for (i = 0; i < PE_PRIO_MAX; i++)
			apn_free_ruleset(p->prio[i]);
		free(p);
	}
}

int
pe_update_db(struct policies *newpdb, struct policies *oldpdb)
{
	struct pe_proc		*pproc;
	struct pe_context	*newctx;
	int			 i;

	if (newpdb == NULL || oldpdb == NULL) {
		log_warnx("pe_update_db: empty database pointers");
		return (-1);
	}

	TAILQ_FOREACH(pproc, &tracker, entry) {
		if (pproc->kcache != NULL) {
			pproc->kcache = kernelcache_clear(pproc->kcache);
			kernelcache_send2master(pproc->kcache, pproc->pid);
		}
		for (i = 0; i < PE_PRIO_MAX; i++) {
			DEBUG(DBG_PE_POLICY, "pe_update_db: proc %p prio %d "
			    "context %p", pproc, i, pproc->context[i]);

			if (pe_update_ctx(pproc, &newctx, i, newpdb) == -1)
				return (-1);
			pe_put_ctx(pproc->context[i]);
			pproc->context[i] = newctx;
		}
	}

	return (0);
}

/*
 * This function is not allowed to fail! It must remove all references
 * to the old ruleset from tracked processes.
 * NOTE: If pproc->context[prio] is not NULL then pproc->context[prio]->ruleset
 *       cannot be NULL either. oldrs == NULL is allowed, however.
 */
void
pe_update_db_one(struct apn_ruleset *oldrs, uid_t uid, int prio)
{
	struct pe_context	*newctx, *oldctx;
	struct pe_proc		*pproc;

	DEBUG(DBG_TRACE, ">pe_update_db_one");
	TAILQ_FOREACH(pproc, &tracker, entry) {
		if (pproc->context[prio] == NULL)
			continue;
		/*
		 * XXX CEH: For now we change the rules of a process if
		 * XXX CEH:  - it is running with rules from the ruleset that
		 * XXX CEH:    is being replaced   or
		 * XXX CEH:  - its registered user ID matches that of the
		 * XXX CEH:    user that is replacing its rules.
		 */
		if (pproc->uid != uid
		    && pproc->context[prio]->ruleset != oldrs)
			continue;
		if (pproc->context[prio]->ruleset != oldrs)
			continue;
		if (pe_update_ctx(pproc, &newctx, prio, pdb) == -1) {
			log_warn("Failed to replace context");
			newctx = NULL;
		}
		oldctx = pproc->context[prio];
		DEBUG(DBG_TRACE, " pe_update_db_one: old=%x new=%x",
		    oldctx, newctx);
		pproc->context[prio] = newctx;
		pe_put_ctx(oldctx);
		if (pproc->kcache != NULL) {
			pproc->kcache = kernelcache_clear(pproc->kcache);
			kernelcache_send2master(pproc->kcache, pproc->pid);
		}
		if (pproc->csum && pproc->pathhint) {
			pe_set_ctx(pproc, pproc->uid, pproc->csum,
			    pproc->pathhint);
		}
	}
	DEBUG(DBG_TRACE, "<pe_update_db_one");
}

/*
 * Update a context.
 * If the members rule and ctx of struct pe_context are set, they
 * reference rules in the old pdb.  If similar rules are available in
 * the new pdb (ie. checksum of the application can be found), update
 * the references.
 *
 * Otherwise, reset them to NULL.
 */
int
pe_update_ctx(struct pe_proc *pproc, struct pe_context **newctx, int prio,
    struct policies *newpdb)
{
	struct apn_ruleset	*newrules;
	struct pe_context	*context;

	if (pproc == NULL) {
		log_warnx("pe_update_ctx: empty process");
		return (-1);
	}
	if (newctx == NULL) {
		log_warnx("pe_update_ctx: invalid new context pointer");
		return (-1);
	}
	if (prio < 0 || prio >= PE_PRIO_MAX) {
		log_warnx("pe_update_ctx: illegal priority %d", prio);
		return (-1);
	}
	if (newpdb == NULL) {
		log_warnx("pe_update_ctx: empty database pointers %p", newpdb);
		return (-1);
	}

	/*
	 * Try to get policies for our uid from the new pdb.  If the
	 * new pdb does not provide policies for our uid, try to get the
	 * default policies.
	 */

	DEBUG(DBG_TRACE, ">pe_update_ctx");
	context = NULL;
	newrules = pe_get_ruleset(pproc->uid, prio, newpdb);
	DEBUG(DBG_TRACE, " pe_update_ctx: newrules = %x", newrules);
	if (newrules) {
		u_int8_t		*csum = NULL;
		char			*pathhint = NULL;
		struct apn_rule		*oldrule;
		struct apn_app		*oldapp;

		/*
		 * XXX CEH: In some cases we might want to look for
		 * XXX CEH: a new any rule if the old context for the priority
		 * XXX CEH: or its associated ruleset is NULL.
		 */
		if (pproc->context[prio] == NULL)
			goto out;
		oldrule = pproc->context[prio]->rule;
		if (!oldrule)
			goto out;
		oldapp = oldrule->app;
		if (oldapp) {
			if (oldapp->hashtype == APN_HASH_SHA256)
				csum = oldapp->hashvalue;
			pathhint = oldapp->name;
		}
		context = pe_search_ctx(newrules, csum, pathhint);
	}
out:

	DEBUG(DBG_PE_POLICY, "pe_update_ctx: context %p rule %p ctx %p",
	    context, context ? context->rule : NULL, context ? context->ctx :
	    NULL);

	*newctx = context;

	return (0);
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
pe_load_db(struct policies *p)
{
	int	count = 0;

	if (p == NULL) {
		log_warnx("pe_load_db: bogus database pointer");
		return (-1);
	}

	/* load admin policies */
	count = pe_load_dir(ANOUBISD_POLICYCHROOT "/" ANOUBISD_ADMINDIR,
	    PE_PRIO_ADMIN, p);

	/* load user policies */
	count += pe_load_dir(ANOUBISD_POLICYCHROOT "/" ANOUBISD_USERDIR,
	    PE_PRIO_USER1, p);

	return (count);
}

int
pe_load_dir(const char *dirname, int prio, struct policies *p)
{
	DIR			*dir;
	struct dirent		*dp;
	struct apn_ruleset	*rs;
	int			 count;
	uid_t			 uid;
	const char		*errstr;
	char			*filename, *t;
	int			 flags = APN_FLAG_NOSCOPE;

	DEBUG(DBG_PE_POLICY, "pe_load_dir: %s %p", dirname, p);

	if (prio < 0 || prio >= PE_PRIO_MAX) {
		log_warnx("pe_load_dir: illegal priority %d", prio);
		return (0);
	}

	if (p == NULL) {
		log_warnx("pe_load_dir: illegal database");
		return (0);
	}

	if ((dir = opendir(dirname)) == NULL) {
		log_warn("opendir");
		return (0);
	}

	if (prio != PE_PRIO_USER1)
		flags |= APN_FLAG_NOASK;

	count = 0;

	/* iterate over directory entries */
	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_type != DT_REG)
			continue;

		/* Skip files starting with a dot. */
		if (dp->d_name[0] == '.')
			continue;
		/*
		 * Validate the file name: It has to be either
		 * ANOUBISD_DEFAULTNAME or a numeric uid.
		 * Signatures have the format "<uid>.sig", we just
		 * skip those.
		 */
		if (strcmp(dp->d_name, ANOUBISD_DEFAULTNAME) == 0)
			uid = -1;
		else if ((t = strrchr(dp->d_name, '.')) != NULL &&
		    strcmp(t, ".sig") == 0) {
			continue;
		} else {
			uid = strtonum(dp->d_name, 0, UID_MAX, &errstr);
			if (errstr) {
				log_warnx("pe_load_dir: filename \"%s/%s\" %s",
				    dirname, dp->d_name, errstr);
				continue;
			}
		}
		if (asprintf(&filename, "%s/%s", dirname, dp->d_name) == -1) {
			log_warnx("asprintf: Out of memory");
			continue;
		}
		/*
		 * Only clean user policies.  Temporary rules in admin
		 * policies are not allowed.
		 */
		if (prio == PE_PRIO_USER1 &&
		    pe_clean_policy(filename, dirname, 1) != 0) {
			log_warnx("Cannot clean %s", filename);
			free(filename);
			continue;
		}
		/* XXX Right now, only use administrators key, ie. uid == 0 */
		if (pe_verify_sig(filename, 0) != 1) {
			log_warnx("not loading \"%s\", invalid siganture",
			    filename);
			free(filename);
			continue;
		}
		rs = pe_load_policy(filename, flags);
		free(filename);

		/* If parsing fails, we just continue */
		if (rs == NULL)
			continue;

		if (pe_insert_rs(rs, uid, prio, p) != 0) {
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

static int
scope_check(struct apn_scope * scope, void * data)
{
	time_t		now = *(time_t *)data;

	/* now == 0 means clean all scopes. */
	if (!now)
		return 1;
	/* Clean it if the scope has a timeout and it has expired. */
	if (scope->timeout && now > scope->timeout)
		return 1;
	/* Clean it if the scope has a task and the task does no exist */
	if (scope->task) {
		struct pe_proc *proc = pe_get_proc(scope->task);
		if (!proc)
			return 1;
		pe_put_proc(proc);
	}
	/* Keep it. */
	return 0;
}

static int
pe_clean_policy(const char *filename, const char *dirname, int all)
{
	time_t			 now = 0;
	int			 err = -1;
	struct apn_ruleset	*rs = NULL;
	char			*tmpname = NULL;
	FILE			*tmp = NULL;

	if (!all) {
		if (time(&now) == (time_t)-1) {
			int err = errno;
			log_warn("Cannot get current time");
			master_terminate(err);
			return -1;
		}
	}
	if (apn_parse(filename, &rs, 0)) {
		log_warnx("apn_parse: Parsing failed");
		goto out;
	}
	if (apn_clean_ruleset(rs, &scope_check, &now)) {
		/* Something changed. Dump modified rules. */
		mode_t	oldmask;
		if (dirname) {
			if (asprintf(&tmpname, "%s/.cleantmp", dirname) == -1) {
				log_warnx("asprintf: Out of memory");
				goto out;
			}
			oldmask = umask(0077);
			tmp = fopen(tmpname, "w");
		} else {
			oldmask = umask(0077);
			tmp = fopen(filename, "w");
		}
		umask(oldmask);
		if (!tmp) {
			log_warnx("fopen");
			goto out;
		}
		if (apn_print_ruleset(rs, 0, tmp)) {
			log_warnx("apn_print_ruleset failed");
			if (tmpname)
				unlink(tmpname);
			goto out;
		}
		if (tmpname && rename(tmpname, filename) < 0) {
			log_warn("rename");
			unlink(tmpname);
			goto out;
		}
	}
	err = 0;
out:
	if (rs)
		apn_free_ruleset(rs);
	if (tmp)
		fclose(tmp);
	if (tmpname)
		free(tmpname);
	return err;
}

struct apn_ruleset *
pe_load_policy(const char *name, int flags)
{
	struct apn_ruleset	*rs;
	int			 ret;

	DEBUG(DBG_PE_POLICY, "pe_load_policy: %s", name);

	ret = apn_parse(name, &rs, flags);
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
pe_insert_rs(struct apn_ruleset *rs, uid_t uid, int prio, struct policies *p)
{
	struct pe_user		*user;

	if (rs == NULL) {
		log_warnx("pe_insert_rs: empty ruleset");
		return (1);
	}
	if (p == NULL) {
		log_warnx("pe_insert_rs: empty database pointer");
		return (1);
	}
	if (prio < 0 || prio >= PE_PRIO_MAX) {
		log_warnx("pe_insert_rs: illegal priority %d", prio);
		return (1);
	}

	/* Find or create user */
	if ((user = pe_get_user(uid, p)) == NULL) {
		if ((user = calloc(1, sizeof(struct pe_user))) == NULL) {
			log_warn("calloc");
			master_terminate(ENOMEM);	/* XXX HSH */
		}
		user->uid = uid;
		TAILQ_INSERT_TAIL(p, user, entry);
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

int
pe_replace_rs(struct apn_ruleset *rs, uid_t uid, int prio)
{
	struct apn_ruleset	*oldrs;
	struct pe_user		*user;

	if (rs == NULL) {
		log_warnx("pe_replac_rs: empty ruleset");
		return (1);
	}
	if (prio < 0  || prio >= PE_PRIO_MAX) {
		log_warnx("pe_replace_rs: illegal priority %d", prio);
		return (1);
	}
	DEBUG(DBG_TRACE, ">pe_replace_rs");
	if ((user = pe_get_user(uid, pdb)) == NULL) {
		user = calloc(1, sizeof(struct pe_user));
		if (user == NULL) {
			log_warn("calloc");
			master_terminate(ENOMEM);
			return 1;
		}
		user->uid = uid;
		TAILQ_INSERT_TAIL(pdb, user, entry);
	}
	oldrs = user->prio[prio];
	user->prio[prio] = rs;
	pe_update_db_one(oldrs, uid, prio);
	apn_free_ruleset(oldrs);
	DEBUG(DBG_TRACE, "<pe_replace_rs");
	return 0;
}

struct pe_user *
pe_get_user(uid_t uid, struct policies *p)
{
	struct pe_user	*puser, *user;

	if (p == NULL) {
		log_warnx("pe_get_user: bogus database pointer");
		return (NULL);
	}

	user = NULL;
	TAILQ_FOREACH(puser, p, entry) {
		if (puser->uid != uid)
			continue;
		user = puser;
		break;
	}

	DEBUG(DBG_PE_POLICY, "pe_get_user: uid %d user %p", (int)uid, user);

	return (user);
}

struct apn_ruleset *
pe_get_ruleset(uid_t uid, int prio, struct policies *p)
{
	struct pe_user *user;

	DEBUG(DBG_TRACE, " pe_get_ruleset");
	user = pe_get_user(uid, p);
	if (user && user->prio[prio])
		return user->prio[prio];
	user = pe_get_user(-1, p);
	if (!user)
		return NULL;
	return user->prio[prio];
}

static char *
__pe_policy_file_name(uid_t uid, int prio, char *pre)
{
	char *name;

	if (asprintf(&name, "/%s/%s%d", prio_to_string[prio], pre, uid) == -1)
		return NULL;
	return name;
}

static char *
pe_policy_file_name(uid_t uid, int prio)
{
	return __pe_policy_file_name(uid, prio, "");
}

static int
pe_open_policy_file(uid_t uid, int prio)
{
	int err, fd;
	char	*name = pe_policy_file_name(uid, prio);

	if (!name)
		return -ENOMEM;
	fd = open(name, O_RDONLY);
	err = errno;
	free(name);
	if (fd >= 0)
		return fd;
	if (err != ENOENT)
		return -err;

	err = asprintf(&name, "/%s/%s", prio_to_string[prio],
	    ANOUBISD_DEFAULTNAME);
	if (err == -1)
		return -ENOMEM;
	fd = open(name, O_RDONLY);
	free(name);
	if (fd >= 0)
		return fd;
	return -errno;
}

anoubisd_reply_t *
policy_engine(anoubisd_msg_t *request)
{
	anoubisd_reply_t *reply;

	DEBUG(DBG_TRACE, ">policy_engine");

	switch (request->mtype) {
	case ANOUBISD_MSG_EVENTDEV: {
		struct eventdev_hdr *hdr = (struct eventdev_hdr *)request->msg;
		reply = pe_dispatch_event(hdr);
		break;
	} case ANOUBISD_MSG_SFSOPEN: {
		anoubisd_msg_sfsopen_t *sfsmsg;
		sfsmsg = (anoubisd_msg_sfsopen_t *)request->msg;
		reply = pe_handle_sfs(sfsmsg);
		break;
	} case ANOUBISD_MSG_POLREQUEST: {
		anoubisd_msg_comm_t *comm = (anoubisd_msg_comm_t *)request->msg;
		reply = pe_dispatch_policy(comm);
		break;
	} default:
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
	if (msg->flags & ANOUBIS_OPEN_FLAG_CSUM) {
		if (proc->csum == NULL) {
			proc->csum = calloc(1, sizeof(msg->csum));
			if (proc->csum == NULL) {
				log_warn("calloc");
				master_terminate(ENOMEM);	/* XXX HSH */
			}
		}
		bcopy(msg->csum, proc->csum, sizeof(msg->csum));
	} else {
		if (proc->csum) {
			free(proc->csum);
			proc->csum = NULL;
		}
	}
	if (proc->pathhint) {
		free(proc->pathhint);
		proc->pathhint = NULL;
	}
	if (msg->flags & ANOUBIS_OPEN_FLAG_PATHHINT) {
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
	return (NULL);
}

anoubisd_reply_t *
pe_handle_process(struct eventdev_hdr *hdr)
{
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
		proc = pe_alloc_proc(hdr->msg_uid, msg->task_cookie,
		    msg->common.task_cookie);
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
		proc = pe_alloc_proc(hdr->msg_uid, msg->common.task_cookie, 0);
		pe_set_ctx(proc, hdr->msg_uid, NULL, NULL);
		pe_track_proc(proc);
	}

	if (proc->pid == -1)
		proc->pid = hdr->msg_pid;

	reply = pe_decide_alf(proc, hdr);
	pe_put_proc(proc);

	DEBUG(DBG_TRACE, "<policy_engine");
	return (reply);
}

anoubisd_reply_t *
pe_decide_alf(struct pe_proc *proc, struct eventdev_hdr *hdr)
{
	static char		 prefix[] = "ALF";
	static char		*verdict[3] = { "allowed", "denied", "asked" };
	struct alf_event	*msg;
	anoubisd_reply_t	*reply;
	int			 i, ret, decision, log, prio;
	u_int32_t		 rule_id = 0;
	char			*dump = NULL;
	char			*context = NULL;

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
	prio = -1;
	rule_id = 0;
	decision = -1;

	for (i = 0; proc->valid_ctx && i < PE_PRIO_MAX; i++) {
		DEBUG(DBG_PE_DECALF, "pe_decide_alf: prio %d context %p", i,
		    proc->context[i]);

		ret = pe_alf_evaluate(proc, proc->context[i], msg, &log,
		    &rule_id);

		if (ret != -1) {
			decision = ret;
			prio = i;
		}
		if (ret == POLICY_DENY) {
			break;
		}
	}
	DEBUG(DBG_PE_DECALF, "pe_decide_alf: decision %d", decision);

	/* If no default rule matched, decide on deny */
	if (decision == -1) {
		decision = POLICY_DENY;
		log = APN_LOG_ALERT;
	}

	if (log != APN_LOG_NONE) {
		context = pe_dump_ctx(hdr, proc, prio);
		dump = pe_dump_alfmsg(msg, hdr, ANOUBISD_LOG_APN);
	}

	/* Logging */
	switch (log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		log_info("%s prio %d rule %d %s %s (%s)", prefix, prio,
		    rule_id, verdict[decision], dump, context);
		send_lognotify(hdr, decision, log, rule_id);
		break;
	case APN_LOG_ALERT:
		log_warnx("%s prio %d rule %d %s %s (%s)", prefix, prio,
		    rule_id, verdict[decision], dump, context);
		send_lognotify(hdr, decision, log, rule_id);
		break;
	default:
		log_warnx("pe_decide_alf: unknown log type %d", log);
	}

	if (dump)
		free(dump);
	if (context)
		free(context);

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_decide_alf: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}

	reply->ask = 0;
	reply->rule_id = rule_id;
	reply->timeout = (time_t)0;
	if (decision == POLICY_ASK) {
		reply->ask = 1;
		reply->timeout = 300;	/* XXX 5 Minutes for now. */
		reply->csum = proc->csum;
		reply->path = proc->pathhint;
	}
	reply->reply = decision;
	reply->len = 0;

	return (reply);
}

int
pe_alf_evaluate(struct pe_proc *proc, struct pe_context *context,
    struct alf_event *msg, int *log, u_int32_t *rule_id)
{
	struct apn_rule	*rule;
	int		 decision;
	time_t		 t;

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

	if (time(&t) == (time_t)-1) {
		int code = errno;
		log_warn("Cannot get current time");
		master_terminate(code);
		return -1;
	}
	decision = pe_decide_alffilt(proc, rule, msg, log, rule_id, t);
	if (decision == -1)
		decision = pe_decide_alfcap(rule, msg, log, rule_id, t);
	if (decision == -1)
		decision = pe_decide_alfdflt(rule, msg, log, rule_id, t);

	DEBUG(DBG_PE_DECALF, "pe_alf_evaluate: decision %d rule %p", decision,
	    rule);

	return (decision);
}

/*
 * ALF filter logic.
 */
int
pe_decide_alffilt(struct pe_proc *proc, struct apn_rule *rule,
    struct alf_event *msg, int *log, u_int32_t *rule_id, time_t now)
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
		if (hp->type != APN_ALF_FILTER
		    || !pe_in_scope(hp->scope, &msg->common, now))
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
			    hp->rule.afilt.filtspec.netaccess != APN_SEND &&
			    hp->rule.afilt.filtspec.netaccess != APN_BOTH)
				continue;
			break;

		case ALF_ACCEPT:
			if (hp->rule.afilt.filtspec.netaccess != APN_ACCEPT &&
			    hp->rule.afilt.filtspec.netaccess != APN_RECEIVE &&
			    hp->rule.afilt.filtspec.netaccess != APN_BOTH)
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
			pe_kcache_alf(hp, decision, proc, msg);
			break;
		case APN_ACTION_DENY:
			decision = POLICY_DENY;
			pe_kcache_alf(hp, decision, proc, msg);
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
		if (rule_id)
			*rule_id = hp->id;
		break;
	}

	return (decision);
}

void
pe_kcache_alf(struct apn_alfrule *rule, int decision, struct pe_proc *proc,
    struct alf_event *msg)
{
	struct anoubis_kernel_policy *policy;
	struct alf_rule *alfrule;

	if (msg->op != ALF_SENDMSG && msg->op != ALF_RECVMSG)
		return;

	policy = malloc(sizeof(struct anoubis_kernel_policy) +
	    sizeof(struct alf_rule));
	if (policy == NULL)
		return;

	policy->anoubis_source = ANOUBIS_SOURCE_ALF;
	policy->rule_len = sizeof(struct alf_rule);
	policy->decision = decision;
	if (rule->rule.afilt.filtspec.statetimeout > 0) {
		policy->expire = rule->rule.afilt.filtspec.statetimeout +
		    time(NULL);
	} else {
		policy->expire = 0;
	}

	alfrule = (struct alf_rule*)(policy->rule);

	alfrule->family = msg->family;
	alfrule->protocol = msg->protocol;
	alfrule->type = msg->type;
	alfrule->op = msg->op;

	switch(alfrule->family) {
	case AF_INET:
		alfrule->local.port_min = msg->local.in_addr.sin_port;
		alfrule->local.port_max = msg->local.in_addr.sin_port;
		alfrule->peer.port_min = msg->peer.in_addr.sin_port;
		alfrule->peer.port_max = msg->peer.in_addr.sin_port;
		alfrule->local.addr.in_addr =
		    msg->local.in_addr.sin_addr;
		alfrule->peer.addr.in_addr =
		    msg->peer.in_addr.sin_addr;
		break;

	case AF_INET6:
		alfrule->local.port_min = msg->local.in6_addr.sin6_port;
		alfrule->local.port_max = msg->local.in6_addr.sin6_port;
		alfrule->peer.port_min = msg->peer.in6_addr.sin6_port;
		alfrule->peer.port_max = msg->peer.in6_addr.sin6_port;
		alfrule->local.addr.in6_addr =
		    msg->local.in6_addr.sin6_addr;
		alfrule->peer.addr.in6_addr =
		    msg->peer.in6_addr.sin6_addr;
		break;
	}

	proc->kcache = kernelcache_add(proc->kcache, policy);
	kernelcache_send2master(proc->kcache, proc->pid);

	free(policy);
}

/*
 * ALF capability logic.
 */
int
pe_decide_alfcap(struct apn_rule *rule, struct alf_event *msg, int *log,
    u_int32_t *rule_id, time_t now)
{
	int			 decision;
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
		if (hp->type != APN_ALF_CAPABILITY ||
		    !pe_in_scope(hp->scope, &msg->common, now)) {
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
			if (msg->type == SOCK_RAW)
				decision = POLICY_ALLOW;
#ifdef LINUX
			if (msg->type == SOCK_PACKET)
				decision = POLICY_ALLOW;
#endif
			break;
		case APN_ALF_CAPOTHER:
#ifdef LINUX
			if (msg->type != SOCK_RAW && msg->type != SOCK_PACKET)
#else
			if (msg->type != SOCK_RAW)
#endif
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
			if (rule_id)
				*rule_id = hp->id;
			break;
		}
		hp = hp->next;
	}

	return (decision);
}

/*
 * ALF default logic.
 */
int
pe_decide_alfdflt(struct apn_rule *rule, struct alf_event *msg, int *log,
    u_int32_t *rule_id, time_t now)
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
		if (hp->type != APN_ALF_DEFAULT
		    || !pe_in_scope(hp->scope, &msg->common, now)) {
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
		case APN_ACTION_ASK:
			decision = POLICY_ASK;
			break;
		default:
		log_warnx("pe_decide_alfdflt: unknown action %d",
			    hp->rule.apndefault.action);
			decision = -1;
		}

		if (decision != -1) {
			if (log)
				*log = hp->rule.apndefault.log;
			if (rule_id)
				*rule_id = hp->id;
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
pe_dump_alfmsg(struct alf_event *msg, struct eventdev_hdr *hdr, int format)
{
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
	snprintf(local, 128, "<unknown>");
	snprintf(peer, 128, "<unknown>");

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
			snprintf(local, 128, "<unknown>");
		if (inet_ntop(msg->family, &msg->peer.in_addr.sin_addr, peer,
		    sizeof(peer)) == NULL)
			snprintf(peer, 128, "<unknown>");
		break;

	case AF_INET6:
		af = "inet6";
		lport = ntohs(msg->local.in6_addr.sin6_port);
		pport = ntohs(msg->peer.in6_addr.sin6_port);

		if (inet_ntop(msg->family, &msg->local.in6_addr.sin6_addr,
		    local, sizeof(local)))
			snprintf(local, 128, "<unknown>");
		if (inet_ntop(msg->family, &msg->peer.in6_addr.sin6_addr, peer,
		    sizeof(peer)) == NULL)
			snprintf(peer, 128, "<unknown>");
		break;
#ifdef LINUX
	case AF_PACKET:
		af = "packet";
		break;
#endif
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
#ifdef LINUX
	case SOCK_PACKET:
		type = "packet";
		break;
#endif
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

	switch (format) {
	case ANOUBISD_LOG_RAW:
		if (asprintf(&dump, "%s (%u): uid %u pid %u %s (%u) %s (%u) "
		    "%s (%u) local %s:%hu peer %s:%hu", op, msg->op,
		    hdr->msg_uid, hdr->msg_pid, type, msg->type, proto,
		    msg->protocol, af, msg->family, local, lport, peer, pport)
		    == -1) {
			dump = NULL;
		}
		break;

	case ANOUBISD_LOG_APN:
		if (asprintf(&dump, "%s %s %s from %s port %hu to %s port %hu",
		    op, af, proto, local, lport, peer, pport) == -1) {
			dump = NULL;
		}
		break;

	default:
		log_warnx("pe_dump_alfmsg: illegal logging format %d", format);
		dump = NULL;
	}

	return (dump);
}

/*
 * Decode a context in a printable string.  This strings is allocated
 * and needs to be freed by the caller.
 */
char *
pe_dump_ctx(struct eventdev_hdr *hdr, struct pe_proc *proc, int prio)
{
	struct pe_context	*ctx;
	unsigned long		 csumctx;
	char			*dump, *progctx;

	/* hdr must be non-NULL, proc might be NULL */
	if (hdr == NULL)
		return (NULL);

	csumctx = 0;
	progctx = "<none>";

	if (proc && 0 <= prio && prio <= PE_PRIO_MAX && proc->valid_ctx) {
		ctx = proc->context[prio];

		if (ctx && ctx->rule) {
			if (ctx->rule->app) {
				csumctx = htonl(*(unsigned long *)
				    ctx->rule->app->hashvalue);
				progctx = ctx->rule->app->name;
			} else
				progctx = "any";
		}
	}

	if (asprintf(&dump, "uid %hu pid %hu program %s checksum 0x%08x... "
	    "context program %s checksum 0x%08lx...",
	    hdr->msg_uid, hdr->msg_pid,
	    (proc && proc->pathhint) ? proc->pathhint : "<none>",
	    (proc && proc->csum) ? htonl(*(unsigned long *)proc->csum) : 0,
	    progctx, csumctx) == -1) {
		dump = NULL;
	}

	return (dump);
}

anoubisd_reply_t *
pe_handle_sfs(anoubisd_msg_sfsopen_t *sfsmsg)
{
	anoubisd_reply_t		*reply = NULL;
	struct pe_user			*user;
	struct eventdev_hdr		*hdr;
	time_t				 now;

	if (sfsmsg == NULL) {
		log_warnx("pe_handle_sfs: empty message");
		return (NULL);
	}
	hdr = (struct eventdev_hdr *)&sfsmsg->hdr;
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct sfs_open_message))) {
		log_warnx("pe_handle_sfs: short message");
		return (NULL);
	}

	user = pe_get_user(hdr->msg_uid, pdb);
	if (user == NULL)
		user = pe_get_user(-1, pdb);

	if (time(&now) == (time_t)-1) {
		int code = errno;
		log_warn("pe_handle_sfs: Cannot get time");
		master_terminate(code);
		return (NULL);
	}
	reply = pe_decide_sfs(user, sfsmsg, now);

	return (reply);
}

anoubisd_reply_t *
pe_decide_sfs(struct pe_user *user, anoubisd_msg_sfsopen_t *sfsmsg, time_t now)
{
	static char		 prefix[] = "SFS";
	static char		*verdict[3] = { "allowed", "denied", "asked" };
	anoubisd_reply_t	*reply = NULL;
	struct eventdev_hdr	*hdr;
	struct sfs_open_message	*msg;
	int			 ret, log, i, decision, prio;
	u_int32_t		 rule_id = 0;
	char			*dump = NULL;
	char			*context = NULL;

	hdr = &sfsmsg->hdr;
	msg = (struct sfs_open_message *)(hdr+1);

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_handle_sfs: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}

	decision = -1;
	prio = -1;
	rule_id = 0;
	log = 0;

	if (msg->flags & ANOUBIS_OPEN_FLAG_CSUM) {
		if (user != NULL) {
			for (i = 0; i < PE_PRIO_MAX; i++) {
				struct apn_ruleset	*rs = user->prio[i];
				struct apn_rule		*rule;

				if (rs == NULL)
					continue;

				if (TAILQ_EMPTY(&rs->sfs_queue))
					continue;

				TAILQ_FOREACH(rule, &rs->sfs_queue, entry) {
					ret = pe_decide_sfscheck(rule, msg,
					    &log, &rule_id, now);

					if (ret == -1)
						ret = pe_decide_sfsdflt(rule,
						    msg, &log, &rule_id, now);

					if (ret != -1) {
						decision = ret;
						prio = i;
					}
					if (ret == POLICY_DENY)
						break;
				}
				if (decision == POLICY_DENY)
					break;
			}
		}
		/* Look into checksum from /var/lib/sfs */
		if ((decision == -1) &&
		    (sfsmsg->anoubisd_csum_set != ANOUBISD_CSUM_NONE)) {
			if (memcmp(msg->csum, sfsmsg->anoubisd_csum,
			    ANOUBIS_SFS_CS_LEN))
				decision = POLICY_DENY;
		}
	} else {
		if (sfsmsg->anoubisd_csum_set != ANOUBISD_CSUM_NONE)
			decision = POLICY_DENY;
	}

	if (decision == -1)
		decision = POLICY_ALLOW;

	if (log != APN_LOG_NONE) {
		context = pe_dump_ctx(hdr, pe_get_proc(msg->common.task_cookie),
		    prio);
		dump = pe_dump_sfsmsg(msg, ANOUBISD_LOG_APN);
	}

	/* Logging */
	switch (log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		log_info("%s prio %d rule %d %s %s (%s)", prefix, prio,
		    rule_id, verdict[decision], dump, context);
		send_lognotify(hdr, decision, log, rule_id);
		break;
	case APN_LOG_ALERT:
		log_warnx("%s %s %s (%s)", prefix, prio, rule_id,
		    verdict[decision], dump, context);
		send_lognotify(hdr, decision, log, rule_id);
		break;
	default:
		log_warnx("pe_decide_sfs: unknown log type %d", log);
	}

	if (dump)
	    free(dump);
	if (context)
		free(context);

	reply->reply = decision;
	reply->ask = 0;		/* false */
	reply->rule_id = rule_id;
	reply->timeout = (time_t)0;
	reply->len = 0;

	return (reply);
}

int
pe_decide_sfscheck(struct apn_rule *rule, struct sfs_open_message *msg,
    int *log, u_int32_t *rule_id, time_t now)
{
	struct apn_sfsrule	*hp;
	int			 decision;

	if (rule == NULL) {
		log_warnx("pe_decide_sfscheck: empty rule");
		return (-1);
	}
	if (msg == NULL) {
		log_warnx("pe_decide_sfscheck: empty sfs event");
		return (-1);
	}

	decision = -1;
	for (hp = rule->rule.sfs; hp; hp = hp->next) {
		/*
		 * skip non-check rules
		 */
		if (hp->type != APN_SFS_CHECK
		    || !pe_in_scope(hp->scope, &msg->common, now))
			continue;

		if (strcmp(hp->rule.sfscheck.app->name, msg->pathhint))
			continue;

		decision = POLICY_ALLOW;
		/*
		 * If a SFS rule exists for a file, the decision is always
		 * deny if the checksum does not match.
		 */
		if (memcmp(msg->csum, hp->rule.sfscheck.app->hashvalue,
		    ANOUBIS_SFS_CS_LEN))
			decision = POLICY_DENY;

		if (log)
			*log = hp->rule.sfscheck.log;
		if (rule_id)
			*rule_id = hp->id;

		break;
	}

	return decision;
}

int
pe_decide_sfsdflt(struct apn_rule *rule, struct sfs_open_message *msg, int *log,
    u_int32_t *rule_id, time_t now)
{
	struct apn_sfsrule	*hp;
	int			 decision;

	if (rule == NULL) {
		log_warnx("pe_decide_sfsdflt: empty rule");
		return (-1);
	}

	decision = -1;
	hp = rule->rule.sfs;
	while (hp) {
		/* Skip non-default rules. */
		if (hp->type != APN_SFS_DEFAULT
		    || !pe_in_scope(hp->scope, &msg->common, now)) {
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
		case APN_ACTION_ASK:
			decision = POLICY_ASK;
			break;
		default:
		log_warnx("pe_decide_sfsdflt: unknown action %d",
			    hp->rule.apndefault.action);
			decision = -1;
		}

		if (decision != -1) {
			if (log)
				*log = hp->rule.apndefault.log;
			if (rule_id)
				*rule_id = hp->id;
			break;
		}
		hp = hp->next;
	}

	return (decision);
}

/*
 * Decode a SFS message into a printable string.  The strings is allocated
 * and needs to be freed by the caller.
 *
 * "format" is specified for symmetry with pe_dump_alfmsg().
 */
char *
pe_dump_sfsmsg(struct sfs_open_message *msg, int format)
{
	char	*dump, *access;

	if (msg == NULL)
		return (NULL);

	if (msg->flags & ANOUBIS_OPEN_FLAG_READ)
		access = "read";
	else if (msg->flags & ANOUBIS_OPEN_FLAG_WRITE)
		access = "write";
	else
		access = "<unknown access>";

	if (asprintf(&dump, "%s %s", access,
	    msg->flags & ANOUBIS_OPEN_FLAG_PATHHINT ? msg->pathhint : "<none>")
	    == -1) {
		dump = NULL;
	}

	return (dump);
}

static struct
policy_request *policy_request_find(u_int64_t token)
{
	struct policy_request * req;
	LIST_FOREACH(req, &preqs, next) {
		if (req->token == token)
			return req;
	}
	return NULL;
}

static void
put_request(struct policy_request *req)
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
		goto reply;
	}
	/* No request yet, start one. */
	if (req == NULL) {
		if (comm->len < sizeof(Policy_Generic)) {
			error = EINVAL;
			goto reply;
		}
		gen = (Policy_Generic *)comm->msg;
		req = calloc(1, sizeof(struct policy_request));
		if (!req) {
			error = ENOMEM;
			goto reply;
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
			goto reply;
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
			goto reply;
		fd = pe_open_policy_file(uid, prio);
		req->uid = uid;
		req->prio = prio;
		if (fd == -ENOENT) {
			error = 0;
			goto reply;
		}
		error = EIO;
		if (fd < 0) {
			/* ENOENT indicates an empty policy and not an error. */
			if (fd == -ENOENT)
				error = 0;
			goto reply;
		}
		error = - send_policy_data(comm->token, fd);
		close(fd);
		if (error)
			goto reply;
		break;
	case ANOUBIS_PTYPE_SETBYUID:
		buf = comm->msg;
		len = comm->len;
		if (comm->flags & POLICY_FLAG_START) {
			char	*tmp;
			if (comm->len < sizeof(Policy_SetByUid)) {
				error = EINVAL;
				goto reply;
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
				goto reply;
			}
			if (prio == PE_PRIO_ADMIN && req->authuid != 0) {
				error = EPERM;
				goto reply;
			}
			req->realname = pe_policy_file_name(uid, prio);
			req->uid = uid;
			req->prio = prio;
			if (!req->realname)
				goto reply;
			tmp = __pe_policy_file_name(uid, prio, ".tmp.");
			if (!tmp)
				goto reply;
			/* splint doesn't understand the %llu modifier */
			if (asprintf(&req->tmpname, "%s.%llu", tmp,
			    /*@i@*/ req->token) == -1) {
				free(tmp);
				req->tmpname = NULL;
				goto reply;
			}
			free(tmp);
			DEBUG(DBG_TRACE, "  open: %s", req->tmpname);
			req->fd = open(req->tmpname,
			    O_WRONLY|O_CREAT|O_EXCL, 0400);
			if (req->fd < 0) {
				error = errno;
				goto reply;
			}
			DEBUG(DBG_TRACE, "  open: %s fd = %d", req->tmpname,
			    req->fd);
			buf += sizeof(Policy_SetByUid);
			len -= sizeof(Policy_SetByUid);
		}
		if (req->authuid != comm->uid) {
			error = EPERM;
			goto reply;
		}
		while(len) {
			int ret = write(req->fd, buf, len);
			if (ret < 0) {
				DEBUG(DBG_TRACE, "  write error fd=%d",
				    req->fd);
				error = errno;
				goto reply;
			}
			buf += ret;
			len -= ret;
		}
		if (comm->flags & POLICY_FLAG_END) {
			char	*bkup, *tmp;
			time_t	 t;
			int	 ret;
			struct apn_ruleset * ruleset = NULL;

			/* Only accept syntactically correct rules. */
			if (pe_clean_policy(req->tmpname,
			    prio_to_string[req->prio], 0)) {
				error = EINVAL;
				goto reply;
			}
			if (apn_parse(req->tmpname, &ruleset,
			    (req->prio != PE_PRIO_USER1)?APN_FLAG_NOASK:0)) {
				if (ruleset)
					free(ruleset);
				error = EINVAL;
				goto reply;
			}
			/* Backup old rules */
			if (time(&t) == (time_t)-1) {
				error = errno;
				apn_free_ruleset(ruleset);
				goto reply;
			}
			tmp = __pe_policy_file_name(req->uid, req->prio,
			    ".save.");
			if (tmp == NULL) {
				error = ENOMEM;
				apn_free_ruleset(ruleset);
				goto reply;
			}
			ret = asprintf(&bkup, "%s.%lld", tmp, (long long)t);
			free(tmp);
			if (ret == -1) {
				errno = ENOMEM;
				apn_free_ruleset(ruleset);
				goto reply;
			}
			DEBUG(DBG_TRACE, "    link: %s->%s", req->realname,
			    bkup);
			if (link(req->realname, bkup) < 0) {
				if (errno != ENOENT && errno != EEXIST) {
					error = errno;
					free(bkup);
					apn_free_ruleset(ruleset);
					goto reply;
				}
			}
			free(bkup);
			DEBUG(DBG_TRACE, "    rename: %s->%s", req->tmpname,
			    req->realname);
			if (rename(req->tmpname, req->realname) < 0) {
				error = errno;
				apn_free_ruleset(ruleset);
				goto reply;
			}
			DEBUG(DBG_TRACE, "    pe_replace_rs uid=%d prio=%d\n",
			    req->uid, req->prio);
			if (pe_replace_rs(ruleset, req->uid, req->prio)) {
				error = EIO;
				apn_free_ruleset(ruleset);
				goto reply;
			}
			/*
			 * XXX CEH: Try to generate a log event about the
			 * XXX CEH: policy change here to let other UIs know
			 * XXX CEH: that policies changed.
			 */
			error = 0;
			goto reply;
		}
		break;
	default:
		/* Unknown opcode. */
		error = EINVAL;
		goto reply;
	}
	if (req && (comm->flags & POLICY_FLAG_END)) {
		LIST_REMOVE(req, next);
		put_request(req);
	}
	DEBUG(DBG_TRACE, "<pe_dispatch_policy (NULL)");
	return NULL;
reply:
	if (req) {
		LIST_REMOVE(req, next);
		put_request(req);
	}
	reply = calloc(1, sizeof(struct anoubisd_reply));
	reply->token = comm->token;
	reply->ask = 0;
	reply->rule_id = 0;
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

	kernelcache_clear(proc->kcache);

	free(proc);
}

struct pe_proc *
pe_alloc_proc(uid_t uid, anoubis_cookie_t cookie,
    anoubis_cookie_t parent_cookie)
{
	struct pe_proc	*proc, *parent;

	if ((proc = calloc(1, sizeof(struct pe_proc))) == NULL)
		goto oom;
	proc->task_cookie = cookie;
	parent = pe_get_proc(parent_cookie);
	proc->parent_proc = parent;
	proc->pid = -1;
	proc->uid = uid;
	proc->refcount = 1;
	proc->kcache = NULL;
	if (parent) {
		if (parent->pathhint) {
			proc->pathhint = strdup(parent->pathhint);
			if (!proc->pathhint)
				goto oom;
		}
		if (parent->csum) {
			proc->csum = malloc(ANOUBIS_SFS_CS_LEN);
			if (!proc->csum)
				goto oom;
			memcpy(proc->csum, parent->csum, ANOUBIS_SFS_CS_LEN);
		}
	}

	DEBUG(DBG_PE_TRACKER, "pe_alloc_proc: proc %p uid %u cookie 0x%08llx "
	    "parent cookie 0x%08llx", proc, uid, proc->task_cookie,
	    proc->parent_proc ? parent_cookie : 0);

	return (proc);
oom:
	log_warn("pe_alloc_proc: cannot allocate memory");
	master_terminate(ENOMEM);
	return (NULL);	/* XXX HSH */
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
 * If our parent was never tracked, we get a new context.
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

		pe_dump_dbgctx("pe_inherit_ctx", proc);
	} else {
		/*
		 * No parent available, derive new context:  We have
		 * neither an UID, nor pathname and checksum.  Thus the only
		 * possible context will be derived from the admin/default
		 * rule set.  If this is not available, the process will
		 * get no context and susequent policy decisions will not
		 * yield a valid result (ie. != -1).  In that case,
		 * the policy engine will enforce the decision POLICY_DENY.
		 */
		pe_set_ctx(proc, -1, NULL, NULL);
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
		    proc->context[0] ? proc->context[0]->rule : NULL,
		    proc->context[0] ? proc->context[0]->ctx : NULL);

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
	if ((user = pe_get_user(uid, pdb)) == NULL &&
	    (user = pe_get_user(-1, pdb)) == NULL) {
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
			struct apn_ruleset *ruleset;
			ruleset = pe_get_ruleset(uid, i, pdb);
			pe_put_ctx(proc->context[i]);
			proc->context[i] = pe_search_ctx(ruleset, csum,
			    pathhint);
		}
		proc->valid_ctx = 1;
	}
	pe_set_parent_proc(proc, proc);
	if (uid >= 0)
		proc->uid = uid;

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
	    csum ? htonl(*(unsigned long *)csum) : 0);

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
	context = pe_alloc_ctx(rule, rs);
	if (!context) {
		master_terminate(ENOMEM);
		return NULL;
	}

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
 * NOTE: rs must not be NULL.
 */
struct pe_context *
pe_alloc_ctx(struct apn_rule *rule, struct apn_ruleset *rs)
{
	struct pe_context	*ctx;

	if (!rs) {
		log_warnx("NULL rulset in pe_alloc_ctx?");
		return NULL;
	}
	if ((ctx = calloc(1, sizeof(struct pe_context))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);	/* XXX HSH */
		return NULL;
	}
	ctx->rule = rule;
	ctx->ruleset = rs;
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

	if (!proc->valid_ctx)
		return (0);
	/*
	 * NOTE: Once we actually use the pathhint in policiy decision
	 * NOTE: this shortcut will no longer be valid.
	 */
	if (!csum)
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
		log_info("proc %p token 0x%08llx pproc %p pid %d csum 0x%08x "
		    "pathhint \"%s\" ctx %p %p rules %p %p", proc,
		    proc->task_cookie, proc->parent_proc, (int)proc->pid,
		    proc->csum ? htonl(*(unsigned long *)proc->csum) : 0,
		    proc->pathhint ? proc->pathhint : "",
		    proc->context[0], proc->context[1],
		    proc->context[0] ? proc->context[0]->rule : NULL,
		    proc->context[1] ? proc->context[1]->rule : NULL);
	}

	log_info("policies (pdb %p)", pdb);
	TAILQ_FOREACH(user, pdb, entry) {
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
pe_dump_dbgctx(const char *prefix, struct pe_proc *proc)
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
 * compare addr1 and addr2 with netmask of length len
 */
static int
compare_ip6_mask(struct in6_addr *addr1, struct in6_addr *addr2, int len)
{
	int		 bits, match, i;
	struct in6_addr	 mask;
	u_int32_t	*pmask;

	if (len > 128)
		len = 128;

	if (len == 128)
		return !bcmp(addr1, addr2, sizeof(struct in6_addr));

	if (len < 0)
		len = 0;

	bits = len;
	bzero(&mask, sizeof(struct in6_addr));
	pmask = &mask.s6_addr32[0];
	while (bits >= 32) {
		*pmask = 0xffffffff;
		pmask++;
		bits -= 32;
	}
	if (bits > 0) {
		*pmask = htonl(0xffffffff << (32 - bits));
	}

	match = 1;
	for (i=0; i <=3 ; i++) {
		if ((addr1->s6_addr32[i] & mask.s6_addr32[i]) !=
		    (addr2->s6_addr32[i] & mask.s6_addr32[i])) {
			match = 0;
			break;
		}
	}

	return match;
}

/*
 * Compare addresses. 1 on match, 0 otherwise.
 */
int
pe_addrmatch_host(struct apn_host *host, void *addr, unsigned short af)
{
	struct apn_host		*hp;
	struct sockaddr_in	*in;
	struct sockaddr_in6	*in6;
	in_addr_t		 mask;
	int			 match;

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
			match = ((in->sin_addr.s_addr & mask) ==
			    (hp->addr.apa.v4.s_addr & mask));
			break;

		case AF_INET6:
			in6 = (struct sockaddr_in6 *)addr;
			match = compare_ip6_mask(&in6->sin6_addr,
			    &hp->addr.apa.v6, hp->addr.len);
			break;

		default:
			log_warnx("pe_addrmatch_host: unknown address family "
			    "%d", af);
			match = 0;
		}
		if (match)
			break;
		hp = hp->next;
	}

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_host: match %d", match);

	return (match);
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
			if (pp->port2) {
				match = (in->sin_port >= pp->port &&
					in->sin_port <= pp->port2);
			}
			else
				match = (in->sin_port == pp->port);
			break;

		case AF_INET6:
			in6 = (struct sockaddr_in6 *)addr;
			if (pp->port2) {
				match = (in6->sin6_port >= pp->port &&
					in6->sin6_port <= pp->port2);
			}
			else
				match = (in6->sin6_port == pp->port);
			break;

		default:
			log_warnx("pe_addrmatch_port: unknown address "
			    "family %d", af);
			match = 0;
		}
		if (match)
			break;
		pp = pp->next;
	}

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_port: match %d", match);

	return (match);
}

static int
pe_in_scope(struct apn_scope *scope, struct anoubis_event_common *common,
    time_t now)
{
	if (!scope)
		return 1;
	if (scope->timeout && now > scope->timeout)
		return 0;
	if (scope->task && common->task_cookie != scope->task)
		return 0;
	return 1;
}

int
pe_load_pubkeys(const char *dirname, struct pubkeys *pubkeys)
{
	DIR			*dir;
	FILE			*fp;
	struct dirent		*dp;
	struct pe_pubkey	*pk;
	uid_t			 uid;
	int			 count;
	const char		*errstr;
	char			*filename;

	DEBUG(DBG_PE_POLICY, "pe_load_pubkeys: %s %p", dirname, pubkeys);

	if (pubkeys == NULL) {
		log_warnx("pe_load_pubkeys: illegal pubkey queue");
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
		 * Validate the file name: It has to be a numeric uid.
		 */
		uid = strtonum(dp->d_name, 0, UID_MAX, &errstr);
		if (errstr) {
			log_warnx("pe_load_pubkeys: filename \"%s/%s\" %s",
			    dirname, dp->d_name, errstr);
			continue;
		}
		if (asprintf(&filename, "%s/%s", dirname, dp->d_name) == -1) {
			log_warnx("asprintf: Out of memory");
			continue;
		}
		if ((pk = calloc(1, sizeof(struct pe_pubkey))) == NULL) {
			log_warnx("calloc: Out or memory");
			free(filename);
			continue;
		}
		if ((fp = fopen(filename, "r")) == NULL) {
			log_warn("fopen");
			free(pk);
			free(filename);
			continue;
		}
		free(filename);

		if ((pk->pubkey = PEM_read_PUBKEY(fp, NULL, NULL, NULL))
		    == NULL) {
			log_warn("PEM_read_PUBKEY");
			free(pk);
			fclose(fp);
			continue;
		}
		pk->uid = uid;
		fclose(fp);
		TAILQ_INSERT_TAIL(pubkeys, pk, entry);
		count++;
	}

	if (closedir(dir) == -1)
		log_warn("closedir");

	DEBUG(DBG_PE_POLICY, "pe_load_pubkeys: %d public keys loaded", count);

	return (count);
}

void
pe_flush_pubkeys(struct pubkeys *pk)
{
	struct pe_pubkey	*p, *next;

	for (p = TAILQ_FIRST(pk); p != TAILQ_END(pk); p = next) {
		next = TAILQ_NEXT(p, entry);
		TAILQ_REMOVE(pk, p, entry);

		EVP_PKEY_free(p->pubkey);
		free(p);
	}
}

int
pe_verify_sig(const char *filename, uid_t uid)
{
	char		 buffer[1024];
	EVP_PKEY	*sigkey;
	EVP_MD_CTX	 ctx;
	unsigned char	*sigbuf;
	char		*sigfile;
	int		 fd, n, siglen, result;

	if (asprintf(&sigfile, "%s.sig", filename) == -1) {
		log_warnx("asprintf: Out of memory");
		return (0);	/* XXX policy will not be loaded... */
	}

	/* If no signature file is available, return successfully */
	if ((fd = open(sigfile, O_RDONLY)) == -1) {
		free(sigfile);
		return (1);
	}
	free(sigfile);

	/* Fail when we have no public key for that uid */
	if ((sigkey = pe_get_pubkey(uid)) == NULL) {
		log_warnx("pe_verify_sig: no key for uid %lu available",
		    (unsigned long)uid);
		close(fd);
		return (0);
	}

	/* Read in signature */
	siglen = EVP_PKEY_size(sigkey);
	if ((sigbuf = calloc(siglen, sizeof(unsigned char))) == NULL) {
		close(fd);
		return (0);
	}
	if (read(fd, sigbuf, siglen) != siglen) {
		log_warnx("pe_verify_sig: error reading signature file %s",
		    sigfile);
		close(fd);
		free(sigbuf);
		return (0);
	}
	close(fd);

	EVP_MD_CTX_init(&ctx);
	if (EVP_VerifyInit(&ctx, EVP_sha1()) == 0) {
		log_warnx("pe_verify_sig: could not verify signature");
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return (0);
	}

	/* Open policy file */
	if ((fd = open(filename, O_RDONLY)) == -1) {
		log_warnx("pe_verify_sig: could not read policy %s", filename);
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return (0);
	}

	while ((n = read(fd, buffer, sizeof(buffer))) > 0)
		EVP_VerifyUpdate(&ctx, buffer, n);
	if (n == -1) {
		log_warn("read");
		close(fd);
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return (0);
	}
	close(fd);

	/* Verify */
	if ((result = EVP_VerifyFinal(&ctx, sigbuf, siglen, sigkey)) == -1)
		log_warnx("pe_verify_sig: could not verify signature");

	EVP_MD_CTX_cleanup(&ctx);
	free(sigbuf);

	return (result);
}

EVP_PKEY *
pe_get_pubkey(uid_t uid)
{
	struct pe_pubkey	*p;

	TAILQ_FOREACH(p, pubkeys, entry) {
		if (p->uid == uid)
			return (p->pubkey);
	}

	return (NULL);
}

/*@=memchecks@*/

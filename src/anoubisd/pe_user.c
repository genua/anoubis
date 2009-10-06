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

#include "config.h"

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#ifdef OPENBSD
#include <sys/limits.h>
#endif

#include <apn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef LINUX
#include <bsdcompat.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/queue.h>
#include <dev/anoubis.h>
#endif

#include <anoubis_protocol.h>
#include <anoubis_sig.h>
#include "anoubisd.h"
#include "pe.h"
#include "cert.h"

struct pe_user {
	TAILQ_ENTRY(pe_user)	 entry;
	uid_t			 uid;
	struct apn_ruleset	*prio[PE_PRIO_MAX];
};
TAILQ_HEAD(pe_policy_db, pe_user) *pdb;

static char * prio_to_string[PE_PRIO_MAX] = {
#ifndef lint
	[ PE_PRIO_ADMIN ] = ANOUBISD_POLICYCHROOT "/" ANOUBISD_ADMINDIR,
	[ PE_PRIO_USER1 ] = ANOUBISD_POLICYCHROOT "/" ANOUBISD_USERDIR,
#endif
};

struct pe_policy_request {
	LIST_ENTRY(pe_policy_request) next;
	u_int64_t	 token;
	u_int32_t	 ptype;
	u_int32_t	 authuid;
	int		 fd;
	int		 fdsig;
	int		 prio;
	uid_t		 uid;
	int		 written;
	int		 siglen;
	char		*tmpname;
	char		*tmpsig;
	char		*realname;
	char		*realsig;
	unsigned char	*key;
};
LIST_HEAD(, pe_policy_request) preqs;

void				 pe_user_init(void);
static int			 pe_user_load_db(struct pe_policy_db *);
static int			 pe_user_load_dir(const char *, unsigned int,
				     struct pe_policy_db *);
static struct apn_ruleset	*pe_user_load_policy(const char *name,
				     int flags);
static int			 pe_user_insert_rs(struct apn_ruleset *,
				     uid_t, unsigned int,
				     struct pe_policy_db *);
static struct pe_user		*pe_user_get(uid_t, struct pe_policy_db *);

void
pe_user_init(void)
{
	struct pe_policy_db	*pp;
	int		 count;

	LIST_INIT(&preqs);

	if ((pp = calloc(1, sizeof(struct pe_policy_db))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);	/* XXX HSH */
	}
	TAILQ_INIT(pp);

	/* We die gracefully if loading fails. */
	if ((count = pe_user_load_db(pp)) == -1)
		fatal("pe_user_init: failed to initialize policy database");
	pdb = pp;

	log_info("pe_user_init: %d policies loaded to pdb %p", count, pp);
}

void
pe_user_reconfigure(void)
{
	struct pe_policy_db	*newpdb, *oldpdb;
	int			 count;

	if ((newpdb = calloc(1, sizeof(struct pe_policy_db))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);       /* XXX HSH */
	}
	TAILQ_INIT(newpdb);
	if ((count = pe_user_load_db(newpdb)) == -1) {
		log_warnx("pe_user_reconfigure: "
		    "could not reload policy database");
		free(newpdb);
		return;
	}

	/* Switch to new policy database */
	oldpdb = pdb;
	pdb = newpdb;

	pe_proc_update_db(newpdb);
	pe_user_flush_db(oldpdb);

	log_info("pe_user_reconfigure: loaded %d policies to new pdb %p, "
	    "flushed old pdb %p", count, newpdb, oldpdb);
}

void
pe_user_flush_db(struct pe_policy_db *ppdb)
{
	struct pe_user	*p, *pnext;
	int		 i;

	if (ppdb == NULL)
		ppdb = pdb;
	for (p = TAILQ_FIRST(ppdb); p != TAILQ_END(ppdb); p = pnext) {
		pnext = TAILQ_NEXT(p, entry);
		TAILQ_REMOVE(ppdb, p, entry);

		for (i = 0; i < PE_PRIO_MAX; i++)
			apn_free_ruleset(p->prio[i]);
		free(p);
	}
}

static int
pe_user_load_db(struct pe_policy_db *p)
{
	int	count = 0;

	if (p == NULL) {
		log_warnx("pe_user_load_db: bogus database pointer");
		return (-1);
	}

	/* load admin policies */
	count = pe_user_load_dir(ANOUBISD_POLICYCHROOT "/" ANOUBISD_ADMINDIR,
	    PE_PRIO_ADMIN, p);

	/* load user policies */
	count += pe_user_load_dir(ANOUBISD_POLICYCHROOT "/" ANOUBISD_USERDIR,
	    PE_PRIO_USER1, p);

	return (count);
}

static int
pe_user_load_dir(const char *dirname, unsigned int prio, struct pe_policy_db *p)
{
	DIR			*dir;
	struct cert		*pub;
	struct dirent		*dp;
	struct apn_ruleset	*rs;
	int			 count;
	uid_t			 uid;
	const char		*errstr;
	char			*filename, *t;
	int			 flags = 0;

	DEBUG(DBG_PE_POLICY, "pe_user_load_dir: %s %p", dirname, p);

	if (prio >= PE_PRIO_MAX) {
		log_warnx("pe_user_load_dir: illegal priority %d", prio);
		return (0);
	}

	if (p == NULL) {
		log_warnx("pe_user_load_dir: illegal database");
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
				log_warnx("pe_user_load_dir: "
				    "filename \"%s/%s\" %s",
				    dirname, dp->d_name, errstr);
				continue;
			}
		}
		if (asprintf(&filename, "%s/%s", dirname, dp->d_name) == -1) {
			log_warnx("asprintf: Out of memory");
			continue;
		}
		/* If root placed a certificate for the user there
		 * the policy should be signed and verified.
		 * For now we just support User policies.
		 */
		if (prio != PE_PRIO_ADMIN) {
			if ((pub = cert_get_by_uid(uid)) != NULL) {
				if (anoubis_sig_verify_policy_file(filename,
				    pub->pubkey) != 1)  {
					log_warnx("not loading \"%s\", invalid "
					    "signature", filename);
					free(filename);
					continue;
				}
			}
		}
		rs = pe_user_load_policy(filename, flags);
		free(filename);

		/* If parsing fails, we just continue */
		if (rs == NULL)
			continue;

		if (pe_user_insert_rs(rs, uid, prio, p) != 0) {
			apn_free_ruleset(rs);
			log_warnx("could not insert policy %s/%s", dirname,
			    dp->d_name);
			continue;
		}
		count++;
	}

	if (closedir(dir) == -1)
		log_warn("closedir");

	DEBUG(DBG_PE_POLICY, "pe_user_load_dir: %d policies inserted", count);

	return (count);
}

static int
pe_user_scope_check(struct apn_scope * scope, void * data)
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
		struct pe_proc *proc = pe_proc_get(scope->task);
		if (!proc)
			return 1;
		pe_proc_put(proc);
	}
	/* Keep it. */
	return 0;
}

/*
 * This function is only called if the daemon is started or after
 * a reload. Thus we have to kill all scopes.
 */
static struct apn_ruleset *
pe_user_load_policy(const char *name, int flags)
{
	struct apn_ruleset	*rs;
	time_t			 now = 0;
	int			 ret;
	char			*errstr;

	DEBUG(DBG_PE_POLICY, "pe_user_load_policy: %s", name);

	ret = apn_parse(name, &rs, flags);
	if (ret == -1) {
		log_warn("could not parse \"%s\"", name);
		return (NULL);
	}
	if (ret != 0) {
		errstr = apn_one_error(rs);
		if (errstr)
			log_warnx("could not parse %s", errstr);
		else
			log_warnx("could not parse \"%s\"", name);
		apn_free_ruleset(rs);
		return (NULL);
	}
	apn_clean_ruleset(rs, &pe_user_scope_check, &now);
	rs->destructor = (void*)&pe_prefixhash_destroy;
	return (rs);
}

static int
pe_user_insert_rs(struct apn_ruleset *rs, uid_t uid, unsigned int prio,
    struct pe_policy_db *p)
{
	struct pe_user		*user;

	if (rs == NULL) {
		log_warnx("pe_user_insert_rs: empty ruleset");
		return (1);
	}
	if (p == NULL) {
		log_warnx("pe_user_insert_rs: empty database pointer");
		return (1);
	}
	if (prio >= PE_PRIO_MAX) {
		log_warnx("pe_user_insert_rs: illegal priority %d", prio);
		return (1);
	}

	/* Find or create user */
	if ((user = pe_user_get(uid, p)) == NULL) {
		if ((user = calloc(1, sizeof(struct pe_user))) == NULL) {
			log_warn("calloc");
			master_terminate(ENOMEM);	/* XXX HSH */
		}
		user->uid = uid;
		TAILQ_INSERT_TAIL(p, user, entry);
	} else if (user->prio[prio]) {
		DEBUG(DBG_PE_POLICY, "pe_user_insert_rs: freeing %p "
		    "prio %d user %p", user->prio[prio], prio, user);
		apn_free_ruleset(user->prio[prio]);
		user->prio[prio] = NULL;
	}

	user->prio[prio] = rs;

	DEBUG(DBG_PE_POLICY, "pe_user_insert_rs: uid %d (%p prio %p, %p)",
	    (int)uid, user, user->prio[0], user->prio[1]);

	return (0);
}

static int
pe_user_replace_rs(struct apn_ruleset *rs, uid_t uid, unsigned int prio)
{
	struct apn_ruleset	*oldrs;
	struct pe_user		*user;

	if (rs == NULL) {
		log_warnx("pe_replac_rs: empty ruleset");
		return (1);
	}
	if (prio >= PE_PRIO_MAX) {
		log_warnx("pe_user_replace_rs: illegal priority %d", prio);
		return (1);
	}
	DEBUG(DBG_TRACE, ">pe_user_replace_rs");
	if ((user = pe_user_get(uid, pdb)) == NULL) {
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
	pe_proc_update_db_one(oldrs, prio, uid);
	apn_free_ruleset(oldrs);
	DEBUG(DBG_TRACE, "<pe_user_replace_rs");
	return 0;
}

static struct pe_user *
pe_user_get(uid_t uid, struct pe_policy_db *p)
{
	struct pe_user	*puser, *user;

	if (p == NULL)
		p = pdb;

	user = NULL;
	TAILQ_FOREACH(puser, p, entry) {
		if (puser->uid != uid)
			continue;
		user = puser;
		break;
	}

	DEBUG(DBG_PE_POLICY, "pe_user_get: uid %d user %p", (int)uid, user);

	return (user);
}

struct apn_ruleset *
pe_user_get_ruleset(uid_t uid, unsigned int prio, struct pe_policy_db *p)
{
	struct pe_user *user;

	if (prio >= PE_PRIO_MAX) {
		log_warnx("pe_user_get_ruleset: illegal priority %d", prio);
		return NULL;
	}

	DEBUG(DBG_TRACE, " pe_user_get_ruleset");
	if (p == NULL)
		p = pdb;
	user = pe_user_get(uid, p);
	if (user && user->prio[prio])
		return user->prio[prio];
	user = pe_user_get(-1, p);
	if (!user)
		return NULL;
	return user->prio[prio];
}

static char *
pe_policy_filename(uid_t uid, unsigned int prio, char *pre)
{
	char *name = NULL;

	if (prio >= PE_PRIO_MAX) {
		log_warnx("pe_policy_filename: illegal priority %d", prio);
		return NULL;
	}

	if (asprintf(&name, "/%s/%s%d", prio_to_string[prio], pre, uid) == -1)
		return NULL;

	return name;
}

static int
pe_policy_get(uid_t uid, unsigned int prio)
{
	time_t			 now = 0;
	struct apn_ruleset	*rs;
	int			err;
	char			*tmp;
	FILE			*fd;
	int			 file;

	tmp = pe_policy_filename(uid, prio, ".tmp.");
	rs = pe_user_get_ruleset(uid, prio, pdb);
	if (!rs) {
		free(tmp);
		return -ENOENT;
	}
	if (time(&now) == (time_t)-1) {
		err = errno;
		log_warn("Cannot get current time");
		master_terminate(err);
		return -1;
	}
	fd = fopen(tmp, "w+");
	if (!fd) {
		free(tmp);
		return -errno;
	}
	err = apn_print_ruleset_cleaned(rs, 0, fd, &pe_user_scope_check, &now);
	if (err) {
		free(tmp);
		fclose(fd);
		return -err;
	}
	fclose(fd);
	file = open(tmp, O_RDONLY);
	unlink(tmp);
	free (tmp);
	if (file >= 0)
		return file;
	else
		return -errno;

}

static struct
pe_policy_request *pe_policy_request_find(u_int64_t token)
{
	struct pe_policy_request * req;
	LIST_FOREACH(req, &preqs, next) {
		if (req->token == token)
			return req;
	}
	return NULL;
}

static void
pe_policy_request_put(struct pe_policy_request *req)
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
	if (req->tmpsig) {
		unlink(req->tmpsig);
		free(req->tmpsig);
	}
	if (req->realsig)
		free(req->realsig);
	if (req->fdsig >= 0)
		close(req->fdsig);

	free(req);
}

void
pe_user_dump(void)
{
	struct pe_user		*user;
	unsigned int		 i;
	struct apn_ruleset	*rs;
	struct apn_rule		*rp;

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

anoubisd_reply_t *
pe_dispatch_policy(struct anoubisd_msg_comm *comm)
{
	anoubisd_reply_t		*reply = NULL;
	Policy_Generic			*gen;
	Policy_GetByUid			*getbyuid;
	Policy_SetByUid			*setbyuid;
	u_int32_t			uid;
	u_int32_t			prio;
	int				error = 0;
	int				fd;
	int				ret;
	int				s_len;
	struct pe_policy_request	*req;
	char				*buf;
	int				len;
	struct cert			*key = NULL;

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
	req = pe_policy_request_find(comm->token);
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
			log_warnx("pe_dispatch_policy: Wrong message size");
			error = EINVAL;
			goto reply;
		}
		gen = (Policy_Generic *)comm->msg;
		req = calloc(1, sizeof(struct pe_policy_request));
		if (!req) {
			error = ENOMEM;
			goto reply;
		}
		req->token = comm->token;
		req->ptype = get_value(gen->ptype);
		req->authuid = comm->uid;
		req->written = 0;
		req->fd = -1;
		LIST_INSERT_HEAD(&preqs, req, next);
	}
	DEBUG(DBG_TRACE, " pe_dispatch_policy: ptype = %d, flags = %d "
	    "token = %llu", req->ptype, comm->flags,
	    (unsigned long long)req->token);
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
		if (comm->uid != uid && comm->uid != 0)
			error = EPERM;
		if (prio >= PE_PRIO_MAX)
			error = EINVAL;
		if (error != 0)
			goto reply;
		fd = pe_policy_get(uid, prio);
		req->uid = uid;
		req->prio = prio;
		/* ENOENT indicates an empty policy and not an error. */
		if (fd < 0) {
			if (fd == -ENOENT)
				error = 0;
			else
				error = EIO;
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
				log_warnx("pe_dispatch_policy: Wrong message "
				    "size from request");
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
			req->realname = pe_policy_filename(uid, prio, "");
			req->uid = uid;
			req->prio = prio;
			req->siglen = get_value(setbyuid->siglen);
			if (prio == PE_PRIO_ADMIN) {
				buf += req->siglen;
				req->siglen = 0;
			} else {
				key = cert_get_by_uid(req->uid);
				if (key && req->siglen <= 0) {
					log_warnx("pe_dispatch_policy: Found "
					    "public-key for uid %u and "
					    "expecting signature", req->uid);
					error = EINVAL;
					goto reply;
				}
			}
			if (!req->realname)
				goto reply;
			tmp = pe_policy_filename(uid, prio, ".tmp.");
			if (!tmp)
				goto reply;
			/* splint doesn't understand the %llu modifier */
			if (asprintf(&req->tmpname, "%s.%llu", tmp,
			    /*@i@*/ (unsigned long long)req->token) == -1) {
				free(tmp);
				req->tmpname = NULL;
				goto reply;
			}
			if (req->siglen > 0) {
				if (asprintf(&req->tmpsig, "%s.sig",
				    req->tmpname) == -1) {
					free(req->tmpname);
					req->tmpname = NULL;
					req->tmpsig = NULL;
					goto reply;
				}
				if (asprintf(&req->realsig, "%s.sig",
				    req->realname) == -1) {
					free(req->tmpname);
					req->tmpname = NULL;
					req->tmpsig = NULL;
					free(req->realname);
					req->realsig = NULL;
					req->realname = NULL;
					goto reply;
				}
				DEBUG(DBG_TRACE, "  open: %s", req->tmpsig);
				req->fdsig = open(req->tmpsig,
					O_WRONLY|O_CREAT|O_EXCL, 0400);
				if (req->fdsig < 0) {
					error = errno;
					goto reply;
				}
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
		if (req->written < req->siglen) {
			if (len > (req->siglen - req->written))
				s_len = req->siglen;
			else
				s_len = len;
			while (s_len) {
				ret = write(req->fdsig, buf, s_len);
				if (ret < 0) {
					DEBUG(DBG_TRACE, "  write error fd=%d",
					    req->fdsig);
					error = errno;
					goto reply;
				}
				s_len -= ret;
				len -= ret;
				buf += ret;
				req->written += ret;
			}
		}
		while(len) {
			ret = write(req->fd, buf, len);
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

			/* Only accept verified signatures if a key is
			 * avaible */
			if (req->siglen > 0)
				key = cert_get_by_uid(req->uid);
			if (key) {
				ret = anoubis_sig_verify_policy_file(
				    req->tmpname, key->pubkey);
				if (ret != 1) {
					log_warnx("pe_dispatch_policy: "
					    "Coudn\'t verify policy for uid: "
					    "%u", req->uid);
					error = EINVAL;
					goto reply;
				}
			}
			/* Only accept syntactically correct rules. */
			if (apn_parse(req->tmpname, &ruleset,
			    (req->prio != PE_PRIO_USER1)?APN_FLAG_NOASK:0)) {
				if (ruleset)
					apn_free_ruleset(ruleset);
				log_warnx("pe_dispatch_policy: "
				    "Could not parse ruleset for uid: %u",
				    req->uid);
				error = EINVAL;
				goto reply;
			}
			ruleset->destructor = (void*)&pe_prefixhash_destroy;
			/* Backup old rules */
			if (time(&t) == (time_t)-1) {
				error = errno;
				apn_free_ruleset(ruleset);
				goto reply;
			}
			tmp = pe_policy_filename(req->uid, req->prio,
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
			DEBUG(DBG_TRACE, "    fsync & rename: %s->%s",
				req->tmpname, req->realname);
			if (fsync(req->fd) < 0 ||
			    rename(req->tmpname, req->realname) < 0) {
				error = errno;
				apn_free_ruleset(ruleset);
				goto reply;
			}
			if (req->siglen > 0) {
				if (fsync(req->fdsig) < 0 ||
				    rename(req->tmpsig, req->realsig) < 0) {
					error = errno;
					apn_free_ruleset(ruleset);
					goto reply;
				}
			}
			DEBUG(DBG_TRACE, "    pe_user_replace_rs uid=%d "
			    "prio=%d\n", req->uid, req->prio);
			if (pe_user_replace_rs(ruleset, req->uid, req->prio)) {
				error = EIO;
				apn_free_ruleset(ruleset);
				goto reply;
			}
			send_policychange(req->uid, req->prio);
			error = 0;
			goto reply;
		}
		break;
	default:
		/* Unknown opcode. */
		log_warnx("pe_dispatch_policy: Unknown operation code");
		error = EINVAL;
		goto reply;
	}
	if (req && (comm->flags & POLICY_FLAG_END)) {
		LIST_REMOVE(req, next);
		pe_policy_request_put(req);
	}
	DEBUG(DBG_TRACE, "<pe_dispatch_policy (NULL)");
	return NULL;
reply:
	if (req) {
		LIST_REMOVE(req, next);
		pe_policy_request_put(req);
	}
	reply = calloc(1, sizeof(struct anoubisd_reply));

	if (!reply) {
		log_warn("pe_dispatch_policy: calloc");
		master_terminate(ENOMEM);	/* XXX HSH */
	}

	reply->token = comm->token;
	reply->ask = 0;
	reply->rule_id = 0;
	reply->prio = 0;
	reply->len = 0;
	reply->flags = POLICY_FLAG_START|POLICY_FLAG_END;
	reply->timeout = 0;
	reply->reply = error;
	reply->sfsmatch = ANOUBIS_SFS_NONE;
	DEBUG(DBG_TRACE, "<pe_dispatch_policy: %d", error);
	return reply;
}

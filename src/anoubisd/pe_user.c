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
 * @file
 * This file manages users and their policies. It maintains a database
 * of all users that have policies and provides functions to load and
 * modify these policies. This includes functions to process policy
 * requests from the user.
 *
 * The special user with ID -1 is used for default policies.
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

#include <anoubis_protocol.h>
#include <anoubis_sig.h>
#include "anoubisd.h"
#include "pe.h"
#include "cert.h"
#include "amsg.h"

/**
 * This structure describes one user. All users are maintained in
 * a list of users, known as the policy database.
 */
struct pe_user {
	/**
	 * This is used to link user strutures within a policy database.
	 */
	TAILQ_ENTRY(pe_user)	 entry;

	/**
	 * The user ID of the user.
	 */
	uid_t			 uid;

	/**
	 * The admin and user policy of the user. If no policy exists, this
	 * value is set to NULL.
	 */
	struct apn_ruleset	*prio[PE_PRIO_MAX];
};

/**
 * The active user database.
 */
TAILQ_HEAD(pe_policy_db, pe_user) *pdb;

/**
 * String constants for the policy directories.
 */
static char * prio_to_string[PE_PRIO_MAX] = {
#ifndef lint
	[ PE_PRIO_ADMIN ] = ANOUBISD_POLICYCHROOT "/" ANOUBISD_ADMINDIR,
	[ PE_PRIO_USER1 ] = ANOUBISD_POLICYCHROOT "/" ANOUBISD_USERDIR,
#endif
};

/**
 * This structure is used to handle an ongoing policy request from
 * the user. It keeps all the state that is needed to continue the
 * request when the next message arrives. All active policy requests
 * are linked in a global list.
 *
 * The strings in this structure are allocated dynamically. Additionally,
 * the structure contains up to two open file descriptors.
 */
struct pe_policy_request {
	/**
	 * This field is used to link policy requests in the global
	 * policy request list.
	 */
	LIST_ENTRY(pe_policy_request) next;

	/**
	 * The token of the request. Subsequent messages will use the
	 * same token.
	 */
	u_int64_t	 token;

	/**
	 * The type of the policy request, either ANOUBIS_PTYPE_GETBYUID
	 * or ANOUBIS_PTYPE_SETBYUID.
	 */
	u_int32_t	 ptype;

	/**
	 * The authenticated user ID of the user that issued the request.
	 * All messages that belong to the same request must come with
	 * the same authenticated user ID.
	 */
	u_int32_t	 authuid;

	/**
	 * An open file descriptor for the policy that is being transferred
	 * from the user. It is unused for GET requests.
	 */
	int		 fd;

	/**
	 * The file descriptor that is used to write the signature
	 * of the policy. It is unused for GET requests.
	 */
	int		 fdsig;

	/**
	 * The priority of the policy that is transferred.
	 */
	int		 prio;

	/**
	 * The user ID of the policy. The administrator is allowed to
	 * read/write policies of other users.
	 */
	uid_t		 uid;

	/**
	 * The accumulated number of bytes written to the signature and
	 * the policy file. The first siglen bytes are written to the
	 * signature file, the rest to the  policy file.
	 */
	int		 written;

	/**
	 * The length of the signature as reported by the first message
	 * in the request.
	 */
	int		 siglen;

	/**
	 * The temporary name of the new policy file. It will be renamed
	 * to the realname after verification.
	 */
	char		*tmpname;

	/**
	 * The temporary name of the new signature file. Will be renamed
	 * to realsig.
	 */
	char		*tmpsig;

	/**
	 * The final name of the new policy file.
	 */
	char		*realname;

	/**
	 * The final name of the new signature file.
	 */
	char		*realsig;
};

/**
 * The global list of all active policy requests.
 */
LIST_HEAD(, pe_policy_request) preqs;

/* Prototypes */
static int			 pe_user_load_db(struct pe_policy_db *);
static int			 pe_user_load_dir(const char *, unsigned int,
				     struct pe_policy_db *);
static struct apn_ruleset	*pe_user_load_policy(const char *name,
				     int flags);
static void			 pe_user_insert_rs(struct apn_ruleset *,
				     uid_t, unsigned int,
				     struct pe_policy_db *);
static struct pe_user		*pe_user_get(uid_t, struct pe_policy_db *);

/**
 * Initialize the user database. This function is called at startup and
 * assumes that the current policy database is not yes initialized.
 */
void
pe_user_init(void)
{
	struct pe_policy_db	*pp;
	int		 count;

	LIST_INIT(&preqs);

	if ((pp = calloc(1, sizeof(struct pe_policy_db))) == NULL) {
		log_warn("calloc");
		master_terminate();
	}
	TAILQ_INIT(pp);

	/* We die gracefully if loading fails. */
	count = pe_user_load_db(pp);
	pdb = pp;

	log_info("pe_user_init: %d policies loaded to pdb %p", count, pp);
}

/**
 * Reconfigure the user database. This function tries to load the
 * policy data from disk. If this is successful the active policy database
 * is replaced with the new data and the old policy database is freed.
 * Before freeing the old database all contexts in all processes are
 * updated (pe_proc_update_db).
 */
void
pe_user_reconfigure(void)
{
	struct pe_policy_db	*newpdb, *oldpdb;
	int			 count;

	if ((newpdb = calloc(1, sizeof(struct pe_policy_db))) == NULL) {
		log_warn("calloc");
		master_terminate();
	}
	TAILQ_INIT(newpdb);
	count = pe_user_load_db(newpdb);

	/* Switch to new policy database */
	oldpdb = pdb;
	pdb = newpdb;

	pe_proc_update_db(newpdb);
	pe_user_flush_db(oldpdb);

	log_info("pe_user_reconfigure: loaded %d policies to new pdb %p, "
	    "flushed old pdb %p", count, newpdb, oldpdb);
}

/**
 * Free all memory associated with a policy database.
 *
 * @param ppdb The policy database. The active database is used if
 *     ppdb is NULL (only possible during shutdown).
 */
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

/**
 * Load the policy database from disk.
 *
 * @param A pre-allocated empty database.
 * @return The total number of policies loaded.
 */
static int
pe_user_load_db(struct pe_policy_db *p)
{
	int	count = 0;

	/* load admin policies */
	count = pe_user_load_dir(ANOUBISD_POLICYCHROOT "/" ANOUBISD_ADMINDIR,
	    PE_PRIO_ADMIN, p);

	/* load user policies */
	count += pe_user_load_dir(ANOUBISD_POLICYCHROOT "/" ANOUBISD_USERDIR,
	    PE_PRIO_USER1, p);

	return count;
}

/**
 * Load all policy files in a directory into the database. The user IDs
 * of the policies are derived from the names. Files that do not have
 * numerical names are skipped.
 *
 * If the user has a key the policy must be signed. Policies with missing
 * or invalid signatures are skipped and a warning is logged.
 *
 * All policies are clean when they are read from disk. This means that
 * rules that are no longer in scope are removed.
 *
 * @param dirname The directory to load policies from.
 * @param prio The priority of the policies in that directory. It this is
 *     PE_PRIO_ADMIN, the ruleset must not contain ASK rules.
 * @param p New policies are inserted into this policy database.
 * @return The number of policies loaded.
 */
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

	if ((dir = opendir(dirname)) == NULL) {
		log_warn("opendir");
		return 0;
	}

	if (prio != PE_PRIO_USER1)
		flags |= APN_FLAG_NOASK;

	count = 0;

	/* iterate over directory entries */
	while ((dp = readdir(dir)) != NULL) {
		DEBUG(DBG_PE_POLICY, "pe_user_load_dir: entry type=%d name=%s",
		    dp->d_type, dp->d_name);
		if (asprintf(&filename, "%s/%s", dirname, dp->d_name) < 0) {
			log_warnx("asprintf: Out of memory");
			continue;
		}
		if (dp->d_type == DT_UNKNOWN) {
			struct stat		statbuf;

			if (lstat(filename, &statbuf) < 0) {
				log_warn("Failed to stat %s", filename);
			} else if (S_ISREG(statbuf.st_mode)) {
				dp->d_type = DT_REG;
				DEBUG(DBG_PE_POLICY, "pe_user_load_dir: "
				    "st_mode=0x%x\n", statbuf.st_mode);
			}
		}
		/* Skip non-regular files and files starting with a dot. */
		if (dp->d_type != DT_REG || dp->d_name[0] == '.') {
			free(filename);
			continue;
		}

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
			free(filename);
			continue;
		} else {
			uid = strtonum(dp->d_name, 0, UID_MAX, &errstr);
			if (errstr) {
				log_warnx("pe_user_load_dir: "
				    "filename \"%s/%s\" %s",
				    dirname, dp->d_name, errstr);
				free(filename);
				continue;
			}
		}
		/*
		 * If root configured a certificate for the user, the policy
		 * must be signed and verified.
		 * For admin policies, root's certificate is used.
		 */
		pub = cert_get_by_uid_prio(uid, prio);
		if (pub != NULL) {
			if (anoubis_sig_verify_policy_file(filename,
			    pub->pubkey) != 1)  {
				log_warnx("not loading \"%s\", invalid "
				    "signature", filename);
				free(filename);
				continue;
			}
		}
		rs = pe_user_load_policy(filename, flags);
		free(filename);

		/* If parsing fails, we just continue */
		if (rs == NULL)
			continue;
		pe_user_insert_rs(rs, uid, prio, p);
		count++;
	}

	if (closedir(dir) == -1)
		log_warn("closedir");

	DEBUG(DBG_PE_POLICY, "pe_user_load_dir: %d policies inserted", count);

	return count;
}

/**
 * Check if a scope from a rule can still be valid. The scope is not
 * valid if its timeout has expired or if its process is no longer running.
 * This function is used as the callback for apn_clean_ruleset.
 *
 * @param scope The scope.
 * @param data The callback data as passed to apn_clean_ruleset. In our
 *     case this is a pointer to a time_t with the current time. If the
 *     current time is zero all scopes are removed from the policy. This is
 *     done on daemon startup where we do not want to keep scopes.
 * @return True if the scope should be removed. False if it still applies to
 *     at leas one process.
 */
static int
pe_user_scope_check(struct apn_scope * scope, void * data)
{
	time_t		now = *(time_t *)data;

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

/**
 * Load a policy from disk and return its parsed version. This function
 * is only called if the daemon is started or after a reload. Thus we have
 * to kill all scopes.
 *
 * @param name The name of the policy file. No signatures are checked, this
 *     must be done by the caller.
 * @return The cleaned ruleset. This ruleset destructor is set to
 *     &pe_prefixhash_destroy. In case of a parse error NULL is returned
 *     and a warning is issued.
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
		log_warnx("could not parse \"%s\"", name);
		return NULL;
	}
	if (ret != 0) {
		errstr = apn_one_error(rs);
		if (errstr)
			log_warnx("could not parse %s", errstr);
		else
			log_warnx("could not parse \"%s\"", name);
		apn_free_ruleset(rs);
		return NULL;
	}
	apn_clean_ruleset(rs, &pe_user_scope_check, &now);
	rs->destructor = (void*)&pe_prefixhash_destroy;
	return rs;
}

/**
 * Insert a new ruleset into the policy database replacing any previous
 * ruleset with the same user ID and priority. If the policy database is
 * NULL the global database is used. In this case the contexts of all
 * processes that use the old ruleset are refreshed, too. The refresh
 * does not happen if a database is specified explicitly.
 *
 * @param rs The new ruleset. This function takes over ownership of the
 *     ruleset. It will be freed if it gets replaced or the database
 *     is flushed.
 * @param uid The user ID of the new ruleset.
 * @param prio The priority of the new ruleset.
 * @param orig_p The database or NULL if the global database should be used
 *     and contexts should be refreshed.
 */
static void
pe_user_insert_rs(struct apn_ruleset *rs, uid_t uid, unsigned int prio,
    struct pe_policy_db *orig_p)
{
	struct pe_user		*user;
	struct pe_policy_db	*p = orig_p;
	struct apn_ruleset	*oldrs;

	if (p == NULL)
		p = pdb;
	/* Find or create user */
	if ((user = pe_user_get(uid, p)) == NULL) {
		if ((user = calloc(1, sizeof(struct pe_user))) == NULL) {
			log_warn("calloc");
			master_terminate();
		}
		user->uid = uid;
		TAILQ_INSERT_TAIL(p, user, entry);
	}
	oldrs = user->prio[prio];
	user->prio[prio] = rs;
	/*
	 * Refresh even if oldrs == NULL. New contexts will be created
	 * in this case.
	 */
	if (orig_p == NULL)
		pe_proc_update_db_one(oldrs, prio, uid);
	if (oldrs)
		apn_free_ruleset(oldrs);

	DEBUG(DBG_PE_POLICY, "pe_user_insert_rs: uid %d (%p prio %p, %p)",
	    (int)uid, user, user->prio[0], user->prio[1]);
}

/**
 * Return a pointer to the user structure for the given user ID.
 *
 * @param The user ID.
 * @param The policy database to search. If this is NULL the active
 *     database is used.
 * @return A pointer to the user structure or NULL if the user is
 *     not in the database.
 */
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

/**
 * Return a pointer to the ruleset of a user.
 *
 * @param uid The user ID of the user.
 * @param prio The priority of the ruleset.
 * @param p The policy database to search. If this is NULL the
 *     global database is used.
 * @return A pointer to the ruleset or NULL if the specified ruleset
 *     is not in the database. The ruleset is still owned by the database.
 */
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

/**
 * Build the filename for a policy on disk. This function concatenates
 * the policy base directory for the priority and the policy name
 * derived from the user ID. The string given by <code>pre</code> is
 * inserted as a prefix to the last component of the filename. This is
 * useful when constructing names for temporary files.
 *
 * @param uid The user ID. Use -1 for default policy filenames.
 * @param prio The priority of the policy.
 * @param pre The filename prefix. May be empty but not NULL.
 * @return The policy path name dynamically allocated.
 */
static char *
pe_policy_filename(uid_t uid, unsigned int prio, char *pre)
{
	char *name = NULL;

	if (prio >= PE_PRIO_MAX) {
		log_warnx("pe_policy_filename: illegal priority %d", prio);
		return NULL;
	}

	if (uid == (uid_t)-1) {
		if (asprintf(&name, "/%s/%s" ANOUBISD_DEFAULTNAME,
		    prio_to_string[prio], pre) == -1)
			return NULL;
	} else {
		if (asprintf(&name, "/%s/%s%d", prio_to_string[prio], pre, uid)
		    == -1)
			return NULL;
	}

	return name;
}

/**
 * Copy a policy to a temporary file and return an open file descriptor
 * to the temporary file. The ruleset is cleaned while copying it, i.e.
 * all outdated scopes are removed.
 *
 * The policy is read from disk not from the user database.
 *
 * @param uid The user ID of the policy.
 * @param prio The priority of the policy.
 * @return An open file descriptor that points to the temporary file.
 *     The temporary file is already unlinked, i.e. it will go away
 *     once the file descriptor is closed. A negative error code is returned
 *     in case of errors.
 */
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
		log_warn("Cannot get current time");
		master_terminate();
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

/**
 * Return a pointer to the policy request with the given token.
 *
 * @param token The token.
 * @return A pointer to the policy request or NULL if the token is
 *     not in use.
 */
static struct pe_policy_request *
pe_policy_request_find(u_int64_t token)
{
	struct pe_policy_request * req;
	LIST_FOREACH(req, &preqs, next) {
		if (req->token == token)
			return req;
	}
	return NULL;
}

/**
 * Free all resources associated with a policy request. This function closes
 * open file descriptors in the policy request and frees memory for path
 * names and the request itself.
 *
 * @param req The request.
 */
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

/**
 * Dump the user database to the log file.
 */
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

/**
 * Handle a policy request from the user. This function deals with new
 * requests, ongoing request and abort requests. The latter can happen
 * if the user closes the connection in the middle of an ongoing policy
 * request.
 *
 * @param The request message. It can be of type ANOUBISD_MSG_POLREQUEST_ABORT
 *     or ANOUBISD_MSG_POLREQUEST.
 * @return Either NULL if the function sent all neccessary messages
 *     itself or a message that must be sent to the session engine. The
 *     latter happens for requests that have an error or for requests
 *     that can be answered with a single reply message. GET requests
 *     send their messages using send_policy_data.
 */
struct anoubisd_msg *
pe_dispatch_policy(struct anoubisd_msg *msg)
{
	struct anoubisd_msg		*replymsg;
	struct anoubisd_msg_polreply	*reply = NULL;
	struct anoubisd_msg_polrequest	*polreq;
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

	if (msg == NULL) {
		log_warnx("pe_dispatch_policy: empty message");
		return NULL;
	}

	DEBUG(DBG_TRACE, ">pe_dispatch_policy");
	if (msg->mtype == ANOUBISD_MSG_POLREQUEST_ABORT) {
		struct anoubisd_msg_polrequest_abort	*abort;

		abort = (struct anoubisd_msg_polrequest_abort *)msg->msg;
		req = pe_policy_request_find(abort->token);
		if (req) {
			LIST_REMOVE(req, next);
			pe_policy_request_put(req);
		}
		DEBUG(DBG_TRACE, "<pe_dispatch_policy(abort)");
		return NULL;
	}
	if (msg->mtype != ANOUBISD_MSG_POLREQUEST) {
		log_warnx("pe_dispatch_policy; Bad message type %d",
		    msg->mtype);
		return NULL;
	}
	/*
	 * See if we have a policy request for this token. It is an error if
	 *  - there is no request and POLICY_FLAG_START is clear   or
	 *  - there is a request and POLICY_FLAG_START ist set.
	 */
	polreq = (struct anoubisd_msg_polrequest *)msg->msg;
	req = pe_policy_request_find(polreq->token);
	if ((req == NULL) == ((polreq->flags & POLICY_FLAG_START) == 0)) {
		if (req) {
			error = EBUSY;
		} else {
			error = ESRCH;
		}
		goto reply;
	}
	/* No request yet, start one. */
	if (req == NULL) {
		if (polreq->len < sizeof(Policy_Generic)) {
			log_warnx("pe_dispatch_policy: Wrong message size");
			error = EINVAL;
			goto reply;
		}
		gen = (Policy_Generic *)polreq->data;
		req = calloc(1, sizeof(struct pe_policy_request));
		if (!req) {
			error = ENOMEM;
			goto reply;
		}
		req->token = polreq->token;
		req->ptype = get_value(gen->ptype);
		req->authuid = polreq->auth_uid;
		req->written = 0;
		req->fd = -1;
		req->fdsig = -1;
		LIST_INSERT_HEAD(&preqs, req, next);
	}
	if (req->authuid != polreq->auth_uid) {
		error = EPERM;
		goto reply;
	}
	DEBUG(DBG_TRACE, " pe_dispatch_policy: ptype = %d, flags = %d "
	    "token = %" PRIu64, req->ptype, polreq->flags, req->token);
	switch (req->ptype) {
	case ANOUBIS_PTYPE_GETBYUID:
		if ((polreq->flags & POLICY_FLAG_END) == 0
		    || (polreq->len < sizeof(Policy_GetByUid))) {
			error = EINVAL;
			goto reply;
		}
		getbyuid = (Policy_GetByUid *)polreq->data;
		uid = get_value(getbyuid->uid);
		prio = get_value(getbyuid->prio);
		/*
		 * XXX CEH: Do more/better permission checks here!
		 * XXX CEH: Authorized user ID is in polreq->uid.
		 * XXX CEH: Currently this assumes Admin == root == uid 0
		 */
		if (polreq->auth_uid != uid && polreq->auth_uid != 0)
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
		error = - send_policy_data(polreq->token, fd);
		close(fd);
		if (error)
			goto reply;
		break;
	case ANOUBIS_PTYPE_SETBYUID:
		buf = polreq->data;
		len = polreq->len;
		if (polreq->flags & POLICY_FLAG_START) {
			char	*tmp;
			if (polreq->len < sizeof(Policy_SetByUid)) {
				log_warnx("pe_dispatch_policy: Wrong message "
				    "size from request");
				error = EINVAL;
				goto reply;
			}
			error = ENOMEM;
			setbyuid = (Policy_SetByUid *)polreq->data;
			uid = get_value(setbyuid->uid);
			prio = get_value(setbyuid->prio);
			if (prio >= PE_PRIO_MAX) {
				error = EINVAL;
				goto reply;
			}
			/*
			 * XXX CEH: Do more/better permission checks here!
			 * XXX CEH: Authorized user ID is in polreq->uid.
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
			key = cert_get_by_uid_prio(req->uid, req->prio);
			if (key && req->siglen <= 0) {
				log_warnx("pe_dispatch_policy: Found "
				    "public-key for uid %u and "
				    "expecting signature", req->uid);
				error = EINVAL;
				goto reply;
			}
			if (!req->realname)
				goto reply;
			tmp = pe_policy_filename(uid, prio, ".tmp.");
			if (!tmp)
				goto reply;
			/* splint doesn't understand the %" PRIu64 " modifier */
			if (asprintf(&req->tmpname, "%s.%" PRIu64, tmp,
			    req->token) == -1) {
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
		if (anoubisd_config.policysize > 0 &&
		    req->written + len > anoubisd_config.policysize) {
			/*
			 * Next write-operation will exceed maximum
			 * policy-size. Abort operation now.
			 */
			error = EFBIG;
			goto reply;
		}
		if (req->authuid != polreq->auth_uid) {
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
			req->written += ret;
			len -= ret;
		}
		if (polreq->flags & POLICY_FLAG_END) {
			int	 ret;
			struct apn_ruleset * ruleset = NULL;

			/*
			 * The case that a signature is required but
			 * req->siglen == 0 has been dealt with above.
			 * Here we only need to check the signature if
			 * req->siglen > 0 and only if a key is available.
			 */
			if (req->siglen > 0)
				key = cert_get_by_uid_prio(req->uid, req->prio);
			if (key) {
				ret = anoubis_sig_verify_policy_file(
				    req->tmpname, key->pubkey);
				if (ret != 1) {
					log_warnx("pe_dispatch_policy: "
					    "Coudn\'t verify policy for uid: "
					    "%u prio: %d", req->uid, req->prio);
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
			DEBUG(DBG_TRACE, "    pe_user_insert_rs uid=%d "
			    "prio=%d\n", req->uid, req->prio);
			pe_user_insert_rs(ruleset, req->uid, req->prio, NULL);
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
	if (req && (polreq->flags & POLICY_FLAG_END)) {
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
	replymsg = msg_factory(ANOUBISD_MSG_POLREPLY, sizeof(*reply));
	if (!replymsg) {
		log_warnx("pe_dispatch_policy: Out of memory\n");
		master_terminate();
	}
	reply = (struct anoubisd_msg_polreply *)replymsg->msg;
	reply->token = polreq->token;
	reply->len = 0;
	reply->flags = POLICY_FLAG_START|POLICY_FLAG_END;
	reply->reply = error;
	DEBUG(DBG_TRACE, "<pe_dispatch_policy: %d", error);
	return replymsg;
}

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

#ifndef _APNVM_H_
#define _APNVM_H_

#include <sys/cdefs.h>

#include "../../libapn/apn.h"

struct _apnvm;
typedef struct _apnvm apnvm;

struct apnvm_user {
	char	*username;

	LIST_ENTRY(apnvm_user) entry;
};

LIST_HEAD(apnvm_user_head, apnvm_user);

struct apnvm_version {
	int	no;		/* Version number */
	time_t	tstamp;		/* Timestamp of version */
	char	*comment;	/* A comment */
	int	auto_store;	/* Auto-store-flag */

	TAILQ_ENTRY(apnvm_version) entries;
};

TAILQ_HEAD(apnvm_version_head, apnvm_version);

struct apnvm_md {
	const char	*comment;
	int		auto_store;
};

typedef enum {
	APNVM_OK = 0,	/* The operation was successful */
	APNVM_VMS,	/* An error was encountered from the underlaying */
			/* version management system */
	APNVM_PARSE,	/* Failed to parse a ruleset */
	APNVM_ARG,	/* Invalid argument */
	APNVM_OOM	/* Out of memory */
} apnvm_result;

__BEGIN_DECLS

/**
 * Initializes the library.
 *
 * The first paramater holds the path to the CVS-repository.
 * The second parameter is the username of the user, which is using the
 * library. This is the user, which administrates a set of policy-sets.
 *
 * A library-handle is returned.
 */
apnvm *apnvm_init(const char *, const char *);

/**
 * Destroys the library-handle.
 */
void apnvm_destroy(apnvm *);

/**
 * Prepares the library.
 *
 * For example, the user-module of the CVS-repository is checked out.
 * You need to invoke the function after initialization and before any
 * library-operation.
 */
apnvm_result apnvm_prepare(apnvm *vm);

/**
 * Receive a list of managed users.
 * These are the user, for whom the policy-sets apply.
 *
 * The result is written into the list. Do not forget to initialize the list
 * with LIST_INIT().
 */
apnvm_result apnvm_getuser(apnvm *, struct apnvm_user_head *);

/**
 * Determines the number of versions for a specified user.
 *
 * The second parameter holds the username, the third parameter holds the
 * profile name. This username can vary from the user specified during
 * initialization! This is the user, for whom the policy-set applies.
 *
 * The function writes the number of versions into the fourth parameter.
 */
apnvm_result apnvm_count(apnvm *, const char *, const char *, int *);

/**
 * Determines a list of versions for the specified user.
 *
 * The second parameter holds the username, the third parameter holds the
 * profile name. This username can vary from the user specified during
 * initialization! This is the user, for whom the policy-set applies.
 *
 * The third parameter is the head of a version-list. The function appends
 * detected versions to the list. The list is not initialized. You need to
 * call TAILQ_INIT(...) from outside. You can use apnvm_version_head_free() to
 * destroy the list again.
 */
apnvm_result apnvm_list(apnvm *, const char *, const char *,
    struct apnvm_version_head *);

/**
 * Helper function to destroy the list created by apnvm_list().
 */
void apnvm_version_head_free(struct apnvm_version_head *);

/**
 * Fetches a ruleset from the versioning-system.
 *
 * The second parameter holds the username. This username can vary from the
 * user specified during initialization! This is the user, for whom the
 * policy-set applies.
 *
 * The third parameter holds the version-number to be fetched. Use apnvm_list()
 * to receive a list of versions-numbers! If you apply am unknown number,
 * APNVM_VMS is returned.
 *
 * The fourth parameter specifies the profile to be fetched. If the profile
 * does not exists in the versioning-system, still APNVM_OK is
 * returned but *rs is set to NULL.
 *
 * APNVM_PARSE is returned if a ruleset was fetched but the following
 * parsing-operation failed.
 */
apnvm_result apnvm_fetch(apnvm *, const char *, int, const char *,
    struct apn_ruleset **);

/**
 * Inserts a ruleset into the versioning-system.
 *
 * The second parameter holds the username. This username can vary from the
 * user specified during initialization! This is the user, for whom the
 * policy-set applies. When no rulesets exists for the user, a new version-line
 * is created.
 *
 * The third parameter holds the name of the profile. If the profile
 * already exists for the user, the ruleset behind the profile is
 * overwritten, otherwise the profile is appended to the profile list.
 *
 * Last you specify metadata for the version, which can be NULL. In this case
 * you are not specifying any metadata.
 */
apnvm_result apnvm_insert(apnvm *, const char *, const char *,
    struct apn_ruleset *, struct apnvm_md *);

/**
 * Remoes a version from the versioning-system.
 *
 * The second parameter holds the username, the third parameter holds the
 * name of the profile. This username can vary from the user specified
 * during initialization! This is the user, for whom the policy-set applies.
 *
 * The fourth parameter specuifies the version to be removed. Use apnvm_list()
 * to receive a list of versions-numbers.
 *
 * On success APNVM_OK is returned. If the user and/or version does not exist,
 * APNVM_VMS is returned.
 */
apnvm_result apnvm_remove(apnvm *, const char *, const char *, int);

__END_DECLS

#endif	/* _APNVM_H_ */

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

struct apnvm_version {
	int	no;
	time_t	tstamp;

	TAILQ_ENTRY(apnvm_version) entries;
};

TAILQ_HEAD(apnvm_version_head, apnvm_version);

typedef enum {
	APNVM_OK,	/* The operation was successful */
	APNVM_VMS,	/* An error was encountered from the underlaying */
			/* version management system */
	APNVM_ARG	/* Invalid argument */
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
 * Determines the number of versions for a specified user.
 *
 * The second parameter holds the username. This username can vary from the
 * user specified during initialization! This is the user, for whom the
 * policy-set applies.
 *
 * The function writes the number of versions into the third parameter.
 */
apnvm_result apnvm_count(apnvm *, const char *, int *);

/**
 * Determines a list of versions for the specified user.
 *
 * The second parameter holds the username. This username can vary from the
 * user specified during initialization! This is the user, for whom the
 * policy-set applies.
 *
 * The third parameter is the head of a version-list. The function appends
 * detected versions to the list. The list is not initialized. You need to
 * call TAILQ_INIT(...) from outside. Do not forget to destroy the list again!
 */
apnvm_result apnvm_list(apnvm *, const char *, struct apnvm_version_head *);

__END_DECLS

#endif	/* _APNVM_H_ */

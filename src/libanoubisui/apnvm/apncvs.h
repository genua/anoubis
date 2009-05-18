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

#ifndef _APNCVS_H_
#define _APNCVS_H_

#include "config.h"

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#ifndef NEEDBSDCOMPAT
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <time.h>

#include "apnvm.h"

/**
 * Structure that holds common used information.
 */
struct apncvs {
	char *cvsroot;	/* CVSROOT */
	char *workdir;	/* Working directory */
	char *module;	/* The module you are working on */
	apnvm_pidcallback_t	pidcallback;
};

/**
 * A single revision.
 */
struct apncvs_rev {
	TAILQ_ENTRY(apncvs_rev)	entry;
	char			*revno;		/* Number of revision */
	time_t			date;		/* Commit timestamp */
	char			*author;	/* Author */
	char			*state;		/* state */
	char			*comment;	/* Comment */
};
TAILQ_HEAD(apncvsrev_queue, apncvs_rev);

/**
 * Revision information of a single file.
 */
struct apncvs_log {
	char	*rcs_file;		/* Path of file in repository */
	char	*working_file;		/* Path of file in working dir */
	char	*head;			/* Head revision */
	char	*branch;		/* Branch */
	char	*locks;			/* Locks */
	char	*access_list;		/* access list */
	char	*symbolic_names;	/* Symbolic names */
	char	*keyword_substitution;	/* Keyword substitution */
	int	total_revisions;	/* Total number of revisions in */
					/* repository */
	int	selected_revisions;	/* Number of selected revisions */
	char	*description;		/* Description of file */

	/*
	 * List of revisions
	 * Number of entries is defined by selected_revisions
	 */
	struct apncvsrev_queue rev_queue;
};

/**
 * Initializes a repository.
 *
 * The repository is located ar "cvs->cvsroot".
 *
 * The return code of the "cvs init" command is returned, thus a 0 means
 * success.
 */
int apncvs_init(struct apncvs *);

/**
 * Checks out a CVS repository.
 *
 * The repository is located at "cvs->cvsroot" and the "cvs->module"-module
 * is checked out. The working copy is created in "cvs->workdir". Make sure,
 * that the working directory exists!
 *
 * The return code of the "cvs checkout" command is returned, thus a 0 means
 * success.
 */
int apncvs_checkout(struct apncvs *);

/**
 * Updates a file to the specified revision.
 *
 * The repository is located at "cvs->cvsroot". The module "cvs->module" is
 * already checked out into the working directory "cvs->workdir". The file
 * "file" is already checkin in under the module-directory.
 *
 * If "rev" is not NULL, the function updates to the specified revision,
 * otherwise it updates to the latest revision (HEAD).
 *
 * The return code of the "cvs update" command is returned, thus a 0 means
 * success.
 */
int apncvs_update(struct apncvs *, const char *, const char *);

/**
 * Receives history-information about a single file in a repository.
 *
 * The repository is located at "cvs->cvsroot". The module "cvs->module" is
 * already checked out into the working directory "cvs->workdir". The file
 * "file" is already checkin in under the module-directory.
 *
 * The history-information are written into "log". No memory is allocated for
 * "log" itself but for members of the structure. Use apncvs_log_destroy()
 * to release the memory again!
 *
 * The return code of the "cvs log" command is returned, thus a 0 means
 * success.
 */
int apncvs_log(struct apncvs *, const char *, struct apncvs_log *);

/**
 * Releases memory allocated for the members of the specified log-stucture.
 */
void apncvs_log_destroy(struct apncvs_log *);

/**
 * Commits changes of the specified file into the CVS repository.
 *
 * The repository is located at "cvs->cvsroot". The module "cvs->module" is
 * already checked out into the working directory "cvs->workdir". The file
 * "file" is already checkin in or added to the repository under the
 * module-directory.
 *
 * "comment" is used to comment the new revision.
 *
 * The return code of the "cvs commit" command is returned, thus a 0 means
 * success. Note: If you are committing a file with no changes, 0 is
 * returned, but no revision is created.
 */
int apncvs_commit(struct apncvs *, const char *, const char *);

/**
 * Appends a new file to a CVS repository.
 *
 * The repository is located at "cvs->cvsroot". The module "cvs->module" is
 * already checked out into the working directory "cvs->workdir". The file
 * "file" is not under revision control but already exists in the working
 * "directory.
 *
 * The return code of the "cvs add" command is returned, thus a 0 means
 * success. Note: Do not forget to commit the add-operation!
 */
int apncvs_add(struct apncvs *, const char *);

/**
 * Removes a revision from the history of a file.
 *
 * The repository is located at "cvs->cvsroot". The module "cvs->module" is
 * already checked out into the working directory "cvs->workdir". The file
 * "file" is already checkin in under the module-directory.
 *
 * The revision named "rev" is removed.
 *
 * The return code of the "cvs add" command is returned, thus a 0 means
 * success.
 */
int apncvs_remrev(struct apncvs *, const char *, const char *);

#endif	/* _APNCVS_H_ */

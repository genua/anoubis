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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "apncvs.h"
#include "apnvm.h"

#ifndef APNVM_WORKDIR
#define APNVM_WORKDIR "/tmp/apnvm_XXXXXX"
#endif

#define APNVM_CHECKPTR(ptr, rc) \
	if (ptr == NULL)	\
		return (rc)

#define APNVM_CHECKSTR(str, rc)	\
	if ((str == NULL) || (strlen(str) == 0))	\
		return (rc)

#define APNVM_CHECKCVS(p, rc)		\
	APNVM_CHECKPTR(p, rc);		\
	APNVM_CHECKSTR(p->cvsroot, rc);	\
	APNVM_CHECKSTR(p->workdir, rc);	\
	APNVM_CHECKSTR(p->module, rc)

#define APNVM_CHECKVM(vm, rc)		\
	APNVM_CHECKPTR(vm, rc);		\
	APNVM_CHECKCVS((&vm->cvs), rc)

struct _apnvm {
	struct apncvs cvs;
};

static int
apnvm_rmdir(const char *path)
{
	DIR		*dir;
	struct dirent	*dp;

	APNVM_CHECKSTR(path, -1);

	if ((dir = opendir(path)) == NULL)
		return (-1);

	while ((dp = readdir(dir)) != NULL) {
		char		childpath[256];
		struct stat	fstat;

		if ((strcmp(dp->d_name, ".") == 0) ||
		    (strcmp(dp->d_name, "..") == 0))
			continue;

		/* Build path of directory entry */
		snprintf(childpath, sizeof(childpath), "%s/%s",
		    path, dp->d_name);

		if (stat(childpath, &fstat) != 0) {
			closedir(dir);
			return (0);
		}

		if (S_ISDIR(fstat.st_mode)) {
			if (!apnvm_rmdir(childpath))
				return (0);
		}
		else {
			if (unlink(childpath) != 0)
				return (0);
		}
	}

	closedir(dir);

	return (rmdir(path) == 0);
}

static int
apnvm_havemodule(struct apncvs *cvs)
{
	char		path[256];
	struct stat	fstat;

	APNVM_CHECKCVS(cvs, 0);

	snprintf(path, sizeof(path), "%s/%s", cvs->cvsroot, cvs->module);

	if (stat(path, &fstat) != 0)
		return (0);

	return S_ISDIR(fstat.st_mode);
}

static int
apnvm_havefile(struct apncvs *cvs, const char *file)
{
	char		path[256];
	struct stat	fstat;

	APNVM_CHECKCVS(cvs, 0);
	APNVM_CHECKSTR(file, 0);

	snprintf(path, sizeof(path), "%s/%s/%s",
	    cvs->workdir, cvs->module, file);

	if (stat(path, &fstat) != 0)
		return (0);

	return S_ISREG(fstat.st_mode);
}

static int
apnvm_createmodule(struct apncvs *cvs)
{
	char path[256];
	const mode_t mode = S_IRWXU;

	APNVM_CHECKCVS(cvs, 0);

	snprintf(path, sizeof(path), "%s/%s", cvs->cvsroot, cvs->module);
	return (mkdir(path, mode) == 0);
}

static int
revno_to_no(const char *revno)
{
	char *p;

	APNVM_CHECKSTR(revno, 0);

	p = rindex(revno, '.');

	if ((p != NULL) && (*(p + 1) != '\0'))
		return atoi(p + 1);
	else
		return (0);
}

apnvm *
apnvm_init(const char *repository, const char *user)
{
	struct _apnvm *vm;

	APNVM_CHECKSTR(repository, NULL);
	APNVM_CHECKSTR(user, NULL);

	if ((vm = malloc(sizeof(struct _apnvm))) == NULL)
		return (NULL);

	vm->cvs.cvsroot = strdup(repository);
	vm->cvs.module = strdup(user);

	vm->cvs.workdir = strdup(APNVM_WORKDIR);
	vm->cvs.workdir = mkdtemp(vm->cvs.workdir);

	return (vm);
}

void
apnvm_destroy(apnvm *vm)
{
	if (vm == NULL)
		return;

	apnvm_rmdir(vm->cvs.workdir);

	free(vm->cvs.cvsroot);
	free(vm->cvs.module);
	free(vm->cvs.workdir);

	free(vm);
}

apnvm_result
apnvm_prepare(apnvm *vm)
{
	APNVM_CHECKVM(vm, APNVM_ARG);

	/* Check weather the module exists */
	if (!apnvm_havemodule(&vm->cvs) && !apnvm_createmodule(&vm->cvs))
		return (APNVM_VMS);

	/* Checkout module */
	if (apncvs_checkout(&vm->cvs) != 0)
		return (APNVM_VMS);

	return (APNVM_OK);
}

apnvm_result
apnvm_count(apnvm *vm, const char *user, int *count)
{
	struct apncvs_log	log;
	int			result;

	APNVM_CHECKVM(vm, APNVM_ARG);
	APNVM_CHECKSTR(user, APNVM_ARG);
	APNVM_CHECKPTR(count, APNVM_ARG);

	if (!apnvm_havefile(&vm->cvs, user)) {
		/* No file for the user -> no versions */
		*count = 0;
		return (APNVM_OK);
	}

	result = apncvs_log(&vm->cvs, user, &log);
	if (result != 0)
		return (APNVM_VMS);

	*count = log.selected_revisions;
	apncvs_log_destroy(&log);

	return (APNVM_OK);
}

apnvm_result
apnvm_list(apnvm *vm, const char *user, struct apnvm_version_head *version_list)
{
	struct apncvs_log	log;
	struct apncvs_rev	*rev;
	int			result;

	APNVM_CHECKVM(vm, APNVM_ARG);
	APNVM_CHECKSTR(user, APNVM_ARG);
	APNVM_CHECKPTR(version_list, APNVM_ARG);

	if (!apnvm_havefile(&vm->cvs, user)) {
		/* No file for the user -> no versions */
		return (APNVM_OK);
	}

	result = apncvs_log(&vm->cvs, user, &log);
	if (result != 0)
		return (APNVM_VMS);

	/* Collect all revisions and create versions from it */
	TAILQ_FOREACH(rev, &log.rev_queue, entry) {
		struct apnvm_version *version;

		version = malloc(sizeof(struct apnvm_version));
		if (version == NULL)
			continue;

		version->no = revno_to_no(rev->revno);
		version->tstamp = rev->date;

		TAILQ_INSERT_TAIL(version_list, version, entries);
	}

	return (APNVM_OK);
}

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

#include <config.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif
#include <sys/queue.h>

#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <dirent.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "anoubis_apnvm_cvs.h"
#include "anoubis_apnvm_md.h"
#include "anoubis_apnvm.h"

#ifndef APNVM_WORKDIR
#define APNVM_WORKDIR "apnvm_XXXXXX"
#endif

#define APNVM_CHECKPTR(ptr, rc) \
	if (ptr == NULL)	\
		return (rc)

#define APNVM_CHECKSTR(str, rc)	\
	if (((str) == NULL) || *(str) == '\0')	\
		return (rc)

#define APNVM_CHECKCVS(p, rc)		\
	APNVM_CHECKPTR(p, rc);		\
	APNVM_CHECKSTR(p->cvsroot, rc);	\
	APNVM_CHECKSTR(p->workdir, rc);	\
	APNVM_CHECKSTR(p->module, rc)

#define APNVM_CHECKVM(vm, rc)		\
	APNVM_CHECKPTR(vm, rc);		\
	APNVM_CHECKCVS((&vm->cvs), rc)

#ifndef APNVM_COMMENT
	#define APNVM_COMMENT "comment"
#endif

#ifndef APNVM_AUTOSTORE
	#define APNVM_AUTOSTORE "autostore"
#endif

#ifndef APNVM_DEFCOMMENT
	#define APNVM_DEFCOMMENT "*****"
#endif

struct _apnvm {
	struct apncvs cvs;
};

struct file_ruleset {
	int	fd;
	char	*buf;
	size_t	sbuf;
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
		char		childpath[PATH_MAX];
		int		nwritten;
		struct stat	fstat;

		if ((strcmp(dp->d_name, ".") == 0) ||
		    (strcmp(dp->d_name, "..") == 0))
			continue;

		/* Build path of directory entry */
		nwritten = snprintf(childpath, sizeof(childpath), "%s/%s",
		    path, dp->d_name);
		if ((nwritten >= PATH_MAX) || (nwritten < 0)) {
			closedir(dir);
			return (0);
		}

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
	char		path[PATH_MAX];
	int		nwritten;
	struct stat	fstat;

	APNVM_CHECKCVS(cvs, 0);

	nwritten = snprintf(path, sizeof(path), "%s/%s",
	    cvs->cvsroot, cvs->module);
	if ((nwritten >= PATH_MAX) || (nwritten < 0))
		return (0);

	if (stat(path, &fstat) != 0)
		return (0);

	return S_ISDIR(fstat.st_mode);
}

static int
apnvm_havefile(struct apncvs *cvs, const char *file)
{
	char		path[PATH_MAX];
	int		nwritten;
	struct stat	fstat;

	APNVM_CHECKCVS(cvs, 0);
	APNVM_CHECKSTR(file, 0);

	nwritten = snprintf(path, sizeof(path), "%s/%s/%s",
	    cvs->workdir, cvs->module, file);
	if ((nwritten >= PATH_MAX) || (nwritten < 0))
		return (0);

	if (stat(path, &fstat) != 0)
		return (0);

	return S_ISREG(fstat.st_mode);
}

static int
apnvm_createfile(struct apncvs *cvs, const char *user, const char *profile)
{
	char	path[PATH_MAX];
	char	dir[PATH_MAX];
	int	nwritten;
	int	fd;
	int	result;

	APNVM_CHECKCVS(cvs, 0);
	APNVM_CHECKSTR(user, 0);
	APNVM_CHECKSTR(profile, 0);

	nwritten = snprintf(dir, PATH_MAX, "%s/%s/%s",
	    cvs->workdir, cvs->module, user);
	if ((nwritten >= PATH_MAX) || (nwritten < 0))
		return (0);
	nwritten = snprintf(path, PATH_MAX, "%s/%s", dir, profile);
	if ((nwritten >= PATH_MAX) || (nwritten < 0))
		return (0);

	if (mkdir(dir, 0700) < 0) {
		if (errno != EEXIST)
			return 0;
	} else {
		result = apncvs_add(cvs, user);
		if (result != 0)
			return 0;
	}
	/* Create an empty file */
	if ((fd = open(path, O_WRONLY|O_CREAT, 0600)) < 0) {
		perror("open");
		return (0);
	}
	close(fd);

	/* Must be ok because length check above was ok. */
	snprintf(path, PATH_MAX, "%s/%s", user, profile);
	/* Append to repository. */
	result = apncvs_add(cvs, path);

	return (result == 0);
}

static int
apnvm_createmodule(struct apncvs *cvs)
{
	char		path[PATH_MAX];
	int		nwritten;
	const mode_t	mode = S_IRWXU;

	APNVM_CHECKCVS(cvs, 0);

	nwritten = snprintf(path, sizeof(path), "%s/%s",
	    cvs->cvsroot, cvs->module);
	if ((nwritten >= PATH_MAX) || (nwritten < 0))
		return (0);

	return (mkdir(path, mode) == 0);
}

static FILE *
apnvm_simpleopen(struct apncvs *cvs, const char *file, const char *mode)
{
	char	path[PATH_MAX];
	int	nwritten;

	nwritten = snprintf(path, sizeof(path), "%s/%s/%s",
	    cvs->workdir, cvs->module, file);
	if ((nwritten >= PATH_MAX) || (nwritten < 0))
		return (NULL);

	return fopen(path, mode);
}

static int
apnvm_openfile(struct apncvs *cvs, const char *file, struct file_ruleset *frs)
{
	char	path[PATH_MAX], *buf;
	int	nwritten;
	int	fd;
	off_t	sbuf;

	nwritten = snprintf(path, sizeof(path), "%s/%s/%s",
	    cvs->workdir, cvs->module, file);
	if ((nwritten >= PATH_MAX) || (nwritten < 0))
		return (0);

	/* Open file for reading */
	if ((fd = open(path, O_RDONLY)) == -1)
		return (0);

	/* Detect filesize by jumping to the end */
	sbuf = lseek(fd, 0, SEEK_END);
	if (sbuf == (off_t)-1) {
		close(fd);
		return (0);
	}

	/* Jump back to the start */
	if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
		close(fd);
		return (0);
	}

	if (sbuf > 0) {
		buf = mmap(NULL, sbuf, PROT_READ, MAP_PRIVATE, fd, 0);
		if (buf == MAP_FAILED) {
			close(fd);
			return (0);
		}
	}
	else
		buf = NULL;

	frs->fd = fd;
	frs->buf = buf;
	frs->sbuf = sbuf;

	return (1);
}

static int
apnvm_closefile(struct file_ruleset *frs)
{
	int rc = 1;

	if (munmap(frs->buf, frs->sbuf) == -1)
		rc = 0;

	if (close(frs->fd) == -1)
		rc = 0;

	return (rc);
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

static int
no_to_revno(int no, char *revno, size_t len)
{
	int nwritten = snprintf(revno, len, "1.%i", no);
	return ((nwritten >= 0) && ((unsigned int)nwritten < len));
}

apnvm *
apnvm_init(const char *repository, const char *user, apnvm_pidcallback_t cb)
{
	struct _apnvm *vm;
	char *tmpdir, *workdir = NULL;

	APNVM_CHECKSTR(repository, NULL);
	APNVM_CHECKSTR(user, NULL);

	if ((vm = calloc(1, sizeof(struct _apnvm))) == NULL)
		return (NULL);

	vm->cvs.cvsroot = strdup(repository);
	if (vm->cvs.cvsroot == NULL)
		goto error;
	vm->cvs.module = strdup(user);
	if (vm->cvs.module == NULL)
		goto error;

	tmpdir = getenv("TMPDIR");
	if (!tmpdir)
		tmpdir = "/tmp";
	if (asprintf(&workdir, "%s/%s", tmpdir, APNVM_WORKDIR) == -1) {
		workdir = NULL;
		goto error;
	}

	vm->cvs.workdir = mkdtemp(workdir);
	if (vm->cvs.workdir == NULL)
		goto error;
	vm->cvs.pidcallback = cb;

	return (vm);

error:
	if (vm->cvs.cvsroot)
		free(vm->cvs.cvsroot);
	if (vm->cvs.module)
		free(vm->cvs.module);
	if (workdir)
		free(workdir);
	free(vm);
	return (NULL);
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

	if (apncvs_init(&vm->cvs) != 0) {
		/* Initialization failed, repository cannot be created */
		return (APNVM_VMS);
	}

	/* Check whether the module exists */
	if (!apnvm_havemodule(&vm->cvs) && !apnvm_createmodule(&vm->cvs))
		return (APNVM_VMS);

	/* Checkout module */
	if (apncvs_checkout(&vm->cvs) != 0)
		return (APNVM_VMS);

	return (APNVM_OK);
}

apnvm_result
apnvm_getuser(apnvm *vm, struct apnvm_user_head *user_list)
{
	char		path[PATH_MAX];
	int		nwritten;
	DIR		*dir;
	struct	dirent	*de;

	APNVM_CHECKVM(vm, APNVM_ARG);
	APNVM_CHECKPTR(user_list, APNVM_ARG);

	/* Build path of module-directory */
	nwritten = snprintf(path, sizeof(path), "%s/%s", vm->cvs.workdir,
	    vm->cvs.module);
	if (nwritten >= PATH_MAX || nwritten < 0)
		return (APNVM_OOM);

	/*
	 * Scan module-directory for directories.
	 * For each managed user a separate directory is created.
	 */
	if ((dir = opendir(path)) == NULL)
		return (APNVM_VMS);

	while ((de = readdir(dir)) != NULL) {
		struct apnvm_user *user;
		if (de->d_type == DT_UNKNOWN) {
			char		*tmppath;
			struct stat	 statbuf;

			if (asprintf(&tmppath, "%s/%s", path, de->d_name) < 0) {
				closedir(dir);
				return APNVM_OOM;
			}
			if (lstat(tmppath, &statbuf) < 0) {
				free(tmppath);
				continue;
			}
			free(tmppath);
			if (!S_ISDIR(statbuf.st_mode))
				continue;
		} else if (de->d_type != DT_DIR)
			continue;
		if (de->d_name[0] == '.') {
			if (de->d_name[1] == 0)
				continue;
			if (de->d_name[1] == '.' && de->d_name[2] == 0)
				continue;
		}
		if (strcmp(de->d_name, "CVS") == 0)
			continue;

		user = malloc(sizeof(struct apnvm_user));
		if (user == NULL) {
			closedir(dir);
			return (APNVM_OOM);
		}

		user->username = strdup(de->d_name);
		if (user->username == NULL) {
			closedir(dir);
			free(user);
			return (APNVM_OOM);
		}

		LIST_INSERT_HEAD(user_list, user, entry);
	}

	closedir(dir);

	return (APNVM_OK);
}

apnvm_result
apnvm_count(apnvm *vm, const char *user, const char *profile, int *count)
{
	struct apncvs_log	log;
	int			result;
	char			file[PATH_MAX];

	APNVM_CHECKVM(vm, APNVM_ARG);
	APNVM_CHECKSTR(user, APNVM_ARG);
	APNVM_CHECKSTR(profile, APNVM_ARG);
	APNVM_CHECKPTR(count, APNVM_ARG);

	result = snprintf(file, PATH_MAX, "%s/%s", user, profile);
	if (result < 0 || result >= PATH_MAX)
		return APNVM_ARG;
	if (!apnvm_havefile(&vm->cvs, file)) {
		/* No file for the user -> no versions */
		*count = 0;
		return (APNVM_OK);
	}

	result = apncvs_log(&vm->cvs, file, &log);
	if (result != 0)
		return (APNVM_VMS);

	*count = log.selected_revisions;
	apncvs_log_destroy(&log);

	return (APNVM_OK);
}

apnvm_result
apnvm_list(apnvm *vm, const char *user, const char *profile,
    struct apnvm_version_head *version_list)
{
	struct apncvs_log	log;
	struct apncvs_rev	*rev;
	int			result;
	char			file[PATH_MAX];

	APNVM_CHECKVM(vm, APNVM_ARG);
	APNVM_CHECKSTR(user, APNVM_ARG);
	APNVM_CHECKSTR(profile, APNVM_ARG);
	APNVM_CHECKPTR(version_list, APNVM_ARG);

	result = snprintf(file, PATH_MAX, "%s/%s", user, profile);
	if (result < 0 || result >= PATH_MAX)
		return (APNVM_ARG);
	if (!apnvm_havefile(&vm->cvs, file)) {
		/* No file for the user -> no versions */
		return (APNVM_OK);
	}

	result = apncvs_log(&vm->cvs, file, &log);
	if (result != 0)
		return (APNVM_VMS);

	/* Collect all revisions and create versions from it */
	TAILQ_FOREACH(rev, &log.rev_queue, entry) {
		struct apnvm_version	*version;
		apnmd			*md;

		version = malloc(sizeof(struct apnvm_version));
		if (version == NULL)
			continue;

		version->no = revno_to_no(rev->revno);
		version->tstamp = rev->date;
		version->comment = NULL;

		/* Parse metadata from revision-comment */
		md = apnmd_parse(rev->comment);
		if (md != NULL) {
			char *comment = apnmd_get(md, APNVM_COMMENT);
			int auto_store = apnmd_get_int(md, APNVM_AUTOSTORE);

			if (comment != NULL)
				version->comment = strdup(comment);
			version->auto_store = auto_store;

			apnmd_destroy(md);
		}

		TAILQ_INSERT_TAIL(version_list, version, entries);
	}

	return (APNVM_OK);
}

void
apnvm_version_head_free(struct apnvm_version_head *head)
{
	if (head == NULL)
		return;

	while (!TAILQ_EMPTY(head)) {
		struct apnvm_version *version = TAILQ_FIRST(head);

		TAILQ_REMOVE(head, version, entries);

		free(version->comment);
		free(version);
	}
}

apnvm_result
apnvm_fetch(apnvm *vm, const char *user, int no, const char *profile,
    struct apn_ruleset **rs)
{
	struct file_ruleset	frs = {0, 0, 0};
	struct iovec		iov;
	char			revno[12];
	int			result;
	char			file[PATH_MAX];

	APNVM_CHECKVM(vm, APNVM_ARG);
	APNVM_CHECKSTR(user, APNVM_ARG);
	APNVM_CHECKSTR(profile, APNVM_ARG);
	APNVM_CHECKPTR(rs, APNVM_ARG);

	/* Convert to CVS-revision no */
	if (!no_to_revno(no, revno, sizeof(revno)))
		return (APNVM_OOM);

	result = snprintf(file, PATH_MAX, "%s/%s", user, profile);
	if (result < 0 || result > PATH_MAX)
		return APNVM_ARG;
	if (!apnvm_havefile(&vm->cvs, file)) {
		/* No file for the user -> no versions */
		*rs = NULL;
		return (APNVM_OK);
	}

	/* Update to requested revision */
	result = apncvs_update(&vm->cvs, file, revno);
	if (result != 0) {
		*rs = NULL;
		return (APNVM_VMS);
	}

	/*
	 * Open file. Put the result into the frs-structure. It contains a
	 * file-destriptor and the complete content of the file.
	 */
	result = apnvm_openfile(&vm->cvs, file, &frs);
	if (!result) {
		*rs = NULL;
		return (APNVM_VMS);
	}

	/*
	 * The frs-structure holds the complete content of the file.
	 * Next find out the section, where the requested profile is stored.
	 * Put the result into the iov-structure.
	 */
	iov.iov_base = frs.buf;
	iov.iov_len = frs.sbuf;

	/* Parse ruleset */
	result = apn_parse_iovec("<iov>", &iov, 1, rs, 0);
	if (result)
		apn_free_ruleset(*rs);

	/* Close file again */
	apnvm_closefile(&frs);

	return (result == 0) ? APNVM_OK : APNVM_PARSE;
}

apnvm_result
apnvm_insert(apnvm *vm, const char *user, const char *profile,
    struct apn_ruleset *rs, struct apnvm_md *vmd)
{
	int			result;
	char			*rev_comment;
	FILE			*fh;
	char			file[PATH_MAX];

	APNVM_CHECKVM(vm, APNVM_ARG);
	APNVM_CHECKSTR(user, APNVM_ARG);
	APNVM_CHECKSTR(profile, APNVM_ARG);
	APNVM_CHECKPTR(rs, APNVM_ARG);

	result = snprintf(file, PATH_MAX, "%s/%s", user, profile);
	if (result < 0 || result >= PATH_MAX)
		return APNVM_ARG;
	if (apnvm_havefile(&vm->cvs, file)) {
		/* Update to HEAD to receive the latest content */
		result = apncvs_update(&vm->cvs, file, NULL);
		if (result != 0)
			return (APNVM_VMS);
	} else {
		/* File not in repository, append now */
		result = apnvm_createfile(&vm->cvs, user, profile);
		if (!result)
			return (APNVM_VMS);
	}

	/* Open file again, content will be overwritten */
	fh = apnvm_simpleopen(&vm->cvs, file, "w");

	/* (2) Dump profile */
	apn_print_ruleset(rs, 0, fh);

	fflush(fh);
	fclose(fh);

	/* Build up comment, contains metadata of the version */
	rev_comment = NULL;
	if (vmd != NULL) {
		apnmd *md = apnmd_parse(NULL);

		if (md != NULL) {
			if (vmd->comment != NULL)
				apnmd_put(md, APNVM_COMMENT, vmd->comment);

			apnmd_put_int(md, APNVM_AUTOSTORE, vmd->auto_store);

			/* Make comment-string */
			rev_comment = apnmd_serialize(md);

			apnmd_destroy(md);
		}
	}

	/* Commit the changes */
	result = apncvs_commit(&vm->cvs, file,
	    ((rev_comment != NULL) ? rev_comment : APNVM_DEFCOMMENT));

	return (result == 0) ? APNVM_OK : APNVM_VMS;
}

apnvm_result
apnvm_remove(apnvm *vm, const char *user, const char *profile, int no)
{
	char	revno[12];
	int	result;
	char	file[PATH_MAX];

	APNVM_CHECKVM(vm, APNVM_ARG);
	APNVM_CHECKSTR(user, APNVM_ARG);
	APNVM_CHECKSTR(profile, APNVM_ARG);

	/* Convert to CVS-revision no */
	if (!no_to_revno(no, revno, sizeof(revno)))
		return (APNVM_OOM);

	result = snprintf(file, PATH_MAX, "%s/%s", user, profile);
	result = apncvs_remrev(&vm->cvs, file, revno);
	if (result != 0) {
		/* Failed to remove revision */
		return (APNVM_VMS);
	}

	return (APNVM_OK);
}

int
anoubis_ui_init(void)
{
	struct stat	sb;
	char		*homepath = NULL,
			*anoubispath = NULL,
			*versionpath = NULL;
	FILE		*fd = NULL;

	homepath = getenv("HOME");
	if (homepath == NULL)
		return -ENOENT;
	if (asprintf(&anoubispath, "%s/%s", homepath, ANOUBIS_UI_DIR) < 0)
		return (-errno);

	if (stat(anoubispath, &sb) >= 0) {
		free(anoubispath);
		return 1;
	}

	/**
	 * If config directory does not exist, create an initial one.
	 */
	if (errno == ENOENT) {
		if (mkdir(anoubispath, S_IRWXU) < 0) {
			free(anoubispath);
			return (-errno);
		}
		if (asprintf(&versionpath, "%s/version",
		    anoubispath) < 0) {
			free(anoubispath);
			return (-errno);
		}
		free(anoubispath);
		if ((fd = fopen(versionpath, "w+")) == NULL) {
			free(versionpath);
			return (-errno);
		}
		fprintf(fd, "%d", ANOUBIS_UI_VER);
		fclose(fd);
		free(versionpath);
		return (1);
	}
	free(anoubispath);
	return (-errno);
}

int
anoubis_ui_readversion(void)
{
	FILE	*fd;
	char	*homepath = NULL, *versionpath;
	int	res = -1;
	char	buf[8];
	const char *errstr;

	homepath = getenv("HOME");
	if (homepath == NULL)
		return -ENOENT;
	if (asprintf(&versionpath, "%s/%s", homepath, ANOUBIS_UI_DIR"/version")
	    < 0)
		return -errno;

	if ((fd = fopen(versionpath, "r")) == NULL) {
		free(versionpath);
		if (errno == ENOENT)
			return 0;
		else
			return -errno;
	}

	if (fgets(buf, sizeof(buf), fd) == NULL) {
		fclose(fd);
		/* If nothing could be read. We assume no version */
		return 0;
	}
	buf[strcspn(buf, "\n")] = '\0';
	res = strtonum(buf, 1, 9999, &errstr);
	if (errstr)
		res = -errno;
	fclose(fd);
	free(versionpath);
	return res;
}

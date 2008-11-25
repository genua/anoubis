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

#ifdef OPENBSD
#include <sys/syslimits.h>
#endif

#ifndef S_SPLINT_S
#include <sys/stat.h>
#endif
#include <sys/param.h>
#include <sys/wait.h>

#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apncvs.h"

#define APNCVS_CHECKPTR(p)	\
	if (p == NULL)		\
		return (-1);

#define APNCVS_CHECKSTR(s)			\
	if ((s == NULL) || (strlen(s) == 0))	\
		return (-1);

#define APNCVS_CHECKAPNCVS(p)		\
	APNCVS_CHECKPTR(p);		\
	APNCVS_CHECKSTR(p->cvsroot);	\
	APNCVS_CHECKSTR(p->workdir);	\
	APNCVS_CHECKSTR(p->module);

extern int apncvs_log_parse(FILE*, struct apncvs_log*);

/**
 * Creates the complete cvs-command, which is executed.
 *
 * @param cvs Information about the cvs-repository
 * @param cdworkdir Set to true, if you want to jump into the working directory
 *                  before cvscmd is executed
 * @param needstdout Set to true, if you need the output to stdout. Otherwise
 *                   the output to stdout is redirected to /dev/null. Please
 *                   note, that the output to stderr is always redirected to
 *                   /dev/null!
 * @param cvscmd The cvs command to be executed
 * @param ... Arguments for cvscmd
 * @return The command, which can be passed to a shell. On error, NULL is
 *         returned. Note, that to have to release the memory allocated for the
 *         result by yourself using free(3).
 */
static char *
apncvs_makecmd(const struct apncvs *cvs, int cdworkdir, int needstdout,
	const char *cvscmd, ...)
{
	va_list	ap;
	char	*buf, *cmd;
	int	result;

	if ((cvscmd == NULL) || (strlen(cvscmd) == 0))
		return (NULL);

	/* Prepare cvscmd */
	va_start(ap, cvscmd);
	result = vasprintf(&buf, cvscmd, ap);
	va_end(ap);

	if (result < 0)
		return (NULL);

	/* Make the command to be executed */
	if (cdworkdir)
		result = asprintf(&cmd, "(cd \"%s\" && %s) %s",
		    cvs->workdir, buf,
		    (needstdout ? "2>/dev/null" : ">/dev/null 2>&1"));
	else
		result = asprintf(&cmd, "%s %s", buf,
		    (needstdout ? "2>/dev/null" : ">/dev/null 2>&1"));

	free(buf);

	if (result > 0)
		return (cmd);
	else
		return (NULL);
}

static int
apncvs_fileexists(struct apncvs *cvs, const char *file)
{
	char		path[PATH_MAX];
	int		nwritten;
	struct stat	fstat;

	APNCVS_CHECKAPNCVS(cvs);
	APNCVS_CHECKSTR(file);

	nwritten = snprintf(path, sizeof(path), "%s/%s/%s",
	    cvs->workdir, cvs->module, file);
	if ((nwritten >= PATH_MAX)  << (nwritten < 0))
		return (-1);

	return stat(path, &fstat);
}

static void
apncvs_replace(char *str, char search, char replace)
{
	char *p = str;

	if (str == NULL || strlen(str) == 0)
		return;

	while (p != NULL) {
		p = strchr(p, search);

		if (p != NULL) {
			*p = replace;
			p++;
		}
	}
}

int
apncvs_init(struct apncvs *cvs)
{	char		*cmd;
	char		*dirc, *dname;
	struct stat	fstat;
	int		rc;

	APNCVS_CHECKAPNCVS(cvs);

	/* Make sure parent directory of cvs->cvsroot exists */
	if ((dirc = strdup(cvs->cvsroot)) == NULL)
		return (-1);
	dname = dirname(dirc);

	if (stat(dname, &fstat) != 0) {
		/* Create the parent directory */
		if (mkdir(dname, S_IRWXU | S_IRGRP | S_IXGRP) != 0) {
			free(dirc);
			return (-1);
		}
	}

	free(dirc);

	cmd = apncvs_makecmd(cvs, 0, 0, "cvs -d \"%s\" init", cvs->cvsroot);

	if (cmd == NULL)
		return (-1);

	rc = system(cmd);
	free(cmd);

	if (WIFEXITED(rc) && WEXITSTATUS(rc) == 0) {
		/* Check permission of repository-directories */
		char *admindir;

		if (chmod(cvs->cvsroot, S_IRWXU) == -1)
			return (-1);

		if (asprintf(&admindir, "%s/CVSROOT", cvs->cvsroot) >= 0) {
			int result = chmod(admindir, S_IRWXU);
			free(admindir);

			if (result == -1)
				return (-1);
		} else
			return (-1);

		return (0);
	} else
		return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

int
apncvs_checkout(struct apncvs *cvs)
{
	char	*cmd;
	int	rc;

	APNCVS_CHECKAPNCVS(cvs);
	cmd = apncvs_makecmd(cvs, 1, 0, "cvs -d \"%s\" checkout \"%s\"",
	    cvs->cvsroot, cvs->module);

	if (cmd == NULL)
		return (-1);

	rc = system(cmd);
	free(cmd);

	return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

int
apncvs_update(struct apncvs *cvs, const char *file, const char *rev)
{
	char	*cmd;
	int	rc;

	APNCVS_CHECKAPNCVS(cvs);
	APNCVS_CHECKSTR(file);

	if (rev != NULL)
		cmd = apncvs_makecmd(cvs, 1, 0,
		    "cvs -d \"%s\" update -r \"%s\" \"%s/%s\"",
		    cvs->cvsroot, rev, cvs->module, file);
	else
		cmd = apncvs_makecmd(cvs, 1, 0,
		    "cvs -d \"%s\" update -A \"%s/%s\"",
		    cvs->cvsroot, cvs->module, file);

	if (cmd == NULL)
		return (-1);

	rc = system(cmd);
	free(cmd);

	if (WIFEXITED(rc) && WEXITSTATUS(rc) == 0) {
		/*
		 * Choosing a wrong file or revision will also lead into
		 * success.
		 * Additionally check for existance of the file in the working
		 * directory to make sure, that the operation was successful.
		 */
		return apncvs_fileexists(cvs, file);
	}
	else
		return (-1);
}

int
apncvs_log(struct apncvs *cvs, const char *file, struct apncvs_log *log)
{
	char	*cmd;
	FILE	*f;
	int	rc, parse_rc;

	APNCVS_CHECKAPNCVS(cvs);
	APNCVS_CHECKSTR(file);
	APNCVS_CHECKPTR(log);

	cmd = apncvs_makecmd(cvs, 1, 1, "cvs -d \"%s\" log \"%s/%s\"",
	    cvs->cvsroot, cvs->module, file);

	if (cmd == NULL)
		return (-1);

	if ((f = popen(cmd, "r")) == NULL) {
		free(cmd);
		return (-1);
	}

	/* Parse output of cvs log command, written into "log" */
	parse_rc = apncvs_log_parse(f, log);

	rc = pclose(f);
	free(cmd);

	return (rc || parse_rc);

}

static void
apncvs_rev_destroy(struct apncvs_rev *rev)
{
	if (rev == NULL)
		return;

	free(rev->revno);
	free(rev->author);
	free(rev->state);
	free(rev->comment);

	free(rev);
}

static void
apncvs_revqueue_destroy(struct apncvsrev_queue *revq)
{
	struct apncvs_rev *rev, *next;

	if ((revq == NULL) || TAILQ_EMPTY(revq))
		return;

	for (rev = TAILQ_FIRST(revq); rev != TAILQ_END(revq); rev = next) {
		next = TAILQ_NEXT(rev, entry);
		TAILQ_REMOVE(revq, rev, entry);
		apncvs_rev_destroy(rev);
	}
}

void
apncvs_log_destroy(struct apncvs_log *log)
{
	if (log == NULL)
		return;

	free(log->rcs_file);
	free(log->working_file);
	free(log->head);
	free(log->branch);
	free(log->locks);
	free(log->access_list);
	free(log->symbolic_names);
	free(log->keyword_substitution);
	free(log->description);

	apncvs_revqueue_destroy(&log->rev_queue);
}

int
apncvs_commit(struct apncvs *cvs, const char *file, const char *comment)
{
	char	*cmd, *escaped_comment;
	int	rc;

	APNCVS_CHECKAPNCVS(cvs);
	APNCVS_CHECKSTR(file);

	/* The comment can be empty, do not check for empty string */
	if (comment == NULL)
		return (-1);

	/* Escape the comment; replace single quotes with double quotes */
	escaped_comment = strdup(comment);
	if (escaped_comment == NULL)
		return (-1);

	apncvs_replace(escaped_comment, '\'', '"');

	cmd = apncvs_makecmd(cvs, 1, 0,
	    "cvs -d \"%s\" commit -m '%s' \"%s/%s\"",
	    cvs->cvsroot, escaped_comment, cvs->module, file);

	free(escaped_comment);

	if (cmd == NULL)
		return (-1);

	rc = system(cmd);
	free(cmd);

	return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

int
apncvs_add(struct apncvs *cvs, const char *file)
{
	char	*cmd;
	int	rc;

	APNCVS_CHECKAPNCVS(cvs);
	APNCVS_CHECKSTR(file);

	cmd = apncvs_makecmd(cvs, 1, 0, "cvs -d \"%s\" add \"%s/%s\"",
	    cvs->cvsroot, cvs->module, file);

	if (cmd == NULL)
		return (-1);

	rc = system(cmd);
	free(cmd);

	return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

int
apncvs_remrev(struct apncvs *cvs, const char *file, const char *rev)
{
	char	*cmd;
	int	rc;

	APNCVS_CHECKAPNCVS(cvs);
	APNCVS_CHECKSTR(file);
	APNCVS_CHECKSTR(rev);

	/*
	 * Test weather the file exists
	 * CVS in obsd returns 0 even if the file does not exist, thus check
	 * for existence before
	 */
	rc = apncvs_fileexists(cvs, file);
	if (rc == 0) {
		cmd = apncvs_makecmd(cvs, 1, 0,
		    "cvs -d \"%s\" admin -o\"%s\" \"%s/%s\"",
		    cvs->cvsroot, rev, cvs->module, file);

		if (cmd == NULL)
			return (-1);

		rc = system(cmd);
		free(cmd);

		return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
	}
	else
		return (rc);
}

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

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <check.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <paths.h>
#include <unistd.h>

#include <anoubis_apnvm_cvs.h>

static int
cvs_init_tc_exec(const char* cmd, ...)
{
	va_list ap;
	char str[128], syscmd[128];
	int rc;

	va_start(ap, cmd);
	vsnprintf(str, sizeof(str), cmd, ap);
	va_end(ap);

	snprintf(syscmd, sizeof(syscmd), "(%s) >/dev/null 2>&1", str);

	rc = system(syscmd);
	return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

START_TEST(cvs_init_tc_create)
{
	char		cvsroot[32];
	char		cvsroot2[64];
	struct apncvs	cvs;
	char		*s;
	int		result;
	struct stat	fstat;

	strcpy(cvsroot, _PATH_TMP "tc_cvs_init_XXXXXX");
	s = mkdtemp(cvsroot);
	fail_if(s == NULL, "failed to create cvsroot");

	strcpy(cvsroot2, cvsroot);
	strcat(cvsroot2, "/CVSROOT");

	cvs.cvsroot = cvsroot;
	cvs.workdir = "bla"; /* Not used */
	cvs.module = "blubb"; /* Not used */
	cvs.pidcallback = NULL;

	result = apncvs_init(&cvs);
	fail_if(result != 0, "Init operation failed with %i", result);

	result = stat(cvsroot, &fstat);
	fail_if(result != 0, "No such directory: %s", cvsroot);
	fail_if(!S_ISDIR(fstat.st_mode), "No such directory: %s", cvsroot);

	result = stat(cvsroot2, &fstat);
	fail_if(result != 0, "No such directory: %s", cvsroot2);
	fail_if(!S_ISDIR(fstat.st_mode), "No such directory: %s", cvsroot2);

	cvs_init_tc_exec("rm -rf \"%s\"", cvsroot);
}
END_TEST

START_TEST(cvs_init_tc_no_parent)
{
	char		template[64], *tmpdir;
	char		cvsroot[64];
	char		cvsroot2[64];
	struct apncvs	cvs;
	int		result;
	struct stat	fstat;

	strcpy(template, _PATH_TMP "tc_cvs_init_XXXXXX");
	tmpdir = mkdtemp(template);
	fail_if(tmpdir == NULL, "failed to create tempdir");

	snprintf(cvsroot, sizeof(cvsroot), "%s/cvsroot_%i", tmpdir, getpid());
	snprintf(cvsroot2, sizeof(cvsroot2), "%s/CVSROOT", cvsroot);

	cvs.cvsroot = cvsroot;
	cvs.workdir = "bla"; /* Not used */
	cvs.module = "blubb"; /* Not used */
	cvs.pidcallback = NULL;

	result = apncvs_init(&cvs);
	fail_if(result != 0, "Init operation failed with %i", result);

	result = stat(cvsroot, &fstat);
	fail_if(result != 0, "No such directory: %s", cvsroot);
	fail_if(!S_ISDIR(fstat.st_mode), "No such directory: %s", cvsroot);

	result = stat(cvsroot2, &fstat);
	fail_if(result != 0, "No such directory: %s", cvsroot2);
	fail_if(!S_ISDIR(fstat.st_mode), "No such directory: %s", cvsroot2);

	cvs_init_tc_exec("rm -rf \"%s\"", cvsroot);
	cvs_init_tc_exec("rm -rf \"%s\"", tmpdir);
}
END_TEST

START_TEST(cvs_init_tc_already_existing)
{
	char		cvsroot[32];
	char		cvsroot2[64];
	struct apncvs	cvs;
	char		*s;
	int		result;
	struct stat	fstat;

	strcpy(cvsroot, _PATH_TMP "tc_cvs_init_XXXXXX");
	s = mkdtemp(cvsroot);
	fail_if(s == NULL, "failed to create cvsroot");

	strcpy(cvsroot2, cvsroot);
	strcat(cvsroot2, "/CVSROOT");

	cvs_init_tc_exec("cvs -d \"%s\" init >/dev/null 2>&1", cvsroot);

	cvs.cvsroot = cvsroot;
	cvs.workdir = "bla"; /* Not used */
	cvs.module = "blubb"; /* Not used */
	cvs.pidcallback = NULL;

	result = apncvs_init(&cvs);
	fail_if(result != 0, "Init operation failed with %i", result);

	result = stat(cvsroot, &fstat);
	fail_if(result != 0, "No such directory: %s", cvsroot);
	fail_if(!S_ISDIR(fstat.st_mode), "No such directory: %s", cvsroot);

	result = stat(cvsroot2, &fstat);
	fail_if(result != 0, "No such directory: %s", cvsroot2);
	fail_if(!S_ISDIR(fstat.st_mode), "No such directory: %s", cvsroot2);

	cvs_init_tc_exec("rm -rf \"%s\"", cvsroot);
}
END_TEST

START_TEST(cvs_init_permission_cvsroot)
{
	char		*cvsroot;
	struct apncvs	cvs;
	int		result;
	struct stat	fstat;
	mode_t		cmask;

	cvsroot = tempnam(NULL, "tccvs");

	/* Change file mode creation mask */
	cmask = umask(0);

	result = mkdir(cvsroot, S_IRWXU|S_IRWXG|S_IRWXO);
	fail_if(result != 0, "Failed to create %s: %s",
	    cvsroot, strerror(errno));

	cvs.cvsroot = cvsroot;
	cvs.workdir = "bla"; /* Not used */
	cvs.module = "blubb"; /* Not used */
	cvs.pidcallback = NULL;

	result = apncvs_init(&cvs);
	fail_if(result != 0, "Init operation failed with %i", result);

	result = stat(cvsroot, &fstat);
	fail_if(result != 0, "No such directory: %s", cvsroot);

	/* Owner has read, write and exec permission */
	result = fstat.st_mode & S_IRWXU;
	fail_if(result != S_IRWXU, "Wrong permission for owner (%i)",
	    fstat.st_mode);

	/* Nobody else can access the directory */
	result = fstat.st_mode & (S_IRWXG | S_IRWXO);
	fail_if(result != 0, "Wrong permission for anybody else (%i)",
	    fstat.st_mode);

	cvs_init_tc_exec("rm -rf \"%s\"", cvsroot);

	/* Switch back to original umask */
	umask(cmask);

	free(cvsroot);
}
END_TEST

START_TEST(cvs_init_permission_cvsroot_cvsroot)
{
	char		cvsroot[32];
	char		cvsroot2[64];
	char		*s;
	struct apncvs	cvs;
	int		result;
	struct stat	fstat;

	strcpy(cvsroot, _PATH_TMP "tc_cvs_init_XXXXXX");
	s = mkdtemp(cvsroot);
	fail_if(s == NULL, "failed to create cvsroot");

	strcpy(cvsroot2, cvsroot);
	strcat(cvsroot2, "/CVSROOT");

	cvs.cvsroot = cvsroot;
	cvs.workdir = "bla"; /* Not used */
	cvs.module = "blubb"; /* Not used */
	cvs.pidcallback = NULL;

	result = apncvs_init(&cvs);
	fail_if(result != 0, "Init operation failed with %i", result);

	result = stat(cvsroot2, &fstat);
	fail_if(result != 0, "No such directory: %s", cvsroot);

	/* Owner has read, write and exec permission */
	result = fstat.st_mode & S_IRWXU;
	fail_if(result != S_IRWXU, "Wrong permission for owner (%i)",
	    fstat.st_mode);

	/* Nobody else can access the directory */
	result = fstat.st_mode & (S_IRWXG | S_IRWXO);
	fail_if(result != 0, "Wrong permission for anybody else (%i)",
	    fstat.st_mode);

	cvs_init_tc_exec("rm -rf \"%s\"", cvsroot);
}
END_TEST

TCase *
apnvm_tc_cvs_init(void)
{
	TCase *tc = tcase_create("CVS_INIT");

	tcase_add_test(tc, cvs_init_tc_create);
	tcase_add_test(tc, cvs_init_tc_no_parent);
	tcase_add_test(tc, cvs_init_tc_already_existing);
	tcase_add_test(tc, cvs_init_permission_cvsroot);
	tcase_add_test(tc, cvs_init_permission_cvsroot_cvsroot);

	return (tc);
}

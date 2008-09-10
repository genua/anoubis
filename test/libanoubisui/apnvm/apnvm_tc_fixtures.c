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
#include <sys/wait.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apnvm_tc_fixtures.h"

static int
vm_tc_exec(const char* cmd, ...)
{
	va_list ap;
	char str[1024], syscmd[2048];
	int rc;

	va_start(ap, cmd);
	vsnprintf(str, sizeof(str), cmd, ap);
	va_end(ap);

	snprintf(syscmd, sizeof(syscmd), "(%s) >/dev/null 2>&1", str);

	rc = system(syscmd);
	return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

static void
vm_tc_write_alf(FILE *f, const char *profile)
{
	fprintf(f, "#@#@#@#@#@#@#@#@#@#@%s\n",
	    ((profile != NULL) ? profile : ""));

	fprintf(f, "alf {\n");
	fprintf(f, "	any {\n");
	fprintf(f, "		context new any\n");
	fprintf(f, "		default allow\n");
	fprintf(f, "	}\n");
	fprintf(f, "}\n");

	fflush(f);
}

static void
vm_tc_write_sfs(FILE *f, const char *profile)
{
	fprintf(f, "#@#@#@#@#@#@#@#@#@#@%s\n",
	    ((profile != NULL) ? profile : ""));

	fprintf(f, "sfs {\n");
	fprintf(f, "	/bin/ping \
a193a2edb06ff39630fed8195c0b043651867b91fccc8db67e4222367736ba73\n");
	fprintf(f, "}\n");

	fflush(f);
}

static void
vm_tc_write_alfsfs(FILE *f, const char *profile)
{
	fprintf(f, "#@#@#@#@#@#@#@#@#@#@%s\n",
	    ((profile != NULL) ? profile : ""));

	fprintf(f, "alf {\n");
	fprintf(f, "	any {\n");
	fprintf(f, "		context new any\n");
	fprintf(f, "		default allow\n");
	fprintf(f, "	}\n");
	fprintf(f, "}\n");
	fprintf(f, "sfs {\n");
	fprintf(f, "	/bin/ping \
a193a2edb06ff39630fed8195c0b043651867b91fccc8db67e4222367736ba73\n");
	fprintf(f, "}\n");

	fflush(f);
}

static void
vm_tc_makecomment(char *out, size_t sout, const char *comment, int auto_store)
{
	snprintf(out, sout, "<apnvm-metadata>\n\
comment := %s\n\
autostore := %i\n\
</apnvm-metadata>\n", comment, auto_store);
}

void
apnvm_setup(void)
{
	char workdir[32];
	char moduledir[64];
	char user2_file[128];
	char user3_file[128];
	char comment[256];
	char *s;
	FILE *f;

	strcpy(apnvm_cvsroot, "/tmp/tc_vm_XXXXXX");
	strcpy(workdir, "/tmp/tc_vm_XXXXXX");
	strcpy(apnvm_user, "user1");
	s = mkdtemp(apnvm_cvsroot);
	s = mkdtemp(workdir);

	snprintf(moduledir, sizeof(moduledir), "%s/user1", apnvm_cvsroot);
	snprintf(user2_file, sizeof(user2_file), "%s/user1/user2", workdir);
	snprintf(user3_file, sizeof(user3_file), "%s/user1/user3", workdir);

	/* Initialize repository and create module inside repository */
	vm_tc_exec("cvs -d \"%s\" init", apnvm_cvsroot);
	mkdir(moduledir, S_IRWXU);

	/* Checkout module to create some files */
	vm_tc_exec("cd \"%s\" && cvs -d \"%s\" checkout user1",
	    workdir, apnvm_cvsroot);

	/* Create some revision for user2 */

	f = fopen(user2_file, "w");
	vm_tc_write_alf(f, "xxx");
	vm_tc_write_sfs(f, "yyy");
	fclose(f);

	vm_tc_exec("cd \"%s\" && cvs -d \"%s\" add user1/user2",
		workdir, apnvm_cvsroot);
	vm_tc_makecomment(comment, sizeof(comment), "1st revision", 0);
	vm_tc_exec(
	    "cd \"%s\" && cvs -d \"%s\" commit -m \"%s\" user1/user2",
	    workdir, apnvm_cvsroot, comment);

	f = fopen(user2_file, "w");
	vm_tc_write_alf(f, "xxx");
	vm_tc_write_alfsfs(f, "yyy");
	fclose(f);
	vm_tc_makecomment(comment, sizeof(comment), "2nd revision", 1);
	vm_tc_exec(
	    "cd \"%s\" && cvs -d \"%s\" commit -m \"%s\" user1/user2",
	    workdir, apnvm_cvsroot, comment);

	f = fopen(user2_file, "w");
	vm_tc_write_alf(f, "xxx");
	vm_tc_write_alfsfs(f, "yyy");
	vm_tc_write_sfs(f, "zzz");
	fclose(f);
	vm_tc_makecomment(comment, sizeof(comment), "3rd revision", 2);
	vm_tc_exec(
	    "cd \"%s\" && cvs -d \"%s\" commit -m \"%s\" user1/user2",
	    workdir, apnvm_cvsroot, comment);

	/* Create some revisions for user3 */

	f = fopen(user3_file, "w");
	vm_tc_write_alf(f, NULL);
	fclose(f);

	vm_tc_exec("cd \"%s\" && cvs -d \"%s\" add user1/user3",
		workdir, apnvm_cvsroot);
	vm_tc_makecomment(comment, sizeof(comment), "1st revision", 0);
	vm_tc_exec(
	    "cd \"%s\" && cvs -d \"%s\" commit -m \"%s\" user1/user3",
	    workdir, apnvm_cvsroot, comment);

	f = fopen(user3_file, "w");
	vm_tc_write_sfs(f, NULL);
	fclose(f);
	vm_tc_makecomment(comment, sizeof(comment), "2nd revision", 1);
	vm_tc_exec(
	    "cd \"%s\" && cvs -d \"%s\" commit -m \"%s\" user1/user3",
	    workdir, apnvm_cvsroot, comment);

	f = fopen(user3_file, "w");
	vm_tc_write_alfsfs(f, NULL);
	fclose(f);
	vm_tc_makecomment(comment, sizeof(comment), "3rd revision", 2);
	vm_tc_exec(
	    "cd \"%s\" && cvs -d \"%s\" commit -m \"%s\" user1/user3",
	    workdir, apnvm_cvsroot, comment);

	vm_tc_exec("rm -rf \"%s\"", workdir);
}

void
apnvm_teardown(void)
{
	vm_tc_exec("rm -rf \"%s\"", apnvm_cvsroot);
}

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

#include <check.h>
#include <stdlib.h>
#include <string.h>

#include <apnvm.h>

char cvsroot[32];
char user[32];

static int
vm_tc_exec(const char* cmd, ...)
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

void
vm_setup(void)
{
	char workdir[32];
	char *s;

	strcpy(cvsroot, "/tmp/tc_vm_XXXXXX");
	strcpy(workdir, "/tmp/tc_vm_XXXXXX");
	strcpy(user, "user1");
	s = mkdtemp(cvsroot);
	s = mkdtemp(workdir);

	vm_tc_exec("cvs -d \"%s\" init", cvsroot);
	vm_tc_exec("mkdir \"%s/user1\"", cvsroot);
	vm_tc_exec("chmod 700 \"%s/user1\"", cvsroot);
	vm_tc_exec("cd \"%s\" && cvs -d \"%s\" checkout user1",
	    workdir, cvsroot);
	vm_tc_exec("echo \"Zeile 1\" > \"%s/user1/user2\"", workdir);
	vm_tc_exec("cd \"%s\" && cvs -d \"%s\" add user1/user2",
		workdir, cvsroot);
	vm_tc_exec(
	"cd \"%s\" && cvs -d \"%s\" commit -m \"1st revision\" user1/user2",
	workdir, cvsroot);
	vm_tc_exec("echo \"Zeile 2\" >> \"%s/user1/user2\"", workdir);
	vm_tc_exec(
	"cd \"%s\" && cvs -d \"%s\" commit -m \"2nd revision\" user1/user2",
	workdir, cvsroot);
	vm_tc_exec("echo \"Zeile 3\" >> \"%s/user1/user2\"", workdir);
	vm_tc_exec(
	"cd \"%s\" && cvs -d \"%s\" commit -m \"3rd revision\" user1/user2",
	workdir, cvsroot);

	vm_tc_exec("rm -rf \"%s\"", workdir);
}

void
vm_teardown(void)
{
	vm_tc_exec("rm -rf \"%s\"", cvsroot);
}

START_TEST(vm_tc_prepare_havemodule)
{
	apnvm		*vm;
	apnvm_result	vmrc;

	vm = apnvm_init(cvsroot, user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_prepare_nomodule)
{
	apnvm		*vm;
	char		path[256];
	struct stat	fstat;
	apnvm_result	vmrc;
	int		rc;

	vm = apnvm_init(cvsroot, "anotheruser");
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	/* apnvm_init should create the module for the user */
	strcpy(path, cvsroot);
	strcat(path, "/anotheruser");

	rc = stat(path, &fstat);
	fail_if(rc != 0, "No module created for new user");

	fail_if(!S_ISDIR(fstat.st_mode), "User-module is not a directory");
	fail_if((fstat.st_mode & S_IRUSR) == 0, "S_IRUSR flag not set");
	fail_if((fstat.st_mode & S_IWUSR) == 0, "S_IWUSR flag not set");
	fail_if((fstat.st_mode & S_IXUSR) == 0, "S_IXUSR flag not set");
	fail_if((fstat.st_mode & S_IRGRP) > 0, "S_IRGRP flag is set");
	fail_if((fstat.st_mode & S_IWGRP) > 0, "S_IWGRP flag is set");
	fail_if((fstat.st_mode & S_IXGRP) > 0, "S_IXGRP flag is set");
	fail_if((fstat.st_mode & S_IROTH) > 0, "S_IROTH flag is set");
	fail_if((fstat.st_mode & S_IWOTH) > 0, "S_IWOTH flag is set");
	fail_if((fstat.st_mode & S_IXOTH) > 0, "S_IXOTH flag is set");

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_count)
{
	apnvm		*vm;
	apnvm_result	vmrc;
	int		count = 0;

	vm = apnvm_init(cvsroot, user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_count(vm, "user2", &count);
	fail_if(vmrc != APNVM_OK, "Count operation failed");
	fail_if(count != 3, "Unexpected number of versions: %i", count);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_count_unknown_user)
{
	apnvm		*vm;
	apnvm_result	vmrc;
	int		count = 0;

	vm = apnvm_init(cvsroot, user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_count(vm, "blablub", &count);
	fail_if(vmrc != APNVM_OK, "Count operation failed");
	fail_if(count != 0, "Unexpected number of versions: %i", count);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_count_nullcount)
{
	apnvm		*vm;
	apnvm_result	vmrc;

	vm = apnvm_init(cvsroot, user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_count(vm, "user2", NULL);
	fail_if(vmrc != APNVM_ARG, "Count operation failed");

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_list)
{
	apnvm				*vm;
	apnvm_result			vmrc;
	struct apnvm_version_head	version_head;
	struct apnvm_version		*version;
	int				exp_num;

	vm = apnvm_init(cvsroot, user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	TAILQ_INIT(&version_head);
	vmrc = apnvm_list(vm, "user2", &version_head);
	fail_if(vmrc != APNVM_OK, "List operation failed");

	exp_num = 3;
	TAILQ_FOREACH(version, &version_head, entries) {
		fail_if(version->no != exp_num,
		    "Unexpected version-#: %i, is %i", version->no, exp_num);

		exp_num--;
	}

	while (version_head.tqh_first != NULL) {
		version = version_head.tqh_first;

		TAILQ_REMOVE(&version_head, version_head.tqh_first, entries);
		free(version);
	}

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_list_no_user)
{
	apnvm				*vm;
	apnvm_result			vmrc;
	struct apnvm_version_head	version_head;
	struct apnvm_version		*version;

	vm = apnvm_init(cvsroot, user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	TAILQ_INIT(&version_head);
	vmrc = apnvm_list(vm, "anotheruser", &version_head);
	fail_if(vmrc != APNVM_OK, "List operation failed");

	TAILQ_FOREACH(version, &version_head, entries) {
		fail("No versions expected");
	}

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_list_no_head)
{
	apnvm				*vm;
	apnvm_result			vmrc;

	vm = apnvm_init(cvsroot, user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_list(vm, "user2", NULL);
	fail_if(vmrc != APNVM_ARG, "List operation failed");

	apnvm_destroy(vm);
}
END_TEST

TCase *
apnvm_tc_vm(void)
{
	TCase *tc = tcase_create("VM");

	tcase_set_timeout(tc, 10);
	tcase_add_checked_fixture(tc, vm_setup, vm_teardown);

	tcase_add_test(tc, vm_tc_prepare_havemodule);
	tcase_add_test(tc, vm_tc_prepare_nomodule);
	tcase_add_test(tc, vm_tc_count);
	tcase_add_test(tc, vm_tc_count_unknown_user);
	tcase_add_test(tc, vm_tc_count_nullcount);
	tcase_add_test(tc, vm_tc_list);
	tcase_add_test(tc, vm_tc_list_no_user);
	tcase_add_test(tc, vm_tc_list_no_head);

	return (tc);
}

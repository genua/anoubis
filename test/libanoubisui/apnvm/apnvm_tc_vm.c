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

#ifndef NEEDBSDCOMPAT
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <check.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <apnvm.h>

#include "apnvm_tc_fixtures.h"

#define TEST_TIMEOUT	20

START_TEST(vm_tc_prepare_no_cvsroot)
{
	char		*cvsroot;
	char		user[16];
	char		*s;
	char		*tmpdir;
	apnvm		*vm;
	apnvm_result	vmrc;

	tmpdir = getenv("TMPDIR");
	if (tmpdir == NULL)
		tmpdir = "/tmp";
	if (asprintf(&cvsroot, "%s/tc_vm_XXXXXX", tmpdir) == -1)
		fail_if(1, "asprintf");

	strcpy(user, "user1");
	s = mkdtemp(cvsroot);

	vm = apnvm_init(cvsroot, user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_prepare_havemodule)
{
	apnvm		*vm;
	apnvm_result	vmrc;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
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

	vm = apnvm_init(apnvm_cvsroot, "anotheruser");
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	/* apnvm_init should create the module for the user */
	strcpy(path, apnvm_cvsroot);
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

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_count(vm, "user2", "xxx", &count);
	fail_if(vmrc != APNVM_OK, "Count operation failed");
	fail_if(count != 1, "Unexpected number of versions: %i", count);

	vmrc = apnvm_count(vm, "user2", "yyy", &count);
	fail_if(vmrc != APNVM_OK, "Count operation failed");
	fail_if(count != 3, "Unexpected number of versions: %i", count);

	vmrc = apnvm_count(vm, "user2", "zzz", &count);
	fail_if(vmrc != APNVM_OK, "Count operation failed");
	fail_if(count != 1, "Unexpected number of versions: %i", count);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_count_unknown_user)
{
	apnvm		*vm;
	apnvm_result	vmrc;
	int		count = 0;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_count(vm, "blablub", "active", &count);
	fail_if(vmrc != APNVM_OK, "Count operation failed");
	fail_if(count != 0, "Unexpected number of versions: %i", count);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_count_nullcount)
{
	apnvm		*vm;
	apnvm_result	vmrc;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_count(vm, "user2", "active", NULL);
	fail_if(vmrc != APNVM_ARG, "Count operation failed");

	apnvm_destroy(vm);
}
END_TEST

struct profile_version	{
	const char	*name;
	const int	 revs[10];
};

static struct profile_version	vm_tc_list_versions[] = {
	{ "xxx", { 1, -1, } },
	{ "yyy", { 3, 2, 1, -1, } },
	{ "zzz", { 3, -1, } },
	{ "active", { -1, } },
	{ NULL, { -1 } },
};

START_TEST(vm_tc_list)
{
	apnvm				*vm;
	apnvm_result			vmrc;
	struct apnvm_version_head	version_head;
	struct apnvm_version		*version;
	int				exp_num;
	int				i, j, cnt;


	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	for (i=0; vm_tc_list_versions[i].name; ++i) {
		struct profile_version	*vdesc = &vm_tc_list_versions[i];
		TAILQ_INIT(&version_head);
		vmrc = apnvm_list(vm, "user2", vdesc->name, &version_head);
		fail_if(vmrc != APNVM_OK, "List operation failed");

		j = 0;
		cnt = 0;
		while(vdesc->revs[cnt] >= 0)
			cnt++;
		TAILQ_FOREACH(version, &version_head, entries) {
			int cmp_cmt, cmp_as;

			exp_num = vdesc->revs[j++];
			fail_if(exp_num < 0, "Unexpected version %i",
			    version->no);
			fail_if(version->no != cnt+1-j, "Unexpected version-#:"
			    " is %i, should be %i", version->no, cnt+1-j);

			switch (exp_num) {
			case 1:
				cmp_cmt = strcmp(version->comment,
				    "1st revision");
				cmp_as = (version->auto_store == 0);
				break;
			case 2:
				cmp_cmt = strcmp(version->comment,
				    "2nd revision");
				cmp_as = (version->auto_store == 1);
				break;
			case 3:
				cmp_cmt = strcmp(version->comment,
				    "3rd revision");
				cmp_as = (version->auto_store == 2);
				break;
			default:
				cmp_cmt = 4711;
				cmp_as = 4711;
				break;
			}
			fail_if(cmp_cmt != 0, "Unexpected comment for v%i: %s",
			    version->no, version->comment);
			fail_if(!cmp_as,
			    "Unexpected auto_store-value for v%i: %i",
			    version->no, version->auto_store);
		}
		apnvm_version_head_free(&version_head);
		fail_if(vdesc->revs[j] != -1,
		    "Additional version(s) for profile %s", vdesc->name);
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

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	TAILQ_INIT(&version_head);
	vmrc = apnvm_list(vm, "anotheruser", "active", &version_head);
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

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_list(vm, "user2", "yyy", NULL);
	fail_if(vmrc != APNVM_ARG, "List operation failed");

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_getuser)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apnvm_user_head	user_head;
	int			have_u2, have_u3;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	LIST_INIT(&user_head);
	vmrc = apnvm_getuser(vm, &user_head);
	fail_if(vmrc != APNVM_OK,
	    "Unexpected result from getuser-operation: %i", vmrc);

	have_u2 = 0;
	have_u3 = 0;

	while (!LIST_EMPTY(&user_head)) {
		struct apnvm_user *user = LIST_FIRST(&user_head);
		LIST_REMOVE(user, entry);

		if (strcmp(user->username, "user2") == 0) {
			fail_if(have_u2, "Already have user2");
			have_u2 = 1;
		}
		else if (strcmp(user->username, "user3") == 0) {
			fail_if(have_u3, "Already have user3");
			have_u3 = 1;
		}
		else
			fail("Unexpected user: %s", user->username);

		free(user->username);
		free(user);
	}

	fail_if(!have_u2, "Missing user2");
	fail_if(!have_u3, "Missing user3");

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_getuser_nolist)
{
	apnvm			*vm;
	apnvm_result		vmrc;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_getuser(vm, NULL);
	fail_if(vmrc != APNVM_ARG,
	    "Unexpected result from getuser-operation: %i", vmrc);

	apnvm_destroy(vm);
}
END_TEST

TCase *
apnvm_tc_vm(void)
{
	TCase *tc = tcase_create("VM");

	tcase_set_timeout(tc, TEST_TIMEOUT);
	tcase_add_checked_fixture(tc, apnvm_setup, apnvm_teardown);

	tcase_add_test(tc, vm_tc_prepare_havemodule);
	tcase_add_test(tc, vm_tc_prepare_nomodule);
	tcase_add_test(tc, vm_tc_count);
	tcase_add_test(tc, vm_tc_count_unknown_user);
	tcase_add_test(tc, vm_tc_count_nullcount);
	tcase_add_test(tc, vm_tc_list);
	tcase_add_test(tc, vm_tc_list_no_user);
	tcase_add_test(tc, vm_tc_list_no_head);
	tcase_add_test(tc, vm_tc_getuser);
	tcase_add_test(tc, vm_tc_getuser_nolist);

	return (tc);
}

TCase *
apnvm_tc_vm_no_fixture(void)
{
	TCase *tc = tcase_create("VM_NO_FIXTURE");

	tcase_set_timeout(tc, TEST_TIMEOUT);

	/* Testcase without fixtures */
	tcase_add_test(tc, vm_tc_prepare_no_cvsroot);

	return (tc);
}

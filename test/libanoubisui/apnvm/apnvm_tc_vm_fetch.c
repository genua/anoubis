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

#include <check.h>

#include <apnvm.h>

#include "apnvm_tc_fixtures.h"

START_TEST(vm_tc_fetch)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs = NULL;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_fetch(vm, "user2", 1, "xxx", &rs);
	fail_if(vmrc != APNVM_OK, "Fetch operation failed");
	fail_if(rs == NULL, "Fetched a NULL-ruleset!");
	fail_if(TAILQ_FIRST(&rs->err_queue) != NULL, "Parser errors");
	fail_if(TAILQ_FIRST(&rs->alf_queue) == NULL, "Missing ALF-rule");
	fail_if(TAILQ_FIRST(&rs->sfs_queue) != NULL, "Unexpected SFS-rule");

	apn_free_ruleset(rs);
	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_fetch_no_profile)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs = NULL;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_fetch(vm, "user3", 2, NULL, &rs);
	fail_if(vmrc != APNVM_OK, "Fetch operation failed");
	fail_if(rs == NULL, "Fetched a NULL-ruleset!");
	fail_if(TAILQ_FIRST(&rs->err_queue) != NULL, "Parser errors");
	fail_if(TAILQ_FIRST(&rs->alf_queue) != NULL, "Missing ALF-rule");
	fail_if(TAILQ_FIRST(&rs->sfs_queue) == NULL, "Unexpected SFS-rule");

	apn_free_ruleset(rs);
	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_fetch_wrong_user)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs = NULL;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_fetch(vm, "babelfish", 1, "xxx", &rs);
	fail_if(vmrc != APNVM_OK, "Fetch operation failed");
	fail_if(rs != NULL, "Fetched a NULL-ruleset!");

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_fetch_no_user)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs = NULL;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_fetch(vm, NULL, 1, "xxx", &rs);
	fail_if(vmrc != APNVM_ARG,
	    "Unexpected result from fetch-operation: %i", vmrc);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_fetch_wrong_no)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs = NULL;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_fetch(vm, "user2", 4711, "xxx", &rs);
	fail_if(vmrc != APNVM_VMS,
	    "Enexpected result from fetch operation: %i", vmrc);
	fail_if(rs != NULL, "Fetched a ruleset!");

	apn_free_ruleset(rs);
	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_fetch_wrong_profile)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs = NULL;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_fetch(vm, "user2", 1, "abc", &rs);
	fail_if(vmrc != APNVM_OK, "Fetch operation failed");
	fail_if(rs != NULL, "Fetched a ruleset!");

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_fetch_no_profile_but_with_profiles)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs = NULL;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_fetch(vm, "user2", 1, NULL, &rs);
	fail_if(vmrc != APNVM_OK, "Fetch operation failed");
	fail_if(rs != NULL, "Fetched a ruleset!");

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_fetch_profile_but_without_profiles)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs = NULL;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_fetch(vm, "user3", 1, "xxx", &rs);
	fail_if(vmrc != APNVM_OK, "Fetch operation failed");
	fail_if(rs != NULL, "Fetched a ruleset!");

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_fetch_no_ruleset)
{
	apnvm		*vm;
	apnvm_result	vmrc;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_fetch(vm, "user2", 1, "xxx", NULL);
	fail_if(vmrc != APNVM_ARG, "Fetch operation failed");

	apnvm_destroy(vm);
}
END_TEST

TCase *
apnvm_tc_vm_fetch(void)
{
	TCase *tc = tcase_create("VM_FETCH");

	tcase_set_timeout(tc, 10);
	tcase_add_checked_fixture(tc, apnvm_setup, apnvm_teardown);

	tcase_add_test(tc, vm_tc_fetch);
	tcase_add_test(tc, vm_tc_fetch_no_profile);
	tcase_add_test(tc, vm_tc_fetch_wrong_user);
	tcase_add_test(tc, vm_tc_fetch_no_user);
	tcase_add_test(tc, vm_tc_fetch_wrong_no);
	tcase_add_test(tc, vm_tc_fetch_wrong_profile);
	tcase_add_test(tc, vm_tc_fetch_no_profile_but_with_profiles);
	tcase_add_test(tc, vm_tc_fetch_profile_but_without_profiles);
	tcase_add_test(tc, vm_tc_fetch_no_ruleset);

	return (tc);
}

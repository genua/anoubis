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

#define TEST_TIMEOUT	20

START_TEST(vm_tc_remove)
{
	apnvm		*vm;
	apnvm_result	vmrc;
	int		count = 0;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user, NULL);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_remove(vm, "user2", "yyy", 1);
	fail_if(vmrc != APNVM_OK, "Fetch operation failed");

	vmrc =  apnvm_count(vm, "user2", "yyy", &count);
	fail_if(vmrc != APNVM_OK, "Count operation failed");
	fail_if(count != 2, "Unexpected number of versions: %i", count);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_remove_no_user)
{
	apnvm		*vm;
	apnvm_result	vmrc;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user, NULL);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_remove(vm, NULL, "active", 1);
	fail_if(vmrc != APNVM_ARG,
	    "Unexpected result from remove-operation: %i", vmrc);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_remove_wrong_user)
{
	apnvm		*vm;
	apnvm_result	vmrc;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user, NULL);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_remove(vm, "abc", "active", 1);
	fail_if(vmrc != APNVM_VMS,
	    "Unexpected result from remove-operation: %i", vmrc);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_remove_wrong_no)
{
	apnvm		*vm;
	apnvm_result	vmrc;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user, NULL);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_remove(vm, "user2", "active", 4711);
	fail_if(vmrc != APNVM_VMS, "Fetch operation failed");

	apnvm_destroy(vm);
}
END_TEST

TCase *
apnvm_tc_vm_remove(void)
{
	TCase *tc = tcase_create("VM_REMOVE");

	tcase_set_timeout(tc, TEST_TIMEOUT);
	tcase_add_checked_fixture(tc, apnvm_setup, apnvm_teardown);

	tcase_add_test(tc, vm_tc_remove);
	tcase_add_test(tc, vm_tc_remove_no_user);
	tcase_add_test(tc, vm_tc_remove_wrong_user);
	tcase_add_test(tc, vm_tc_remove_wrong_no);

	return (tc);
}

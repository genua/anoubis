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
#include <string.h>

#include <apnvm.h>

#include "apnvm_tc_fixtures.h"

#define TEST_TIMEOUT	20

#define fail_if_alfempty(queue, ...) \
	fail_if(TAILQ_EMPTY(queue), ## __VA_ARGS__, NULL)

#define fail_if_alffull(queue, ...) \
	fail_if(!TAILQ_EMPTY(queue), ## __VA_ARGS__, NULL)

#define fail_if_sfsempty(queue, ...) \
	fail_if(TAILQ_EMPTY(queue)	\
	    || TAILQ_EMPTY(&(TAILQ_FIRST(queue)->rule.chain)), \
	    ## __VA_ARGS__, NULL)

#define fail_if_sfsfull(queue, ...) \
	fail_if(!TAILQ_EMPTY(queue) \
	    && !TAILQ_EMPTY(&(TAILQ_FIRST(queue)->rule.chain)), \
	    ## __VA_ARGS__, NULL)

static struct apn_ruleset *
vm_tc_create_ruleset(int type)
{
	struct apn_ruleset	*rs;
	struct iovec		iov;
	int			result;

	if (type == 0) {
		iov.iov_base = "alf {\n\
	any {\n\
		default allow\n\
	}\n\
}\n";
	}
	else if (type == 1) {
		iov.iov_base = "sfs {\n\
	/bin/ping \
a193a2edb06ff39630fed8195c0b043651867b91fccc8db67e4222367736ba73\n\
}\n";
	}
	else {
		iov.iov_base = "alf {\n\
	any {\n\
		default allow\n\
	}\n\
}\n\
sfs {\n\
	/bin/ping \
a193a2edb06ff39630fed8195c0b043651867b91fccc8db67e4222367736ba73\n\
}\n";
	}
	iov.iov_len = strlen(iov.iov_base);

	result = apn_parse_iovec("<iov>", &iov, 1, &rs, 0);
	return (result == 0) ? rs : NULL;
}

START_TEST(vm_tc_insert_with_profile_overwrite_first)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	rs = vm_tc_create_ruleset(2); /* alf-sfs */
	vmrc = apnvm_insert(vm, "user2", "xxx", rs, NULL);
	fail_if(vmrc != APNVM_OK, "Failed to insert ruleset");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "xxx", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alfempty(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsempty(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "yyy", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alfempty(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsempty(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "zzz", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alffull(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsempty(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_insert_with_profile_overwrite_last)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	rs = vm_tc_create_ruleset(0); /* alf */
	vmrc = apnvm_insert(vm, "user2", "zzz", rs, NULL);
	fail_if(vmrc != APNVM_OK, "Failed to insert ruleset");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "xxx", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alfempty(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsfull(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "yyy", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alfempty(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsempty(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);
	rs = NULL;

	vmrc = apnvm_fetch(vm, "user2", 4, "zzz", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alfempty(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsfull(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_insert_with_profile_overwrite_middle)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	rs = vm_tc_create_ruleset(1); /* sfs */
	vmrc = apnvm_insert(vm, "user2", "yyy", rs, NULL);
	fail_if(vmrc != APNVM_OK, "Failed to insert ruleset");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "xxx", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alfempty(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsfull(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "yyy", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alffull(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsempty(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "zzz", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alffull(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsempty(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_insert_with_profile_new_profile)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	rs = vm_tc_create_ruleset(2); /* alf-sfs */
	vmrc = apnvm_insert(vm, "user2", "abc", rs, NULL);
	fail_if(vmrc != APNVM_OK, "Failed to insert ruleset");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "xxx", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alfempty(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsfull(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "yyy", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alfempty(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsempty(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "zzz", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alffull(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsempty(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user2", 4, "abc", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alfempty(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsempty(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_insert_without_profile_overwrite)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	rs = vm_tc_create_ruleset(0); /* alf */
	vmrc = apnvm_insert(vm, "user3", NULL, rs, NULL);
	fail_if(vmrc != APNVM_OK, "Failed to insert ruleset");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user3", 4, NULL, &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alfempty(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsfull(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_insert_with_profile_new_user)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	rs = vm_tc_create_ruleset(1); /* sfs */
	vmrc = apnvm_insert(vm, "user4", "abc", rs, NULL);
	fail_if(vmrc != APNVM_OK, "Failed to insert ruleset");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user4", 1, "abc", &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alffull(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsempty(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_insert_without_profile_new_user)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	rs = vm_tc_create_ruleset(1); /* sfs */
	vmrc = apnvm_insert(vm, "user4", NULL, rs, NULL);
	fail_if(vmrc != APNVM_OK, "Failed to insert ruleset");
	apn_free_ruleset(rs);

	vmrc = apnvm_fetch(vm, "user4", 1, NULL, &rs);
	fail_if(vmrc != APNVM_OK, "Failed to fetch a ruleset");
	fail_if(rs == NULL, "A NULL-ruleset was fetched!");
	fail_if_alffull(&rs->alf_queue, "Wrong size of ALF-queue");
	fail_if_sfsempty(&rs->sfs_queue, "Wrong size of SFS-queue");
	apn_free_ruleset(rs);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_insert_no_user)
{
	apnvm			*vm;
	apnvm_result		vmrc;
	struct apn_ruleset	*rs;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	rs = vm_tc_create_ruleset(0); /* alf */
	vmrc = apnvm_insert(vm, NULL, "xxx", rs, NULL);
	fail_if(vmrc != APNVM_ARG,
	    "Unexpected result from insert-operation: %i", vmrc);
	apn_free_ruleset(rs);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_insert_no_ruleset)
{
	apnvm			*vm;
	apnvm_result		vmrc;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	vmrc = apnvm_insert(vm, "user2", "xxx", NULL, NULL);
	fail_if(vmrc != APNVM_ARG,
	    "Unexpected result from insert-operation: %i", vmrc);

	apnvm_destroy(vm);
}
END_TEST

START_TEST(vm_tc_insert_fetch_metadata)
{
	apnvm				*vm;
	apnvm_result			vmrc;
	struct apn_ruleset		*rs;
	struct apnvm_md			md;
	struct apnvm_version_head	version_head;
	struct apnvm_version		*version;

	vm = apnvm_init(apnvm_cvsroot, apnvm_user);
	fail_if(vm == NULL, "Initialization of apnvm failed");

	vmrc = apnvm_prepare(vm);
	fail_if(vmrc != APNVM_OK, "Failed to prepare library");

	rs = vm_tc_create_ruleset(2); /* alf-sfs */
	md.comment = "A silly comment";
	md.auto_store = 4711;

	vmrc = apnvm_insert(vm, "user2", "xxx", rs, &md);
	fail_if(vmrc != APNVM_OK, "Failed to insert ruleset");
	apn_free_ruleset(rs);

	TAILQ_INIT(&version_head);
	vmrc = apnvm_list(vm, "user2", &version_head);
	fail_if(vmrc != APNVM_OK, "List operation failed");

	TAILQ_FOREACH(version, &version_head, entries) {
		if (version->no == 4) { /* The version previously inserted */
			int result = strcmp(version->comment,
			    "A silly comment");

			fail_if(result != 0, "Unexpected comment: %s",
			    version->comment);
			fail_if(version->auto_store != 4711,
			    "Unexpected auto_store-value: %i",
			    version->auto_store);
		}
	}

	apnvm_version_head_free(&version_head);

	apnvm_destroy(vm);
}
END_TEST

TCase *
apnvm_tc_vm_insert(void)
{
	TCase *tc = tcase_create("VM_INSERT");

	tcase_set_timeout(tc, TEST_TIMEOUT);
	tcase_add_checked_fixture(tc, apnvm_setup, apnvm_teardown);

	tcase_add_test(tc, vm_tc_insert_with_profile_overwrite_first);
	tcase_add_test(tc, vm_tc_insert_with_profile_overwrite_last);
	tcase_add_test(tc, vm_tc_insert_with_profile_overwrite_middle);
	tcase_add_test(tc, vm_tc_insert_with_profile_new_profile);
	tcase_add_test(tc, vm_tc_insert_without_profile_overwrite);
	tcase_add_test(tc, vm_tc_insert_with_profile_new_user);
	tcase_add_test(tc, vm_tc_insert_without_profile_new_user);
	tcase_add_test(tc, vm_tc_insert_no_user);
	tcase_add_test(tc, vm_tc_insert_no_ruleset);
	tcase_add_test(tc, vm_tc_insert_fetch_metadata);

	return (tc);
}

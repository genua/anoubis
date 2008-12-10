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

#include <sys/time.h>
#include <sys/resource.h>

#include <anoubischeck.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

START_TEST(tc_success)
{
	/* Nothing to do here */
}
END_TEST

START_TEST(tc_failure)
{
	fail("A silly failure");
}
END_TEST

START_TEST(tc_error)
{
	struct rlimit	rlim;
	int		result;

	result = getrlimit(RLIMIT_CORE, &rlim);
	fail_unless(result == 0, "getrlimit failed with %i (%s)",
	    errno, strerror(errno));

	rlim.rlim_cur = 0;
	result = setrlimit(RLIMIT_CORE, &rlim);
	fail_unless(result == 0, "setrlimit failed with %i (%s)",
	    errno, strerror(errno));

	abort();
}
END_TEST

TCase *
testcase_success(void)
{
	TCase *tc = tcase_create("TestCaseSuccess");

	tcase_add_test(tc, tc_success);

	return (tc);
}

TCase *
testcase_failure(void)
{
	TCase *tc = tcase_create("TestCaseFailure");

	tcase_add_test(tc, tc_success);
	tcase_add_test(tc, tc_failure);

	return (tc);
}

TCase *
testcase_error(void)
{
	TCase *tc = tcase_create("TestCaseError");

	tcase_add_test(tc, tc_success);
	tcase_add_test(tc, tc_failure);
	tcase_add_test(tc, tc_error);

	return (tc);
}

Suite *
suite_success(void)
{
	Suite *s = suite_create("SuiteSuccess");

	suite_add_tcase(s, testcase_success());

	return (s);
}

Suite *
suite_failure(void)
{
	Suite *s = suite_create("SuiteFailure");

	suite_add_tcase(s, testcase_failure());

	return (s);
}

Suite *
suite_error(void)
{
	Suite *s = suite_create("SuiteError");

	suite_add_tcase(s, testcase_error());

	return (s);
}

int
main(void)
{
	SRunner	*srunner_success, *srunner_failure, *srunner_error;
	int result_success, result_failure, result_error;
	int result = 0;

	srunner_success = srunner_create(suite_success());
	srunner_failure = srunner_create(suite_failure());
	srunner_error = srunner_create(suite_error());

	srunner_run_all(srunner_success, CK_NORMAL);
	srunner_run_all(srunner_failure, CK_NORMAL);
	srunner_run_all(srunner_error, CK_NORMAL);

	result_success = check_eval_srunner(srunner_success);
	result_failure = check_eval_srunner(srunner_failure);
	result_error = check_eval_srunner(srunner_error);

	if (result_success != 0) {
		fprintf(stderr, "Wrong evaluation from srunner_success!\n");
		fprintf(stderr, "Is %i but should be 0\n", result_success);

		result |= 1;
	}

	if (result_failure != 1) {
		fprintf(stderr, "Wrong evaluation from srunner_failure!\n");
		fprintf(stderr, "Is %i but should be 1\n", result_failure);

		result |= 1;
	}

	if (result_error != 2) {
		fprintf(stderr, "Wrong evaluation from srunner_error!\n");
		fprintf(stderr, "Is %i but should be 2\n", result_error);

		result |= 1;
	}

	srunner_free(srunner_success);
	srunner_free(srunner_failure);
	srunner_free(srunner_error);

	return (result);
}

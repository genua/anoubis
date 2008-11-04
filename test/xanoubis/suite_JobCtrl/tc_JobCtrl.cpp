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

#include "JobCtrlApp.h"

START_TEST(tc_jobctrl)
{
	int argc = 3;
	const char *argv[] = {
		__FUNCTION__,
		"-t", "tc_jobctrl"
	};

	int result = jobCtrlAppEntry(argc, (char**)argv);
	fail_if(result != 0, "Unexpected result: %i", result);
}
END_TEST

START_TEST(tc_jobctrl_nodaemon)
{
	int argc = 3;
	const char *argv[] = {
		__FUNCTION__,
		"-t", "tc_jobctrl_nodaemon"
	};

	int result = jobCtrlAppEntry(argc, (char**)argv);
	fail_if(result != 0, "Unexpected result: %i", result);
}
END_TEST

TCase *
getTc_JobCtrl(void)
{
	TCase *testCase;

	testCase = tcase_create("JobCtrl");
	tcase_set_timeout(testCase, 10);

	tcase_add_test(testCase, tc_jobctrl);

	return (testCase);
}

TCase *
getTc_JobCtrl_NoConnection(void)
{
	TCase *testCase;

	testCase = tcase_create("JobCtrlNoConnection");
	tcase_set_timeout(testCase, 10);

	tcase_add_test(testCase, tc_jobctrl_nodaemon);

	return (testCase);
}

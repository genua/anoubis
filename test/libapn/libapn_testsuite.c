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

extern TCase *libapn_testcase_errorcodes(void);
extern TCase *libapn_testcase_invalidparams(void);
extern TCase *libapn_testcase_crash_apn_free_ruleset(void);
extern TCase *libapn_testcase_crash_parse_files(void);
extern TCase *libapn_testcase_crash_print_errors(void);
extern TCase *libapn_testcase_iovec(void);
extern TCase *libapn_testcase_firstinsert(void);

Suite *
libapn_testsuite(void)
{
	Suite *s = suite_create("Suite");

	/* sessions test case */
	TCase *tc_errorcodes = libapn_testcase_errorcodes();
	TCase *tc_invalidparams = libapn_testcase_invalidparams();
	TCase *tc_crash_apn_free_ruleset =
	    libapn_testcase_crash_apn_free_ruleset();
	TCase *tc_crash_parse_files = libapn_testcase_crash_parse_files();
	TCase *tc_crash_print_error = libapn_testcase_crash_print_errors();
	TCase *tc_iovec = libapn_testcase_iovec();
	tcase_set_timeout(tc_iovec, 60);
	TCase *tc_firstinsert = libapn_testcase_firstinsert();

	suite_add_tcase(s, tc_errorcodes);
	suite_add_tcase(s, tc_invalidparams);
	suite_add_tcase(s, tc_crash_apn_free_ruleset);
	suite_add_tcase(s, tc_crash_parse_files);
	suite_add_tcase(s, tc_crash_print_error);
	suite_add_tcase(s, tc_iovec);
	suite_add_tcase(s, tc_firstinsert);

	return (s);
}

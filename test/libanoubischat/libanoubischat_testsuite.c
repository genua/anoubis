/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

extern TCase *libanoubischat_testcase_core(void);
extern TCase *libanoubischat_testcase_connect(void);
extern TCase *libanoubischat_testcase_chat(void);
extern TCase *libanoubischat_testcase_prepare(void);
extern TCase *libanoubischat_testcase_recvmsg(void);


Suite *
libanoubischat_testsuite(void)
{
	Suite *s = suite_create("Suite");

	/* Core test case */
	TCase *tc_core = libanoubischat_testcase_core();
	suite_add_tcase(s, tc_core);

	/* Connect test case */
	TCase *tc_connect = libanoubischat_testcase_connect();
	suite_add_tcase(s, tc_connect);

	/* Chat test case */
	TCase *tc_chat = libanoubischat_testcase_chat();
	suite_add_tcase(s, tc_chat);

	/* Preparation test case */
	TCase *tc_prepare = libanoubischat_testcase_prepare();
	suite_add_tcase(s, tc_prepare);

	/* recvmsg test case */
	TCase *tc_recvmsg = libanoubischat_testcase_recvmsg();
	suite_add_tcase(s, tc_recvmsg);

	return (s);
}

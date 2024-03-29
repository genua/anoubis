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

extern TCase *getTc_PolicyRuleSet(void);
extern TCase *getTc_AlfAppPolicy(void);
extern TCase *getTc_AlfCapabilityFilterPolicy(void);
extern TCase *getTc_AlfFilterPolicy(void);
extern TCase *getTc_ContextAppPolicy(void);
extern TCase *getTc_ContextFilterPolicy(void);
extern TCase *getTc_DefaultFilterPolicy(void);
extern TCase *getTc_SbAccessFilterPolicy(void);
extern TCase *getTc_SbAppPolicy(void);
extern TCase *getTc_SfsAppPolicy(void);
extern TCase *getTc_SfsFilterPolicy(void);
extern TCase *getTc_SfsDefaultFilterPolicy(void);
extern TCase *getTc_PolicyUtils(void);
extern TCase *getTc_PolicyChecks(void);

Suite *
getTestSuite(void)
{
	Suite *testSuite;

	testSuite = suite_create("Policies");

	suite_add_tcase(testSuite, getTc_PolicyRuleSet());
	suite_add_tcase(testSuite, getTc_AlfAppPolicy());
	suite_add_tcase(testSuite, getTc_AlfCapabilityFilterPolicy());
	suite_add_tcase(testSuite, getTc_AlfFilterPolicy());
	suite_add_tcase(testSuite, getTc_ContextAppPolicy());
	suite_add_tcase(testSuite, getTc_ContextFilterPolicy());
	suite_add_tcase(testSuite, getTc_DefaultFilterPolicy());
	suite_add_tcase(testSuite, getTc_SbAccessFilterPolicy());
	suite_add_tcase(testSuite, getTc_SbAppPolicy());
	suite_add_tcase(testSuite, getTc_SfsAppPolicy());
	suite_add_tcase(testSuite, getTc_SfsFilterPolicy());
	suite_add_tcase(testSuite, getTc_SfsDefaultFilterPolicy());
	suite_add_tcase(testSuite, getTc_PolicyUtils());
	suite_add_tcase(testSuite, getTc_PolicyChecks());

	return (testSuite);
}

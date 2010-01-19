/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include "check.h"

#include <PolicyRuleSet.h>

static PolicyRuleSet *
createPolicyRuleSet(const char *data)
{
	struct iovec		 iv;
	struct apn_ruleset	*rs;

	iv.iov_base = (void *)data;
	iv.iov_len = strlen((char *)iv.iov_base) - 1;

	if (apn_parse_iovec("<iov>", &iv, 1, &rs, 0) != 0) {
		apn_print_errors(rs, stderr);
		fail("Couldn't create apn rule set.");
	}

	return (new PolicyRuleSet(1, geteuid(), rs));
}

START_TEST(empty)
{
	PolicyRuleSet	*rs;
	AppPolicy	*app;
	FilterPolicy	*filter;

	rs = createPolicyRuleSet("\n");
	fail_unless(rs->getAppPolicyCount() == 1);

	app = rs->getPolicyAt(0);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(SfsAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(1);
	fail_unless(app == 0);

	delete rs;
}
END_TEST

START_TEST(alf)
{
	PolicyRuleSet	*rs;
	AppPolicy	*app;
	FilterPolicy	*filter;

	rs= createPolicyRuleSet(
	    "alf {\nany {\ndefault deny\n}\n}\n");
	fail_unless(rs->getAppPolicyCount() == 2);

	app = rs->getPolicyAt(0);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(AlfAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter != 0);
	fail_unless(filter->IsKindOf(CLASSINFO(DefaultFilterPolicy)));

	filter = app->getFilterPolicyAt(1);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(1);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(SfsAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(2);
	fail_unless(app == 0);

	delete rs;
}
END_TEST

START_TEST(sfs)
{
	PolicyRuleSet	*rs;
	AppPolicy	*app;
	FilterPolicy	*filter;

	rs = createPolicyRuleSet(
	    "sfs {\ndefault any allow\n}\n");

	fail_unless(rs->getAppPolicyCount() == 1);

	app = rs->getPolicyAt(0);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(SfsAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter != 0);
	fail_unless(filter->IsKindOf(CLASSINFO(SfsDefaultFilterPolicy)));

	filter = app->getFilterPolicyAt(1);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(1);
	fail_unless(app == 0);

	delete rs;
}
END_TEST

START_TEST(sb)
{
	PolicyRuleSet	*rs;
	AppPolicy	*app;
	FilterPolicy	*filter;

	rs = createPolicyRuleSet(
	    "sandbox {\n\"/bin/true\" {\ndefault allow\n}\n}\n");
	fail_unless(rs->getAppPolicyCount() == 2);

	app = rs->getPolicyAt(0);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(SfsAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(1);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(SbAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter != 0);
	fail_unless(filter->IsKindOf(CLASSINFO(DefaultFilterPolicy)));

	filter = app->getFilterPolicyAt(1);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(2);
	fail_unless(app == 0);

	delete rs;
}
END_TEST

START_TEST(ctx)
{
	PolicyRuleSet	*rs;
	AppPolicy	*app;
	FilterPolicy	*filter;

	rs = createPolicyRuleSet(
	    "context {\nany {\ncontext new any\n}\n}\n");
	fail_unless(rs->getAppPolicyCount() == 2);

	app = rs->getPolicyAt(0);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(SfsAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(1);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(ContextAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter != 0);
	fail_unless(filter->IsKindOf(CLASSINFO(ContextFilterPolicy)));

	filter = app->getFilterPolicyAt(1);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(2);
	fail_unless(app == 0);

	delete rs;
}
END_TEST

START_TEST(combined)
{
	PolicyRuleSet	*rs;
	AppPolicy	*app;
	FilterPolicy	*filter;

	rs = createPolicyRuleSet(
	    "alf {\nany {\ndefault deny\n}\n}\n"
	    "sfs {\ndefault any allow\n}\n"
	    "sandbox {\n\"/bin/true\" {\ndefault allow\n}\n}\n"
	    "context {\nany {\ncontext new any\n}\n}\n");
	fail_unless(rs->getAppPolicyCount() == 4);

	app = rs->getPolicyAt(0);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(AlfAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter != 0);
	fail_unless(filter->IsKindOf(CLASSINFO(DefaultFilterPolicy)));

	filter = app->getFilterPolicyAt(1);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(1);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(SfsAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter != 0);
	fail_unless(filter->IsKindOf(CLASSINFO(SfsDefaultFilterPolicy)));

	filter = app->getFilterPolicyAt(1);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(2);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(ContextAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter != 0);
	fail_unless(filter->IsKindOf(CLASSINFO(ContextFilterPolicy)));

	filter = app->getFilterPolicyAt(1);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(3);
	fail_unless(app != 0);
	fail_unless(app->IsKindOf(CLASSINFO(SbAppPolicy)));

	filter = app->getFilterPolicyAt(0);
	fail_unless(filter != 0);
	fail_unless(filter->IsKindOf(CLASSINFO(DefaultFilterPolicy)));

	filter = app->getFilterPolicyAt(1);
	fail_unless(filter == 0);

	app = rs->getPolicyAt(4);
	fail_unless(app == 0);

	delete rs;
}
END_TEST

TCase *
getTc_PolicyRuleSet(void)
{
	TCase *testCase;

	testCase = tcase_create("PolicyRuleSet");

	tcase_add_test(testCase, empty);
	tcase_add_test(testCase, alf);
	tcase_add_test(testCase, sfs);
	tcase_add_test(testCase, sb);
	tcase_add_test(testCase, ctx);
	tcase_add_test(testCase, combined);

	return (testCase);
}

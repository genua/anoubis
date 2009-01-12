/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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
#include <SbAppPolicy.h>
#include <SbAccessFilterPolicy.h>
#include <PolicyRuleSet.h>

#include "policyChecks.h"
#include "PolicyObserver.h"

static apn_rule			*rule     = NULL;
static PolicyObserver		*observer = NULL;
static SbAccessFilterPolicy	*policy   = NULL;

static void
setup(void)
{
	SbAppPolicy	*app;

	fail_if(rule     != NULL, "apn rule already exists @ setup().");
	fail_if(observer != NULL, "observer already exists @ setup().");
	fail_if(policy   != NULL, "Policy already exists @ setup().");

	app = new SbAppPolicy(NULL, NULL);
	FAIL_IFZERO(app, "Couldn't create SbAccessAppPolicy.");

	rule = SbAccessFilterPolicy::createApnRule();
	FAIL_IFZERO(rule, "Couldn't create apn filter rule.");

	policy = new SbAccessFilterPolicy(app, rule);
	FAIL_IFZERO(policy, "Couldn't create SbAccessFilterPolicy.");

	observer = new PolicyObserver(policy);
	FAIL_IFZERO(observer, "Couldn't create observer.");
}

static void
teardown(void)
{
	FAIL_IFZERO(rule,     "No apn rule for teardown().");
	FAIL_IFZERO(observer, "No observer for teardown().");
	FAIL_IFZERO(policy,   "No policy for teardown().");

	delete observer;
	observer = NULL;
	mark_point();

	delete policy->getParentPolicy();
	delete policy;
	policy = NULL;
	mark_point();

	apn_free_one_rule(rule, NULL);
	rule = NULL;
	mark_point();
}

/*
 * Unit tests
 */
START_TEST(SbAccessFilterPolicy_getTypeIdentifier)
{
	if (!policy->getTypeIdentifier().IsSameAs(wxT("SB"))) {
		fail("getTypeIdentifier() value not as expected: %ls",
		    policy->getTypeIdentifier().c_str());
	}
}
END_TEST

START_TEST(SbAccessFilterPolicy_LogNo)
{
	wxString logName;

	/* Check initialization. */
	logName = wxT("(unknown)");
	CHECK_POLICY_GETLOGNO(policy, -1);
	CHECK_POLICY_GETLOGNAME(policy, logName);

	if (policy->setLogNo(APN_LOG_NORMAL)) {
		fail("setLogNo(): successfull - fail expected.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETLOGNO(policy, -1);
	CHECK_POLICY_GETLOGNAME(policy, logName);
}
END_TEST

START_TEST(SbAccessFilterPolicy_ActionNo)
{
	wxString actionName;

	/* Check initialization. */
	actionName = wxT("(unknown)");
	CHECK_POLICY_GETACTIONNO(policy, -1);
	CHECK_POLICY_GETACTIONNAME(policy, actionName);

	if (policy->setActionNo(APN_ACTION_ALLOW)) {
		fail("setActionNo(): successfull - fail expected.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETACTIONNO(policy, -1);
	CHECK_POLICY_GETACTIONNAME(policy, actionName);
}
END_TEST

/*
 * Test case
 */
TCase *
getTc_SbAccessFilterPolicy(void)
{
	TCase *testCase;

	testCase = tcase_create("SbAccessFilterPolicy");
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, SbAccessFilterPolicy_getTypeIdentifier);
	tcase_add_test(testCase, SbAccessFilterPolicy_LogNo);
	tcase_add_test(testCase, SbAccessFilterPolicy_ActionNo);

	return (testCase);
}

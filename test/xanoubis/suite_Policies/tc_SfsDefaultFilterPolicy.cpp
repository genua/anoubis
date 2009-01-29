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
#include <SfsAppPolicy.h>
#include <SfsDefaultFilterPolicy.h>
#include <PolicyRuleSet.h>

#include "policyChecks.h"
#include "PolicyObserver.h"

static apn_rule			*rule     = NULL;
static PolicyObserver		*observer = NULL;
static SfsDefaultFilterPolicy	*policy   = NULL;

static void
setup(void)
{
	SfsAppPolicy	*app;

	fail_if(rule     != NULL, "apn rule already exists @ setup().");
	fail_if(observer != NULL, "observer already exists @ setup().");
	fail_if(policy   != NULL, "Policy already exists @ setup().");

	app = new SfsAppPolicy(NULL, NULL);
	FAIL_IFZERO(app, "Couldn't create SfsAppPolicy.");

	rule = SfsDefaultFilterPolicy::createApnRule();
	FAIL_IFZERO(rule, "Couldn't create apn capability rule.");

	policy = new SfsDefaultFilterPolicy(app, rule);
	FAIL_IFZERO(policy, "Couldn't create SfsDefaultFilterPolicy.");

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

START_TEST(SfsDefaultFilterPolicy_getTypeIdentifier)
{
	if (!policy->getTypeIdentifier().IsSameAs(wxT("Default"))) {
		fail("getTypeIdentifier() value not as expected: %ls",
		    policy->getTypeIdentifier().c_str());
	}
}
END_TEST

START_TEST(SfsDefaultFilterPolicy_LogNo)
{
	wxString logName;

	/* Check initialization. */
	logName = wxT("none");
	CHECK_POLICY_GETLOGNO(policy, APN_LOG_NONE);
	CHECK_POLICY_GETLOGNAME(policy, logName);

	logName = wxT("normal");
	if (!policy->setLogNo(APN_LOG_NORMAL)) {
		fail("setLogNo(APN_LOG_NORMAL): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETLOGNO(policy, APN_LOG_NORMAL);
	CHECK_POLICY_GETLOGNAME(policy, logName);

	logName = wxT("alert");
	if (!policy->setLogNo(APN_LOG_ALERT)) {
		fail("setLogNo(APN_LOG_ALERT): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETLOGNO(policy, APN_LOG_ALERT);
	CHECK_POLICY_GETLOGNAME(policy, logName);

	logName = wxT("none");
	if (!policy->setLogNo(APN_LOG_NONE)) {
		fail("setLogNo(APN_LOG_NONE): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETLOGNO(policy, APN_LOG_NONE);
	CHECK_POLICY_GETLOGNAME(policy, logName);
}
END_TEST

START_TEST(SfsDefaultFilterPolicy_ActionNo)
{
	wxString actionName;

	/* Check initialization. */
	actionName = wxT("allow");
	CHECK_POLICY_GETACTIONNO(policy, APN_ACTION_ALLOW);
	CHECK_POLICY_GETACTIONNAME(policy, actionName);

	actionName = wxT("allow");
	if (!policy->setActionNo(APN_ACTION_ALLOW)) {
		fail("setActionNo(APN_ACTION_ALLOW): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETACTIONNO(policy, APN_ACTION_ALLOW);
	CHECK_POLICY_GETACTIONNAME(policy, actionName);

	actionName = wxT("ask");
	if (!policy->setActionNo(APN_ACTION_ASK)) {
		fail("setActionNo(APN_ACTION_ASK): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETACTIONNO(policy, APN_ACTION_ASK);
	CHECK_POLICY_GETACTIONNAME(policy, actionName);

	actionName = wxT("deny");
	if (!policy->setActionNo(APN_ACTION_DENY)) {
		fail("setActionNo(APN_ACTION_DENY): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETACTIONNO(policy, APN_ACTION_DENY);
	CHECK_POLICY_GETACTIONNAME(policy, actionName);
}
END_TEST

START_TEST(SfsDefaultFilterPolicy_Path)
{
	wxString path;

	/* Check initialization. */
	path = wxT("any");
	CHECK_POLICY_GETPATH(policy, path);

	path = wxT("/usr/local/bin");
	if (!policy->setPath(path)) {
		fail("setPath(): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETPATH(policy, path);

	path = wxT("/usr/xxx/zz");
	if (!policy->setPath(path)) {
		fail("setPath(): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETPATH(policy, path);

	path = wxT("any");
	if (!policy->setPath(path)) {
		fail("setPath(): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETPATH(policy, path);
}
END_TEST

TCase *
getTc_SfsDefaultFilterPolicy(void)
{
	TCase *testCase;

	testCase = tcase_create("SfsDefaultFilterPolicy");
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, SfsDefaultFilterPolicy_getTypeIdentifier);
	tcase_add_test(testCase, SfsDefaultFilterPolicy_LogNo);
	tcase_add_test(testCase, SfsDefaultFilterPolicy_ActionNo);
	tcase_add_test(testCase, SfsDefaultFilterPolicy_Path);

	return (testCase);
}

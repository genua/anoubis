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

START_TEST(SbAccessFilterPolicy_ActionNo)
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

START_TEST(SbAccessFilterPolicy_Path)
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

START_TEST(SbAccessFilterPolicy_Subject)
{
	wxString subject;

	/* Check initialization. */
	subject = wxT("none");
	CHECK_POLICY_GETSUBJECTTYPENO(policy, APN_CS_NONE);
	CHECK_POLICY_GETSUBJECTNAME(policy, subject);

	/* SELF */
	subject = wxT("self");
	if (!policy->setSubjectSelf(false)) {
		fail("setSubjectSelf(false): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETSUBJECTTYPENO(policy, APN_CS_UID_SELF);
	CHECK_POLICY_GETSUBJECTNAME(policy, subject);

	/* SELF-SIGNED */
	subject = wxT("signed-self");
	if (!policy->setSubjectSelf(true)) {
		fail("setSubjectSelf(true): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETSUBJECTTYPENO(policy, APN_CS_KEY_SELF);
	CHECK_POLICY_GETSUBJECTNAME(policy, subject);

	/* KEY asdfasdf */
	subject = wxT("key asdfasdf");
	if (!policy->setSubjectKey(wxT("asdfasdf"))) {
		fail("setSubjectKey(): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETSUBJECTTYPENO(policy, APN_CS_KEY);
	CHECK_POLICY_GETSUBJECTNAME(policy, subject);

	/* UID 123 */
	subject = wxT("uid 123");
	if (!policy->setSubjectUid(123)) {
		fail("setSubjectUid(): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETSUBJECTTYPENO(policy, APN_CS_UID);
	CHECK_POLICY_GETSUBJECTNAME(policy, subject);
}
END_TEST

START_TEST(SbAccessFilterPolicy_AccessMask)
{
	wxString maskName;

	/* Check initialization. */
	maskName = wxT("rwx");
	CHECK_POLICY_GETMASKNO(policy, APN_SBA_ALL);
	CHECK_POLICY_GETMASKNAME(policy, maskName);

	maskName = wxT("r");
	if (!policy->setAccessMask(APN_SBA_READ)) {
		fail("setAccessMask(APN_SBA_READ): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETMASKNO(policy, APN_SBA_READ);
	CHECK_POLICY_GETMASKNAME(policy, maskName);

	maskName = wxT("w");
	if (!policy->setAccessMask(APN_SBA_WRITE)) {
		fail("setAccessMask(APN_SBA_WRITE): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETMASKNO(policy, APN_SBA_WRITE);
	CHECK_POLICY_GETMASKNAME(policy, maskName);

	maskName = wxT("x");
	if (!policy->setAccessMask(APN_SBA_EXEC)) {
		fail("setAccessMask(APN_SBA_EXEC): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETMASKNO(policy, APN_SBA_EXEC);
	CHECK_POLICY_GETMASKNAME(policy, maskName);

	maskName = wxT("rw");
	if (!policy->setAccessMask(APN_SBA_READ | APN_SBA_WRITE)) {
		fail("setAccessMask(READ | WRITE): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETMASKNO(policy, (APN_SBA_READ | APN_SBA_WRITE));
	CHECK_POLICY_GETMASKNAME(policy, maskName);

	maskName = wxT("rx");
	if (!policy->setAccessMask(APN_SBA_READ | APN_SBA_EXEC)) {
		fail("setAccessMask(READ | EXEC): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETMASKNO(policy, (APN_SBA_READ | APN_SBA_EXEC));
	CHECK_POLICY_GETMASKNAME(policy, maskName);

	maskName = wxT("wx");
	if (!policy->setAccessMask(APN_SBA_WRITE | APN_SBA_EXEC)) {
		fail("setAccessMask(WRITE | EXEC): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETMASKNO(policy, (APN_SBA_WRITE | APN_SBA_EXEC));
	CHECK_POLICY_GETMASKNAME(policy, maskName);

	maskName = wxT("rwx");
	if (!policy->setAccessMask(APN_SBA_READ|APN_SBA_WRITE|APN_SBA_EXEC)) {
		fail("setAccessMask(READ | WRITE | EXEC): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETMASKNO(policy,
	    (APN_SBA_READ | APN_SBA_WRITE | APN_SBA_EXEC));
	CHECK_POLICY_GETMASKNAME(policy, maskName);
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
	tcase_add_test(testCase, SbAccessFilterPolicy_Path);
	tcase_add_test(testCase, SbAccessFilterPolicy_Subject);
	tcase_add_test(testCase, SbAccessFilterPolicy_AccessMask);

	return (testCase);
}

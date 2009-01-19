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
#include <SfsFilterPolicy.h>
#include <PolicyRuleSet.h>

#include "policyChecks.h"
#include "PolicyObserver.h"

static apn_rule			*rule     = NULL;
static PolicyObserver		*observer = NULL;
static SfsFilterPolicy	*policy   = NULL;

static void
setup(void)
{
	SfsAppPolicy	*app;

	fail_if(rule     != NULL, "apn rule already exists @ setup().");
	fail_if(observer != NULL, "observer already exists @ setup().");
	fail_if(policy   != NULL, "Policy already exists @ setup().");

	app = new SfsAppPolicy(NULL, NULL);
	FAIL_IFZERO(app, "Couldn't create SfsAppPolicy.");

	rule = SfsFilterPolicy::createApnRule();
	FAIL_IFZERO(rule, "Couldn't create apn filter rule.");

	policy = new SfsFilterPolicy(app, rule);
	FAIL_IFZERO(policy, "Couldn't create SfsFilterPolicy.");

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
START_TEST(SfsFilterPolicy_getTypeIdentifier)
{
	if (!policy->getTypeIdentifier().IsSameAs(wxT("SFS"))) {
		fail("getTypeIdentifier() value not as expected: %ls",
		    policy->getTypeIdentifier().c_str());
	}
}
END_TEST

START_TEST(SfsFilterPolicy_LogNo)
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

START_TEST(SfsFilterPolicy_ActionNo)
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

START_TEST(SfsFilterPolicy_Path)
{
	wxString path;

	/* Check initialization. */
	path = wxT("any");
	CHECK_POLICY_GETPATH(policy, path);

	path = wxT("/usr/local/bin");
	if (!policy->setPath(path)) {
		fail("setPath(): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETPATH(policy, path);

	path = wxT("/usr/xxx/zz");
	if (!policy->setPath(path)) {
		fail("setPath(): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETPATH(policy, path);

	path = wxT("any");
	if (!policy->setPath(path)) {
		fail("setPath(): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETPATH(policy, path);
}
END_TEST

START_TEST(SfsFilterPolicy_Subject)
{
	wxString subject;

	/* Check initialization. */
	subject = wxT("none");
	CHECK_POLICY_GETSUBJECTTYPENO(policy, APN_CS_NONE);
	CHECK_POLICY_GETSUBJECTNAME(policy, subject);

	/* SELF */
	subject = wxT("self");
	if (!policy->setSubjectSelf(false)) {
		fail("setSubjectSelf(false): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETSUBJECTTYPENO(policy, APN_CS_UID_SELF);
	CHECK_POLICY_GETSUBJECTNAME(policy, subject);

	/* SELF-SIGNED */
	subject = wxT("signed-self");
	if (!policy->setSubjectSelf(true)) {
		fail("setSubjectSelf(true): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETSUBJECTTYPENO(policy, APN_CS_KEY_SELF);
	CHECK_POLICY_GETSUBJECTNAME(policy, subject);

	/* KEY asdfasdf */
	subject = wxT("key asdfasdf");
	if (!policy->setSubjectKey(wxT("asdfasdf"))) {
		fail("setSubjectKey(): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETSUBJECTTYPENO(policy, APN_CS_KEY);
	CHECK_POLICY_GETSUBJECTNAME(policy, subject);

	/* UID 123 */
	subject = wxT("uid 123");
	if (!policy->setSubjectUid(123)) {
		fail("setSubjectUid(): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETSUBJECTTYPENO(policy, APN_CS_UID);
	CHECK_POLICY_GETSUBJECTNAME(policy, subject);
}
END_TEST

START_TEST(SfsFilterPolicy_ValidAction)
{
	wxString action;

	/* Check initialization. */
	action = wxT("allow");
	CHECK_POLICY_GETVALIDACTION(policy, APN_ACTION_ALLOW, action);

	action = wxT("continue");
	if (!policy->setValidAction(APN_ACTION_CONTINUE)) {
		fail("setActionNo(APN_ACTION_CONTINUE): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETVALIDACTION(policy, APN_ACTION_CONTINUE, action);

	action = wxT("allow");
	if (!policy->setValidAction(APN_ACTION_ALLOW)) {
		fail("setActionNo(APN_ACTION_ALLOW): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETVALIDACTION(policy, APN_ACTION_ALLOW, action);

	action = wxT("ask");
	if (!policy->setValidAction(APN_ACTION_ASK)) {
		fail("setActionNo(APN_ACTION_ASK): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETVALIDACTION(policy, APN_ACTION_ASK, action);

	action = wxT("deny");
	if (!policy->setValidAction(APN_ACTION_DENY)) {
		fail("setActionNo(APN_ACTION_DENY): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETVALIDACTION(policy, APN_ACTION_DENY, action);
}
END_TEST

START_TEST(SfsFilterPolicy_ValidLog)
{
	wxString log;

	/* Check initialization. */
	log = wxT("none");
	CHECK_POLICY_GETVALIDLOG(policy, APN_LOG_NONE, log);

	log = wxT("normal");
	if (!policy->setValidLog(APN_LOG_NORMAL)) {
		fail("setLogNo(APN_LOG_NORMAL): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETVALIDLOG(policy, APN_LOG_NORMAL, log);

	log = wxT("alert");
	if (!policy->setValidLog(APN_LOG_ALERT)) {
		fail("setLogNo(APN_LOG_ALERT): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETVALIDLOG(policy, APN_LOG_ALERT, log);

	log = wxT("none");
	if (!policy->setValidLog(APN_LOG_NONE)) {
		fail("setLogNo(APN_LOG_NONE): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETVALIDLOG(policy, APN_LOG_NONE, log);
}
END_TEST

START_TEST(SfsFilterPolicy_InvalidAction)
{
	wxString action;

	/* Check initialization. */
	action = wxT("allow");
	CHECK_POLICY_GETINVALIDACTION(policy, APN_ACTION_ALLOW, action);

	action = wxT("continue");
	if (!policy->setInvalidAction(APN_ACTION_CONTINUE)) {
		fail("setActionNo(APN_ACTION_CONTINUE): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETINVALIDACTION(policy, APN_ACTION_CONTINUE, action);

	action = wxT("allow");
	if (!policy->setInvalidAction(APN_ACTION_ALLOW)) {
		fail("setActionNo(APN_ACTION_ALLOW): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETINVALIDACTION(policy, APN_ACTION_ALLOW, action);

	action = wxT("ask");
	if (!policy->setInvalidAction(APN_ACTION_ASK)) {
		fail("setActionNo(APN_ACTION_ASK): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETINVALIDACTION(policy, APN_ACTION_ASK, action);

	action = wxT("deny");
	if (!policy->setInvalidAction(APN_ACTION_DENY)) {
		fail("setActionNo(APN_ACTION_DENY): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETINVALIDACTION(policy, APN_ACTION_DENY, action);
}
END_TEST

START_TEST(SfsFilterPolicy_InvalidLog)
{
	wxString log;

	/* Check initialization. */
	log = wxT("none");
	CHECK_POLICY_GETINVALIDLOG(policy, APN_LOG_NONE, log);

	log = wxT("normal");
	if (!policy->setInvalidLog(APN_LOG_NORMAL)) {
		fail("setLogNo(APN_LOG_NORMAL): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETINVALIDLOG(policy, APN_LOG_NORMAL, log);

	log = wxT("alert");
	if (!policy->setInvalidLog(APN_LOG_ALERT)) {
		fail("setLogNo(APN_LOG_ALERT): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETINVALIDLOG(policy, APN_LOG_ALERT, log);

	log = wxT("none");
	if (!policy->setInvalidLog(APN_LOG_NONE)) {
		fail("setLogNo(APN_LOG_NONE): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETINVALIDLOG(policy, APN_LOG_NONE, log);
}
END_TEST

START_TEST(SfsFilterPolicy_UnknownAction)
{
	wxString action;

	/* Check initialization. */
	action = wxT("allow");
	CHECK_POLICY_GETUNKNOWNACTION(policy, APN_ACTION_ALLOW, action);

	action = wxT("continue");
	if (!policy->setUnknownAction(APN_ACTION_CONTINUE)) {
		fail("setActionNo(APN_ACTION_CONTINUE): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETUNKNOWNACTION(policy, APN_ACTION_CONTINUE, action);

	action = wxT("allow");
	if (!policy->setUnknownAction(APN_ACTION_ALLOW)) {
		fail("setActionNo(APN_ACTION_ALLOW): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETUNKNOWNACTION(policy, APN_ACTION_ALLOW, action);

	action = wxT("ask");
	if (!policy->setUnknownAction(APN_ACTION_ASK)) {
		fail("setActionNo(APN_ACTION_ASK): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETUNKNOWNACTION(policy, APN_ACTION_ASK, action);

	action = wxT("deny");
	if (!policy->setUnknownAction(APN_ACTION_DENY)) {
		fail("setActionNo(APN_ACTION_DENY): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETUNKNOWNACTION(policy, APN_ACTION_DENY, action);
}
END_TEST

START_TEST(SfsFilterPolicy_UnknownLog)
{
	wxString log;

	/* Check initialization. */
	log = wxT("none");
	CHECK_POLICY_GETUNKNOWNLOG(policy, APN_LOG_NONE, log);

	log = wxT("normal");
	if (!policy->setUnknownLog(APN_LOG_NORMAL)) {
		fail("setLogNo(APN_LOG_NORMAL): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETUNKNOWNLOG(policy, APN_LOG_NORMAL, log);

	log = wxT("alert");
	if (!policy->setUnknownLog(APN_LOG_ALERT)) {
		fail("setLogNo(APN_LOG_ALERT): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETUNKNOWNLOG(policy, APN_LOG_ALERT, log);

	log = wxT("none");
	if (!policy->setUnknownLog(APN_LOG_NONE)) {
		fail("setLogNo(APN_LOG_NONE): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETUNKNOWNLOG(policy, APN_LOG_NONE, log);
}
END_TEST

/*
 * Test case
 */
TCase *
getTc_SfsFilterPolicy(void)
{
	TCase *testCase;

	testCase = tcase_create("SfsFilterPolicy");
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, SfsFilterPolicy_getTypeIdentifier);
	tcase_add_test(testCase, SfsFilterPolicy_LogNo);
	tcase_add_test(testCase, SfsFilterPolicy_ActionNo);
	tcase_add_test(testCase, SfsFilterPolicy_Path);
	tcase_add_test(testCase, SfsFilterPolicy_Subject);
	tcase_add_test(testCase, SfsFilterPolicy_ValidAction);
	tcase_add_test(testCase, SfsFilterPolicy_ValidLog);
	tcase_add_test(testCase, SfsFilterPolicy_InvalidAction);
	tcase_add_test(testCase, SfsFilterPolicy_InvalidLog);
	tcase_add_test(testCase, SfsFilterPolicy_UnknownAction);
	tcase_add_test(testCase, SfsFilterPolicy_UnknownLog);

	return (testCase);
}

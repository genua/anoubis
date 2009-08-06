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
#include <ContextAppPolicy.h>
#include <PolicyRuleSet.h>

#include "policyChecks.h"
#include "PolicyObserver.h"

static PolicyRuleSet *
createPolicyRuleSet(void)
{
	struct iovec		 iv;
	struct apn_ruleset	*rs;

	iv.iov_base = (void *)"\nalf {\n any {\n default deny\n }\n }\n ";
	iv.iov_len = strlen((char *)iv.iov_base) - 1;

	if (apn_parse_iovec("<iov>", &iv, 1, &rs, 0) != 0) {
		apn_print_errors(rs, stderr);
		fail("Couldn't create apn rule set.");
	}

	return (new PolicyRuleSet(1, geteuid(), rs));
}

static struct apn_rule *
createApnRule(const char *name)
{
	struct apn_rule *rule;

	rule = ContextAppPolicy::createApnRule();
	FAIL_IFZERO(rule, "Couldn't create apn rule.");

	fail_if(rule->app != NULL, "app already set.");
	rule->app = AppPolicy::createApnApp();
	FAIL_IFZERO(rule->app, "Couldn't create app.");

	fail_if(rule->app->name != NULL, "name already set.");
	rule->app->name = strdup(name);
	rule->app->subject.type = APN_CS_CSUM;
	rule->app->subject.value.csum =
	    (u_int8_t *)calloc(APN_HASH_SHA256_LEN, sizeof(u_int8_t));

	return (rule);
}

static wxString		 initName;
static apn_rule		*rule     = NULL;
static PolicyObserver	*observer = NULL;
static ContextAppPolicy	*policy   = NULL;

static void
setup(void)
{
	fail_if(rule     != NULL, "apn rule already exists @ setup().");
	fail_if(observer != NULL, "observer already exists @ setup().");
	fail_if(policy   != NULL, "Policy already exists @ setup().");

	initName = wxT("/usr/bin/test");
	rule = createApnRule(initName.fn_str());
	FAIL_IFZERO(rule, "Couldn't create apn filter rule.");

	policy = new ContextAppPolicy(NULL, rule);
	FAIL_IFZERO(policy, "Couldn't create ContextFilterPolicy.");

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
START_TEST(ContextAppPolicy_create)
{
	PolicyRuleSet	*ruleset;

	delete policy;
	policy = NULL;
	mark_point();

	apn_free_one_rule(rule, NULL);
	rule = NULL;
	mark_point();

	policy = new ContextAppPolicy(NULL, NULL);
	FAIL_IFZERO(policy, "Couldn't create ContextAppPolicy(NULL, NULL).");
	delete policy;

	ruleset = createPolicyRuleSet();
	policy  = new ContextAppPolicy(ruleset, NULL);
	FAIL_IFZERO(policy, "Couldn't create ContextAppPolicy(ruleset, NULL).");
	if (policy->getParentRuleSet() != ruleset) {
		fail("getParentRuleSet() diffes from constructor.");
	}
	delete policy;
	delete ruleset;

	rule   = createApnRule("/usr/bin/test");
	policy = new ContextAppPolicy(NULL, rule);
	FAIL_IFZERO(policy, "Couldn't create ContextAppPolicy(NULL, rule).");
	delete policy;
	apn_free_one_rule(rule, NULL);

	rule    = createApnRule("/usr/bin/find");
	ruleset = createPolicyRuleSet();
	policy  = new ContextAppPolicy(ruleset, rule);
	FAIL_IFZERO(policy, "Couldn't create ContextAppPolicy(ruleset, rule).");
	if (policy->getParentRuleSet() != ruleset) {
		fail("getParentRuleSet() diffes from constructor.");
	}
}
END_TEST

START_TEST(ContextAppPolicy_getTypeIdentifier)
{
	if (!policy->getTypeIdentifier().IsSameAs(wxT("CTX"))) {
		fail("getTypeIdentifier() value not as expected: %ls",
		    policy->getTypeIdentifier().c_str());
	}
}
END_TEST

START_TEST(ContextAppPolicy_setBinaryName_zero)
{
	wxString	 setName;

	delete policy;
	policy = NULL;
	mark_point();

	policy = new ContextAppPolicy(NULL, NULL);
	FAIL_IFZERO(policy, "Couldn't create ContextAppPolicy(NULL, NULL)");

	if (!observer->addSubject(policy)) {
		fail("Couldn't add new policy.");
	}

	setName = wxT("/usr/bin/test");

	/* test index 0 */
	if (policy->setBinaryName(setName, 0)) {
		fail("setBinaryName() add app to non-existing rule, idx 0.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 0);

	/* test index 1 */
	if (policy->setBinaryName(setName, 1)) {
		fail("setBinaryName() add app to non-existing rule, idx 1.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 0);
}
END_TEST

START_TEST(ContextAppPolicy_setBinaryName_one)
{
	wxString	 setName;
	wxString	 getName;

	/* test: index 1 - expect to fail */
	setName = wxT("/usr/bin/fooobaaa");
	if (policy->setBinaryName(setName, 1)) {
		fail("setBinaryName(): invalid index succeeded.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 1);
	getName = policy->getBinaryName();
	if (!getName.IsSameAs(initName)) {
		fail("setBinaryName(): unexpected name changed %ls != %ls",
		    initName.c_str(), getName.c_str());
	}

	/* test: index 1 - expect to fail */
	setName = wxT("any");
	if (policy->setBinaryName(setName, 1)) {
		fail("setBinaryName(): invalid index succeeded.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 1);
	getName = policy->getBinaryName();
	if (!getName.IsSameAs(initName)) {
		fail("setBinaryName(): unexpected name changed %ls != %ls",
		    initName.c_str(), getName.c_str());
	}

	/* test: index 1 - expect to fail */
	setName = wxEmptyString;
	if (policy->setBinaryName(setName, 1)) {
		fail("setBinaryName(): invalid index succeeded.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 1);
	getName = policy->getBinaryName();
	if (!getName.IsSameAs(initName)) {
		fail("setBinaryName(): unexpected name changed %ls != %ls",
		    initName.c_str(), getName.c_str());
	}

	/* test: index 0 - expected to fail */
	setName = wxEmptyString;
	if (policy->setBinaryName(setName, 0)) {
		fail("setBinaryName(): invalid name succeeded.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 1);
	getName = policy->getBinaryName();
	if (!getName.IsSameAs(initName)) {
		fail("setBinaryName(): unexpected name changed %ls != %ls",
		    initName.c_str(), getName.c_str());
	}

	/* test: index 0 - expected to succeed */
	setName = wxT("/usr/bin/find");
	if (!policy->setBinaryName(setName, 0)) {
		fail("setBinaryName(): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETBINARYCOUNT(policy, 1);
	getName = policy->getBinaryName();
	if (!getName.IsSameAs(setName)) {
		fail("setBinaryName(): unexpected name %ls != %ls",
		    getName.c_str(), setName.c_str());
	}
	if (policy->isAnyBlock()) {
		fail("isAnyBlock() = true, expected: false");
	}

	/* test: index 0 - expected to succeed */
	setName = wxT("any");
	if (!policy->setBinaryName(setName, 0)) {
		fail("setBinaryName(): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETBINARYCOUNT(policy, 0);
	getName = policy->getBinaryName();
	if (!getName.IsSameAs(setName)) {
		fail("setBinaryName(): unexpected name %ls != %ls",
		    getName.c_str(), setName.c_str());
	}
	if (!policy->isAnyBlock()) {
		fail("isAnyBlock() = false, expected: true");
	}
}
END_TEST

START_TEST(ContextAppPolicy_setBinaryName_two)
{
	wxString	 setName;
	wxString	 getName;
	wxString	 initName1;
	struct apn_app	*napp;

	initName1 = wxT("/usr/bin/find");

	/* afterwards add 2nd app */
	rule->app->next = AppPolicy::createApnApp();
	if (rule->app->next == NULL) {
		fail("Couldn't create 2nd app.");
	}

	if (rule->app->next->name != NULL) {
		fail("name already set.");
	}
	napp = rule->app->next;
	napp->name = strdup(initName1.fn_str());
	napp->subject.type = APN_CS_CSUM;
	napp->subject.value.csum =
	    (u_int8_t *)calloc(APN_HASH_SHA256_LEN, sizeof(u_int8_t));

	/* test: index 2 - expect to fail */
	setName = wxT("/usr/bin/fooobaaa");
	if (policy->setBinaryName(setName, 2)) {
		fail("setBinaryName(): invalid index succeeded.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 2);
	CHECK_POLICY_GETBINARYNAME(policy, 0, initName);
	CHECK_POLICY_GETBINARYNAME(policy, 1, initName1);

	/* test: index 2 - expect to fail */
	setName = wxT("any");
	if (policy->setBinaryName(setName, 2)) {
		fail("setBinaryName(): invalid index succeeded.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 2);
	CHECK_POLICY_GETBINARYNAME(policy, 0, initName);
	CHECK_POLICY_GETBINARYNAME(policy, 1, initName1);

	/* test: index 2 - expect to fail */
	setName = wxEmptyString;
	if (policy->setBinaryName(setName, 2)) {
		fail("setBinaryName(): invalid index succeeded.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 2);
	CHECK_POLICY_GETBINARYNAME(policy, 0, initName);
	CHECK_POLICY_GETBINARYNAME(policy, 1, initName1);

	/* test: index 1 - expected to fail */
	setName = wxEmptyString;
	if (policy->setBinaryName(setName, 1)) {
		fail("setBinaryName(): invalid name succeeded.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 2);
	CHECK_POLICY_GETBINARYNAME(policy, 0, initName);
	CHECK_POLICY_GETBINARYNAME(policy, 1, initName1);

	/* test: index 1 - expected to succeed */
	setName = wxT("/usr/bin/less");
	if (!policy->setBinaryName(setName, 1)) {
		fail("setBinaryName(): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETBINARYCOUNT(policy, 2);
	CHECK_POLICY_GETBINARYNAME(policy, 0, initName);
	CHECK_POLICY_GETBINARYNAME(policy, 1, setName);

	/* restore for next tests */
	policy->setBinaryName(initName1, 1);
	CHECK_POLICY_GETBINARYNAME(policy, 1, initName1);
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);

	/* test: index 0 - expected to fail */
	setName = wxEmptyString;
	if (policy->setBinaryName(setName, 0)) {
		fail("setBinaryName(): invalid name succeeded.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 2);
	CHECK_POLICY_GETBINARYNAME(policy, 0, initName);
	CHECK_POLICY_GETBINARYNAME(policy, 1, initName1);

	/* test: index 0 - expected to succeed */
	setName = wxT("/usr/bin/tail");
	if (!policy->setBinaryName(setName, 0)) {
		fail("setBinaryName(): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETBINARYCOUNT(policy, 2);
	CHECK_POLICY_GETBINARYNAME(policy, 0, setName);
	CHECK_POLICY_GETBINARYNAME(policy, 1, initName1);
	if (policy->isAnyBlock()) {
		fail("isAnyBlock() = true, expected: false");
	}

	/* test: index 0 - expected to succeed */
	setName = wxT("any");
	if (!policy->setBinaryName(setName, 0)) {
		fail("setBinaryName(): not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETBINARYCOUNT(policy, 0);
	CHECK_POLICY_GETBINARYNAME(policy, 0, setName);
	if (!policy->isAnyBlock()) {
		fail("isAnyBlock() = false, expected: true");
	}
}
END_TEST

static int
setBinaryListApp(AppPolicy *policy, const wxArrayString &list)
{
	unsigned int	i;
	while(policy->getBinaryCount()) {
		policy->removeBinary(0);
	}
	if (policy->getBinaryCount())
		return false;
	for (i=0; i<list.GetCount(); ++i)
		if (!policy->addBinary(list[i]))
			return false;
	return true;
}

START_TEST(ContextAppPolicy_setBinaryList)
{
	wxArrayString	 getList;
	wxArrayString	 initList;

	initList.Add(wxT("/usr/bin/tail"));
	initList.Add(wxT("/usr/bin/find"));

	/* test non-existing rule - expect to fail */
	delete policy;
	mark_point();

	policy = new ContextAppPolicy(NULL, NULL);
	FAIL_IFZERO(policy, "Couldn't create ContextAppPolicy(NULL, NULL)");

	if (!observer->addSubject(policy)) {
		fail("Couldn't add new policy.");
	}

	if (setBinaryListApp(policy, initList)) {
		fail("Adding binaries to non-existing rule.");
	}
	CHECK_POLICY_MODIFIED(policy, false);
	CHECK_OBSERVER_NOTIFIED(observer, false);
	CHECK_POLICY_GETBINARYCOUNT(policy, 0);

	delete policy;

	/* test replace one element - expect to succeed */
	policy = new ContextAppPolicy(NULL, rule);
	FAIL_IFZERO(policy, "Couldn't create ContextAppPolicy(NULL, rule)");

	if (!observer->addSubject(policy)) {
		fail("Couldn't add new policy.");
	}

	if (!setBinaryListApp(policy, initList)) {
		fail("Adding binaries not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETBINARYCOUNT(policy, 2);
	CHECK_POLICY_GETBINARYNAME(policy, 0, initList.Item(0));
	CHECK_POLICY_GETBINARYNAME(policy, 1, initList.Item(1));
	getList = policy->getBinaryList();
	if ((getList.Item(0).Cmp(initList.Item(0)) != 0) ||
	    (getList.Item(1).Cmp(initList.Item(1)) != 0)) {
		fail("getBinaryList() unexpected return values.");
	}

	/* test replace two elements - expect to succeed */
	initList.Clear();
	initList.Add(wxT("/usr/bin/xxyyzz"));
	initList.Add(wxT("/usr/bin/ccbbaa"));

	if (!setBinaryListApp(policy, initList)) {
		fail("Adding binaries not successfull.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETBINARYCOUNT(policy, 2);
	CHECK_POLICY_GETBINARYNAME(policy, 0, initList.Item(0));
	CHECK_POLICY_GETBINARYNAME(policy, 1, initList.Item(1));
	getList = policy->getBinaryList();
	if ((getList.Item(0).Cmp(initList.Item(0)) != 0) ||
	    (getList.Item(1).Cmp(initList.Item(1)) != 0)) {
		fail("getBinaryList() unexpected return values.");
	}
}
END_TEST

/*
 * Test case
 */
TCase *
getTc_ContextAppPolicy(void)
{
	TCase *testCase;

	testCase = tcase_create("ContextAppPolicy");
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, ContextAppPolicy_create);
	tcase_add_test(testCase, ContextAppPolicy_getTypeIdentifier);
	tcase_add_test(testCase, ContextAppPolicy_setBinaryName_zero);
	tcase_add_test(testCase, ContextAppPolicy_setBinaryName_one);
	tcase_add_test(testCase, ContextAppPolicy_setBinaryName_two);
	tcase_add_test(testCase, ContextAppPolicy_setBinaryList);

	return (testCase);
}

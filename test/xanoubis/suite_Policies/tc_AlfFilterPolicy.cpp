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
#include <AlfAppPolicy.h>
#include <AlfFilterPolicy.h>
#include <PolicyRuleSet.h>

#include "policyChecks.h"
#include "PolicyObserver.h"

static apn_rule		*rule     = NULL;
static PolicyObserver	*observer = NULL;
static AlfFilterPolicy	*policy   = NULL;

static void
setup(void)
{
	AlfAppPolicy	*app;

	fail_if(rule     != NULL, "apn rule already exists @ setup().");
	fail_if(observer != NULL, "observer already exists @ setup().");
	fail_if(policy   != NULL, "Policy already exists @ setup().");

	app = new AlfAppPolicy(NULL, NULL);
	FAIL_IFZERO(app, "Couldn't create AlfAppPolicy.");

	rule = AlfFilterPolicy::createApnRule();
	FAIL_IFZERO(rule, "Couldn't create apn filter rule.");

	policy = new AlfFilterPolicy(app, rule);
	FAIL_IFZERO(policy, "Couldn't create AlfFilterPolicy.");

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

START_TEST(AlfFilterPolicy_getTypeIdentifier)
{
	if (!policy->getTypeIdentifier().IsSameAs(wxT("ALF"))) {
		fail("getTypeIdentifier() value not as expected: %ls",
		    policy->getTypeIdentifier().c_str());
	}
}
END_TEST

START_TEST(AlfFilterPolicy_LogNo)
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

START_TEST(AlfFilterPolicy_ActionNo)
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

START_TEST(AlfFilterPolicy_Protocol)
{
	wxString protocolName;

	/* Check initialization. */
	protocolName = wxT("tcp");
	CHECK_POLICY_GETPROTOCOLNO(policy, IPPROTO_TCP);
	CHECK_POLICY_GETPROTOCOLNAME(policy, protocolName);

	protocolName = wxT("udp");
	if (!policy->setProtocol(IPPROTO_UDP)) {
		fail("setProtocol(IPPROTO_UDP): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETPROTOCOLNO(policy, IPPROTO_UDP);
	CHECK_POLICY_GETPROTOCOLNAME(policy, protocolName);

	protocolName = wxT("tcp");
	if (!policy->setProtocol(IPPROTO_TCP)) {
		fail("setProtocol(IPPROTO_TCP): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETPROTOCOLNO(policy, IPPROTO_TCP);
	CHECK_POLICY_GETPROTOCOLNAME(policy, protocolName);

	protocolName = wxT("any");
	if (!policy->setProtocol(0)) {
		fail("setProtocol(0): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETPROTOCOLNO(policy, 0);
	CHECK_POLICY_GETPROTOCOLNAME(policy, protocolName);
}
END_TEST

START_TEST(AlfFilterPolicy_DirectionNo)
{
	wxString directionName;

	/* Check initialization. */
	directionName = wxT("connect");
	CHECK_POLICY_GETDIRECTIONNO(policy, APN_CONNECT);
	CHECK_POLICY_GETDIRECTIONNAME(policy, directionName);
	CHECK_POLICY_GETPROTOCOLNO(policy, IPPROTO_TCP);

	/*
	 * TCP
	 */
	if (!policy->setProtocol(IPPROTO_TCP)) {
		fail("Couldn't switch to tcp protocol.");
	}
	directionName = wxT("accept");
	if (!policy->setDirectionNo(APN_ACCEPT)) {
		fail("setDirectionNo(APN_ACCEPT): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETDIRECTIONNO(policy, APN_ACCEPT);
	CHECK_POLICY_GETDIRECTIONNAME(policy, directionName);
	CHECK_POLICY_GETPROTOCOLNO(policy, IPPROTO_TCP);

	directionName = wxT("connect");
	if (!policy->setDirectionNo(APN_CONNECT)) {
		fail("setDirectionNo(APN_CONNECT): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETDIRECTIONNO(policy, APN_CONNECT);
	CHECK_POLICY_GETDIRECTIONNAME(policy, directionName);
	CHECK_POLICY_GETPROTOCOLNO(policy, IPPROTO_TCP);

	directionName = wxT("both");
	if (!policy->setDirectionNo(APN_BOTH)) {
		fail("setDirectionNo(APN_BOTH): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETDIRECTIONNO(policy, APN_BOTH);
	CHECK_POLICY_GETDIRECTIONNAME(policy, directionName);
	CHECK_POLICY_GETPROTOCOLNO(policy, IPPROTO_TCP);

	/*
	 * UDP
	 */
	if (!policy->setProtocol(IPPROTO_UDP)) {
		fail("Couldn't switch to upd protocol.");
	}
	directionName = wxT("send");
	if (!policy->setDirectionNo(APN_SEND)) {
		fail("setDirectionNo(APN_SEND): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETDIRECTIONNO(policy, APN_SEND);
	CHECK_POLICY_GETDIRECTIONNAME(policy, directionName);
	CHECK_POLICY_GETPROTOCOLNO(policy, IPPROTO_UDP);

	directionName = wxT("receive");
	if (!policy->setDirectionNo(APN_RECEIVE)) {
		fail("setDirectionNo(APN_RECEIVE): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETDIRECTIONNO(policy, APN_RECEIVE);
	CHECK_POLICY_GETDIRECTIONNAME(policy, directionName);
	CHECK_POLICY_GETPROTOCOLNO(policy, IPPROTO_UDP);

	directionName = wxT("both");
	if (!policy->setDirectionNo(APN_BOTH)) {
		fail("setDirectionNo(APN_BOTH): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETDIRECTIONNO(policy, APN_BOTH);
	CHECK_POLICY_GETDIRECTIONNAME(policy, directionName);
	CHECK_POLICY_GETPROTOCOLNO(policy, IPPROTO_UDP);
}
END_TEST

START_TEST(AlfFilterPolicy_AddrFamilyNo)
{
	wxString addrFamilyName;

	/* Check initialization. */
	addrFamilyName = wxT("any");
	CHECK_POLICY_GETADDRFAMILYNO(policy, 0);
	CHECK_POLICY_GETADDRFAMILYNAME(policy, addrFamilyName);

	addrFamilyName = wxT("inet");
	if (!policy->setAddrFamilyNo(AF_INET)) {
		fail("setAddrFamilyNo(AF_INET): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETADDRFAMILYNO(policy, AF_INET);
	CHECK_POLICY_GETADDRFAMILYNAME(policy, addrFamilyName);

	addrFamilyName = wxT("inet6");
	if (!policy->setAddrFamilyNo(AF_INET6)) {
		fail("setAddrFamilyNo(AF_INET6): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETADDRFAMILYNO(policy, AF_INET6);
	CHECK_POLICY_GETADDRFAMILYNAME(policy, addrFamilyName);

	addrFamilyName = wxT("any");
	if (!policy->setAddrFamilyNo(0)) {
		fail("setAddrFamilyNo(0): not successful.");
	}
	CHECK_POLICY_MODIFIED(policy, true);
	CHECK_OBSERVER_NOTIFIED(observer, true);
	CHECK_POLICY_GETADDRFAMILYNO(policy, 0);
	CHECK_POLICY_GETADDRFAMILYNAME(policy, addrFamilyName);
}
END_TEST

TCase *
getTc_AlfFilterPolicy(void)
{
	TCase *testCase;

	testCase = tcase_create("AlfFilterPolicy");
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, AlfFilterPolicy_getTypeIdentifier);
	tcase_add_test(testCase, AlfFilterPolicy_LogNo);
	tcase_add_test(testCase, AlfFilterPolicy_ActionNo);
	tcase_add_test(testCase, AlfFilterPolicy_Protocol);
	tcase_add_test(testCase, AlfFilterPolicy_DirectionNo);
	tcase_add_test(testCase, AlfFilterPolicy_AddrFamilyNo);

	return (testCase);
}

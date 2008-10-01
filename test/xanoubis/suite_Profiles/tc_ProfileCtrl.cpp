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
#include <PolicyRuleSet.h>
#include <ProfileCtrl.h>

static ProfileCtrl	*pc = NULL;

static void
setup(void)
{
	fail_if(pc != NULL, "Previous ProfileCtrl was not removed.");
	pc = ProfileCtrl::getInstance();
	fail_if(pc == NULL, "Couldn't create new ProfileCtrl.");
}

static void
teardown(void)
{
	delete pc;
	pc = NULL;
}

static PolicyRuleSet *
createPolicyRuleSet(int priority)
{
	struct iovec		 iv;
	struct apn_ruleset	*rs;

	iv.iov_base = (void *)"\nalf {\n any {\n default deny\n }\n }\n ";
	iv.iov_len = strlen((char *)iv.iov_base) - 1;

	if (apn_parse_iovec("<iov>", &iv, 1, &rs, 0) != 0) {
		apn_print_errors(rs, stderr);
		fail("Couldn't create apn rule set.");
	}

	return (new PolicyRuleSet(priority, geteuid(), rs));
}

START_TEST(ProfileCtrl_create)
{
	if (pc != pc->getInstance()) {
		fail("Got different instance.");
	}

	if (pc != ProfileCtrl::getInstance()) {
		fail("Got different instance.");
	}
}
END_TEST

START_TEST(ProfileCtrl_getProfile)
{
	if (pc->getProfile() != ProfileMgr::PROFILE_NONE) {
		fail("Got wrong profile.");
	}
}
END_TEST

START_TEST(ProfileCtrl_getUserId)
{
	PolicyRuleSet	*rs;
	long		 id;
	bool		 result;

	rs = createPolicyRuleSet(1);
	fail_if(rs == NULL, "Couldn't create rule set.");

	id = pc->getUserId();
	fail_if(id != -1, "Found unexpeced user rule set.");

	id = pc->getUserId(ProfileMgr::PROFILE_NONE);
	fail_if(id != -1, "Found unexpeced user rule set.");

	result = pc->store(ProfileMgr::PROFILE_NONE, rs);
	fail_if(result == false, "Couldn't store rule set.");

	id = pc->getUserId();
	fail_if(id != rs->getId(), "Couldn't find user rule set.");

	id = pc->getUserId(ProfileMgr::PROFILE_NONE);
	fail_if(id != rs->getId(), "Couldn't find user rule set.");
}
END_TEST

START_TEST(ProfileCtrl_getAdminId)
{
	PolicyRuleSet	*rs;
	long		 id;
	bool		 result;

	rs = createPolicyRuleSet(0);
	fail_if(rs == NULL, "Couldn't create rule set.");

	id = pc->getAdminId(geteuid());
	fail_if(id != -1, "Found unexpeced user rule set.");

	id = pc->getAdminId(ProfileMgr::PROFILE_NONE, geteuid());
	fail_if(id != -1, "Found unexpeced user rule set.");

	result = pc->store(ProfileMgr::PROFILE_NONE, rs);
	fail_if(result == false, "Couldn't store rule set.");

	id = pc->getAdminId(geteuid());
	fail_if(id != rs->getId(), "Couldn't find user rule set.");

	id = pc->getAdminId(ProfileMgr::PROFILE_NONE, geteuid());
	fail_if(id != rs->getId(), "Couldn't find user rule set.");
}
END_TEST

START_TEST(ProfileCtrl_lockShow)
{
	PolicyRuleSet	*rs;
	void		*me;
	void		*you;
	bool		 result;

	me  = (void*)5;
	you = (void*)7;
	rs  = createPolicyRuleSet(0);
	fail_if(rs == NULL, "Couldn't create rule set.");
	fail_if(me == you, "Me and you are equal!");

	result = pc->lockToShow(0, NULL);
	fail_if(result == true, "Could lock non-existing rule set.");

	result = pc->lockToShow(0, me);
	fail_if(result == true, "Could lock non-existing rule set.");

	result = pc->lockToShow(rs->getId(), me);
	fail_if(result == true, "Could lock non-stored rule set.");

	result = pc->store(ProfileMgr::PROFILE_NONE, rs);
	fail_if(result == false, "Couldn't store rule set.");

	result = pc->lockToShow(rs->getId(), me);
	fail_if(result == false, "Couldn't lock rule set for me.");

	result = pc->lockToShow(rs->getId(), me);
	fail_if(result == false, "Couldn't lock rule set for me twice.");

	result = pc->lockToShow(rs->getId(), you);
	fail_if(result == false, "Couln't lock rule set for you.");
}
END_TEST

START_TEST(ProfileCtrl_unlockShow)
{
	PolicyRuleSet	*rs;
	void		*me;
	bool		 result;

	me = (void*)&ProfileCtrl_lockShow;
	rs = createPolicyRuleSet(0);
	fail_if(rs == NULL, "Couldn't create rule set.");

	result = pc->lockToShow(rs->getId(), me);
	fail_if(result == true, "Could lock non-stored rule set.");

	result = pc->store(ProfileMgr::PROFILE_NONE, rs);
	fail_if(result == false, "Couldn't store rule set.");

	result = pc->lockToShow(rs->getId(), me);
	fail_if(result == false, "Couldn't lock rule set.");

	result = pc->unlockFromShow(0, me);
	fail_if(result == true, "Could unlock unknown rule set.");

	result = pc->unlockFromShow(rs->getId(), (void*)6);
	fail_if(result == true, "Could unlock not owned rule set.");

	result = pc->unlockFromShow(rs->getId(), me);
	fail_if(result == false, "Couldn't unlock owned rule set.");

	result = pc->unlockFromShow(rs->getId(), me);
	fail_if(result == true, "Could unlock owned rule set twice.");
}
END_TEST


START_TEST(ProfileCtrl_store)
{
	PolicyRuleSet	*rs;
	bool		 result;

	rs = createPolicyRuleSet(1);

	result = pc->store(rs);
	fail_if(result != true, "Couldn't store rule set.");

	result = pc->lockToShow(rs->getId(), NULL);
	fail_if(result == false, "Couldn't lock rule set.");

	if (pc->getRuleSetToShow(rs->getId(), NULL) != rs) {
		fail("Couldn't get rule set.");
	}
}
END_TEST

START_TEST(ProfileCtrl_store_remap)
{
	PolicyRuleSet	*rs;
	long		 id;
	bool		 result;

	rs = createPolicyRuleSet(1);

	result = pc->store(ProfileMgr::PROFILE_NONE, rs);
	fail_if(result != true, "Couldn't store rule set.");

	id = pc->getUserId(ProfileMgr::PROFILE_NONE);
	fail_if(id == -1, "No rule set found.");
	fail_if(id != rs->getId(), "Found a different rule set.");

	result = pc->store(ProfileMgr::PROFILE_ADMIN, rs);
	fail_if(result != true, "Couldn't not remap rule set.");

	id = pc->getUserId(ProfileMgr::PROFILE_NONE);
	fail_if(id != -1, "Found rule set where no one should be.");
	id = pc->getUserId(ProfileMgr::PROFILE_ADMIN);
	fail_if(id != rs->getId(), "Found a different rule set.");
}
END_TEST

START_TEST(ProfileCtrl_store_displace)
{
	PolicyRuleSet	*rs1;
	PolicyRuleSet	*rs2;
	long		 id;
	bool		 result;

	rs1 = createPolicyRuleSet(1);
	rs2 = createPolicyRuleSet(1);
	fail_if(rs1 == NULL, "Couldn't create rule set 1.");
	fail_if(rs2 == NULL, "Couldn't create rule set 2.");

	result = pc->store(ProfileMgr::PROFILE_ADMIN, rs1);
	fail_if(result != true, "Couldn't store rule set 1");

	id = pc->getUserId(ProfileMgr::PROFILE_ADMIN);
	fail_if(id != rs1->getId(), "Found a different rule set.");
	id = pc->getUserId(ProfileMgr::PROFILE_NONE);
	fail_if(id != -1, "Found rule set where no one should be.");

	result = pc->store(ProfileMgr::PROFILE_ADMIN, rs2);
	fail_if(result != true, "Couldn't store rule set 2");

	id = pc->getUserId(ProfileMgr::PROFILE_ADMIN);
	fail_if(id != rs2->getId(), "Found different rule set (not rs2).");
	id = pc->getUserId(ProfileMgr::PROFILE_NONE);
	fail_if(id != rs1->getId(), "Found different rule set (not rs1).");
}
END_TEST

START_TEST(ProfileCtrl_store_multi_none)
{
	PolicyRuleSet	*rs1;
	PolicyRuleSet	*rs2;
	long		 id;
	bool		 result;

	rs1 = createPolicyRuleSet(1);
	rs2 = createPolicyRuleSet(1);
	fail_if(rs1 == NULL, "Couldn't create rule set 1.");
	fail_if(rs2 == NULL, "Couldn't create rule set 2.");

	result = pc->store(ProfileMgr::PROFILE_NONE, rs1);
	fail_if(result != true, "Couldn't store rule set 1");

	result = pc->lockToShow(rs1->getId(), NULL);
	fail_if(result == false, "Couldn't lock rule set 1.");

	id = pc->getUserId(ProfileMgr::PROFILE_NONE);
	fail_if(id != rs1->getId(), "Found a different rule set (not rs1).");

	result = pc->store(ProfileMgr::PROFILE_NONE, rs2);
	fail_if(result != true, "Couldn't store rule set 2");

	if (rs1 != pc->getRuleSetToShow(rs1->getId(), NULL)) {
		fail("Rule set 1 erased from list.");
	}

	result = pc->lockToShow(rs2->getId(), NULL);
	fail_if(result == false, "Couldn't lock rule set 2.");

	if (rs2 != pc->getRuleSetToShow(rs2->getId(), NULL)) {
		fail("Rule set 2 erased from list.");
	}
}
END_TEST

START_TEST(ProfileCtrl_store_multi_user_admin)
{
	PolicyRuleSet	*rs1;
	PolicyRuleSet	*rs2;
	long		 id;
	bool		 result;

	rs1 = createPolicyRuleSet(1);
	rs2 = createPolicyRuleSet(0);
	fail_if(rs1 == NULL, "Couldn't create rule set 1.");
	fail_if(rs2 == NULL, "Couldn't create rule set 2.");

	result = pc->store(ProfileMgr::PROFILE_HIGH, rs1);
	fail_if(result != true, "Couldn't store rule set 1");

	result = pc->store(ProfileMgr::PROFILE_HIGH, rs2);
	fail_if(result != true, "Couldn't store rule set 2");

	id = pc->getUserId(ProfileMgr::PROFILE_HIGH);
	fail_if(id != rs1->getId(), "Found a different rule set (not rs1).");

	id = pc->getAdminId(ProfileMgr::PROFILE_HIGH, geteuid());
	fail_if(id != rs2->getId(), "Found a different rule set (not rs2).");

	id = pc->getUserId(ProfileMgr::PROFILE_NONE);
	fail_if(id != -1, "Found rule set where no one should be.");
}
END_TEST

START_TEST(ProfileCtrl_getRuleSet_show)
{
	PolicyRuleSet	*rs;
	bool		 result;

	rs = createPolicyRuleSet(1);

	result = pc->store(rs);
	fail_if(result != true, "Couldn't store rule set.");

	if (pc->getRuleSetToShow(-1, NULL) != NULL) {
		fail("Could get unlocked and unknown rule set.");
	}

	if (pc->getRuleSetToShow(rs->getId(), NULL) != NULL) {
		fail("Could get unlocked rule set.");
	}

	result = pc->lockToShow(rs->getId(), NULL);
	fail_if(result == false, "Couldn't lock rule set.");

	if (pc->getRuleSetToShow(rs->getId(), NULL) != rs) {
		fail("Couldn't get rule set.");
	}
}
END_TEST


TCase *
getTc_ProfileCtrl(void)
{
	TCase *testCase;

	testCase = tcase_create("ProfileCtrl");
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, ProfileCtrl_create);
	tcase_add_test(testCase, ProfileCtrl_getProfile);
	tcase_add_test(testCase, ProfileCtrl_getUserId);
	tcase_add_test(testCase, ProfileCtrl_getAdminId);
	tcase_add_test(testCase, ProfileCtrl_lockShow);
	tcase_add_test(testCase, ProfileCtrl_unlockShow);
	tcase_add_test(testCase, ProfileCtrl_store);
	tcase_add_test(testCase, ProfileCtrl_store_remap);
	tcase_add_test(testCase, ProfileCtrl_store_displace);
	tcase_add_test(testCase, ProfileCtrl_store_multi_none);
	tcase_add_test(testCase, ProfileCtrl_store_multi_user_admin);
	tcase_add_test(testCase, ProfileCtrl_getRuleSet_show);

	return (testCase);
}

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
#include <ProfileMgr.h>
#include <wx/utils.h>

static ProfileMgr *pm = NULL;

static void
setup(void)
{
	fail_if(pm != NULL, "Previous ProfileMgr was not removed.");
	pm = new ProfileMgr();
	fail_if(pm == NULL, "Couldn't create new ProfileMgr.");
}

static void
teardown(void)
{
	delete pm;
	pm = NULL;
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

START_TEST(ProfileMgr_Profiles)
{
	if (pm->getProfile() != ProfileMgr::PROFILE_NONE) {
		fail("Profile not initialized propperly.");
	}

	pm->setProfile(ProfileMgr::PROFILE_ADMIN);
	if (pm->getProfile() != ProfileMgr::PROFILE_ADMIN) {
		fail("Couldn't set 'admin' profile.");
	}

	pm->setProfile(ProfileMgr::PROFILE_MEDIUM);
	if (pm->getProfile() != ProfileMgr::PROFILE_MEDIUM) {
		fail("Couldn't set 'medium' profile.");
	}

	pm->setProfile(ProfileMgr::PROFILE_HIGH);
	if (pm->getProfile() != ProfileMgr::PROFILE_HIGH) {
		fail("Couldn't set 'high' profile.");
	}

	pm->setProfile(ProfileMgr::PROFILE_NONE);
	if (pm->getProfile() != ProfileMgr::PROFILE_NONE) {
		fail("Couldn't set no profile.");
	}
}
END_TEST

START_TEST(ProfileMgr_store)
{
	PolicyRuleSet	*rs;
	bool		 result;

	rs = createPolicyRuleSet(1);

	result = pm->storeRuleSet(rs);
	fail_if(result != true, "Couldn't store rule set.");
	if (pm->getRuleSet(rs->getId()) != rs) {
		fail("Couldn't get rule set.");
	}
}
END_TEST

START_TEST(ProfileMgr_store_remap)
{
	PolicyRuleSet	*rs;
	long		 id;
	bool		 result;

	rs = createPolicyRuleSet(1);

	result = pm->storeRuleSet(ProfileMgr::PROFILE_NONE, rs);
	fail_if(result != true, "Couldn't store rule set.");

	id = pm->getUserRsId(ProfileMgr::PROFILE_NONE);
	fail_if(id == -1, "No rule set found.");
	fail_if(id != rs->getId(), "Found a different rule set.");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_ADMIN, rs);
	fail_if(result != true, "Couldn't not remap rule set.");

	id = pm->getUserRsId(ProfileMgr::PROFILE_NONE);
	fail_if(id != -1, "Found rule set where no one should be.");
	id = pm->getUserRsId(ProfileMgr::PROFILE_ADMIN);
	fail_if(id != rs->getId(), "Found a different rule set.");
}
END_TEST

START_TEST(ProfileMgr_store_displace)
{
	PolicyRuleSet	*rs1;
	PolicyRuleSet	*rs2;
	long		 id;
	bool		 result;

	rs1 = createPolicyRuleSet(1);
	rs2 = createPolicyRuleSet(1);
	fail_if(rs1 == NULL, "Couldn't create rule set 1.");
	fail_if(rs2 == NULL, "Couldn't create rule set 2.");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_ADMIN, rs1);
	fail_if(result != true, "Couldn't store rule set 1");

	id = pm->getUserRsId(ProfileMgr::PROFILE_ADMIN);
	fail_if(id != rs1->getId(), "Found a different rule set.");
	id = pm->getUserRsId(ProfileMgr::PROFILE_NONE);
	fail_if(id != -1, "Found rule set where no one should be.");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_ADMIN, rs2);
	fail_if(result != true, "Couldn't store rule set 2");

	id = pm->getUserRsId(ProfileMgr::PROFILE_ADMIN);
	fail_if(id != rs2->getId(), "Found different rule set (not rs2).");
	id = pm->getUserRsId(ProfileMgr::PROFILE_NONE);
	fail_if(id != rs1->getId(), "Found different rule set (not rs1).");
}
END_TEST

START_TEST(ProfileMgr_store_multi_none)
{
	PolicyRuleSet	*rs1;
	PolicyRuleSet	*rs2;
	long		 id;
	bool		 result;

	rs1 = createPolicyRuleSet(1);
	rs2 = createPolicyRuleSet(1);
	fail_if(rs1 == NULL, "Couldn't create rule set 1.");
	fail_if(rs2 == NULL, "Couldn't create rule set 2.");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_NONE, rs1);
	fail_if(result != true, "Couldn't store rule set 1");
	/* Protect rs1 from been erased by next store operation. */
	result = pm->acquireRuleSet(rs1->getId());
	fail_if(result == false, "Couldn't acquire rule set 1.");

	id = pm->getUserRsId(ProfileMgr::PROFILE_NONE);
	fail_if(id != rs1->getId(), "Found a different rule set (not rs1).");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_NONE, rs2);
	fail_if(result != true, "Couldn't store rule set 2");

	if (rs1 != pm->getRuleSet(rs1->getId())) {
		fail("Rule set 1 erased from list.");
	}
	if (rs2 != pm->getRuleSet(rs2->getId())) {
		fail("Rule set 2 erased from list.");
	}
}
END_TEST

START_TEST(ProfileMgr_store_multi_user_admin)
{
	PolicyRuleSet	*rs1;
	PolicyRuleSet	*rs2;
	long		 id;
	bool		 result;

	rs1 = createPolicyRuleSet(1);
	rs2 = createPolicyRuleSet(0);
	fail_if(rs1 == NULL, "Couldn't create rule set 1.");
	fail_if(rs2 == NULL, "Couldn't create rule set 2.");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_HIGH, rs1);
	fail_if(result != true, "Couldn't store rule set 1");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_HIGH, rs2);
	fail_if(result != true, "Couldn't store rule set 2");

	id = pm->getUserRsId(ProfileMgr::PROFILE_HIGH);
	fail_if(id != rs1->getId(), "Found a different rule set (not rs1).");

	id = pm->getAdminRsId(ProfileMgr::PROFILE_HIGH, geteuid());
	fail_if(id != rs2->getId(), "Found a different rule set (not rs2).");

	id = pm->getAdminRsId(ProfileMgr::PROFILE_HIGH, 33);
	fail_if(id != -1, "Found admin rule set for unknown user.");

	id = pm->getUserRsId(ProfileMgr::PROFILE_NONE);
	fail_if(id != -1, "Found rule set where no one should be.");
}
END_TEST

START_TEST(ProfileMgr_getAdminId)
{
	PolicyRuleSet	*rs;
	long		 id;
	bool		 result;

	rs = createPolicyRuleSet(0);
	fail_if(rs == NULL, "Couldn't create rule set.");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_NONE, rs);
	fail_if(result != true, "Couldn't store rule set.");

	id = pm->getAdminRsId(ProfileMgr::PROFILE_NONE, geteuid());
	fail_if(id != rs->getId(), "Couldn't find admin rule set.");

	id = pm->getAdminRsId(ProfileMgr::PROFILE_NONE, 33);
	fail_if(id != -1, "Found admin rule set for unknown user.");

	id = pm->getAdminRsId(ProfileMgr::PROFILE_HIGH, geteuid());
	fail_if(id != -1, "Found unexpeced admin rule set.");

	id = pm->getUserRsId(ProfileMgr::PROFILE_NONE);
	fail_if(id != -1, "Found unexpeced user rule set.");

	id = pm->getUserRsId();
	fail_if(id != -1, "Found unexpeced user rule set.");
}
END_TEST

START_TEST(ProfileMgr_getUserId)
{
	PolicyRuleSet	*rs;
	long		 id;
	bool		 result;

	rs = createPolicyRuleSet(1);
	fail_if(rs == NULL, "Couldn't create rule set.");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_NONE, rs);
	fail_if(result != true, "Couldn't store rule set.");

	id = pm->getUserRsId(ProfileMgr::PROFILE_NONE);
	fail_if(id != rs->getId(), "Couldn't find user rule set.");

	id = pm->getUserRsId(ProfileMgr::PROFILE_HIGH);
	fail_if(id != -1, "Found unexpeced user rule set.");

	id = pm->getAdminRsId(ProfileMgr::PROFILE_NONE, geteuid());
	fail_if(id != -1, "Found unexpeced admin rule set.");

	id = pm->getAdminRsId(geteuid());
	fail_if(id != -1, "Found unexpeced admin rule set.");
}
END_TEST

START_TEST(ProfileMgr_getRuleSet)
{
	PolicyRuleSet	*rs;
	bool		 result;

	rs = createPolicyRuleSet(1);
	fail_if(rs == NULL, "Couldn't create rule set.");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_NONE, rs);
	fail_if(result != true, "Couldn't store rule set.");

	if (pm->getRuleSet(rs->getId()) != rs) {
		fail("Couldn't get rule set.");
	}
	if (pm->getRuleSet(-1) != NULL) {
		fail("Got unknown rule set.");
	}
}
END_TEST

START_TEST(ProfileMgr_acquire)
{
	PolicyRuleSet	*rs;
	bool		 result;

	rs = createPolicyRuleSet(1);
	fail_if(rs == NULL, "Couldn't create rule set.");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_NONE, rs);
	fail_if(result != true, "Couldn't store rule set.");

	result = pm->acquireRuleSet(rs->getId());
	fail_if(result != true, "Couldn't acquire rule set.");

	result = pm->acquireRuleSet(rs->getId());
	fail_if(result != true, "Couldn't acquire rule set twice.");

	result = pm->acquireRuleSet(-1);
	fail_if(result != false, "Could acquire unknown rule set.");

}
END_TEST

START_TEST(ProfileMgr_release)
{
	PolicyRuleSet	*rs;
	bool		 result;

	rs = createPolicyRuleSet(1);
	fail_if(rs == NULL, "Couldn't create rule set.");

	result = pm->storeRuleSet(ProfileMgr::PROFILE_NONE, rs);
	fail_if(result != true, "Couldn't store rule set.");

	result = pm->acquireRuleSet(rs->getId());
	fail_if(result != true, "Couldn't acquire rule set.");

	result = pm->releaseRuleSet(rs->getId());
	fail_if(result != true, "Couldn't release rule set.");

	result = pm->releaseRuleSet(rs->getId());
	fail_if(result != true, "Couldn't release rule set twice.");

	result = pm->releaseRuleSet(-1);
	fail_if(result != false, "Could release unknown rule set.");
}
END_TEST

TCase *
getTc_ProfileMgr(void)
{
	TCase *testCase;

	testCase = tcase_create("ProfileMgr");
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, ProfileMgr_Profiles);
	tcase_add_test(testCase, ProfileMgr_store);
	tcase_add_test(testCase, ProfileMgr_store_remap);
	tcase_add_test(testCase, ProfileMgr_store_displace);
	tcase_add_test(testCase, ProfileMgr_store_multi_none);
	tcase_add_test(testCase, ProfileMgr_store_multi_user_admin);
	tcase_add_test(testCase, ProfileMgr_getAdminId);
	tcase_add_test(testCase, ProfileMgr_getUserId);
	tcase_add_test(testCase, ProfileMgr_getRuleSet);
	tcase_add_test(testCase, ProfileMgr_acquire);
	tcase_add_test(testCase, ProfileMgr_release);

	return (testCase);
}

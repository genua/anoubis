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

#include <sys/types.h>
#include <sys/wait.h>

#include <wx/app.h>
#include <wx/apptrait.h>
#include <wx/stdpaths.h>

#include <check.h>

#include <PolicyRuleSet.h>
#include <PolicyCtrl.h>

class tc_PolicyCtrl_AppTraits : public wxGUIAppTraits
{
	public:
		tc_PolicyCtrl_AppTraits(const wxString &prefix)
		{
			paths.SetInstallPrefix(prefix);
		}

		virtual wxStandardPaths &GetStandardPaths()
		{
			return (paths);
		}

	private:
		wxStandardPaths paths;
};

class tc_PolicyCtrl_App : public wxApp
{
	public:
		tc_PolicyCtrl_App(const wxString &prefix)
		{
			this->prefix = prefix;
			SetAppName(wxT("tc_PolicyCtrl"));
		}

		virtual wxAppTraits *CreateTraits()
		{
			return new tc_PolicyCtrl_AppTraits(prefix);
		}

	private:
		wxString prefix;
};

static tc_PolicyCtrl_App	*tc_App;
static PolicyCtrl		*pc = NULL;
static char			tmp_home[64];
static PolicyRuleSet		*userPolicy;
static PolicyRuleSet		*adminPolicy;

static int
make_dir(const char *dir, ...)
{
	va_list ap;
	char *result_dir, *cmd;
	int result;

	va_start(ap, dir);
	result = vasprintf(&result_dir, dir, ap);
	va_end(ap);

	if (result == -1)
		return (0);

	result = asprintf(&cmd, "mkdir -p \"%s\"", result_dir);
	if (result == -1) {
		free(result_dir);
		return (0);
	}

	result = system(cmd);

	free(cmd);
	free(result_dir);

	return (WIFEXITED(result) && (WEXITSTATUS(result) == 0));
}

static int
touch_file(const char *path, ...)
{
	va_list ap;
	char *file;
	int result;

	va_start(ap, path);
	result = vasprintf(&file, path, ap);
	va_end(ap);

	if (result == -1)
		return (0);

	FILE *f = fopen(file, "w");
	if (f == NULL) {
		free(file);
		return (0);
	}

	fprintf(f, "\nalf {\n any {\n default deny\n }\n }\n");
	fflush(f);
	fclose(f);

	if (chmod(file, S_IRUSR) == -1)
		perror(path);

	free(file);

	return (1);
}

static int
remove_dir(const char *dir, ...)
{
	va_list ap;
	char *path, *cmd;
	int result;

	va_start(ap, dir);
	result = vasprintf(&path, dir, ap);
	va_end(ap);

	if (result == -1)
		return (0);

	result = asprintf(&cmd, "rm -rf \"%s\"", path);
	if (result == -1) {
		free(path);
		return (0);
	}

	result = system(cmd);

	free(cmd);
	free(path);

	return (WIFEXITED(result) && (WEXITSTATUS(result) == 0));
}

static PolicyRuleSet *
createPolicyRuleSet(int priority, uid_t uid)
{
	struct iovec		 iv;
	struct apn_ruleset	*rs;

	iv.iov_base = (void *)"\nalf {\n any {\n default deny\n }\n }\n ";
	iv.iov_len = strlen((char *)iv.iov_base) - 1;

	if (apn_parse_iovec("<iov>", &iv, 1, &rs, 0) != 0) {
		apn_print_errors(rs, stderr);
		fail("Couldn't create apn rule set.");
	}

	return (new PolicyRuleSet(priority, uid, rs));
}

static void
setup(void)
{
	char *s;

	strcpy(tmp_home, "/tmp/tc_PolicyCtrl_XXXXXX");
	s = mkdtemp(tmp_home);
	setenv("HOME", tmp_home, 1);

	tc_App = new tc_PolicyCtrl_App(wxString(tmp_home, wxConvFile));
	wxApp::SetInstance(tc_App);

	make_dir("%s/share/tc_PolicyCtrl/profiles", tmp_home);
	make_dir("%s/.tc_PolicyCtrl/profiles", tmp_home);

	touch_file("%s/share/tc_PolicyCtrl/profiles/default1", tmp_home);
	touch_file("%s/share/tc_PolicyCtrl/profiles/default2", tmp_home);
	touch_file("%s/share/tc_PolicyCtrl/profiles/default3", tmp_home);
	touch_file("%s/share/tc_PolicyCtrl/profiles/default4", tmp_home);

	touch_file("%s/.tc_PolicyCtrl/profiles/user1", tmp_home);
	touch_file("%s/.tc_PolicyCtrl/profiles/user2", tmp_home);
	touch_file("%s/.tc_PolicyCtrl/profiles/user3", tmp_home);
	touch_file("%s/.tc_PolicyCtrl/profiles/user4", tmp_home);
	touch_file("%s/.tc_PolicyCtrl/profiles/user5", tmp_home);
	touch_file("%s/.tc_PolicyCtrl/profiles/user6", tmp_home);

	fail_if(pc != NULL, "Previous PolicyCtrl was not removed.");
	pc = PolicyCtrl::getInstance();
	fail_if(pc == NULL, "Couldn't create new PolicyCtrl.");
	pc->setEventBroadcastEnabled(false);

	userPolicy = createPolicyRuleSet(1, geteuid());
	adminPolicy = createPolicyRuleSet(0, geteuid());

	pc->importPolicy(userPolicy);
	pc->importPolicy(adminPolicy);


}

static void
teardown(void)
{
	remove_dir(tmp_home);

	wxApp::SetInstance(0);
	delete tc_App;

	delete pc;
	pc = NULL;
}

int
testPolicyCtrl(PolicyCtrl *ctrl, int flags)
{
	if (flags == 0)
		return (ctrl->gcRuleSetList_.size());
	else {
		/* Open for other checks */
		return (0);
	}
}

START_TEST(PolicyCtrl_create)
{
	if (pc != pc->getInstance()) {
		fail("Got different instance.");
	}

	if (pc != PolicyCtrl::getInstance()) {
		fail("Got different instance.");
	}
}
END_TEST

START_TEST(PolicyCtrl_getUserId)
{
	long id = pc->getUserId();
	fail_unless(id == userPolicy->getRuleSetId(),
	    "Fetched wrong user-id\n"
	    "Is: %i\n"
	    "Expected: %i",
	    id, userPolicy->getRuleSetId());

	PolicyRuleSet *rs = pc->getRuleSet(id);
	fail_unless(rs == userPolicy, "Wrong user-policy fetched!");
}
END_TEST

START_TEST(PolicyCtrl_getAdminId)
{
	long id = pc->getAdminId(geteuid());
	fail_unless(id == adminPolicy->getRuleSetId(),
	    "Fetched wrong admin-id\n"
	    "Is: %i\n"
	    "Expected: %i",
	    id, adminPolicy->getRuleSetId());

	PolicyRuleSet *rs = pc->getRuleSet(id);
	fail_unless(rs == adminPolicy, "Wrong user-policy fetched!");
}
END_TEST

START_TEST(PolicyCtrl_getUnknownAdminId)
{
	long id = pc->getAdminId(0);
	fail_unless(id == -1,
	    "Fetched wrong admin-id\n"
	    "Is: %i\n"
	    "Expected: -1", id);

	PolicyRuleSet *rs = pc->getRuleSet(id);
	fail_unless(rs == 0, "Wrong user-policy fetched!");
}
END_TEST

START_TEST(PolicyCtrl_ProfileList)
{
	wxArrayString result;

	for (int i = 0; i < pc->getSize(); i++)
		result.Add(pc->getProfile(i)->getProfileName());
	fail_unless(result.GetCount() == 10,
	    "Unexpected # of profiles\n"
	    "Is: %i\n"
	    "Expected: 10", result.GetCount());

	fail_if(result.Index(wxT("default1")) == wxNOT_FOUND,
	   "Profile \"default1\" not in list");
	fail_if(result.Index(wxT("default2")) == wxNOT_FOUND,
	   "Profile \"default2\" not in list");
	fail_if(result.Index(wxT("default3")) == wxNOT_FOUND,
	   "Profile \"default3\" not in list");
	fail_if(result.Index(wxT("default4")) == wxNOT_FOUND,
	   "Profile \"default4\" not in list");
	fail_if(result.Index(wxT("user1")) == wxNOT_FOUND,
	   "Profile \"user1\" not in list");
	fail_if(result.Index(wxT("user2")) == wxNOT_FOUND,
	   "Profile \"user2\" not in list");
	fail_if(result.Index(wxT("user3")) == wxNOT_FOUND,
	   "Profile \"user3\" not in list");
	fail_if(result.Index(wxT("user4")) == wxNOT_FOUND,
	   "Profile \"user4\" not in list");
	fail_if(result.Index(wxT("user5")) == wxNOT_FOUND,
	   "Profile \"user5\" not in list");
	fail_if(result.Index(wxT("user6")) == wxNOT_FOUND,
	   "Profile \"user6\" not in list");
}
END_TEST

START_TEST(PolicyCtrl_ProfileListUserRemoveProfiles)
{
	pc->removeProfile(wxT("user1"));
	pc->removeProfile(wxT("user3"));
	pc->removeProfile(wxT("user4"));
	pc->removeProfile(wxT("user5"));
	pc->removeProfile(wxT("user6"));

	remove_dir("%s/.tc_PolicyCtrl/profiles", tmp_home);

	wxArrayString result;
	for (int i = 0; i < pc->getSize(); i++)
		result.Add(pc->getProfile(i)->getProfileName());
	fail_unless(result.GetCount() == 5,
	    "Unexpected # of profiles\n"
	    "Is: %i\n"
	    "Expected: 5", result.GetCount());

	fail_if(result.Index(wxT("default1")) == wxNOT_FOUND,
	   "Profile \"default1\" not in list");
	fail_if(result.Index(wxT("default2")) == wxNOT_FOUND,
	   "Profile \"default2\" not in list");
	fail_if(result.Index(wxT("default3")) == wxNOT_FOUND,
	   "Profile \"default3\" not in list");
	fail_if(result.Index(wxT("default4")) == wxNOT_FOUND,
	   "Profile \"default4\" not in list");
	fail_if(result.Index(wxT("user2")) == wxNOT_FOUND,
	   "Profile \"user2\" not in list");
}
END_TEST

START_TEST(PolicyCtrl_haveDefaultProfile)
{
	bool result = pc->haveProfile(wxT("default2"));
	fail_unless(result == true, "Profile \"default2\" not found");
}
END_TEST

START_TEST(PolicyCtrl_haveUserProfile)
{
	bool result = pc->haveProfile(wxT("user4"));
	fail_unless(result == true, "Profile \"user4\" not found");
}
END_TEST

START_TEST(PolicyCtrl_haveNoProfile)
{
	bool result = pc->haveProfile(wxT("foobar"));
	fail_unless(result == false, "Profile \"foobar\" exists");
}
END_TEST

START_TEST(PolicyCtrl_isWritableUser)
{
	bool result = pc->isProfileWritable(wxT("user5"));
	fail_unless(result == true, "Profile \"user5\" is not writable.");
}
END_TEST

START_TEST(PolicyCtrl_isWritableDefault)
{
	bool result = pc->isProfileWritable(wxT("default1"));
	fail_unless(result == false, "Profile \"default1\" is writable.");
}
END_TEST

START_TEST(PolicyCtrl_isWritableNoSuchFile)
{
	bool result = pc->isProfileWritable(wxT("foobar"));
	fail_unless(result == true, "Profile \"foobar\" is writable.");
}
END_TEST

START_TEST(PolicyCtrl_removeUserProfile)
{
	bool result;

	result = pc->removeProfile(wxT("user1"));
	fail_unless(result == true, "Failed to remove \"user1\"");

	result = pc->haveProfile(wxT("user1"));
	fail_unless(result == false, "Profile \"user1\" still exists.");

	wxArrayString profileList;
	for (int i = 0; i < pc->getSize(); i++)
		profileList.Add(pc->getProfile(i)->getProfileName());
	fail_unless(profileList.GetCount() == 9,
	    "Unexpected # of profiles\n"
	    "Is: %i\n"
	    "Expected: 9", profileList.GetCount());

	result = (profileList.Index(wxT("user1")) == wxNOT_FOUND);
	fail_unless(result == true,
	    "Profile \"user1\" still in profile-list.");
}
END_TEST

START_TEST(PolicyCtrl_removeDefaultProfile)
{
	bool result;

	result = pc->removeProfile(wxT("default1"));
	fail_unless(result == false,
	    "Removing of \"default1\" was successful.");

	result = pc->haveProfile(wxT("default1"));
	fail_unless(result == true, "No such profile: \"defaull1\"");

	wxArrayString profileList;
	for (int i = 0; i < pc->getSize(); i++)
		profileList.Add(pc->getProfile(i)->getProfileName());
	fail_unless(profileList.GetCount() == 10,
	    "Unexpected # of profiles\n"
	    "Is: %i\n"
	    "Expected: 10", profileList.GetCount());

	result = (profileList.Index(wxT("default1")) == wxNOT_FOUND);
	fail_unless(result == false,
	    "Profile \"default1\" not in profile-list.");
}
END_TEST

START_TEST(PolicyCtrl_removeUnknownProfile)
{
	bool result = pc->removeProfile(wxT("foobar"));
	fail_unless(result == false,
	    "Removing of unknown profile \"foobar\" was successful.");

	wxArrayString profileList;
	for (int i = 0; i < pc->getSize(); i++)
		profileList.Add(pc->getProfile(i)->getProfileName());
	fail_unless(profileList.GetCount() == 10,
	    "Unexpected # of profiles\n"
	    "Is: %i\n"
	    "Expected: 10", profileList.GetCount());
}
END_TEST

START_TEST(PolicyCtrl_getUserPolicy)
{
	long id = pc->getUserId();
	fail_unless(id == userPolicy->getRuleSetId(),
	    "Wrong id policy-id fetched\n"
	    "Is: %i\n"
	    "Expected: %i", id, userPolicy->getRuleSetId());

	PolicyRuleSet *rs = pc->getRuleSet(id);
	fail_unless(rs == userPolicy, "Wrong policy fetched");
}
END_TEST

START_TEST(PolicyCtrl_getAdminPolicy)
{
	long id = pc->getAdminId(geteuid());
	fail_unless(id == adminPolicy->getRuleSetId(),
	    "Wrong id policy-id fetched\n"
	    "Is: %i\n"
	    "Expected: %i", id, userPolicy->getRuleSetId());

	PolicyRuleSet *rs = pc->getRuleSet(id);
	fail_unless(rs == adminPolicy, "Wrong policy fetched");
}
END_TEST

START_TEST(PolicyCtrl_getAdminPolicyWrongUid)
{
	long id = pc->getAdminId(geteuid() + 1);
	fail_unless(id == -1,
	    "Wrong id policy-id fetched\n"
	    "Is: %i\n"
	    "Expected: -1");
}
END_TEST

START_TEST(PolicyCtrl_getRuleSetWrongId)
{
	long id = userPolicy->getRuleSetId() + adminPolicy->getRuleSetId();
	PolicyRuleSet *rs = pc->getRuleSet(id);
	fail_unless(rs == 0, "Policy fetch where nothing is expected");
}
END_TEST

START_TEST(PolicyCtrl_getRuleSetGarbage)
{
	long oldId = pc->getUserId();

	/*
	 * Lock the ruleset, let it move to the garbage-list after importing
	 * another policy.
	 */
	PolicyRuleSet *oldRs = pc->getRuleSet(oldId);
	oldRs->lock();

	bool result = pc->importFromProfile(wxT("user1"));
	fail_unless(result == true, "Import from \"user1\" failed");

	long newId = pc->getUserId();

	fail_unless(oldId != newId, "Id of user-policy has not changed.\n"
	    "OldId = %i\n"
	    "NewId = %i", oldId, newId);

	oldRs = pc->getRuleSet(oldId);
	PolicyRuleSet *newRs = pc->getRuleSet(newId);

	fail_unless(oldRs != 0, "No policy for OldId = %i fetched!", oldId);
	fail_unless(newRs != 0, "No policy for NewId = %i fetched!", newId);
	fail_unless(oldRs != newRs,
	    "Same policy fetched for OldId = %i, NewId = %i",
	    oldId, newId);
}
END_TEST

START_TEST(PolicyCtrl_importNoProfile)
{
	bool result = pc->importFromProfile(wxT("foobar"));
	fail_unless(result == false, "Import succeeded from unknown profile");
}
END_TEST

START_TEST(PolicyCtrl_importUserProfile)
{
	int oldId = userPolicy->getRuleSetId();
	bool result = pc->importFromProfile(wxT("user5"));
	fail_unless(result == true, "Import from \"user5\" failed");

	int id = pc->getUserId();
	fail_unless(id != oldId,
	    "User-policy is still the same.");
}
END_TEST

START_TEST(PolicyCtrl_importDefaultProfile)
{
	int oldId = userPolicy->getRuleSetId();
	bool result = pc->importFromProfile(wxT("default2"));
	fail_unless(result == true, "Import from \"default2\" failed");

	int id = pc->getUserId();
	fail_unless(id != oldId,
	    "User-policy is still the same.");
}
END_TEST

START_TEST(PolicyCtrl_exportNoProfile)
{
	bool result = pc->importFromProfile(wxT("foobar"));
	fail_unless(result == false, "Import from \"foobar\" was successful");
}
END_TEST

START_TEST(PolicyCtrl_exportUserProfile)
{
	bool result = pc->exportToProfile(wxT("user1"));
	fail_unless(result == true, "Export to \"user1\" failed");
}
END_TEST

START_TEST(PolicyCtrl_exportDefaultProfile)
{
	bool result = pc->exportToProfile(wxT("default1"));
	fail_unless(result == false, "Export to \"default1\" was successful");
}
END_TEST

START_TEST(PolicyCtrl_exportNewProfile)
{
	bool export_result = pc->exportToProfile(wxT("foobar"));
	fail_unless(export_result == true, "Export to \"foobar\" failed.");

	wxArrayString profiles;
	for (int i = 0; i < pc->getSize(); i++)
		profiles.Add(pc->getProfile(i)->getProfileName());
	fail_unless(profiles.GetCount() == 11,
	    "Unexpected # of profiles\n"
	    "Is: %i\n"
	    "Expected: 11", profiles.GetCount());

	fail_if(profiles.Index(wxT("default1")) == wxNOT_FOUND,
	   "Profile \"default1\" not in list");
	fail_if(profiles.Index(wxT("default2")) == wxNOT_FOUND,
	   "Profile \"default2\" not in list");
	fail_if(profiles.Index(wxT("default3")) == wxNOT_FOUND,
	   "Profile \"default3\" not in list");
	fail_if(profiles.Index(wxT("default4")) == wxNOT_FOUND,
	   "Profile \"default4\" not in list");
	fail_if(profiles.Index(wxT("user1")) == wxNOT_FOUND,
	   "Profile \"user1\" not in list");
	fail_if(profiles.Index(wxT("user2")) == wxNOT_FOUND,
	   "Profile \"user2\" not in list");
	fail_if(profiles.Index(wxT("user3")) == wxNOT_FOUND,
	   "Profile \"user3\" not in list");
	fail_if(profiles.Index(wxT("user4")) == wxNOT_FOUND,
	   "Profile \"user4\" not in list");
	fail_if(profiles.Index(wxT("user5")) == wxNOT_FOUND,
	   "Profile \"user5\" not in list");
	fail_if(profiles.Index(wxT("user6")) == wxNOT_FOUND,
	   "Profile \"user6\" not in list");
	fail_if(profiles.Index(wxT("foobar")) == wxNOT_FOUND,
	   "Profile \"foobar\" not in list");
}
END_TEST

START_TEST(PolicyCtrl_getUserRuleSet)
{
	PolicyRuleSet *rs = pc->getRuleSet(wxT("user2"));
	fail_unless(rs != 0, "Failed to fetch policy of \"user2\"");

	delete rs;
}
END_TEST

START_TEST(PolicyCtrl_getAdminRuleSet)
{
	PolicyRuleSet *rs = pc->getRuleSet(wxT("default2"));
	fail_unless(rs != 0, "Failed to fetch policy of \"default2\"");

	delete rs;
}
END_TEST

START_TEST(PolicyCtrl_getUnknownRuleSet)
{
	PolicyRuleSet *rs = pc->getRuleSet(wxT("foobar"));
	fail_unless(rs == 0, "Failed to fetch policy of \"foobar\"");

	delete rs;
}
END_TEST

START_TEST(PolicyCtrl_MoveGcListNotLocked)
{
	long oldId = pc->getUserId();
	PolicyRuleSet *oldRs = pc->getRuleSet(oldId);
	fail_unless(oldRs != 0, "Failed to fetch policy");

	fail_unless(pc->importFromProfile(wxT("default2")),
	    "Failed to import another policy");

	long newId = pc->getUserId();
	fail_unless(oldId != newId, "Policy has not changed");

	PolicyRuleSet *newRs = pc->getRuleSet(newId);
	fail_unless(newRs != oldRs, "Policy has not changed");

	/* oldRs is not locked and thus destroyed */
	fail_unless(testPolicyCtrl(pc, 0) == 0, "Gargabe list is not empty");
}
END_TEST

START_TEST(PolicyCtrl_MoveGcListLocked)
{
	long oldId = pc->getUserId();
	PolicyRuleSet *oldRs = pc->getRuleSet(oldId);
	fail_unless(oldRs != 0, "Failed to fetch policy");

	oldRs->lock();

	fail_unless(pc->importFromProfile(wxT("default2")),
	    "Failed to import another policy");

	long newId = pc->getUserId();
	fail_unless(oldId != newId, "Policy has not changed");

	PolicyRuleSet *newRs = pc->getRuleSet(newId);
	fail_unless(newRs != oldRs, "Policy has not changed");

	/* oldRs is locked and thus moved to garbage-list */
	fail_unless(testPolicyCtrl(pc, 0) == 1, "Gargabe list is empty");
}
END_TEST

TCase *
getTc_PolicyCtrl(void)
{
	TCase *testCase;

	testCase = tcase_create("PolicyCtrl");
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, PolicyCtrl_create);
	tcase_add_test(testCase, PolicyCtrl_getUserId);
	tcase_add_test(testCase, PolicyCtrl_getAdminId);
	tcase_add_test(testCase, PolicyCtrl_getUnknownAdminId);
	tcase_add_test(testCase, PolicyCtrl_ProfileList);
	tcase_add_test(testCase, PolicyCtrl_ProfileListUserRemoveProfiles);
	tcase_add_test(testCase, PolicyCtrl_haveDefaultProfile);
	tcase_add_test(testCase, PolicyCtrl_haveUserProfile);
	tcase_add_test(testCase, PolicyCtrl_haveNoProfile);
	tcase_add_test(testCase, PolicyCtrl_isWritableUser);
	tcase_add_test(testCase, PolicyCtrl_isWritableDefault);
	tcase_add_test(testCase, PolicyCtrl_isWritableNoSuchFile);
	tcase_add_test(testCase, PolicyCtrl_removeUserProfile);
	tcase_add_test(testCase, PolicyCtrl_removeDefaultProfile);
	tcase_add_test(testCase, PolicyCtrl_removeUnknownProfile);
	tcase_add_test(testCase, PolicyCtrl_getUserPolicy);
	tcase_add_test(testCase, PolicyCtrl_getAdminPolicy);
	tcase_add_test(testCase, PolicyCtrl_getAdminPolicyWrongUid);
	tcase_add_test(testCase, PolicyCtrl_getRuleSetWrongId);
	tcase_add_test(testCase, PolicyCtrl_getRuleSetGarbage);
	tcase_add_test(testCase, PolicyCtrl_importNoProfile);
	tcase_add_test(testCase, PolicyCtrl_importUserProfile);
	tcase_add_test(testCase, PolicyCtrl_importDefaultProfile);
	tcase_add_test(testCase, PolicyCtrl_exportNewProfile);
	tcase_add_test(testCase, PolicyCtrl_exportNoProfile);
	tcase_add_test(testCase, PolicyCtrl_exportUserProfile);
	tcase_add_test(testCase, PolicyCtrl_exportDefaultProfile);
	tcase_add_test(testCase, PolicyCtrl_getUserRuleSet);
	tcase_add_test(testCase, PolicyCtrl_getAdminRuleSet);
	tcase_add_test(testCase, PolicyCtrl_getUnknownRuleSet);
	tcase_add_test(testCase, PolicyCtrl_MoveGcListNotLocked);
	tcase_add_test(testCase, PolicyCtrl_MoveGcListLocked);

	return (testCase);
}

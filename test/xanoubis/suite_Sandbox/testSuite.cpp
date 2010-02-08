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

#include <check.h>
#include <errno.h>

#include <wx/app.h>
#include <wx/apptrait.h>
#include <wx/stdpaths.h>

#include <SbModel.h>
#include <SbModelRowProvider.h>

class SandboxAppTraits : public wxGUIAppTraits
{
	public:
		SandboxAppTraits(const wxString &prefix)
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


class SandboxApp : public wxApp
{
	public:
		SandboxApp(const wxString &prefix)
		{
			this->prefix = prefix;
			SetAppName(wxT("SandboxApp"));
		}

		virtual wxAppTraits *CreateTraits()
		{
			return new SandboxAppTraits(prefix);
		}

	private:
		wxString prefix;
};

static SandboxApp	*app;
static char		tmp_prefix[PATH_MAX];

static void
setup(void)
{
	char path[PATH_MAX];
	char *s;

	strcpy(tmp_prefix, "/tmp/tc_Sandbox_XXXXXX");
	s = mkdtemp(tmp_prefix);
	setenv("HOME", tmp_prefix, 1);

	snprintf(path, sizeof(path), "%s/share", tmp_prefix);
	fail_unless(mkdir(path, S_IRWXU) == 0,
	    "Failed to create %s: %s", path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share/SandboxApp", tmp_prefix);
	    fail_unless(mkdir(path, S_IRWXU) == 0,
	    "Failed to create %s: %s", path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share/SandboxApp/profiles",
	    tmp_prefix);
	fail_unless(mkdir(path, S_IRWXU) == 0,
	    "Failed to create %s: %s", path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share/SandboxApp/profiles/wizard",
	    tmp_prefix);
	fail_unless(mkdir(path, S_IRWXU) == 0,
	    "Failed to create %s: %s", path, strerror(errno));

	snprintf(path, sizeof(path),
	    "%s/share/SandboxApp/profiles/wizard/sandbox", tmp_prefix);

	FILE *f = fopen(path, "w");
	fail_unless(f != 0);
	fail_unless(fwrite("r r\n", 4, 1, f) == 1);
	fail_unless(fwrite("w w\n", 4, 1, f) == 1);
	fail_unless(fwrite("x x\n", 4, 1, f) == 1);
	fail_unless(fwrite("rw rw\n", 6, 1, f) == 1);
	fail_unless(fwrite("rx rx\n", 6, 1, f) == 1);
	fail_unless(fwrite("wx wx\n", 6, 1, f) == 1);
	fail_unless(fwrite("rwx rwx\n", 8, 1, f) == 1);
	fclose(f);

	app = new SandboxApp(wxString(tmp_prefix, wxConvFile));
	wxApp::SetInstance(app);
}

static void
teardown(void)
{
	char path[PATH_MAX];

	wxApp::SetInstance(0);
	delete app;

	snprintf(path, sizeof(path),
	    "%s/share/SandboxApp/profiles/wizard/sandbox", tmp_prefix);
	unlink(path); /* Ignore error, file can be removed by a test */

	snprintf(path, sizeof(path), "%s/share/SandboxApp/profiles/wizard",
	    tmp_prefix);
	fail_unless(rmdir(path) == 0, "Failed to remove %s: %s",
	    path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share/SandboxApp/profiles",
	    tmp_prefix);
	fail_unless(rmdir(path) == 0, "Failed to remove %s: %s",
	    path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share/SandboxApp", tmp_prefix);
	fail_unless(rmdir(path) == 0, "Failed to remove %s: %s",
	    path, strerror(errno));

	snprintf(path, sizeof(path), "%s/share", tmp_prefix);
	fail_unless(rmdir(path) == 0, "Failed to remove %s: %s",
	    path, strerror(errno));

	fail_unless(rmdir(tmp_prefix) == 0, "Failed to remove %s: %s",
	    tmp_prefix, strerror(errno));
}

START_TEST(empty)
{
	SbModel model;
	SbModelRowProvider r_provider(&model, SbEntry::READ);
	SbModelRowProvider w_provider(&model, SbEntry::WRITE);
	SbModelRowProvider x_provider(&model, SbEntry::EXECUTE);

	fail_unless(model.getEntryCount() == 0);
	fail_unless(model.getEntryCount(SbEntry::READ) == 0);
	fail_unless(model.getEntryCount(SbEntry::WRITE) == 0);
	fail_unless(model.getEntryCount(SbEntry::EXECUTE) == 0);
	fail_unless(model.getEntry(0) == 0);
	fail_unless(model.getEntry(wxT("foo")) == 0);
	fail_unless(model.getEntry(wxT("foo"), false) == 0);

	fail_unless(r_provider.getSize() == 0);
	fail_unless(r_provider.getRow(0) == 0);
	fail_unless(w_provider.getSize() == 0);
	fail_unless(w_provider.getRow(0) == 0);
	fail_unless(x_provider.getSize() == 0);
	fail_unless(x_provider.getRow(0) == 0);
}
END_TEST

START_TEST(add)
{
	SbModel model;
	SbEntry *r_entry, *w_entry, *x_entry;
	SbEntry *rw_entry, *rx_entry, *wx_entry, *rwx_entry;
	SbModelRowProvider r_provider(&model, SbEntry::READ);
	SbModelRowProvider w_provider(&model, SbEntry::WRITE);
	SbModelRowProvider x_provider(&model, SbEntry::EXECUTE);

	r_entry = model.getEntry(wxT("r"), true);
	w_entry = model.getEntry(wxT("w"), true);
	x_entry = model.getEntry(wxT("x"), true);
	rw_entry = model.getEntry(wxT("rw"), true);
	rx_entry = model.getEntry(wxT("rx"), true);
	wx_entry = model.getEntry(wxT("wx"), true);
	rwx_entry = model.getEntry(wxT("rwx"), true);

	fail_unless(r_entry != 0);
	fail_unless(w_entry != 0);
	fail_unless(x_entry != 0);
	fail_unless(rw_entry != 0);
	fail_unless(rx_entry != 0);
	fail_unless(wx_entry != 0);
	fail_unless(rwx_entry != 0);

	r_entry->setPermission(SbEntry::READ, true);
	w_entry->setPermission(SbEntry::WRITE, true);
	x_entry->setPermission(SbEntry::EXECUTE, true);

	rw_entry->setPermission(SbEntry::READ, true);
	rw_entry->setPermission(SbEntry::WRITE, true);

	rx_entry->setPermission(SbEntry::READ, true);
	rx_entry->setPermission(SbEntry::EXECUTE, true);

	wx_entry->setPermission(SbEntry::WRITE, true);
	wx_entry->setPermission(SbEntry::EXECUTE, true);

	rwx_entry->setPermission(SbEntry::READ, true);
	rwx_entry->setPermission(SbEntry::WRITE, true);
	rwx_entry->setPermission(SbEntry::EXECUTE, true);

	fail_unless(model.getEntryCount() == 7);
	fail_unless(model.getEntryCount(SbEntry::READ) == 4);
	fail_unless(model.getEntryCount(SbEntry::WRITE) == 4);
	fail_unless(model.getEntryCount(SbEntry::EXECUTE) == 4);
	fail_unless(model.getEntry(0) == r_entry);
	fail_unless(model.getEntry(1) == w_entry);
	fail_unless(model.getEntry(2) == x_entry);
	fail_unless(model.getEntry(3) == rw_entry);
	fail_unless(model.getEntry(4) == rx_entry);
	fail_unless(model.getEntry(5) == wx_entry);
	fail_unless(model.getEntry(6) == rwx_entry);
	fail_unless(model.getEntry(7) == 0);

	fail_unless(model.getEntry(wxT("r")) == r_entry);
	fail_unless(model.getEntry(wxT("r"), false) == r_entry);
	fail_unless(model.getEntry(wxT("w")) == w_entry);
	fail_unless(model.getEntry(wxT("w"), false) == w_entry);
	fail_unless(model.getEntry(wxT("x")) == x_entry);
	fail_unless(model.getEntry(wxT("x"), false) == x_entry);
	fail_unless(model.getEntry(wxT("rw")) == rw_entry);
	fail_unless(model.getEntry(wxT("rw"), false) == rw_entry);
	fail_unless(model.getEntry(wxT("rx")) == rx_entry);
	fail_unless(model.getEntry(wxT("rx"), false) == rx_entry);
	fail_unless(model.getEntry(wxT("wx")) == wx_entry);
	fail_unless(model.getEntry(wxT("wx"), false) == wx_entry);
	fail_unless(model.getEntry(wxT("rwx")) == rwx_entry);
	fail_unless(model.getEntry(wxT("rwx"), false) == rwx_entry);
	fail_unless(model.getEntry(wxT("foo")) == 0);
	fail_unless(model.getEntry(wxT("foo"), false) == 0);

	fail_unless(r_provider.getSize() == 4);
	fail_unless(r_provider.getRow(0) == r_entry);
	fail_unless(r_provider.getRow(1) == rw_entry);
	fail_unless(r_provider.getRow(2) == rx_entry);
	fail_unless(r_provider.getRow(3) == rwx_entry);
	fail_unless(r_provider.getRow(4) == 0);

	fail_unless(w_provider.getSize() == 4);
	fail_unless(w_provider.getRow(0) == w_entry);
	fail_unless(w_provider.getRow(1) == rw_entry);
	fail_unless(w_provider.getRow(2) == wx_entry);
	fail_unless(w_provider.getRow(3) == rwx_entry);
	fail_unless(w_provider.getRow(4) == 0);

	fail_unless(x_provider.getSize() == 4);
	fail_unless(x_provider.getRow(0) == x_entry);
	fail_unless(x_provider.getRow(1) == rx_entry);
	fail_unless(x_provider.getRow(2) == wx_entry);
	fail_unless(x_provider.getRow(3) == rwx_entry);
	fail_unless(x_provider.getRow(4) == 0);
}
END_TEST

START_TEST(remove_idx)
{
	SbModel model;
	SbEntry *r_entry, *w_entry, *x_entry;
	SbModelRowProvider r_provider(&model, SbEntry::READ);
	SbModelRowProvider w_provider(&model, SbEntry::WRITE);
	SbModelRowProvider x_provider(&model, SbEntry::EXECUTE);

	r_entry = model.getEntry(wxT("r"), true);
	w_entry = model.getEntry(wxT("w"), true);
	x_entry = model.getEntry(wxT("x"), true);

	fail_unless(r_entry != 0);
	fail_unless(w_entry != 0);
	fail_unless(x_entry != 0);

	r_entry->setPermission(SbEntry::READ, true);
	w_entry->setPermission(SbEntry::WRITE, true);
	x_entry->setPermission(SbEntry::EXECUTE, true);

	fail_unless(model.removeEntry(1));
	fail_unless(model.getEntryCount() == 2);
	fail_unless(model.getEntryCount(SbEntry::READ) == 1);
	fail_unless(model.getEntryCount(SbEntry::WRITE) == 0);
	fail_unless(model.getEntryCount(SbEntry::EXECUTE) == 1);
	fail_unless(model.getEntry(0) == r_entry);
	fail_unless(model.getEntry(1) == x_entry);
	fail_unless(model.getEntry(2) == 0);

	fail_unless(r_provider.getSize() == 1);
	fail_unless(r_provider.getRow(0) == r_entry);
	fail_unless(r_provider.getRow(1) == 0);

	fail_unless(w_provider.getSize() == 0);
	fail_unless(w_provider.getRow(0) == 0);

	fail_unless(x_provider.getSize() == 1);
	fail_unless(x_provider.getRow(0) == x_entry);
	fail_unless(x_provider.getRow(1) == 0);
}
END_TEST

START_TEST(remove_path)
{
	SbModel model;
	SbEntry *r_entry, *w_entry, *x_entry;
	SbModelRowProvider r_provider(&model, SbEntry::READ);
	SbModelRowProvider w_provider(&model, SbEntry::WRITE);
	SbModelRowProvider x_provider(&model, SbEntry::EXECUTE);

	r_entry = model.getEntry(wxT("r"), true);
	w_entry = model.getEntry(wxT("w"), true);
	x_entry = model.getEntry(wxT("x"), true);

	fail_unless(r_entry != 0);
	fail_unless(w_entry != 0);
	fail_unless(x_entry != 0);

	r_entry->setPermission(SbEntry::READ, true);
	w_entry->setPermission(SbEntry::WRITE, true);
	x_entry->setPermission(SbEntry::EXECUTE, true);

	fail_unless(model.removeEntry(wxT("x")));
	fail_unless(model.getEntryCount() == 2);
	fail_unless(model.getEntryCount(SbEntry::READ) == 1);
	fail_unless(model.getEntryCount(SbEntry::WRITE) == 1);
	fail_unless(model.getEntryCount(SbEntry::EXECUTE) == 0);
	fail_unless(model.getEntry(0) == r_entry);
	fail_unless(model.getEntry(1) == w_entry);
	fail_unless(model.getEntry(2) == 0);

	fail_unless(r_provider.getSize() == 1);
	fail_unless(r_provider.getRow(0) == r_entry);
	fail_unless(r_provider.getRow(1) == 0);

	fail_unless(w_provider.getSize() == 1);
	fail_unless(w_provider.getRow(0) == w_entry);
	fail_unless(w_provider.getRow(1) == 0);

	fail_unless(x_provider.getSize() == 0);
	fail_unless(x_provider.getRow(0) == 0);
}
END_TEST

START_TEST(remove_outofrange)
{
	SbModel model;
	SbEntry *r_entry, *w_entry, *x_entry;
	SbModelRowProvider r_provider(&model, SbEntry::READ);
	SbModelRowProvider w_provider(&model, SbEntry::WRITE);
	SbModelRowProvider x_provider(&model, SbEntry::EXECUTE);

	r_entry = model.getEntry(wxT("r"), true);
	w_entry = model.getEntry(wxT("w"), true);
	x_entry = model.getEntry(wxT("x"), true);

	fail_unless(r_entry != 0);
	fail_unless(w_entry != 0);
	fail_unless(x_entry != 0);

	r_entry->setPermission(SbEntry::READ, true);
	w_entry->setPermission(SbEntry::WRITE, true);
	x_entry->setPermission(SbEntry::EXECUTE, true);

	fail_unless(!model.removeEntry(3));
	fail_unless(!model.removeEntry(wxT("foo")));

	fail_unless(model.getEntryCount() == 3);
	fail_unless(model.getEntryCount(SbEntry::READ) == 1);
	fail_unless(model.getEntryCount(SbEntry::WRITE) == 1);
	fail_unless(model.getEntryCount(SbEntry::EXECUTE) == 1);
	fail_unless(model.getEntry(0) == r_entry);
	fail_unless(model.getEntry(1) == w_entry);
	fail_unless(model.getEntry(2) == x_entry);
	fail_unless(model.getEntry(3) == 0);

	fail_unless(r_provider.getSize() == 1);
	fail_unless(r_provider.getRow(0) == r_entry);
	fail_unless(r_provider.getRow(1) == 0);

	fail_unless(w_provider.getSize() == 1);
	fail_unless(w_provider.getRow(0) == w_entry);
	fail_unless(w_provider.getRow(1) == 0);

	fail_unless(x_provider.getSize() == 1);
	fail_unless(x_provider.getRow(0) == x_entry);
	fail_unless(x_provider.getRow(1) == 0);
}
END_TEST

START_TEST(default_sandbox)
{
	SbModel model;

	fail_unless(model.canAssignDefaults());

	model.assignDefaults(SbEntry::READ);
	fail_unless(model.getEntryCount() == 4);
	fail_unless(model.getEntryCount(SbEntry::READ) == 4);
	fail_unless(model.getEntryCount(SbEntry::WRITE) == 0);
	fail_unless(model.getEntryCount(SbEntry::EXECUTE) == 0);
	fail_unless(model.getEntry(0)->getPath() == wxT("r"));
	fail_unless(model.getEntry(1)->getPath() == wxT("rw"));
	fail_unless(model.getEntry(2)->getPath() == wxT("rx"));
	fail_unless(model.getEntry(3)->getPath() == wxT("rwx"));
	fail_unless(model.getEntry(4) == 0);

	model.assignDefaults(SbEntry::WRITE);
	fail_unless(model.getEntryCount() == 6);
	fail_unless(model.getEntryCount(SbEntry::READ) == 4);
	fail_unless(model.getEntryCount(SbEntry::WRITE) == 4);
	fail_unless(model.getEntryCount(SbEntry::EXECUTE) == 0);
	fail_unless(model.getEntry(0)->getPath() == wxT("r"));
	fail_unless(model.getEntry(1)->getPath() == wxT("rw"));
	fail_unless(model.getEntry(2)->getPath() == wxT("rx"));
	fail_unless(model.getEntry(3)->getPath() == wxT("rwx"));
	fail_unless(model.getEntry(4)->getPath() == wxT("w"));
	fail_unless(model.getEntry(5)->getPath() == wxT("wx"));

	model.assignDefaults(SbEntry::EXECUTE);
	fail_unless(model.getEntryCount() == 7);
	fail_unless(model.getEntryCount(SbEntry::READ) == 4);
	fail_unless(model.getEntryCount(SbEntry::WRITE) == 4);
	fail_unless(model.getEntryCount(SbEntry::EXECUTE) == 4);
	fail_unless(model.getEntry(0)->getPath() == wxT("r"));
	fail_unless(model.getEntry(1)->getPath() == wxT("rw"));
	fail_unless(model.getEntry(2)->getPath() == wxT("rx"));
	fail_unless(model.getEntry(3)->getPath() == wxT("rwx"));
	fail_unless(model.getEntry(4)->getPath() == wxT("w"));
	fail_unless(model.getEntry(5)->getPath() == wxT("wx"));
	fail_unless(model.getEntry(6)->getPath() == wxT("x"));
	fail_unless(model.getEntry(7) == 0);
}
END_TEST

START_TEST(no_default_sandbox)
{
	SbModel model;
	char path[PATH_MAX];

	snprintf(path, sizeof(path),
	    "%s/share/SandboxApp/profiles/wizard/sandbox", tmp_prefix);
	fail_unless(unlink(path) == 0, "Failed to remove %s: %s",
	    path, strerror(errno));

	fail_unless(!model.canAssignDefaults());
	model.assignDefaults(SbEntry::READ);
	model.assignDefaults(SbEntry::WRITE);
	model.assignDefaults(SbEntry::EXECUTE);

	fail_unless(model.getEntryCount() == 0);
	fail_unless(model.getEntry(0) == 0);
}
END_TEST

Suite *
getTestSuite(void)
{
	Suite *testSuite = suite_create("Sandbox");
	TCase *testCase = tcase_create("Sandbox");

	suite_add_tcase(testSuite, testCase);

	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, empty);
	tcase_add_test(testCase, add);
	tcase_add_test(testCase, remove_idx);
	tcase_add_test(testCase, remove_path);
	tcase_add_test(testCase, remove_outofrange);
	tcase_add_test(testCase, default_sandbox);
	tcase_add_test(testCase, no_default_sandbox);

	return (testSuite);
}

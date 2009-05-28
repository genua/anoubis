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

#include <sys/wait.h>

#include <wx/app.h>
#include <wx/apptrait.h>
#include <wx/stdpaths.h>

#include <check.h>
#include <stdlib.h>

#include <KeyCtrl.h>

extern "C" int test_1234_cb(char *buf, int size, int, void *);
extern "C" int  test_foo_cb(char *buf, int size, int, void *);

int
test_1234_cb(char *buf, int size, int, void *)
{
	char pw[] = "1234";
	int len = strlen(pw);
	if (len > size)
		len = size;
	memcpy(buf, pw, len);
	return len;
}

int
test_foo_cb(char *buf, int size, int, void *)
{
	char pw[] = "foobar";
	int len = strlen(pw);
	if (len > size)
		len = size;
	memcpy(buf, pw, len);
	return len;
}

class tc_PrivKey_AppTraits : public wxGUIAppTraits
{
	public:
		tc_PrivKey_AppTraits(const wxString &prefix)
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

class tc_PrivKey_App : public wxApp
{
	public:
		tc_PrivKey_App(const wxString &prefix)
		{
			this->prefix = prefix;
			SetAppName(wxT("tc_PrivKey"));
		}

		virtual wxAppTraits *CreateTraits()
		{
			return new tc_PrivKey_AppTraits(prefix);
		}

	private:
		wxString prefix;
};

static tc_PrivKey_App	*tc_App;
static char		*tc_dir = 0;
static char		path_privkey[64];
static char		path_nokey[64];

static void
setup()
{
	char cmd[1024];
	int rc;

	if ((tc_dir = (char *)malloc(32)) == 0) {
		perror("malloc");
		return;
	} else
		strcpy(tc_dir, "/tmp/tc_PrivKey_XXXXXX");

	if ((tc_dir = mkdtemp(tc_dir)) == 0) {
		perror("mkdir");
		return;
	}

	sprintf(path_privkey, "%s/privkey.pem", tc_dir);
	sprintf(path_nokey, "%s/privkey.no", tc_dir);

	sprintf(cmd,
	    "openssl genrsa -des3 -out %s -passout pass:1234 2048 "
	    ">/dev/null 2>&1",
	    path_privkey);
	rc = system(cmd);
	if (!WIFEXITED(rc) || WEXITSTATUS(rc) != 0) {
		perror("openssl");
		return;
	}

	sprintf(cmd, "touch %s", path_nokey);
	rc = system(cmd);
	if (!WIFEXITED(rc) || WEXITSTATUS(rc) != 0) {
		perror("openssl");
		return;
	}

	setenv("HOME", tc_dir, 1);
	tc_App = new tc_PrivKey_App(wxString::FromAscii(tc_dir));
	wxApp::SetInstance(tc_App);
}

static void
teardown()
{
	char cmd[1024];
	int rc;

	sprintf(cmd, "rm -rf %s", tc_dir);
	rc = system(cmd);
	if (!WIFEXITED(rc) || WEXITSTATUS(rc) != 0) {
		perror("rm");
		return;
	}

	free(tc_dir);
	memset(path_privkey, 0, sizeof(path_privkey));
	memset(path_nokey, 0, sizeof(path_nokey));

	wxApp::SetInstance(0);
	delete tc_App;
}

START_TEST(tc_PrivKey_create)
{
	PrivKey privKey;

	fail_unless(!privKey.canLoad(), "Can load private key");
	fail_unless(!privKey.isLoaded(), "Private key is loaded");
}
END_TEST

START_TEST(tc_PrivKey_create_default_key)
{
	const wxString userDataDir =
	    wxString::FromAscii(tc_dir) + wxT("/.tc_PrivKey");
	const wxString defKey = userDataDir + wxT("/default.key");

	fail_unless(wxMkdir(userDataDir), "Failed to create userdatadir");
	fail_unless(wxCopyFile(wxString::FromAscii(path_privkey), defKey),
	    "Failed to create default-key.");

	PrivKey privKey;

	fail_unless(privKey.canLoad(), "Cannot load private key");
	fail_unless(!privKey.isLoaded(), "Private key is loaded");
	fail_unless(privKey.getFile() == defKey, "Wrong file assigned");
}
END_TEST

START_TEST(tc_PrivKey_load)
{
	PrivKey privKey;
	PrivKey::PrivKeyResult pRes;

	privKey.setFile(wxString::FromAscii(path_privkey));
	fail_unless(privKey.canLoad(), "Cannot load private key");

	pRes = privKey.load(test_1234_cb);
	fail_unless(pRes == PrivKey::ERR_PRIV_OK, "Failed to load private key");
	fail_unless(privKey.isLoaded(), "Private key is not loaded");
}
END_TEST

START_TEST(tc_PrivKey_load_MissingFile)
{
	PrivKey privKey;
	PrivKey::PrivKeyResult pRes;

	fail_unless(!privKey.canLoad(), "Can load private key");

	pRes = privKey.load(test_1234_cb);
	fail_unless(pRes == PrivKey::ERR_PRIV_ERR,
	    "Loading of private key was successful");
	fail_unless(!privKey.isLoaded(), "Private key is loaded");
}
END_TEST

START_TEST(tc_PrivKey_load_WrongFile)
{
	PrivKey privKey;
	PrivKey::PrivKeyResult pRes;

	privKey.setFile(wxString::FromAscii(path_nokey));
	fail_unless(privKey.canLoad(), "Can load private key");

	pRes = privKey.load(test_foo_cb);
	fail_unless(pRes == PrivKey::ERR_PRIV_ERR,
	    "Loading of private key was successful");
	fail_unless(!privKey.isLoaded(), "Private key is loaded");
}
END_TEST

START_TEST(tc_PrivKey_load_WrongPassphrase)
{
	PrivKey privKey;
	PrivKey::PrivKeyResult pRes;

	privKey.setFile(wxString::FromAscii(path_privkey));
	fail_unless(privKey.canLoad(), "Can load private key");

	pRes = privKey.load(test_foo_cb);
	fail_unless(pRes == PrivKey::ERR_PRIV_WRONG_PASS,
	    "Loading of private key was successful");
	fail_unless(!privKey.isLoaded(), "Private key is loaded");
}
END_TEST

START_TEST(tc_PrivKey_unload)
{
	PrivKey privKey;
	PrivKey::PrivKeyResult pRes;

	privKey.setFile(wxString::FromAscii(path_privkey));
	fail_unless(privKey.canLoad(), "Cannot load private key");

	pRes = privKey.load(test_1234_cb);
	fail_unless(pRes == PrivKey::ERR_PRIV_OK,
	    "Failed to load private key");
	fail_unless(privKey.isLoaded(), "Private key is not loaded");

	bool success = privKey.unload();
	fail_unless(success, "Failed to unload private key");
	fail_unless(!privKey.isLoaded(), "Private key is loaded");
}
END_TEST

START_TEST(tc_PrivKey_unload_NotLoaded)
{
	PrivKey privKey;
	PrivKey::PrivKeyResult pRes;

	privKey.setFile(wxString::FromAscii(path_privkey));
	fail_unless(privKey.canLoad(), "Can load private key");

	pRes = privKey.load(test_foo_cb);
	fail_unless(pRes == PrivKey::ERR_PRIV_WRONG_PASS,
	    "Loading of private key was successful");
	fail_unless(!privKey.isLoaded(), "Private key is loaded");

	bool success = privKey.unload();
	fail_unless(!success, "Unload the private key was successful");
	fail_unless(!privKey.isLoaded(), "Private key is loaded");
}
END_TEST

TCase *
getTc_PrivKey(void)
{
	TCase *testCase;

	testCase = tcase_create("PrivKey");

	tcase_set_timeout(testCase, 120);
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, tc_PrivKey_create);
	tcase_add_test(testCase, tc_PrivKey_create_default_key);
	tcase_add_test(testCase, tc_PrivKey_load);
	tcase_add_test(testCase, tc_PrivKey_load_MissingFile);
	tcase_add_test(testCase, tc_PrivKey_load_WrongFile);
	tcase_add_test(testCase, tc_PrivKey_load_WrongPassphrase);
	tcase_add_test(testCase, tc_PrivKey_unload);
	tcase_add_test(testCase, tc_PrivKey_unload_NotLoaded);

	return (testCase);
}

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

class tc_KeyCtrl_AppTraits : public wxGUIAppTraits
{
	public:
		tc_KeyCtrl_AppTraits(const wxString &prefix)
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

class tc_KeyCtrl_App : public wxApp
{
	public:
		tc_KeyCtrl_App(const wxString &prefix)
		{
			this->prefix = prefix;
			SetAppName(wxT("tc_KeyCtrl"));
		}

		virtual wxAppTraits *CreateTraits()
		{
			return new tc_KeyCtrl_AppTraits(prefix);
		}

	private:
		wxString prefix;
};

class tc_KeyCtrl_PassphraseReader : public PassphraseReader
{
	public:
		tc_KeyCtrl_PassphraseReader(const wxString &passphrase,
		    bool ok) {
			this->passphrase_ = passphrase;
			this->ok_ = ok;
		}

		wxString readPassphrase(bool *ok) {
			*ok = this->ok_;
			return (this->passphrase_);
		}

	private:
		wxString passphrase_;
		bool ok_;
};

static tc_KeyCtrl_App	*tc_App;
static KeyCtrl		*tc_keyCtrl = 0;
static char		*tc_dir = 0;
static char		path_privkey[64];
static char		path_nokey[64];
static char		path_cert[64];
static char		path_cnf[64];

static void
setup()
{
	char cmd[1024];
	int rc;

	if ((tc_dir = (char *)malloc(32)) == 0) {
		perror("malloc");
		return;
	} else
		strcpy(tc_dir, "/tmp/tc_KeyCtrl_XXXXXX");

	if ((tc_dir = mkdtemp(tc_dir)) == 0) {
		perror("mkdir");
		return;
	}

	sprintf(path_privkey, "%s/privkey.pem", tc_dir);
	sprintf(path_nokey, "%s/privkey.no", tc_dir);
	sprintf(path_cert, "%s/cacert.pem", tc_dir);
	sprintf(path_cnf, "%s/openssl.cnf", tc_dir);

	FILE *fh = fopen(path_cnf, "w");
	if (fh == NULL) {
		perror(path_cnf);
		return;
	}

	fprintf(fh, "[ req ]\n");
	fprintf(fh, "distinguished_name = root_ca_distinguished_name\n");
	fprintf(fh, "x509_extensions = v3_ca\n");
	fprintf(fh, "\n");
	fprintf(fh, "[ root_ca_distinguished_name ]\n");
	fprintf(fh, "\n");
	fprintf(fh, "[ v3_ca ]\n");
	fprintf(fh, "subjectKeyIdentifier=hash\n");
	fprintf(fh, "authorityKeyIdentifier=keyid:always,issuer:always\n");

	fflush(fh);
	fclose(fh);

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

	sprintf(cmd,
	    "openssl req -new -x509 -config %s -key %s -out %s -days 1 -subj "
	    "/C=DE/ST=Bayern/L=Kirchheim/O=Genua/OU=CoDev/CN=Worker"
	    "/emailAddress=info@genua.de -passin pass:1234 "
	    ">/dev/null 2>&1",
	    path_cnf, path_privkey, path_cert);
	rc = system(cmd);
	if (!WIFEXITED(rc) || WEXITSTATUS(rc) != 0) {
		perror("openssl");
		return;
	}

	setenv("HOME", tc_dir, 1);
	tc_App = new tc_KeyCtrl_App(wxString::FromAscii(tc_dir));
	wxApp::SetInstance(tc_App);

	tc_keyCtrl = KeyCtrl::getInstance();
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
	memset(path_cert, 0, sizeof(path_cert));
	memset(path_cnf, 0, sizeof(path_cnf));

	wxApp::SetInstance(0);
	delete tc_App;

	delete tc_keyCtrl;
	tc_keyCtrl = 0;
}

START_TEST(tc_KeyCtrl_canUseLocalKeys_ok)
{
	LocalCertificate &cert = tc_keyCtrl->getLocalCertificate();
	cert.setFile(wxString::FromAscii(path_cert));

	fail_unless(cert.canLoad(), "Unable to load the certificate");
	fail_unless(cert.load(), "Failed to load the certificate");

	PrivKey &privKey = tc_keyCtrl->getPrivateKey();
	privKey.setFile(wxString::FromAscii(path_privkey));

	fail_unless(privKey.canLoad(), "Unable to load private key");

	fail_unless(tc_keyCtrl->canUseLocalKeys(), "Cannot use local keys");
}
END_TEST

START_TEST(tc_KeyCtrl_canUseLocalKeys_NoCert)
{
	PrivKey &privKey = tc_keyCtrl->getPrivateKey();
	privKey.setFile(wxString::FromAscii(path_privkey));

	fail_unless(privKey.canLoad(), "Unable to load private key");

	fail_unless(!tc_keyCtrl->canUseLocalKeys(),
	    "You are able to use local keys");
}
END_TEST

START_TEST(tc_KeyCtrl_canUseLocalKeys_NoPrivKey)
{
	LocalCertificate &cert = tc_keyCtrl->getLocalCertificate();
	cert.setFile(wxString::FromAscii(path_cert));

	fail_unless(cert.canLoad(), "Unable to load the certificate");
	fail_unless(cert.load(), "Failed to load the certificate");

	fail_unless(!tc_keyCtrl->canUseLocalKeys(),
	    "You are able to use local keys");
}
END_TEST

START_TEST(tc_KeyCtrl_loadPrivateKey_ok)
{
	KeyCtrl::KeyResult kRes;

	tc_KeyCtrl_PassphraseReader reader(wxT("1234"), true);
	tc_keyCtrl->setPassphraseReader(&reader);

	PrivKey &privKey = tc_keyCtrl->getPrivateKey();
	privKey.setFile(wxString::FromAscii(path_privkey));

	kRes = tc_keyCtrl->loadPrivateKey();
	fail_unless(kRes == KeyCtrl::RESULT_KEY_OK,
	    "Failed to load private key");
	fail_unless(privKey.isLoaded(), "Private key is not loaded");
}
END_TEST

START_TEST(tc_KeyCtrl_loadPrivateKey_wrongpass)
{
	KeyCtrl::KeyResult kRes;

	tc_KeyCtrl_PassphraseReader reader(wxT("foobar"), true);
	tc_keyCtrl->setPassphraseReader(&reader);

	PrivKey &privKey = tc_keyCtrl->getPrivateKey();
	privKey.setFile(wxString::FromAscii(path_privkey));

	kRes = tc_keyCtrl->loadPrivateKey();
	fail_unless(kRes == KeyCtrl::RESULT_KEY_WRONG_PASS,
	    "Load private key was successful");
	fail_unless(!privKey.isLoaded(), "Private key is loaded");
}
END_TEST

START_TEST(tc_KeyCtrl_loadPrivateKey_canceled)
{
	KeyCtrl::KeyResult kRes;

	tc_KeyCtrl_PassphraseReader reader(wxT("1234"), false);
	tc_keyCtrl->setPassphraseReader(&reader);

	PrivKey &privKey = tc_keyCtrl->getPrivateKey();
	privKey.setFile(wxString::FromAscii(path_privkey));

	kRes = tc_keyCtrl->loadPrivateKey();
	fail_unless(kRes == KeyCtrl::RESULT_KEY_ABORT,
	    "Load private key was successful");
	fail_unless(!privKey.isLoaded(), "Private key is loaded");
}
END_TEST

TCase *
getTc_KeyCtrl(void)
{
	TCase *testCase;

	testCase = tcase_create("KeyCtrl");

	tcase_set_timeout(testCase, 120);
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, tc_KeyCtrl_canUseLocalKeys_ok);
	tcase_add_test(testCase, tc_KeyCtrl_canUseLocalKeys_NoCert);
	tcase_add_test(testCase, tc_KeyCtrl_canUseLocalKeys_NoPrivKey);
	tcase_add_test(testCase, tc_KeyCtrl_loadPrivateKey_ok);
	tcase_add_test(testCase, tc_KeyCtrl_loadPrivateKey_wrongpass);
	tcase_add_test(testCase, tc_KeyCtrl_loadPrivateKey_canceled);

	return (testCase);
}

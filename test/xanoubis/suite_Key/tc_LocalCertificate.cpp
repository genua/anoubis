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

class tc_LocalCertificate_AppTraits : public wxGUIAppTraits
{
	public:
		tc_LocalCertificate_AppTraits(const wxString &prefix)
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

class tc_LocalCertificate_App : public wxApp
{
	public:
		tc_LocalCertificate_App(const wxString &prefix)
		{
			this->prefix = prefix;
			SetAppName(wxT("tc_LocalCertificate"));
		}

		virtual wxAppTraits *CreateTraits()
		{
			return new tc_LocalCertificate_AppTraits(prefix);
		}

	private:
		wxString prefix;
};

static tc_LocalCertificate_App	*tc_App;
static char			*tc_dir = 0;
static char			path_privkey[64];
static char			path_nokey[64];
static char			path_cert[64];
static char			path_cnf[64];
static const char		*dname = "/C=DE/ST=Bayern/L=Kirchheim/O=Genua/"
				    "OU=CoDev/CN=Worker/"
				    "emailAddress=info@genua.de";
static void
setup()
{
	char cmd[1024];
	int rc;

	if ((tc_dir = (char *)malloc(32)) == 0) {
		perror("malloc");
		return;
	} else
		strcpy(tc_dir, "/tmp/tc_LocalCertificate_XXXXXX");

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
	tc_App = new tc_LocalCertificate_App(wxString::FromAscii(tc_dir));
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
	memset(path_cert, 0, sizeof(path_cert));
	memset(path_cnf, 0, sizeof(path_cnf));

	wxApp::SetInstance(0);
	delete tc_App;
}

START_TEST(tc_LocalCertificate_create)
{
	LocalCertificate cert;

	fail_unless(cert.getFile() == wxEmptyString,
	    "A file is assiged to the certificate");
	fail_unless(cert.getKeyId() == wxEmptyString,
	    "A keyid is assiged to the certificate");
	fail_unless(cert.getFingerprint() == wxEmptyString,
	    "A fingerprint is assiged to the certificate");
	fail_unless(cert.getDistinguishedName() == wxEmptyString,
	    "A DN is assiged to the certificate");
	fail_unless(!cert.canLoad(), "The certificate can be loaded");
	fail_unless(!cert.isLoaded(), "The certificate is loaded");
}
END_TEST

START_TEST(tc_LocalCertificate_create_default_cert)
{
	const wxString userDataDir =
	    wxString::FromAscii(tc_dir) + wxT("/.tc_LocalCertificate");
	const wxString defKey = userDataDir + wxT("/default.crt");

	fail_unless(wxMkdir(userDataDir), "Failed to create userdatadir");
	fail_unless(wxCopyFile(wxString::FromAscii(path_privkey), defKey),
	    "Failed to create default-key.");

	LocalCertificate cert;

	fail_unless(cert.getFile() == defKey,
	    "No file is assiged to the certificate");
	fail_unless(cert.getKeyId() == wxEmptyString,
	    "A keyid is assiged to the certificate");
	fail_unless(cert.getFingerprint() == wxEmptyString,
	    "A fingerprint is assiged to the certificate");
	fail_unless(cert.getDistinguishedName() == wxEmptyString,
	    "A DN is assiged to the certificate");
	fail_unless(cert.canLoad(), "The certificate cannot be loaded");
	fail_unless(!cert.isLoaded(), "The certificate is loaded");
}
END_TEST

START_TEST(tc_LocalCertificate_setFile_ok)
{
	const wxString path = wxString::FromAscii(path_cert);
	LocalCertificate cert;

	cert.setFile(path);

	fail_unless(cert.getFile() == path,
	    "No file is assiged to the certificate");
	fail_unless(cert.getKeyId() == wxEmptyString,
	    "A keyid is assiged to the certificate");
	fail_unless(cert.getFingerprint() == wxEmptyString,
	    "A fingerprint is assiged to the certificate");
	fail_unless(cert.getDistinguishedName() == wxEmptyString,
	    "A DN is assiged to the certificate");
	fail_unless(cert.canLoad(), "The certificate cannot be loaded");
	fail_unless(!cert.isLoaded(), "The certificate is loaded");
}
END_TEST

START_TEST(tc_LocalCertificate_setFile_empty)
{
	LocalCertificate cert;

	cert.setFile(wxEmptyString);

	fail_unless(cert.getFile() == wxEmptyString,
	    "A file is assiged to the certificate");
	fail_unless(cert.getKeyId() == wxEmptyString,
	    "A keyid is assiged to the certificate");
	fail_unless(cert.getFingerprint() == wxEmptyString,
	    "A fingerprint is assiged to the certificate");
	fail_unless(cert.getDistinguishedName() == wxEmptyString,
	    "A DN is assiged to the certificate");
	fail_unless(!cert.canLoad(), "The certificate can be loaded");
	fail_unless(!cert.isLoaded(), "The certificate is loaded");
}
END_TEST

START_TEST(tc_LocalCertificate_setFile_unknown)
{
	const wxString path = wxString::FromAscii(tc_dir) + wxT("/foobar");
	LocalCertificate cert;

	cert.setFile(path);

	fail_unless(cert.getFile() == path,
	    "No file is assiged to the certificate");
	fail_unless(cert.getKeyId() == wxEmptyString,
	    "A keyid is assiged to the certificate");
	fail_unless(cert.getFingerprint() == wxEmptyString,
	    "A fingerprint is assiged to the certificate");
	fail_unless(cert.getDistinguishedName() == wxEmptyString,
	    "A DN is assiged to the certificate");
	fail_unless(!cert.canLoad(), "The certificate can be loaded");
	fail_unless(!cert.isLoaded(), "The certificate is loaded");
}
END_TEST

START_TEST(tc_LocalCertificate_load_ok)
{
	const wxString path = wxString::FromAscii(path_cert);
	LocalCertificate cert;

	cert.setFile(path);
	wxString disName = wxString::FromAscii(dname);

	fail_unless(cert.load(), "Failed to load the certificate");
	fail_unless(cert.getFile() == path,
	    "No file is assiged to the certificate");
	fail_unless(cert.getKeyId() != wxEmptyString,
	    "No keyid is assiged to the certificate");
	fail_unless(cert.getFingerprint() != wxEmptyString,
	    "No fingerprint is assiged to the certificate");
	fail_unless(cert.getDistinguishedName() == disName,
	    "No DN is assiged to the certificate");
	fail_unless(cert.canLoad(), "The certificate cannot be loaded");
	fail_unless(cert.isLoaded(), "The certificate is not loaded");
}
END_TEST

START_TEST(tc_LocalCertificate_load_nofile)
{
	LocalCertificate cert;

	fail_unless(!cert.load(), "Loading of the certificate succeeded");
	fail_unless(cert.getFile() == wxEmptyString,
	    "A file is assiged to the certificate");
	fail_unless(cert.getKeyId() == wxEmptyString,
	    "A keyid is assiged to the certificate");
	fail_unless(cert.getFingerprint() == wxEmptyString,
	    "A fingerprint is assiged to the certificate");
	fail_unless(cert.getDistinguishedName() == wxEmptyString,
	    "A DN is assiged to the certificate");
	fail_unless(!cert.canLoad(), "The certificate can be loaded");
	fail_unless(!cert.isLoaded(), "The certificate is loaded");
}
END_TEST

START_TEST(tc_LocalCertificate_load_wrongfile)
{
	const wxString path = wxString::FromAscii(path_nokey);
	LocalCertificate cert;

	cert.setFile(path);

	fail_unless(!cert.load(), "Loading the certificate succeeded");
	fail_unless(cert.getFile() == path,
	    "No file is assiged to the certificate");
	fail_unless(cert.getKeyId() == wxEmptyString,
	    "A keyid is assiged to the certificate");
	fail_unless(cert.getFingerprint() == wxEmptyString,
	    "A fingerprint is assiged to the certificate");
	fail_unless(cert.getDistinguishedName() == wxEmptyString,
	    "A DN is assiged to the certificate");
	fail_unless(cert.canLoad(), "The certificate cannot be loaded");
	fail_unless(!cert.isLoaded(), "The certificate is loaded");
}
END_TEST

START_TEST(tc_LocalCertificate_unload_ok)
{
	const wxString path = wxString::FromAscii(path_cert);
	LocalCertificate cert;

	cert.setFile(path);

	fail_unless(cert.load(), "Failed to load the certificate");

	fail_unless(cert.unload(), "Failed to unload certificate");

	fail_unless(cert.getFile() == path,
	    "No file is assiged to the certificate");
	fail_unless(cert.getKeyId() == wxEmptyString,
	    "A keyid is assiged to the certificate");
	fail_unless(cert.getFingerprint() == wxEmptyString,
	    "A fingerprint is assiged to the certificate");
	fail_unless(cert.getDistinguishedName() == wxEmptyString,
	    "A DN is assiged to the certificate");
	fail_unless(cert.canLoad(), "The certificate cannot be loaded");
	fail_unless(!cert.isLoaded(), "The certificate is loaded");
}
END_TEST

START_TEST(tc_LocalCertificate_unload_notloaded)
{
	LocalCertificate cert;

	fail_unless(!cert.load(), "Loading the certificate succedded");

	fail_unless(!cert.unload(), "Unloading the certificate succeeded");

	fail_unless(cert.getFile() == wxEmptyString,
	    "A file is assiged to the certificate");
	fail_unless(cert.getKeyId() == wxEmptyString,
	    "A keyid is assiged to the certificate");
	fail_unless(cert.getFingerprint() == wxEmptyString,
	    "A fingerprint is assiged to the certificate");
	fail_unless(cert.getDistinguishedName() == wxEmptyString,
	    "A DN is assiged to the certificate");
	fail_unless(!cert.canLoad(), "The certificate can be loaded");
	fail_unless(!cert.isLoaded(), "The certificate is loaded");
}
END_TEST

TCase *
getTc_LocalCertificate(void)
{
	TCase *testCase;

	testCase = tcase_create("LocalCertificate");

	tcase_set_timeout(testCase, 120);
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, tc_LocalCertificate_create);
	tcase_add_test(testCase, tc_LocalCertificate_create_default_cert);
	tcase_add_test(testCase, tc_LocalCertificate_setFile_ok);
	tcase_add_test(testCase, tc_LocalCertificate_setFile_empty);
	tcase_add_test(testCase, tc_LocalCertificate_setFile_unknown);
	tcase_add_test(testCase, tc_LocalCertificate_load_ok);
	tcase_add_test(testCase, tc_LocalCertificate_load_nofile);
	tcase_add_test(testCase, tc_LocalCertificate_load_wrongfile);
	tcase_add_test(testCase, tc_LocalCertificate_unload_ok);
	tcase_add_test(testCase, tc_LocalCertificate_unload_notloaded);

	return (testCase);
}

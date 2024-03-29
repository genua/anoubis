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


#include <anoubis_sig.h>
#include <anoubis_errno.h>
#include <sys/types.h>

#include <stdarg.h>
#include <check.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <anoubischeck.h>


static const char pol_pri[] = "private.pem";
static const char pol_pub[] = "public.pem";
static const char pol_cert[] = "key_self.crt";
static const char pol[] = "policy";

static char	 workdir[32];
static char	*pol_prikey = NULL,
		*pol_certfile = NULL,
		*policy = NULL,
		*polsig = NULL;

static int
libanoubissig_tc_pol_exec(const char *cmd, ...)
{
	va_list ap;
	char *str, *syscmd;
	int rc;

	va_start(ap, cmd);
	if (vasprintf(&str, cmd, ap) < 0)
		return -1;
	va_end(ap);

	if (asprintf(&syscmd, "(%s) >/dev/null 2>&1", str) < 0)
		return -1;

	rc = system(syscmd);

	free(str);
	free(syscmd);

	return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

static void
libanoubissig_tc_pol_setup(void)
{
	strcpy(workdir, "/tmp/tc_sig_XXXXXX");
	mkdtemp_or_fail(workdir);

	/* Creating openssl.cnf */
	libanoubissig_tc_pol_exec("echo \"[ req ] \" >> %s/openssl.cnf",
	    workdir);
	libanoubissig_tc_pol_exec("echo \"default_bits = 2048 \" >>"
	    "%s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"default_keyfile = "
	    "./private/root.pem\" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"default_md = sha1 \" "
	    " >> %s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"prompt = no \" >> %s/openssl.cnf",
	    workdir);
	libanoubissig_tc_pol_exec("echo \"distinguished_name = "
	    "root_ca_distinguished_name \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"x509_extensions = v3_ca \" >> "
	    "%s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"[ root_ca_distinguished_name ]"
	    " \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"countryName = DE \" >> "
	    "%s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"stateOrProvinceName = BAYERN \" "
	    ">> %s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"localityName = Muenchen \" "
	    ">> %s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"0.organizationName = Example Inc"
	    " \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"commonName = Example Inc Root CA"
	    " \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"emailAddress = dad@example.com \" "
	    ">> %s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"[ v3_ca ] \" >> %s/openssl.cnf",
	    workdir);
	libanoubissig_tc_pol_exec("echo \"subjectKeyIdentifier=hash "
	    " \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec(
	    "echo \"authorityKeyIdentifier=keyid:always,issuer:always"
	    " \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_pol_exec("echo \"basicConstraints = CA:true "
	    " \" >> %s/openssl.cnf", workdir);

	/* Creating test policy */
	if(asprintf(&policy, "%s/%s", workdir, pol) < 0)
		return;
	libanoubissig_tc_pol_exec("echo \"alf {\" >> %s", policy);
	libanoubissig_tc_pol_exec("echo \"\tany {\" >> %s", policy);
	libanoubissig_tc_pol_exec("echo \"\t\tdefault log deny\" >> %s",
	    policy);
	libanoubissig_tc_pol_exec("echo \"\t}\" >> %s", policy);
	libanoubissig_tc_pol_exec("echo \"}\" >> %s", policy);

	libanoubissig_tc_pol_exec("openssl genrsa > %s/%s", workdir, pol_pri);
	libanoubissig_tc_pol_exec("openssl req -new -days 1 -x509 \
	    -subj /C=US/ST=Oregon/L=Portland/CN=www.madboa.com -out %s/%s \
	    -key %s/%s -config %s/openssl.cnf", workdir, pol_cert, workdir,
	    pol_pri, workdir);
	libanoubissig_tc_pol_exec("openssl rsa -in %s/%s -pubout -out %s/%s",
	    workdir, pol_pri, workdir, pol_pub);
	if(asprintf(&pol_prikey, "%s/%s", workdir, pol_pri) < 0)
		return;
	if (asprintf(&pol_certfile, "%s/%s", workdir, pol_cert) < 0)
		return;
	if (asprintf(&polsig, "%s/%s.sig", workdir, pol) < 0)
		return;
	libanoubissig_tc_pol_exec("touch %s", polsig);
}

static void
libanoubissig_tc_pol_teardown(void)
{
	/*libanoubissig_tc_pol_exec("rm -rf %s", workdir);*/
	free(pol_prikey);
}

START_TEST(sign_and_verify_policy_buf_match_tc)
{
	int			 rc, fd, err = 0;
	unsigned int		 len = 0, n = 0;
	char			 buf[4096];
	struct anoubis_sig	*as = NULL;
	unsigned char		*sign = NULL;

	fail_if(pol_prikey == NULL || policy == NULL,
	    "Error while setup testcase");

	bzero(buf, sizeof(buf));
	err = anoubis_sig_create(&as, pol_prikey, pol_certfile, pass_cb);
	fail_if(err != 0  || as == NULL, "Could not load Private Key");

	fd = open(policy, O_RDONLY);
	fail_if (fd == -1, "Couldn't open file");

	n = read(fd, buf, sizeof(buf));
	fail_if (n >= sizeof(buf), "Could not whole read file");
	fail_if (n <= 0, "Error while read file");
	close(fd);

	len = strlen(buf);
	sign = anoubis_sign_policy_buf(as, buf, &len);
	fail_if(sign == NULL, "Could not sign buf: %s", anoubis_strerror(-len));

	rc = anoubis_sig_verify_policy(policy, sign, len, as->pkey);
	anoubis_sig_free(as);
	fail_if(rc != 1, "Signatures dont match from %s",
		anoubis_strerror(-rc));
}
END_TEST

START_TEST(sign_and_verify_policy_match_tc)
{
	int			 rc, fd, err = 0;
	unsigned int		 len = 0, n = 0;
	struct anoubis_sig	*as = NULL;
	unsigned char		*sign = NULL;
	EVP_PKEY		*pubkey = NULL;

	fail_if(pol_prikey == NULL || policy == NULL,
	    "Error while setup testcase");

	err = anoubis_sig_create(&as, pol_prikey, pol_certfile, pass_cb);
	fail_if(err != 0 || as == NULL, "Could not load Private Key");

	sign = anoubis_sign_policy(as, policy, &len);
	fail_if(sign == NULL, "Could not sign policy");

	anoubis_sig_free(as);

	/* To simulate the anoubisd behavior we now writing the signature
	 * to a file named policy.sig */
	fd = open(polsig, O_WRONLY | O_CREAT, 0600);
	fail_if(fd == -1, "Could not open file %s", polsig);

	while(n < len) {
		rc = write(fd, sign, len - n);
		if (rc < 0) {
			close(fd);
			fail_if(rc < 0, "Could not write to file");
			/* NOT REACHED */
		}
		n += rc;
	}
	// cppcheck-suppress deallocDealloc
	close(fd);

	err = anoubis_sig_create(&as, NULL, pol_certfile, pass_cb);
	fail_if(err != 0 || as == NULL, "Could not load Public Key");

	pubkey = X509_get_pubkey(as->cert);
	rc = anoubis_sig_verify_policy_file(policy, pubkey);
	EVP_PKEY_free(pubkey);
	anoubis_sig_free(as);
	fail_if(rc == 0, "Signatures dont match from %s", policy);
}
END_TEST

START_TEST(sign_and_verify_policy_mismatch_tc)
{
	int			 rc, fd, err = 0;
	unsigned int		 len = 0, n = 0;
	struct anoubis_sig	*as = NULL;
	unsigned char		*sign = NULL;
	EVP_PKEY		*pubkey;

	fail_if(pol_prikey == NULL || policy == NULL,
	    "Error while setup testcase");

	err = anoubis_sig_create(&as, pol_prikey, pol_certfile, pass_cb);
	fail_if(err != 0 || as == NULL, "Could not load Private Key");

	sign = anoubis_sign_policy(as, policy, &len);
	fail_if(sign == NULL, "Could not sign policy");

	anoubis_sig_free(as);

	/* To simulate the anoubisd behavior we now writing the signature
	 * to a file named policy.sig */
	fd = open(polsig, O_WRONLY | O_CREAT, 0600);
	fail_if(fd == -1, "Could not open file %s", polsig);

	/* Now playing around with sign to simulate a wrong sig file */
	for (n = 0; n < len; n++)
		sign[n] = 2;
	n = 0;
	while(n < len) {
		rc = write(fd, sign, len - n);
		if (rc < 0) {
			close(fd);
			fail_if(rc < 0, "Could not write to file");
			/* NOT REACHED */
		}
		n += rc;
	}
	// cppcheck-suppress deallocDealloc
	close(fd);

	err = anoubis_sig_create(&as, NULL, pol_certfile, pass_cb);
	fail_if(err != 0 || as == NULL, "Could not load Public Key");

	pubkey = X509_get_pubkey(as->cert);
	rc = anoubis_sig_verify_policy_file(policy, pubkey);
	EVP_PKEY_free(pubkey);
	anoubis_sig_free(as);
	fail_if(rc == 1, "Signatures match");
}
END_TEST

TCase *
libanoubissig_testcase_policy(void)
{
	TCase *testcase = tcase_create("Policy");

	tcase_set_timeout(testcase, 120);
	tcase_add_checked_fixture(testcase, libanoubissig_tc_pol_setup,
	    libanoubissig_tc_pol_teardown);
	tcase_add_test(testcase, sign_and_verify_policy_match_tc);
	tcase_add_test(testcase, sign_and_verify_policy_buf_match_tc);
	tcase_add_test(testcase, sign_and_verify_policy_mismatch_tc);

	return (testcase);
}

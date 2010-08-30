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


const char pri[] = "private.pem";
const char pub[] = "public.pem";
const char cert[] = "key_self.crt";
const char testfile[] = "data.txt";

#define WDIR_SIZE 32

char	 workdir[WDIR_SIZE];
char	*prikey = NULL,
	*certfile = NULL,
	*infile = NULL,
	 pass[] = "test";

int
libanoubis_tc_calc_sum(char *file,
    unsigned char csum[ANOUBIS_SIG_HASH_SHA256_LEN])
{
	SHA256_CTX	 shaCtx;
	u_int8_t	 buf[4096];
	size_t		 ret;
	FILE		*fd;

	if (!file || !csum)
		return 0;

	fd = fopen(file, "r");
	if (!fd) {
		perror(file);
		return 0;
	}

	bzero(csum, ANOUBIS_SIG_HASH_SHA256_LEN);

	SHA256_Init(&shaCtx);
	while((ret = fread(buf, 1, 4096, fd)) != 0)
		SHA256_Update(&shaCtx, buf, ret);

	SHA256_Final(csum, &shaCtx);

	fclose(fd);

	return 1;
}

static int
libanoubissig_tc_exec(const char *cmd, ...)
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

void
libanoubissig_tc_setup(void)
{
	char *s;

	strncpy(workdir, "/tmp/tc_sig_XXXXXX", WDIR_SIZE);
	s = mkdtemp(workdir);

	libanoubissig_tc_exec("echo \"[ req ] \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"default_bits = 2048 \" >> %s/openssl.cnf",
	    workdir);
	libanoubissig_tc_exec("echo \"default_keyfile = ./private/root.pem\" "
	    ">> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"default_md = sha1 \" >> %s/openssl.cnf",
	    workdir);
	libanoubissig_tc_exec("echo \"prompt = no \" >> %s/openssl.cnf",
	    workdir);
	libanoubissig_tc_exec("echo \"distinguished_name = "
	    "root_ca_distinguished_name \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"x509_extensions = v3_ca \" >> "
	    "%s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"[ root_ca_distinguished_name ]"
	    " \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"countryName = DE \" >> "
	    "%s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"stateOrProvinceName = BAYERN \" "
	    ">> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"localityName = München \" "
	    ">> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"0.organizationName = Example Inc"
	    " \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"commonName = Example Inc Root CA"
	    " \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"emailAddress = dad@example.com \" "
	    ">> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"[ v3_ca ] \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"subjectKeyIdentifier=hash "
	    " \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec(
	    "echo \"authorityKeyIdentifier=keyid:always,issuer:always"
	    " \" >> %s/openssl.cnf", workdir);
	libanoubissig_tc_exec("echo \"basicConstraints = CA:true "
	    " \" >> %s/openssl.cnf", workdir);

	libanoubissig_tc_exec("openssl genrsa > %s/%s", workdir, pri);
	libanoubissig_tc_exec("openssl req -new -days 1 -x509 \
	    -subj /C=US/ST=Oregon/L=Portland/CN=www.madboa.com -out %s/%s \
	    -key %s/%s -config %s/openssl.cnf", workdir, cert, workdir, pri,
	    workdir);
	libanoubissig_tc_exec("openssl rsa -in %s/%s -pubout -out %s/%s",
	    workdir, pri, workdir, pub);
	libanoubissig_tc_exec("echo \"bla\" >  %s/%s", workdir, testfile);
	if(asprintf(&prikey, "%s/%s", workdir, pri) < 0)
		return;
	if (asprintf(&infile, "%s/%s", workdir, testfile) < 0)
		return;
	if (asprintf(&certfile, "%s/%s", workdir, cert) < 0)
		return;
}

void
libanoubissig_tc_teardown(void)
{
	libanoubissig_tc_exec("rm -rf %s", workdir);
	free(prikey);
	free(infile);
}

START_TEST(sign_and_verify_match_tc)
{
	int			 rc;
	int			 err = 0;
	unsigned int		 len = 0;
	struct anoubis_sig	*as = NULL;
	unsigned char		 csum[ANOUBIS_SIG_HASH_SHA256_LEN],
				*sign = NULL;

	fail_if(prikey == NULL || infile == NULL,
	    "Error while setup testcase");

	err = anoubis_sig_create(&as, prikey, certfile, pass_cb);
	fail_if(err < 0 || as == NULL, "Could not load Private Key");

	rc = libanoubis_tc_calc_sum(infile, csum);
	fail_if(rc == 0, "Could not calculate checksum");

	sign = anoubis_sign_csum(as, csum, &len);
	fail_if(sign == NULL, "Could not sign checksum");

	anoubis_sig_free(as);

	err = anoubis_sig_create(&as, NULL, certfile, pass_cb);
	fail_if(err < 0 || as == NULL, "Could not load Public Key");

	rc = anoubisd_verify_csum(as->pkey, csum, sign +
	    ANOUBIS_SIG_HASH_SHA256_LEN,
	    len - ANOUBIS_SIG_HASH_SHA256_LEN);
	fail_if(rc == 0, "Signatures dont match in daemon");
	anoubis_sig_free(as);
}
END_TEST

START_TEST(sign_and_verify_mismatch_tc)
{
	int			 rc, err = 0;
	unsigned int		 i, len = 0;
	char			*name = NULL;
	struct anoubis_sig	*as = NULL;
	unsigned char		 csum[ANOUBIS_SIG_HASH_SHA256_LEN],
				*sign = NULL;
	EVP_PKEY		*pubkey;

	fail_if(prikey == NULL || infile == NULL,
	    "Error while setup testcase");

	err = anoubis_sig_create(&as, prikey, certfile, pass_cb);
	fail_if(err < 0 || as == NULL, "Could not load Private Key");

	rc = libanoubis_tc_calc_sum(infile, csum);
	fail_if(rc == 0, "Could not calculate checksum");

	sign = anoubis_sign_csum(as, csum, &len);
	fail_if(sign == NULL, "Could not sign checksum");

	for(i = ANOUBIS_SIG_HASH_SHA256_LEN; i < len; i++)
		sign[i] = 2;

	anoubis_sig_free(as);

	err = anoubis_sig_create(&as, NULL, certfile, pass_cb);
	fail_if(err != 0 || as == NULL, "Could not load Public Key");

	name = anoubis_sig_cert_name(as->cert);
	fail_if(name == NULL, "Could not get DN of certificate\n");

	const char expected_name[] =
	    "C=DE, ST=BAYERN, L=M\xc3\xbcnchen, O=Example Inc, "
	    "CN=Example Inc Root CA, emailAddress=dad@example.com";

	fail_unless(strlen(name) == strlen(expected_name),
	    "DN %s has wrong length %d should be %d", name,
	    strlen(name), strlen(expected_name));
	for (i = 0; i < strlen(name); i++) {
		fail_unless(name[i] == expected_name[i],
		    "Unexpected byte at position: %i (\"%x\" <=> \"%x\"",
		    i, name[i], expected_name[i]);
	}

	pubkey = X509_get_pubkey(as->cert);
	rc = anoubisd_verify_csum(pubkey, csum, sign +
	    ANOUBIS_SIG_HASH_SHA256_LEN, len - ANOUBIS_SIG_HASH_SHA256_LEN);
	EVP_PKEY_free(pubkey);
	fail_if(rc == 1, "Signatures match in daemon %d", rc);
	anoubis_sig_free(as);
}
END_TEST

START_TEST(t_anoubis_sign_csum)
{
	struct anoubis_sig	*as = NULL;
	int			 rc = 0;
	unsigned		 len;
	unsigned char		*sign = NULL,
				 csum[ANOUBIS_SIG_HASH_SHA256_LEN];

	fail_if(prikey == NULL || infile == NULL,
	    "Error while setup testcase");

	rc = libanoubis_tc_calc_sum(infile, csum);
	fail_if(rc == 0, "Could not calculate checksum");

	sign = anoubis_sign_csum(as, csum, &len);
	fail_if(sign != NULL, "anoubis_sign_csum didn't return NULL");

	sign = anoubis_sign_csum(as, NULL, NULL);
	fail_if(sign != NULL, "anoubis_sign_csum didn't return NULL");

	/* Test behavior of anoubis_sig_free with NULL */
	anoubis_sig_free(as);
}
END_TEST

START_TEST(t_anoubis_verify_policy)
{
	int rc;

	rc = anoubis_sig_verify_policy_file(NULL, NULL);
	fail_if (rc != -1, "anoubis_sig_verify_policy_file should return -1");

	rc = anoubis_sig_verify_policy(NULL, NULL, rc, NULL);
	fail_if (rc != -6, "anoubis_sig_verify_policy should return -6 not %d",
	    rc);
}
END_TEST

START_TEST(t_anoubis_validity_check)
{
	int			 err;
	struct tm		 time;
	struct anoubis_sig	*as = NULL;

	fail_if(prikey == NULL || infile == NULL,
	    "Error while setup testcase");

	err = anoubis_sig_create(&as, NULL, certfile, pass_cb);
	fail_if(err != 0 || as == NULL, "Could not load Public Key");

	err = anoubis_sig_cert_validity(as->cert, &time);
	fail_if(err != 0, "Could not get validity time.");

	anoubis_sig_free(as);
}
END_TEST

START_TEST(t_anoubis_sig_keycmp_einval)
{
	int			 rc   = -1;
	struct anoubis_sig	*keyA = NULL;
	struct anoubis_sig	*keyB = NULL;

	fail_if(prikey == NULL || infile == NULL,
	    "Error while setup testcase.");

	rc = anoubis_sig_create(&keyA, prikey, certfile, pass_cb);
	fail_if(rc < 0 || keyA == NULL, "Could not load private key (%d / %s).",
	    rc, anoubis_strerror(-rc));

	rc = anoubis_sig_create(&keyB, NULL, certfile, pass_cb);
	fail_if(rc < 0 || keyB == NULL, "Could not load public key (%d / %s).",
	    rc, anoubis_strerror(-rc));

	rc = anoubis_sig_keycmp(NULL, NULL);
	fail_if(rc != 0, "keycmp should work with NULL pointers.");

	rc = anoubis_sig_keycmp(keyA, NULL);
	fail_if(rc != 0, "keycmp should work with NULL pointers.");

	rc = anoubis_sig_keycmp(NULL, keyB);
	fail_if(rc != 0, "keycmp should work with NULL pointers.");
}
END_TEST

/* Compare private key <-> privare key - match */
START_TEST(t_anoubis_sig_keycmp_pp_match)
{
	int			 rc   = -1;
	struct anoubis_sig	*keyA = NULL;
	struct anoubis_sig	*keyB = NULL;

	fail_if(prikey == NULL || infile == NULL,
	    "Error while setup testcase.");

	rc = anoubis_sig_create(&keyA, prikey, NULL, pass_cb);
	fail_if(rc < 0 || keyA == NULL, "Could not load private key (%d / %s).",
	    rc, anoubis_strerror(-rc));

	rc = anoubis_sig_create(&keyB, prikey, NULL, pass_cb);
	fail_if(rc < 0 || keyB == NULL, "Could not load private key (%d / %s).",
	    rc, anoubis_strerror(-rc));

	rc = anoubis_sig_keycmp(keyA, keyB);
	fail_if(rc != 0, "keycmp didn't detect same private keys (%d / %s).",
	    rc, anoubis_strerror(-rc));
}
END_TEST

/* Compare private key <-> privare key - mismatch */
START_TEST(t_anoubis_sig_keycmp_pp_mismatch)
{
	int			 rc   = -1;
	struct anoubis_sig	*keyA = NULL;
	struct anoubis_sig	*keyB = NULL;

	fail_if(prikey == NULL || infile == NULL,
	    "Error while setup testcase.");

	rc = anoubis_sig_create(&keyA, prikey, NULL, pass_cb);
	fail_if(rc != 0 || keyA == NULL,
	    "Could not load private key (%d / %s).", rc, anoubis_strerror(-rc));

	/* Create different key. */
	libanoubissig_tc_teardown();
	libanoubissig_tc_setup();

	rc = anoubis_sig_create(&keyB, prikey, NULL, pass_cb);
	fail_if(rc != 0 || keyB == NULL,
	    "Could not load private key (%d / %s).", rc, anoubis_strerror(-rc));

	rc = anoubis_sig_keycmp(keyA, keyB);
	fail_if(rc == 0, "keycmp didn't detect non equal private keys"
	    " (%d / %s).", rc, anoubis_strerror(-rc));
}
END_TEST

/* Compare private key <-> certificate - match */
START_TEST(t_anoubis_sig_keycmp_pc_match)
{
	int			 rc   = -1;
	struct anoubis_sig	*keyA = NULL;
	struct anoubis_sig	*keyB = NULL;

	fail_if(prikey == NULL || infile == NULL,
	    "Error while setup testcase.");

	rc = anoubis_sig_create(&keyA, prikey, certfile, pass_cb);
	fail_if(rc != 0 || keyA == NULL,
	    "Could not load private key (%d / %s).", rc, anoubis_strerror(-rc));

	rc = anoubis_sig_create(&keyB, NULL, certfile, pass_cb);
	fail_if(rc != 0 || keyB == NULL, "Could not load public key (%d / %s).",
	    rc, anoubis_strerror(-rc));

	rc = anoubis_sig_keycmp(keyA, keyB);
	fail_if(rc != 0, "keycmp didn't detect same key pair (%d / %s).",
	    rc, anoubis_strerror(-rc));
}
END_TEST

/* Compare private key <-> certificate - mismatch */
START_TEST(t_anoubis_sig_keycmp_pc_mismatch)
{
	int			 rc   = -1;
	struct anoubis_sig	*keyA = NULL;
	struct anoubis_sig	*keyB = NULL;

	fail_if(prikey == NULL || infile == NULL,
	    "Error while setup testcase.");

	rc = anoubis_sig_create(&keyA, prikey, certfile, pass_cb);
	fail_if(rc != 0 || keyA == NULL,
	    "Could not load private key (%d / %s).", rc, anoubis_strerror(-rc));

	/* Create different key. */
	libanoubissig_tc_teardown();
	libanoubissig_tc_setup();

	rc = anoubis_sig_create(&keyB, NULL, certfile, pass_cb);
	fail_if(rc != 0 || keyB == NULL, "Could not load public key (%d / %s).",
	    rc, anoubis_strerror(-rc));

	rc = anoubis_sig_keycmp(keyA, keyB);
	fail_if(rc == 0, "keycmp didn't detect non equal key pair"
	    " (%d / %s).", rc, anoubis_strerror(-rc));
}
END_TEST

/* Compare certificate <-> certificate - match */
START_TEST(t_anoubis_sig_keycmp_cc_match)
{
	int			 rc   = -1;
	struct anoubis_sig	*keyA = NULL;
	struct anoubis_sig	*keyB = NULL;

	fail_if(prikey == NULL || infile == NULL,
	    "Error while setup testcase.");

	rc = anoubis_sig_create(&keyA, NULL, certfile, pass_cb);
	fail_if(rc != 0 || keyA == NULL,
	    "Could not load public key (%d / %s).", rc, anoubis_strerror(-rc));

	rc = anoubis_sig_create(&keyB, NULL, certfile, pass_cb);
	fail_if(rc != 0 || keyB == NULL, "Could not load public key (%d / %s).",
	    rc, anoubis_strerror(-rc));

	rc = anoubis_sig_keycmp(keyA, keyB);
	fail_if(rc != 0, "keycmp didn't detect same key pair (%d / %s).",
	    rc, anoubis_strerror(-rc));
}
END_TEST

/* Compare certificate <-> certificate - mismatch */
START_TEST(t_anoubis_sig_keycmp_cc_mismatch)
{
	int			 rc   = -1;
	struct anoubis_sig	*keyA = NULL;
	struct anoubis_sig	*keyB = NULL;

	fail_if(prikey == NULL || infile == NULL,
	    "Error while setup testcase.");

	rc = anoubis_sig_create(&keyA, NULL, certfile, pass_cb);
	fail_if(rc != 0 || keyA == NULL, "Could not load public key (%d / %s).",
	    rc, anoubis_strerror(-rc));

	/* Create different key. */
	libanoubissig_tc_teardown();
	libanoubissig_tc_setup();

	rc = anoubis_sig_create(&keyB, NULL, certfile, pass_cb);
	fail_if(keyB == NULL, "Could not load public key (%d / %s).",
	    rc, anoubis_strerror(-rc));

	rc = anoubis_sig_keycmp(keyA, keyB);
	fail_if(rc == 0, "keycmp didn't detect non equal key pair"
	   " (%d / %s).", rc, anoubis_strerror(-rc));
}
END_TEST


TCase *
libanoubissig_testcase_signature(void)
{
	TCase *testcase = tcase_create("Signature");

	tcase_set_timeout(testcase, 120);
	tcase_add_checked_fixture(testcase, libanoubissig_tc_setup,
	    libanoubissig_tc_teardown);
	tcase_add_test(testcase, sign_and_verify_match_tc);
	tcase_add_test(testcase, sign_and_verify_mismatch_tc);
	tcase_add_test(testcase, t_anoubis_sign_csum);
	tcase_add_test(testcase, t_anoubis_verify_policy);
	tcase_add_test(testcase, t_anoubis_validity_check);

	tcase_add_test(testcase, t_anoubis_sig_keycmp_einval);
	tcase_add_test(testcase, t_anoubis_sig_keycmp_pp_match);
	tcase_add_test(testcase, t_anoubis_sig_keycmp_pp_mismatch);
	tcase_add_test(testcase, t_anoubis_sig_keycmp_pc_match);
	tcase_add_test(testcase, t_anoubis_sig_keycmp_pc_mismatch);
	tcase_add_test(testcase, t_anoubis_sig_keycmp_cc_match);
	tcase_add_test(testcase, t_anoubis_sig_keycmp_cc_mismatch);

	return (testcase);
}

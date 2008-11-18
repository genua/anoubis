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
#include <sys/types.h>

#include <stdarg.h>
#include <check.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>


const char pri[] = "private.pem";
const char pub[] = "public.pem";
const char testfile[] = "data.txt";

char	 workdir[32];
char	*prikey = NULL,
	*pubkey = NULL,
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
	char str[128], syscmd[128];
	int rc;

	va_start(ap, cmd);
	vsnprintf(str, sizeof(str), cmd, ap);
	va_end(ap);

	snprintf(syscmd, sizeof(syscmd), "(%s) >/dev/null 2>&1", str);

	rc = system(syscmd);
	return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

void
libanoubissig_tc_setup(void)
{
	char *s;

	strcpy(workdir, "/tmp/tc_cvs_XXXXXX");
	s = mkdtemp(workdir);

	libanoubissig_tc_exec("openssl genrsa > %s/%s", workdir, pri);
	libanoubissig_tc_exec("openssl rsa -in %s/%s -pubout -out %s/%s",
	    workdir, pri, workdir, pub);
	libanoubissig_tc_exec("echo \"bla\" >  %s/%s", workdir, testfile);
	if(asprintf(&prikey, "%s/%s", workdir, pri) < 0)
		return;
	if (asprintf(&pubkey, "%s/%s", workdir, pub) < 0)
		return;
	if (asprintf(&infile, "%s/%s", workdir, testfile) <0)
		return;
}

void
libanoubissig_tc_teardown(void)
{
	libanoubissig_tc_exec("rm -rf %s", workdir);
	free(prikey);
	free(pubkey);
	free(infile);
}

START_TEST(sign_and_verify_match_tc)
{
	int			 rc;
	unsigned int		 len = 0;
	struct anoubis_sig	*as = NULL;
	unsigned char		 csum[ANOUBIS_SIG_HASH_SHA256_LEN],
				*sign = NULL;

	fail_if(prikey == NULL || pubkey == NULL || infile == NULL,
	    "Error while setup testcase");
	OpenSSL_add_all_algorithms();

	as = anoubis_sig_priv_init(prikey, ANOUBIS_SIG_HASH_SHA1, pass, 0);
	fail_if(as == NULL, "Could not load Private Key");

	rc = libanoubis_tc_calc_sum(infile, csum);
	fail_if(rc == 0, "Could not calculate checksum");

	sign = anoubis_sign_csum(as, csum, &len);
	fail_if(sign == NULL, "Could not sign checksum");

	anoubis_sig_free(as);

	as = anoubis_sig_pub_init(pubkey, ANOUBIS_SIG_HASH_SHA1, pass, 0);
	fail_if(as == NULL, "Could not load Public Key");

	rc = anoubis_verify_csum(as, csum, sign, len);
	anoubis_sig_free(as);
	fail_if(rc == 0, "Signatures dont match");
}
END_TEST

TCase *
libanoubissig_testcase_signature(void)
{
	TCase *testcase = tcase_create("Signature");

	tcase_add_checked_fixture(testcase, libanoubissig_tc_setup,
	    libanoubissig_tc_teardown);
	tcase_add_test(testcase, sign_and_verify_match_tc);

	return (testcase);
}
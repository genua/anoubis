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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <anoubischeck.h>
#include <anoubischat.h>
#include <anoubis_msg.h>
#include <csum.h>

START_TEST(csum_calc_userspace)
{
	const unsigned char sha256[] = {
	    0x2d, 0x2d, 0xa1, 0x96, 0x05, 0xa3, 0x4e, 0x03, 0x7d, 0xbe, 0x82,
	    0x17, 0x3f, 0x98, 0xa9, 0x92, 0xa5, 0x30, 0xa5, 0xfd, 0xd5, 0x3d,
	    0xad, 0x88, 0x2f, 0x57, 0x0d, 0x4b, 0xa2, 0x04, 0xef, 0x30};
	char path[PATH_MAX];
	int fd, result;
	unsigned char csum[ANOUBIS_CS_LEN];
	int csum_len = ANOUBIS_CS_LEN;

	strncpy(path, "/tmp/csum_tc_XXXXXX", PATH_MAX);
	fd = mkstemp(path);
	fail_if(fd == -1, "Failed to create %s: %s",
	    path, strerror(errno));

	result = write(fd, "Hallo Welt", 10);
	fail_unless(result == 10, "Failed to prepare input-file: %s",
	    strerror(errno));
	close(fd);

	result = anoubis_csum_calc_userspace(path, csum, &csum_len);
	fail_unless(result == 0, "Failed to calculate the checksum: %s",
	    strerror(-result));

	fail_unless(csum_len == sizeof(sha256),
	    "Unexpected len of csum: %i != %i", csum_len, sizeof(sha256));

	result = memcmp(sha256, csum, sizeof(sha256));
	fail_unless(result == 0, "Checksum mismatch");

	result = unlink(path);
	fail_unless(result == 0, "Failed to remove %s: %s",
	    path, strerror(errno));
}
END_TEST

START_TEST(csum_calc_userspace_einval)
{
	const char *path = "foo";
	unsigned char csum[ANOUBIS_CS_LEN];
	int csum_len = ANOUBIS_CS_LEN;
	int result;

	result = anoubis_csum_calc_userspace(NULL, csum, &csum_len);
	fail_unless(result == -EINVAL, "Unexpected result: %s",
	    strerror(-result));

	result = anoubis_csum_calc_userspace(path, NULL, &csum_len);
	fail_unless(result == -EINVAL, "Unexpected result: %s",
	    strerror(-result));

	result = anoubis_csum_calc_userspace(path, csum, NULL);
	fail_unless(result == -EINVAL, "Unexpected result: %s",
	    strerror(-result));

	csum_len -= 1;
	result = anoubis_csum_calc_userspace(path, csum, &csum_len);
	fail_unless(result == -EINVAL, "Unexpected result: %s",
	    strerror(-result));
}
END_TEST

START_TEST(csum_calc_userspace_enoent)
{
	const char *path = "foo";
	unsigned char csum[ANOUBIS_CS_LEN];
	int csum_len = ANOUBIS_CS_LEN;
	int result;

	result = anoubis_csum_calc_userspace(path, csum, &csum_len);
	fail_unless(result == -ENOENT, "Unexpected result: %s",
	    strerror(-result));
}
END_TEST

TCase *
csum_tcase(void)
{
	TCase *tc = tcase_create("CsumTcase");

	tcase_add_test(tc, csum_calc_userspace);
	tcase_add_test(tc, csum_calc_userspace_einval);
	tcase_add_test(tc, csum_calc_userspace_enoent);

	return (tc);
}

static Suite*
csum_testsuite(void)
{
	Suite *s = suite_create("CsumSuite");

	suite_add_tcase(s, csum_tcase());

	return (s);
}

int
main (void)
{
	int result;

	SRunner *suiterunner = srunner_create(csum_testsuite());

	srunner_run_all(suiterunner, CK_NORMAL);
	result = check_eval_srunner(suiterunner);
	srunner_free(suiterunner);

	return (result);
}

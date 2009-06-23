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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef GCOV
#include <anoubischat.h>
#include <anoubis_msg.h>
#endif

#include <anoubischeck.h>
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

START_TEST(csum_utils)
{
	int cnt = 0;
	int len = 0;
	char hex[] = "4142A4b5c6d7e8f9";
	unsigned char *res = NULL;

	res = string2hex(hex, &cnt);
	fail_if(res == NULL, "sting2hex failed");
	if (res)
		free(res);
	res = NULL;
	len = strlen(hex) - 1;
	hex[len] = '}';
	res = string2hex(hex, &cnt);
	fail_if(res != NULL, "sting2hex should failed");
	if (res)
		free(res);
	res = NULL;

}
END_TEST

#ifndef GCOV
/* KM: XXX this test needs shared libraries
 * since we can't support that for GCOV
 * we can't run this test with GCVO
 */
START_TEST(csum_list)
{
	struct anoubis_msg *msg = NULL;
	unsigned char **res_list = NULL;
	u_int8_t *keyids = NULL;
	char keyid[] = "0123456789abcdef";
	int *ident_list = NULL;
	int i, len = 0;
	int list_cnt = 0;

	len = strlen(keyid) + 1;
	keyids = calloc(3, len);
	fail_if(keyids == NULL, "Error while alloc memory");

	for(i = 0; i < 3; i++) {
		memcpy(&keyids[i*len],keyid,len);
		keyids[(i*len)+len] = '\0';
	}

	len = len * 3;
	res_list = anoubis_keyid_list(msg, &ident_list, &list_cnt);
	fail_if (res_list != NULL, "anoubis_keyid_list should fail");

	msg = anoubis_msg_new(sizeof(Anoubis_ChecksumPayloadMessage) + len + 1);
	fail_if(msg == NULL, "Could not create new msg");

	msg->length = sizeof(Anoubis_ChecksumPayloadMessage) - 3;
	res_list = anoubis_keyid_list(msg, &ident_list, &list_cnt);
	fail_if(res_list != NULL, "anoubis_keyid_list should fail");

	msg->length = sizeof(Anoubis_ChecksumPayloadMessage) + CSUM_LEN + len;
	memcpy(msg->u.checksumpayload->payload, keyids, len + 1);
	res_list = anoubis_keyid_list(msg, &ident_list, &list_cnt);
	fail_if(res_list == NULL, "anoubis_keyid_list NOT should fail: %d",
	    list_cnt);

}
END_TEST
#endif

START_TEST(csum_link)
{
	char t_dir[] = "/tmp/",
	     t_pre[] = "csum_link";
	char *t_name1 = NULL;
	char *t_name2 = NULL;
	unsigned char csum[ANOUBIS_CS_LEN];
	int fd, rc, len = ANOUBIS_CS_LEN;

	t_name1 = tempnam(t_dir, t_pre);
	t_name2 = tempnam(t_dir, t_pre);
	fail_if(t_name1 == NULL || t_name2 == NULL, "Could not create tmpname");
	fd = creat(t_name1, S_IRWXU|S_IRWXG|S_IRWXO);
	fail_if(fd == -1, "Could not create file: %s", strerror(errno));

	rc = link(t_name1, t_name2);
	fail_if(rc == -1, "Could not link file: %s", strerror(errno));

	rc = anoubis_csum_link_calc(t_name2, csum, &len);
	rc = unlink(t_name2);
	rc = unlink(t_name1);
	fail_if(rc < 0, "Could not calc csum for link %s", strerror(-rc));

	close(fd);
}
END_TEST

START_TEST(csum_prints)
{
	char t_dir[] = "/tmp/";
	char t_pre[] = "csum_prints";
	char *t_nam = NULL;
	FILE *fd = NULL;
	struct sfs_entry *entry = NULL,
			 *list[2];
	int rc = 0,
	    len = 0;
	unsigned char csum[] = "adfasdafiilkjasflkjaslfiiaaaaaj";
	t_nam = tempnam(t_dir, t_pre);
	fail_if(t_nam == NULL, "Could not create tmp name.");

	fd = fopen(t_nam, "w+");
	fail_if(fd == NULL, "Could not open file %s", strerror(errno));

	len = sizeof(csum);
	rc = anoubis_print_checksum(fd, csum, len);
	fail_if(rc != 0, "Fail anoubis_print_checksum: %s", strerror(errno));

	rc = anoubis_print_checksum(fd, csum, -1);
	fail_if(rc == 0, "Fail anoubis_print_checksum should fail");

	rc = anoubis_print_checksum(fd, NULL, len);
	fail_if(rc == 0, "Fail anoubis_print_checksum should fail");

	rc = anoubis_print_checksum(NULL, csum, len);
	fail_if(rc == 0, "Fail anoubis_print_checksum should fail");

	rc = anoubis_print_keyid(fd, csum, len);
	fail_if(rc != 0, "Fail anoubis_print_keyid: %s", strerror(errno));

	rc = anoubis_print_signature(fd, csum, len);
	fail_if(rc != 0, "Fail anoubis_print_keyid: %s", strerror(errno));

	rc = anoubis_print_file(fd, "/p ath/to\n/file");
	fail_if(rc != 0, "Fail anoubis_print_file: %s", strerror(rc));

	rc = anoubis_print_file(NULL, NULL);
	fail_if(rc == 0, "anoubis_print_file should fail");

	entry = anoubis_build_entry(NULL, csum, len, csum, len, 0, csum, len);
	fail_if(entry != NULL, "anoubis_build_entry should fail");

	entry = anoubis_build_entry("asdf", csum, 20, csum, len, 0, csum, len);
	fail_if(entry != NULL, "anoubis_build_entry should fail");

	entry = anoubis_build_entry("asdf", csum, 20, NULL, 0, 0, csum, len);
	fail_if(entry != NULL, "anoubis_build_entry should fail");

	entry = anoubis_build_entry("asdf", csum, len, csum, len, 0, csum, len);
	fail_if(entry == NULL, "Fail anoubis_build_entry");

	entry = anoubis_build_entry("asdf", csum, len, csum, -1, 0, csum, len);
	fail_if(entry == NULL, "anoubis_build_entry fail");

	list[0] = entry;

	rc = anoubis_print_entries(fd, list, 1);
	fail_if(rc != 0, "Fail anoubis_print_entries %s", strerror(rc));

	anoubis_entry_free(entry);
	fclose(fd);
}
END_TEST

TCase *
csum_tcase(void)
{
	TCase *tc = tcase_create("CsumTcase");

	tcase_add_test(tc, csum_calc_userspace);
	tcase_add_test(tc, csum_calc_userspace_einval);
	tcase_add_test(tc, csum_calc_userspace_enoent);
	tcase_add_test(tc, csum_utils);
#ifndef GCOV
	tcase_add_test(tc, csum_list);
#endif
	tcase_add_test(tc, csum_link);
	tcase_add_test(tc, csum_prints);

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

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

#include <config.h>

#include <sys/param.h>

#include <netinet/in.h>

#include <apn.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

char *test = NULL;

char *
generate_sane_file(void)
{
	char	*sfn;
	FILE	*sfp;
	int	 fd;

	if (asprintf(&sfn, "/tmp/errorcodes.XXXXXX") == -1)
		fail_if(1, "asprintf");
	if ((fd = mkstemp(sfn)) == -1)
		fail_if(1, "mkstemp");
	if ((sfp = fdopen(fd, "w+")) == NULL) {
		unlink(sfn);
		fail_if(1, "fdopen");
	}

	/* To avoid races, we keep the file open. */

	fprintf(sfp, "alf {\nany {\n default deny\n}\n}\n");
	fflush(sfp);

	return(sfn);
}


void
handler(int sig)
{
	switch (sig) {
	case SIGSEGV:
	case SIGABRT:
		fail_if(1, "%s crashed", test);
		break;
	}
}

START_TEST(tc_Crash_apn_free_ruleset)
{
	signal(SIGSEGV, handler);
	test = "tc_Crash_apn_free_ruleset";

	apn_free_ruleset(NULL);

	/* If we reach this point, everything's ok. */
	fail_if(0);
}
END_TEST

START_TEST(tc_Crash_parse_files)
{
	struct apn_ruleset	*rs;
	char			*file1, *file2;
	int	 		 ret;

	signal(SIGABRT, handler);
	test = "tc_Crash_parse_files";

	file1 = generate_sane_file();
	file2 = generate_sane_file();

	ret = apn_parse(file1, &rs, APN_FLAG_VERBOSE);
	unlink(file1);
	if (ret != 0)
		apn_print_errors(rs);
	fail_if(ret != 0, "Parsing of file 1 failed");

	rs = NULL;
	ret = apn_parse(file2, &rs, APN_FLAG_VERBOSE);
	unlink(file2);
	if (ret != 0)
		apn_print_errors(rs);
	fail_if(ret != 0, "Parsing of file 2 failed");
}
END_TEST

TCase *
libapn_testcase_crash_apn_free_ruleset(void)
{
	TCase *tc_crash_apn_free_ruleset =
	    tcase_create("Crash apn_free_ruleset(NULL)");
	
	tcase_add_test(tc_crash_apn_free_ruleset, tc_Crash_apn_free_ruleset);

	return (tc_crash_apn_free_ruleset);
}

TCase *
libapn_testcase_crash_parse_files(void)
{
	TCase *tc_crash_parse_files = tcase_create("Crash on parsing more "
	    "than one input file");
	
	tcase_add_test(tc_crash_parse_files, tc_Crash_parse_files);

	return (tc_crash_parse_files);
}

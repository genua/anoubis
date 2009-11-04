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
#include <sys/socket.h>

#include <netinet/in.h>

#include <apn.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

char *
generate_file(void)
{
	char	*sfn;
	FILE	*sfp;
	int	 fd;

	if (asprintf(&sfn, "/tmp/printing.XXXXXX") == -1)
		fail_if(1, "asprintf");
	if ((fd = mkstemp(sfn)) == -1)
		fail_if(1, "mkstemp");
	if ((sfp = fdopen(fd, "w+")) == NULL) {
		unlink(sfn);
		fail_if(1, "fdopen");
	}

	/* To avoid races, we keep the file open. */

	fprintf(sfp, "apnversion 1.1\n");
	fprintf(sfp, "alf {\n");
	fprintf(sfp, "/bin/sh {\n");
	fprintf(sfp, "allow connect tcp all\n");
	fprintf(sfp, "allow accept tcp from any to any\n");
	fprintf(sfp, "allow connect tcp from any to 1.2.3.4\n");
	fprintf(sfp, "allow accept tcp from any to 1.2.3.4 port www\n");
	fprintf(sfp, "allow connect tcp from 1.2.3.4 to any\n");
	fprintf(sfp, "allow accept tcp from 1.2.3.4 port www to any\n");
	fprintf(sfp, "allow connect tcp from 1.2.3.0/24 to 4.0.0.0/8\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "/usr/bin/ssh {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "allow connect tcp all\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "/sbin/dhclient {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "allow raw\n");
	fprintf(sfp, "allow receive udp all\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "\n");
	fprintf(sfp, "/bin/sh uid 10 {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "allow connect tcp from any to 1.2.3.4 port www\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "/usr/bin/ssh self {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "allow connect alert tcp all\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "/sbin/dhclient uid 20 {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "allow raw\n");
	fprintf(sfp, "allow receive log udp all\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "any {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "sfs{\n");
	fprintf(sfp, "path /tmp/blah uid 0 valid allow invalid alert deny "
	    "unknown log continue\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "context{\n");
	fprintf(sfp, "/usr/bin/dings {\n");
	fprintf(sfp, "context new any\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "/bin/bums nosfs {\n");
	fprintf(sfp, "context open any\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "{ /bin/bu, /bin/ms } nosfs {\n");
	fprintf(sfp, "context open any\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "}\n");

	fflush(sfp);

	return(sfn);
}

START_TEST(tc_Ruleset)
{
	struct apn_ruleset	*rs;
	int			 ret;
	char			*filename;

	filename = generate_file();
	ret = apn_parse(filename, &rs, 0);

	unlink(filename);	/* File will disappear on exit(2). */

	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "apn_parse() failed");

	ret = apn_print_ruleset(rs, APN_FLAG_VERBOSE, stdout);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "apn_print_ruleset() failed");
}
END_TEST

TCase *
printing_testcase_ruleset(void)
{
	/* sessions test case */
	TCase *tc_ruleset = tcase_create("Print full ruleset");

	tcase_add_test(tc_ruleset, tc_Ruleset);

	return (tc_ruleset);
}

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

	if (asprintf(&sfn, "/tmp/errorcodes.XXXXXX") == -1)
		fail_if(1, "asprintf");
	if ((fd = mkstemp(sfn)) == -1)
		fail_if(1, "mkstemp");
	if ((sfp = fdopen(fd, "w+")) == NULL) {
		unlink(sfn);
		fail_if(1, "fdopen");
	}

	/* To avoid races, we keep the file open. */

	fprintf(sfp, "foobar");
	fflush(sfp);

	return(sfn);
}

START_TEST(tc_Errorcodes)
{
	struct apn_ruleset	*rs;
	int			 ret;
	char			*filename;

	filename = generate_file();
	ret = apn_parse(filename, &rs, APN_FLAG_VERBOSE);

	unlink(filename);	/* File will disappear on exit(2). */

	if (ret != 0)
		apn_print_errors(rs);

	fail_if(ret != 1, "apn_parse() did not return required value \"1\"");
}
END_TEST

TCase *
libapn_testcase_errorcodes(void)
{
	/* sessions test case */
	TCase *tc_errorcodes = tcase_create("Error codes");

	tcase_add_test(tc_errorcodes, tc_Errorcodes);

	return (tc_errorcodes);
}

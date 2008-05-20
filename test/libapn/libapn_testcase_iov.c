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
#include <sys/stat.h>

#ifdef OPENBSD
#include <sys/uio.h>
#endif

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

static char *
generate_file(FILE **infp)
{
	char	*sfn;
	FILE	*sfp;
	int	 fd;

	if (asprintf(&sfn, "/tmp/iovin.XXXXXX") == -1)
		fail_if(1, "asprintf");
	if ((fd = mkstemp(sfn)) == -1)
		fail_if(1, "mkstemp");
	if ((sfp = fdopen(fd, "w+")) == NULL) {
		unlink(sfn);
		fail_if(1, "fdopen");
	}

	/* To avoid races, we keep the file open. */

	fprintf(sfp, "alf {\n");
	fprintf(sfp, "/bin/sh sha256 \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "allow connect tcp all\n");
	fprintf(sfp, "allow accept tcp from any to any\n");
	fprintf(sfp, "allow connect tcp from any to 1.2.3.4\n");
	fprintf(sfp, "allow accept tcp from any to 1.2.3.4 port www\n");
	fprintf(sfp, "allow connect tcp from 1.2.3.4 to any\n");
	fprintf(sfp, "allow accept tcp from 1.2.3.4 port www to any\n");
	fprintf(sfp, "allow connect tcp from 1.2.3.0/24 to 4.0.0.0/8\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "context new any\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "/usr/bin/ssh sha256 \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "allow connect tcp all\n");
	fprintf(sfp, "context new { /sbin/dhclient sha256 \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef,\n");
	fprintf(sfp, "/bin/sh \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef }\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "/sbin/dhclient sha256 \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "allow raw\n");
	fprintf(sfp, "allow send udp all\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "\n");
	fprintf(sfp, "/bin/sh \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "context new any\n");
	fprintf(sfp, "allow connect tcp from any to 1.2.3.4 port www\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "/usr/bin/ssh \\\n");	/* This rule has ID 24 */
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "default deny\n");		/* This is rule 22 */
	fprintf(sfp, "allow connect alert tcp all\n"); /* This is rule 23 */
	fprintf(sfp, "}\n");
	fprintf(sfp, "/sbin/dhclient \\\n");
	fprintf(sfp, "0123456789abcdef0123456789abcdef0123456789abcdef"
	    "0123456789abcdef {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "allow raw\n");
	fprintf(sfp, "allow send log udp all\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "any {\n");
	fprintf(sfp, "default deny\n");
	fprintf(sfp, "context new any\n");
	fprintf(sfp, "}\n");
	fprintf(sfp, "}\n");

	fflush(sfp);
	(*infp) = sfp;
	return(sfn);
}

void dump_rules(struct apn_ruleset *rs, char **bufp, int *lenp)
{
	char		*sfn;
	FILE		*sfp;
	int		 ret, fd;
	struct stat	 statbuf;
	char		*buf;

	(*bufp) = NULL;
	(*lenp) = 0;
	if (asprintf(&sfn, "/tmp/iotmp.XXXXXX") == -1)
		fail_if(1, "asprintf");
	if ((fd = mkstemp(sfn)) == -1)
		fail_if(1, "mkstemp");
	if ((sfp = fdopen(fd, "w+")) == NULL) {
		unlink(sfn);
		fail_if(1, "fdopen");
	}
	unlink(sfn);
	free(sfn);
	ret = apn_print_ruleset(rs, 0, sfp);
	fail_if(ret != 0, "dump_rules");
	fflush(sfp);
	rewind(sfp);
	ret = fstat(fd, &statbuf);
	fail_if(ret < 0, "fstat");
	buf = malloc(statbuf.st_size);
	fail_if(buf == NULL, "malloc");
	ret = fread(buf, 1, statbuf.st_size, sfp);
	fail_if(ret != statbuf.st_size, "fread");
	fclose(sfp);
	(*bufp) = buf;
	(*lenp) = statbuf.st_size;
}

START_TEST(tc_iovec)
{
	struct apn_ruleset	*rs;
	char			*file;
	int			 ret;
	FILE			*fp;
	char			*ref;
	int			 reflen, i, j;
	struct iovec		*iov;

	file = generate_file(&fp);
	ret = apn_parse(file, &rs, 0);
	unlink(file);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	fail_if(ret != 0, "Parsing failed");
	dump_rules(rs, &ref, &reflen);
	apn_free_ruleset(rs);

	srand(0);
	for (i=0; i<500; ++i) {
		int	 this, iovcnt, total;
		char	*buf;

		rewind(fp);
		iovcnt = 0;
		iov = NULL;
		total = 0;
		rs = NULL;

		/* Read file into an iovec with random sized chunks. */
		while(1)  {
			this = rand() % 100;
			if (this) {
				buf = malloc(this);
				fail_if(buf == NULL, "malloc");
				this = fread(buf, 1, this, fp);
				if (this == 0)
					break;
			}
			iovcnt++;
			iov = realloc(iov, sizeof(struct iovec)*iovcnt);
			iov[iovcnt-1].iov_len = this;
			iov[iovcnt-1].iov_base = this?buf:NULL;
			total += this;
		}
		ret = apn_parse_iovec("<iov>", iov, iovcnt, &rs, 0);
		fail_if(ret != 0, "apn_parse_iov");
		dump_rules(rs, &buf, &this);
		apn_free_ruleset(rs);
		fail_if(this != reflen, "iovec gave different ruleset lenght");
		fail_if(memcmp(buf, ref, reflen) != 0,
		    "buffer contents form iovec differ from reference");
		for(j=0; j<iovcnt; ++j)
			free(iov[j].iov_base);
		free(iov);
	}
}
END_TEST

TCase *
libapn_testcase_iovec(void)
{
	/* sessions test case */
	TCase *testcase_iovec = tcase_create("iovec testcase");

	tcase_add_test(testcase_iovec, tc_iovec);

	return (testcase_iovec);
}


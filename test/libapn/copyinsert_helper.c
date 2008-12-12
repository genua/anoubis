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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <apn.h>

void usage(const char *progname)
{
	fprintf(stderr, "usage: %s rulefile type triggerid srcid\n", progname);
	exit(1);
}

int main(int argc, char **argv)
{
	struct apn_ruleset	*rs;
	int			 triggerid, srcid, ret, type = 0;
	char			 ch;
	static const char	 path[] = "/bin/foobar";
	static u_int8_t		 fakecs[64];
	struct apn_rule		*rule = NULL;

	memset(fakecs, 0xa0, sizeof(fakecs));
	if (argc != 5)
		usage(argv[0]);
	if (sscanf(argv[3], "%d%c", &triggerid, &ch) != 1)
		usage(argv[0]);
	if (sscanf(argv[4], "%d%c", &srcid, &ch) != 1)
		usage(argv[0]);
	if (strcmp(argv[2], "alf") == 0) {
		type = APN_ALF;
	} else if (strcmp(argv[2], "sb") == 0
	    || strcmp(argv[2], "sandbox") == 0) {
		type = APN_SB;
	} else if (strcmp(argv[2], "ctx") == 0
	    || strcmp(argv[2], "context") == 0) {
		type = APN_CTX;
	} else {
		usage(argv[0]);
	}
	ret = apn_parse(argv[1], &rs, APN_FLAG_NOPERMCHECK);
	if (ret != 0)
		apn_print_errors(rs, stderr);
	assert(ret == 0);
	rule = apn_find_rule(rs, srcid);
	assert(rule);
	rule = apn_copy_one_rule(rule);
	assert(rule);
	switch(type) {
	case APN_ALF:
		ret = apn_copyinsert_alf(rs, rule, triggerid, path, fakecs,
		    APN_HASH_SHA256);
		break;
	case APN_SB:
		ret = apn_copyinsert_sb(rs, rule, triggerid, path, fakecs,
		    APN_HASH_SHA256);
		break;
	case APN_CTX:
		ret = apn_copyinsert_ctx(rs, rule, triggerid, path, fakecs,
		    APN_HASH_SHA256);
		break;
	default:
		assert(0);
	}
	assert(ret == 0);
	apn_print_ruleset(rs, 0, stdout);
	return 0;
}

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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <apn.h>

void usage(const char *progname)
{
	fprintf(stderr, "usage: %s cmdfile input\n", progname);
	exit(1);
}

static int		 line = 0;

static int
cmd_updown_common(struct apn_ruleset *rs, char *args,
    int (*func)(struct apn_rule *), int boolret)
{
	int			 id, ret;
	char			 ch;
	struct apn_rule		*rule;

	if (sscanf(args, "%d%c", &id, &ch) != 1) {
		fprintf(stderr, "cmd_updown_common(%d): Invalid syntax\n",
		    line);
		return 0;
	}
	rule = apn_find_rule(rs, id);
	if (!rule) {
		fprintf(stderr, "cmd_updown_common(%d): Cannot find rule %d\n",
		    line, id);
		return 0;
	}
	ret = func(rule);
	if ((boolret && ret == 0) || (!boolret && ret != 0)) {
		fprintf(stderr, "cmd_updown_common(%d): apn function "
		    "failed with %d\n", line, ret);
		return 0;
	}
	return 1;
}

static int
cmd_print(struct apn_ruleset *rs, char *args)
{
	if (*args) {
		fprintf(stderr, "cmd_print(%d): "
		    "Unexpected arguments on print\n", line);
		return 0;
	}
	if (apn_print_ruleset(rs, 0, stdout) == 0) {
		printf("-------------------------------------------------\n");
		return 1;
	}
	fprintf(stderr, "cmd_print(%d): apn_print_ruleset failed\n", line);
	return 0;
}

static int
get_type(const char *type)
{
	if (strcasecmp(type, "alf") == 0)
		return APN_ALF;
	if (strcasecmp(type, "sfs") == 0)
		return APN_SFS;
	if (strcasecmp(type, "sb") == 0)
		return APN_SB;
	if (strcasecmp(type, "sandbox") == 0)
		return APN_SB;
	if (strcasecmp(type, "context") == 0)
		return APN_CTX;
	if (strcasecmp(type, "ctx") == 0)
		return APN_CTX;
	fprintf(stderr, "get_type(%d): %s not a valid type\n", line, type);
	return -1;
}

static int
xdigit(char ch)
{
	if (isdigit(ch))
		return ch - '0';
	if ('a' <= ch && ch <= 'z')
		return 10 + ch - 'a';
	if ('A' <= ch && ch <= 'Z')
		return 10 + ch - 'A';
	return -1;
}

static int
parse_csum(char *string, u_int8_t cs[ANOUBIS_CS_LEN])
{
	int i;
	for (i=0; i<ANOUBIS_CS_LEN; ++i) {
		int d1, d2;
		if ((d1 = xdigit(string[2*i])) < 0)
			return 0;
		if ((d2 = xdigit(string[2*i+1])) < 0)
			return 0;
		cs[i] = d1*16 + d2;
	}
	if (string[2*i])
		return 0;
	return 1;
}

static int
cmd_copyinsert(struct apn_ruleset *rs, char *args)
{
	int			 triggerid, srcid, type, ret;
	char			 ch;
	char			 typestr[100], cstxt[256], path[1024];
	u_int8_t		 cs[ANOUBIS_CS_LEN];
	struct apn_rule		*src;

	if (sscanf(args, "%s %d %d %s %s%c",
	    typestr, &triggerid, &srcid, path, cstxt, &ch) != 5) {
		fprintf(stderr, "cmd_copyinsert(%d): Invald syntax\n", line);
		return 0;
	}
	if (!parse_csum(cstxt, cs)) {
		fprintf(stderr, "cmd_copyinsert(%d): Invalid csum\n", line);
		return 0;
	}
	type = get_type(typestr);
	if (type < 0)
		return 0;
	src = apn_find_rule(rs, srcid);
	if (src == NULL) {
		fprintf(stderr, "cmd_copyinsert(%d): Cannot find rule %d\n",
		    line, srcid);
		return 0;
	}
	src = apn_copy_one_rule(src);
	if (src == NULL) {
		fprintf(stderr, "cmd_copyinsert(%d): Cannot copy src rule\n",
		    line);
		return 0;
	}
	switch(type) {
	case APN_ALF:
		ret = apn_copyinsert_alf(rs, src, triggerid, path, cs,
		    APN_HASH_SHA256);
		break;
	case APN_SB:
		ret = apn_copyinsert_sb(rs, src, triggerid, path, cs,
		    APN_HASH_SHA256);
		break;
	case APN_CTX:
		ret = apn_copyinsert_ctx(rs, src, triggerid, path, cs,
		    APN_HASH_SHA256);
		break;
	default:
		fprintf(stderr, "cmd_copyinsert(%d): Bad type %s/%d\n",
		    line, typestr, type);
		return 0;
	}
	if (ret != 0) {
		fprintf(stderr, "cmd_copyinsert(%d): copyinsert failed "
		    "with %d\n", line, ret);
		return 0;
	}
	return 1;
}

int main(int argc, char **argv)
{
	struct apn_ruleset	*rs;
	char			 l[1024];
	FILE			*cmd;
	int			 ret;

	if (argc != 3)
		usage(argv[0]);

	ret = apn_parse(argv[2], &rs, APN_FLAG_NOPERMCHECK);
	if (ret != 0) {
		apn_print_errors(rs, stderr);
		fprintf(stderr, "Could not parse %s\n", argv[2]);
		return 1;
	}
	cmd = fopen(argv[1], "r");
	if (!cmd) {
		fprintf(stderr, "Could not open %s\n", argv[1]);
		return 1;
	}
	while (1) {
		int	 i, fail, ret;
		char	*p, *args;
		if (fgets(l, sizeof(l), cmd) == NULL)
			break;
		line++;
		i = strlen(l);
		while (i > 0) {
			if (!isspace(l[i-1]))
				break;
			i--;
		}
		if (i == 0)
			continue;
		l[i] = 0;
		p = l;
		while(isspace(*p))
			p++;
		if (*p == '#')
			continue;
		fail = 0;
		if (*p == '!') {
			fail = 1;
			p++;
			while(isspace(*p))
			p++;
		}
		args = p;
		while(*args && !isspace(*args))
			args++;
		if (*args != 0) {
			*args = 0;
			args++;
		}
		while(isspace(*args))
			args++;
		if (strcasecmp(p, "print") == 0) {
			ret = cmd_print(rs, args);
		} else if (strcasecmp(p, "copyinsert") == 0) {
			ret = cmd_copyinsert(rs, args);
		} else if (strcasecmp(p, "up") == 0) {
			ret = cmd_updown_common(rs, args, &apn_move_up, 0);
		} else if (strcasecmp(p, "down") == 0) {
			ret = cmd_updown_common(rs, args, &apn_move_down, 0);
		} else if (strcasecmp(p, "can_up") == 0) {
			ret = cmd_updown_common(rs, args, &apn_can_move_up, 1);
		} else if (strcasecmp(p, "can_down") == 0) {
			ret = cmd_updown_common(rs, args,
			    &apn_can_move_down, 1);
		} else {
			fprintf(stderr, "line %d: Unknown command %s\n",
			    line, p);
			return 1;
		}
		if (ret && fail) {
			fprintf(stderr, "line %d: Operation did not fail\n",
			    line);
			return 1;
		}
		if (!ret && !fail) {
			fprintf(stderr, "%d: Operation did not succeed\n",
			    line);
			return 1;
		}
	}
	return 0;
}

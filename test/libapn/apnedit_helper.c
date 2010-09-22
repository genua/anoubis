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
#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <apn.h>
#include <anoubis_protocol.h>
#include <anoubischeck.h>

FILE	*output = NULL;

void usage(const char *progname)
{
	if (!output)
		output = stderr;
	fprintf(output, "usage: %s cmdfile input\n", progname);
	exit(1);
}

static int		 line = 0;

#define SKIP(P) while(isspace(*(P))) { (P)++; }

static char *
token(char **p)
{
	char	*ret = *p, *last;
	SKIP(ret);
	if (*ret == 0)
		return NULL;
	last = ret;
	while(*last && !isspace(*last))
		last++;
	if (*last) {
		*last = 0;
		last++;
	} else {
		*last = 0;
	}
	SKIP(last);
	(*p) = last;
	return ret;
}

struct strtab_entry {
	const char *key;
	int value;
};

static int
strtab_lookup(struct strtab_entry tab[], const char *key)
{
	int	i;
	for (i=0; tab[i].key; ++i) {
		if (strcasecmp(tab[i].key, key) == 0)
			return tab[i].value;
	}
	return -1;
}

static struct strtab_entry protos[] = {
	{ "tcp", IPPROTO_TCP },
	{ "udp", IPPROTO_UDP },
	{ "icmp", IPPROTO_ICMP },
	{ NULL, 0 }
};

static struct strtab_entry families[] = {
	{ "v4", AF_INET },
	{ "v6", AF_INET6 },
	{ NULL, 0 }
};

static struct strtab_entry ops[] = {
	{ "connect", ALF_CONNECT },
	{ "accept", ALF_ACCEPT },
	{ "send", ALF_SENDMSG },
	{ "receive", ALF_RECVMSG },
	{ NULL, 0 }
};

static struct strtab_entry alfflags[] = {
	{ "port", ALF_EV_ALLPORT },
	{ "peer", ALF_EV_ALLPEER },
	{ "dir", ALF_EV_ALLDIR },
	{ "proto", ALF_EV_ALLPROTO },
	{ NULL, 0 }
};

static struct strtab_entry sbflags[] = {
	{ "read", APN_SBA_READ },
	{ "write", APN_SBA_WRITE },
	{ "exec", APN_SBA_EXEC },
	{ NULL, 0 }
};

static struct strtab_entry sfsflags[] = {
	{ "sfsvalid", (ANOUBIS_SFS_VALID) },
	{ "sfsinvalid", (ANOUBIS_SFS_INVALID) },
	{ "sfsunknown", (ANOUBIS_SFS_UNKNOWN) },
	{ "sfsdefault", (ANOUBIS_SFS_DEFAULT) },
	{ NULL, 0 }
};

int
fill_alf_event(struct alf_event *ret, char *evstr)
{
	char			*p = evstr, *tok;
	int			 tmp;
	int			 gotlocal = 0, gotremote = 0;

	SKIP(p);
	if (strncasecmp(p, "alf[", 4) != 0)
		return 0;
	p += 4;
	/* Protocol */
	tok = token(&p);
	if (!tok)
		return 0;
	tmp = strtab_lookup(protos, tok);
	if (tmp < 0)
		return 0;
	ret->protocol = tmp;

	/* Family */
	tok = token(&p);
	if (!tok)
		return 0;
	tmp = strtab_lookup(families, tok);
	if (tmp < 0)
		return 0;
	ret->family = tmp;

	/* Operation */
	tok = token(&p);
	if (!tok)
		return 0;
	tmp = strtab_lookup(ops, tok);
	if (tmp < 0)
		return 0;
	ret->op = tmp;

	while (!gotlocal || !gotremote) {
		struct sockaddr		*addr;
		char			*portp;
		int			 port;
		char			 ch;

		tok = token(&p);
		if (!tok)
			return 0;
		addr = NULL;
		if (strcasecmp(tok, "local") == 0) {
			if (gotlocal)
				return 0;
			gotlocal = 1;
			addr = (struct sockaddr *)&ret->local;
		} else if (strcasecmp(tok, "remote") == 0) {
			if (gotremote)
				return 0;
			gotremote = 1;
			addr = (struct sockaddr *)&ret->peer;
		} else {
			return 0;
		}
		tok = token(&p);
		if (!tok)
			return 0;
		portp = token(&p);
		if (!portp)
			return 0;
		if (sscanf(portp, "%d%c", &port, &ch) != 1 || port < 0)
			return 0;
		if (ret->family == AF_INET) {
			struct sockaddr_in	*maddr;
			maddr = (struct sockaddr_in *)addr;
			maddr->sin_family = AF_INET;
			if (inet_pton(AF_INET, tok, &maddr->sin_addr) <= 0)
				return 0;
			maddr->sin_port = htons(port);
		} else if (ret->family == AF_INET6) {
			struct sockaddr_in6	*maddr;
			maddr = (struct sockaddr_in6 *)addr;
			maddr->sin6_family = AF_INET6;
			if (inet_pton(AF_INET6, tok, &maddr->sin6_addr) <= 0)
				return 0;
			maddr->sin6_port = htons(port);
		} else {
			return 0;
		}
	}
	SKIP(p);
	if (*p)
		return 0;
	return 1;
}

static int
cmd_updown_common(struct apn_ruleset *rs, char *args,
    int (*func)(struct apn_rule *), int boolret)
{
	int			 id, ret;
	char			 ch;
	struct apn_rule		*rule;

	if (sscanf(args, "%d%c", &id, &ch) != 1) {
		fprintf(output, "cmd_updown_common(%d): Invalid syntax\n",
		    line);
		return 0;
	}
	rule = apn_find_rule(rs, id);
	if (!rule) {
		fprintf(output, "cmd_updown_common(%d): Cannot find rule %d\n",
		    line, id);
		return 0;
	}
	ret = func(rule);
	if ((boolret && ret == 0) || (!boolret && ret != 0)) {
		fprintf(output, "cmd_updown_common(%d): apn function "
		    "failed with %d\n", line, ret);
		return 0;
	}
	return 1;
}

static int
cmd_print(struct apn_ruleset *rs, char *args)
{
	if (*args) {
		fprintf(output, "cmd_print(%d): "
		    "Unexpected arguments on print\n", line);
		return 0;
	}
	if (apn_print_ruleset(rs, 0, stdout) == 0) {
		printf("-------------------------------------------------\n");
		return 1;
	}
	fprintf(output, "cmd_print(%d): apn_print_ruleset failed\n", line);
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
	fprintf(output, "get_type(%d): %s not a valid type\n", line, type);
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

char *
parse_keyid(char *string)
{
	int		 i;

	for (i=0; string[2*i] && string[2*i+1]; ++i) {
		if (xdigit(string[2*i]) < 0)
			return NULL;
		if (xdigit(string[2*i+1]) < 0)
			return NULL;
	}
	if (string[2*i])
		return NULL;
	return strdup(string);
}

static int
parse_subject(char *args, struct apn_subject *subject)
{
	int	ret = -1;
	int	uid;
	char	keyid[200];

	while(isspace(*args))
		args++;
	memset(subject, 0, sizeof(struct apn_subject));
	subject->type = APN_CS_NONE;
	if (strncasecmp(args, "signed-self", 11) == 0) {
		ret = 11;
		subject->type = APN_CS_UID_SELF;
	} else if (strncasecmp(args, "self", 4) == 0) {
		ret = 4;
		subject->type = APN_CS_KEY_SELF;
	} else if (sscanf(args, "uid %d%n", &uid, &ret) >= 1) {
		subject->type = APN_CS_UID;
		subject->value.uid = uid;
	} else if (sscanf(args, "key %s%n", keyid, &ret) >= 1) {
		subject->type = APN_CS_KEY;
		subject->value.keyid = parse_keyid(keyid);
		if (subject->value.keyid == NULL)
			return -1;
	} else {
		subject->type = APN_CS_NONE;
		return 0;
	}
	if (ret < 0 || (args[ret] && !isspace(args[ret])))
		return -1;
	return ret;
}

static int
cmd_copyinsert(struct apn_ruleset *rs, char *args)
{
	int			 triggerid, srcid, type, ret;
	char			 typestr[100], path[1024];
	struct apn_rule		*src;
	struct apn_subject	 subject;
	int			 len;

	if (sscanf(args, "%s %d %d %s %n",
	    typestr, &triggerid, &srcid, path, &len) < 4) {
		fprintf(output, "cmd_copyinsert(%d): Invald syntax\n", line);
		return 0;
	}
	args += len;
	len = parse_subject(args, &subject);
	if (len < 0) {
		fprintf(output, "cmd_copyinsert(%d): Invalid subject\n", line);
		return 0;
	}
	args += len;
	while(isspace(*args))
		args++;
	if (*args) {
		fprintf(output, "cmd_copyinsert(%d): Garbage after subject\n",
		    line);
		return 0;
	}
	type = get_type(typestr);
	if (type < 0)
		return 0;
	src = apn_find_rule(rs, srcid);
	if (src == NULL) {
		fprintf(output, "cmd_copyinsert(%d): Cannot find rule %d\n",
		    line, srcid);
		return 0;
	}
	src = apn_copy_one_rule(src);
	if (src == NULL) {
		fprintf(output, "cmd_copyinsert(%d): Cannot copy src rule\n",
		    line);
		return 0;
	}
	switch(type) {
	case APN_ALF:
		ret = apn_copyinsert_alf(rs, src, triggerid, path, &subject);
		break;
	case APN_SB:
		ret = apn_copyinsert_sb(rs, src, triggerid, path, &subject);
		break;
	case APN_CTX:
		ret = apn_copyinsert_ctx(rs, src, triggerid, path, &subject);
		break;
	default:
		fprintf(output, "cmd_copyinsert(%d): Bad type %s/%d\n",
		    line, typestr, type);
		return 0;
	}
	if (ret != 0) {
		fprintf(output, "cmd_copyinsert(%d): copyinsert failed "
		    "with %d\n", line, ret);
		return 0;
	}
	return 1;
}

static int
parse_escalation_options(char *args, struct apn_default *action,
    unsigned int *flagsp, struct strtab_entry tab[], int oneflag)
{
	char		*tok;
	unsigned int	 flags = 0;

	SKIP(args);
	action->action = APN_ACTION_ASK;
	action->log = APN_LOG_NONE;
	while(*args) {
		int opt;
		tok = token(&args);
		if (!tok)
			break;
		if (action->action == APN_ACTION_ASK) {
			if (strcasecmp(tok, "deny") == 0) {
				action->action = APN_ACTION_DENY;
				continue;
			}
			if (strcasecmp(tok, "allow") == 0) {
				action->action = APN_ACTION_ALLOW;
				continue;
			}
		}
		if (action->log == APN_LOG_NONE) {
			if (strcasecmp(tok, "log") == 0) {
				action->log = APN_LOG_NORMAL;
				continue;
			}
			if (strcasecmp(tok, "alert") == 0) {
				action->log = APN_LOG_ALERT;
				continue;
			}
		}
		opt = strtab_lookup(tab, tok);
		if (opt < 0)
			return 0;
		if (oneflag) {
			if (flags)
				return 0;
			flags = opt;
		} else {
			if (flags & opt)
				return 0;
			flags |= opt;
		}
		SKIP(args);
	}
	if (action->action == APN_ACTION_ASK)
		return 0;
	if (oneflag && !flags)
		return 0;
	(*flagsp) = flags;
	return 1;
}

static int
cmd_sfs_escalation(struct apn_ruleset *rs, char *args)
{
	const char		*tok, *prefix;
	int			 triggerid, ret;
	char			 ch;
	struct apn_default	 action;
	struct apn_chain	 nchain;
	struct apn_rule		*triggerrule;
	unsigned int		 sfsmatch;

	tok = token(&args);
	if (!tok || sscanf(tok, "%d%c", &triggerid, &ch) != 1)
		goto parse_error;
	prefix = token(&args);
	if (!prefix)
		goto parse_error;
	action.action = APN_ACTION_ASK;
	action.log = APN_LOG_NONE;
	SKIP(args);
	if (!parse_escalation_options(args, &action, &sfsmatch, sfsflags, 1))
		goto parse_error;
	TAILQ_INIT(&nchain);
	triggerrule = apn_find_rule(rs, triggerid);
	if (!triggerrule) {
		fprintf(output, "cmd_sfs_escalation(%d): "
		    "Cannot find rule %d\n", line, triggerid);
		return 0;
	}
	ret = apn_escalation_rule_sfs(&nchain, triggerrule, &action,
	    prefix, sfsmatch);
	if (ret < 0) {
		fprintf(output, "cmd_sfs_escalation(%d): "
		    "Cannot create escalation rules (%d)\n", line, -ret);
		return 0;
	}
	if (apn_escalation_splice(rs, triggerrule, &nchain) < 0) {
		fprintf(output, "cmd_sfs_escalation(%d): "
		    "Cannot splice rules\n", line);
		return 0;
	}
	return 1;
parse_error:
	fprintf(output, "cmd_sfs_esclation(%d): Syntax error\n", line);
	return 0;
}

static int
cmd_sb_escalation(struct apn_ruleset *rs, char *args)
{
	const char		*tok, *prefix;
	int			 triggerid, ret;
	char			 ch;
	struct apn_default	 action;
	unsigned int		 flags;
	struct apn_chain	 nchain;
	struct apn_rule		*triggerrule;

	tok = token(&args);
	if (!tok || sscanf(tok, "%d%c", &triggerid, &ch) != 1)
		goto parse_error;
	prefix = token(&args);
	if (!prefix)
		goto parse_error;
	action.action = APN_ACTION_ASK;
	action.log = APN_LOG_NONE;
	SKIP(args);
	if (!parse_escalation_options(args, &action, &flags, sbflags, 0))
		goto parse_error;
	TAILQ_INIT(&nchain);
	triggerrule = apn_find_rule(rs, triggerid);
	if (!triggerrule) {
		fprintf(output, "cmd_sb_escalation(%d): "
		    "Cannot find rule %d\n", line, triggerid);
		return 0;
	}
	ret = apn_escalation_rule_sb(&nchain, triggerrule, &action,
	    prefix, flags);
	if (ret < 0) {
		fprintf(output, "cmd_sb_escalation(%d): "
		    "Cannot create escalation rules (%d)\n", line, -ret);
		return 0;
	}
	if (apn_escalation_splice(rs, triggerrule, &nchain) < 0) {
		fprintf(output, "cmd_sb_escalation(%d): "
		    "Cannot splice rules\n", line);
		return 0;
	}
	return 1;
parse_error:
	fprintf(output, "cmd_sb_esclation(%d): Syntax error\n", line);
	return 0;
}

static int
cmd_alf_escalation(struct apn_ruleset *rs, char *args)
{
	struct alf_event	 event;
	int			 len, ruleid, ret;
	unsigned int		 flags;
	struct apn_chain	 nchain;
	struct apn_default	 action;
	struct apn_rule		*rule;

	SKIP(args);
	len = 0;
	while(args[len]) {
		if (args[len] == ']')
			break;
		len++;
	}
	if (args[len] == 0)
		goto parse_error;
	args[len] = 0;
	if (!fill_alf_event(&event, args))
		goto parse_error;
	args += len+1;
	if (sscanf(args, "%d%n", &ruleid, &len) < 1)
		goto parse_error;
	args += len;
	SKIP(args);
	if (!parse_escalation_options(args, &action, &flags, alfflags, 0))
		goto parse_error;
	TAILQ_INIT(&nchain);
	rule = apn_find_rule(rs, ruleid);
	if (!rule) {
		fprintf(output, "cmd_alf_escalation(%d): "
		    "Cannot find rule %d\n", line, ruleid);
		return 0;
	}
	ret = apn_escalation_rule_alf(&nchain, &event, &action, flags);
	if (ret < 0) {
		fprintf(output, "cmd_alf_escalation(%d): "
		    "Cannot create escalation rules (%d)\n", line, -ret);
		return 0;
	}
	if (apn_escalation_splice(rs, rule, &nchain) < 0) {
		fprintf(output, "cmd_alf_escalation(%d): "
		    "Cannot splice rules\n", line);
		return 0;
	}
	return 1;
parse_error:
	fprintf(output, "cmd_alf_escalation(%d): "
	    "Parse Error in command line\n", line);
	return 0;
}

static int
cmd_insert(struct apn_ruleset *rs, char *args)
{
	char			 ch;
	char			 typestr[100];
	int			 anchor, srcid, type, ret;
	struct apn_rule		*src;

	if (sscanf(args, "%s %d %d%c", typestr, &anchor, &srcid, &ch) != 3) {
		fprintf(output, "cmd_insert(%d): Invald syntax\n", line);
		return 0;
	}
	type = get_type(typestr);
	if (type < 0)
		return 0;
	src = apn_find_rule(rs, srcid);
	if (src == NULL) {
		fprintf(output, "cmd_insert(%d): Cannot find rule %d\n",
		    line, srcid);
		return 0;
	}
	src = apn_copy_one_rule(src);
	if (src == NULL) {
		fprintf(output, "cmd_insert(%d): Cannot copy src rule\n",
		    line);
		return 0;
	}
	src->apn_id = 0;
	switch(type) {
	case APN_ALF:
		ret = apn_insert_alfrule(rs, src, anchor);
		break;
	case APN_SB:
		ret = apn_insert_sbrule(rs, src, anchor);
		break;
	case APN_SFS:
		ret = apn_insert_sfsrule(rs, src, anchor);
		break;
	case APN_CTX:
		ret = apn_insert_ctxrule(rs, src, anchor);
		break;
	default:
		fprintf(output, "cmd_insert(%d): Bad type %s/%d\n",
		    line, typestr, type);
		return 0;
	}
	if (ret != 0) {
		fprintf(output, "cmd_insert(%d): insert failed with %d\n",
		    line, ret);
		return 0;
	}
	return 1;
}

static int
cmd_remove(struct apn_ruleset *rs, char *args)
{
	int		id, ret;
	char		ch;

	if (sscanf(args, "%d%c", &id, &ch) != 1) {
		fprintf(output, "cmd_remove(%d): Invalid syntax\n", line);
		return 0;
	}
	ret = apn_remove(rs, id);
	return (ret == 0);
}

int main(int argc, char **argv)
{
	struct apn_ruleset	*rs;
	char			 l[1024];
	FILE			*cmd, *null;
	int			 ret;

	if (argc != 3)
		usage(argv[0]);
	null = fopen("/dev/null", "w");
	if (!null) {
		fprintf(stderr, "Cannot open /dev/null");
		return 1;
	}

	ret = apn_parse(argv[2], &rs, 0);
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
		if (fail) {
			/* Suppress error output for expected fails. */
			output = null;
		} else {
			output = stderr;
		}
		if (strcasecmp(p, "print") == 0) {
			ret = cmd_print(rs, args);
		} else if (strcasecmp(p, "copyinsert") == 0) {
			ret = cmd_copyinsert(rs, args);
		} else if (strcasecmp(p, "insert") == 0) {
			ret = cmd_insert(rs, args);
		} else if (strcasecmp(p, "up") == 0) {
			ret = cmd_updown_common(rs, args, &apn_move_up, 0);
		} else if (strcasecmp(p, "down") == 0) {
			ret = cmd_updown_common(rs, args, &apn_move_down, 0);
		} else if (strcasecmp(p, "can_up") == 0) {
			ret = cmd_updown_common(rs, args, &apn_can_move_up, 1);
		} else if (strcasecmp(p, "can_down") == 0) {
			ret = cmd_updown_common(rs, args,
			    &apn_can_move_down, 1);
		} else if (strcasecmp(p, "remove") == 0) {
			ret = cmd_remove(rs, args);
		} else if (strcasecmp(p, "alf_escalation") == 0) {
			ret = cmd_alf_escalation(rs, args);
		} else if (strcasecmp(p, "sb_escalation") == 0) {
			ret = cmd_sb_escalation(rs, args);
		} else if (strcasecmp(p, "sfs_escalation") == 0) {
			ret = cmd_sfs_escalation(rs, args);
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

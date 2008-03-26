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

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#ifndef LINUX
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apn.h"

/* Implemented in parse.y */
extern int	parse_rules(const char *, struct apn_ruleset *);

/* Only for internal use */
static int	apn_print_app(struct apn_app *, int);
static int	apn_print_alfrule(struct apn_alfrule *, int);
static void	apn_print_hash(char *, int, int);
static int	apn_print_afiltrule(struct apn_afiltrule *);
static int	apn_print_host(struct apn_host *);
static int	apn_print_address(struct apn_addr *);
static int	apn_print_port(struct apn_port *);
static int	apn_print_acaprule(struct apn_acaprule *);
static int	apn_print_defaultrule(struct apn_default *);
static int	apn_print_action(int, int);
static int	apn_print_netaccess(int);
static int	apn_print_log(int);
static int	apn_print_af(int);
static int	apn_print_proto(int);

/*
 * Parse the specified file and return the ruleset, which is allocated
 * and which is to be freed be the caller.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: file was parsed succesfully
 *  1: file could not be parsed
 */
int
apn_parse(const char *filename, struct apn_ruleset **rsp, int flags)
{
	struct apn_ruleset	*rs;

	if ((rs = calloc(sizeof(struct apn_ruleset), 1)) == NULL)
		return (-1);
	TAILQ_INIT(&rs->alf_queue);
	TAILQ_INIT(&rs->sfs_queue);
	TAILQ_INIT(&rs->var_queue);
	rs->flags = flags;
	*rsp = rs;

	return (parse_rules(filename, rs));
}

/*
 * Add a rule or a list of rules to the ALF ruleset.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was added.
 *
 * In case of an error, no rules are added, thus caller can free them safely.
 */
int
apn_add_alfrule(struct apn_rule *rule, struct apn_ruleset *ruleset)
{
	int ret = 0;

	TAILQ_INSERT_TAIL(&ruleset->alf_queue, rule, entry);

	if (ruleset->flags & APN_FLAG_VERBOSE)
		ret = apn_print_rule(rule, ruleset->flags);

	return (ret);
}

/*
 * Print a rule.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule could be printed
 *  1: an error occured while parsing the rule
 */
int
apn_print_rule(struct apn_rule *rule, int flags)
{
	int	ret = 0;

	if ((ret = apn_print_app(rule->app, flags)) != 0)
		return (ret);

	printf(" {\n");

	switch (rule->type) {
	case APN_ALF:
		ret = apn_print_alfrule(rule->rule.alf, flags);
		break;
	default:
		ret = 1;
		break;
	}

	printf("}\n");

	return (ret);
}

static int
apn_print_app(struct apn_app *app, int flags)
{
	struct apn_app *hp = app;

	if (hp == NULL)
		return (1);

	if (app->next)
		printf("{");

	while (hp) {
		if (hp->name == NULL)
			return (1);
		printf("%s ", hp->name);

		if (hp->hashtype != APN_HASH_SHA256)
			return (1);
		printf("sha256 \\\n\t");

		apn_print_hash(hp->hashvalue, flags, 256 / 8);

		hp = hp->next;
		if (hp)
			printf(",\n");
	}

	if (app->next)
		printf("}");

	return (0);
}

static int
apn_print_alfrule(struct apn_alfrule *rule, int flags)
{
	struct apn_alfrule	*hp = rule;
	int			 ret = 0;

	if (hp == NULL)
		return (1);

	while (hp) {
		switch (hp->type) {
		case APN_ALF_FILTER:
			ret = apn_print_afiltrule(&hp->rule.afilt);
			break;
		case APN_ALF_CAPABILITY:
			ret = apn_print_acaprule(&hp->rule.acap);
			break;
		case APN_ALF_DEFAULT:
			ret = apn_print_defaultrule(&hp->rule.apndefault);
			break;
		default:
			return (1);
		}

		hp = hp->next;
	}

	return (ret);
}

static int
apn_print_afiltrule(struct apn_afiltrule *rule)
{
	if (rule == NULL)
		return (1);

	printf("\t");

	if (apn_print_action(rule->action, 1) == 1)
		return (1);
	if (apn_print_netaccess(rule->filtspec.netaccess) == 1)
		return (1);
	if (apn_print_log(rule->filtspec.log) == 1)
		return (1);
	if (apn_print_af(rule->filtspec.af) == 1)
		return (1);
	if (apn_print_proto(rule->filtspec.proto) == 1)
		return (1);

	if (rule->filtspec.fromhost == NULL && rule->filtspec.tohost == NULL)
		printf("all");
	else {
		printf("from ");
		if (apn_print_host(rule->filtspec.fromhost) == 1)
			return (1);
		if (apn_print_port(rule->filtspec.fromport) == 1)
			return (1);
		printf(" to ");
		if (apn_print_host(rule->filtspec.tohost) == 1)
			return (1);
		if (apn_print_port(rule->filtspec.toport) == 1)
			return (1);
	}

	printf("\n");

	return (0);
}

static int
apn_print_acaprule(struct apn_acaprule *rule)
{
	if (rule == NULL)
		return (1);

	printf("\t");

	if (apn_print_action(rule->action, 1) == 1)
		return (1);

	switch (rule->capability) {
	case APN_ALF_CAPRAW:
		printf("raw");
		break;
	case APN_ALF_CAPOTHER:
		printf("other");
		break;
	case APN_ALF_CAPALL:
		printf("all");
		break;
	default:
		return (1);
	}

	printf("\n");

	return (0);
}

static int
apn_print_defaultrule(struct apn_default *rule)
{
	if (rule == NULL)
		return (1);

	printf("\tdefault ");

	if (apn_print_action(rule->action, 0) == 1)
		return (1);

	printf("\n");

	return (0);
}

static int
apn_print_host(struct apn_host *host)
{
	struct apn_host *hp = host;

	if (hp == NULL) {
		printf("any");
		return (0);
	}

	if (host->next)
		printf("{");

	while (hp) {
		if (hp->not)
			printf("!");
		if (apn_print_address(&hp->addr) == 1)
			return (1);
		hp = hp->next;
		if (hp)
			printf(", ");
	}

	if (host->next)
		printf("} ");

	return (0);
}

static int
apn_print_address(struct apn_addr *addr)
{
	char	buffer[256];

	if (inet_ntop(addr->af, &addr->apa.addr32, buffer, sizeof(buffer))
	    == NULL)
		return (1);

	printf("%s", buffer);

	if (addr->len != 32 && addr->len != 128)
		printf("/%u", addr->len);

	return (0);
}

static int
apn_print_port(struct apn_port *port)
{
	struct apn_port *hp = port;

	if (hp == NULL)
		return (0);

	printf(" port ");

	if (port->next)
		printf("{");

	while (hp) {
		printf("%hu", ntohs(hp->port));

		hp = hp->next;
		if (hp)
			printf(", ");
	}

	if (port->next)
		printf("} ");

	return (0);
}

static int
apn_print_action(int action, int space)
{
	switch (action) {
	case APN_ACTION_ALLOW:
		printf("allow");
		break;
	case APN_ACTION_DENY:
		printf("deny");
		break;
	case APN_ACTION_ASK:
		printf("ask");
		break;
	default:
		return (1);
	}

	if (space)
		printf(" ");

	return (0);
}

static int
apn_print_netaccess(int netaccess)
{
	switch (netaccess) {
	case APN_CONNECT:
		printf("connect ");
		break;
	case APN_ACCEPT:
		printf("accept ");
		break;
	default:
		return (1);
	}

	return (0);
}

static int
apn_print_log(int log)
{
	switch (log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		printf("log ");
		break;
	case APN_LOG_ALERT:
		printf("alert ");
		break;
	default:
		return (1);
	}

	return (0);
}

static int
apn_print_af(int af)
{
	switch (af) {
	case AF_INET:
		printf("inet ");
		break;
	case AF_INET6:
		printf("inet6 ");
		break;
	default:
		break;
	}

	return (0);
}

static int
apn_print_proto(int proto)
{
	switch (proto) {
	case IPPROTO_TCP:
		printf("tcp ");
		break;
	case IPPROTO_UDP:
		printf("udp ");
		break;
	default:
		return (1);
	}

	return (0);
}

static void
apn_print_hash(char *hash, int flags, int len)
{
	int	i;

	for (i = 0; i < len; i++)
		printf("%2.2x", (unsigned char)hash[i]);
}

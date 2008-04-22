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
#ifndef NEEDBSDCOMPAT
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
static int	apn_print_app(struct apn_app *);
static int	apn_print_alfrule(struct apn_alfrule *, int);
static int	apn_print_sfsrule(struct apn_sfsrule *, int);
static void	apn_print_hash(char *, int);
static int	apn_print_afiltrule(struct apn_afiltrule *);
static int	apn_print_host(struct apn_host *);
static int	apn_print_address(struct apn_addr *);
static int	apn_print_port(struct apn_port *);
static int	apn_print_acaprule(struct apn_acaprule *);
static int	apn_print_defaultrule(struct apn_default *);
static int	apn_print_contextrule(struct apn_context *);
static int	apn_print_scheckrule(struct apn_sfscheck *);
static int	apn_print_action(int, int);
static int	apn_print_netaccess(int);
static int	apn_print_log(int);
static int	apn_print_af(int);
static int	apn_print_proto(int);
static void	apn_free_errq(struct apnerr_queue *);
static void	apn_free_ruleq(struct apnrule_queue *);
static void	apn_free_varq(struct apnvar_queue *);
static void	apn_free_rule(struct apn_rule *);
static void	apn_free_var(struct var *);
static void	apn_free_alfrule(struct apn_alfrule *);
static void	apn_free_host(struct apn_host *);
static void	apn_free_port(struct apn_port *);
static void	apn_free_app(struct apn_app *);

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
	int			 ret;

	if ((rs = calloc(sizeof(struct apn_ruleset), 1)) == NULL)
		return (-1);
	TAILQ_INIT(&rs->alf_queue);
	TAILQ_INIT(&rs->sfs_queue);
	TAILQ_INIT(&rs->var_queue);
	TAILQ_INIT(&rs->err_queue);
	rs->flags = flags;
	*rsp = rs;

	if ((ret = parse_rules(filename, rs)) != 0) {
		apn_free_ruleq(&rs->alf_queue);
		apn_free_ruleq(&rs->sfs_queue);
		apn_free_varq(&rs->var_queue);
	}

	return (ret);
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

	if (ruleset == NULL || rule == NULL)
		return (1);

	TAILQ_INSERT_TAIL(&ruleset->alf_queue, rule, entry);

	if (ruleset->flags & APN_FLAG_VERBOSE)
		ret = apn_print_rule(rule, ruleset->flags);

	return (ret);
}

/*
 * Add a rule or a list of rules to the SFS ruleset.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was added.
 *
 * In case of an error, no rules are added, thus caller can free them safely.
 */
int
apn_add_sfsrule(struct apn_rule *rule, struct apn_ruleset *ruleset)
{
	int ret = 0;

	if (ruleset == NULL || rule == NULL)
		return (1);

	TAILQ_INSERT_TAIL(&ruleset->sfs_queue, rule, entry);

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

	if (rule == NULL)
		return (1);

	switch (rule->type) {
	case APN_ALF:
		/* ALF rules are application specific. */
		if (flags & APN_FLAG_VERBOSE2)
			printf("%d: ", rule->id);
		if ((ret = apn_print_app(rule->app)) != 0)
			return (ret);
		printf(" {\n");
		ret = apn_print_alfrule(rule->rule.alf, flags);
		printf("}\n");
		break;
	case APN_SFS:
		/* SFS rule are not application specific. */
		ret = apn_print_sfsrule(rule->rule.sfs, flags);
		break;
	default:
		ret = 1;
		break;
	}

	return (ret);
}

/*
 * Print error messages generated by libapn.
 */
void
apn_print_errors(struct apn_ruleset *rs)
{
	struct apnerr_queue	*errq;
	struct apn_errmsg	*msg;

	if (rs == NULL)
		return;

	errq = &rs->err_queue;
	if (errq == NULL || TAILQ_EMPTY(errq))
		return;

	TAILQ_FOREACH(msg, errq, entry)
		fprintf(stderr, "%s\n", msg->msg);

	return;
}

/*
 * Free a full ruleset.
 */
void
apn_free_ruleset(struct apn_ruleset *rs)
{
	struct apnerr_queue	*errq = &rs->err_queue;
	struct apnrule_queue	*alfq = &rs->alf_queue;
	struct apnrule_queue	*sfsq = &rs->sfs_queue;
	struct apnvar_queue	*varq = &rs->var_queue;

	if (rs == NULL)
		return;

	apn_free_errq(errq);
	apn_free_ruleq(alfq);
	apn_free_ruleq(sfsq);
	apn_free_varq(varq);

	free(rs);
}

static int
apn_print_app(struct apn_app *app)
{
	struct apn_app *hp = app;

	if (hp == NULL) {
		printf("any");
		return (0);
	}

	if (app->next)
		printf("{");

	while (hp) {
		if (hp->name == NULL)
			return (1);
		printf("%s ", hp->name);

		switch (hp->hashtype) {
		case APN_HASH_NONE:
			break;
		case APN_HASH_SHA256:
			printf("sha256 \\\n");
			apn_print_hash(hp->hashvalue, 256 / 8);
			break;
		default:
			return (1);
		}

		hp = hp->next;
		if (hp)
			printf(",\n");
	}

	if (app->next)
		printf(" }");

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
		if (flags & APN_FLAG_VERBOSE2)
			printf("%d: ", hp->id);

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
		case APN_ALF_CTX:
			ret = apn_print_contextrule(&hp->rule.apncontext);
			break;
		default:
			return (1);
		}

		if (ret)
			break;

		hp = hp->next;
	}

	return (ret);
}

static int
apn_print_sfsrule(struct apn_sfsrule *rule, int flags)
{
	struct apn_sfsrule	*hp = rule;
	int			 ret = 0;

	if (hp == NULL)
		return (1);

	while (hp) {
		if (flags & APN_FLAG_VERBOSE2)
			printf("%d: ", hp->id);

		switch (hp->type) {
		case APN_SFS_CHECK:
			ret = apn_print_scheckrule(&hp->rule.sfscheck);
			break;
		case APN_SFS_DEFAULT:
			ret = apn_print_defaultrule(&hp->rule.apndefault);
			break;
		default:
			return (1);
		}

		if (ret)
			break;

		hp = hp->next;
	}

	return (ret);
}
static int
apn_print_afiltrule(struct apn_afiltrule *rule)
{
	if (rule == NULL)
		return (1);

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

	if (apn_print_action(rule->action, 1) == 1)
		return (1);
	if (apn_print_log(rule->log) == 1)
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

	printf("default ");

	if (apn_print_log(rule->log) == 1)
		return (1);
	if (apn_print_action(rule->action, 0) == 1)
		return (1);

	printf("\n");

	return (0);
}

static int
apn_print_contextrule(struct apn_context *rule)
{
	if (rule == NULL)
		return (1);

	printf("context new ");

	if (apn_print_app(rule->application) == 1)
		return (1);

	printf("\n");

	return (0);
}

static int
apn_print_scheckrule(struct apn_sfscheck *rule)
{
	if (rule == NULL)
		return (1);

	if (apn_print_app(rule->app) == 1)
		return (1);
	if (rule->log != APN_LOG_NONE)
		printf(" ");
	if (apn_print_log(rule->log) == 1)
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

	if (addr == NULL)
		return (1);
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
	case APN_INOUT:
		/* nothing */
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
apn_print_hash(char *hash, int len)
{
	int	i;

	if (hash == NULL)
		return;

	for (i = 0; i < len; i++)
		printf("%2.2x", (unsigned char)hash[i]);
}

static void
apn_free_errq(struct apnerr_queue *errq)
{
	struct apn_errmsg	*msg, *next;

	if (errq == NULL || TAILQ_EMPTY(errq))
		return;
	for (msg = TAILQ_FIRST(errq); msg != TAILQ_END(varq); msg = next) {
		next = TAILQ_NEXT(msg, entry);
		TAILQ_REMOVE(errq, msg, entry);
		free(msg->msg);
		free(msg);
	}
}

static void
apn_free_ruleq(struct apnrule_queue *ruleq)
{
	struct apn_rule	*rule, *next;

	if (ruleq == NULL || TAILQ_EMPTY(ruleq))
		return;
	for (rule = TAILQ_FIRST(ruleq); rule != TAILQ_END(ruleq); rule = next) {
		next = TAILQ_NEXT(rule, entry);
		TAILQ_REMOVE(ruleq, rule, entry);
		apn_free_rule(rule);
	}
}

static void
apn_free_varq(struct apnvar_queue *varq)
{
	struct var	*var, *next;

	if (varq == NULL || TAILQ_EMPTY(varq))
		return;
	for (var = TAILQ_FIRST(varq); var != TAILQ_END(varq); var = next) {
		next = TAILQ_NEXT(var, entry);
		TAILQ_REMOVE(varq, var, entry);
		apn_free_var(var);
	}
}

static void
apn_free_rule(struct apn_rule *rule)
{
	struct apn_rule	*hp, *next;

	hp = rule;
	while (hp) {
		next = hp->next;

		switch (hp->type) {
		case APN_ALF:
			apn_free_alfrule(hp->rule.alf);
			break;
		default:
			break;
		}
		apn_free_app(hp->app);
		free(hp);
		hp = next;

	}
}

static void
apn_free_var(struct var *var)
{
	if (var == NULL)
		return;

	switch (var->type) {
	case VAR_APPLICATION:
		apn_free_app((struct apn_app *)var->value);
		break;

	case VAR_RULE:
		apn_free_rule((struct apn_rule *)var->value);
		break;

	case VAR_DEFAULT:
		/* nothing to free */
		break;

	case VAR_HOST:
		apn_free_host((struct apn_host *)var->value);
		break;

	case VAR_PORT:
		apn_free_port((struct apn_port *)var->value);
		break;

	default:
		break;
	}
	free(var->name);
	free(var);
}

static void
apn_free_alfrule(struct apn_alfrule *rule)
{
	struct apn_alfrule	*hp, *next;

	hp = rule;
	while (hp) {
		next = hp->next;

		switch (hp->type) {
		case APN_ALF_FILTER:
			/* XXX HSH not yet, vars. need to be enabled first */
			/* apn_free_host(hp->rule.afilt.filtspec.fromhost); */
			/* apn_free_host(hp->rule.afilt.filtspec.tohost); */
			/* apn_free_port(hp->rule.afilt.filtspec.fromport); */
			/* apn_free_port(hp->rule.afilt.filtspec.toport); */
			break;

		case APN_ALF_CAPABILITY:
			/* nothing to free */
			break;

		case APN_ALF_DEFAULT:
			/* nothing to free */
			break;

		case APN_ALF_CTX:
			/* XXX HSH not yet, vars. need to be enabled first */
			/* apn_free_app(hp->rule.apncontext.application); */
			break;

		default:
			break;
		}
		free(hp);
		hp = next;
	}
}

static void
apn_free_host(struct apn_host *addr)
{
	struct apn_host	*hp, *next;

	hp = addr;
	while (hp) {
		next = hp->next;
		free(hp);
		hp = next;
	}
}

static void
apn_free_port(struct apn_port *port)
{
	struct apn_port	*hp, *next;

	hp = port;
	while (hp) {
		next = hp->next;
		free(hp);
		hp = next;
	}
}

static void
apn_free_app(struct apn_app *app)
{
	struct apn_app	*hp, *next;

	hp = app;
	while (hp) {
		next = hp->next;
		free(hp->name);
		free(hp);
		hp = next;
	}
}

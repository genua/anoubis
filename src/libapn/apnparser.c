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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

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

#include <sys/types.h>
#include "apn.h"

/* Implemented in parse.y */
extern int	parse_rules(const char *, struct apn_ruleset *);
extern int	parse_rules_iovec(const char *, struct iovec *, int count,
		    struct apn_ruleset *);

/* Only for internal use */
static int	apn_print_app(struct apn_app *, FILE *);
static int	apn_print_alfrule(struct apn_alfrule *, int, FILE *);
static int	apn_print_sfsrule(struct apn_sfsrule *, int, FILE *);
static void	apn_print_hash(u_int8_t *, int, FILE *);
static int	apn_print_afiltrule(struct apn_afiltrule *, FILE *);
static int	apn_print_host(struct apn_host *, FILE *);
static int	apn_print_address(struct apn_addr *, FILE *);
static int	apn_print_port(struct apn_port *, FILE *);
static int	apn_print_acaprule(struct apn_acaprule *, FILE *);
static int	apn_print_defaultrule(struct apn_default *, FILE *);
static int	apn_print_contextrule(struct apn_context *, FILE *);
static int	apn_print_scheckrule(struct apn_sfscheck *, FILE *);
static int	apn_print_action(int, int, FILE *);
static int	apn_print_netaccess(int, FILE *);
static int	apn_print_log(int, FILE *);
static int	apn_print_af(int, FILE *);
static int	apn_print_proto(int, FILE *);
static void	apn_free_errq(struct apnerr_queue *);
static void	apn_free_ruleq(struct apnrule_queue *);
static void	apn_free_varq(struct apnvar_queue *);
static void	apn_free_rule(struct apn_rule *);
static void	apn_free_var(struct var *);
static struct apn_rule	*apn_search_rule(struct apnrule_queue *, int);
static int	apn_update_ids(struct apn_rule *, struct apn_ruleset *);
static struct apn_alfrule *apn_searchinsert_alfrule(struct apnrule_queue *,
		 struct apn_alfrule *, int);
static struct apn_sfsrule *apn_searchinsert_sfsrule(struct apnrule_queue *,
    struct apn_sfsrule *, int);
static struct apn_rule	*apn_search_rulealf(struct apnrule_queue *, int);
static struct apn_rule	*apn_copy_rule(struct apn_rule *);
static int	 apn_copy_afilt(struct apn_afiltrule *, struct apn_afiltrule *);
static int	 apn_copy_acap(struct apn_acaprule *, struct apn_acaprule *);
static int	 apn_copy_apndefault(struct apn_default *,
		     struct apn_default *);
static struct apn_host *apn_copy_hosts(struct apn_host *);
static struct apn_port *apn_copy_ports(struct apn_port *);
static int	 apn_set_application(struct apn_rule *, const char *,
		     const u_int8_t *, int);
static int	apn_remove_alf(struct apn_rule *, int);
static int	apn_remove_sfs(struct apn_rule *, int);
static int	apn_remove_queue(struct apnrule_queue *, int);

/*
 * Parse the specified file or iovec and return the ruleset, which is allocated
 * and which is to be freed be the caller.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: file was parsed succesfully
 *  1: file could not be parsed or parameters are invalid
 */
static int
__apn_parse_common(const char *filename, struct apn_ruleset **rsp, int flags)
{
	struct apn_ruleset	*rs;

	if (filename == NULL || rsp == NULL)
		return (1);

	if ((rs = calloc(sizeof(struct apn_ruleset), 1)) == NULL)
		return (-1);
	TAILQ_INIT(&rs->alf_queue);
	TAILQ_INIT(&rs->sfs_queue);
	TAILQ_INIT(&rs->var_queue);
	TAILQ_INIT(&rs->err_queue);
	rs->flags = flags;
	rs->maxid = 0;
	*rsp = rs;

	return 0;
}

int
apn_parse(const char *filename, struct apn_ruleset **rsp, int flags)
{
	int			 ret;
	struct apn_ruleset	*rs;

	ret = __apn_parse_common(filename, rsp, flags);
	if (ret)
		return ret;
	rs = *(rsp);
	if ((ret = parse_rules(filename, rs)) != 0) {
		apn_free_ruleq(&rs->alf_queue);
		apn_free_ruleq(&rs->sfs_queue);
		apn_free_varq(&rs->var_queue);
	}

	return (ret);
}

int
apn_parse_iovec(const char *filename, struct iovec *vec, int count,
    struct apn_ruleset **rsp, int flags)
{
	int			 ret;
	struct apn_ruleset	*rs;

	ret = __apn_parse_common(filename, rsp, flags);
	if (ret)
		return ret;
	rs = *(rsp);
	if ((ret = parse_rules_iovec(filename, vec, count, rs)) != 0) {
		apn_free_ruleq(&rs->alf_queue);
		apn_free_ruleq(&rs->sfs_queue);
		apn_free_varq(&rs->var_queue);
	}
	return ret;
}

static int
apn_hash_equal(int type, const u_int8_t *h1, const u_int8_t *h2)
{
	int len;
	switch(type) {
	case APN_HASH_NONE:
		return 1;
	case APN_HASH_SHA256:
		len = APN_HASH_SHA256_LEN;
		break;
	default:
		return 0;
	}
	return (memcmp(h1, h2, len) == 0);
}

/*
 * Add a rule or a list of rules to the ALF ruleset.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was added.
 *  1: invalid parameters
 *
 * In case of an error, no rules are added, thus caller can free them safely.
 */
int
apn_add_alfrule(struct apn_rule *rule, struct apn_ruleset *ruleset,
    const char * filename, int lineno)
{
	int ret = 0;
	struct apn_rule *tmp;

	if (ruleset == NULL || rule == NULL)
		return (1);

	/*
	 * Issue an error if the ruleset appears after an any rule or
	 * another application rule for the same application.
	 */
	TAILQ_FOREACH(tmp, &ruleset->alf_queue, entry) {
		if (tmp->app == NULL)
			goto duplicate;
		if (rule->app == NULL)
			continue;
		if (strcmp(rule->app->name, tmp->app->name) != 0)
			continue;
		if (rule->app->hashtype != tmp->app->hashtype)
			continue;
		if (!apn_hash_equal(rule->app->hashtype, rule->app->hashvalue,
		    tmp->app->hashvalue))
			continue;
		goto duplicate;
	}

	TAILQ_INSERT_TAIL(&ruleset->alf_queue, rule, entry);

	if (ruleset->flags & APN_FLAG_VERBOSE)
		ret = apn_print_rule(rule, ruleset->flags, stdout);

	return (ret);
duplicate:
	if (filename)
		apn_error(ruleset, filename, lineno, "Rule will never match!");
	return (1);
}

/*
 * Add a rule or a list of rules to the SFS ruleset.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was added.
 *  1: invalid parameters
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
		ret = apn_print_rule(rule, ruleset->flags, stdout);

	return (ret);
}

/*
 * Insert rule to rule set before rule with the identification ID.
 * The IDs of the passed struct apn_rule will be updated!
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was inserted.
 *  1: invalid parameters
 *
 * In case of an error, no rules are added, thus caller can free them safely.
 */
int
apn_insert(struct apn_ruleset *rs, struct apn_rule *rule, int id)
{
	struct apnrule_queue	*queue;
	struct apn_rule		*p;

	if (rs == NULL || rule == NULL || id < 1 || rs->maxid == INT_MAX)
		return (1);

	switch (rule->type) {
	case APN_ALF:
		queue = &rs->alf_queue;
		break;
	case APN_SFS:
		queue = &rs->sfs_queue;
		break;
	default:
		return (1);
	}

	if ((p = apn_search_rule(queue, id)) == NULL)
		return (1);

	if (apn_update_ids(rule, rs))
		return (1);

	TAILQ_INSERT_BEFORE(p, rule, entry);

	return (0);
}

/*
 * Insert alf rule before alf rule with identification ID.  The IDs of
 * the passed struct apn_alfrule is updated.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was inserted
 *  1: invalid parameters
 */
int
apn_insert_alfrule(struct apn_ruleset *rs, struct apn_alfrule *arule, int id)
{
	struct apnrule_queue	*queue;
	struct apn_alfrule	*p;

	if (rs == NULL || arule == NULL || id < 1 || rs->maxid == INT_MAX)
		return (1);

	queue = &rs->alf_queue;
	if ((p = apn_searchinsert_alfrule(queue, arule, id)) == NULL)
		return (1);

	arule->id = rs->maxid;
	rs->maxid += 1;

	return (0);
}

/*
 * Insert sfs rule before sfs rule with identification ID.  The IDs of
 * the passed struct apn_sfsrule is updated.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was inserted
 *  1: invalid parameters
 */
int
apn_insert_sfsrule(struct apn_ruleset *rs, struct apn_sfsrule *srule, int id)
{
	struct apnrule_queue	*queue;
	struct apn_sfsrule	*p;

	if (rs == NULL || srule == NULL || id < 1 || rs->maxid == INT_MAX)
		return (1);

	queue = &rs->sfs_queue;
	if ((p = apn_searchinsert_sfsrule(queue, srule, id)) == NULL)
		return (1);

	srule->id = rs->maxid;
	rs->maxid += 1;

	return (0);
}

/*
 * Add struct apn_alfrule arule to struct apn_rule identified by id. The
 * IDs of the passed struct apn_alfrule is updated.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was added
 *  1: invalid parameters
 */
int
apn_add2app_alfrule(struct apn_ruleset *rs, struct apn_alfrule *arule, int id)
{
	struct apn_rule		*app;

	if (rs == NULL || arule == NULL || id < 1 || rs->maxid == INT_MAX)
		return (1);

	if ((app = apn_search_rule(&rs->alf_queue, id)) == NULL) {
		return (1);
	}

	if (app->rule.alf == NULL) {
		app->rule.alf = arule;
		arule->id = rs->maxid;
		rs->maxid += 1;
	} else {
		return (apn_insert_alfrule(rs, arule, app->rule.alf->id));
	}

	return (0);
}

/*
 * Copy a full application rule and insert the provided ALF rule before
 * the original rule with ID id.
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was inserted
 *  1: invalid parameters
 */
int
apn_copyinsert(struct apn_ruleset *rs, struct apn_alfrule *arule, int id,
    const char *filename, const u_int8_t *csum, int type)
{
	struct apnrule_queue	*queue;
	struct apn_alfrule	*hp, *previous;
	struct apn_rule		*rule, *newrule;

	if (rs == NULL || arule == NULL || id < 1 || rs->maxid == INT_MAX)
		return (1);

	/* find app_rule that includes rule with ID id */
	queue = &rs->alf_queue;
	if ((rule = apn_search_rulealf(queue, id)) == NULL)
		return (1);

	/* copy that app_rule without context rules and applications */
	if ((newrule = apn_copy_rule(rule)) == NULL)
		return (1);

	/* set applications */
	if (apn_set_application(newrule, filename, csum, type) != 0) {
		apn_free_rule(newrule);
		return (1);
	}

	/* insert arule before alf rule with ID id */
	hp = newrule->rule.alf;
	previous = NULL;
	while (hp) {
		if (hp->id == id) {
			arule->tail = hp->tail;
			arule->next = hp;

			if (previous)
				previous->next = arule;
			else
				newrule->rule.alf = arule;
		}
		previous = hp;
		hp = hp->next;
	}

	/*
	 * Insert new rule before appliation rule including rule with
	 * ID id.  apn_insert() will generate new IDs for all rules
	 * in newrule.
	 */
	if (apn_insert(rs, newrule, rule->id) != 0) {
		apn_free_rule(newrule);
		return (1);
	}

	return (0);
}

/*
 * Print a rule.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule could be printed
 *  1: an error occured while printing the rule
 */
int
apn_print_rule(struct apn_rule *rule, int flags, FILE *file)
{
	int	ret = 0;

	if (rule == NULL || file == NULL)
		return (1);

	switch (rule->type) {
	case APN_ALF:
		/* ALF rules are application specific. */
		if (flags & APN_FLAG_VERBOSE2)
			fprintf(file, "%d: ", rule->id);
		if ((ret = apn_print_app(rule->app, file)) != 0)
			return (ret);
		fprintf(file, " {\n");
		ret = apn_print_alfrule(rule->rule.alf, flags, file);
		fprintf(file, "}\n");
		break;
	case APN_SFS:
		/* SFS rule are not application specific. */
		ret = apn_print_sfsrule(rule->rule.sfs, flags, file);
		break;
	default:
		ret = 1;
		break;
	}

	return (ret);
}

/*
 * Print a full rule set.
 *
 * Return code:
 *
 * -1: a systemcall failed and errno is set
 *  0: rule could be printed
 *  1: an error occured printing the rule set
 */
int
apn_print_ruleset(struct apn_ruleset *rs, int flags, FILE *file)
{
	struct apn_rule		*rule;
	struct apnrule_queue	*queue;
	int			 ret = 0;

	if (rs == NULL || file == NULL)
		return (1);

	fprintf(file, "alf {\n");
	queue = &rs->alf_queue;
	if (!TAILQ_EMPTY(queue)) {
		TAILQ_FOREACH(rule, queue, entry) {
			if ((ret = apn_print_rule(rule, flags, file)) != 0)
				return (ret);
		}
	}
	fprintf(file, "}\n");

	fprintf(file, "sfs {\n");
	queue = &rs->sfs_queue;
	if (!TAILQ_EMPTY(&rs->sfs_queue)) {
		TAILQ_FOREACH(rule, queue, entry) {
			if ((ret = apn_print_rule(rule, flags, file)) != 0)
				return (ret);
		}
	}
	fprintf(file, "}\n");

	return (0);
}

int
apn_error(struct apn_ruleset *ruleset, const char * filename, int lineno,
    const char *fmt, ...)
{
	va_list		args;
	int		ret;

	va_start(args, fmt);
	ret = apn_verror(ruleset, filename, lineno, fmt, args);
	va_end(args);
	return ret;
}

int
apn_verror(struct apn_ruleset *ruleset, const char *filename, int lineno,
    const char *fmt, va_list args)
{
	struct apn_errmsg	*msg;
	char			*s1, *s2;

	if ((msg = calloc(1, sizeof(struct apn_errmsg))) == NULL)
		return (-1);

	if (vasprintf(&s1, fmt, args) == -1) {
		free(msg);
		return (-1);
	}

	if (asprintf(&s2, "%s: %d: %s", filename, lineno, s1) == -1) {
		free(msg);
		free(s1);
		return (-1);
	}
	free(s1);

	msg->msg = s2;
	TAILQ_INSERT_TAIL(&ruleset->err_queue, msg, entry);

	return (0);
}

/*
 * Print error messages generated by libapn.
 */
void
apn_print_errors(struct apn_ruleset *rs, FILE *file)
{
	struct apnerr_queue	*errq;
	struct apn_errmsg	*msg;

	if (rs == NULL || file == NULL)
		return;

	errq = &rs->err_queue;
	if (errq == NULL || TAILQ_EMPTY(errq))
		return;

	TAILQ_FOREACH(msg, errq, entry)
		fprintf(file, "%s\n", msg->msg);

	return;
}

/*
 * Free a full ruleset.
 */
void
apn_free_ruleset(struct apn_ruleset *rs)
{
	struct apnerr_queue	*errq;
	struct apnrule_queue	*alfq;
	struct apnrule_queue	*sfsq;
	struct apnvar_queue	*varq;

	if (rs == NULL)
		return;

	errq = &rs->err_queue;
	alfq = &rs->alf_queue;
	sfsq = &rs->sfs_queue;
	varq = &rs->var_queue;

	apn_free_errq(errq);
	apn_free_ruleq(alfq);
	apn_free_ruleq(sfsq);
	apn_free_varq(varq);

	free(rs);
}

static int
apn_print_app(struct apn_app *app, FILE *file)
{
	struct apn_app *hp = app;

	if (file == NULL)
		return (1);

	if (hp == NULL) {
		fprintf(file, "any");
		return (0);
	}

	if (app && app->next)
		fprintf(file, "{");

	while (hp) {
		if (hp->name == NULL)
			return (1);
		fprintf(file, "%s ", hp->name);

		switch (hp->hashtype) {
		case APN_HASH_NONE:
			break;
		case APN_HASH_SHA256:
			fprintf(file, "sha256 \\\n");
			apn_print_hash(hp->hashvalue, 256 / 8, file);
			break;
		default:
			return (1);
		}

		hp = hp->next;
		if (hp)
			fprintf(file, ",\n");
	}

	if (app && app->next)
		fprintf(file, " }");

	return (0);
}

static int
apn_print_scope(struct apn_scope *scope, FILE * file)
{
	if (!scope)
		return 0;
	if (scope->task)
		fprintf(file, " task %lld", scope->task);
	if (scope->timeout)
		fprintf(file, " until %ld", (long int)scope->timeout);
	return 0;
}

static int
apn_print_alfrule(struct apn_alfrule *rule, int flags, FILE *file)
{
	struct apn_alfrule	*hp = rule;
	int			 ret = 0;

	if (file == NULL)
		return (1);

	while (hp) {
		if (flags & APN_FLAG_VERBOSE2)
			fprintf(file, "%d: ", hp->id);

		switch (hp->type) {
		case APN_ALF_FILTER:
			ret = apn_print_afiltrule(&hp->rule.afilt, file);
			break;
		case APN_ALF_CAPABILITY:
			ret = apn_print_acaprule(&hp->rule.acap, file);
			break;
		case APN_ALF_DEFAULT:
			ret = apn_print_defaultrule(&hp->rule.apndefault, file);
			break;
		case APN_ALF_CTX:
			ret = apn_print_contextrule(&hp->rule.apncontext, file);
			break;
		default:
			return (1);
		}

		if (ret)
			break;
		if (hp->scope)
			ret = apn_print_scope(hp->scope, file);
		if (ret)
			break;

		fprintf(file, "\n");

		hp = hp->next;
	}

	return (ret);
}

static int
apn_print_sfsrule(struct apn_sfsrule *rule, int flags, FILE *file)
{
	struct apn_sfsrule	*hp = rule;
	int			 ret = 0;

	if (file == NULL)
		return (1);

	while (hp) {
		if (flags & APN_FLAG_VERBOSE2)
			fprintf(file, "%d: ", hp->id);

		switch (hp->type) {
		case APN_SFS_CHECK:
			ret = apn_print_scheckrule(&hp->rule.sfscheck, file);
			break;
		case APN_SFS_DEFAULT:
			ret = apn_print_defaultrule(&hp->rule.apndefault, file);
			break;
		default:
			return (1);
		}

		if (ret)
			break;
		if (hp->scope)
			ret = apn_print_scope(hp->scope, file);
		if (ret)
			break;

		fprintf(file, "\n");

		hp = hp->next;
	}

	return (ret);
}
static int
apn_print_afiltrule(struct apn_afiltrule *rule, FILE *file)
{
	if (rule == NULL || file == NULL)
		return (1);

	if (apn_print_action(rule->action, 1, file) == 1)
		return (1);
	if (apn_print_netaccess(rule->filtspec.netaccess, file) == 1)
		return (1);
	if (apn_print_log(rule->filtspec.log, file) == 1)
		return (1);
	if (apn_print_af(rule->filtspec.af, file) == 1)
		return (1);
	if (apn_print_proto(rule->filtspec.proto, file) == 1)
		return (1);

	if (rule->filtspec.fromhost == NULL && rule->filtspec.tohost == NULL &&
	    rule->filtspec.fromport == NULL && rule->filtspec.toport == NULL)
		fprintf(file, "all");
	else {
		fprintf(file, "from ");
		if (apn_print_host(rule->filtspec.fromhost, file) == 1)
			return (1);
		if (apn_print_port(rule->filtspec.fromport, file) == 1)
			return (1);
		fprintf(file, " to ");
		if (apn_print_host(rule->filtspec.tohost, file) == 1)
			return (1);
		if (apn_print_port(rule->filtspec.toport, file) == 1)
			return (1);
	}

	fprintf(file, " timeout %u", rule->filtspec.statetimeout);

	return (0);
}

static int
apn_print_acaprule(struct apn_acaprule *rule, FILE *file)
{
	if (rule == NULL || file == NULL)
		return (1);

	if (apn_print_action(rule->action, 1, file) == 1)
		return (1);
	if (apn_print_log(rule->log, file) == 1)
		return (1);

	switch (rule->capability) {
	case APN_ALF_CAPRAW:
		fprintf(file, "raw");
		break;
	case APN_ALF_CAPOTHER:
		fprintf(file, "other");
		break;
	case APN_ALF_CAPALL:
		fprintf(file, "all");
		break;
	default:
		return (1);
	}

	return (0);
}

static int
apn_print_defaultrule(struct apn_default *rule, FILE *file)
{
	if (rule == NULL || file == NULL)
		return (1);

	fprintf(file, "default ");

	if (apn_print_log(rule->log, file) == 1)
		return (1);
	if (apn_print_action(rule->action, 0, file) == 1)
		return (1);

	return (0);
}

static int
apn_print_contextrule(struct apn_context *rule, FILE *file)
{
	if (rule == NULL || file == NULL)
		return (1);

	fprintf(file, "context new ");

	if (apn_print_app(rule->application, file) == 1)
		return (1);

	return (0);
}

static int
apn_print_scheckrule(struct apn_sfscheck *rule, FILE *file)
{
	if (rule == NULL || file == NULL)
		return (1);

	if (apn_print_app(rule->app, file) == 1)
		return (1);
	if (rule->log != APN_LOG_NONE)
		fprintf(file, " ");
	if (apn_print_log(rule->log, file) == 1)
		return (1);

	return (0);
}

static int
apn_print_host(struct apn_host *host, FILE *file)
{
	struct apn_host *hp = host;

	if (file == NULL)
		return (1);

	if (hp == NULL) {
		fprintf(file, "any");
		return (0);
	}

	if (host->next)
		fprintf(file, "{");

	while (hp) {
		if (hp->negate)
			fprintf(file, "!");
		if (apn_print_address(&hp->addr, file) == 1)
			return (1);
		hp = hp->next;
		if (hp)
			fprintf(file, ", ");
	}

	if (host->next)
		fprintf(file, "} ");

	return (0);
}

static int
apn_print_address(struct apn_addr *addr, FILE *file)
{
	char	buffer[256];

	if (addr == NULL || file == NULL)
		return (1);
	if (inet_ntop(addr->af, &addr->apa.addr32, buffer, sizeof(buffer))
	    == NULL)
		return (1);

	fprintf(file, "%s", buffer);

	if (addr->len != 32 && addr->len != 128)
		fprintf(file, "/%u", addr->len);

	return (0);
}

static int
apn_print_port(struct apn_port *port, FILE *file)
{
	struct apn_port *hp = port;

	if (file == NULL)
		return (1);

	if (hp == NULL)
		return (0);

	fprintf(file, " port ");

	if (port->next)
		fprintf(file, "{");

	while (hp) {
		fprintf(file, "%hu", ntohs(hp->port));
		if (hp->port2)
			fprintf(file, " - %hu", ntohs(hp->port2));

		hp = hp->next;
		if (hp)
			fprintf(file, ", ");
	}

	if (port->next)
		fprintf(file, "} ");

	return (0);
}

static int
apn_print_action(int action, int space, FILE *file)
{
	if (file == NULL)
		return (1);

	switch (action) {
	case APN_ACTION_ALLOW:
		fprintf(file, "allow");
		break;
	case APN_ACTION_DENY:
		fprintf(file, "deny");
		break;
	case APN_ACTION_ASK:
		fprintf(file, "ask");
		break;
	default:
		return (1);
	}

	if (space)
		fprintf(file, " ");

	return (0);
}

static int
apn_print_netaccess(int netaccess, FILE *file)
{
	if (file == NULL)
		return (1);

	switch (netaccess) {
	case APN_CONNECT:
		fprintf(file, "connect ");
		break;
	case APN_ACCEPT:
		fprintf(file, "accept ");
		break;
	case APN_SEND:
		fprintf(file, "send ");
		break;
	case APN_RECEIVE:
		fprintf(file, "receive ");
		break;
	case APN_BOTH:
		fprintf(file, "both ");
		break;
	default:
		return (1);
	}

	return (0);
}

static int
apn_print_log(int log, FILE *file)
{
	if (file == NULL)
		return (1);

	switch (log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		fprintf(file, "log ");
		break;
	case APN_LOG_ALERT:
		fprintf(file, "alert ");
		break;
	default:
		return (1);
	}

	return (0);
}

static int
apn_print_af(int af, FILE *file)
{
	if (file == NULL)
		return (1);

	switch (af) {
	case AF_INET:
		fprintf(file, "inet ");
		break;
	case AF_INET6:
		fprintf(file, "inet6 ");
		break;
	default:
		break;
	}

	return (0);
}

static int
apn_print_proto(int proto, FILE *file)
{
	if (file == NULL)
		return (1);

	switch (proto) {
	case IPPROTO_TCP:
		fprintf(file, "tcp ");
		break;
	case IPPROTO_UDP:
		fprintf(file, "udp ");
		break;
	default:
		return (1);
	}

	return (0);
}

static void
apn_print_hash(u_int8_t *hash, int len, FILE *file)
{
	int	i;

	if (hash == NULL || file == NULL)
		return;

	fprintf(file, "\"");

	for (i = 0; i < len; i++)
		fprintf(file, "%2.2x", (unsigned char)hash[i]);

	fprintf(file, "\"");
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
	if (rule == NULL)
		return;

	switch (rule->type) {
	case APN_ALF:
		apn_free_alfrule(rule->rule.alf);
		break;
	case APN_SFS:
		apn_free_sfsrule(rule->rule.sfs);
		break;
	default:
		break;
	}
	apn_free_app(rule->app);
	free(rule);
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

void
apn_free_filter(struct apn_afiltspec *filtspec)
{
	if (filtspec) {
		apn_free_host(filtspec->fromhost);
		apn_free_host(filtspec->tohost);
		apn_free_port(filtspec->fromport);
		apn_free_port(filtspec->toport);
	}
}

void
apn_free_alfrule(struct apn_alfrule *rule)
{
	struct apn_alfrule	*hp, *next;

	hp = rule;
	while (hp) {
		next = hp->next;

		switch (hp->type) {
		case APN_ALF_FILTER:
			apn_free_filter(&hp->rule.afilt.filtspec);
			break;

		case APN_ALF_CAPABILITY:
			/* nothing to free */
			break;

		case APN_ALF_DEFAULT:
			/* nothing to free */
			break;

		case APN_ALF_CTX:
			apn_free_app(hp->rule.apncontext.application);
			break;

		default:
			break;
		}
		if (hp->scope)
			free(hp->scope);
		free(hp);
		hp = next;
	}
}

void
apn_free_sfsrule(struct apn_sfsrule *rule)
{
	struct apn_sfsrule	*hp, *next;

	hp = rule;
	while (hp) {
		next = hp->next;

		if (hp->scope)
			free(hp->scope);
		switch (hp->type) {
		case APN_SFS_CHECK:
			apn_free_app(hp->rule.sfscheck.app);
			break;
		case APN_SFS_DEFAULT:
			/* Nothing else */
			break;
		}
		free(hp);
		hp = next;
	}
}

void
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

void
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

void
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

static struct apn_rule *
apn_search_rule(struct apnrule_queue *queue, int id)
{
	struct apn_rule	*p;

	TAILQ_FOREACH(p, queue, entry) {
		if (p->id == id)
			return (p);
	}

	return (NULL);
}

/*
 * Update IDs.  In case of error, return 1.
 */
static int
apn_update_ids(struct apn_rule *rule, struct apn_ruleset *rs)
{
	struct apn_alfrule	*alf;
	struct apn_sfsrule	*sfs;
	int			 counter;

	if (rule == NULL || rs == NULL)
		return (1);

	counter = rs->maxid;

	switch (rule->type) {
	case APN_ALF:
		alf = rule->rule.alf;
		while (alf) {
			alf->id = counter++;
			alf = alf->next;
		}
		break;

	case APN_SFS:
		sfs = rule->rule.sfs;
		while (sfs) {
			sfs->id = counter++;
			sfs = sfs->next;
		}
		break;

	default:
		return (1);
	}

	rule->id = counter++;
	rs->maxid = counter;

	return (0);
}

/*
 * Search for alf rule with ID id and add arule before that one, if
 * arule != NULL.  Return the rule with ID rule.
 */
static struct apn_alfrule *
apn_searchinsert_alfrule(struct apnrule_queue *queue, struct apn_alfrule
    *arule, int id)
{
	struct apn_rule		*p;
	struct apn_alfrule	*hp, *previous;

	TAILQ_FOREACH(p, queue, entry) {
		hp = p->rule.alf;
		previous = NULL;
		while (hp) {
			if (hp->id == id) {
				if (arule) {
					arule->tail = hp->tail;
					arule->next = hp;

					if (previous)
						previous->next = arule;
					else
						p->rule.alf = arule;
				}
				return (hp);
			}
			previous = hp;
			hp = hp->next;
		}
	}

	return (NULL);
}

/*
 * Search for sfs rule with ID id and add arule before that one, if
 * arule != NULL.  Return the rule with ID rule.
 */
static struct apn_sfsrule *
apn_searchinsert_sfsrule(struct apnrule_queue *queue, struct apn_sfsrule
    *srule, int id)
{
	struct apn_rule		*p;
	struct apn_sfsrule	*hp, *previous;

	TAILQ_FOREACH(p, queue, entry) {
		hp = p->rule.sfs;
		previous = NULL;
		while (hp) {
			if (hp->id == id) {
				if (srule) {
					srule->tail = hp->tail;
					srule->next = hp;

					if (previous)
						previous->next = srule;
					else
						p->rule.sfs = srule;
				}
				return (hp);
			}
			previous = hp;
			hp = hp->next;
		}
	}

	return (NULL);
}

/*
 * Search for alf rule with ID id.  Return the apn_rule including that
 * alf rule.
 */
static struct apn_rule *
apn_search_rulealf(struct apnrule_queue *queue, int id)
{
	struct apn_rule		*p;
	struct apn_alfrule	*hp;

	TAILQ_FOREACH(p, queue, entry) {
		hp = p->rule.alf;
		while (hp) {
			if (hp->id == id)
				return (p);
			hp = hp->next;
		}
	}

	return (NULL);
}

/*
 * Copy rules, including their IDs.
 */
static struct apn_rule *
apn_copy_rule(struct apn_rule *rule)
{
	struct apn_rule	*newrule;

	if (rule == NULL)
		return (NULL);

	if ((newrule = calloc(1, sizeof(struct apn_rule))) == NULL)
		return (NULL);

	newrule->type = rule->type;
	newrule->id = rule->id;

	switch (rule->type) {
	case APN_ALF:
		newrule->rule.alf = apn_copy_alfrules(rule->rule.alf);
		break;
	default:
		free(newrule);
		return (NULL);
	}

	return (newrule);
}

/*
 * Copy a chain of struct apn_alfrule, however skip context rules!
 * Include IDs.
 */
struct apn_alfrule *
apn_copy_alfrules(struct apn_alfrule *rule)
{
	struct apn_alfrule	*newrule = NULL, *newhead, *hp;

	hp = rule;
	newhead = NULL;
	while (hp) {
		if ((newrule = calloc(1, sizeof(struct apn_alfrule))) == NULL)
			goto errout;

		newrule->type = hp->type;
		newrule->id = hp->id;
		if (rule->scope) {
			newrule->scope = calloc(1, sizeof(struct apn_scope));
			if (newrule->scope == NULL)
				goto errout;
			*(newrule->scope) = *(rule->scope);
		} else {
			newrule->scope = NULL;
		}

		switch (hp->type) {
		case APN_ALF_FILTER:
			if (apn_copy_afilt(&hp->rule.afilt,
			    &newrule->rule.afilt) != 0)
				goto errout;
			break;

		case APN_ALF_CAPABILITY:
			if (apn_copy_acap(&hp->rule.acap, &newrule->rule.acap)
			    != 0)
				goto errout;
			break;

		case APN_ALF_DEFAULT:
			if (apn_copy_apndefault(&hp->rule.apndefault,
			    &newrule->rule.apndefault) != 0)
				goto errout;
			break;

		case APN_ALF_CTX:
			/*FALLTHROUGH*/
		default:
			/* just ignore and go on */
			free(newrule->scope);
			free(newrule);
			hp = hp->next;
			continue;
		}

		newrule->tail = newrule;
		if (newhead == NULL)
			newhead = newrule;
		else {
			newhead->tail->next = newrule;
			newhead->tail = newrule;
		}

		hp = hp->next;
	}

	return (newhead);

errout:
	if (newrule) {
		if (newrule->scope)
			free(newrule->scope);
		free(newrule);
	}
	apn_free_alfrule(newhead);
	return (NULL);
}

static int
apn_copy_afilt(struct apn_afiltrule *src, struct apn_afiltrule *dst)
{
	if (src == NULL || dst == NULL)
		return (0);

	bcopy(src, dst, sizeof(*dst));

	if (src->filtspec.fromhost && (dst->filtspec.fromhost =
	    apn_copy_hosts(src->filtspec.fromhost)) == NULL) {
		return (1);
	}
	if (src->filtspec.tohost && (dst->filtspec.tohost =
	    apn_copy_hosts(src->filtspec.tohost)) == NULL) {
		apn_free_host(dst->filtspec.fromhost);
		return (1);
	}
	if (src->filtspec.fromport && (dst->filtspec.fromport =
	    apn_copy_ports(src->filtspec.fromport)) == NULL) {
		apn_free_host(dst->filtspec.fromhost);
		apn_free_host(dst->filtspec.tohost);
		return (1);
	}
	if (src->filtspec.toport && (dst->filtspec.toport =
	    apn_copy_ports(src->filtspec.toport)) == NULL) {
		apn_free_host(dst->filtspec.fromhost);
		apn_free_host(dst->filtspec.tohost);
		apn_free_port(dst->filtspec.fromport);
		return (1);
	}

	return (0);
}

static int
apn_copy_acap(struct apn_acaprule *src, struct apn_acaprule *dst)
{
	bcopy(src, dst, sizeof(struct apn_acaprule));
	return (0);
}

static int
apn_copy_apndefault(struct apn_default *src, struct apn_default *dst)
{
	bcopy(src, dst, sizeof(struct apn_default));
	return (0);
}

static struct apn_host *
apn_copy_hosts(struct apn_host *host)
{
	struct apn_host	*hp, *newhost, *newhead;

	newhead = NULL;
	hp = host;
	while (hp) {
		if ((newhost = calloc(1, sizeof(struct apn_host))) == NULL)
			goto errout;

		bcopy(hp, newhost, sizeof(struct apn_host));
		newhost->tail = newhost;
		if (newhead == NULL)
			newhead = newhost;
		else {
			newhead->tail->next = newhost;
			newhead->tail = newhost;
		}

		hp = hp->next;
	}

	return (newhead);

errout:
	apn_free_host(newhead);
	return (NULL);
}

static struct apn_port *
apn_copy_ports(struct apn_port *port)
{
	struct apn_port	*hp, *newport, *newhead;

	newhead = NULL;
	hp = port;
	while (hp) {
		if ((newport = calloc(1, sizeof(struct apn_port))) == NULL)
			goto errout;

		bcopy(hp, newport, sizeof(struct apn_port));
		newport->tail = newport;
		if (newhead == NULL)
			newhead = newport;
		else {
			newhead->tail->next = newport;
			newhead->tail = newport;
		}

		hp = hp->next;
	}

	return (newhead);

errout:
	apn_free_port(newhead);
	return (NULL);
}

static int
apn_set_application(struct apn_rule *rule, const char *filename,
    const u_int8_t *csum, int type)
{
	struct apn_app	*app;
	size_t		 len;

	if (rule == NULL || rule->app != NULL)
		return (1);

	/*
	 * Empty filename _and_ empty checksum is ok, ie. this means
	 * "any".
	 */
	if (filename == NULL && csum == NULL)
		return (0);

	switch (type) {
	case APN_HASH_SHA256:
		len = APN_HASH_SHA256_LEN;
		break;
	default:
		return (-1);
	}

	if ((app = calloc(1, sizeof(struct apn_app))) == NULL)
		return (-1);
	if ((app->name = strdup(filename)) == NULL) {
		free(app);
		return (-1);
	}
	bcopy(csum, app->hashvalue, len);
	app->hashtype = type;

	rule->app = app;

	return (0);
}

static int
apn_clean_alfrule(struct apn_alfrule **rule,
    int (*check)(struct apn_scope *, void*), void *data)
{
	struct apn_alfrule	*hp, *newtail = NULL;
	struct apn_alfrule	**prevp;
	int ret = 0;

	prevp = rule;
	while ((hp = *prevp)) {
		if (hp->scope && (*check)(hp->scope, data)) {
			(*prevp) = hp->next;
			hp->next = hp->tail = NULL;
			apn_free_alfrule(hp);
			ret++;
		} else {
			newtail = hp;
			prevp = &hp->next;
		}
	}
	hp = *rule;
	while(hp) {
		hp->tail = newtail;
		hp = hp->next;
	}
	return ret;
}

static int
apn_clean_sfsrule(struct apn_sfsrule **rule,
    int (*check)(struct apn_scope *, void*), void *data)
{
	struct apn_sfsrule	*hp, *newtail = NULL;
	struct apn_sfsrule	**prevp;
	int ret = 0;

	prevp = rule;
	while ((hp = *prevp)) {
		if (hp->scope && (*check)(hp->scope, data)) {
			(*prevp) = hp->next;
			hp->next = NULL;
			apn_free_sfsrule(hp);
			ret++;
		} else {
			newtail = hp;
			prevp = &hp->next;
		}
	}
	hp = *rule;
	while(hp) {
		hp->tail = newtail;
		hp = hp->next;
	}
	return ret;
}

static int
apn_clean_rule(struct apn_rule *rule,
    int (*check)(struct apn_scope *, void*), void *data)
{
	int ret = 0;
	if (rule == NULL)
		return 0;

	switch (rule->type) {
	case APN_ALF:
		ret = apn_clean_alfrule(&rule->rule.alf, check, data);
		break;
	case APN_SFS:
		ret = apn_clean_sfsrule(&rule->rule.sfs, check, data);
		break;
	default:
		break;
	}
	return ret;
}

static int
apn_clean_ruleq(struct apnrule_queue *ruleq,
    int (*check)(struct apn_scope *, void*), void *data)
{
	struct apn_rule	*rule, *next;
	int ret = 0;

	if (ruleq == NULL || TAILQ_EMPTY(ruleq))
		return 0;
	for (rule = TAILQ_FIRST(ruleq); rule != TAILQ_END(ruleq); rule = next) {
		next = TAILQ_NEXT(rule, entry);
		ret += apn_clean_rule(rule, check, data);
	}
	return ret;
}

int
apn_clean_ruleset(struct apn_ruleset *rs,
    int (*check)(struct apn_scope *, void*), void *data)
{
	int ret;

	ret = apn_clean_ruleq(&rs->alf_queue, check, data);
	ret += apn_clean_ruleq(&rs->sfs_queue, check, data);
	return ret;
}

/*
 * Removes the rule with the given id from a alf rule list.
 *
 * Return codes:
 * -1: error
 *  0: rule was removed
 *  1: invalid parameters
 *  2: no rule was found
 */
static int
apn_remove_alf(struct apn_rule *rule, int id)
{
	struct apn_alfrule	*hp, *previous;

	if (rule == NULL || id < 1) {
		return (1);
	}

	hp = rule->rule.alf;
	previous = NULL;
	while (hp) {
		if (hp->id == id) {
			if (previous) {
				previous->next = hp->next;
				previous->tail = hp->tail;
			} else {
				rule->rule.alf = hp->next;
			}
			apn_free_alfrule(hp);
			return (0);
		}
		previous = hp;
		hp = hp->next;
	}
	return (2);
}

/*
 * Removes the rule with the given id from a sfs rule list.
 *
 * Return codes:
 * -1: error
 *  0: rule was removed
 *  1: invalid parameters
 *  2: no rule was found
 */
static int
apn_remove_sfs(struct apn_rule *rule, int id)
{
	struct apn_sfsrule	*hp, *previous;

	if (rule == NULL || id < 1) {
		return (1);
	}

	hp = rule->rule.sfs;
	previous = NULL;
	while (hp) {
		if (hp->id == id) {
			if (previous) {
				previous->next = hp->next;
				previous->tail = hp->tail;
			} else {
				rule->rule.sfs = hp->next;
			}
			apn_free_sfsrule(hp);
			return (0);
		}
		previous = hp;
		hp = hp->next;
	}
	return (2);
}

/*
 * Removes the rule with the given id from a ruleset queue.
 *
 * Return codes:
 * -1: error
 *  0: rule was removed
 *  1: invalid parameters
 *  2: no rule was found
 */
static int
apn_remove_queue(struct apnrule_queue *queue, int id)
{
	struct apn_rule	*p;
	int		 rc = 2;

	if (queue == NULL || id < 1) {
		return (1);
	}

	TAILQ_FOREACH(p, queue, entry) {
		if (p->id == id) {
			TAILQ_REMOVE(queue, p, entry);
			apn_free_rule(p);
			rc = 0;
		} else {
			switch (p->type) {
			case APN_ALF:
				rc = apn_remove_alf(p, id);
				break;
			case APN_SFS:
				rc = apn_remove_sfs(p, id);
				break;
			default:
				rc = 2;
				break;
			}
		}
		if (rc == 0) {
			return (0);
		}
	}
	return (2);
}

/*
 * Removes the rule with the given id from a ruleset.
 *
 * Return codes:
 * -1: error
 *  0: rule was removed
 *  1: invalid parameters
 *  2: no rule was found
 */
int
apn_remove(struct apn_ruleset *rs, int id)
{
	int rc;

	if (rs == NULL || id < 1 || rs->maxid == INT_MAX) {
		return (1);
	}

	rc = apn_remove_queue(&rs->alf_queue, id);
	if (rc == 0) {
		return (0);
	}

	rc = apn_remove_queue(&rs->sfs_queue, id);
	if (rc == 0) {
		return (0);
	}

	return (2);
}

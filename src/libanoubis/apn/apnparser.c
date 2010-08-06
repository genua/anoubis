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
#include <sys/queue.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <sys/types.h>
#include <limits.h>
#include "apn.h"
#include "apninternals.h"

/* Only for internal use */
static int	apn_print_app(struct apn_app *, FILE *);
static int	apn_print_scope(struct apn_scope *scope, FILE * file);
static int	apn_print_afiltrule(struct apn_afiltrule *, FILE *);
static int	apn_print_host(struct apn_host *, FILE *);
static int	apn_print_address(struct apn_addr *, FILE *);
static int	apn_print_port(struct apn_port *, FILE *);
static int	apn_print_acaprule(struct apn_acaprule *, FILE *);
static int	apn_print_defaultrule(struct apn_default *, FILE *);
static int	apn_print_defaultrule2(struct apn_default *, const char *,
		    FILE *);
static int	apn_print_sbaccess(struct apn_sbaccess *, FILE *);
static int	apn_print_contextrule(struct apn_context *, FILE *);
static int	apn_print_sfsaccessrule(struct apn_sfsaccess *, FILE *);
static int	apn_print_sfsdefaultrule(struct apn_sfsdefault *, FILE *);
static int	apn_print_action(int, FILE *);
static int	apn_print_netaccess(int, FILE *);
static int	apn_print_log(int, FILE *);
static int	apn_print_proto(int, FILE *);
static void	apn_free_errq(struct apnerr_queue *);
static struct apn_rule	*apn_search_rule(struct apn_ruleset *rs,
		     struct apn_chain *, unsigned int);
static int	apn_update_ids(struct apn_rule *, struct apn_ruleset *);
static struct apn_rule	*apn_search_rule_deep(struct apn_ruleset *,
		     struct apn_chain *, unsigned int);
static int	 apn_copy_afilt(struct apn_afiltrule *, struct apn_afiltrule *);
static int	 apn_copy_acap(struct apn_acaprule *, struct apn_acaprule *);
static int	 apn_copy_apndefault(struct apn_default *,
		     struct apn_default *);
static int	 apn_copy_sbaccess(struct apn_sbaccess *,
		     struct apn_sbaccess *);
static int	 apn_copy_subject(const struct apn_subject *,
		     struct apn_subject *);
static struct apn_app	*apn_copy_app(struct apn_app *app);
static struct apn_port *apn_copy_ports(struct apn_port *);
static int	 apn_set_application(struct apn_rule *, const char *,
		     const struct apn_subject *);
static void	apn_assign_ids_chain(struct apn_ruleset *, struct apn_chain *);
static void	apn_assign_ids_one(struct apn_ruleset *, struct apn_rule *);
static void	apn_insert_id(struct apn_ruleset *, struct rb_entry *, void *);
static int	apn_verify_types(int parent, int child);
static int	apn_print_rule_cleaned(struct apn_rule *, int, FILE *,
		    int (*)(struct apn_scope *, void *), void *);


/*
 * APN-Syntax version supported by this version of libapn.
 *
 * History:
 * - 1.0  Initial version
 * - 1.1  nosfs-flag appended to context-block
 * - 1.2  sctp protocol support
 */
static int	apn_version = APN_PARSER_MKVERSION(1,3);

/*
 * Parser descriptions for different APN syntax versions. The first entry
 * is always the parser for the current version.
 */
static struct apn_parser	*apn_parsers[] = {
	&apn_parser_current,
	&apn_parser_v042,
	NULL
};

/*
 * Retrun the current apn parser version.
 */
int
apn_parser_version(void)
{
	return apn_version;
}

/*
 * Return true if an APN rule of type child is allowed inside an APN
 * rule of type parent.
 */
static int
apn_verify_types(int parent, int child)
{
	switch(parent) {
	case APN_ALF:
		return (child == APN_DEFAULT || child == APN_ALF_FILTER
		    || child == APN_ALF_CAPABILITY);
	case APN_SB:
		return (child == APN_DEFAULT || child == APN_SB_ACCESS);
	case APN_SFS:
		return (child == APN_SFS_ACCESS || child == APN_SFS_DEFAULT);
	case APN_CTX:
		return (child == APN_CTX_RULE);
	default:
		return 0;
	}
	return 0;
}

static int
init_sfs_rule(struct apn_ruleset *rs)
{
	struct apn_rule		*nrule;
	if (!TAILQ_EMPTY(&rs->sfs_queue))
		return 0;
	nrule = calloc(1, sizeof(struct apn_rule));
	if (!nrule)
		return -1;
	nrule->apn_id = 0;
	nrule->apn_type = APN_SFS;
	nrule->app = NULL;
	nrule->userdata = NULL;
	nrule->pchain = NULL;
	nrule->flags = 0;
	TAILQ_INIT(&nrule->rule.chain);
	return apn_insert(rs, nrule, 0);
}

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
apn_parse1(const char *filename, struct iovec *iov, int iovcnt,
    struct apn_ruleset **rsp, int flags, struct apn_parser *parser)
{
	int			 ret;
	struct apn_ruleset	*rs;

	if ((rs = calloc(sizeof(struct apn_ruleset), 1)) == NULL)
		return -1;
	TAILQ_INIT(&rs->alf_queue);
	TAILQ_INIT(&rs->sfs_queue);
	TAILQ_INIT(&rs->sb_queue);
	TAILQ_INIT(&rs->ctx_queue);
	TAILQ_INIT(&rs->err_queue);
	rs->flags = flags;
	rs->maxid = 1;
	rs->idtree = NULL;
	rs->destructor = NULL;
	rs->version = 0;
	*rsp = rs;
	if (iov == NULL) {
		ret = parser->parse_file(filename, rs);
	} else {
		ret = parser->parse_iovec(filename, iov, iovcnt, rs);
	}
	if (ret == 0)
		ret = init_sfs_rule(rs);
	if (ret != 0) {
		rs->idtree = NULL;
		apn_free_chain(&rs->alf_queue, rs);
		apn_free_chain(&rs->sfs_queue, rs);
		apn_free_chain(&rs->sb_queue, rs);
		apn_free_chain(&rs->ctx_queue, rs);
	}
	return ret;
}

static int
apn_parse_common(const char *filename, struct iovec *iov, int iovcnt,
    struct apn_ruleset **rsp, int flags)
{
	struct apn_parser	*parser = apn_parsers[0];
	int			 ret;
	struct apn_ruleset	*rs = NULL;

	if (filename == NULL || rsp == NULL)
		return 1;
	ret = apn_parse1(filename, iov, iovcnt, &rs, flags, parser);
	/* Try older parsers if we got a version number */
	if (ret != 0 && rs && rs->version) {
		int	i;
		for (i=1; (parser = apn_parsers[i]); ++i) {
			if (parser->minversion <= rs->version
			    && rs->version <= parser->maxversion)
				break;
		}
		if (parser) {
			apn_free_ruleset(rs);
			rs = NULL;
			ret = apn_parse1(filename, iov, iovcnt, &rs,
			    flags, parser);
		}
	}
	/* Successful parsing converts the ruleset to the current version. */
	if (ret == 0)
		rs->version = apn_version;
	(*rsp) = rs;
	return ret;
}

int
apn_parse(const char *filename, struct apn_ruleset **rsp, int flags)
{
	return apn_parse_common(filename, NULL, 0, rsp, flags);
}

int
apn_parse_iovec(const char *filename, struct iovec *vec, int count,
    struct apn_ruleset **rsp, int flags)
{
	return apn_parse_common(filename, vec, count, rsp, flags);
}

static int
apn_duplicate_ids(struct apn_rule *rule)
{
	struct rb_entry *root = NULL;
	struct apn_rule *tmp;

	if (rule->apn_id)
		rb_insert_entry(&root, &rule->_rbentry);
	switch (rule->apn_type) {
	case APN_ALF:
	case APN_SFS:
	case APN_SB:
	case APN_CTX:
		TAILQ_FOREACH(tmp, &rule->rule.chain, entry) {
			if (tmp->apn_id == 0)
				continue;
			if (rb_find(root, tmp->apn_id))
				return 1;
			rb_insert_entry(&root, &tmp->_rbentry);
		}
		break;
	default:
		break;
	}
	return 0;
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
static int
__apn_add_block_common(struct apn_ruleset *ruleset, struct apn_chain *chain,
    struct apn_rule *block, const char *filename, int lineno)
{
	int ret = 0;
	struct apn_rule *subrule;

	if (ruleset == NULL || block == NULL)
		return (1);
	/*
	 * Issue an error if the ruleset has non-zero IDs that are already
	 * in use or if the ruleset contains duplicate IDs.
	 */
	if (apn_duplicate_ids(block))
		goto invalidid;
	if (block->apn_id && !apn_valid_id(ruleset, block->apn_id))
		goto invalidid;
	TAILQ_FOREACH(subrule, &block->rule.chain, entry) {
		if (subrule->apn_id && !apn_valid_id(ruleset, subrule->apn_id))
			goto invalidid;
	}
	if (block->apn_id)
		apn_insert_id(ruleset, &block->_rbentry, block);
	TAILQ_FOREACH(subrule, &block->rule.chain, entry) {
		if (subrule->apn_id)
			apn_insert_id(ruleset, &subrule->_rbentry, subrule);
	}
	if (ruleset->flags & APN_FLAG_VERBOSE)
		ret = apn_print_rule(block, ruleset->flags, stdout);
	if (ret == 0) {
		TAILQ_INSERT_TAIL(chain, block, entry);
		block->pchain = chain;
	}

	return (ret);
invalidid:
	if (filename)
		apn_error(ruleset, filename, lineno, "Duplicate rule IDs");
	return (1);
}


int
apn_add_alfblock(struct apn_ruleset *ruleset, struct apn_rule *block,
    const char * filename, int lineno)
{
	return __apn_add_block_common(ruleset, &ruleset->alf_queue,
	    block, filename, lineno);
}

int
apn_add_sfsblock(struct apn_ruleset *ruleset, struct apn_rule *block,
    const char *filename, int lineno)
{
	if (!block)
		return (1);
	if (block->app)
		goto duplicate;
	return __apn_add_block_common(ruleset, &ruleset->sfs_queue,
	    block, filename, lineno);
duplicate:
	if (filename)
		apn_error(ruleset, filename, lineno, "SFS block with app");
	return (1);
}

int
apn_add_sbblock(struct apn_ruleset *ruleset, struct apn_rule *block,
    const char *filename, int lineno)
{
	return __apn_add_block_common(ruleset, &ruleset->sb_queue,
	    block, filename, lineno);
}

int
apn_add_ctxblock(struct apn_ruleset *ruleset, struct apn_rule *block,
    const char *filename, int lineno)
{
	return __apn_add_block_common(ruleset, &ruleset->ctx_queue, block,
	    filename, lineno);
}

/*
 * Add rule to head of concerning queue of rule set.
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
apn_add(struct apn_ruleset *rs, struct apn_rule *rule)
{
	struct apn_chain	*queue;

	if (rs == NULL || rule == NULL || rs->maxid == UINT_MAX)
		return (1);

	switch (rule->apn_type) {
	case APN_ALF:
		queue = &rs->alf_queue;
		break;
	case APN_SFS:
		queue = &rs->sfs_queue;
		break;
	case APN_SB:
		queue = &rs->sb_queue;
		break;
	case APN_CTX:
		queue = &rs->ctx_queue;
		break;
	default:
		return (1);
	}

	/* Assign new IDs to the rule block before inserting it. */
	if (apn_update_ids(rule, rs))
		return (1);

	TAILQ_INSERT_HEAD(queue, rule, entry);
	rule->pchain = queue;

	return (0);
}

/*
 * Insert rule to rule set before rule with the identification ID.
 * If ID is 0 and the queue to insert is empty, the rule is inserted
 * at the start of the queue. The IDs of the passed struct apn_rule
 * will be updated!
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was inserted.
 *  1: invalid parameters
 *
 * In case of an error, no rules are added, thus caller can free them safely.
 */
int
apn_insert(struct apn_ruleset *rs, struct apn_rule *rule, unsigned int id)
{
	struct apn_chain	*queue;
	struct apn_rule		*p = NULL;

	if (rs == NULL || rule == NULL || rs->maxid == UINT_MAX)
		return (1);

	switch (rule->apn_type) {
	case APN_ALF:
		queue = &rs->alf_queue;
		break;
	case APN_SFS:
		queue = &rs->sfs_queue;
		if (!TAILQ_EMPTY(queue))
			return (1);
		break;
	case APN_SB:
		queue = &rs->sb_queue;
		break;
	case APN_CTX:
		queue = &rs->ctx_queue;
		break;
	default:
		return (1);
	}

	if (id == 0 && !TAILQ_EMPTY(queue)) {
		return (1);
	} else if (id) {
		p = apn_search_rule(rs, queue, id);
		if (p == NULL)
			return (1);
	}

	/* Assign new IDs to the rule block before inserting it. */
	if (apn_update_ids(rule, rs))
		return (1);

	if (p) {
		TAILQ_INSERT_BEFORE(p, rule, entry);
	} else {
		TAILQ_INSERT_HEAD(queue, rule, entry);
	}
	rule->pchain = queue;

	return (0);
}

/*
 * Insert a filter rule @rule. If @id refers to an application block, the
 * filter rule is inserted at the beginning of that application block.
 * Otherwise, the rule is inserted before the filter rule @id.
 * The ID of the new rule is retained. If the ID of the new rule is 0 a
 * new ID is assigned. Trying to use an ID of an existing rule will cause
 * an error.
 *
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was inserted
 *  1: invalid parameters
 */

static int
apn_insert_rule_common(struct apn_ruleset *rs,
    struct apn_chain *queue, struct apn_rule *rule, unsigned int id)
{
	struct apn_rule		*anchor, *block;

	if (!rs || rs->maxid == UINT_MAX)
		return 1;
	if (rule == NULL || id < 1 || rs->maxid == UINT_MAX)
		return 1;
	if (rule->apn_id && !apn_valid_id(rs, rule->apn_id))
		return 1;
	block = apn_search_rule_deep(rs, queue, id);
	if (block) {
		anchor = apn_find_rule(rs, id);
		if (!anchor)
			return 1;
		TAILQ_INSERT_BEFORE(anchor, rule, entry);
		rule->pchain = anchor->pchain;
	} else {
		anchor = apn_search_rule(rs, queue, id);
		if (!anchor)
			return 1;
		TAILQ_INSERT_HEAD(&anchor->rule.chain, rule, entry);
		rule->pchain = &anchor->rule.chain;
	}
	if (rule->apn_id == 0) {
		apn_assign_id(rs, &rule->_rbentry, rule);
	} else {
		apn_insert_id(rs, &rule->_rbentry, rule);
	}

	return (0);
}

/* ALF wrapper for apn_insert_rule_common */
int
apn_insert_alfrule(struct apn_ruleset *rs, struct apn_rule *arule,
    unsigned int id)
{
	if (!apn_verify_types(APN_ALF, arule->apn_type))
		return (1);
	return apn_insert_rule_common(rs, &rs->alf_queue, arule, id);
}

/*
 * SFS wrapper for apn_insert_rule_common. This wrapper is slightly
 * different from the other wrappers: If @id is 0, this refers to the
 * SFS block.
 */
int
apn_insert_sfsrule(struct apn_ruleset *rs, struct apn_rule *srule,
    unsigned int id)
{
	if (!apn_verify_types(APN_SFS, srule->apn_type))
		return (1);
	if (id == 0) {
		struct apn_rule	*sfsblock = TAILQ_FIRST(&rs->sfs_queue);
		if (!sfsblock)
			return 1;
		id = sfsblock->apn_id;
	}
	return apn_insert_rule_common(rs, &rs->sfs_queue, srule, id);
}

/* SANDBOX wrapper for apn_insert_rule_common */
int
apn_insert_sbrule(struct apn_ruleset *rs, struct apn_rule *sbrule,
    unsigned int id)
{
	if (!apn_verify_types(APN_SB, sbrule->apn_type))
		return (1);
	return apn_insert_rule_common(rs, &rs->sb_queue, sbrule, id);
}

/* CONTEXT wrapper for apn_insert_rule_common */
int
apn_insert_ctxrule(struct apn_ruleset *rs, struct apn_rule *ctxrule,
    unsigned int id)
{
	if (!apn_verify_types(APN_CTX, ctxrule->apn_type))
		return (1);
	return apn_insert_rule_common(rs, &rs->ctx_queue, ctxrule, id);
}

/*
 * Copy a full application rule and insert the provided rule @nrule before
 * the original rule with ID @id in the copy. The application of the copy
 * is set to the value corresponding to @filename and @subject.
 * Return codes:
 * -1: a systemcall failed and errno is set
 *  0: rule was inserted
 *  1: invalid parameters
 */
static int
apn_copyinsert_common(struct apn_ruleset *rs, struct apn_rule *nrule,
    unsigned int id, const char *filename, const struct apn_subject *subject,
    int chaintype)
{
	struct apn_chain	*queue;
	struct apn_rule		*hp;
	struct apn_rule		*rule, *newrule;

	if (rs == NULL || nrule == NULL || id < 1 || rs->maxid == UINT_MAX)
		return (1);
	if (!apn_verify_types(chaintype, nrule->apn_type))
		return (1);

	/* find app_rule that includes rule with ID id */
	switch (chaintype) {
	case APN_ALF:
		queue = &rs->alf_queue;
		break;
	case APN_SB:
		queue = &rs->sb_queue;
		break;
	case APN_CTX:
		queue = &rs->ctx_queue;
		break;
	case APN_SFS:
		return (1);
	default:
		return (1);
	}
	if ((rule = apn_search_rule_deep(rs, queue, id)) == NULL)
		return (1);

	/* copy that app_rule without context rules and applications */
	if ((newrule = apn_copy_one_rule(rule)) == NULL)
		return (1);
	if (newrule->app) {
		apn_free_app(newrule->app);
		newrule->app = NULL;
	}

	/* set applications */
	if (apn_set_application(newrule, filename, subject) != 0) {
		apn_free_one_rule(newrule, NULL);
		return (1);
	}

	/* insert nrule before alf rule with ID id */
	TAILQ_FOREACH(hp, &newrule->rule.chain, entry) {
		if (hp->apn_id == id) {
			TAILQ_INSERT_BEFORE(hp, nrule, entry);
			nrule->pchain = &newrule->rule.chain;
			break;
		}
	}
	if (!hp) {
		apn_free_one_rule(newrule, NULL);
		return (1);
	}

	/*
	 * Insert new rule before appliation rule including rule with
	 * ID id.  apn_insert() will generate new IDs for all rules
	 * in newrule.
	 */
	if (apn_insert(rs, newrule, rule->apn_id) != 0) {
		/* Must not free nrule! */
		TAILQ_REMOVE(&newrule->rule.chain, nrule, entry);
		apn_free_one_rule(newrule, NULL);
		return (1);
	}

	return (0);
}

/* ALF wrapper for apn_copyinsert_common */
int
apn_copyinsert_alf(struct apn_ruleset *rs, struct apn_rule *nrule,
    unsigned int id, const char *filename, const struct apn_subject *subject)
{
	return apn_copyinsert_common(rs, nrule, id, filename, subject, APN_ALF);
}

/* CTX wrapper for apn_copyinsert_common */
int
apn_copyinsert_ctx(struct apn_ruleset *rs, struct apn_rule *nrule,
    unsigned int id, const char *filename, const struct apn_subject *subject)
{
	return apn_copyinsert_common(rs, nrule, id, filename, subject, APN_CTX);
}

/* SANDBOX wrapper for apn_copyinsert_common */
int
apn_copyinsert_sb(struct apn_ruleset *rs, struct apn_rule *nrule,
    unsigned int id, const char *filename, const struct apn_subject *subject)
{
	return apn_copyinsert_common(rs, nrule, id, filename, subject, APN_SB);
}

/**
 * Print a string (usually a path name and escape everything within the
 * string to make sure that the printed string does not cause parse errors
 *
 * @param file The target file.
 * @param string The string.
 */
static void
apn_print_string(FILE *file, const char *string)
{
	char	c;
	int	i;

	fprintf(file, "\"");
	for (i=0; (c=string[i]); ++i) {
		if (c == '"' || c == '\\')
			fprintf(file, "\\");
		fprintf(file, "%c", c);
	}
	fprintf(file, "\"");
}

static int
apn_print_chain_cleaned(struct apn_chain * chain, int flags, FILE * file,
    int (*check)(struct apn_scope *, void *), void *data)
{
	struct apn_rule * r;
	TAILQ_FOREACH(r, chain, entry) {
		int ret = apn_print_rule_cleaned(r, flags, file, check, data);
		if (ret)
			return (ret);
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
static int
apn_print_rule_cleaned(struct apn_rule *rule, int flags, FILE *file,
    int (*check)(struct apn_scope *, void *), void *data)
{
	int	ret = 0;

	if (rule == NULL || file == NULL)
		return (1);

	/*
	 * If a cleaned print is requested and the scope is expired
	 * do not print the rule.
	 */
	if (rule->scope && check && check(rule->scope, data)) {
		return 0;
	}
	/*
	 * Skip rules with no sandbox permission. This is for the benefit
	 * of the GUI which cannot easily support checkboxes where at least
	 * one value ist set.
	 */
	if (rule->apn_type == APN_SB_ACCESS) {
		if ((rule->rule.sbaccess.amask & APN_SBA_ALL) == 0)
			return 0;
	}

	if (rule->apn_type != APN_SFS) {
		fprintf(file, "%lu: ", rule->apn_id);
	}
	switch (rule->apn_type) {
	case APN_ALF:
	case APN_SB:
	case APN_CTX:
		if ((ret = apn_print_app(rule->app, file)) != 0)
			return (ret);
		if (rule->flags & APN_RULE_NOSFS) {
			if (apn_version >= APN_PARSER_MKVERSION(1, 1))
				fprintf(file, " nosfs");
			else
				return (1);
		}
		if (rule->flags & APN_RULE_PGFORCE) {
			if (apn_version >= APN_PARSER_MKVERSION(1, 3))
				fprintf(file, " pgforce");
			else
				return (1);
		}
		if (rule->flags & APN_RULE_PGONLY) {
			if (apn_version >= APN_PARSER_MKVERSION(1, 3))
				fprintf(file, " pgonly");
			else
				return (1);
		}
		fprintf(file, " {\n");
		ret = apn_print_chain_cleaned(&rule->rule.chain, flags, file,
		    check, data);
		fprintf(file, "}");
		break;
	case APN_SFS:
		ret = apn_print_chain_cleaned(&rule->rule.chain, flags, file,
		    check, data);
		break;
	case APN_ALF_FILTER:
		ret = apn_print_afiltrule(&rule->rule.afilt, file);
		break;
	case APN_ALF_CAPABILITY:
		ret = apn_print_acaprule(&rule->rule.acap, file);
		break;
	case APN_DEFAULT:
		ret = apn_print_defaultrule(&rule->rule.apndefault,
		    file);
		break;
	case APN_CTX_RULE:
		ret = apn_print_contextrule(&rule->rule.apncontext, file);
		break;
	case APN_SFS_ACCESS:
		ret = apn_print_sfsaccessrule(&rule->rule.sfsaccess, file);
		break;
	case APN_SFS_DEFAULT:
		ret = apn_print_sfsdefaultrule(&rule->rule.sfsdefault, file);
		break;
	case APN_SB_ACCESS:
		ret = apn_print_sbaccess(&rule->rule.sbaccess, file);
		break;
	default:
		return (1);
	}
	if (ret)
		return (ret);
	if (rule->scope)
		ret = apn_print_scope(rule->scope, file);
	if (rule->apn_type != APN_SFS)
		fprintf(file, "\n");
	return (ret);
}

int
apn_print_rule(struct apn_rule *rule, int flags, FILE *file)
{
	return apn_print_rule_cleaned(rule, flags, file, NULL, NULL);
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
apn_print_ruleset_cleaned(struct apn_ruleset *rs, int flags, FILE *file,
    int (*check)(struct apn_scope *, void *), void *data)
{
	if (rs == NULL || file == NULL)
		return (1);

	fprintf(file, "apnversion %d.%d\n", APN_PARSER_MAJOR(apn_version),
	    APN_PARSER_MINOR(apn_version));

	fprintf(file, "alf {\n");
	if (apn_print_chain_cleaned(&rs->alf_queue, flags, file, check, data))
		return 1;
	fprintf(file, "}\n");

	fprintf(file, "sfs {\n");
	if (apn_print_chain_cleaned(&rs->sfs_queue, flags, file, check, data))
		return 1;
	fprintf(file, "}\n");

	fprintf(file, "sandbox {\n");
	if (apn_print_chain_cleaned(&rs->sb_queue, flags, file, check, data))
		return 1;
	fprintf(file, "}\n");

	fprintf(file, "context {\n");
	if (apn_print_chain_cleaned(&rs->ctx_queue, flags, file, check, data))
		return 1;
	fprintf(file, "}\n");

	return 0;
}

int
apn_print_ruleset(struct apn_ruleset *rs, int flags, FILE *file)
{
	return apn_print_ruleset_cleaned(rs, flags, file, NULL, NULL);
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
int
apn_print_errors(struct apn_ruleset *rs, FILE *file)
{
	struct apnerr_queue	*errq;
	struct apn_errmsg	*msg;

	if (rs == NULL || file == NULL)
		return -1;

	errq = &rs->err_queue;
	if (errq == NULL || TAILQ_EMPTY(errq))
		return 0;

	TAILQ_FOREACH(msg, errq, entry)
		fprintf(file, "%s\n", msg->msg);

	return 1;
}

/*
 * Return one error message generated by libapn.
 */
char *
apn_one_error(struct apn_ruleset *rs)
{
	struct apnerr_queue	*errq;
	struct apn_errmsg	*msg, *nmsg;

	if (rs == NULL)
		return NULL;

	errq = &rs->err_queue;
	if (errq == NULL || TAILQ_EMPTY(errq))
		return NULL;

	if ((msg = TAILQ_FIRST(errq)) == NULL)
		return NULL;
	if ((nmsg = TAILQ_NEXT(msg, entry)) != NULL)
		return NULL;

	return(msg->msg);
}

/*
 * Free a full ruleset.
 */
void
apn_free_ruleset(struct apn_ruleset *rs)
{
	struct apnerr_queue	*errq;
	struct apn_chain	*alfq, *sfsq, *sbq, *ctxq;

	if (rs == NULL)
		return;

	errq = &rs->err_queue;
	alfq = &rs->alf_queue;
	sfsq = &rs->sfs_queue;
	sbq = &rs->sb_queue;
	ctxq = &rs->ctx_queue;

	apn_free_errq(errq);
	rs->idtree = NULL;
	apn_free_chain(alfq, rs);
	apn_free_chain(sfsq, rs);
	apn_free_chain(sbq, rs);
	apn_free_chain(ctxq, rs);

	free(rs);
}

static int
apn_print_subject(struct apn_subject *subject, FILE * file)
{
	switch(subject->type) {
	case APN_CS_NONE:
		break;
	case APN_CS_UID_SELF:
		fprintf(file, "self");
		break;
	case APN_CS_UID:
		if (subject->value.uid == (uid_t)-1)
			return 1;
		fprintf(file, "uid %d", (int)subject->value.uid);
		break;
	case APN_CS_KEY_SELF:
		fprintf(file, "signed-self");
		break;
	case APN_CS_KEY:
		if (!subject->value.keyid)
			return 1;
		fprintf(file, "key \"%s\"", subject->value.keyid);
		break;
	default:
		return 1;
	}
	return 0;
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
		apn_print_string(file, hp->name);
		fprintf(file, " ");
		if (apn_print_subject(&hp->subject, file))
			return 1;
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
		fprintf(file, " task %" PRIu64, scope->task);
	if (scope->timeout)
		fprintf(file, " until %lu", (unsigned long int)scope->timeout);
	return 0;
}

static int
apn_print_afiltrule(struct apn_afiltrule *rule, FILE *file)
{
	if (rule == NULL || file == NULL)
		return (1);

	if (apn_print_action(rule->action, file) == 1)
		return (1);
	fprintf(file, " ");
	if (apn_print_netaccess(rule->filtspec.netaccess, file) == 1)
		return (1);
	if (apn_print_log(rule->filtspec.log, file) == 1)
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

	return (0);
}

static int
apn_print_acaprule(struct apn_acaprule *rule, FILE *file)
{
	if (rule == NULL || file == NULL)
		return (1);

	if (apn_print_action(rule->action, file) == 1)
		return (1);
	fprintf(file, " ");
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
apn_print_defaultrule2(struct apn_default *rule, const char *prefix,
    FILE *file)
{
	if (rule == NULL || file == NULL)
		return (1);

	if (prefix)
		fprintf(file, "%s ", prefix);

	if (apn_print_log(rule->log, file) == 1)
		return (1);
	if (apn_print_action(rule->action, file) == 1)
		return (1);

	return (0);
}

static int
apn_print_defaultrule(struct apn_default *rule, FILE *file)
{
	return apn_print_defaultrule2(rule, "default", file);
}

static int
apn_print_contextrule(struct apn_context *rule, FILE *file)
{
	if (rule == NULL || file == NULL)
		return (1);

	fprintf(file, "context ");

	switch (rule->type) {
	case APN_CTX_NEW:
		fprintf(file, "new ");
		break;
	case APN_CTX_OPEN:
		fprintf(file, "open ");
		break;
	case APN_CTX_BORROW:
		fprintf(file, "borrow ");
		break;
	default:
		return (1);
	}

	if (apn_print_app(rule->application, file) == 1)
		return (1);

	return (0);
}

static int
apn_print_sfsaccessrule(struct apn_sfsaccess *rule, FILE *file)
{
	if (rule == NULL || file == NULL)
		return (1);

	if (rule->path) {
		fprintf(file, "path ");
		apn_print_string(file, rule->path);
		fprintf(file, " ");
	} else {
		fprintf(file, "any ");
	}

	if (rule->subject.type == APN_CS_NONE)
		return 1;
	if (apn_print_subject(&rule->subject, file))
		return 1;

	fprintf(file, " ");
	if (apn_print_defaultrule2(&rule->valid, "valid", file) == 1)
		return 1;

	fprintf(file, " ");
	if (apn_print_defaultrule2(&rule->invalid, "invalid", file) == 1)
		return 1;

	fprintf(file, " ");
	if (apn_print_defaultrule2(&rule->unknown, "unknown", file) == 1)
		return 1;

	return (0);
}

static int
apn_print_sfsdefaultrule(struct apn_sfsdefault *rule, FILE *file)
{
	if (rule == NULL || file == NULL)
		return (1);

	fprintf(file, "default ");

	if (rule->path) {
		fprintf(file, "path ");
		apn_print_string(file, rule->path);
		fprintf(file, " ");
	} else {
		fprintf(file, "any ");
	}

	if (apn_print_log(rule->log, file) == 1)
		return 1;

	return apn_print_action(rule->action, file);
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
apn_print_action(int action, FILE *file)
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
	case APN_ACTION_CONTINUE:
		fprintf(file, "continue");
		break;
	default:
		return (1);
	}

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
	case IPPROTO_SCTP:
		fprintf(file, "sctp ");
		break;
	default:
		return (1);
	}

	return (0);
}

static int
apn_print_sbaccess(struct apn_sbaccess *sba, FILE *file)
{
	if ((sba->amask & APN_SBA_ALL) == 0)
		return 1;
	if (apn_print_action(sba->action, file))
		return 1;
	fprintf(file, " ");
	if (apn_print_log(sba->log, file))
		return 1;
	if (!sba->path && sba->cs.type == APN_CS_NONE) {
		fprintf(file, " any");
	} else {
		if (sba->path) {
			fprintf(file, " path ");
			apn_print_string(file, sba->path);
			fprintf(file, " ");
		}
		if (apn_print_subject(&sba->cs, file))
			return 1;
	}
	fprintf(file, " ");
	if (sba->amask & APN_SBA_READ)
		fprintf(file, "r");
	if (sba->amask & APN_SBA_WRITE)
		fprintf(file, "w");
	if (sba->amask & APN_SBA_EXEC)
		fprintf(file, "x");
	return 0;
}

static void
apn_free_errq(struct apnerr_queue *errq)
{
	struct apn_errmsg	*msg, *next;

	if (errq == NULL || TAILQ_EMPTY(errq))
		return;
	for (msg = TAILQ_FIRST(errq); msg != TAILQ_END(errq); msg = next) {
		next = TAILQ_NEXT(msg, entry);
		TAILQ_REMOVE(errq, msg, entry);
		free(msg->msg);
		free(msg);
	}
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

/*
 * NOTE: Does _not_ free the apn_subject structure itself because
 * NOTE: it is usually embedded in the surrounding sturcture!
 */
void
apn_free_subject(struct apn_subject *subject)
{
	switch(subject->type) {
	case APN_CS_KEY:
		if (subject->value.keyid) {
			free(subject->value.keyid);
			subject->value.keyid = NULL;
		}
		break;
	}
	subject->type = APN_CS_NONE;	/* Paranoia */
}

/*
 * NOTE: Does _not_ free the sbaccess structure itself because
 * NOTE: it is usually embedded in the surrounding structure!
 */
void
apn_free_sbaccess(struct apn_sbaccess *sba)
{
	if (!sba)
		return;
	if (sba->path) {
		free(sba->path);
		sba->path = NULL;
	}
	apn_free_subject(&sba->cs);
}

void
apn_free_sfsaccess(struct apn_sfsaccess *sa)
{
	if (!sa)
		return;

	if (sa->path) {
		free(sa->path);
		sa->path = NULL;
	}

	apn_free_subject(&sa->subject);
}

void
apn_free_sfsdefault(struct apn_sfsdefault *sd)
{
	if (!sd)
		return;

	if (sd->path) {
		free(sd->path);
		sd->path = NULL;
	}
}

void
apn_free_chain(struct apn_chain *chain, struct apn_ruleset *rs)
{
	struct apn_rule *tmp;
	while(!TAILQ_EMPTY(chain)) {
		tmp = TAILQ_FIRST(chain);
		TAILQ_REMOVE(chain, tmp, entry);
		apn_free_one_rule(tmp, rs);
	}
}

void
apn_free_one_rule(struct apn_rule *rule, struct apn_ruleset *rs)
{
	if (rs && rs->destructor && rule->userdata)
		(*rs->destructor)(rule->userdata);
	switch (rule->apn_type) {
	case APN_ALF:
	case APN_SFS:
	case APN_SB:
	case APN_CTX:
		apn_free_chain(&rule->rule.chain, rs);
		break;
	case APN_ALF_FILTER:
		apn_free_filter(&rule->rule.afilt.filtspec);
		break;
	case APN_SB_ACCESS:
		apn_free_sbaccess(&rule->rule.sbaccess);
		break;
	case APN_ALF_CAPABILITY:
		/* nothing to free */
		break;
	case APN_DEFAULT:
		/* nothing to free */
		break;
	case APN_CTX_RULE:
		apn_free_app(rule->rule.apncontext.application);
		break;
	case APN_SFS_ACCESS:
		apn_free_sfsaccess(&rule->rule.sfsaccess);
		break;
	case APN_SFS_DEFAULT:
		apn_free_sfsdefault(&rule->rule.sfsdefault);
		break;
	default:
		break;
	}
	if (rule->scope)
		free(rule->scope);
	if (rule->app)
		apn_free_app(rule->app);
	rule->scope = NULL;
	rule->app = NULL;
	/*
	 * Some callers set rs->idtree to NULL in order to avoid the
	 * overhead of the rb_remove_entry if the entire ruleset is freed.
	 */
	if (rs && rs->idtree)
		rb_remove_entry(&rs->idtree, &rule->_rbentry);
	free(rule);
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
		apn_free_subject(&hp->subject);
		free(hp);
		hp = next;
	}
}

/*
 * Search for the rule with ID @id in the qeueu @queue that is part of
 * ruleset @rs.
 * Return the rule if it is found in the ruleset and a direct member of
 * @queue. Otherwise return NULL.
 */
static struct apn_rule *
apn_search_rule(struct apn_ruleset *rs, struct apn_chain *queue,
    unsigned int id)
{
	struct apn_rule	*p = apn_find_rule(rs, id);

	if (!p || !queue || p->pchain != queue)
		return NULL;
	return  p;
}

/*
 * Update IDs.  In case of error, return 1.
 */
static int
apn_update_ids(struct apn_rule *rule, struct apn_ruleset *rs)
{
	struct apn_rule	*r;

	if (rule == NULL || rs == NULL)
		return (1);

	switch (rule->apn_type) {
	case APN_ALF:
	case APN_SFS:
	case APN_SB:
	case APN_CTX:
		TAILQ_FOREACH(r, &rule->rule.chain, entry)
			apn_assign_id(rs, &r->_rbentry, r);
		break;
	default:
		return (1);
	}

	apn_assign_id(rs, &rule->_rbentry, rule);

	return (0);
}

/*
 * Search for a filter rule with ID @id in the queue of application blocks
 * given by @queue. Return the surrounding block if the rule is found in
 * one of the block in @queue and NULL otherwise.
 *
 * NOTE: This function returns the surrounding block not the rule itself.
 */
static struct apn_rule *
apn_search_rule_deep(struct apn_ruleset *rs, struct apn_chain *queue,
    unsigned int id)
{
	struct apn_rule		*p = apn_find_rule(rs, id);
	struct apn_chain	*chain;

	if (!p || !queue)
		return NULL;
	chain = p->pchain;
	if (!chain)
		return NULL;
	/*
	 * At this point, chain is the chain that the rule with id @id is in.
	 * Now go through queue and see if there is a block that corresponds
	 * to this chain. If so return it.
	 */
	TAILQ_FOREACH(p, queue, entry) {
		if (&p->rule.chain == chain)
			return p;
	}
	return (NULL);
}

struct apn_rule *
apn_copy_one_rule(struct apn_rule *old)
{
	struct apn_rule	*newrule;

	if (old == NULL)
		return NULL;
	if ((newrule = calloc(1, sizeof(struct apn_rule))) == NULL)
		return NULL;

	newrule->apn_type = old->apn_type;
	newrule->apn_id = old->apn_id;
	newrule->app = NULL;
	newrule->scope = NULL;
	newrule->userdata = NULL;
	newrule->pchain = NULL;
	newrule->flags = 0;
	if (old->scope) {
		newrule->scope = calloc(1, sizeof(struct apn_scope));
		if (newrule->scope == NULL)
			goto errout;
		*(newrule->scope) = *(old->scope);
	}
	if (old->app) {
		newrule->app = apn_copy_app(old->app);
		if (newrule->app == NULL)
			goto errout;
	}
	switch (old->apn_type) {
	case APN_ALF:
	case APN_SFS:
	case APN_SB:
	case APN_CTX:
		TAILQ_INIT(&newrule->rule.chain);
		if (apn_copy_chain(&old->rule.chain, &newrule->rule.chain) != 0)
			goto errout;
		break;
	case APN_ALF_FILTER:
		if (apn_copy_afilt(&old->rule.afilt,
		    &newrule->rule.afilt) != 0)
			goto errout;
		break;

	case APN_ALF_CAPABILITY:
		if (apn_copy_acap(&old->rule.acap, &newrule->rule.acap) != 0)
			goto errout;
		break;

	case APN_DEFAULT:
		if (apn_copy_apndefault(&old->rule.apndefault,
		    &newrule->rule.apndefault) != 0)
			goto errout;
		break;
	case APN_CTX_RULE:
		newrule->rule.apncontext.application = NULL;
		if (old->rule.apncontext.application) {
			newrule->rule.apncontext.application =
			    apn_copy_app(old->rule.apncontext.application);
			if (newrule->rule.apncontext.application == NULL)
				goto errout;
		}
		newrule->rule.apncontext.type = old->rule.apncontext.type;
		break;
	case APN_SFS_DEFAULT:
		newrule->rule.sfsdefault = old->rule.sfsdefault;
		if (old->rule.sfsdefault.path) {
			newrule->rule.sfsdefault.path =
			    strdup(old->rule.sfsdefault.path);
			if (newrule->rule.sfsdefault.path == NULL)
				goto errout;
		}
		break;
	case APN_SFS_ACCESS:
		newrule->rule.sfsaccess.path = NULL;
		if (old->rule.sfsaccess.path) {
			newrule->rule.sfsaccess.path =
			    strdup(old->rule.sfsaccess.path);
			if (newrule->rule.sfsaccess.path == NULL)
				goto errout;
		}
		newrule->rule.sfsaccess.valid = old->rule.sfsaccess.valid;
		newrule->rule.sfsaccess.invalid = old->rule.sfsaccess.invalid;
		newrule->rule.sfsaccess.unknown = old->rule.sfsaccess.unknown;
		if (apn_copy_subject(&old->rule.sfsaccess.subject,
		    &newrule->rule.sfsaccess.subject)) {
			if (newrule->rule.sfsaccess.path)
				free(newrule->rule.sfsaccess.path);
			goto errout;
		}
		break;
	case APN_SB_ACCESS:
		if (apn_copy_sbaccess(&old->rule.sbaccess,
		    &newrule->rule.sbaccess) != 0)
			goto errout;
		break;
	default:
		/* just ignore and go on */
		goto errout;
	}

	return (newrule);

errout:
	if (newrule) {
		if (newrule->scope)
			free(newrule->scope);
		free(newrule);
	}
	return NULL;
}

/*
 * Create an independant copy of a rule chain of apn_rules. The
 * copy uses the same IDs as the original rules and is _not_ part
 * of any ruleset.
 */
int
apn_copy_chain(struct apn_chain *src, struct apn_chain *dst)
{
	struct apn_rule	*oldrule, *newrule;
	TAILQ_FOREACH(oldrule, src, entry) {
		newrule = apn_copy_one_rule(oldrule);
		if (!newrule)
			goto errout;
		TAILQ_INSERT_TAIL(dst, newrule, entry);
		newrule->pchain = dst;
	}
	return 0;
errout:
	apn_free_chain(dst, NULL);
	return -1;
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

struct apn_host *
apn_copy_hosts(struct apn_host *host)
{
	struct apn_host	*hp, *newhost, *newhead, *newtail = NULL;

	newhead = NULL;
	hp = host;
	while (hp) {
		if ((newhost = calloc(1, sizeof(struct apn_host))) == NULL)
			goto errout;

		bcopy(hp, newhost, sizeof(struct apn_host));
		if (newhead == NULL)
			newhead = newhost;
		else {
			newtail->next = newhost;
		}
		newtail = newhost;

		hp = hp->next;
	}

	return (newhead);

errout:
	apn_free_host(newhead);
	return (NULL);
}

static int
apn_copy_subject(const struct apn_subject *src, struct apn_subject *dst)
{
	dst->type = src->type;
	switch(src->type) {
	case APN_CS_NONE:
	case APN_CS_UID_SELF:
	case APN_CS_KEY_SELF:
		dst->value.keyid = NULL;
		break;
	case APN_CS_UID:
		dst->value.uid = src->value.uid;
		break;
	case APN_CS_KEY:
		dst->value.keyid = strdup(src->value.keyid);
		if (!dst->value.keyid)
			return -1;
		break;
	}
	return 0;
}

static struct apn_port *
apn_copy_ports(struct apn_port *port)
{
	struct apn_port	*hp, *newport, *newhead, *newtail = NULL;

	newhead = NULL;
	hp = port;
	while (hp) {
		if ((newport = calloc(1, sizeof(struct apn_port))) == NULL)
			goto errout;

		bcopy(hp, newport, sizeof(struct apn_port));
		if (newhead == NULL)
			newhead = newport;
		else {
			newtail->next = newport;
		}
		newtail = newport;

		hp = hp->next;
	}

	return (newhead);

errout:
	apn_free_port(newhead);
	return (NULL);
}

static struct apn_app *
apn_copy_app(struct apn_app *app)
{
	struct apn_app	*hp, *napp, *nhead, *ntail;

	nhead = ntail = NULL;
	hp = app;
	while (hp) {
		if ((napp = calloc(1, sizeof(struct apn_app))) == NULL)
			goto errout;
		*napp = *hp;
		napp->name = NULL;
		napp->next = NULL;
		if (apn_copy_subject(&app->subject, &napp->subject) < 0) {
			free(napp);
			goto errout;
		}
		if (hp->name) {
			napp->name = strdup(hp->name);
			if (napp->name == NULL) {
				apn_free_subject(&napp->subject);
				free(napp);
				goto errout;
			}
		}
		if (ntail) {
			ntail->next = napp;
		} else {
			nhead = napp;
		}
		ntail = napp;
		hp = hp->next;
	}
	return nhead;
errout:
	apn_free_app(nhead);
	return (NULL);
}

static int
apn_copy_sbaccess(struct apn_sbaccess *src, struct apn_sbaccess *dst)
{
	dst->path = NULL;
	dst->amask = src->amask;
	dst->log = src->log;
	dst->action = src->action;
	if (src->path) {
		dst->path = strdup(src->path);
		if (dst->path == NULL)
			return -1;
	}
	if (apn_copy_subject(&src->cs, &dst->cs) < 0)
		goto errout;
	return 0;
errout:
	if (dst->path) {
		free(dst->path);
		dst->path = NULL;
	}
	return -1;
}

static int
apn_set_application(struct apn_rule *rule, const char *filename,
    const struct apn_subject *subject)
{
	struct apn_app	*app;

	if (rule == NULL || rule->app != NULL)
		return 1;

	if ((app = calloc(1, sizeof(struct apn_app))) == NULL)
		return -1;
	if (apn_copy_subject(subject, &app->subject) < 0) {
		free(app);
		return -1;
	}
	if ((app->name = strdup(filename)) == NULL) {
		apn_free_subject(&app->subject);
		free(app);
		return -1;
	}
	rule->app = app;

	return (0);
}

static int
apn_clean_rule(struct apn_ruleset *rs, struct apn_rule *block,
    int (*check)(struct apn_scope *, void*), void *data)
{
	struct apn_rule	*hp, *next;
	int			 ret = 0;

	if (!block)
		return 0;
	switch(block->apn_type) {
	case APN_ALF:
	case APN_SFS:
	case APN_SB:
	case APN_CTX:
		break;
	default:
		return 0;
	}
	for (hp = TAILQ_FIRST(&block->rule.chain);
	    hp != TAILQ_END(&block->rule.chain); hp = next) {
		next = TAILQ_NEXT(hp, entry);
		if (hp->scope && (*check)(hp->scope, data)) {
			TAILQ_REMOVE(&block->rule.chain, hp, entry);
			apn_free_one_rule(hp, rs);
			ret++;
		}
	}
	return ret;
}

static int
apn_clean_ruleq(struct apn_ruleset *rs, struct apn_chain *ruleq,
    int (*check)(struct apn_scope *, void*), void *data)
{
	struct apn_rule	*rule, *next;
	int ret = 0;

	if (ruleq == NULL || TAILQ_EMPTY(ruleq))
		return 0;
	for (rule = TAILQ_FIRST(ruleq); rule != TAILQ_END(ruleq); rule = next) {
		next = TAILQ_NEXT(rule, entry);
		ret += apn_clean_rule(rs, rule, check, data);
	}
	return ret;
}

int
apn_clean_ruleset(struct apn_ruleset *rs,
    int (*check)(struct apn_scope *, void*), void *data)
{
	int ret;

	ret = apn_clean_ruleq(rs, &rs->alf_queue, check, data);
	ret += apn_clean_ruleq(rs, &rs->sfs_queue, check, data);
	ret += apn_clean_ruleq(rs, &rs->sb_queue, check, data);
	ret += apn_clean_ruleq(rs, &rs->ctx_queue, check, data);
	return ret;
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
apn_remove(struct apn_ruleset *rs, unsigned int id)
{
	struct apn_rule		*rule;
	struct apn_chain	*pchain;

	if (rs == NULL || id < 1 || rs->maxid == UINT_MAX)
		return 1;
	rule = apn_find_rule(rs, id);
	if (!rule)
		return 2;
	pchain = rule->pchain;
	if (!pchain)
		return -1;

	/* Not allowed to remove the SFS application block. */
	if (pchain == &rs->sfs_queue)
		return -1;

	TAILQ_REMOVE(pchain, rule, entry);
	apn_free_one_rule(rule, rs);

	return 0;
}

int
apn_valid_id(struct apn_ruleset *rs, unsigned int id)
{
	return (rb_find(rs->idtree, id) == NULL);
}

struct apn_rule *
apn_find_rule(struct apn_ruleset *rs, unsigned int id)
{
	struct rb_entry		*entry;
	entry = rb_find(rs->idtree, id);
	if (entry == NULL)
		return NULL;
	return (struct apn_rule*)entry->data;
}

static void
apn_assign_ids_one(struct apn_ruleset *rs, struct apn_rule *r)
{
	switch (r->apn_type) {
	case APN_ALF:
	case APN_SFS:
	case APN_SB:
	case APN_CTX:
		apn_assign_ids_chain(rs, &r->rule.chain);
		break;
	}
	if (r->apn_id == 0)
		apn_assign_id(rs, &r->_rbentry, r);
}

static void
apn_assign_ids_chain(struct apn_ruleset *rs, struct apn_chain *chain)
{
	struct apn_rule *r;
	TAILQ_FOREACH(r, chain, entry)
		apn_assign_ids_one(rs, r);
}

/*
 * NOTE: rs->maxid must be bigger than any ID already assigned inside
 * NOTE: the rule set before calling this function.
 */
void
apn_assign_ids(struct apn_ruleset *rs)
{
	apn_assign_ids_chain(rs, &rs->alf_queue);
	apn_assign_ids_chain(rs, &rs->sfs_queue);
	apn_assign_ids_chain(rs, &rs->sb_queue);
	apn_assign_ids_chain(rs, &rs->ctx_queue);
}

static void
apn_insert_id(struct apn_ruleset *rs, struct rb_entry *e, void *data)
{
	if (rs->maxid < 10)
		rs->maxid = 10;
	e->data = data;
	rb_insert_entry(&rs->idtree, e);
	/* NOTE: e->key must be the same as data->apn_id */
	if (e->key > rs->maxid)
		rs->maxid = e->key;
}

void
apn_assign_id(struct apn_ruleset *rs, struct rb_entry *e, void *data)
{
	unsigned long nid;
	while (1) {
		int i;
		/* Try to find a random free ID below maxid 20 times */
		for (i=0; i<20; ++i) {
			nid = 1 + rand() % rs->maxid;
			if (rb_find(rs->idtree, nid) == NULL)
				break;
		}
		if (i < 20)
			break;
		/* No random ID found. Increase maxid and retry */
		if (2 * rs->maxid > rs->maxid) {
			rs->maxid = 2*rs->maxid;
			continue;
		}
		/*
		 * No suitable ID found and cannot increase maxid
		 * anymore, try a linear search as a last resort.
		 * The chance that this happens with INT_MAX/2 rule
		 * IDs is still less than one in a million.
		 */
		nid = ((unsigned long )e) % rs->maxid;
		while (rb_find(rs->idtree, nid)) {
			nid = (nid + 1) % rs->maxid;
		}
		break;
	}
	/* NOTE: e->key must be the same as data->apn_id */
	e->data = data;
	e->key = nid;
	rb_insert_entry(&rs->idtree, e);
}

int
apn_can_move_up(struct apn_rule *rule)
{
	struct apn_rule		*prev = TAILQ_PREV(rule, apn_chain, entry);

	if (!rule || !prev)
		return 0;
	switch(rule->apn_type) {
	case APN_ALF:
	case APN_SFS:
	case APN_SB:
	case APN_CTX:
		if (rule->app == NULL)
			return 0;
	}
	return (prev != rule);
}

int
apn_can_move_down(struct apn_rule *rule)
{
	struct apn_rule		*next = TAILQ_NEXT(rule, entry);

	if (!rule || !next)
		return 0;
	switch(next->apn_type) {
	case APN_ALF:
	case APN_SFS:
	case APN_SB:
	case APN_CTX:
		if (next->app == NULL)
			return 0;
	}
	return (next != rule);
}

int
apn_move_up(struct apn_rule *rule)
{
	struct apn_rule		*tmp;
	struct apn_chain	*chain = rule->pchain;

	if (!chain)
		return -1;
	if (!apn_can_move_up(rule))
		return -1;
	tmp = TAILQ_PREV(rule, apn_chain, entry);
	if (!tmp || tmp == rule)
		return -1;
	TAILQ_REMOVE(chain, rule, entry);
	TAILQ_INSERT_BEFORE(tmp, rule, entry);
	return 0;
}

int
apn_move_down(struct apn_rule *rule)
{
	struct apn_rule		*tmp;
	struct apn_chain	*chain = rule->pchain;

	if (!chain)
		return -1;
	if (!apn_can_move_down(rule))
		return -1;
	tmp = TAILQ_NEXT(rule, entry);
	if (!tmp || tmp == rule)
		return -1;
	TAILQ_REMOVE(chain, rule, entry);
	TAILQ_INSERT_AFTER(chain, tmp, rule, entry);
	return 0;
}

int
apn_add_app(struct apn_rule *rule, const char *name,
    const struct apn_subject *subject)
{
	struct apn_app		*napp;

	napp = malloc(sizeof (struct apn_app));
	if (!napp)
		return -1;
	napp->name = strdup(name);
	if (!napp->name)
		goto err;
	if (apn_copy_subject(subject, &napp->subject) < 0)
		goto err;

	napp->next = rule->app;
	rule->app = napp;

	return 0;
err:
	if (napp->name)
		free(napp->name);
	free(napp);
	return -1;
}

struct apn_rule *
apn_match_appname(struct apn_chain *chain, const char *name, int ispg)
{
	struct apn_rule		*tmp;
	TAILQ_FOREACH(tmp, chain, entry) {
		struct apn_app	*app, *napp;

		if ((tmp->flags & APN_RULE_PGONLY) && !ispg)
			continue;
		if (tmp->app == NULL)
			return tmp;
		napp = tmp->app;
		while (napp) {
			app = napp;
			napp = app->next;
			if (name && app->name) {
				if (strcmp(app->name, name) == 0)
					return tmp;
			}
		}
	}
	return NULL;
}

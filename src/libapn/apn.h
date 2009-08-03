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

#ifndef _APN_H_
#define _APN_H_

#include "config.h"

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <stdio.h>	/* For FILE */
#include <stdarg.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#ifndef LINUX
#include <sys/queue.h>
#include <dev/anoubis.h>
#include <dev/anoubis_alf.h>
#include <sys/uio.h>
#else
#include <queue.h>
#include <linux/anoubis.h>
#include <linux/anoubis_alf.h>
#endif

#include "rbtree.h"

#define APN_FLAG_VERBOSE	0x0001
#define APN_FLAG_VERBOSE2	0x0002
#define APN_FLAG_NOSCOPE	0x0004
#define APN_FLAG_NOASK		0x0008
#define APN_FLAG_NOPERMCHECK	0x0010

#define APN_HASH_SHA256_LEN	32
#define MAX_APN_HASH_LEN	APN_HASH_SHA256_LEN

#define APN_DFLT_STATETIMEOUT	600

enum apn_hash_type {
	APN_HASH_NONE, APN_HASH_SHA256
};

enum apn_direction {
	APN_CONNECT, APN_ACCEPT, APN_SEND, APN_RECEIVE, APN_BOTH
};

enum apn_action_type {
	APN_ACTION_ALLOW, APN_ACTION_DENY, APN_ACTION_ASK, APN_ACTION_CONTINUE
};

enum apn_log_level {
	APN_LOG_NONE, APN_LOG_NORMAL, APN_LOG_ALERT
};

struct apn_app {
	char		*name;
	int		 hashtype;
	u_int8_t	 hashvalue[MAX_APN_HASH_LEN];

	struct apn_app	*next;
};

struct apn_addr {
	sa_family_t		af;
	u_int8_t		len;	/* The netmask */
	union {
		struct in_addr		v4;
		struct in6_addr		v6;
		u_int8_t		addr8[16];
		u_int16_t		addr16[8];
		u_int32_t		addr32[4];
	} apa;		/* 128-bit address */
};

struct apn_host {
	struct apn_addr		 addr;
	int			 negate;
	struct apn_host		*next;
};

struct apn_port {
	u_int16_t		 port;
	u_int16_t		 port2;
	struct apn_port		*next;
};

struct apn_afiltspec {
	int			 log;
	int			 af;
	int			 proto;
	int			 netaccess;
	unsigned int		 statetimeout;

	struct apn_host		*fromhost;
	struct apn_port		*fromport;
	struct apn_host		*tohost;
	struct apn_port		*toport;
};

struct apn_afiltrule {
	int			 action;
	struct apn_afiltspec	 filtspec;
};

enum {
	APN_ALF_CAPRAW, APN_ALF_CAPOTHER, APN_ALF_CAPALL
};

struct apn_acaprule {
	int			 action;
	int			 log;
	int			 capability;
};

struct apn_default {
	int			action;
	int			log;
};

enum {
	APN_CTX_NEW, APN_CTX_OPEN, APN_CTX_BORROW
};

struct apn_context {
	struct apn_app		*application;
	int			 type;
};

#define		APN_CS_NONE	0
#define		APN_CS_CSUM	1
#define		APN_CS_UID	2
#define		APN_CS_KEY	3
#define		APN_CS_UID_SELF	4
#define		APN_CS_KEY_SELF	5

struct apn_subject {
	int			 type;
	union {
		char		*keyid;
		uid_t		 uid;
		u_int8_t	*csum;
	} value;
};

#define		APN_SBA_READ	0x0001
#define		APN_SBA_WRITE	0x0002
#define		APN_SBA_EXEC	0x0004
#define		APN_SBA_ALL	0x0007

struct apn_sbaccess {
	char			*path;
	struct apn_subject	 cs;
	unsigned int		 amask;
	int			 log;
	int			 action;
};

struct apn_sfsaccess {
	char			*path;
	struct apn_subject	 subject;
	struct apn_default	 valid;
	struct apn_default	 invalid;
	struct apn_default	 unknown;
};

struct apn_sfsdefault {
	char			*path;
	int			 log;
	int			 action;
};

enum {
	APN_ALF,		/* rule.chain, scope == NULL */
	APN_SFS,		/* rule.chain, scope == NULL */
	APN_SB,			/* rule.chain, scope == NULL */
	APN_VS,			/* rule.chain, scope == NULL */
	APN_CTX,		/* rule.chain, scope = NULL */
	APN_ALF_FILTER,		/* rule.afilt, app == NULL */
	APN_ALF_CAPABILITY,	/* rule.acap, app == NULL */
	APN_DEFAULT,		/* rule.apndefault, app == NULL */
	APN_CTX_RULE,		/* rule.apncontext, app == NULL */
	APN_SFS_ACCESS,		/* rule.sfsaccess, app == NULL */
	APN_SFS_DEFAULT,	/* rule.sfsdefault, app == NULL */
	APN_SB_ACCESS,		/* rule.sbaccess, app = NULL */
};

#define apn_type	_rbentry.dtype
#define apn_id		_rbentry.key

TAILQ_HEAD(apn_chain, apn_rule);
struct apn_rule {
	struct rb_entry			 _rbentry;
	TAILQ_ENTRY(apn_rule)		 entry;
	void				*userdata;
	struct apn_scope		*scope;
	struct apn_app			*app;
	struct apn_chain		*pchain;
	union {
		struct apn_afiltrule	afilt;
		struct apn_acaprule	acap;
		struct apn_default	apndefault;
		struct apn_context	apncontext;
		struct apn_sfsaccess	sfsaccess;
		struct apn_sfsdefault	sfsdefault;
		struct apn_chain	chain;
		struct apn_sbaccess	sbaccess;
	} rule;
};

struct apn_scope {
	time_t			timeout;
	anoubis_cookie_t	task;
};

/* Error message */
struct apn_errmsg {
	TAILQ_ENTRY(apn_errmsg)	 entry;
	char			*msg;
};

TAILQ_HEAD(apnerr_queue, apn_errmsg);

/* Complete APN ruleset. */
struct apn_ruleset {
	int			 flags;
	int			 compatids;
	unsigned int		 maxid;
	struct rb_entry		*idtree;

	/* Rulesets */
	struct apn_chain	 alf_queue;
	struct apn_chain	 sfs_queue;
	struct apn_chain	 sb_queue;
	struct apn_chain	 ctx_queue;

	/* Error messages from the parser */
	struct apnerr_queue	 err_queue;
	/* User data destructor. */
	void (*destructor)(void *);
};

/*
 * Escalation handling
 */

/*
 * Flags for apn_escalation_rule_alf
 * Suggested options for TCP:
 *	0
 *	ALF_EV_ALLPEER
 *	ALF_EV_ALLPORT
 *	(ALF_EV_ALLPEER|ALF_EV_ALLPORT)
 *
 * Suggested options for UDP:
 *	(ALF_EV_ALLDIR)
 *	(ALF_EV_ALLPEER|ALF_EV_ALLDIR)
 *	(ALF_EV_ALLPORT|ALF_EV_ALLDIR)
 *	(ALF_EV_ALLPROTO|ALF_EV_ALLDIR)
 *
 * Suggested options for ICMP:
 *	0
 */

#define ALF_EV_ALLPORT		0x001UL
#define ALF_EV_ALLPEER		0x002UL
#define ALF_EV_ALLDIR		0x004UL
#define ALF_EV_ALLPROTO		0x008UL

__BEGIN_DECLS

/*
 * Functions for parsing and error reporting.
 */
int	apn_parse(const char *, struct apn_ruleset **, int);
int	apn_parse_iovec(const char *filename, struct iovec *vec, int count,
	    struct apn_ruleset **rsp, int flags);
int	apn_print_rule(struct apn_rule *, int, FILE *);
int	apn_print_ruleset(struct apn_ruleset *, int, FILE *);
int	apn_print_errors(struct apn_ruleset *, FILE *);
int	apn_print_ruleset_cleaned(struct apn_ruleset *rs, int, FILE *,
	    int (*)(struct apn_scope *, void *), void *);

/*
 * Use these function to add an application block to a ruleset.
 */
int	apn_add(struct apn_ruleset *, struct apn_rule *);
int	apn_insert(struct apn_ruleset *, struct apn_rule *, unsigned int);

/*
 * Use these functions to insert a single filter rule into a ruleset
 */
int	apn_insert_alfrule(struct apn_ruleset *, struct apn_rule *,
	    unsigned int);
int	apn_insert_sfsrule(struct apn_ruleset *, struct apn_rule *,
	    unsigned int);
int	apn_insert_sbrule(struct apn_ruleset *, struct apn_rule *,
	    unsigned int id);
int	apn_insert_ctxrule(struct apn_ruleset *, struct apn_rule *,
	    unsigned int id);

/*
 * Analyse and modifiy application lists of application blocks.
 */
int		 apn_add_app(struct apn_rule *, const char *, const u_int8_t *);
struct apn_rule	*apn_match_app(struct apn_chain *, const char *,
		     const u_int8_t *);

/*
 * Use these functions to copy an application block and simultaneously
 * add a new rule to that block.
 */
int	apn_copyinsert_alf(struct apn_ruleset *, struct apn_rule *,
	    unsigned int, const char *, const u_int8_t *, int);
int	apn_copyinsert_ctx(struct apn_ruleset *, struct apn_rule *,
	    unsigned int, const char *, const u_int8_t *, int);
int	apn_copyinsert_sb(struct apn_ruleset *, struct apn_rule *,
	    unsigned int, const char *, const u_int8_t *, int);

/*
 * Functions used to free misc. rule structures
 */
void	apn_free_ruleset(struct apn_ruleset *);
void	apn_free_one_rule(struct apn_rule *, struct apn_ruleset *);
void	apn_free_host(struct apn_host *);
void	apn_free_port(struct apn_port *);
void	apn_free_app(struct apn_app *);

/* NOTE: Only frees the contents of the subject, not the subject itself. */
void   apn_free_subject(struct apn_subject *subject);

/*
 * Searching, Copying cleaning
 */
int	apn_clean_ruleset(struct apn_ruleset *rs,
	    int (*)(struct apn_scope *, void *), void *);
struct apn_rule *apn_copy_one_rule(struct apn_rule *);
struct apn_rule *apn_find_rule(struct apn_ruleset *, unsigned int);
int	apn_copy_chain(struct apn_chain *src, struct apn_chain *dst);

/*
 * Remove a rule from a ruleset.
 */
int	apn_remove(struct apn_ruleset *, unsigned int);

/*
 * Move rules up/down within their rule chain.
 */
int	apn_can_move_up(struct apn_rule *);
int	apn_can_move_down(struct apn_rule *);
int	apn_move_up(struct apn_rule *);
int	apn_move_down(struct apn_rule *);

/*
 * Handling of escalations
 */
int	apn_escalation_addscope(struct apn_chain *, struct apn_scope *,
	    time_t, anoubis_cookie_t);
int	apn_escalation_splice(struct apn_ruleset *, struct apn_rule *,
	    struct apn_chain *);
int	apn_escalation_rule_alf(struct apn_chain *, const struct alf_event *,
	    struct apn_default *, unsigned long flags);
int	apn_escalation_rule_sb(struct apn_chain *, struct apn_rule *,
	    struct apn_default *, const char *, unsigned long);
int	apn_escalation_rule_sfs(struct apn_chain *, struct apn_rule *,
	    struct apn_default *, const char *, int);

__END_DECLS

#endif /* _APN_H_ */

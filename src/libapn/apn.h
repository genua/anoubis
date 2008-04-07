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

#ifndef LINUX
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#define APN_FLAG_VERBOSE	0x0001
#define APN_FLAG_VERBOSE2	0x0002

#define APN_HASH_SHA256_LEN	32
#define MAX_APN_HASH_LEN	APN_HASH_SHA256_LEN

enum {
	APN_HASH_SHA256
};

enum {
	APN_CONNECT, APN_ACCEPT
};

enum {
	APN_ACTION_ALLOW, APN_ACTION_DENY, APN_ACTION_ASK
};

enum {
	APN_LOG_NONE, APN_LOG_NORMAL, APN_LOG_ALERT
};

enum {
	VAR_APPLICATION, VAR_RULE, VAR_DEFAULT, VAR_HOST, VAR_PORT,
	VAR_FILENAME
};

/* APN variables. */
struct var {
	TAILQ_ENTRY(var)	 entry;
	u_int8_t		 type;
	u_int8_t		 used;
	char			*name;
	size_t			 valsize;
	void			*value;
};

TAILQ_HEAD(apnvar_queue, var);

struct apn_app {
	char		*name;
	int		 hashtype;
	char		 hashvalue[MAX_APN_HASH_LEN];

	struct apn_app	*tail;
	struct apn_app	*next;
};

struct apn_addr {
	sa_family_t		af;
	u_int8_t		len;
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
	int			 not;
	struct apn_host		*next;
	struct apn_host		*tail;
};

struct apn_port {
	u_int16_t		 port;
	struct apn_port		*next;
	struct apn_port		*tail;
};

struct apn_afiltspec {
	int			 log;
	int			 af;
	int			 proto;
	int			 netaccess;

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

struct apn_context {
	struct apn_app		*application;
};

enum {
	APN_ALF_FILTER, APN_ALF_CAPABILITY, APN_ALF_DEFAULT, APN_ALF_CTX
};

struct apn_alfrule {
	u_int8_t		 type;
	union {
		struct apn_afiltrule	afilt;
		struct apn_acaprule	acap;
		struct apn_default	apndefault;
		struct apn_context	apncontext;
	} rule;

	struct apn_alfrule	*next;
	struct apn_alfrule	*tail;
};

enum {
	APN_ALF,  APN_SFS, APN_SB, APN_VS
};

/* Complete state of one rule. */
struct apn_rule {
	TAILQ_ENTRY(apn_rule)	 entry;
	u_int8_t		 type;

	struct apn_app		*app;

	union {
		struct apn_alfrule	*alf;
	} rule;

	struct apn_rule		*tail;
	struct apn_rule		*next;
};

/* Error message */
struct apn_errmsg {
	TAILQ_ENTRY(apn_errmsg)	 entry;
	char			*msg;
};
TAILQ_HEAD(apnerr_queue, apn_errmsg);

TAILQ_HEAD(apnrule_queue, apn_rule);

/* Complete APN ruleset. */
struct apn_ruleset {
	int			flags;

	/* Rulesets and variables */
	struct apnrule_queue	alf_queue;
	struct apnrule_queue	sfs_queue;
	struct apnvar_queue	var_queue;

	/* Error messages from the parser */
	struct apnerr_queue	err_queue;
};

int	apn_parse(const char *, struct apn_ruleset **, int);
int	apn_add_alfrule(struct apn_rule *, struct apn_ruleset *);
int	apn_print_rule(struct apn_rule *);
void	apn_print_errors(struct apn_ruleset *);
void	apn_free_ruleset(struct apn_ruleset *);

#endif /* _APN_H_ */

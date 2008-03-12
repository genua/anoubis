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

#define APN_FLAG_VERBOSE	0x0001
#define APN_FLAG_VERBOSE2	0x0002

#define APN_HASH_SHA256		1

#define APN_HASH_SHA256_LEN	32
#define MAX_APN_HASH_LEN	APN_HASH_SHA256_LEN

/* APN variables. */
struct var {
	TAILQ_ENTRY(var)	 entry;
#define VAR_APPLICATION	1
#define VAR_RULE	2
#define VAR_DEFAULT	3
#define VAR_HOST	4
#define VAR_PORT	5
#define VAR_FILENAME	6
	u_int8_t		 type;
	u_int8_t		 used;
	char			*name;
	size_t			 valsize;
	void			*value;
};

TAILQ_HEAD(apnvar_queue, var);

struct application {
	char		*name;
	int		 hashtype;
	char		 hashvalue[MAX_APN_HASH_LEN];
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

struct host {
	struct apn_addr		addr;
	int			not;
};


/* Complete state of one rule. */
struct apnrule {
	TAILQ_ENTRY(apn_rule)	rule_entry;
	u_int8_t		type;
};

TAILQ_HEAD(apnrule_queue, apnrule);

/* Complete APN ruleset. */
struct apnruleset {
	u_int32_t		rule_nr;
	int			opts;
	struct apnrule_queue	rule_queue;
	struct apnvar_queue	var_queue;
};

int	apn_parse(const char *, struct apnruleset **, int);

#endif /* _APN_H_ */

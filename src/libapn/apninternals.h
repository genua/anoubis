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

#ifndef _APNINTERNALS_H_
#define _APNINTERNALS_H_

#include "config.h"

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include "apn.h"
#include "rbtree.h"

struct apn_parser {
	int	(*parse_file)(const char *, struct apn_ruleset *);
	int	(*parse_iovec)(const char *, struct iovec *, int,
		    struct apn_ruleset *);
	int	minversion;
	int	maxversion;
};

extern struct apn_parser	apn_parser_current;

__BEGIN_DECLS

/*
 * Implemented in apnparser.c
 */
int	apn_add_alfblock(struct apn_ruleset *, struct apn_rule *,
	    const char *, int lineno);
int	apn_add_sfsblock(struct apn_ruleset *, struct apn_rule *,
	    const char *, int lineno);
int	apn_add_sbblock(struct apn_ruleset *, struct apn_rule *,
	    const char *, int lineno);
int	apn_add_ctxblock(struct apn_ruleset *, struct apn_rule *,
	    const char *, int lineno);
int	apn_error(struct apn_ruleset *, const char *, int lineno,
	    const char *fmt, ...)
	    __attribute__((format(printf, 4, 5)));
int	apn_verror(struct apn_ruleset *, const char *, int lineno,
	    const char *, va_list);
void	apn_free_chain(struct apn_chain *, struct apn_ruleset *);
void	apn_free_filter(struct apn_afiltspec *filtspec);
void	apn_free_sbaccess(struct apn_sbaccess *);
void	apn_free_sfsaccess(struct apn_sfsaccess *);
void	apn_free_sfsdefault(struct apn_sfsdefault *);
struct apn_host *apn_copy_hosts(struct apn_host *);
void	apn_assign_ids(struct apn_ruleset *);
void	apn_assign_id(struct apn_ruleset *, struct rb_entry *, void *);
int	apn_valid_id(struct apn_ruleset *, unsigned int);

__END_DECLS

#endif /* _APNINTERNALS_H_ */

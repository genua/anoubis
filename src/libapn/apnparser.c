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

#ifndef LINUX
#include <sys/queue.h>
#else
#include "queue.h"
#endif

#include <stdlib.h>

#include "apn.h"

#if 0	/* XXX HJH not yet */
static int	 apn_add_rule(struct apnruleset *, struct apnrule *);
static void	 apn_free_rule(struct apnrule *);
#endif

/* Implemented in parse.y */
extern int	 parse_rules(const char *, struct apnruleset *);

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
apn_parse(const char *filename, struct apnruleset **rsp, int flags)
{
	struct apnruleset	*rs;

	if ((rs = calloc(sizeof(struct apnruleset), 1)) == NULL)
		return (-1);
	TAILQ_INIT(&rs->rule_queue);
	TAILQ_INIT(&rs->var_queue);
	*rsp = rs;

	return (parse_rules(filename, rs));
}

#if 0	/* XXX HJH not yet */
static int
apn_add_rule(struct apnruleset *rsp, struct apnrule *rp)
{
	return (0);
}

static void
apn_free_rule(struct apnrule *rp)
{
	return;
}
#endif	/* 0 */

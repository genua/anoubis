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

#ifndef _PE_H_
#define _PE_H_

#include <sys/types.h>
#include <anoubisd.h>
#include <apn.h>

#define PE_PRIO_ADMIN	0
#define PE_PRIO_USER1	1
#define PE_PRIO_MAX	2

struct pe_proc;
struct pe_context;
struct pe_policy_db;
struct pe_user;

struct pe_proc_ident {
	unsigned char *csum;
	char *pathhint;
};

/* pe_proc access functions */
void			 pe_proc_init(void);
void			 pe_proc_dump(void);
void			 pe_proc_flush(void);
struct pe_proc		*pe_proc_get(anoubis_cookie_t cookie);
void			 pe_proc_put(struct pe_proc *proc);
struct pe_proc		*pe_proc_alloc(uid_t uid, anoubis_cookie_t cookie,
			     anoubis_cookie_t parent_cookie);
void			 pe_proc_set_parent(struct pe_proc *proc,
			     struct pe_proc *newparent);
void			 pe_proc_track(struct pe_proc *proc);
void			 pe_proc_untrack(struct pe_proc *proc);
struct pe_context	*pe_proc_get_context(struct pe_proc *, int prio);
void			 pe_proc_set_context(struct pe_proc *, int prio,
			     struct pe_context *);
void			 pe_proc_set_uid(struct pe_proc *, uid_t);
uid_t			 pe_proc_get_uid(struct pe_proc *);
int			 pe_proc_valid_context(struct pe_proc *);
anoubis_cookie_t	 pe_proc_task_cookie(struct pe_proc *);
struct pe_proc		*pe_proc_get_parent(struct pe_proc *);
pid_t			 pe_proc_get_pid(struct pe_proc *);
void			 pe_proc_set_pid(struct pe_proc *, pid_t);

/* pe_context access functions */
struct apn_rule		*pe_context_get_rule(struct pe_context *);
void			 pe_reference_ctx(struct pe_context *);
void			 pe_put_ctx(struct pe_context *);
struct pe_proc_ident	*pe_proc_ident(struct pe_proc *);
int			 pe_proc_update_db(struct pe_policy_db *,
			     struct pe_policy_db *);
void			 pe_proc_update_db_one(struct apn_ruleset *,
			     uid_t, int);
int			 pe_context_uses_rs(struct pe_context *,
			     struct apn_ruleset *);

/* Context change functions */
int			 pe_update_ctx(struct pe_proc *, struct pe_context **,
			     int, struct pe_policy_db *);
void			 pe_set_ctx(struct pe_proc *, uid_t, const u_int8_t *,
			     const char *);
void			 pe_proc_kcache_add(struct pe_proc *proc,
			     struct anoubis_kernel_policy *policy);

/* User and Policy Management */
struct apn_ruleset	*pe_user_get_ruleset(uid_t, int, struct pe_policy_db *);
anoubisd_reply_t	*pe_dispatch_policy(struct anoubisd_msg_comm *);
void			 pe_user_init(void);
void			 pe_user_flush_db(struct pe_policy_db *);
void			 pe_user_dump(void);
void			 pe_user_reconfigure(void);

/* Signature management functions */
int			 pe_verify_sig(const char *, uid_t);
#endif	/* _PE_H_ */

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
#ifdef LINUX
#include <linux/eventdev.h>
#endif
#ifdef OPENBSD
#include <dev/eventdev.h>
#endif

#define PE_PRIO_ADMIN	0
#define PE_PRIO_USER1	1
#define PE_PRIO_MAX	2

struct pe_proc;
struct pe_context;
struct pe_policy_db;
struct pe_user;
struct pe_pubkey_db;


struct pe_proc_ident {
	unsigned char *csum;
	char *pathhint;
};

struct pe_file_event {
	anoubis_cookie_t	 cookie;
	char			*path;
	u_int8_t		 cs[ANOUBIS_CS_LEN];
	unsigned int		 amask;
	int			 cslen;
	int			 uid;
};

struct pe_path_event {
	anoubis_cookie_t	 cookie;
	unsigned int		 op;
	int			 uid;
	char			*path[2];
};

/* Proc Ident management functions. */
void			 pe_proc_ident_set(struct pe_proc_ident *, const
			     u_int8_t *, const char *);
void			 pe_proc_ident_put(struct pe_proc_ident *);

/* pe_proc access functions */
void			 pe_proc_init(void);
void			 pe_proc_dump(void);
void			 pe_proc_flush(void);
struct pe_proc		*pe_proc_get(anoubis_cookie_t cookie);
void			 pe_proc_put(struct pe_proc *proc);
void			 pe_proc_fork(uid_t, anoubis_cookie_t,
			     anoubis_cookie_t);
void			 pe_proc_exec(anoubis_cookie_t, uid_t, pid_t,
			     const u_int8_t *csum, const char *pathhint);
void			 pe_proc_exit(anoubis_cookie_t);
struct pe_context	*pe_proc_get_context(struct pe_proc *, int);
void			 pe_proc_set_context(struct pe_proc *, int,
			     struct pe_context *);
void			 pe_proc_set_uid(struct pe_proc *, uid_t);
uid_t			 pe_proc_get_uid(struct pe_proc *);
anoubis_cookie_t	 pe_proc_task_cookie(struct pe_proc *);
pid_t			 pe_proc_get_pid(struct pe_proc *);
void			 pe_proc_set_pid(struct pe_proc *, pid_t);
struct pe_proc_ident	*pe_proc_ident(struct pe_proc *);
int			 pe_proc_set_sfsdisable(pid_t, uid_t);
int			 pe_proc_is_sfsdisable(struct pe_proc *, uid_t);

/* pe_context access functions */
struct pe_proc_ident	*pe_context_get_ident(struct pe_context *);
struct apn_rule		*pe_context_get_alfrule(struct pe_context *);
struct apn_rule		*pe_context_get_sbrule(struct pe_context *);
void			 pe_context_reference(struct pe_context *);
void			 pe_context_put(struct pe_context *);
int			 pe_context_uses_rs(struct pe_context *,
			     struct apn_ruleset *);

/* Context change functions */
void			 pe_context_refresh(struct pe_proc *, int,
			     struct pe_policy_db *);
void			 pe_context_exec(struct pe_proc *, uid_t,
			     struct pe_proc_ident *);
void			 pe_context_fork(struct pe_proc *, struct pe_proc *);
void			 pe_context_open(struct pe_proc *,
			     struct eventdev_hdr *);
char			*pe_context_dump(struct eventdev_hdr *,
			     struct pe_proc *, int);

/* Rule change/reload functions */
void			 pe_proc_update_db(struct pe_policy_db *);
void			 pe_proc_update_db_one(struct apn_ruleset *, int,
			     uid_t);

/* Kernel Cache related functions */
void			 pe_proc_kcache_add(struct pe_proc *proc,
			     struct anoubis_kernel_policy *policy);
void			 pe_proc_kcache_clear(struct pe_proc *proc);

/* User and Policy Management */
struct apn_ruleset	*pe_user_get_ruleset(uid_t, unsigned int,
			     struct pe_policy_db *);
anoubisd_reply_t	*pe_dispatch_policy(struct anoubisd_msg_comm *);
void			 pe_user_init(void);
void			 pe_user_flush_db(struct pe_policy_db *);
void			 pe_user_dump(void);
void			 pe_user_reconfigure(void);

/* Public Key Management */
void			 pe_pubkey_init(void);
void			 pe_pubkey_reconfigure(void);
void			 pe_pubkey_flush_db(struct pe_pubkey_db *);
int			 pe_pubkey_verifysig(const char *, uid_t);

/* General policy evaluation functions */
int			 pe_in_scope(struct apn_scope *,
			     anoubis_cookie_t, time_t);
struct pe_file_event	*pe_parse_file_event(struct eventdev_hdr *hdr);
struct pe_path_event	*pe_parse_path_event(struct eventdev_hdr *hdr);
int			 pe_compare(struct pe_proc *proc,
			     struct pe_path_event *, int, time_t);
int			 pe_compare_path(struct apn_rule **, int,
			     struct pe_path_event *, time_t);


/* Subsystem entry points for Policy decisions. */
anoubisd_reply_t	*pe_decide_alf(struct pe_proc *, struct eventdev_hdr *);
anoubisd_reply_t	*pe_decide_sfs(struct pe_proc *,
			     struct pe_file_event *, struct eventdev_hdr *);
anoubisd_reply_t	*pe_decide_sandbox(struct pe_proc *proc,
			     struct pe_file_event *, struct eventdev_hdr *);
int			 pe_sfs_getrules(uid_t, int, const char *,
			     struct apn_rule ***);
int			 pe_sb_getrules(struct pe_proc *, uid_t, int, 
			     const char *, struct apn_rule ***);


/* IPC handling */
void			 pe_ipc_connect(struct ac_ipc_message *);
void			 pe_ipc_destroy(struct ac_ipc_message *);

/* Prefix Hash */
struct pe_prefixhash;
struct pe_prefixhash	*pe_prefixhash_create(unsigned int);
void			 pe_prefixhash_destroy(struct pe_prefixhash *);
int			 pe_prefixhash_add(struct pe_prefixhash *,
			     const char *str, struct apn_rule *, int idx);
int			 pe_prefixhash_getrules(struct pe_prefixhash *,
			     const char *, struct apn_rule ***, int *);
int			 pe_build_prefixhash(struct apn_rule *);

#endif	/* _PE_H_ */

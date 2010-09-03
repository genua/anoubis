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
#include <apnrules_array.h>
#ifdef LINUX
#include <linux/eventdev.h>
#endif
#ifdef OPENBSD
#include <dev/eventdev.h>
#endif

#include <anoubis_alloc.h>
#include "aqueue.h"

#define PE_PRIO_ADMIN	0
#define PE_PRIO_USER1	1
#define PE_PRIO_MAX	2

struct pe_proc;
struct pe_context;
struct pe_policy_db;
struct pe_user;
struct pe_pubkey_db;

/*
 * Return type for the policy engine function. DO NOT use for anything else.
 * In particular, this structure must not be sent to other daemon processes
 * as an struct anoubisd_msg payload.
 */
struct anoubisd_reply {
	char		ask;		/* flag - ask GUI */
	char		hold;		/* Flag hold back answer until
					 * upgrade end. */
	time_t		timeout;	/* from policy engine, if ask GUI */
	int		reply;		/* result code */
	int		log;		/* Loglevel for the result of an ASK */
	u_int32_t	rule_id;	/* Rule ID if ask is true */
	u_int32_t	prio;		/* Priority of the rule. */
	u_int32_t	sfsmatch;	/* The type of match in SFS rules. */
	struct pe_proc_ident *pident;	/* Ident of active program */
	struct pe_proc_ident *ctxident;	/* Ident of active context */
	short		len;		/* of following msg */
	char		msg[0];
};
typedef struct anoubisd_reply anoubisd_reply_t;

/* Policy Engine main entry point. */
struct anoubisd_reply *policy_engine(struct anoubisd_msg *request);


struct pe_proc_ident {
	struct abuf_buffer	 csum;
	char			*pathhint;
};

#define PE_UPGRADE_TOUCHED	0x0001
#define PE_UPGRADE_WRITEOK	0x0002

struct pe_file_event {
	anoubis_cookie_t	 cookie;
	char			*path;
	struct abuf_buffer	 csum;
	unsigned int		 amask;
	int			 uid;
	unsigned int		 upgrade_flags;
	struct eventdev_hdr	*rawhdr;
};

struct pe_path_event {
	anoubis_cookie_t	 cookie;
	unsigned int		 op;
	int			 uid;
	char			*path[2];
};

/* Proc Ident management functions. */
void			 pe_proc_ident_set(struct pe_proc_ident *,
			     const struct abuf_buffer csum, const char * path);
void			 pe_proc_ident_put(struct pe_proc_ident *);

/* pe_proc access functions */
void			 pe_proc_init(void);
void			 pe_proc_dump(void);
void			 pe_proc_flush(void);
struct pe_proc		*pe_proc_get(anoubis_cookie_t cookie);
void			 pe_proc_put(struct pe_proc *proc);
void			 pe_proc_fork(uid_t, anoubis_cookie_t cookie,
			     anoubis_cookie_t parent, anoubis_cookie_t pgid);
void			 pe_proc_exec(anoubis_cookie_t, uid_t, pid_t,
			     const struct abuf_buffer csum,
			     const char *pathhint, anoubis_cookie_t pgid,
			     int secure);
int			 pe_proc_flag_transition(struct pe_proc *proc,
			     struct pe_file_event *fevent);
void			 pe_proc_exit(anoubis_cookie_t);
void			 pe_proc_addinstance(anoubis_cookie_t);
void			 pe_proc_add_thread(anoubis_cookie_t);
void			 pe_proc_remove_thread(anoubis_cookie_t);
int			 pe_proc_is_running(anoubis_cookie_t);
struct pe_context	*pe_proc_get_context(struct pe_proc *, int);
void			 pe_proc_set_context(struct pe_proc *, int,
			     struct pe_context *);
void			 pe_proc_set_uid(struct pe_proc *, uid_t);
uid_t			 pe_proc_get_uid(struct pe_proc *);
anoubis_cookie_t	 pe_proc_task_cookie(struct pe_proc *);
pid_t			 pe_proc_get_pid(struct pe_proc *);
void			 pe_proc_set_pid(struct pe_proc *, pid_t);
struct pe_proc_ident	*pe_proc_ident(struct pe_proc *);
int			 pe_proc_is_secure(struct pe_proc *);
void			 pe_proc_save_ctx(struct pe_proc *, int,
			     anoubis_cookie_t);
void			 pe_proc_restore_ctx(struct pe_proc *, int,
			     anoubis_cookie_t);
struct pe_context	*pe_proc_get_savedctx(struct pe_proc *, int);
void			 pe_proc_set_savedctx( struct pe_proc *, int,
			     struct pe_context *);
unsigned int		 pe_proc_is_upgrade(struct pe_proc *);
unsigned int		 pe_proc_is_upgrade_parent(struct pe_proc *);
void			 pe_proc_upgrade_addmark(struct pe_proc *);
void			 pe_proc_upgrade_clrmark(struct pe_proc *);
void			 pe_proc_upgrade_clrallmarks(void);
void			 pe_proc_hold(struct pe_proc *);
int			 pe_proc_is_hold(struct pe_proc *);
void			 pe_proc_release(void);

/* pe_context access functions */
struct pe_proc_ident	*pe_context_get_ident(struct pe_context *);
struct apn_rule		*pe_context_get_alfrule(struct pe_context *);
struct apn_rule		*pe_context_get_sbrule(struct pe_context *);
void			 pe_context_reference(struct pe_context *);
void			 pe_context_put(struct pe_context *);
int			 pe_context_uses_rs(struct pe_context *,
			     struct apn_ruleset *);
int			 pe_context_is_nosfs(struct pe_context *);
int			 pe_context_is_pg(struct pe_context *);

/* Context change functions */
void			 pe_context_refresh(struct pe_proc *, int,
			     struct pe_policy_db *);
void			 pe_context_exec(struct pe_proc *, uid_t,
			     struct pe_proc_ident *);
int			 pe_context_will_transition(struct pe_proc *, uid_t,
			     struct pe_proc_ident *);
int			 pe_context_will_pg(struct pe_proc *, uid_t,
			     struct pe_proc_ident *, int *ruleidp, int *priop);
void			 pe_context_fork(struct pe_proc *, struct pe_proc *);
void			 pe_context_open(struct pe_proc *,
			     struct eventdev_hdr *);
void			 pe_context_borrow(struct pe_proc *, struct pe_proc *,
			     anoubis_cookie_t);
void			 pe_context_restore(struct pe_proc *, anoubis_cookie_t);
char			*pe_context_dump(struct eventdev_hdr *,
			     struct pe_proc *, int);

/* Rule change/reload functions */
void			 pe_proc_update_db(struct pe_policy_db *);
void			 pe_proc_update_db_one(struct apn_ruleset *, int,
			     uid_t);

/* User and Policy Management */
struct apn_ruleset	*pe_user_get_ruleset(uid_t, unsigned int,
			     struct pe_policy_db *);
struct anoubisd_msg	*pe_dispatch_policy(struct anoubisd_msg *);
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
int			 pe_compare(struct pe_proc *proc,
			     struct pe_path_event *, int, time_t);

/* Upgrade related functions. */
void			 pe_set_upgrade_ok(int);
void			 pe_upgrade_start(struct pe_proc *);
void			 pe_upgrade_end(struct pe_proc *);
void			 pe_upgrade_finish(void);
void			 pe_upgrade_filelist_start(void);
void			 pe_upgrade_filelist_next(void);
struct pe_file_node	*pe_upgrade_filelist_get(void);

/* Subsystem entry points for Policy decisions. */
anoubisd_reply_t	*pe_decide_alf(struct pe_proc *, struct eventdev_hdr *);
anoubisd_reply_t	*pe_decide_sfs(struct pe_proc *,
			     struct pe_file_event *);
anoubisd_reply_t	*pe_decide_sandbox(struct pe_proc *proc,
			     struct pe_file_event *);
int			 pe_sfs_getrules(uid_t, int, const char *,
			     struct apnarr_array *);
int			 pe_sb_getrules(struct pe_proc *, uid_t, int,
			     const char *, struct apnarr_array *);


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
			     const char *, struct apnarr_array *rulesp);
int			 pe_build_prefixhash(struct apn_rule *);

/* Playground management */
void			 pe_playground_add(anoubis_cookie_t pgid,
			     struct pe_proc *);
void			 pe_playground_delete(anoubis_cookie_t pgid,
			     struct pe_proc *);
anoubis_cookie_t	 pe_proc_get_playgroundid(struct pe_proc *);
void			 pe_proc_set_playgroundid(struct pe_proc *,
			     anoubis_cookie_t pgid);
void			 pe_playground_postexec(anoubis_cookie_t pgid,
			     struct pe_proc *);
void			 pe_playground_file_instantiate(anoubis_cookie_t pgid,
			     uint64_t dev, uint64_t ino, const char *path);
void			 pe_playground_file_delete(anoubis_cookie_t pgid,
			     uint64_t dev, uint64_t ino);
int			 pe_playground_file_scanrequest(anoubis_cookie_t,
			     uint64_t dev, uint64_t ino, const char *path,
			     uid_t uid);
void			 pe_playground_dump(void);
void			 pe_playground_init(void);
void			 pe_playground_dispatch_request(struct anoubisd_msg *,
			     Queue *queue);
void			 pe_playground_dispatch_commit(struct anoubisd_msg *,
			     Queue *session, Queue *master);
void			 pe_playground_dispatch_commitreply(
			     struct anoubisd_msg*);
void			 pe_playground_notify_forced(struct pe_proc_ident *,
			     struct eventdev_hdr *hdr, uint32_t ruleid,
			     uint32_t prio);

/*
 * Entry points exported for the benefit of unit tests.
 * DO NOT USE THESE FROM NORMAL CODE.
 */

anoubisd_reply_t	*test_pe_handle_sfs(struct eventdev_hdr *hdr);

#endif	/* _PE_H_ */

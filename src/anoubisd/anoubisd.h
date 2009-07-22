/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#ifndef _ANOUBISD_H
#define _ANOUBISD_H

#include "config.h"

#include <sys/file.h>
#include <sys/types.h>
#include <sys/param.h>
#include <grp.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#include <linux/anoubis_sfs.h>
#endif
#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#include <dev/anoubis_sfs.h>
#endif
#include <assert.h>

#ifndef NEEDBSDCOMPAT
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#ifdef LINUX
#define ANOUBISCORE_MIN_VERSION	0x00010002UL
#elif defined OPENBSD
#define ANOUBISCORE_MIN_VERSION	0x00010001UL
#else
#define ANOUBISCORE_MIN_VERSION ANOUBISCORE_VERSION
#endif
#define ANOUBISCORE_LOCK_VERSION 0x00010004UL

#include <anoubis_protocol.h>

#define ANOUBISD_OPT_VERBOSE		0x0001
#define ANOUBISD_OPT_VERBOSE2		0x0002
#define ANOUBISD_OPT_NOACTION		0x0004

#define ANOUBISD_USER			"_anoubisd"

#define ANOUBISD_POLICYCHROOT		"/policy"
#define ANOUBISD_USERDIR		"user"
#define ANOUBISD_ADMINDIR		"admin"
#define ANOUBISD_DEFAULTNAME		"default"
#define ANOUBISD_PUBKEYDIR		"pubkeys"

#define SFS_CHECKSUMROOT		PACKAGE_POLICYDIR "/sfs"
#define SFS_CHECKSUMCHROOT		"/sfs"

#define CERT_DIR_CHROOT			ANOUBISD_POLICYCHROOT "/pubkeys"
#define CERT_DIR			PACKAGE_POLICYDIR CERT_DIR_CHROOT

#define __used __attribute__((unused))

#ifdef LINUX
#define __dead
#define UID_MAX	UINT_MAX
#endif

typedef enum
{
	ANOUBISD_UPGRADE_MODE_OFF,
	ANOUBISD_UPGRADE_MODE_STRICT_LOCK,
	ANOUBISD_UPGRADE_MODE_LOOSE_LOCK,
	ANOUBISD_UPGRADE_MODE_PROCESS
} anoubisd_upgrade_mode;

LIST_HEAD(anoubisd_upgrade_trigger_list, anoubisd_upgrade_trigger);

struct anoubisd_upgrade_trigger {
	char *arg;
	LIST_ENTRY(anoubisd_upgrade_trigger) entries;
};

struct anoubisd_config {
	int	opts;
	char * unixsocket;        /* defaults to ANOUBISD_SOCKETNAME */

	/* Upgrade options */
	anoubisd_upgrade_mode			upgrade_mode;
	struct anoubisd_upgrade_trigger_list	upgrade_trigger;
};

extern struct anoubisd_config anoubisd_config;

struct session_reg {
	u_int32_t	session_id;
	int		flag;
};

struct anoubisd_msg {
	short		size;
	short		mtype;
	char		msg[0];
};
typedef struct anoubisd_msg anoubisd_msg_t;

enum anoubisd_msg_type {
	ANOUBISD_MSG_POLREQUEST,
	ANOUBISD_MSG_POLREPLY,
	ANOUBISD_MSG_EVENTDEV,
	ANOUBISD_MSG_LOGREQUEST,
	ANOUBISD_MSG_EVENTREPLY,
	ANOUBISD_MSG_EVENTCANCEL,
	ANOUBISD_MSG_CHECKSUM_OP,
	ANOUBISD_MSG_EVENTASK,
	ANOUBISD_MSG_SFSDISABLE,
	ANOUBISD_MSG_POLICYCHANGE,
	ANOUBISD_MSG_SFSCACHE_INVALIDATE,
	ANOUBISD_MSG_UPGRADE,
	ANOUBISD_MSG_SFS_UPDATE_ALL,
	ANOUBISD_MSG_CONFIG,
	ANOUBISD_MSG_LOGIT,
};

/* format of ANOUBISD_MSG_EVENTDEV is struct eventdev_hdr */

struct anoubisd_msg_eventask
{
	u_int32_t	rule_id;
	u_int32_t	prio;
	u_int32_t	sfsmatch;
	u_int16_t	csumoff, csumlen;
	u_int16_t	pathoff, pathlen;
	u_int16_t	ctxcsumoff, ctxcsumlen;
	u_int16_t	ctxpathoff, ctxpathlen;
	u_int16_t	evoff, evlen;
	char		payload[0];
};
typedef struct anoubisd_msg_eventask anoubisd_msg_eventask_t;

/* format of ANOUBISD_MSG_POLREQUEST */
struct anoubisd_msg_comm {
	u_int64_t	token;
	u_int32_t	uid;
	u_int32_t	flags;		/* Only for POLREQUEST */
	unsigned short	len;		/* of following msg */
	char		msg[0];
};
typedef struct anoubisd_msg_comm anoubisd_msg_comm_t;

struct anoubisd_msg_checksum_op {
	u_int64_t	token;
	u_int32_t	uid;
	short		len;
	char		msg[0];
};
typedef struct anoubisd_msg_checksum_op anoubisd_msg_checksum_op_t;

struct anoubisd_msg_sfsdisable {
	u_int64_t	token;
	u_int32_t	uid;
	u_int32_t	pid;
};
typedef struct anoubisd_msg_sfsdisable anoubisd_msg_sfsdisable_t;

struct anoubisd_reply {
	char		ask;		/* flag - ask GUI */
	char		hold;		/* Flag hold back answer until
					 * upgrade end. */
	time_t		timeout;	/* from policy engine, if ask GUI */
	int		reply;		/* result code */
	int		log;		/* Loglevel for the result of an ASK */
	u_int64_t	token;		/* only for anoubisd_msg_comm_t msgs */
	u_int32_t	flags;		/* Only for POLREPLY */
	u_int32_t	rule_id;	/* Rule ID if ask is true */
	u_int32_t	prio;		/* Priority of the rule. */
	u_int32_t	sfsmatch;	/* The type of match in SFS rules. */
	struct pe_proc_ident *pident;	/* Ident of active program */
	struct pe_proc_ident *ctxident;	/* Ident of active context */
	short		len;		/* of following msg */
	char		msg[0];
};
typedef struct anoubisd_reply anoubisd_reply_t;

struct anoubisd_sfscache_invalidate {
	u_int32_t	uid;
	u_int32_t	plen;
	u_int32_t	keylen;
	char		payload[0];
};
typedef struct anoubisd_sfscache_invalidate anoubisd_sfscache_invalidate_t;

struct anoubisd_sfs_update_all {
	u_int32_t	cslen;
	char		payload[0];
};
typedef struct anoubisd_sfs_update_all anoubisd_sfs_update_all_t;

/* format of ANOUBISD_MSG_LOGREQUEST */
struct anoubisd_msg_logrequest
{
	u_int32_t		error;
	u_int32_t		loglevel;
	u_int32_t		rule_id;
	u_int32_t		prio;
	u_int32_t		sfsmatch;
	struct eventdev_hdr	hdr;
	/* Eventdev data follows. */
};

struct anoubisd_msg_logit {
	u_int32_t		prio;
	char			msg[0];
};

/* format of ANOUBISD_MSG_POLICYCHANGE */
struct anoubisd_msg_pchange
{
	u_int32_t	uid;
	u_int32_t	prio;
};

enum anoubisd_upgrade {
	ANOUBISD_UPGRADE_START,
	ANOUBISD_UPGRADE_END,
	ANOUBISD_UPGRADE_CHUNK_REQ,
	ANOUBISD_UPGRADE_CHUNK,
};

struct anoubisd_msg_upgrade
{
	u_int8_t	upgradetype;
	u_int32_t	chunksize;
	char		chunk[0];
};
typedef struct anoubisd_msg_upgrade anoubisd_msg_upgrade_t;

struct anoubisd_msg_config
{
	u_int32_t	opts;
	u_int8_t	upgrade_mode;
	u_int32_t	triggercount;
	char		chunk[0];	/* socket and trigger list */
};
typedef struct anoubisd_msg_config anoubisd_msg_config_t;

enum anoubisd_process_type {
	PROC_MAIN,
	PROC_POLICY,
	PROC_SESSION,
	PROC_LOGGER,
	PROC_UPGRADE
};
extern enum anoubisd_process_type	anoubisd_process;

pid_t	session_main(int[], int[], int[], int[], int[]);
pid_t	policy_main(int[], int[], int[], int[], int[]);
pid_t	logger_main(int[], int[], int[], int[]);
pid_t	upgrade_main(int[], int[], int[], int[], int[]);

void	pe_init(void);

void	pe_shutdown(void);

void	pe_reconfigure(void);

anoubisd_reply_t *policy_engine(anoubisd_msg_t *request);

void	log_init(int fd);

void	log_warn(const char *, ...)
    __attribute__ ((format (printf, 1, 2)));

void	log_warnx(const char *, ...)
    __attribute__ ((format (printf, 1, 2)));

void	log_info(const char *, ...)
    __attribute__ ((format (printf, 1, 2)));

void	log_debug(const char *, ...)
    __attribute__ ((format (printf, 1, 2)));

/*@noreturn@*/
__dead void	fatalx(const char *);

/*@noreturn@*/
__dead void	fatal(const char *);

/*@noreturn@*/
__dead void	master_terminate(int);

/*@noreturn@*/
__dead void	early_err(int, const char *);

/*@noreturn@*/
__dead void	early_errx(int, const char *);

anoubisd_msg_t *msg_factory(int, int);
void		msg_shrink(anoubisd_msg_t *, int);

void	pe_dump(void);
int	send_policy_data(u_int64_t token, int fd);

void	send_lognotify(struct eventdev_hdr *, u_int32_t, u_int32_t, u_int32_t,
	    u_int32_t, u_int32_t);
void	send_policychange(u_int32_t uid, u_int32_t prio);
void	flush_log_queue(void);

void send_upgrade_start(void);

extern char *logname;

#ifndef S_SPLINT_S
static inline const char	*debug_proc_name(void)
{
	switch (anoubisd_process) {
	case PROC_MAIN:		return "main"; break;
	case PROC_LOGGER:	return "logger"; break;
	case PROC_POLICY:	return "policy"; break;
	case PROC_SESSION:	return "session"; break;
	case PROC_UPGRADE:	return "upgrade"; break;
	default:		return "<unknown>"; break;
	}
}
#define DEBUG(flag, fmt, ...)					\
	do {							\
		if (flag & debug_flags) {			\
			log_debug("%s: " fmt,			\
			    debug_proc_name(), ## __VA_ARGS__);	\
		}						\
	} while(0)
#else
/* Splint does not parse varadic macros */
#define DEBUG
#endif

extern u_int32_t	debug_flags;
extern u_int32_t	debug_stderr;
extern gid_t		anoubisd_gid;
extern unsigned long	version;

#define DBG_MSG_FD	0x0001
#define DBG_MSG_SEND	0x0002
#define DBG_MSG_RECV	0x0004
#define DBG_TRACE	0x0008
#define DBG_QUEUE	0x0010
#define DBG_PE		0x0020
#define DBG_PE_PROC	0x0040
#define DBG_PE_SFS	0x0080
#define DBG_PE_ALF	0x0100
#define DBG_PE_POLICY	0x0200
#define DBG_PE_TRACKER	0x0400
#define DBG_PE_CTX	0x0800
#define DBG_PE_DECALF	0x1000
#define DBG_SESSION	0x2000
#define DBG_SANDBOX	0x4000
#define DBG_SFSCACHE	0x8000
#define DBG_PE_BORROW	0x10000
#define DBG_UPGRADE	0x20000

#endif /* !_ANOUBISD_H */

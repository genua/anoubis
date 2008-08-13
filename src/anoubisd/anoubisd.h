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

#define ANOUBISD_OPT_VERBOSE		0x0001
#define ANOUBISD_OPT_VERBOSE2		0x0002
#define ANOUBISD_OPT_NOACTION		0x0004

#define ANOUBISD_USER			"_anoubisd"

#define ANOUBISD_SOCKETNAME		"/var/run/anoubisd.sock"
#define ANOUBISD_PIDFILENAME		"/var/run/anoubisd.pid"

#define ANOUBISD_POLICYDIR		"/etc/anoubis/policy"
#define ANOUBISD_POLICYCHROOT		"/"
#define ANOUBISD_USERDIR		"user"
#define ANOUBISD_ADMINDIR		"admin"
#define ANOUBISD_DEFAULTNAME		"default"
#define ANOUBISD_PUBKEYDIR		"pubkeys"

#define SFS_CHECKSUMROOT		"/var/lib/sfs"

#define __used __attribute__((unused))

enum {
	ANOUBISD_LOG_RAW,
	ANOUBISD_LOG_APN
};

#ifdef LINUX
#define __dead
#define UID_MAX	UINT_MAX
#endif

struct anoubisd_config {
	int	opts;
	const char * unixsocket;        /* defaults to ANOUBISD_SOCKETNAME */
};

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
enum {
	ANOUBISD_MSG_POLREQUEST,
	ANOUBISD_MSG_POLREPLY,
	ANOUBISD_MSG_EVENTDEV,
	ANOUBISD_MSG_LOGREQUEST,
	ANOUBISD_MSG_EVENTREPLY,
	ANOUBISD_MSG_EVENTCANCEL,
	ANOUBISD_MSG_SESSION_REG,
	ANOUBISD_MSG_CHECKSUM_OP,
	ANOUBISD_MSG_SFSOPEN,
	ANOUBISD_MSG_EVENTASK,
	ANOUBISD_MSG_KCACHE,
} anoubisd_msg;

/* format of ANOUBISD_MSG_EVENTDEV is struct eventdev_hdr */

struct anoubisd_msg_eventask
{
	u_int32_t	rule_id;
	u_int16_t	csumoff, csumlen;
	u_int16_t	pathoff, pathlen;
	u_int16_t	evoff, evlen;
	char		payload[0];
};
typedef struct anoubisd_msg_eventask anoubisd_msg_eventask_t;

struct anoubisd_msg_sfsopen {
	u_int32_t		rule_id;
	u_int32_t		anoubisd_csum_set;
	u_int8_t		anoubisd_csum[ANOUBIS_SFS_CS_LEN];
	struct eventdev_hdr	hdr;
};
typedef struct anoubisd_msg_sfsopen anoubisd_msg_sfsopen_t;

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

struct anoubisd_reply {
	short		ask;		/* flag - ask GUI */
	time_t		timeout;	/* from policy engine, if ask GUI */
	int		reply;		/* result code */
	u_int64_t	token;		/* only for anoubisd_msg_comm_t msgs */
	u_int32_t	flags;		/* Only for POLREPLY */
	u_int32_t	rule_id;	/* Rule ID if ask is true */
	u_int8_t	*csum;
	char		*path;
	short		len;		/* of following msg */
	char		msg[0];
};
typedef struct anoubisd_reply anoubisd_reply_t;

/* format of ANOUBISD_MSG_LOGREQUEST */
struct anoubisd_msg_logrequest
{
	u_int32_t		error;
	u_int32_t		loglevel;
	u_int32_t		rule_id;
	struct eventdev_hdr	hdr;
	/* Eventdev data follows. */
};

enum {
	PROC_MAIN,
	PROC_POLICY,
	PROC_SESSION,
	PROC_LOGGER
} anoubisd_process;

pid_t	session_main(struct anoubisd_config *, int[], int[], int[], int[]);

pid_t	policy_main(struct anoubisd_config *, int[], int[], int[], int[]);

pid_t	logger_main(struct anoubisd_config *, int[], int[], int[]);

void	pe_init(void);

void	pe_shutdown(void);

void	pe_reconfigure(void);

anoubisd_reply_t *policy_engine(anoubisd_msg_t *request);

void	policy_msg2master(anoubisd_msg_t *);

void	log_init(int fd);

void	log_warn(const char *, ...);

void	log_warnx(const char *, ...);

void	log_info(const char *, ...);

void	log_debug(const char *, ...);

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

void	pe_dump(void);
int	send_policy_data(u_int64_t token, int fd);

void	send_lognotify(struct eventdev_hdr *, u_int32_t, u_int32_t, u_int32_t);

struct credentials {
	uid_t   euid;
	gid_t   egid;
	gid_t   groups[NGROUPS_MAX];
	int     ngroups;
};
int	restore_uid(struct credentials *cred);
int	switch_uid(uid_t uid, struct credentials *cred);

#ifndef S_SPLINT_S
#define DEBUG(flag, ...) {if (flag & debug_flags) log_debug(__VA_ARGS__);}
#else
/* Splint does not parse varadic macros */
#define DEBUG
#endif

u_int32_t debug_flags;
u_int32_t debug_stderr;

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

#endif /* !_ANOUBISD_H */

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
#ifndef ANOUBIS_PROTOCOL_H
#define ANOUBIS_PROTOCOL_H

#include <sys/param.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <assert.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#define ANOUBIS_PROTO_CONNECT		0
#define ANOUBIS_PROTO_POLICY		1
#define ANOUBIS_PROTO_NOTIFY		2
#define ANOUBIS_PROTO_BOTH		3

#define ANOUBIS_SFS_NONE		0
#define ANOUBIS_SFS_DEFAULT		1
#define ANOUBIS_SFS_VALID		2
#define ANOUBIS_SFS_INVALID		3
#define ANOUBIS_SFS_UNKNOWN		4

#define DEFINE_UNSIGNED_TYPE(BITS)		\
	typedef union __attribute__((packed)) {	\
		unsigned char bytes[BITS/8];	\
		u_int##BITS##_t netint;		\
	} /*@unsignedintegraltype@*/ u##BITS##n;

#define DEFINE_SIGNED_TYPE(BITS)		\
	typedef union __attribute__((packed)) {	\
		unsigned char bytes[BITS/8];	\
		int##BITS##_t netint;		\
	} /*@signedintegraltype@*/ s##BITS##n;

/* Tokens are opaque data, i.e. we do not have to worry about endianness. */
typedef  u_int64_t /*@unsignedintegraltype@*/ anoubis_token_t;

DEFINE_UNSIGNED_TYPE(8);
DEFINE_UNSIGNED_TYPE(16);
DEFINE_UNSIGNED_TYPE(32);
DEFINE_UNSIGNED_TYPE(64);

DEFINE_SIGNED_TYPE(8);
DEFINE_SIGNED_TYPE(16);
DEFINE_SIGNED_TYPE(32);

#if ! defined (S_SPLINT_S) && ! defined (lint)
#if BYTE_ORDER == BIG_ENDIAN
#define __ntohll(X) (X)
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
static inline u_int64_t __ntohll(u_int64_t arg)
{
	unsigned char tmp;
	union {
		u_int64_t	val;
		unsigned char	buf[8];
	} u;
	u.val = arg;
	tmp = u.buf[0]; u.buf[0] = u.buf[7]; u.buf[7] = tmp;
	tmp = u.buf[1]; u.buf[1] = u.buf[6]; u.buf[6] = tmp;
	tmp = u.buf[2]; u.buf[2] = u.buf[5]; u.buf[5] = tmp;
	tmp = u.buf[3]; u.buf[3] = u.buf[4]; u.buf[4] = tmp;
	return u.val;
}
#endif

#define get_value(VAR)		({				\
	__typeof__(&(VAR)) __ptr = &(VAR);			\
	__typeof__(__ptr->netint) __ret;			\
	switch (sizeof(__ptr->netint)) {			\
	case 1:		__ret = (__ptr->netint); break;		\
	case 2:		__ret = ntohs(__ptr->netint); break;	\
	case 4:		__ret = ntohl(__ptr->netint); break;	\
	case 8:		__ret = __ntohll(__ptr->netint); break;	\
	default:						\
		assert(0);					\
	}							\
	__ret;							\
})
#else
#define get_value(VAR) 0 /* ((VAR).netint) */
#endif

#if ! defined (S_SPLINT_S) && ! defined (lint)
#define set_value(VAR,VAL)	do {				\
	__typeof__(&(VAR)) __ptr = &(VAR);			\
	__typeof__(__ptr->netint) __val = (VAL);		\
	switch (sizeof(__ptr->netint)) {			\
	case 1:		__ptr->netint = __val; break;		\
	case 2:		__ptr->netint = htons(__val); break;	\
	case 4:		__ptr->netint = htonl(__val); break;	\
	case 8:		__ptr->netint = __ntohll(__val); break;	\
	default:						\
		assert(0);					\
	}							\
} while(0)
#else
#define set_value(VAR,VAL) (((VAR).netint) = (VAL))
#endif

/* MESSAGE TYPES */

/* General Message types */
#define		ANOUBIS_REPLY		0x1000

/* CONNECT Protocol */
#define		ANOUBIS_C_HELLO		0x2000
#define		ANOUBIS_C_VERSEL	0x2001
#define		ANOUBIS_C_AUTH		0x2010
#define		ANOUBIS_C_AUTHDATA	0x2011
#define		ANOUBIS_C_AUTHREPLY	0x2012
#define		ANOUBIS_C_OPTREQ	0x2020
#define		ANOUBIS_C_OPTACK	0x2021
#define		ANOUBIS_C_PROTOSEL	0x2022
#define		ANOUBIS_C_CLOSEREQ	0x2030
#define		ANOUBIS_C_CLOSEACK	0x2031

/* POLICY Protocol */

#define		ANOUBIS_P_MIN		0x3000

#define		ANOUBIS_P_CSUMREQUEST	0x3100
#define		ANOUBIS_P_CSUM_LIST	0x3101

#define		ANOUBIS_P_SFSDISABLE	0x3200

#define		ANOUBIS_P_REQUEST	0x3800
#define		ANOUBIS_P_REPLY		0x3801

#define		ANOUBIS_P_MAX		0x3FFF

/* NOTIFY Protocol */
#define		ANOUBIS_N_MIN		0x4000

#define		ANOUBIS_N_REGISTER	0x4000
#define		ANOUBIS_N_UNREGISTER	0x4001
#define		ANOUBIS_N_ASK		0x4010
#define		ANOUBIS_N_DELEGATE	0x4011
#define		ANOUBIS_N_RESYOU	0x4012
#define		ANOUBIS_N_RESOTHER	0x4013
#define		ANOUBIS_N_NOTIFY	0x4014
#define		ANOUBIS_N_LOGNOTIFY	0x4015
#define		ANOUBIS_N_POLICYCHANGE	0x4016
#define		ANOUBIS_N_CTXREQ	0x4020
#define		ANOUBIS_N_CTXREPLY	0x4021

#define		ANOUBIS_N_MAX		0x4FFF

/*
 * Checksum Request Flags:
 * The flags marked as _Deprecated_ must not be used in new code.
 * Meaning of individual flags:
 *   - ANOUBIS_CSUM_UID: Apply command to user IDS/unsigend checksums
 *   - ANOBUIS_CSUM_KEY: Apply command to key IDS/signed checksums
 *   - ANOUBIS_CSUM_ALL: Apply command to all user/key IDs not just to
 *	a single ID. This requires root privileges.
 *   - ANOUBIS_CSUM_WANTIDS: List available signatures/checksums for a single
 *	file instead of listing contents of a directory.
 *   - ANOUBIS_CSUM_UPGRADED: Report only files/directories that contain
 *      upgraded checksums that differ from an existing checksum.
 *
 * Only certain combinations of flags are allowed:
 *    WANTIDS     ALL         UID           KEY            UPGRADED
 *    true        true        true/false    true/false     false
 *    false       true        true          true           false
 *    false       false       true/false    true/false     false
 *    false       false       false         true           true
 *
 * Non-List operations only support three values:
 *    ANOUBIS_CSUM_NONE, ANOUBIS_CSUM_UID, ANOUBIS_CSUM_KEY.
 * UID and KEY cannot be combined, ANOUBIS_CSUM_NONE is equivalent
 * ot ANOUBIS_CSUM_UID with the current user ID.
 */

#define		ANOUBIS_CSUM_NONE	0x0000
#define		ANOUBIS_CSUM_UID	0x0001
#define		ANOUBIS_CSUM_KEY	0x0002
#define		_ANOUBIS_CSUM_UID_ALL	0x0004		/* Deprecated */
#define		_ANOUBIS_CSUM_KEY_ALL	0x0008		/* Deprecated */
#define		ANOUBIS_CSUM_ALL	0x0010
#define		ANOUBIS_CSUM_WANTIDS	0x0020
#define		ANOUBIS_CSUM_UPGRADED	0x0040

#define		ANOUBIS_CSUM_FLAG_MASK	(0x0073UL)

#define ANOUBIS_IS_NOTIFY(X) ((ANOUBIS_N_MIN)<=(X) && (X) <= (ANOUBIS_N_MAX))
#define ANOUBIS_IS_POLICY(X) ((ANOUBIS_P_MIN)<=(X) && (X) <= (ANOUBIS_P_MAX))

#define CSUM_LEN	4

typedef struct {
	u32n	type;
	char	payload[0];
} __attribute__((packed)) Anoubis_GeneralMessage;

typedef struct{
	u32n	type;
	u16n	version;
	u16n	min_version;
} __attribute__((packed)) Anoubis_HelloMessage;

typedef struct {
	u32n	type;
	anoubis_token_t	token;
	u32n	opcode;
	u32n	error;
} __attribute__((packed)) Anoubis_AckMessage;

typedef struct {
	u32n		type;
	anoubis_token_t	token;
	u32n		opcode;
	u32n		error;
	u_int8_t	payload[0];
} __attribute__((packed)) Anoubis_AckPayloadMessage;

typedef struct {
	u32n		type;
	u32n		flags;
	u32n		error;
	u_int8_t	payload[0];
} __attribute__((packed)) Anoubis_ChecksumPayloadMessage;

typedef struct {
	u32n	type;
	u16n	version;
	u16n	_pad;
} __attribute__((packed)) Anoubis_VerselMessage;

typedef struct {
	u32n	type;
	u32n	error;
	u32n	uid;
	char	name[0];

} __attribute__((packed)) Anoubis_AuthReplyMessage;

typedef struct {
	u32n	type;
	char	stringlist[0];
} __attribute__((packed)) Anoubis_StringListMessage;

typedef struct {
	u32n	type;
	anoubis_token_t token;
} __attribute__((packed)) Anoubis_TokenMessage;

typedef struct {
	u32n	type;
	anoubis_token_t token;
	u32n	rule_id;
	u32n	uid;
	u32n	subsystem;
} __attribute__((packed)) Anoubis_NotifyRegMessage;

typedef struct {
	u32n	type;
	anoubis_token_t	token;
	u64n	task_cookie;
	u32n	pid;
	u32n	rule_id;
	u32n	prio;
	u32n	uid;
	u32n	subsystem;
	u32n	sfsmatch;
	/*
	 * The following two fields are only valid if the message type
	 * is ANOUBIS_N_LOGNOTIFY.
	 */
	u32n	loglevel;
	u32n	error;
	u16n	csumoff, csumlen;
	u16n	pathoff, pathlen;
	u16n	ctxcsumoff, ctxcsumlen;
	u16n	ctxpathoff, ctxpathlen;
	u16n	evoff, evlen;
	char	payload[0];
} __attribute__((packed)) Anoubis_NotifyMessage;

typedef struct {
	u32n	type;
	u32n	uid;
	u32n	prio;
} __attribute__((packed)) Anoubis_PolicyChangeMessage;

typedef struct {
	u32n	type;
	anoubis_token_t token;
	u32n	uid;
	u32n	error;
} __attribute__((packed)) Anoubis_NotifyResultMessage;

#define	POLICY_FLAG_START	0x0001UL
#define	POLICY_FLAG_END		0x0002UL

typedef struct {
	u32n	type;
	u32n	flags;
	char	payload[0];
} __attribute__((packed)) Anoubis_PolicyRequestMessage;

typedef struct {
	u32n	type;
	u32n	flags;
	u32n	error;
	char	payload[0];
} __attribute__((packed)) Anoubis_PolicyReplyMessage;

/* Payload subformat for PolicyRequest messages. */

#define ANOUBIS_PTYPE_GETBYUID		0x100UL
#define ANOUBIS_PTYPE_SETBYUID		0x101UL

typedef struct {
	u32n	ptype;
	char	payload[0];
} __attribute__((packed)) Policy_Generic;

typedef struct {
	u32n	ptype;
	u32n	uid;
	u32n	prio;
} __attribute__((packed)) Policy_GetByUid;

typedef struct {
	u32n	ptype;
	u32n	uid;
	u32n	prio;
	u16n	siglen;
	char	payload[0];
} __attribute__((packed)) Policy_SetByUid;

/*
 * The list functions that are marked as _Deprecated_ are only defined
 * here for backwards compatibility with older versions. All new code
 * must use ANOUBIS_CHECKSUM_OP_GENERIC_LIST.
 */

#define ANOUBIS_CHECKSUM_OP_DEL			0x0002UL
#define _ANOUBIS_CHECKSUM_OP_GET_OLD		0x0003UL	/* Deprecated */
#define ANOUBIS_CHECKSUM_OP_ADDSUM		0x0005UL
#define _ANOUBIS_CHECKSUM_OP_LIST		0x0006UL	/* Deprecated */
#define _ANOUBIS_CHECKSUM_OP_UID_LIST		0x0007UL	/* Deprecated */
#define ANOUBIS_CHECKSUM_OP_ADDSIG		0x0008UL
#define _ANOUBIS_CHECKSUM_OP_GETSIG_OLD		0x0009UL	/* Deprecated */
#define ANOUBIS_CHECKSUM_OP_DELSIG		0x000BUL
#define _ANOUBIS_CHECKSUM_OP_LIST_ALL		0x000CUL	/* Deprecated */
#define _ANOUBIS_CHECKSUM_OP_KEYID_LIST		0x000DUL	/* Deprecated */
#define _ANOUBIS_CHECKSUM_OP_VALIDATE		0x000EUL	/* Deprecated */
#define ANOUBIS_CHECKSUM_OP_GENERIC_LIST	0x000FUL
#define ANOUBIS_CHECKSUM_OP_GET2		0x0010UL
#define ANOUBIS_CHECKSUM_OP_GETSIG2		0x0011UL

typedef struct {
	u32n	type;
	u32n	operation;
	u32n	flags;
	u32n	uid;
	u16n	idlen;
	char	payload[0];
} __attribute__((packed)) Anoubis_ChecksumRequestMessage;

/* Types of signatures in a GET message */
#define ANOUBIS_SIG_TYPE_EOT		0	/* End of data */
#define ANOUBIS_SIG_TYPE_CS		1	/* An unsigned checksum */
#define ANOUBIS_SIG_TYPE_SIG		2	/* A signed checksum */
#define ANOUBIS_SIG_TYPE_UPGRADECS	3	/* Upgrade marker */

typedef struct {
	u32n	type;
	u32n	operation;
	u32n	flags;
	u32n	uid;
	u16n	idlen;
	u16n	cslen;
	char payload[0];	/* cslen bytes checksum followed by path. */
} __attribute__((packed)) Anoubis_ChecksumAddMessage;

typedef struct {
	u32n	type;
	u32n	pid;
} __attribute__((packed)) Anoubis_SfsDisableMessage;

#endif

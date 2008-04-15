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

DEFINE_SIGNED_TYPE(8);
DEFINE_SIGNED_TYPE(16);
DEFINE_SIGNED_TYPE(32);

#if ! defined (S_SPLINT_S) && ! defined (lint)
#define get_value(VAR)		({				\
	__typeof__(&(VAR)) __ptr = &(VAR);			\
	__typeof__(__ptr->netint) __ret;			\
	switch (sizeof(__ptr->netint)) {			\
	case 1:		__ret = (__ptr->netint); break;		\
	case 2:		__ret = ntohs(__ptr->netint); break;	\
	case 4:		__ret = ntohl(__ptr->netint); break;	\
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

#if 0
/*
 * These are currently not implemented and replaced with a more general
 * interface.
 */
#define		ANOUBIS_P_INSERT	0x3000
#define		ANOUBIS_P_INSERTOK	0x3001
#define		ANOUBIS_P_DELETE	0x3002
#define		ANOUBIS_P_NEWVERSION	0x3010
#define		ANOUBIS_P_LISTVERSIONS	0x3011
#define		ANOUBIS_P_VERSIONS	0x3012
#define		ANOUBIS_P_DELVERSION	0x3013
#define		ANOUBIS_P_GETRULES	0x3020
#define		ANOUBIS_P_RULELIST	0x3021
#endif

#define		ANOUBIS_P_CONTROL	0x3700

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
#define		ANOUBIS_N_CTXREQ	0x4020
#define		ANOUBIS_N_CTXREPLY	0x4021

#define		ANOUBIS_N_MAX		0x4FFF

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
	u32n	pid;
	u32n	rule_id;
	u32n	uid;
	u32n	subsystem;
} __attribute__((packed)) Anoubis_NotifyRegMessage;

typedef struct {
	u32n	type;
	anoubis_token_t	token;
	u32n	pid;
	u32n	rule_id;
	u32n	uid;
	u32n	subsystem;
	u32n	operation;
	/*
	 * The following two fields are only valid if the message type
	 * is ANOUBIS_N_LOGNOTIFY.
	 */
	u32n	loglevel;
	u32n	error;
	char	payload[0];
} __attribute__((packed)) Anoubis_NotifyMessage;

typedef struct {
	u32n	type;
	anoubis_token_t token;
	u32n	uid;
	u32n	error;
} __attribute__((packed)) Anoubis_NotifyResultMessage;

typedef struct {
	u32n	type;
	char	payload[0];
} __attribute__((packed)) Anoubis_PolicyRequestMessage;

typedef struct {
	u32n	type;
	u32n	error;
	char	payload[0];
} __attribute__((packed)) Anoubis_PolicyReplyMessage;

#endif

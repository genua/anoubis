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

/*
 * History of ANOUBIS_PROTO_VERSIONS:
 *     Version 1: Historic (pre 0.9.1)
 *     Version 2: Added ANOUBIS_N_STATUSNOTIFY messages (Version 0.9.[12]).
 *     Version 3: Removed SFSDISABLE messages.
 *     Version 4: Add key based authentication.
 *     Version 5: Add CSMULTI messages
 */
#define ANOUBIS_PROTO_VERSION		5
#define ANOUBIS_PROTO_MINVERSION	4

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

#define		ANOUBIS_P_CSMULTIREQUEST	0x3110
#define		ANOUBIS_P_CSMULTIREPLY		0x3111

#define		ANOUBIS_P_PASSPHRASE	0x3201

#define		ANOUBIS_P_REQUEST	0x3800
#define		ANOUBIS_P_REPLY		0x3801

#define		ANOUBIS_P_VERSION	0x3900
#define		ANOUBIS_P_VERSIONREPLY	0x3901

#define		ANOUBIS_P_PGLISTREQ	0x3A00
#define		ANOUBIS_P_PGLISTREP	0x3A01

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
#define		ANOUBIS_N_STATUSNOTIFY	0x4017

#define		ANOUBIS_N_MAX		0x4FFF

/*
 * Status notify keys. These are here to simplify exchange of status
 * notify messages. UIs should ignore status notify message if they do
 * do not understand the key.
 */

#define ANOUBIS_STATUS_UPGRADE		0x1000UL
		/* Upgrade end. Value: Number of upgraded files */


/*
 * Checksum Request Flags:
 * The flags marked as _Deprecated_ must not be used in new code.
 * Meaning of individual flags:
 *   - ANOUBIS_CSUM_UID: Apply command to user IDS/unsigend checksums
 *   - ANOUBIS_CSUM_KEY: Apply command to key IDS/signed checksums
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
 * to ANOUBIS_CSUM_UID with the current user ID.
 */

#define		ANOUBIS_CSUM_NONE	0x0000
#define		ANOUBIS_CSUM_UID	0x0001
#define		ANOUBIS_CSUM_KEY	0x0002
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
	u32n	error;
	u32n	protocol;
	u32n	apn;
} __attribute__((packed)) Anoubis_VersionMessage;

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
	u32n	loglevel;	/* Only valid for ANOUBIS_N_LOGNOTIFY */
	u32n	error;		/* Only valid for ANOUBIS_N_LOGNOTIFY */
	u16n	csumoff, csumlen;
	u16n	pathoff, pathlen;
	u16n	ctxcsumoff, ctxcsumlen;
	u16n	ctxpathoff, ctxpathlen;
	u16n	evoff, evlen;
	char	payload[0];
} __attribute__((packed)) Anoubis_NotifyMessage;

typedef struct {
	u32n	type;
	u32n	statuskey;
	u32n	statusvalue;
} __attribute__((packed)) Anoubis_StatusNotifyMessage;

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
#define ANOUBIS_CHECKSUM_OP_ADDSUM		0x0005UL
#define ANOUBIS_CHECKSUM_OP_ADDSIG		0x0008UL
#define ANOUBIS_CHECKSUM_OP_DELSIG		0x000BUL
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

/**
 * A Request to add/get/delete multiple checksums or signature.
 *
 * - type  must be set to ANOUBIS_P_CSMULTI_REQUEST
 * - operation  can be one of ADDSUM, ADDSIG, GET2, GETSIG2, DEL or DELSIG.
 * - uid  must be zero for ADDSIG, GETSIG2 and DELSIG.
 *     For ADDSUM, GET2 and DEL it specifies the user ID for all requests.
 * - idlen  must be zero for ADDSUM, GET2 and DEL.
 *     For ADDSIG, GETSIG2 and DELSIG it specifies the key ID for all requests.
 * - recoff  The offset of the first request record in the payload data.
 *     Usually the same as idlen but may be slightly smaller to align records
 *     on a suitable byte boundary.
 * - payload  contains the keyid and an arbitrary number of
 *     Anoubis_CSMultiRequestRecord structures. A request record of length
 *     zero terminates the request list.
 */
typedef struct {
	u32n	type;
	u32n	operation;
	u32n	uid;
	u16n	idlen;
	u16n	recoff;
	char	payload[0];
} __attribute__((packed)) Anoubis_CSMultiRequestMessage;

/**
 * A single checksum request within a CSMulti request.
 *
 * - length  The total length of this record.
 * - index  An index assigned by the requester to the record. This
 *     value is not interpreted by the server but it will be copied into
 *     the reply record. This allows the client to associate individual
 *     replies with requests.
 * - cslen  The length of the checksum or signature data in the request.
 *     This value is only non-zero for ADDSUM and ADDSIG requests.
 * - payload:
 *     - cslen bytes of checksum/signature data.
 *     - A NUL-Terminated path name.
 */
typedef	struct {
	u32n	length;		/* Total record length (including header). */
	u16n	index;		/* Index of the record. */
	u16n	cslen;		/* Length of the checksum in the record. */
	char	payload[0];
	/*
	 * Format of payload:
	 * - cslen bytes of checksum data.
	 * - A NUL-Terminated path  name.
	 */
} __attribute__((packed)) Anoubis_CSMultiRequestRecord;

/**
 * A reply to a CSMulti request. This message contains the answer to
 * one or more requests from the daemon. The daemon can answer a request
 * with EAGAIN. In this case the client must retry the request. This
 * can happen if the resulting message would otherwise be larger than
 * the limit imposed by the daemon.
 *
 * - type  ANOUBIS_P_CSMULTI_REPLY
 * - operation  Repeats the operation from the request.
 * - error  An error for the whole request. If this value is non-zero
 *     the request data does not contain any reply records.
 * - payload  contains an arbitrary number of Anoubis_CSMultiReplyRecords.
 *     The list is terminated by a record of length zero.
 */
typedef struct {
	u32n	type;
	u32n	operation;
	u32n	error;
	char	payload[0];
} __attribute__((packed)) Anoubis_CSMultiReplyMessage;

/**
 * A single entry in a checksum Reply Message.
 *
 * - cstype  The checksum type (ANOUBIS_SIG_TYPE_*)
 * - cslen  The length of the following checksum data.
 * - payload  The actual checksum data.
 */
struct anoubis_csentry {
	u32n		cstype;
	u32n		cslen;
	char		csdata[0];
};

/**
 * A single reply record for a checksum request. In case of a GET request,
 * it is possible that the reply record contains multiple checksum types.
 * In particular there may be an upgrade checksum and a signature * record.
 *
 * length  The total length of the record.
 * index  The index copied from the request record. This reply record
 *    answers the request in the corresponding request record. The daemon
 *    will answer replies in the order given by the client but the it
 *    is possible that not all request are answered. The client must
 *    resend those request.
 * error  An error code for this request. If the error code is non-zero
 *    the payload is empty.
 * payload  Is empty if error is non-zero or if the operation in the reply
 *    message is not a GET2 or GETSIG2 request.
 *    For successful GET2 or GETSIG2 requests the payload contains an
 *    arbitrary number of anoubis_csentry structures. A structure of type
 *    ANOUBIS_SIG_TYPE_EOT terminates the list.
 */
typedef struct {
	u16n	length;
	u16n	index;
	u32n	error;
	char	payload[0];
} __attribute__((packed)) Anoubis_CSMultiReplyRecord;

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
	char	payload[0];
} __attribute__((packed)) Anoubis_PassphraseMessage;

/* Record types for Anoubis_PgReplyMessages */
#define ANOUBIS_PGREC_NOTYPE		0	/* Error or empty. */
#define ANOUBIS_PGREC_PGLIST		1	/* Anoubis_PgInfoRecord list */
#define ANOUBIS_PGREC_FILELIST		2	/* Anoubis_PgFileRecord. */

/**
 * This structure is used to request playground status information from
 * the anoubis daemon. The result is one or more Anoubis_PgReplyMessage
 * messages. Message fields:
 *
 * type The type of the message. Must be ANOUBIS_P_PGLISTREQ.
 * listtype The type of information to list. Possible values are:
 *     - ANOUBIS_PGREC_PGLIST: List all known playgrounds. The answer will
 *         contain a list of Anoubis_PgInfoRecord structures. The pgid
 *         field is ignored for this list type.
 *     - ANOUBIS_PGREC_FILELIST: List all playground files for a paricular
 *         playground. This is only allowed for playgrounds that were
 *         created by the user or for root.
 * _pad Padding. Should, not used.
 * pgid The playground ID of the affected playground. This is only used
 *     for some list requests.
 */
typedef struct {
	u32n	type;
	u16n	listtype;
	u16n	_pad;
	u64n	pgid;
} __attribute__((packed)) Anoubis_PgRequestMessage;

/**
 * This message is sent in reply to an ANOUBIS_P_PGLISTREQ request.
 * It contains (part of) the result of the request. If the result is split
 * into multiply messages, it is still guaranteed that individual records
 * do not span accross multiple reply messages. Fields:
 *
 * type The type of the message. Must be ANOUBIS_P_PGLISTREP.
 * flags Possible flags are POLICY_FLAG_START and POLICY_FLAG_END. The
 *     start flag is set on the first message that answers a request and
 *     the end flag is set on the last message.
 * error The result of the requst (an anoubis error code). If this value is
 *     non-zero the payload must be empty and nrec must be zero.
 * nrec The number of records in this message.
 * rectype The record type used in this message. Possible values and types are:
 *     - ANOUBIS_PGREC_NOTYPE: The message contains no records. This is
 *         only allowed, if the error field is non-zero.
 *     - ANOUBIS_PGREC_PGLIST: The message contains Anoubis_PgInfoRecord
 *         records.
 *     - ANOUBIS_PGREC_FILELIST: The message contains Anobuis_PgFileRecord
 *         records. Each of these records references a single path name,
 *         multiple path names for the same dev/ino pair result in multiple
 *         records.
 */
typedef struct {
	u32n	type;
	u32n	flags;
	u32n	error;
	u16n	nrec;
	u16n	rectype;
	char	payload[0];
} __attribute__((packed)) Anoubis_PgReplyMessage;

/**
 * This structure contains information about a single playground. It is
 * used in Anoubis_PgReplyMessage replies. Fields:
 *
 * reclen The total length of this record including the length itself.
 * uid The used ID of the user that created the playground.
 * pgid The playground ID of the playground.
 * starttime The time of playground creation (seconds since the epoc).
 * nrprocs The total number of active processes in this playground.
 * nrfiles The total number of files in this playground.
 * path The command name of the command that was first run in this playground.
 */
typedef struct {
	u32n			reclen;
	u32n			uid;
	u64n			pgid;
	u64n			starttime;
	u32n			nrprocs;
	u32n			nrfiles;
	char			path[0];
} __attribute__((packed)) Anoubis_PgInfoRecord;

/**
 * This structure contains information about a single file in the playground.
 * It is used in Anobuis_PgReplyMessage replies. Fields:
 *
 * reclen The total length of this record including the length itself.
 * _pad Padding. Do not use.
 * pgid The playgroud ID of the playground that this file belongs to.
 * dev The device number (kernel representation) of the device that this
 *     file lives on.
 * ino The inode number of the file.
 * path The path name of file relative to the root of the device. The
 *     client must search for and prepend an approriate mount point.
 *     The file is reported in production system format, i.e. it includes
 *     the ".plgr.xxx." prefix.
 */
typedef struct {
	u32n			reclen;
	u32n			_pad;
	u64n			pgid;
	u64n			dev;
	u64n			ino;
	char			path[0];
} __attribute__((packed)) Anoubis_PgFileRecord;

#endif

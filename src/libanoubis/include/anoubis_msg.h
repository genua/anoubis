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
#ifndef ANOUBIS_MSG_H
#define ANOUBIS_MSG_H

#include <sys/types.h>
#include <errno.h>
#include <anoubis_protocol.h>
#include <anoubis_protocol_auth.h>
#include <stddef.h>	/* for offsetof */

struct anoubis_msg {
	u_int32_t	length;
	/*@null@*/ /*@dependent@*/
	struct anoubis_msg * next;
	/*@relnull@*/ /*@reldef@*/
	union {
		/*@relnull@*/
		Anoubis_GeneralMessage  * general;
		Anoubis_TokenMessage * token;
		Anoubis_HelloMessage * hello;
		Anoubis_VersionMessage * version;
		Anoubis_AckMessage * ack;
		Anoubis_AckPayloadMessage * ackpayload;
		Anoubis_ChecksumPayloadMessage * checksumpayload;
		Anoubis_VerselMessage * versel;
		Anoubis_AuthReplyMessage * authreply;
		Anoubis_StringListMessage * stringlist;
		Anoubis_AuthTransportMessage * authtransport;
		Anoubis_AuthChallengeMessage *authchallenge;
		Anoubis_AuthChallengeReplyMessage *authchallengereply;
		Anoubis_NotifyRegMessage * notifyreg;
		Anoubis_NotifyMessage * notify;
		Anoubis_NotifyResultMessage * notifyresult;
		Anoubis_PolicyRequestMessage * policyrequest;
		Anoubis_PolicyReplyMessage * policyreply;
		Anoubis_ChecksumRequestMessage * checksumrequest;
		Anoubis_ChecksumAddMessage * checksumadd;
		Anoubis_CSMultiRequestMessage * csmultireq;
		Anoubis_CSMultiReplyMessage * csmultireply;
		Anoubis_PolicyChangeMessage * policychange;
		Anoubis_PgChangeMessage * pgchange;
		Anoubis_StatusNotifyMessage * statusnotify;
		Anoubis_PassphraseMessage *passphrase;
		Anoubis_ListRequestMessage *listreq;
		Anoubis_ListMessage *listreply;
		Anoubis_PgCommitMessage *pgcommit;
		void * buf;
	} u;
};

#define ANOUBIS_MESSAGE_MAXLEN		100000UL /* Arbitrary limit for now. */

#if ! defined (S_SPLINT_S) && ! defined (lint)
/*
 * Interpret the message M according to the union member SEL and verify that
 * the field FIELD is entirely within the boundaries of the message.
 * Returns true if accessing the field is ok.
 */
#define VERIFY_FIELD(M, SEL, FIELD)	\
    ((M)->length >= (offsetof(typeof(*((M)->u.SEL)), FIELD)	\
			+ sizeof((M)->u.SEL->FIELD) + CSUM_LEN))

/*
 * Verify that the message M is at least LEN bytes long.
 * Returns true if the message is long enough.
 */
#define VERIFY_LENGTH(M, LEN)						\
    ((int)(LEN) >= 0 && (int)(LEN) <= (int)ANOUBIS_MESSAGE_MAXLEN	\
    && (M)->length >= ((unsigned int)(LEN) + CSUM_LEN))

/*
 * Interpret the message M according to the  union member SEL. BASE must
 * be an element of union type. The macro verifies that LEN elements
 * starting at array offset START are within the boundaries of the message.
 * Returns true if all array elements can be safely accessed.
 */
#define VERIFY_BUFFER(M, SEL, BASE, START, LEN) ({			\
	unsigned int __t_soff = (START);				\
	unsigned int __t_len = (LEN);					\
	unsigned int __t_end = offsetof(typeof(*((M)->u.SEL)),		\
	    BASE[__t_soff+__t_len]);					\
	(								\
	    __t_soff <= __t_soff + __t_len				\
	    && __t_soff + __t_len <= ANOUBIS_MESSAGE_MAXLEN		\
	    && VERIFY_LENGTH((M), __t_end)				\
	);								\
})

/*
 * Interpret the message M according to the union member SEL. FIELD
 * should be the last field of the struct and should have variable length.
 * The macro returns the total size of the variable length vield according
 * to the message length. Returns zero if FIELD is already out of bounds.
 */
#define PAYLOAD_LEN(M, SEL, FIELD) ({					\
	int	__ret = 0;						\
	int	__start = offsetof(typeof(*((M)->u.SEL)), FIELD);	\
	if (__start >= 0 && VERIFY_LENGTH((M), __start)) {		\
		__ret = (M)->length - __start - CSUM_LEN;		\
	}								\
	__ret;								\
})
#else
/*
 * Define SPLINT versions that satisfy splint but will certainly not
 * work if they are accidentally compiled into a real executable.
 */
#define VERIFY_FIELD(M, SEL, FIELD)		((M)==(void*)0x12345678)
#define VERIFY_LENGTH(M, LEN)			((M)==(void*)0x12345678)
#define VERIFY_BUFFER(M, SEL, BASE, START, END)	((M)==(void*)0x12345678)
#define PAYLOAD_LEN(M, SEL, FIELD)		0x12345
#endif


struct proto_opt {
	int val;
	const char * str;
};

struct stringlist_iterator {
	const struct anoubis_msg * m;
	char * buf;
	int len;
	int pos;
	struct proto_opt * opts;
};

__BEGIN_DECLS
struct anoubis_msg * anoubis_msg_new(size_t len);
struct anoubis_msg * anoubis_msg_clone(struct anoubis_msg * m);
int anoubis_msg_resize(struct anoubis_msg * m, size_t len);
void anoubis_msg_free(struct anoubis_msg * m);
int anoubis_msg_verify(const struct anoubis_msg *m);

void stringlist_iterator_init(struct stringlist_iterator * it,
    const struct anoubis_msg * m, struct proto_opt * opts);
int stringlist_iterator_get(struct stringlist_iterator * it);
struct anoubis_msg * anoubis_stringlist_msg(u_int32_t type,
    struct proto_opt * opts, int * ovals);

int anoubis_extract_sig_type(const struct anoubis_msg *m, int type,
    const void **data);

int anoubis_msg_send(struct achat_channel * chan, struct anoubis_msg * m);
__END_DECLS

#endif

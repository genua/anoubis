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
#ifdef DEBUG

#include <config.h>
#include <stdio.h>
#include <anoubis_msg.h>
#include <anoubis_protocol.h>
#include <crc32.h>

#define __used __attribute__((unused))

#define ASSERT(X)							\
	if (!(X)) {							\
		printf("Broken Message at %d (%s)\n", __LINE__, #X);	\
		return;							\
	}
#define CASE(X, SEL)							\
	case X:								\
		printf(" type = %s(%x)", #X, X);			\
		ASSERT(VERIFY_LENGTH(m, sizeof(*(m->u.SEL))));		\
		dump_ ## SEL (m->u.SEL, m->length - CSUM_LEN);		\
		break;

#define DUMP_NETX(PTR,FIELD)						\
	printf(" %s = 0x%x", #FIELD, get_value((PTR)->FIELD));
#define DUMP_NETU(PTR,FIELD)						\
	printf(" %s = %u", #FIELD, get_value((PTR)->FIELD));

static void DUMP_DATA(void * _buf, size_t len)
{
	int i;
	unsigned char * cbuf = _buf;
	if (!len)
		return;
	printf(" data =");
	for (i=0; i<(int)len; ++i) {
		if (i%4 == 0)
			printf(" ");
		printf("%02x", cbuf[i]);
	}
}

static void dump_general(Anoubis_GeneralMessage * m, size_t len)
{
	if (len > sizeof(*m))
		DUMP_DATA(m->payload, len - sizeof(Anoubis_GeneralMessage));
}

static void dump_hello(Anoubis_HelloMessage * m, size_t len __used)
{
	DUMP_NETX(m, version);
	DUMP_NETX(m, min_version);
}

static void dump_versel(Anoubis_VerselMessage * m, size_t len __used)
{
	DUMP_NETX(m, version);
	DUMP_NETX(m, _pad);
}

static void dump_stringlist(Anoubis_StringListMessage * m, size_t len)
{
	len -= sizeof(Anoubis_StringListMessage);
	printf(" stringlist = %.*s", (int)len, m->stringlist);
}

static void dump_ack(Anoubis_AckMessage * m, size_t len __used)
{
	DUMP_NETX(m, opcode);
	printf(" token = %llx", m->token);
	DUMP_NETX(m, error);
}

static void dump_authreply(Anoubis_AuthReplyMessage * m, size_t len)
{
	DUMP_NETX(m, error);
	DUMP_NETU(m, uid);
	len -= sizeof(Anoubis_AuthReplyMessage);
	printf(" name = %.*s", (int)len, m->name);
}

static void dump_notify(Anoubis_NotifyMessage * m, size_t len)
{
	printf(" token = %llx", m->token);
	DUMP_NETU(m, pid);
	DUMP_NETU(m, rule_id);
	DUMP_NETU(m, uid);
	DUMP_NETU(m, subsystem);
	DUMP_NETU(m, operation);
	DUMP_DATA(m->payload, len-sizeof(*m));
}

static void dump_notifyreg(Anoubis_NotifyRegMessage * m, size_t len __used)
{
	printf(" token = %llx", m->token);
	DUMP_NETU(m, pid);
	DUMP_NETU(m, rule_id);
	DUMP_NETU(m, uid);
	DUMP_NETU(m, subsystem);
}

void anoubis_dump(struct anoubis_msg * m, const char * str)
{
	int opcode;
	printf("%s:", str?str:"(null)");
	ASSERT(VERIFY_FIELD(m, general, type));
	ASSERT(crc32_check(m->u.buf, m->length));
	opcode = get_value(m->u.general->type);
	switch(opcode) {
	CASE(ANOUBIS_REPLY, ack)
	CASE(ANOUBIS_N_DELEGATE, ack)
	CASE(ANOUBIS_C_HELLO, hello)
	CASE(ANOUBIS_C_VERSEL, versel)
	CASE(ANOUBIS_C_AUTH, general)
	CASE(ANOUBIS_C_AUTHREPLY, authreply)
	CASE(ANOUBIS_C_OPTREQ, stringlist)
	CASE(ANOUBIS_C_OPTACK, stringlist)
	CASE(ANOUBIS_C_PROTOSEL, stringlist)
	CASE(ANOUBIS_C_CLOSEREQ, general)
	CASE(ANOUBIS_C_CLOSEACK, general)
	CASE(ANOUBIS_N_NOTIFY, notify)
	CASE(ANOUBIS_N_ASK, notify)
	CASE(ANOUBIS_N_REGISTER, notifyreg)
	CASE(ANOUBIS_N_UNREGISTER, notifyreg)
	default:
		printf(" type = %x", opcode);
		dump_general(m->u.general, m->length-CSUM_LEN);
	}
	printf("\n");
}

void anoubis_dump_buffer(void * buf, size_t len, const char * str)
{
	struct anoubis_msg m = {
		.u.buf = buf,
		.length = len,
	};
	anoubis_dump(&m, str);
}

#endif

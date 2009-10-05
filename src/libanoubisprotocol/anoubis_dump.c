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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <anoubis_msg.h>
#include <anoubis_protocol.h>
#include <anoubis_dump.h>
#include <ctype.h>
#include <crc32.h>

#define __used __attribute__((unused))
/* Used to avoid printf warnings on 64-bit architectures. */
#define XL(X) (unsigned long long)(X)

char *__dstr = NULL;
#define DSTR		__dstr + strlen(__dstr)
#define DLEN		BUFSIZ - strlen(__dstr) - 1

#define ASSERT(X)							\
	if (!(X)) {							\
		printf("Broken Message at %d (%s)\n", __LINE__, #X);	\
		return;							\
	}
#define CASE(X, SEL)							\
	case X:								\
		snprintf(DSTR, DLEN, " type = %s(%x)", #X, X);		\
		ASSERT(VERIFY_LENGTH(m, sizeof(*(m->u.SEL))));		\
		dump_ ## SEL (m->u.SEL, m->length - CSUM_LEN);		\
		break;

#define CASE2(X, SEL, ARG)						\
	case X:								\
		snprintf(DSTR, DLEN, " type = %s(%x)", #X, X);		\
		ASSERT(VERIFY_LENGTH(m, sizeof(*(m->u.SEL))));		\
		dump_ ## SEL (m->u.SEL, m->length - CSUM_LEN, (ARG));	\
		break;

#define DUMP_NETX(PTR,FIELD)						\
	snprintf(DSTR, DLEN, " %s = 0x%x", #FIELD, get_value((PTR)->FIELD));
#define DUMP_NETU(PTR,FIELD)						\
	snprintf(DSTR, DLEN, " %s = %u", #FIELD, get_value((PTR)->FIELD));
#define DUMP_NETULL(PTR,FIELD)						\
	snprintf(DSTR, DLEN, " %s = %llu", #FIELD, XL(get_value((PTR)->FIELD)));

static void DUMP_DATA_LABEL(const void * _buf, size_t len, const char *label)
{
	int i;
	unsigned const char * cbuf = _buf;
	if (!len)
		return;
	snprintf(DSTR, DLEN, " %s =", label);
	for (i=0; i<(int)len; ++i) {
		if (i%4 == 0)
			printf(" ");
		snprintf(DSTR, DLEN, "%02x", cbuf[i]);
	}
}
#define DUMP_DATA(BUF,LEN)	DUMP_DATA_LABEL((BUF),(LEN),"data")

static void
DUMP_CHARDATA_LABEL(const void * _buf, size_t len, const char *label)
{
	int i;
	unsigned const char *cbuf = _buf;
	if (!len)
		return;
	snprintf(DSTR, DLEN, " %s =", label);
	for (i=0; i<(int)len; ++i) {
		if (i%4 == 0)
			snprintf(DSTR, DLEN, " ");
		if (isprint(cbuf[i])) {
			snprintf(DSTR, DLEN, "%c", cbuf[i]);
		} else {
			snprintf(DSTR, DLEN, "\\x%02x", cbuf[i]);
		}
	}
}
#define DUMP_CHARDATA(BUF,LEN)	DUMP_CHARDATA_LABEL((BUF),(LEN),"data")


static void dump_checksumpayload(Anoubis_ChecksumPayloadMessage * m, size_t len)
{
	int i;
	int cnt = len - sizeof(*m);
	unsigned const char * cbuf = m->payload;
	if (!cnt)
		return;
	snprintf(DSTR, DLEN, " data =");
	for(i = 0; i<cnt; ++i) {
		if (cbuf[i] == 0)
			snprintf(DSTR, DLEN, " ");
		else
			snprintf(DSTR, DLEN, "%c", cbuf[i]);
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
	snprintf(DSTR, DLEN, " stringlist = %.*s", (int)len, m->stringlist);
}

static void dump_ack(Anoubis_AckMessage * m, size_t len __used)
{
	DUMP_NETX(m, opcode);
	snprintf(DSTR, DLEN, " token = 0x%llx", XL(m->token));
	DUMP_NETX(m, error);
}

static void dump_authreply(Anoubis_AuthReplyMessage * m, size_t len)
{
	DUMP_NETX(m, error);
	DUMP_NETU(m, uid);
	len -= sizeof(Anoubis_AuthReplyMessage);
	snprintf(DSTR, DLEN, " name = %.*s", (int)len, m->name);
}

static void dump_version(Anoubis_VersionMessage * m, size_t len __used)
{
	DUMP_NETX(m, error);
	DUMP_NETX(m, protocol);
	DUMP_NETX(m, apn);
}

static void
dump_part_hex(const char *payload, int totallen, int off, int len,
    const char *label)
{
	int trunc = 0;
	if (len == 0)
		return;
	if (len > totallen - off) {
		len = totallen - off;
		trunc = 1;
	}
	if (len < 0)
		len = 0;
	DUMP_DATA_LABEL(payload+off, len, label);
	if (trunc)
		snprintf(DSTR, DLEN, "truncated");
}

static void
dump_part_string(const char *payload, int totallen, int off, int len,
    const char *label)
{
	int trunc = 0;
	if (len == 0)
		return;
	if (len > totallen - off) {
		len = totallen - off;
		trunc = 1;
	}
	if (len < 0)
		len = 0;
	snprintf(DSTR, DLEN, " %s = %*s", label, len, payload+off);
	if (trunc)
		snprintf(DSTR, DLEN, "truncated");
}

static void dump_notify(Anoubis_NotifyMessage * m, size_t len, int arg)
{
	int payloadlen = len - sizeof(*m);

	snprintf(DSTR, DLEN, " token = 0x%llx", XL(m->token));
	DUMP_NETU(m, pid);
	DUMP_NETU(m, rule_id);
	DUMP_NETU(m, prio);
	DUMP_NETULL(m, task_cookie);
	DUMP_NETU(m, uid);
	DUMP_NETU(m, subsystem);
	DUMP_NETU(m, sfsmatch);
	DUMP_NETU(m, evoff);
	DUMP_NETU(m, evlen);
	DUMP_NETU(m, pathoff);
	DUMP_NETU(m, pathlen);
	DUMP_NETU(m, csumoff);
	DUMP_NETU(m, csumlen);
	DUMP_NETU(m, ctxpathoff);
	DUMP_NETU(m, ctxpathlen);
	DUMP_NETU(m, ctxcsumoff);
	DUMP_NETU(m, ctxcsumlen);
	if (arg) {
		DUMP_NETU(m, error);
		DUMP_NETU(m, loglevel);
	}
	dump_part_hex(m->payload, payloadlen, get_value(m->csumoff),
	    get_value(m->csumlen), "csum");
	dump_part_string(m->payload, payloadlen, get_value(m->pathoff),
	    get_value(m->pathlen), "path");
	dump_part_hex(m->payload, payloadlen, get_value(m->ctxcsumoff),
	    get_value(m->ctxcsumlen), "ctxcsum");
	dump_part_string(m->payload, payloadlen, get_value(m->ctxpathoff),
	    get_value(m->ctxpathlen), "ctxpath");
	dump_part_hex(m->payload, payloadlen, get_value(m->evoff),
	    get_value(m->evlen), "raw event");
}

static void dump_notifyreg(Anoubis_NotifyRegMessage * m, size_t len __used)
{
	snprintf(DSTR, DLEN, " token = 0x%llx", XL(m->token));
	DUMP_NETU(m, uid);
	DUMP_NETU(m, rule_id);
	DUMP_NETU(m, subsystem);
}

static void dump_notifyresult(Anoubis_NotifyResultMessage * m,
    size_t len __used)
{
	snprintf(DSTR, DLEN, " token = 0x%llx", XL(m->token));
	DUMP_NETU(m, error);
	DUMP_NETU(m, uid);
}

static void dump_policyrequest(Anoubis_PolicyRequestMessage * m, size_t len)
{
	DUMP_DATA(m->payload, len-sizeof(*m));
}

static void dump_policyreply(Anoubis_PolicyReplyMessage * m, size_t len)
{
	DUMP_CHARDATA(m->payload, len-sizeof(*m));
}

static void dump_checksumrequest(Anoubis_ChecksumRequestMessage * m,
    size_t len __used)
{
	DUMP_NETU(m, operation);
	snprintf(DSTR, DLEN, " path = %s", m->payload);
}

static void
dump_policychange(Anoubis_PolicyChangeMessage *m, size_t len __used)
{
	DUMP_NETU(m, uid);
	DUMP_NETU(m, prio);
}

void __anoubis_dump(struct anoubis_msg * m, const char * pre, char **pstr)
{
	u_int32_t opcode;

	__dstr = malloc(BUFSIZ);
	if (!__dstr)
		return;
	memset(__dstr, 0, BUFSIZ);

	snprintf(DSTR, DLEN, "%s:", pre?pre:"(null)");
	ASSERT(VERIFY_FIELD(m, general, type));
	ASSERT(m->length < INT_MAX);
	ASSERT(crc32_check(m->u.buf, (int) m->length));
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
	CASE2(ANOUBIS_N_NOTIFY, notify, 0)
	CASE2(ANOUBIS_N_ASK, notify, 0)
	CASE2(ANOUBIS_N_LOGNOTIFY, notify, 1)
	CASE(ANOUBIS_N_POLICYCHANGE, policychange)
	CASE(ANOUBIS_N_REGISTER, notifyreg)
	CASE(ANOUBIS_N_UNREGISTER, notifyreg)
	CASE(ANOUBIS_N_RESYOU, notifyresult)
	CASE(ANOUBIS_N_RESOTHER, notifyresult)
	CASE(ANOUBIS_P_REQUEST, policyrequest)
	CASE(ANOUBIS_P_REPLY, policyreply)
	CASE(ANOUBIS_P_CSUMREQUEST, checksumrequest)
	CASE(ANOUBIS_P_CSUM_LIST, checksumpayload)
	CASE(ANOUBIS_P_VERSION, general)
	CASE(ANOUBIS_P_VERSIONREPLY, version)
	default:
		snprintf(DSTR, DLEN, " type = %x", opcode);
		dump_general(m->u.general, m->length-CSUM_LEN);
	}

	if (pstr) {
		*pstr = __dstr;
	} else {
		printf("%s\n", __dstr);
		free(__dstr);
	}
}

void __anoubis_dump_buf(void * buf, size_t len, const char * pre)
{
#ifndef lint
	struct anoubis_msg m = {
		.u.buf = buf,
		.length = len,
	};
#else
	struct anoubis_msg m;
#endif
	__anoubis_dump(&m, pre, NULL);
}

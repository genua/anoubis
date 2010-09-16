/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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
#include <anoubis_protocol_auth.h>
#include <ctype.h>
#include <anoubis_crc32.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>


#define CASE(CONST,FUNC)				\
	case CONST : return verify_ ## FUNC(m)

#define DEFINE_CHECK_FUNCTION(FUNC, TYPE)		\
	static int					\
	verify_ ## FUNC(const struct anoubis_msg *m)	\
	{						\
		return VERIFY_LENGTH(m, sizeof(TYPE));	\
	}

#define DEFINE_STRICT_CHECK_FUNCTION(FUNC, TYPE)		\
	static int						\
	verify_ ## FUNC(const struct anoubis_msg *m)		\
	{							\
		return VERIFY_LENGTH(m, sizeof(TYPE))		\
		    && !VERIFY_LENGTH(m, 8 + sizeof(TYPE));	\
	}

static int
buffer_strlen(const char *buf, int buflen)
{
	int	i;
	for (i=0; i<buflen; ++i)
		if (buf[i] == 0)
			return i;
	return buflen;
}

DEFINE_CHECK_FUNCTION(reply, Anoubis_AckPayloadMessage)
DEFINE_STRICT_CHECK_FUNCTION(general, Anoubis_GeneralMessage)
DEFINE_STRICT_CHECK_FUNCTION(versel, Anoubis_VerselMessage)
DEFINE_STRICT_CHECK_FUNCTION(authreply, Anoubis_AuthReplyMessage)

/*
 * NOTE CEH: No other verification possible for stringlists.
 * NOTE CEH: They are not even NUL-Termminated. However, they are only
 * NOTE CEH: accessed via a stringlist iterator, so that should be safe.
 */
static int
verify_stringlist(const struct anoubis_msg *m)
{
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_StringListMessage)))
		return 0;
	return 1;
}

static int
verify_challengereply(const struct anoubis_msg *m)
{
	int	ivlen, siglen;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_AuthChallengeReplyMessage)))
		return 0;
	ivlen = get_value(m->u.authchallengereply->ivlen);
	siglen = get_value(m->u.authchallengereply->siglen);
	if (!VERIFY_BUFFER(m, authchallengereply, payload, 0, ivlen))
		return 0;
	if (!VERIFY_BUFFER(m, authchallengereply, payload, ivlen, siglen))
		return 0;
	if (PAYLOAD_LEN(m, authchallengereply, payload) > ivlen + siglen + 8)
		return 0;
	return 1;
}

static int
verify_challenge(const struct anoubis_msg *m)
{
	int	idlen, challengelen;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_AuthChallengeMessage)))
		return 0;
	challengelen = get_value(m->u.authchallenge->challengelen);
	idlen = get_value(m->u.authchallenge->idlen);
	if (!VERIFY_BUFFER(m, authchallenge, payload, 0, challengelen))
		return 0;
	if (!VERIFY_BUFFER(m, authchallenge, payload, challengelen, idlen))
		return 0;
	if (PAYLOAD_LEN(m, authchallenge, payload) > challengelen + idlen + 8)
		return 0;
	return 1;
}

static int
verify_authdata(const struct anoubis_msg *m)
{
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_AuthTransportMessage)))
		return 0;
	switch(get_value(m->u.authtransport->auth_type)) {
	case ANOUBIS_AUTH_TRANSPORT:
	case ANOUBIS_AUTH_TRANSPORTANDKEY:
		return 1;
	CASE(ANOUBIS_AUTH_CHALLENGE, challenge);
	CASE(ANOUBIS_AUTH_CHALLENGEREPLY, challengereply);
	default:
		return 0;
	}
	return 0;
}

static int
verify_csumaddrequest(const struct anoubis_msg *m)
{
	int	idlen, cslen, plen, slen;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_ChecksumAddMessage)))
		return 0;
	idlen = get_value(m->u.checksumadd->idlen);
	cslen = get_value(m->u.checksumadd->cslen);
	if (cslen == 0)
		return 0;
	if (!VERIFY_BUFFER(m, checksumadd, payload, 0, idlen))
		return 0;
	if (!VERIFY_BUFFER(m, checksumadd, payload, idlen, cslen))
		return 0;
	plen = PAYLOAD_LEN(m, checksumadd, payload);
	if (idlen + cslen >= plen)
		return 0;
	plen -= idlen + cslen;
	slen = buffer_strlen(m->u.checksumadd->payload + idlen + cslen,
	    plen);
	return slen < plen && slen + 8 > plen;
}

static int
verify_csumrequest(const struct anoubis_msg *m)
{
	int	idlen, plen, slen;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_ChecksumRequestMessage)))
		return 0;
	switch(get_value(m->u.checksumrequest->operation)) {
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
		return verify_csumaddrequest(m);
	default:
		break;
	}
	idlen = get_value(m->u.checksumrequest->idlen);
	if (!VERIFY_BUFFER(m, checksumrequest, payload, 0, idlen))
		return 0;
	plen = PAYLOAD_LEN(m, checksumrequest, payload);
	if (idlen >= plen)
		return 0;
	plen -= idlen;
	slen = buffer_strlen(m->u.checksumrequest->payload + idlen, plen);
	return slen < plen && slen + 8 > plen;
}

DEFINE_CHECK_FUNCTION(csumlist, Anoubis_ChecksumPayloadMessage)

static int
verify_csmultireq(const struct anoubis_msg *m)
{
	int				 add = 0;
	unsigned int			 idlen;
	unsigned int			 off = 0;
	Anoubis_CSMultiRequestRecord	*r;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_CSMultiRequestMessage)))
		return 0;
	switch(get_value(m->u.csmultireq->operation)) {
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
		add = 1;
		break;
	case ANOUBIS_CHECKSUM_OP_GET2:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
	case ANOUBIS_CHECKSUM_OP_DEL:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
		break;
	default:
		return 0;
	}
	idlen = get_value(m->u.csmultireq->idlen);
	off = get_value(m->u.csmultireq->recoff);
	if (off < idlen || off > idlen + 8)
		return 0;
	if (!VERIFY_BUFFER(m, csmultireq, payload, 0, off))
		return 0;
	while (1) {
		unsigned int			 reclen;
		unsigned int			 cslen;
		unsigned int			 reallen;
		char				*path;

		if (!VERIFY_BUFFER(m, csmultireq, payload, off,
		    sizeof(Anoubis_CSMultiRequestRecord)))
			return 0;
		r = (void *)(m->u.csmultireq->payload + off);
		reclen = get_value(r->length);
		if (reclen == 0)
			break;
		cslen = get_value(r->cslen);
		if (reclen < sizeof(Anoubis_CSMultiRequestRecord)
		    || !VERIFY_BUFFER(m, csmultireq, payload, off, reclen))
			return 0;
		off += reclen;
		if (cslen > reclen)
			return 0;
		if (add) {
			reallen = sizeof(Anoubis_CSMultiRequestRecord) + cslen;
			path = r->payload + cslen;
		} else {
			if (cslen)
				return 0;
			reallen = sizeof(Anoubis_CSMultiRequestRecord);
			path = r->payload;
		}
		if (reallen >= reclen)
			return 0;
		while (reallen < reclen) {
			if ((*path) == 0) {
				reallen++;
				break;
			}
			reallen++;
			path++;
		}
		if (reallen > reclen || reallen + 8 < reclen)
			return 0;
	}
	if (get_value(r->length) || get_value(r->cslen))
		return 0;
	off += sizeof(Anoubis_CSMultiRequestRecord);
	if (off + 8 < (unsigned int)PAYLOAD_LEN(m, csmultireq, payload))
		return 0;
	return 1;
}

static int
verify_csmultireply(const struct anoubis_msg *m)
{
	int				 get = 0;
	Anoubis_CSMultiReplyRecord	*r;
	unsigned int			 plen;
	unsigned int			 off = 0;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_CSMultiReplyMessage)))
		return 0;
	plen = PAYLOAD_LEN(m, csmultireply, payload);

	switch(get_value(m->u.csmultireply->operation)) {
	case ANOUBIS_CHECKSUM_OP_GET2:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
		get = 1;
		break;
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
	case ANOUBIS_CHECKSUM_OP_DEL:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
		break;
	default:
		return 0;
	}
	if (get_value(m->u.csmultireply->error))
		return plen < 8;
	while (1) {
		unsigned int		 reclen;
		unsigned int		 off2;
		struct anoubis_csentry	*e;

		if (!VERIFY_BUFFER(m, csmultireply, payload, off,
		    sizeof(Anoubis_CSMultiReplyRecord)))
			return 0;
		r = (void *)(m->u.csmultireply->payload + off);
		reclen = get_value(r->length);
		if (reclen == 0)
			break;
		if (reclen < sizeof(Anoubis_CSMultiReplyRecord)
		    || !VERIFY_BUFFER(m, csmultireply, payload, off, reclen))
			return 0;
		off += reclen;
		/* Only successful get requests can have a payload. */
		if (!get || get_value(r->error)) {
			if (reclen > sizeof(Anoubis_CSMultiReplyRecord) + 8)
				return 0;
			continue;
		}
		/* The rest of the loop only deals with get requests. */
		off2 = 0;
		reclen -= sizeof(Anoubis_CSMultiReplyRecord);
		while (1) {
			unsigned int		cslen;

			if (reclen < sizeof(struct anoubis_csentry))
				return 0;
			e = (struct anoubis_csentry *)(r->payload + off2);
			reclen -= sizeof(struct anoubis_csentry);
			off2 += sizeof(struct anoubis_csentry);
			cslen = get_value(e->cslen);
			if (cslen > reclen)
				return 0;
			off2 += cslen;
			reclen -= cslen;
			if (get_value(e->cstype) == ANOUBIS_SIG_TYPE_EOT) {
				if (cslen)
					return 0;
				break;
			}
			switch(get_value(e->cstype)) {
			case ANOUBIS_SIG_TYPE_CS:
			case ANOUBIS_SIG_TYPE_SIG:
			case ANOUBIS_SIG_TYPE_UPGRADECS:
				break;
			default:
				return 0;
			}
		}
		if (reclen > 8)
			return 0;
	}
	/* Sentinel */
	off += sizeof(Anoubis_CSMultiReplyRecord);
	if (get_value(r->error) || get_value(r->index))
		return 0;
	if (off + 8 < (unsigned int)PAYLOAD_LEN(m, csmultireq, payload))
		return 0;
	return 1;
}

static int
verify_passphrase(const struct anoubis_msg *m)
{
	int	len, plen;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_PassphraseMessage)))
		return 0;
	plen = PAYLOAD_LEN(m, passphrase, payload);
	len = buffer_strlen(m->u.passphrase->payload, plen);
	return (len<plen) && (len+8 > plen);
}

DEFINE_CHECK_FUNCTION(polrequest, Anoubis_PolicyRequestMessage)
DEFINE_CHECK_FUNCTION(polreply, Anoubis_PolicyReplyMessage)
DEFINE_STRICT_CHECK_FUNCTION(version, Anoubis_VersionMessage)
DEFINE_STRICT_CHECK_FUNCTION(register, Anoubis_NotifyRegMessage)

/*
 * Interpret the message M as a notify message and verify that the
 * buffer given by the fields named OFF and LEN is within the
 * boundaries of the message. Return zero if the buffer is out of bounds.
 */
#define VERIFY_NOTIFYBUFFER(M, OFF, LEN)			\
    (VERIFY_BUFFER((M), notify, payload,			\
    get_value((M)->u.notify->OFF),				\
    get_value((M)->u.notify->LEN)))

static inline int
verify_notify_string(const void *buffer, int off, int len)
{
	int	slen;
	if (len == 0)
		return 1;
	slen = buffer_strlen(buffer+off, len);
	return slen < len;
}

/*
 * Interpret M as a notify message and verify that the buffer of length
 * LEN starting at offset OFF into the payload is NUL terminated. Also
 * verifies that the buffer is within the boundaries of the message.
 */
#define VERIFY_NOTIFYSTRING(M, OFF, LEN)			\
    (VERIFY_NOTIFYBUFFER((M), OFF, LEN)				\
    && verify_notify_string(m->u.notify->payload,		\
    get_value(m->u.notify->OFF),				\
    get_value(m->u.notify->LEN)))


static int
verify_notify(const struct anoubis_msg *m)
{
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_NotifyMessage)))
		return 0;
	if (!VERIFY_NOTIFYBUFFER(m, csumoff, csumlen))
		return 0;
	if (!VERIFY_NOTIFYSTRING(m, pathoff, pathlen))
		return 0;
	if (!VERIFY_NOTIFYBUFFER(m, ctxcsumoff, ctxcsumlen))
		return 0;
	if (!VERIFY_NOTIFYSTRING(m, ctxpathoff, ctxpathlen))
		return 0;
	if (!VERIFY_NOTIFYBUFFER(m, evoff, evlen))
		return 0;
	return 1;
}

static int
verify_pgrequest(const struct anoubis_msg *m)
{
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_ListRequestMessage)))
		return 0;
	return 1;
}

static int
verify_pgcommit(const struct anoubis_msg *m)
{
	int	i, plen;
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_PgCommitMessage)))
		return 0;
	plen = PAYLOAD_LEN(m, pgcommit, payload);
	for (i=0; i<plen; ++i)
		if (m->u.pgcommit->payload[i] == 0)
			return 1;
	return 0;
}

static int
verify_pginfo_record(Anoubis_PgInfoRecord *r, int reclen)
{
	int		i;

	reclen -= sizeof(Anoubis_PgInfoRecord);
	/* Make sure that the path name is NUL terminated. */
	for (i=0; i<reclen; ++i)
		if (r->path[i] == 0)
			break;
	if (i==reclen)
		return 0;
	return 1;
}

static int
verify_pgfile_record(Anoubis_PgFileRecord *r, int reclen)
{
	int		i;
	reclen -= sizeof(Anoubis_PgFileRecord);
	/* Make sure that the path name is NUL terminated. */
	for (i=0; i<reclen; ++i)
		if (r->path[i] == 0)
			break;
	if (i==reclen)
		return 0;
	return 1;
}

static int
verify_proc_record(Anoubis_ProcRecord *r __attribute__((unused)),
    int reclen __attribute__((unused)))
{
	/* XXX */
	return 1;
}

static int
verify_listreply(const struct anoubis_msg *m)
{
	int	off = 0;
	int	nrec, rectype;
	if (!VERIFY_LENGTH(m, sizeof(Anoubis_ListMessage)))
		return 0;
	if (get_value(m->u.listreply->error))
		return 1;
	nrec = get_value(m->u.listreply->nrec);
	rectype = get_value(m->u.listreply->rectype);
	while (nrec--) {
		uint32_t	reclen;

		if (rectype == ANOUBIS_REC_PGLIST) {
			Anoubis_PgInfoRecord	*r;
			if (!VERIFY_BUFFER(m, listreply, payload, off,
			    sizeof(Anoubis_PgInfoRecord)))
				return 0;
			r = (Anoubis_PgInfoRecord *)
			    (m->u.listreply->payload+off);
			reclen = get_value(r->reclen);
			if (reclen < sizeof(Anoubis_PgInfoRecord))
				return 0;
			if (!VERIFY_BUFFER(m, listreply, payload, off, reclen))
				return 0;
			if (!verify_pginfo_record(r, reclen))
				return 0;
		} else if (rectype == ANOUBIS_REC_PGFILELIST) {
			Anoubis_PgFileRecord	*r;
			if (!VERIFY_BUFFER(m, listreply, payload, off,
			    sizeof(Anoubis_PgFileRecord)))
				return 0;
			r = (Anoubis_PgFileRecord *)
			    (m->u.listreply->payload+off);
			reclen = get_value(r->reclen);
			if (reclen < sizeof(Anoubis_PgFileRecord))
				return 0;
			if (!VERIFY_BUFFER(m, listreply, payload, off, reclen))
				return 0;
			if (!verify_pgfile_record(r, reclen))
				return 0;
		} else if (rectype == ANOUBIS_REC_PROCLIST) {
			Anoubis_ProcRecord	*r;
			if (!VERIFY_BUFFER(m, listreply, payload, off,
			    sizeof(Anoubis_ProcRecord)))
				return 0;
			r = (Anoubis_ProcRecord *)(m->u.listreply->payload+off);
			reclen = get_value(r->reclen);
			if (reclen < sizeof(Anoubis_ProcRecord))
				return 0;
			if (!VERIFY_BUFFER(m, listreply, payload, off, reclen))
				return 0;
			if (!verify_proc_record(r, reclen))
				return 0;
		} else {
			return 0;
		}
		off += reclen;
	}
	return 1;
}

DEFINE_STRICT_CHECK_FUNCTION(notifyresult, Anoubis_NotifyResultMessage)
DEFINE_STRICT_CHECK_FUNCTION(policychange, Anoubis_PolicyChangeMessage)
DEFINE_STRICT_CHECK_FUNCTION(statusnotify, Anoubis_StatusNotifyMessage)

int
anoubis_msg_verify(const struct anoubis_msg *m)
{
	if (m->length > ANOUBIS_MESSAGE_MAXLEN)
		return 0;
	if (!VERIFY_FIELD(m, general, type))
		return 0;
	switch(get_value(m->u.general->type)) {
	CASE(ANOUBIS_REPLY, reply);
	CASE(ANOUBIS_C_HELLO, general);
	CASE(ANOUBIS_C_VERSEL, versel);
	CASE(ANOUBIS_C_AUTH, general);
	CASE(ANOUBIS_C_AUTHDATA, authdata);
	CASE(ANOUBIS_C_AUTHREPLY, authreply);
	CASE(ANOUBIS_C_OPTREQ, stringlist);
	CASE(ANOUBIS_C_OPTACK, stringlist);
	CASE(ANOUBIS_C_PROTOSEL, stringlist);
	CASE(ANOUBIS_C_CLOSEREQ, general);
	CASE(ANOUBIS_C_CLOSEACK, general);
	CASE(ANOUBIS_P_CSUMREQUEST, csumrequest);
	CASE(ANOUBIS_P_CSUM_LIST, csumlist);
	CASE(ANOUBIS_P_CSMULTIREQUEST, csmultireq);
	CASE(ANOUBIS_P_CSMULTIREPLY, csmultireply);
	CASE(ANOUBIS_P_PASSPHRASE, passphrase);
	CASE(ANOUBIS_P_REQUEST, polrequest);
	CASE(ANOUBIS_P_REPLY, polreply);
	CASE(ANOUBIS_P_VERSION, general);
	CASE(ANOUBIS_P_VERSIONREPLY, version);
	CASE(ANOUBIS_P_LISTREQ, pgrequest);
	CASE(ANOUBIS_P_LISTREP, listreply);
	CASE(ANOUBIS_P_PGCOMMIT, pgcommit);
	CASE(ANOUBIS_N_REGISTER, register);
	CASE(ANOUBIS_N_UNREGISTER, register);
	CASE(ANOUBIS_N_ASK, notify);
	CASE(ANOUBIS_N_DELEGATE, reply);
	CASE(ANOUBIS_N_RESYOU, notifyresult);
	CASE(ANOUBIS_N_RESOTHER, notifyresult);
	CASE(ANOUBIS_N_NOTIFY, notify);
	CASE(ANOUBIS_N_LOGNOTIFY, notify);
	CASE(ANOUBIS_N_POLICYCHANGE, policychange);
	CASE(ANOUBIS_N_STATUSNOTIFY, statusnotify);

	default:
		return 0;
	}
	return 0;
}

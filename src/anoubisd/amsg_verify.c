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

#include "config.h"

#include <stdio.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>
#ifdef LINUX
#include <bsdcompat.h>
#include <openssl/sha.h>
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis_sfs.h>
#endif
#ifdef OPENBSD
#include <sha2.h>
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#include <dev/anoubis_alf.h>
#include <dev/anoubis_sfs.h>
#endif

#include <anoubis_protocol.h>

#include "anoubisd.h"
#include "amsg.h"
#include "aqueue.h"

#define SIZE_LIMIT	50000

/*
 * Declare a variable that tracks the size of the current data structure.
 * Use ADD_SIZE to increase the size manually and RETURN_SIZE to return
 * its value.
 */
#define DECLARE_SIZE()	int __local_size = 0;

/*
 * Check that (V) is a valid value that can be used as a size.
 * Returns -1 from the surrounding function if this is not the case.
 * Otherwise it is a NOOP.
 */
#define CHECK_SIZE(V)						\
	do {							\
		int ___tmp = (V);				\
		if (___tmp < 0 || ___tmp > SIZE_LIMIT)		\
			return -1;				\
	} while (0)

/*
 * Check that both (V) and (L) are valid sizes using CHECK_SIZE and
 * verify that a structure of length (V) can be contained in a buffer
 * of length (L)
 */
#define CHECK_LEN(V,L)						\
	do {							\
		CHECK_SIZE(V);					\
		CHECK_SIZE(L);					\
		if ((int)(V) > (int)(L))			\
			return -1;				\
	} while (0)

/*
 * Return the cumulated real size of a the structure.
 */
#define	RETURN_SIZE()						\
	do {							\
		CHECK_SIZE(__local_size);			\
		return __local_size;				\
	} while (0)

/*
 * Add (V) bytes to the cumulated real size of the structure.
 * Both (V) and the new value are checked using CHECK_SIZE.
 * Returns -1 from the surrounding function if the sanity checks fail.
 */
#define ADD_SIZE(V)						\
	do {							\
		CHECK_SIZE((V));				\
		__local_size += (V);				\
		CHECK_SIZE(__local_size);			\
	} while (0)

/*
 * Cast the buffer pointed to by B of length L to a message of
 * type typeof(V). This does not modify the cumulated size of the
 * structure.
 * Return -1 from the surrounding function if length check fails.
 */
#define CAST(V,B,L)					\
	do {						\
		if ((int)sizeof(*V) > (L))		\
			return -1;			\
		(V) = (typeof(V)) (B);			\
	} while (0);

/*
 * In a message (M) assume that FIELD is the last field of the
 * structure and can have variable length. The message (M) must be
 * stored at the start of the buffer defined by (B) and (L).
 * This modifies the buffer such that (B) and (L) now point at the
 * variable length argument. The cumulated size of the message is
 * increase by the offset of FIELD into the structure (M).
 * Returns -1 from the surrounding function if any length checks fail
 * or if the offset of the variable length argument does not fit
 * into the buffer.
 */
#define SHIFT_FIELD(M,FIELD,B,L)				\
	do {							\
		int __shift;					\
		if ((L) <= 0 || ((void*)(M) != (void*)(B)))	\
			return -1;				\
		__shift = offsetof(typeof(*(M)),FIELD);		\
		if ((L) < __shift)				\
			return -1;				\
		(B) = (void*)&((M)->FIELD);			\
		(L) -= __shift;					\
		ADD_SIZE(__shift);				\
	} while (0)

/*
 * This works like SHIFT_FIELD except that the offset is given
 * explicitly by (CNT).
 */
#define SHIFT_CNT(CNT,B,L)					\
	do {							\
		int __shift = (CNT);				\
		if (__shift < 0 || (L) < __shift)		\
			return -1;				\
		(B) = ((void*)(B)) + __shift;			\
		(L) -= __shift;					\
		ADD_SIZE(__shift);				\
	} while (0)

/*
 * Similar to SHIFT_CNT, but takes the bytes from the end without
 * changing the buffer itself. The cumulated size of the structure
 * is increased by (CNT).
 * Returns -1 from the surrounding function if any length checks fail
 * or if the buffer size is smaller than (CNT).
 */
#define POP_CNT(CNT, L)						\
	do {							\
		int __pop = (CNT);				\
		if (__pop < 0 || (L) < __pop)			\
			return -1;				\
		(L) -= __pop;					\
		ADD_SIZE(__pop);				\
	} while (0)

/*
 * Verify that the buffer defined by (B) and (L) contains one or
 * more NUL terminated C-Strings. Does not modify the cumulated
 * size of the message.
 * Returns -1 if no NUL byte is found in the buffer or if the buffer
 * length is out of range.
 */
#define CHECK_STRING(B,L)					\
	do {							\
		int __i;					\
		if ((L) < 1 || (L) >= SIZE_LIMIT)		\
			return -1;				\
		for (__i = (L)-1; __i >= 0; __i--)		\
			if (((char*)(B))[__i] == 0)		\
				break;				\
		if (__i < 0)					\
			return -1;				\
	} while (0)

/*
 * This works like CHECK_STRING except that the cumulated size
 * of the message is increased by the real length of the string.
 * Buffer is not mofied in any case.
 */
#define STRINGS(B,L)						\
	do {							\
		int __i;					\
		if ((L) < 1 || (L) >= SIZE_LIMIT)		\
			return -1;				\
		for (__i = (L)-1; __i >= 0; __i--)		\
			if (((char*)(B))[__i] == 0)		\
				break;				\
		if (__i < 0)					\
			return -1;				\
		ADD_SIZE(__i+1);				\
	} while (0)

/*
 * Find the first NUL byte in the buffer and shift the buffer
 * to the place after that NUL byte. This increases the message
 * size by the size of the string.
 * Returns -1 from the surrounding function if no NUL byte is
 * found.
 */
#define SHIFT_STRING(B,L)					\
	do {							\
		int __i;					\
		if ((int)(L) < 1 || (L) >= SIZE_LIMIT)		\
			return -1;				\
		for (__i = 0; __i<(int)(L); ++__i)		\
			if (((char*)(B))[__i] == 0)		\
				break;				\
		if (__i >= (L))					\
			return -1;				\
		__i++;						\
		(L) -= __i;					\
		(B) = ((void*)(B)) + __i;			\
		ADD_SIZE(__i);					\
	} while (0)

/*
 * This function defines a single case in a switch statement that
 * destinguishes messages based on an embedded type.
 * (CONST) must be a constant value for the message type. The message
 * of that type must be contained in the buffer given by (B) and (L).
 * The type name of the message should be given in TYPE and a function
 * <TYPE>_size must be defined that calculates the size of the message.
 * The length of the message is added to the cumulated size of the message.
 * If any values are out of range -1 is returned from the surrounding function.
 */
#define VARIANT(CONST,TYPE, B, L)				\
	case CONST: {						\
		int	__tmp;					\
		__tmp = TYPE ## _size((B), (L));		\
		ADD_SIZE(__tmp);				\
		break;						\
	}

/*
 * Define a function that verifies the length of the buffer against
 * the data type (TYPE).
 */
#define DEFINE_CHECK_FUNCTION(STRUCT, TYPE)				\
static int								\
TYPE ## _size(const char *buf __attribute__((unused)), int buflen)	\
{									\
	if (buflen < (int)sizeof(STRUCT TYPE))				\
		return -1;						\
	return sizeof(STRUCT TYPE);					\
}

/*
 * Size of an sfs_checksumop payload.
 */
int
amsg_sfs_checksumop_size(const char *buf, int buflen)
{
	Anoubis_ChecksumRequestMessage	*msg;
	Anoubis_ChecksumAddMessage	*addmsg;
	int				 idlen = 0, cslen = 0;
	int				 op;
	DECLARE_SIZE();

	/* Space for the anoubis protocol Checksum message at the end. */
	POP_CNT(CSUM_LEN, buflen);
	CAST(msg, buf, buflen);
	op = get_value(msg->operation);
	if (op == ANOUBIS_CHECKSUM_OP_ADDSUM
	    || op == ANOUBIS_CHECKSUM_OP_ADDSIG) {
		CAST(addmsg, buf, buflen);
		cslen = get_value(addmsg->cslen);
		if (cslen < SHA256_DIGEST_LENGTH)
			return -1;
		SHIFT_FIELD(addmsg, payload, buf, buflen);
	} else {
		SHIFT_FIELD(msg, payload, buf, buflen);
	}
	idlen = get_value(msg->idlen);
	SHIFT_CNT(idlen, buf, buflen);
	SHIFT_CNT(cslen, buf, buflen);
	SHIFT_STRING(buf, buflen);

	RETURN_SIZE();
}

/*
 * Size of an anobuisd_msg_csumreply structure.
 */
static int
anoubisd_msg_csumreply_size(const char *buf, int buflen)
{
	struct anoubisd_msg_csumreply	*msg;
	DECLARE_SIZE();

	CAST(msg, buf, buflen);
	SHIFT_FIELD(msg, data, buf, buflen);
	CHECK_LEN(msg->len, buflen);
	ADD_SIZE(msg->len);

	RETURN_SIZE();
}

DEFINE_CHECK_FUNCTION(struct, alf_event)
DEFINE_CHECK_FUNCTION(struct, ac_process_message)
DEFINE_CHECK_FUNCTION(struct, ac_ipc_message)

/*
 * Size of an sfs_open_message.
 */
static int
sfs_open_message_size(const char *buf, int buflen)
{
	struct sfs_open_message	*sfs;
	DECLARE_SIZE();

	CAST(sfs, buf, buflen);
	SHIFT_FIELD(sfs, pathhint, buf, buflen);
	SHIFT_STRING(buf, buflen);

	RETURN_SIZE();
}

#ifdef ANOUBIS_SOURCE_SFSPATH
/*
 * Size of an sfs_path_message structure.
 */
static int
sfs_path_message_size(const char *buf, int buflen)
{
	struct sfs_path_message		*msg;
	DECLARE_SIZE();

	CAST(msg, buf, buflen);
	SHIFT_FIELD(msg, paths, buf, buflen);
	if (msg->pathlen[0])
		SHIFT_STRING(buf, buflen);
	if (msg->pathlen[1])
		SHIFT_STRING(buf, buflen);
	RETURN_SIZE();
}
#endif

/*
 * Size of an anoubis_stat_message.
 * NOTE: We return the buffer size here because the actual length is
 *       only determined by the surrounding buffer. We do verify that
 *       at least the header fits into the buffer.
 */
static int
anoubis_stat_message_size(const char *buf, int buflen)
{
	struct anoubis_stat_message	*msg;
	CAST(msg, buf, buflen);
	/*
	 * Length of the stat message is determined by the
	 * size of the surrounding buffer.
	 */
	return buflen;
}

/*
 * Return -1 if the policy reuqest payload data is inconsistent, 0 otherwise.
 */
int
verify_polrequest(const char *buf, int buflen, unsigned int flags)
{
	Policy_Generic		*gen;
	Policy_GetByUid		*get;
	Policy_SetByUid		*set;

	/* If the start flag is not set, all of the data is payload. */
	if ((flags & POLICY_FLAG_START) == 0)
		return 0;
	CAST(gen, buf, buflen);
	switch(get_value(gen->ptype)) {
	case ANOUBIS_PTYPE_GETBYUID:
		CAST(get, buf, buflen);
		break;
	case ANOUBIS_PTYPE_SETBYUID:
		CAST(set, buf, buflen);
		/* The rest of the buffer is payload. */
		break;
	default:
		return -1;
	}
	return 0;
}

/*
 * Return the size of an anoubisd_msg_polrequest message.
 */
static int
anoubisd_msg_polrequest_size(const char *buf, int buflen)
{
	DECLARE_SIZE();
	struct anoubisd_msg_polrequest	*req;

	CAST(req, buf, buflen);
	SHIFT_FIELD(req, data, buf, buflen);
	CHECK_LEN(req->len, buflen);
	ADD_SIZE(req->len);

	if (verify_polrequest(buf, req->len, req->flags) < 0)
		return -1;

	RETURN_SIZE();
}

static int
anoubisd_msg_polreply_size(const char *buf, int buflen)
{
	DECLARE_SIZE();
	struct anoubisd_msg_polreply	*reply;

	CAST(reply, buf, buflen);
	SHIFT_FIELD(reply, data, buf, buflen);
	CHECK_LEN(reply->len, buflen);
	ADD_SIZE(reply->len);

	RETURN_SIZE();
}

/*
 * Size of an eventdev_hdr.
 */
int
eventdev_hdr_size(const char *buf, int buflen)
{
	struct eventdev_hdr	*hdr;
	DECLARE_SIZE();

	CAST(hdr, buf, buflen);
	CHECK_LEN(hdr->msg_size, buflen);
	CHECK_SIZE(hdr->msg_size);
	SHIFT_CNT(sizeof(*hdr), buf, buflen);
	switch(hdr->msg_source) {
	VARIANT(ANOUBIS_SOURCE_ALF, alf_event, buf, buflen);
	VARIANT(ANOUBIS_SOURCE_SANDBOX, sfs_open_message, buf, buflen);
	VARIANT(ANOUBIS_SOURCE_SFS, sfs_open_message, buf, buflen);
	VARIANT(ANOUBIS_SOURCE_SFSEXEC, sfs_open_message, buf, buflen);
#ifdef ANOUBIS_SOURCE_SFSPATH
	VARIANT(ANOUBIS_SOURCE_SFSPATH, sfs_path_message, buf, buflen);
#endif
	VARIANT(ANOUBIS_SOURCE_PROCESS, ac_process_message, buf, buflen);
	VARIANT(ANOUBIS_SOURCE_STAT, anoubis_stat_message, buf, buflen);
	VARIANT(ANOUBIS_SOURCE_IPC, ac_ipc_message, buf, buflen);
	default:
		return -1;
	}
	if (__local_size > hdr->msg_size)
		return -1;
	if (__local_size + 8 < hdr->msg_size)
		return -1;
	/*
	 * NOTE: Do NOT return the calculated size here because this
	 * NOTE: is usually used within a container and that container
	 * NOTE: must treat the padding space as part of the message.
	 */
	return hdr->msg_size;
}

/*
 * Size of a struct anoubisd_msg_logrequest
 */
static int
anoubisd_msg_logrequest_size(const char *buf, int buflen)
{
	struct anoubisd_msg_logrequest	*msg;
	DECLARE_SIZE();
	int	evsize;

	CAST(msg, buf, buflen);
	SHIFT_FIELD(msg, hdr, buf, buflen);
	evsize = eventdev_hdr_size(buf, buflen);
	ADD_SIZE(evsize);

	RETURN_SIZE();
}

DEFINE_CHECK_FUNCTION(struct, eventdev_reply)
DEFINE_CHECK_FUNCTION(,eventdev_token)

static int
anoubisd_msg_csumop_size(const char *buf, int buflen)
{
	struct anoubisd_msg_csumop		*msg;
	int					 sfsoplen;
	DECLARE_SIZE()

	CAST(msg, buf, buflen);
	SHIFT_FIELD(msg, msg, buf, buflen);

	ADD_SIZE(msg->len);
	sfsoplen = amsg_sfs_checksumop_size(buf, buflen);
	if (sfsoplen < 0 || sfsoplen > msg->len || sfsoplen > buflen)
		return -1;

	RETURN_SIZE();
}

struct buf_offset {
	int	off, len;
};
#define ADDOFFSET(idx,prefix)					\
	do {							\
		offs[(idx)].off = msg->prefix##off;		\
		offs[(idx)].len = msg->prefix##len;		\
		CHECK_SIZE(offs[(idx)].off);			\
		CHECK_SIZE(offs[(idx)].len);			\
	} while (0)

static int
anoubisd_msg_eventask_size(const char *buf, int buflen)
{
	struct anoubisd_msg_eventask	*msg;
	struct buf_offset		 offs[5];
	int				 i, j, total = 0;
	DECLARE_SIZE();

	CAST(msg, buf, buflen);

	/* Detect offset overlaps */
	ADDOFFSET(0, csum);
	ADDOFFSET(1, path);
	ADDOFFSET(2, ctxcsum);
	ADDOFFSET(3, ctxpath);
	ADDOFFSET(4, ev);

	for (i=0; i<5; ++i) {
		int	s1, e1;
		if (offs[i].len == 0)
			continue;
		s1 = offs[i].off;
		e1 = s1 + offs[i].len;
		if (e1 > total)
			total = e1;
		for (j=0; j<i; ++j) {
			int	s2, e2;
			if (offs[j].len == 0)
				continue;
			s2 = offs[j].off;
			e2 = s2 + offs[j].len;
			if (s2 < e1 && s1 < e2)
				return -1;
		}
	}

	SHIFT_FIELD(msg, payload, buf, buflen);
	if (total > buflen)
		return -1;
	ADD_SIZE(total);
	if (msg->pathlen)
		CHECK_STRING(buf+msg->pathoff, msg->pathlen);
	if (msg->ctxpathlen)
		CHECK_STRING(buf+msg->ctxpathoff, msg->ctxpathlen);
	if (msg->evlen) {
		int	size = eventdev_hdr_size(buf+msg->evoff, msg->evlen);
		CHECK_SIZE(size);
	}
	RETURN_SIZE();
}

#undef ADDOFFSET

DEFINE_CHECK_FUNCTION(struct, anoubisd_msg_pchange);

static int
anoubisd_sfscache_invalidate_size(const char *buf, int buflen)
{
	struct anoubisd_sfscache_invalidate	*msg;
	DECLARE_SIZE();

	CAST(msg, buf, buflen);
	SHIFT_FIELD(msg, payload, buf, buflen);
	CHECK_LEN(msg->plen, buflen);
	CHECK_STRING(buf, msg->plen);
	SHIFT_CNT(msg->plen, buf, buflen);
	if (msg->keylen) {
		CHECK_LEN(msg->keylen, buflen);
		STRINGS(buf, buflen);
	}
	RETURN_SIZE();
}

static int
anoubisd_msg_upgrade_size(const char *buf, int buflen)
{
	struct anoubisd_msg_upgrade	*msg;
	DECLARE_SIZE();

	CAST(msg, buf, buflen);
	SHIFT_FIELD(msg, chunk, buf, buflen);
	if (msg->upgradetype != ANOUBISD_UPGRADE_NOTIFY)
		CHECK_LEN(msg->chunksize, buflen);
	switch(msg->upgradetype) {
	case ANOUBISD_UPGRADE_OK:
		if (msg->chunksize != 1)
			return -1;
		ADD_SIZE(msg->chunksize);
		break;
	case ANOUBISD_UPGRADE_NOTIFY:
		/*
		 * chunksize is the number of upgraded files. No data in
		 * in chunk.
		 */
		break;
	case ANOUBISD_UPGRADE_CHUNK:
		if (msg->chunksize)
			STRINGS(buf, buflen);
		break;
	default:
		if (msg->chunksize)
			return -1;
	}
	RETURN_SIZE();
}

static int
anoubisd_sfs_update_all_size(const char *buf, int buflen)
{
	struct anoubisd_sfs_update_all	*msg;
	DECLARE_SIZE();

	CAST(msg, buf, buflen);
	SHIFT_FIELD(msg, payload, buf, buflen);
	SHIFT_CNT(msg->cslen, buf, buflen);
	STRINGS(buf, buflen);

	RETURN_SIZE();
}

static int
anoubisd_msg_config_size(const char *buf, int buflen)
{
	struct anoubisd_msg_config	*msg;
	unsigned int			 i;
	DECLARE_SIZE();

	CAST(msg, buf, buflen);
	SHIFT_FIELD(msg, chunk, buf, buflen);
	SHIFT_STRING(buf, buflen);
	if (msg->triggercount > SIZE_LIMIT)
		return -1;
	for (i=0; i<msg->triggercount; ++i) {
		SHIFT_STRING(buf, buflen);
	}

	RETURN_SIZE();
}

static int
anoubisd_msg_logit_size(const char *buf, int buflen)
{
	struct anoubisd_msg_logit	*msg;
	DECLARE_SIZE();

	CAST(msg, buf, buflen);
	SHIFT_FIELD(msg, msg, buf, buflen);
	STRINGS(buf, buflen);

	RETURN_SIZE();
}

static int
anoubisd_msg_passphrase_size(const char *buf, int buflen)
{
	struct anoubisd_msg_passphrase	*msg;
	DECLARE_SIZE();

	CAST(msg, buf, buflen);
	SHIFT_FIELD(msg, payload, buf, buflen);
	SHIFT_STRING(buf, buflen);

	RETURN_SIZE();
}

static int
anoubisd_msg_size(const char *buf, int buflen)
{
	DECLARE_SIZE();
	struct anoubisd_msg	*msg;

	CAST(msg, buf, buflen);
	SHIFT_FIELD(msg, msg, buf, buflen);
	switch(msg->mtype) {
	VARIANT(ANOUBISD_MSG_POLREQUEST, anoubisd_msg_polrequest, buf, buflen)
	VARIANT(ANOUBISD_MSG_POLREPLY, anoubisd_msg_polreply, buf, buflen)
	VARIANT(ANOUBISD_MSG_CHECKSUM_OP, anoubisd_msg_csumop, buf, buflen)
	VARIANT(ANOUBISD_MSG_CHECKSUMREPLY, anoubisd_msg_csumreply, buf, buflen)
	VARIANT(ANOUBISD_MSG_EVENTDEV, eventdev_hdr, buf, buflen)
	VARIANT(ANOUBISD_MSG_LOGREQUEST, anoubisd_msg_logrequest, buf, buflen)
	VARIANT(ANOUBISD_MSG_EVENTREPLY, eventdev_reply, buf, buflen)
	VARIANT(ANOUBISD_MSG_EVENTCANCEL, eventdev_token, buf, buflen)
	VARIANT(ANOUBISD_MSG_EVENTASK, anoubisd_msg_eventask, buf, buflen)
	VARIANT(ANOUBISD_MSG_POLICYCHANGE, anoubisd_msg_pchange, buf, buflen)
	VARIANT(ANOUBISD_MSG_SFSCACHE_INVALIDATE, anoubisd_sfscache_invalidate,
	    buf, buflen)
	VARIANT(ANOUBISD_MSG_UPGRADE, anoubisd_msg_upgrade, buf, buflen)
	VARIANT(ANOUBISD_MSG_SFS_UPDATE_ALL, anoubisd_sfs_update_all,
	    buf, buflen)
	VARIANT(ANOUBISD_MSG_CONFIG, anoubisd_msg_config, buf, buflen)
	VARIANT(ANOUBISD_MSG_LOGIT, anoubisd_msg_logit, buf, buflen)
	VARIANT(ANOUBISD_MSG_PASSPHRASE, anoubisd_msg_passphrase, buf, buflen);
	default:
		log_warnx("anoubisd_msg_size: Bad message type %d",
		    msg->mtype);
		return -1;
	}
	RETURN_SIZE();
}

void
amsg_verify(struct anoubisd_msg *msg)
{
	int		 buflen, tmp;
	char		*buf = (void *)msg;
	static char	 fill[4] = { 0xde, 0xad, 0xbe, 0xef };

	if (msg->size < (int)sizeof(*msg)) {
		log_warnx("Invalid message size: %d", msg->size);
		master_terminate(EINVAL);
	}
	buflen = msg->size;
	tmp = anoubisd_msg_size(buf, buflen);
	if (tmp < 0 || tmp > buflen) {
		char	msgfmt[1024];
		int	i, pos;
		/*
		 * Messages that are larger than the surrounding buffer
		 * are never valid. We terminate the daemon.
		 */
		log_warnx("MESSAGE SIZE ERROR: buf=%d, real=%d, type=%d",
		    buflen, tmp, msg->mtype);
		for (i=pos=0; i<buflen && pos<1000; ++i, pos+=2)
			sprintf(msgfmt+pos, "%02x", (unsigned char)(buf[i]));
		if (i<buflen) {
			msgfmt[pos++] = '.';
			msgfmt[pos++] = '.';
			msgfmt[pos++] = '.';
		}
		msgfmt[pos] = 0;
		log_warnx("MSG DATA: %s", msgfmt);
		master_terminate(EINVAL);
	} else if (tmp < buflen) {
		/*
		 * Fill space in messages that are smaller than the surrounding
		 * buffer with data that is unlikely to make sense in case it
		 * is ever accessed. Issue a warning for message that are
		 * significantly smaller than the buffer. Minor differences
		 * can occur due to alignment errors etc.
		 */
		int	i;
		for (i=tmp; i<buflen; ++i)
			buf[i] = fill[i%4];
		if (tmp < buflen - 8)
			log_warnx("Oversized message: buf=%d, real=%d, type=%d",
			    buflen, tmp, msg->mtype);
	}
}

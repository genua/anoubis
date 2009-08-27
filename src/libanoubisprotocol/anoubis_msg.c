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
#include <stdlib.h>
#include <string.h>
#include <anoubis_msg.h>
#include <anoubis_dump.h>
#include <crc32.h>

struct anoubis_msg * anoubis_msg_new(size_t len)
{
	struct anoubis_msg * ret;

	if (len > ANOUBIS_MESSAGE_MAXLEN-CSUM_LEN)
		return NULL;
	len += CSUM_LEN;
	ret = malloc(sizeof(struct anoubis_msg));
	if (!ret)
		return NULL;
	ret->length = (u_int32_t) len;
	ret->u.buf = malloc(len);
	if (!ret->u.buf) {
		free(ret);
		return NULL;
	}
	ret->next = NULL;
	return ret;
}

struct anoubis_msg * anoubis_msg_clone(struct anoubis_msg * m)
{
	struct anoubis_msg * ret = anoubis_msg_new(m->length - CSUM_LEN);
	if (!ret)
		return NULL;
	assert(m->length == ret->length);
	memcpy(ret->u.buf, m->u.buf, m->length);
	return ret;
}

int anoubis_msg_resize(struct anoubis_msg * m, size_t len)
{
	/*@-memchecks@*/ /* splint doesn't support realloc properly */
	void * new = realloc(m->u.buf, len);
	if (!new)
		return -ENOMEM;
	m->u.buf = new;
	m->length = (u_int32_t) len;
	return 0;
	/*@=memchecks@*/
}

void anoubis_msg_free(struct anoubis_msg * m)
{
	free(m->u.buf);
	free(m);
}

void stringlist_iterator_init(struct stringlist_iterator * it,
    struct anoubis_msg * m, struct proto_opt * opts)
{
	unsigned int overhead = CSUM_LEN + sizeof(Anoubis_StringListMessage);

	it->m = m;
	it->pos = 0;
	if (m->length <= overhead) {
		it->len = 0;
	} else {
		it->len = m->length - overhead;
		it->buf = m->u.stringlist->stringlist;
	}
	it->opts = opts;
}

int stringlist_iterator_get(struct stringlist_iterator * it)
{
	int i;
	int len;
	do {
		if (it->pos >= it->len)
			return -1;
		len = 0;
		while(it->pos+len < it->len && it->buf[it->pos+len] != ',')
			len++;
		for (i=0; it->opts[i].val >= 0; i++) {
			if (strlen(it->opts[i].str) != (size_t)len)
				continue;
			if (strncmp(it->opts[i].str, it->buf+it->pos, len) == 0)
			    break;
		}
		it->pos += len+1;
	} while(it->opts[i].val < 0);
	return it->opts[i].val;
}

struct anoubis_msg * anoubis_stringlist_msg(u_int32_t type,
    struct proto_opt * opts, int * ovals)
{
	int i, k, slen = 0;
	char * p;
	struct anoubis_msg * m;
	for (i=0; ovals[i] >= 0; ++i) {
		for (k=0; opts[k].val != ovals[i]; ++k)
			if (opts[k].val < 0)
				return NULL;
		slen += strlen(opts[k].str);
		if (i)
			slen++;
	}
	m = anoubis_msg_new(sizeof(Anoubis_StringListMessage) + slen);
	set_value(m->u.stringlist->type, type);
	p = m->u.stringlist->stringlist;
	for (i=0; ovals[i] >= 0; ++i) {
		int len;
		if (i) {
			*p = ',';
			p++;
		}
		for (k=0; opts[k].val != ovals[i]; k++)
			;
		len = strlen(opts[k].str);
		strncpy(p, opts[k].str, len);
		p += len;
	}
	return m;
}

int
anoubis_extract_sig_type(const struct anoubis_msg *m, int reqtype,
    const void **data)
{
	int		 error;
	int		 payloadlen, off = 0, type, len;
	const void	*payload;

	if (!m || !VERIFY_LENGTH(m, sizeof(Anoubis_AckPayloadMessage)))
		return -EFAULT;
	error = get_value(m->u.ackpayload->error);
	if (error != 0)
		return -error;
	payloadlen = m->length - sizeof(Anoubis_AckPayloadMessage) - CSUM_LEN;
	if (payloadlen < 0)
		return -EFAULT;
	payload = m->u.ackpayload->payload;
	while(1) {
		if (off + (int)sizeof(u32n) > payloadlen)
			return -EFAULT;
		type = get_value(*(u32n*)(payload + off));
		if (type == ANOUBIS_SIG_TYPE_EOT)
			return 0;
		off += sizeof(u32n);
		if (off + (int)sizeof(u32n) > payloadlen)
			return -EFAULT;
		len = get_value(*(u32n*)(payload + off));
		off += sizeof(u32n);
		if (len < 0 || off + len > payloadlen)
			return -EFAULT;
		if (type == reqtype)
			break;
		off += len;
	}
	if (data) {
		(*data) = payload + off;
	}
	return len;
}

/* Does NOT free the message! */
int anoubis_msg_send(struct achat_channel * chan, struct anoubis_msg * m)
{
	achat_rc ret;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_GeneralMessage)))
		return -EINVAL;
	crc32_set(m->u.buf, m->length);
	anoubis_dump(m, "anoubis_msg_send");
	ret = acc_sendmsg(chan, m->u.buf, m->length);
	if (ret != ACHAT_RC_OK)
		return -EIO;
	return 0;
}

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

struct anoubis_msg * anoubis_msg_new(u_int32_t len)
{
	struct anoubis_msg * ret;

	if (len > ANOUBIS_MESSAGE_MAXLEN-CSUM_LEN)
		return NULL;
	len += CSUM_LEN;
	ret = malloc(sizeof(struct anoubis_msg));
	if (!ret)
		return NULL;
	ret->length = len;
	ret->u.buf = malloc(len);
	if (!ret->u.buf) {
		free(ret);
		return NULL;
	}
	return ret;
}

int anoubis_msg_resize(struct anoubis_msg * m, u_int32_t len)
{
	void * new = realloc(m->u.buf, len);
	if (!new)
		return -ENOMEM;
	m->u.buf = new;
	m->length = len;
	return 0;
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

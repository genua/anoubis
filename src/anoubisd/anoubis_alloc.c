/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <anoubis_alloc.h>
/**
 * Magic cookie that is stored in front of an allocated buffer. It
 * ensures that a buffer was actually allocated by the abuf allocation
 * functions.
 */
#define MAGIC_COOKIE		0xf00fba20UL
#define MAGIC_COOKIE_FREE	0x0ff0deadUL

/**
 * This structure represents the actual buffer in memory. The layout of
 * the memory returned by the allocation functions is as follows:
 *
 * +---------------------------------------------------------------+
 * | struct abuf_buffer | MAGIC_COOKIE | actual data ...           |
 * +---------------------------------------------------------------+
 *                                      ^
 *                                      |
 *                                abuf_buffer.data points here
 *
 * The abuf_buffer and the magic cookie are represented by this structure.
 * NOTE: The abuf_buffer structures used by the callers are a copy and
 * NOTE: not a reference to this structure. The abuf_free function will
 * NOTE: use the data pointer in the buffer to find the actual management
 * NOTE: structure associated with the buffer.
 *
 * This structure is internal to the memory allocation library and must
 * never be accessed outside of this file.
 */
struct abuf_buffer_internal {
	struct abuf_buffer	buf;
	unsigned long		magic;
};


struct abuf_buffer
abuf_alloc(unsigned int length)
{
	struct abuf_buffer_internal	*intbufp = NULL;

	if (length)
		intbufp = malloc(length
		    + sizeof(struct abuf_buffer_internal));
	if (!intbufp)
		return ABUF_EMPTY;

	intbufp->buf.length = length;
	intbufp->magic = MAGIC_COOKIE;
	intbufp->buf.data = intbufp+1;
	return intbufp->buf;
}

struct abuf_buffer
abuf_realloc(struct abuf_buffer src, size_t newlen)
{
	struct abuf_buffer_internal	*intbufp, *newbufp;

	if (src.length == 0)
		return abuf_alloc(newlen);
	if (newlen <= src.length)
		return src;
	intbufp = ((struct abuf_buffer_internal *)(src.data)) - 1;
	ASSERT_MEM(intbufp->magic == MAGIC_COOKIE);
	ASSERT_MEM(intbufp->buf.data == src.data);
	/* Mark buffer as free in case realloc frees the original data. */
	intbufp->magic = MAGIC_COOKIE_FREE;
	newbufp = realloc(intbufp,
	    newlen + sizeof(struct abuf_buffer_internal));
	if (newbufp == NULL) {
		intbufp->magic = MAGIC_COOKIE;
		abuf_free(intbufp->buf);
		return ABUF_EMPTY;
	}
	newbufp->magic = MAGIC_COOKIE;
	newbufp->buf.data = newbufp+1;
	newbufp->buf.length = newlen;
	return newbufp->buf;
}

struct abuf_buffer
abuf_open(struct abuf_buffer buf, unsigned int off)
{
	ASSERT_MEM(off <= buf.length);
	if (off > buf.length)
		return ABUF_EMPTY;
	buf.data += off;
	buf.length -= off;
	return buf;
}

struct abuf_buffer
abuf_open_frommem(void *data, unsigned int len)
{
	struct abuf_buffer	buf;

	buf.data = data;
	buf.length = len;

	return buf;
}

unsigned int
abuf_limit(struct abuf_buffer *bufp, unsigned int len)
{
	if (len < bufp->length)
		bufp->length = len;
	return bufp->length;
}

struct abuf_buffer
abuf_zalloc(unsigned int length)
{
	struct abuf_buffer	buf = abuf_alloc(length);

	if (buf.length)
		memset(buf.data, 0, buf.length);
	return buf;
}

void
abuf_free(struct abuf_buffer buffer)
{
	struct abuf_buffer_internal	*realbuf;

	if (buffer.length == 0)
		return;
	realbuf = ((struct abuf_buffer_internal *)(buffer.data)) - 1;
	ASSERT_MEM(realbuf->magic == MAGIC_COOKIE);
	ASSERT_MEM(realbuf->buf.data == buffer.data);
	/* Destinguish double frees from other memory errors. */
	realbuf->magic = MAGIC_COOKIE_FREE;
	free(realbuf);
}

void
__abuf_free_type(void *data, unsigned int length)
{
	struct abuf_buffer_internal	*realbuf;

	realbuf = ((struct abuf_buffer_internal *)data) - 1;
	ASSERT_MEM(realbuf->magic == MAGIC_COOKIE);
	ASSERT_MEM(realbuf->buf.length == length);
	ASSERT_MEM(realbuf->buf.data == data);
	/* Destinguish double frees from other memory errors. */
	realbuf->magic = MAGIC_COOKIE_FREE;
	free(realbuf);
}

int
abuf_ncmp(struct abuf_buffer b1, struct abuf_buffer b2,
    unsigned int cmplen)
{
	int		ret;

	if (cmplen <= b1.length && cmplen <= b2.length)
		return memcmp(b1.data, b2.data, cmplen);
	if (b1.length == b2.length)
		return memcmp(b1.data, b1.data, b1.length);
	if (b1.length < b2.length) {
		ret = memcmp(b1.data, b2.data, b1.length);
		if (ret == 0)
			ret = -1;
	} else {
		ret = memcmp(b1.data, b2.data, b2.length);
		if (ret == 0)
			ret = 1;
	}
	return ret;
}

int
abuf_equal(const struct abuf_buffer b1, const struct abuf_buffer b2)
{
	if (b1.length != b2.length)
		return 0;
	if (b1.length == 0)
		return 1;
	return (memcmp(b1.data, b2.data, b1.length) == 0);
}

unsigned int
abuf_copy_frombuf(void *dst, const struct abuf_buffer src, unsigned int len)
{
	ASSERT_MEM(len <= src.length);
	if (src.length < len)
		len = src.length;
	memcpy(dst, src.data, len);
	return len;
}

unsigned int
abuf_copy_tobuf(struct abuf_buffer dst, const void *src, unsigned int len)
{
	ASSERT_MEM(len <= dst.length);
	if (dst.length < len)
		len = dst.length;
	memcpy(dst.data, src, len);
	return len;
}

unsigned int
abuf_copy(struct abuf_buffer dst, struct abuf_buffer src)
{
	unsigned int	len = src.length;

	if (!__abuf_check(dst, 0, len))
		len = dst.length;
	memmove(dst.data, src.data, len);
	return len;
}

unsigned int
abuf_copy_part(struct abuf_buffer dst, unsigned int dstoff,
    struct abuf_buffer src, unsigned int srcoff, unsigned int len)
{
	if (!__abuf_check(dst, dstoff, len)
	    || !__abuf_check(src, srcoff, len))
		return 0;
	memmove(dst.data + dstoff, src.data + srcoff, len);
	return len;
}

const char *
abuf_tostr(struct abuf_buffer buf, unsigned int off)
{
	unsigned int	 i = off;
	const char	*data = buf.data;

	for (i=off; i<buf.length; ++i)
		if (data[i] == 0)
			return buf.data+off;

	return NULL;
}

char *
abuf_convert_tohexstr(struct abuf_buffer buf)
{
	uint8_t		*ptr = abuf_toptr(buf, 0, abuf_length(buf));
	char		*ret;
	unsigned int	 i, len;

	if (ptr == NULL || abuf_length(buf) == 0)
		return NULL;
	len = 2*abuf_length(buf) + 1;
	ret = malloc(len);
	if (ret == NULL)
		return NULL;
	for (i=0; i<abuf_length(buf); ++i)
		snprintf(ret+2*i, len-2*i, "%02x", ptr[i]);
	return ret;
}

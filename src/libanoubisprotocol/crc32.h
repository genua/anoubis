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
#ifndef ANOUBIS_CRC32_H
#define ANOUBIS_CRC32_H

#define POLY 0x04C11DB7UL

static inline unsigned long _crc32_do(unsigned char * buf, int len)
{
	unsigned long sum = sum = 0xFFFFFFFFUL;
	int i;
	for (i=0; i<len; buf++,i++) {
		unsigned char byte = *buf;
		unsigned char mask;
		for (mask = 0x80; mask != 0; mask >>= 1) {
			int highbit = sum >> 31;
			int curbit = !!(byte & mask);
			sum <<= 1;
			if (highbit != curbit)
				sum ^= POLY;
		}
	}
	return sum;
}

static inline unsigned long crc32_get(void * buf, int len)
{
	return _crc32_do(buf, len);
}

static inline int crc32_check(void * buf, int len)
{
	return _crc32_do(buf, len) == 0;
}

static inline void crc32_set(void * _buf, int len)
{
	unsigned char * buf = _buf;
	unsigned long sum = _crc32_do(buf, len-4);
	int i;
	buf += len-4;
	for (i=3; i>=0; i--) {
		buf[i] = sum % 256;
		sum /= 256;
	}
}

#endif

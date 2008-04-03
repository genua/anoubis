/* $OpenBSD: buffer.h,v 1.16 2006/08/03 03:34:41 deraadt Exp $ */

/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Code for manipulating FIFO buffers.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#ifndef __ACCBUFFER_H__
#define __ACCBUFFER_H__

#include <sys/cdefs.h>

#include "anoubischat.h"

#define ACHAT_BUFFER_DEFAULTSZ 0x001000
#define ACHAT_BUFFER_MAX_CHUNK 0x100000
#define ACHAT_BUFFER_MAX_LEN   0xa00000
#define ACHAT_BUFFER_ALLOCSZ   0x008000

struct achat_buffer {
	u_char	*buf;		/* achat_buffer for data. */
	size_t	 alloc;		/* Number of bytes allocated for data. */
	size_t	 offset;	/* Offset of first byte containing data. */
	size_t	 end;		/* Offset of last byte containing data. */
};
typedef struct achat_buffer achat_buffer;

__BEGIN_DECLS
achat_rc acc_bufferinit(achat_buffer *);
achat_rc /*@alt void@*/ acc_bufferclear(achat_buffer *);
achat_rc /*@alt void@*/ acc_bufferfree(achat_buffer *);

size_t	 acc_bufferlen(achat_buffer *);
void	*acc_bufferptr(achat_buffer *);

achat_rc acc_bufferappend(achat_buffer *, const void *, size_t);
void *acc_bufferappend_space(achat_buffer *, size_t);
__END_DECLS

#endif /* __ACCBUFFER_H__ */

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

#include "anoubis_chat.h"

/**
 * The default-size of a buffer.
 */
#define ACHAT_BUFFER_DEFAULTSZ 0x001000

/**
 * Maximum size of a data-chunk you can append to the buffer.
 */
#define ACHAT_BUFFER_MAX_CHUNK 0x100000

/**
 * Maximum size of a buffer.
 */
#define ACHAT_BUFFER_MAX_LEN   0xa00000

/**
 * Buffer-alignment.
 * The size of the buffer is aligned at this size.
 */
#define ACHAT_BUFFER_ALLOCSZ   0x008000

/**
 * Structure defines a chat-buffer.
 * The buffer is used to send/receive data from/to the chat-library.
 */
struct achat_buffer {
	/**
	 * achat_buffer for data.
	 */
	u_char	*buf;

	/**
	 * Number of bytes allocated for data.
	 */
	size_t	 alloc;

	/**
	 * Offset of first byte containing data.
	 */
	size_t	 offset;

	/**
	 * Offset of last byte containing data.
	 */
	size_t	 end;
};
typedef struct achat_buffer achat_buffer;

__BEGIN_DECLS
achat_rc acc_bufferinit(/*@special@*/ achat_buffer *b)
	/*@sets b@*/ /*@allocates b->buf@*/;
achat_rc /*@alt void@*/ acc_bufferclear(achat_buffer *);
achat_rc /*@alt void@*/ acc_bufferfree(/*@special@*/achat_buffer *b)
    /*@releases b->buf@*/ /*@uses b->alloc@*/;

size_t	 acc_bufferlen(achat_buffer *);
/*@exposed@*/ void	*acc_bufferptr(achat_buffer *);

achat_rc acc_bufferappend(/*@out@*/achat_buffer *, const void *,
    size_t);
achat_rc acc_bufferconsume(achat_buffer *, size_t);
achat_rc acc_buffertrunc(achat_buffer *, size_t);
/*@null@*/ /*@exposed@*/void *acc_bufferappend_space(/*@out@*/achat_buffer *,
    size_t);
__END_DECLS

#endif /* __ACCBUFFER_H__ */

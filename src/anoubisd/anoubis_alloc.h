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

/**
 * \file anoubis_alloc.h
 *
 * This file implements memory allocation functions for anoubis. The
 * functions are designed to be easy to use and still safe wrt. memory
 * errors.
 *
 * The basic unit that all the functions in this file deal with is
 * a buffer.
 *
 * Error handling:
 * The allocation functions use the marco ASSERT_MEM to handle
 * memory allocation errors. This macro defaults to a call to assert
 * but the code behaves reasonably if the assert function is defined
 * to be a no-op. The documentation will describe the behaviour that
 * happens if the assertions are not compiled into the code.
 *
 * If assertions are enabled, all memory allocation failures and all
 * buffer overruns that are detected will trigger an assertion instead of
 * the described error handling behaviour.
 */

#ifndef _ANOUBIS_ALLOC_H_
#define _ANOUBIS_ALLOC_H_

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#ifndef ASSERT_MEM
#define ASSERT_MEM(X)	assert(X)
#endif

/**
 * This structure is the basic handle that is used by all memory allocation
 * functions. Do not pass this structure around via pointers, instead simply
 * pass it as is.
 *
 * This is an opaque structure, i.e. its fields should never be accessed
 * outside of the implementation of the allocation functions.
 * This structure is only defined in the header fill for the benefit of
 * some access macros that are implemented in this header file for
 * performance reasons.
 */
struct abuf_buffer {
	unsigned long	 length;
	void		*data;
};

/**
 * Internal checking function: Verify that a memory area of length len
 * that starts at offset off in the buffer does not exceed the bounds of
 * the buffer.
 * @param buf The buffer.
 * @param off The offset into the buffer where the memory area starts.
 * @param len The length of the memory area.
 * @return True iff the memory area is intirely within the bounds of the
 *     buffer.
 */
static inline int
__abuf_check(struct abuf_buffer buf, unsigned int off,
    unsigned int len)
{
	int	ret;
	ret = (off <= buf.length && len <= buf.length && off+len <= buf.length);
	ASSERT_MEM(ret);
	return ret;
}

/**
 * Internal checking function, similar to __abuf_check. However, it
 * returns a pointer tot the data.
 * @param buf The buffer.
 * @param off The offset into the buffer whee the memory area starts.
 * @param len The length of the memory area.
 * @return A pointer to the memory area on success or NULL.
 */
static inline void *
__abuf_check_ptr(struct abuf_buffer buf, unsigned int off,
    unsigned int len)
{
	int	ok = __abuf_check(buf, off, len);

	if (ok)
		return buf.data + off;
	return NULL;
}

/**
 * This macro returs true if the buffer is empty.
 * @param BUF The buffer.
 * @return True iff the buffer is empty.
 */
#define abuf_empty(BUF)		((BUF).length == 0)

/**
 * Return the lenght of th buffer.
 * @param BUF The buffer.
 * @return The length of the buffer. An empty buffer has length 0.
 */
#define abuf_length(BUF)	((BUF).length)

/**
 * Cast the buffer to a structure of a given type.
 * @param BUF The buffer.
 * @param TYPE The target type.
 * @return A pointer to the start of the buffer data, cast to type TYPE.
 *     NULL if the buffer is not large enough to hold a structure of the
 *     given type.
 */
#define abuf_cast(BUF, TYPE)						\
	((TYPE*)(__abuf_check_ptr((BUF), 0, sizeof(TYPE))))

/**
 * Cast the data at offset OFF in the buffer BUF to type TYPE.
 * This is the same as @see abuf_cast except that it is possible to provide
 * an offset into the buffer.
 * @param BUF The buffer.
 * @param OFF The offset into the buffer.
 * @param TYPE the target type.
 * @return A pointer offset bytes into the buffer data, cast tot type TYPE.
 *     NULL if the buffer is not large enough to hold a structure of the
 *     given type.
 */
#define abuf_cast_off(BUF, OFF, TYPE)					\
	((TYPE*)(__abuf_check_ptr((BUF), (OFF), sizeof(TYPE))))

/**
 * Allocate a buffer that is large enough to hold a structure of
 * type TYPE and return a pointer to that structure. This function is
 * intended for structure that have a fixed size and no variable length
 * part at the end of the structure.
 * Use abuf_free_type to free a structure that was allocated in this way.
 *
 * @param TYPE The type of the structure.
 * @return A pointer of type TYPE or NULL in case of an allocation
 *     failure.
 */
#define		abuf_alloc_type(TYPE)					\
	abuf_cast(abuf_alloc(sizeof(TYPE)), TYPE)

/**
 * The same as abuf_alloc_type except that the memory in the buffe is
 * initialized with zeros.
 *
 * @param TYPE The type of the structure.
 * @return A pointer of type TYPE or NULL in case of an allocation
 *     failure.
 */
#define		abuf_zalloc_type(TYPE)					\
	abuf_cast(abuf_zalloc(sizeof(TYPE)), TYPE)

/* Internal helper function for abuf_free_type */
extern void	__abuf_free_type(void *, unsigned int size);

/**
 * Free a data structure allocated by abuf_alloc_type.
 * @param PTR The pointer to the data structure.
 * @param TYPE The type of the data structure (used for error checking).
 * @return None.
 */
#define abuf_free_type(PTR, TYPE) do {					\
		TYPE			*tmpptr = (PTR);		\
		__abuf_free_type(tmpptr, sizeof(*tmpptr));		\
	} while(0)

#define ABUF_EMPTY_INITIALIZER { 0, NULL }
/**
 * An empty buffer defined in every module that includes this
 * header file.
 */
static const struct abuf_buffer ABUF_EMPTY = ABUF_EMPTY_INITIALIZER;

/**
 * Allocate a new buffer.
 * Use abuf_free to free a buffer allocated in this way.
 *
 * @param length The length of the buffer.
 * @return The new buffer.
 *
 * In case of a memory allocation failure, an empty buffer is returned.
 * This case cannot be destinguished from an allocation of an empty buffer!
 */
extern struct abuf_buffer	abuf_alloc(unsigned int length);

/**
 * Change (increase) the size of the data allocated to a particular
 * buffer. Buffers are not shrinked! Reallocation will return the original
 * buffer if shrinking is requested. In case of an error, an empty buffer
 * will be returned and the original buffer will be freed!
 *
 * @param BUF The buffer. Callers must assume that abuf_free is called
 *     on this buffer, i.e. pointers to its memory and other buffers that
 *     reference the same memory are invalidated.
 * @param newsize The new size of the buffer.
 * @return A buffer with the new size or an empty buffer if memory allocation
 *     fails. The data contained in the original buffer is copied to the
 *     start of the new buffer. The rest of the buffer remains unchanged.
 *
 * NOTE: There are several differences to the libc realloc(3c) function:
 *  - realloc(3c) does not free the original buffer in case of an error.
 *  - realloc(3c) can be used to shrink an existing memory region.
 * If you need to shrink the actual memory size occupied by a buffer use
 * a sequence like abuf_limit followed by abuf_clone and abuf_free on the
 * original buffer.
 */
extern struct abuf_buffer	abuf_realloc(struct abuf_buffer buf,
				    size_t newsize);

/**
 * Allocate a new buffer and initialize the data with zero.
 * Use abuf_free to free a buffer allocated in this way.
 *
 * @param length The length of the buffer.
 * @return The new buffer.
 *
 * In case of a memory allocation failure, an empty buffer is returned.
 * This case cannot be destinguished from an allocation of an empty buffer!
 */
extern struct abuf_buffer	abuf_zalloc(unsigned int length);

/**
 * Create an identical copy of a buffer and its associated data.
 *
 * @param 1st The source buffer.
 * @return A buffer of the same length the source buffer that contains
 *     a copy of the source buffer's data. An empty buffer if memory
 *     allocation failed.
 */
static inline struct abuf_buffer
abuf_clone(const struct abuf_buffer src)
{
	struct abuf_buffer	ret = abuf_alloc(abuf_length(src));

	if (abuf_length(ret) == 0)
		return ABUF_EMPTY;
	ASSERT_MEM(abuf_length(ret) == abuf_length(src));
	memcpy(ret.data, src.data, abuf_length(ret));
	return ret;
}

/**
 * Free a buffer that was previously allocated with @see abuf_alloc and
 * friends. Any buffer returned from one of the buffer allocation functions
 * can be freed by this function even if the allocation actually failed.
 *
 * @param buf The buffer.
 * @return None.
 */
extern void			abuf_free(struct abuf_buffer buf);

/**
 * Create a new buffer that represents the data at some offset in
 * another buffer. A buffer returned from this function can be used
 * in all function calls that expect a buffer.
 * The only exception is is the abuf_free function! A buffer returned
 * from this function must never be freed. Instead the initial buffer that
 * holds the data must be freed and this will invalidate the dependent
 * buffer returned from this function as well.
 *
 * @param buf The buffer.
 * @param off The offset into th buffer where the memory area represented
 *     by the new buffer starts.
 * @return A buffer that represents the memory area starting at offset
 *     OFF in the buffer. The memory area extends up to the end of the
 *     buffer.
 */
extern struct abuf_buffer	abuf_open(struct abuf_buffer buf,
				    unsigned int off);


/**
 * Limit the size of an existing buffer to the given number of bytes.
 * This is mostly useful for dependent buffers that are not created by
 * abuf_alloc.
 *
 * @param bufp A _pointer_ to the buffer.
 * @param len The new length limit of the buffer.
 * @return The actual length. This may be smaller than the length parameter
 *    if the initial buffer size is smaller.
 */
extern unsigned int		abuf_limit(struct abuf_buffer *buf,
				    unsigned int len);

/**
 * Creata new buffer that represents pre-allocated memory that is not
 * under the control of this library. Obviously, such a buffer must not
 * be passed bo abuf_free and it is invalidated if the underlying memory
 * is freed.
 *
 * @param data A pointer to the memory area.
 * @param len The length of the buffer.
 * @return A buffer that represents the memory region.
 */
extern struct abuf_buffer	abuf_open_frommem(void *data, unsigned int len);

/**
 * Return true if the buffers have the same length and the same contents.
 *
 * @param b1 The first buffer.
 * @param b2 The second buffer.
 * @return True if the buffer contents are equal.
 */
extern int	 abuf_equal(const struct abuf_buffer b1,
		     const struct abuf_buffer b2);

/**
 * Compare the contents of two buffers using memcmp. At most cmplen
 * bytes are compared. If any of the buffers is smaller than cmplen,
 * the comparison stops at the end of that buffer. If the first cmplen
 * bytes of the comparison are the same, the result is zero (equal) even
 * if the buffers have different length. If one buffer is a prefix of the
 * other (that is shorter than cmplen bytes) the buffer length is used to
 * break ties.
 *
 * @param b1 The first buffer.
 * @param b2 The second buffer.
 * @return Zero, -1 or +1 depending on the result of the comparison.
 */
extern int	abuf_ncmp(struct abuf_buffer b1, struct abuf_buffer b2,
		    unsigned int cmplen);

/**
 * Copy data from an abuf_buffer to an ordinary memory buffer.
 *
 * @param dst A normal pointer to the destination buffer.
 * @param src The abuf that is the source of the copy operation.
 * @param len The number of bytes that should be copied.
 * @return The total number of bytes actually copied. If this is
 *     different from len an asseration triggers.
 */
extern unsigned int	abuf_copy_frombuf(void *dst,
			    const struct abuf_buffer src, unsigned int len);

/**
 * Copy data from an ordinary memory buffer to an abuf.
 *
 * @param dst The abuf that is the destination of the copy.
 * @param src A normal pointer to the source buffer.
 * @param len The number of bytes to be copied.
 * @return The number of bytes actually copied. If this is different fro
 *     len an assertion triggers.
 */
extern unsigned int	abuf_copy_tobuf(struct abuf_buffer dst,
			    const void *src, unsigned int len);

/**
 * Copy the contens of the source buffer to the destination buffer.
 * Length of the copy is determined by the source buffer. If the
 * length buffer is not long enough an assertion triggers and only
 * as many bytes as possible are copied.
 *
 * @param dst The destination buffer.
 * @param src The srouce buffer.
 * @return The number of bytes actually copied. If this is less than the
 *     length of the source buffer an assertion triggers.
 */
extern unsigned int	abuf_copy(struct abuf_buffer dst,
			    struct abuf_buffer src);

/**
 * Copy parts of one buffer to another buffer.
 *
 * @param dst The destination buffer.
 * @param dstoff The offset into the destination buffer where the data
 *        is copied to.
 * @param src The source buffer.
 * @param srcoff The offset into the source buffer where the data is copied
 *        from.
 * @len The total number of bytes to be copied.
 * @return The number of bytes actually copied. If this is different from
 *     len an assertion triggers.
 */
extern unsigned int	abuf_copy_part(struct abuf_buffer dst,
			    unsigned int dstoff, struct abuf_buffer src,
			    unsigned int srcoff, unsigned int len);

/**
 * Return a pointer to a string within a buffer. This function verifies
 * that the string is NUL-Terminated.
 *
 * @param buf The buffer.
 * @param off The offset where the string starts.
 * @return A pointer to the string or NULL if the string exceeds the
 *     boundaries of the buffer.
 */
extern const char *	abuf_tostr(struct abuf_buffer buf, unsigned int off);

/**
 * XXX CEH: Try to avoid the use of this function. It defeats the purpose
 * XXX CEH: of the buffer overflow library.
 * Returns a pointer to a memory area in the buffer that can be used
 * without any memory checks.
 * @param buf The buffer.
 * @param off The offset of the memory area inside the buffer.
 * @param len The length of the memory area.
 * @return A pointer to the memory area. If the specified memory area is
 *     not within the bounds of the buffer an assertion triggers and NULL
 *     is returned.
 */
static inline void *
abuf_toptr(struct abuf_buffer buf, unsigned int off, unsigned int len)
{
	if (!__abuf_check(buf, off, len))
		return NULL;
	return buf.data + off;
}

#endif	/* _ANOUBIS_ALLOC_H_ */

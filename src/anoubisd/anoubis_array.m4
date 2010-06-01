dnl This file is preprocessed by m4. It assumes that the two macros are
dnl predefined:
dnl   * PREFIX is the function name prefix used for the new functions and
dnl     types. E.g. if PREFIX is fooarr the new type will be called
dnl     "struct fooarr_array".
dnl   * TYPE is the base type of the array. E.g. if TYPE is "struct foo *"
dnl     the array will have an element type of struct foo *"
dnl
dnl Change quote characters to [ ... ]
dnl
changequote([,])dnl
dnl
dnl Define some names based on PREFIX and TYPE. All of these macros are
dnl only known to m4, i.e. they only exist inside this file.
dnl
dnl HEADERMACRO is the macro used for the generated header
define([HEADERMACRO],[_ANOUBIS_[]PREFIX[]_ARRAY_H_])dnl
dnl
dnl ELMSIZE is the size of the element type.
define([ELMSIZE],[(sizeof(TYPE))])dnl
dnl
dnl ARRTYPE is the name of the array type including the struct
define([ARRTYPE],[struct PREFIX[]_array])dnl
dnl
dnl EMPTY is the static empty array.
define([EMPTY],[PREFIX[_EMPTY]])dnl
dnl
dnl Prefix a function name with the prefix PREFIX
define([FUNC],[PREFIX[]_$1])dnl
[#]line 31 "anoubis_array.m4"
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

[#]ifndef HEADERMACRO
[#]define HEADERMACRO

[#]include <anoubis_alloc.h>

ARRTYPE {
	struct abuf_buffer	buf;
	size_t			limit;
};

static const ARRTYPE EMPTY = {
	ABUF_EMPTY_INITIALIZER, 0
};

/**
 * Allocate an ARRTYPE array with @CNT elements of type TYPE. This
 * function allocates memory for the array.
 *
 * @param cnt The number of elements.
 * @return A newly allocated ARRTYPE array. In case of errors an
 *     empty array is returned. If the memory needed for the array overflows
 *     a size_t a valid array is returned but it may have a different
 *     number of elements than requested.
 */
static inline ARRTYPE
FUNC(alloc)(size_t cnt)
{
	struct PREFIX[]_array	arr;
	size_t			memsz = cnt * ELMSIZE;

	if (memsz == 0)
		return EMPTY;
	arr.buf = abuf_alloc(memsz);
	if (abuf_empty(arr.buf))
		return EMPTY;
	arr.limit = abuf_length(arr.buf) / ELMSIZE;
	return arr;
}

/**
 * Free the memory associated with an ARRTYPE array.
 * This is only valid if the array was allocated with FUNC(alloc).
 *
 * @param arr The array.
 * @return None.
 */
static inline void
FUNC(free)(ARRTYPE arr)
{
	abuf_free(arr.buf);
}

/**
 * Resize an existing ARRTYPE.
 * This is only possible if the array was allocated using FUNC(alloc). If
 * the new memory allocation fails all memory associated with the existing
 * array will be freed. Note that this is different from the behaviour of
 * realloc[(3c)]. Additionally, this function will not actually shrink the
 * size of the memory allocated for the array.
 * This function invalidates all pointers to the memory associated with
 * the argument array. Callers should assume that this function calls
 * FUNC(free).
 *
 * @param arr The old array.
 * @param elmcount The new element count.
 * @return A new array with @elmcount number of elements. Existing elements
 *     of the array are copied to the new array, newly allocated memory is
 *     uninitialized.
 */
static inline ARRTYPE
FUNC(resize)(ARRTYPE arr, size_t elmcount)
{
	size_t		bufsize = elmcount * ELMSIZE;

	if (elmcount < arr.limit || elmcount == 0)
		return  arr;
	/* Check for overflow. */
	if (bufsize / ELMSIZE != elmcount) {
		FUNC(free)(arr);
		return EMPTY;
	}
	arr.buf = abuf_realloc(arr.buf, bufsize);
	if (abuf_empty(arr.buf))
		return EMPTY;
	arr.limit = abuf_length(arr.buf) / ELMSIZE;
	ASSERT_MEM(arr.limit == elmcount);
	return arr;
}

/**
 * Create a new ARRTYPE from the buffer @buf.
 *
 * @param buf The buffer that holds the memory associated with the array.
 *     The buffer must not be freed or shrinked while the array is still
 *     in use.
 * @return A new ARRTYPE array that represents the entire
 *     memory in @buf.
 */
static inline ARRTYPE
FUNC(open)(struct abuf_buffer buf)
{
	ARRTYPE		ret;

	ret.buf = buf;
	ret.limit = (abuf_length(ret.buf) / ELMSIZE);
	return ret;
}

/**
 * Access an element of an ARRTYPE array and return a pointer to it.
 * an abuf_array. Use abuf_array_access instead.
 *
 * @param 1st The array.
 * @param 2nd The index of the element starting at zero.
 * @param 3rd The size of the type
 * @return A pointer to the array or NULL if the element is not within
 *     the array bounds.
 */
static inline void *
__[]FUNC(access)(ARRTYPE arr, size_t idx)
{
	ASSERT_MEM(idx < arr.limit);
	return abuf_toptr(arr.buf, idx*ELMSIZE, ELMSIZE);
}

/**
 *
 * @param 1st The array.
 * @param 2nd The index of the element in the array.
 * @return A pointer to the element with index @IDX or NULL if the
 *     pointer is out of bounds.
 */
[#]define	FUNC(access)(ARR, IDX)				\
	(*(TYPE *)__[]FUNC(access)((ARR), (IDX)))

/**
 * Return the number of elements in an ARRTYPE array.
 *
 * @param arr The array
 * @return The length of the array.
 */
static inline size_t
FUNC(size)(ARRTYPE arr)
{
	return arr.limit;
}

static inline void
FUNC(qsort)(ARRTYPE arr, int (*cmpf)(const void *, const void*))
{
	void	*ptr = abuf_toptr(arr.buf, 0, arr.limit*ELMSIZE);
	qsort(ptr, arr.limit, ELMSIZE, cmpf);
}

[#]endif	/* HEADERMACRO */

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef NEEDBSDCOMPAT

#ifndef __BSDCOMPAT_H__
#define __BSDCOMPAT_H__

#include <features.h>
#define __need_size_t
#include <stddef.h>
#include <unistd.h>

#ifndef attr_always_inline
# define attr_always_inline __attribute__ ((always_inline))
#endif

#ifndef attr_unused
# define attr_unused __attribute__ ((unused))
#endif

__BEGIN_DECLS

extern void __chk_fail (void) __attribute__ ((__noreturn__));

#ifndef HAVE_STRLCPY
extern size_t /*@alt void@*/ __strlcpy_nrm(char *dest, const char *src,
    size_t len);

#if __USE_FORTIFY_LEVEL > 0 && !defined __cplusplus

#define strlcpy(dest, src, len) __strlcpy_chk (dest, src, len)
static attr_always_inline attr_unused size_t /*@alt void@*/ __strlcpy_chk(
    char *dest, const char *src, size_t len) {
	if (__bos(dest) != (size_t) -1 && __bos(dest)  < len ) {
		__chk_fail();
	}
	else
		return __strlcpy_nrm(dest, src, len);
}

#else
#define strlcpy(dest, src, len) __strlcpy_nrm(dest, src, len)
#endif /* __USE_FORTIFY_LEVEL */
#endif /* HAVE_STRLCPY */

#ifndef HAVE_STRLCAT
extern size_t __strlcat_nrm(char *dest, const char *src, size_t len);

#if __USE_FORTIFY_LEVEL > 0 && !defined __cplusplus

#define strlcat(dest, src, len) __strlcat_chk (dest, src, len)
static attr_always_inline attr_unused size_t __strlcat_chk(char *dest,
    const char *src, size_t len) {
	if (__bos(dest) != (size_t) -1 && __bos(dest)  < len ) {
		__chk_fail();
	}
	else
		return __strlcat_nrm(dest, src, len);
}

#else
#define strlcat(dest, src, len) __strlcat_nrm(dest, src, len)
#endif /* __USE_FORTIFY_LEVEL */
#endif /* HAVE_STRLCAT */

#ifndef HAVE_STRTONUM
long long strtonum(const char *, long long, long long, const char **);
#endif

#ifndef HAVE_GETPEEREID
int getpeereid(int, uid_t *, gid_t *);
#endif

#ifndef HAVE_INET_NET_PTON
int inet_net_pton(int , const char *, void *, size_t);
#endif

#ifndef HAVE_SETPROCTITLE
void setproctitle(const char *fmt, ...);
void compat_init_setproctitle(int argc, char *argv[]);
#endif

__END_DECLS

#endif /* __BSDCOMPAT_H__ */
#endif /* NEEDBSDCOMPAT */

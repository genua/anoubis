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

#ifndef _SPLINT_INCLUDES_H_
#define _SPLINT_INCLUDES_H_

/*
 * This file exists only to defined some types that or not defined by
 * C89, but still used in anoubis. This way we can use splint anyways.
 *
 * cf. http://thread.gmane.org/gmane.comp.programming.splint.general/290
 *
 * For saftey raisins, this file will only be processed when parsed with
 * splint.
 */

#ifdef S_SPLINT_S

#define _GNU_SOURCE
#include <unistd.h>

typedef	/*@unsignedintegraltype@*/ unsigned char u_int8_t;
typedef	/*@unsignedintegraltype@*/ unsigned short int u_int16_t;
typedef	/*@unsignedintegraltype@*/ unsigned int u_int32_t;
typedef	/*@unsignedintegraltype@*/ unsigned long long int u_int64_t;


/*@-type@*/ /* These are also defined by unix.h: */
extern void bzero (/*@out@*/ void *b1, size_t length) /*@modifies *b1@*/ ;
/*@=type@*/

/*@-type@*/ /* These are also defined by unix.h: */
extern void bcopy (void *b1, /*@out@*/ void *b2, size_t length)
	/*@modifies *b2@*/ ;  /* Yes, the second parameter is the out param! */
/*@=type@*/

extern int daemon(int nochdir, int noclose);

/*@-skipposixheaders@*/
#include <stdint.h>
#include <sys/types.h>
/*@=skipposixheaders@*/

int /*@printflike@*/ asprintf(char **strp, const char *fmt, ...);

int setresuid(uid_t ruid, uid_t euid, uid_t suid);
int setresgid(gid_t rgid, gid_t egid, gid_t sgid);


#endif /* S_SPLINT_S */
#endif	/* _SPLINT-INCLUDES_H_ */
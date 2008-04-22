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

/*
 * The splint unix library does not seem to know about neither ENOATTR
 * nor about ENODATA. We therefore copy the definition of ENODATA from
 * dev:/usr/include/asm-generig/errno.h.
 */

#ifndef ENODATA
#define	ENODATA		61	/* No data available */
#endif

extern int event_loopexit(/*@null@*/ struct timeval *tv);
extern void event_set(/*@out@*/ struct event *, int, short,
    void (*)(int, short, void *), /*@null@*/ void *);
extern int event_add(/*@dependent@*/struct event *,
     /*@null@*/ struct timeval *);

extern int fcntl(int fd, int cmd, /*@null@*/struct flock *lock);

int /*@alt void@*/ asprintf (/*@out@*/ char **s, char *format, ...)
   /*@allocates *s@*/ ;


/* the following was copied from dev:usr/include/dirent.h */
/* File types for `d_type'.  */
enum
  {
    DT_UNKNOWN = 0,
# define DT_UNKNOWN	DT_UNKNOWN
    DT_FIFO = 1,
# define DT_FIFO	DT_FIFO
    DT_CHR = 2,
# define DT_CHR		DT_CHR
    DT_DIR = 4,
# define DT_DIR		DT_DIR
    DT_BLK = 6,
# define DT_BLK		DT_BLK
    DT_REG = 8,
# define DT_REG		DT_REG
    DT_LNK = 10,
# define DT_LNK		DT_LNK
    DT_SOCK = 12,
# define DT_SOCK	DT_SOCK
    DT_WHT = 14
# define DT_WHT		DT_WHT
  };

/* Convert between stat structure types and directory types.  */
# define IFTODT(mode)	(((mode) & 0170000) >> 12)
# define DTTOIF(dirtype)	((dirtype) << 12)


#endif /* S_SPLINT_S */
#endif	/* _SPLINT-INCLUDES_H_ */

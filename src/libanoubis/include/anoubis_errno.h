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

#ifndef _ANOUBIS_ERRNO_H_
#define _ANOUBIS_ERRNO_H_

#include <sys/cdefs.h>

#include <errno.h>
#include <libintl.h>
#include <string.h>

/* following defines are used by anoubisprotocol*/

#define ANOUBIS_E_OK		0
#define ANOUBIS_E_PERM		1
#define ANOUBIS_E_NOENT		2
#define ANOUBIS_E_SRCH		3
#define ANOUBIS_E_INTR		4
#define ANOUBIS_E_IO		5
#define ANOUBIS_E_NXIO		6
#define ANOUBIS_E_2BIG		7
#define ANOUBIS_E_NOEXEC	8
#define ANOUBIS_E_BADF		9
#define ANOUBIS_E_CHILD		10
/* reserved			11 */
#define ANOUBIS_E_NOMEM		12
#define ANOUBIS_E_ACCESS	13
#define ANOUBIS_E_FAULT		14
#define ANOUBIS_E_NOTBLK	15
#define ANOUBIS_E_BUSY		16
#define ANOUBIS_E_EXIST		17
#define ANOUBIS_E_XDEV		18
#define ANOUBIS_E_NODEV		19
#define ANOUBIS_E_NOTDIR	20
#define ANOUBIS_E_ISDIR		21
#define ANOUBIS_E_INVAL		22
#define ANOUBIS_E_NFILE		23
#define ANOUBIS_E_MFILE		24
#define ANOUBIS_E_NOTTY		25
#define ANOUBIS_E_TXTBUSY	26
#define ANOUBIS_E_FBIG		27
#define ANOUBIS_E_NOSPC		28
#define ANOUBIS_E_SPIPE		29
#define ANOUBIS_E_ROFS		30
#define ANOUBIS_E_MLINK		31
#define ANOUBIS_E_PIPE		32

/* following defines and fucntion are used by anoubiserrorcodes*/

/*
 * all error_codes > A_ERRORCODE_BASE will be (re-)defined by anoubis
 */
#define A_ERRORCODE_BASE 1024

/*
 * some macros to define anoubis_errorcodes
 */
#define A_EPERM_NO_CERTIFICATE 1024
#define A_EPERM_UID_MISMATCH 1025

/*
 * Get Anoubis error code string.
 *
 * This method returns an error string corresponding to given error code.
 *
 * @param[in]	error code
 *
 * @return	corresponding error text
 *		If errnum < A_ERROR_CODE_BASE, systems strerror will
 *		be called and its error text will be returned.
 */

__BEGIN_DECLS

char *anoubis_strerror(int errnum);

__END_DECLS

#endif /*_ANOUBIS_ERRNO_H_*/

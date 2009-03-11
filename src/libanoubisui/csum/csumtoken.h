/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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
#ifndef CSUMTOKEN_H
#define CSUMTOKEN_H

/*
 * Each token in the token stream is of type YYSTYPE. This usually
 * is (or contains) a union with fields for the differenc C-Types
 * that a token can have.
 *
 * In this example each token consists of a line nubmer and the actual
 * data in the union v.
 *
 * yacc must be able to know for each token, which of the union members
 * it should access if we refer to the token using $$ or $<num>.
 * This specified by a %type directive.
 */
#include "config.h"

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include "csum.h"

struct csum_entry {
	unsigned char	*csum;
	uid_t		 uid;
};

struct sig_entry {
	unsigned char	*sign;
	unsigned char	*keyid;
	int		 siglen;
	int		 keylen;
};

typedef struct {
	union {
		int			 Number;
		char			*String;
		struct csum_entry	*CsumEntry;
		struct sig_entry	*SigEntry;
		struct sfs_entry	*Line;
	} v;
	int lineno;
} YYSTYPE;
#define YYSTYPE_IS_DECLARED

void lex_input(FILE *file);
#endif

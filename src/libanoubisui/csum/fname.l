	/*
	 * Character classes: Whitspace, hexdigits and octal digits
	 */
W	[ \t\n]
H	[0-9abcdefABCDEF]
O	[0-7]

	/*
	 * Declare the filename start condition. It is exclusive (%x),
	 * thus rules without a start conditions are not active while
	 * we parse a filename.
	 */
%x	filename

%option	noyywrap
%option nounput
%option prefix="csum"
	/*
	 * C-Source code in the header sections is wrapped in %{ ... %}
	 */
%{
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

#include <config.h>

#include <ctype.h>
#include <string.h>
#include "csumtoken.h"
#include "csum_import.h"

	/*
	 * If used with a yacc/bison parser, these macros must fill
	 * csumlval. PRODUCE should return from the lexer with the token
	 * type.
	 */

/* Defining a maximal Line length */
#define MAXLINE PATH_MAX + ANOUBIS_CS_LEN + 8192
#define ECHO if (fwrite(yytext, yyleng, 1, yyout) != 1) {}

static char buf[MAXLINE];
static int buflen;
extern int currentline;
extern YYSTYPE csumlval;

void
lex_input(FILE *file)
{
	csumin = file;
}

void APPEND(const char *s, size_t l)
{
	size_t cp = l;
	/* This ignores the rest of a long token. */
	if (l + buflen >= sizeof(buf)) {
		l = sizeof(buf) - buflen - 1;
	}
	memcpy(buf+buflen, s, l);
	buflen += cp;
}

#define PRODUCE(V)	{					\
		csumlval.lineno = currentline;			\
		if (buflen) {					\
			buf[buflen] = 0;			\
			csumlval.v.String = strdup(buf);		\
		}						\
		if ((V) == NEWLINE)				\
			currentline++;				\
		buflen = 0;					\
		return (V);					\
	}

%}
%%

	/* The minus token */
"-"{W}	{
		yyless(yyleng-1);
		PRODUCE(MINUS);
	}

	/* Start of a filenmae */
"/"		{
		buflen = 0;
		APPEND(yytext, 1);
		BEGIN(filename);
	}

	/* A hex String */
{H}+{W}	{
		buflen = 0;
		APPEND(yytext, yyleng-1);
		yyless(yyleng-1);
		PRODUCE(HEX);
	}

	/* End of line */
\n	{
		PRODUCE(NEWLINE);
	}

	/* Skip other whitespace */
{W}
	/* Everything else is a character. */
.	{
		PRODUCE(BADCHAR);
	}

	/* Backslash in a filename. */
<filename>\\\\	{
		APPEND("\\", 1);
	}

	/* Escaped newline in a filename is forbidden. */
<filename>\\\n	{
		BEGIN(0);
		yyless(0);
		PRODUCE(FILENAME);
	}

	/* Escaped whitespace in a filename (newline matches above). */
<filename>\\{W}	{
		APPEND(&yytext[yyleng-1], 1);
	}

	/* Backslash followed by three octal characters in a filename. */
<filename>\\{O}{O}{O}	{
		int i, base = 1;
		char res = 0;
		for (i=1; i<=3; ++i) {
			res += base * (yytext[yyleng-i] - '0');
			base *= 8;
		}
		APPEND(&res, 1);
	}

	/* A stray backslash in a filename is forbidden. */
<filename>\\	{
		BEGIN(0);
		yyless(0);
		PRODUCE(FILENAME);
	}

	/* Whitespace ands a filename. */
<filename>{W}	{
		BEGIN(0);
		yyless(0);
		PRODUCE(FILENAME);
	}

	/* Printable characters in a filename are allowed. */
<filename>.	{
		if (isprint(yytext[yyleng-1])) {
			APPEND(yytext, 1);
		} else {
			BEGIN(0);
			yyless(0);
			PRODUCE(FILENAME);
		}
	}

%%

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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "anoubis_csumtoken.h"

/*
 * Function prototypes for functions used during parsing.
 */
static int yyparse(void);
static void yyerror(const char *);
extern int yylex(void);
int currentline;
struct sfs_entry *result;
char test_char;
int hexcnt;
%}

%name-prefix="csum"

%expect	0

%token	ZERO BADCHAR MINUS HEX NEWLINE FILENAME

%type	<v.String>		HEX FILENAME
%destructor { free($$); }	HEX FILENAME
%type	<v.CsumEntry>		csentry
%type	<v.SigEntry>		sigentry
%type	<v.Line>		line file

%%

file	:	/* Empty */ {
		$$ = NULL;
	}
	|	line file {
		$1->next = result;
		result = $1;
		}
	;

line	: NEWLINE {
		$$ = NULL;
	}
	| FILENAME csentry sigentry NEWLINE {
		$$ = calloc(1, sizeof(struct sfs_entry));
		if (!$$) {
			yyerror("Error while calloc\n");
			YYERROR;
		}
		$$->name = strdup($1);
		$$->next = NULL;
		if ($$->name == NULL) {
			free($$);
			yyerror("Error while calloc\n");
			YYERROR;
		}
		if ($2 != NULL) {
			$$->uid = $2->uid;
			$$->checksum = $2->csum;
		}
		if ($3 != NULL) {
			$$->signature = $3->sign;
			$$->siglen = $3->siglen;
			$$->keyid = $3->keyid;
			$$->keylen = $3->keylen;
		}
	}
	;

csentry	: MINUS {
		$$ = NULL;
	}
	| HEX HEX {
		hexcnt = 0;
		$$ = calloc (1, sizeof(struct csum_entry));
		if (!$$) {
			yyerror("Error while calloc");
			YYERROR;
		}
		$$->uid = atoi($1);
		if (sscanf($1, "%d%c", &$$->uid, &test_char)!= 1) {
			yyerror("invalid uid");
			YYERROR;
		}
		$$->csum = string2hex($2, &hexcnt);
		if (hexcnt <= 0 || $$->csum == NULL ||
		    hexcnt != ANOUBIS_CS_LEN) {
			yyerror("Error while string2hex\n");
			YYERROR;
		}
		free($1); free($2);
	}
	;

sigentry : MINUS {
		$$ = NULL;
	}
	| HEX HEX {
		hexcnt = 0;
		$$ = calloc(1, sizeof(struct sig_entry));
		if (!$$) {
			yyerror("Error while calloc\n");
			YYERROR;
		}
		$$->keyid = string2hex($1, &hexcnt);
		if (hexcnt <= 0 || $$->keyid == NULL) {
			yyerror("Error while string2hex\n");
			YYERROR;
		}
		$$->keylen = hexcnt;
		hexcnt = 0;
		$$->sign = string2hex($2, &hexcnt);
		if (hexcnt <= 0 || $$->sign == NULL) {
			yyerror("Error while string2hex");
			YYERROR;
		}
		$$->siglen = hexcnt;
		free($1); free($2);
	}
	;

%%

static void
yyerror(const char *s)
{
	fprintf(stderr, "line %d: %s\n", yylval.lineno, s);
}

struct sfs_entry *
import_csum(FILE *file)
{
	if (!file)
		return NULL;

	result = NULL;
	lex_input(file);
	currentline = 1;
	if (yyparse()) {
		return NULL;
	} else {
		return result;
	}
}

%option nounput
%option noinput
%option prefix="apnvm_"
%option noyywrap

%s DESCR REVL1 REVL2 COMMENT

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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "anoubis_apnvm_cvs.h"
#include "anoubis_apnvm_util.h"

#define ECHO if (fwrite(yytext, yyleng, 1, yyout) != 1) {}

extern time_t cvs_date_parse(const char*); /* Defined in date.y */

static struct apncvs_log *cvslog; /* Structure filled by the parser */
static struct apncvs_rev *currev; /* Current revision */

%}

%%

^"RCS file:".*$ {
	cvslog->rcs_file = strdup(apnvm_trim(yytext + 9));
}

^"Working file:".*$ {
	cvslog->working_file = strdup(apnvm_trim(yytext + 13));
}

^"head:".*$ {
	cvslog->head = strdup(apnvm_trim(yytext + 5));
}

^"branch:".*$ {
	cvslog->branch = strdup(apnvm_trim(yytext + 7));
}

^"locks:".*$ {
	cvslog->locks = strdup(apnvm_trim(yytext + 6));
}

^"access list:".*$ {
	cvslog->access_list = strdup(apnvm_trim(yytext + 12));
}

^"symbolic names:".*$ {
	cvslog->symbolic_names = strdup(apnvm_trim(yytext + 15));
}

^"keyword substitution:".*$ {
	cvslog->keyword_substitution = strdup(apnvm_trim(yytext + 21));
}

^"total revisions: "([0-9]*) {
	cvslog->total_revisions = atoi(apnvm_trim(yytext + 17));
}

"selected revisions: "([0-9]*)$ {
	cvslog->selected_revisions = atoi(apnvm_trim(yytext + 20));
}

^"description:\n" {
	/* Prepare the description */
	cvslog->description = apnvm_emptystring();

	if (cvslog->description == NULL)
		return (1);

	BEGIN(DESCR); /* The following lines contains the description */
}

^-+"\n" {
	/* Create a new revision, append to rev_queue */
	currev = malloc(sizeof(struct apncvs_rev));
	TAILQ_INSERT_TAIL(&cvslog->rev_queue, currev, entry);

	/* The following file is the first line of a revision */
	BEGIN(REVL1);
}

^=+"\n" {
	BEGIN(INITIAL); /* Reset to starting state */
	return 0;
}

<DESCR>(.*)\n {
	/* Append a line to the description-field */
	char *line = apnvm_trim(yytext);
	char *new_descr = apnvm_appendline(cvslog->description, line);

	if (new_descr == NULL) {
		free(cvslog->description);
		cvslog->description = NULL;

		return (1);
	} else
		cvslog->description = new_descr;
}

<REVL1>(.*)\n {
	/* strlen("revision") = 8 */
	currev->revno = strdup(apnvm_trim(yytext + 8));

	BEGIN(REVL2); /* Next line is second line of revision */
}

<REVL2>([[:alnum:]]+)":"([^;\n]*) {
	/*
	 * The second line contains a semicolon-separated list of
	 * key-value-pairs
	 */
	char *str = apnvm_trim(yytext);
	char *colon;

	colon = index(str, ':');
	if (colon != NULL) {
		char *key, *value;

		*colon = '\0';
		key = apnvm_trim(str);
		value = apnvm_trim(colon + 1);

		if (strcmp(key, "date") == 0)
			currev->date = cvs_date_parse(value);
		else if (strcmp(key, "author") == 0)
			currev->author = strdup(value);
		else if (strcmp(key, "state") == 0)
			currev->state = strdup(value);
	}
}

<REVL2>\n {
	/* End of second line of a revision */

	/* Best time to prepare revision comment */
	currev->comment = apnvm_emptystring();

	if (currev->comment == NULL)
		return (1);

	BEGIN(COMMENT); /* Next you have the comment of the revision */
}

<COMMENT>(.*)\n {
	/* Append line to revision-comment */
	char *line = apnvm_trim(yytext);
	char *new_comment = apnvm_appendline(currev->comment, line);

	if (new_comment == NULL) {
		free(currev->comment);
		currev->comment = NULL;

		return (1);
	} else
		currev->comment = new_comment;
}

\n {}
. {}

%%

int
apncvs_log_parse(FILE *f, struct apncvs_log *log)
{
	int parse_result;

	yyin = f;
	yyrestart(yyin);
	BEGIN(INITIAL);

	cvslog = log;

	currev = NULL;

	/* Invoke the parser */
	parse_result = apnvm_lex();

	return (parse_result);
}

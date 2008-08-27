%option nounput

%s DESCR REVL1 REVL2 COMMENT
%x END

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

#include "apncvs.h"

#define ECHO if (fwrite(yytext, yyleng, 1, yyout) != 1) {}

extern time_t cvs_date_parse(const char*); /* Defined in date.y */

static struct apncvs_log *cvslog; /* Structure filled by the parser */
static struct apncvs_rev *currev; /* Current revision */

static char*
apncvs_trim(char *s)
{
	char *p = s;
	char *q;

	/* Trim leading whitespaces */
	while (isspace(*p) && *p != '\0')
		p++;

	/* Trim trailing whitespaces */
	q = s + strlen(s) - 1; /* Points to the last character */
	while (isspace(*q) && (q != s)) {
		*q = '\0';
		q--;
	}

	return p;
}

static char*
apncvs_appendline(char *s, const char *l)
{
	char	*newstr	= s;
	size_t	oldsize	= strlen(s);
	size_t	newsize	= oldsize + strlen(l) + 1;

	newstr = realloc(newstr, newsize);

	if (oldsize > 0)
		strcat(newstr, "\n");

	strcat(newstr, l);

	return (newstr);
}

static char*
apncvs_empty()
{
	char *s = malloc(1);
	*s = '\0';

	return (s);
}

%}

%%

^"RCS file:".*$ {
	cvslog->rcs_file = strdup(apncvs_trim(yytext + 9));
}

^"Working file:".*$ {
	cvslog->working_file = strdup(apncvs_trim(yytext + 13));
}

^"head:".*$ {
	cvslog->head = strdup(apncvs_trim(yytext + 5));
}

^"branch:".*$ {
	cvslog->branch = strdup(apncvs_trim(yytext + 7));
}

^"locks:".*$ {
	cvslog->locks = strdup(apncvs_trim(yytext + 6));
}

^"access list:".*$ {
	cvslog->access_list = strdup(apncvs_trim(yytext + 12));
}

^"symbolic names:".*$ {
	cvslog->symbolic_names = strdup(apncvs_trim(yytext + 15));
}

^"keyword substitution:".*$ {
	cvslog->keyword_substitution = strdup(apncvs_trim(yytext + 21));
}

^"total revisions: "([0-9]*) {
	cvslog->total_revisions = atoi(apncvs_trim(yytext + 17));
}

"selected revisions: "([0-9]*)$ {
	cvslog->selected_revisions = atoi(apncvs_trim(yytext + 20));
}

^"description:\n" {
	cvslog->description = apncvs_empty(); /* Prepare the description */

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
	BEGIN(END); /* EOF */
}

<DESCR>(.*)\n {
	/* Append a line to the description-field */
	char *line = apncvs_trim(yytext);
	cvslog->description = apncvs_appendline(cvslog->description, line);
}

<REVL1>(.*)\n {
	/* strlen("revision") = 8 */
	currev->revno = strdup(apncvs_trim(yytext + 8));

	BEGIN(REVL2); /* Next line is second line of revision */
}

<REVL2>([[:alnum:]]+)":"([^;\n]*) {
	/*
	 * The second line contains a semicolon-separated list of
	 * key-value-pairs
	 */
	char *str = apncvs_trim(yytext);
	char *colon;

	colon = index(str, ':');
	if (colon != NULL) {
		char *key, *value;

		*colon = '\0';
		key = apncvs_trim(str);
		value = apncvs_trim(colon + 1);

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
	currev->comment = apncvs_empty();

	BEGIN(COMMENT); /* Next you have the comment of the revision */
}

<COMMENT>(.*)\n {
	/* Append line to revision-comment */
	char *line = apncvs_trim(yytext);

	currev->comment = apncvs_appendline(currev->comment, line);
}

\n {}
. {}

%%

int
apncvs_log_parse(FILE *f, struct apncvs_log *log)
{
	yyin = f;

	/* Reset variable used by the parser */
	memset(log, 0, sizeof(struct apncvs_log));
	TAILQ_INIT(&log->rev_queue);

	cvslog = log;

	currev = NULL;

	/* Invoke the parser */
	yylex();

	return (0);
}

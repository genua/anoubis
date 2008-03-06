/*
 * Copyright (c) 2002, 2003, 2004 Henning Brauer <henning@openbsd.org>
 * Copyright (c) 2001 Markus Friedl.  All rights reserved.
 * Copyright (c) 2001 Daniel Hartmeier.  All rights reserved.
 * Copyright (c) 2001 Theo de Raadt.  All rights reserved.
 * Copyright (c) 2004, 2005 Hans-Joerg Hoexer <hshoexer@openbsd.org>
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

%{

#include "config.h"

#include <sys/types.h>
#ifndef LINUX
#include <sys/queue.h>
#else
#include "bsdcompat.h"
#include "queue.h"
#endif	/* !LINUX */
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "apn.h"

static struct file {
	TAILQ_ENTRY(file)	 entry;
	FILE			*stream;
	char			*name;
	int			 lineno;
	int			 errors;
} *file;
TAILQ_HEAD(files, file)		 files = TAILQ_HEAD_INITIALIZER(files);

struct file	*pushfile(const char *, int);
int		 popfile(void);
int		 check_file_secrecy(int, const char *);
int		 yyparse(void);
int		 yylex(void);
int		 yyerror(const char *, ...);
int		 kw_cmp(const void *, const void *);
int		 lookup(char *);
int		 lgetc(int);
int		 lungetc(int);
int		 findeol(void);
int		 varset(const char *, void *, size_t, int);
struct var	*varget(const char *);
int		 str2hash(const char *, char *, size_t);
int		 validate_hash(int, const char *);

static struct apnruleset	*apnrsp = NULL;
static int			 debug = 0;

typedef struct {
	union {
		int64_t			 number;
		char			*string;
		struct application	*app;
		int			 hashtype;
		struct {
			int		 type;
			char		 value[MAX_APN_HASH_LEN];
		} hashspec;
	} v;
	int lineno;
} YYSTYPE;
#define YYSTYPE_IS_DECLARED

%}

%token	ALLOW DENY ALF SFS SB VS CAP CONTEXT RAW ALL OTHER LOG CONNECT ACCEPT
%token	INET INET6 FROM TO PORT ANY SHA256 TCP UDP DEFAULT NEW ASK NOT
%token	READ WRITE EXEC CHMOD ERROR APPLICATION RULE HOST TFILE
%token	<v.string>		STRING
%token	<v.number>		NUMBER
%type	<v.app>			app
%type	<v.hashtype>		hashtype
%type	<v.hashspec>		hashspec
%%

grammar		: /* empty */
		| grammar '\n'
		| grammar modules '\n'
		| grammar varset '\n'
		| grammar error '\n'		{ file->errors++; }
		;

varset		: varapp
		| varrules
		| vardefault
		| varhost
		| varport
		| varfilename
		;

varapp		: APPLICATION STRING '=' app		{
			if (varset($2, NULL, 0, 0) == -1)
				YYERROR;
		}
		;

vardefault	: DEFAULT STRING '=' action		{
			if (varset($2, NULL, 0, 0) == -1)
				YYERROR;
		}
		;

varrules	: varalfrule
		| varsfsrule
		;

varalfrule	: RULE STRING '=' alfspecs		{
			if (varset($2, NULL, 0, 0) == -1)
				YYERROR;
		}
		;

varsfsrule	: RULE STRING '=' sfsspec		{
			if (varset($2, NULL, 0, 0) == -1)
				YYERROR;
		}
		;

varhost		: HOST STRING '=' hostspec		{
			if (varset($2, NULL, 0, 0) == -1)
				YYERROR;
		}
		;

varport		: PORT STRING '=' ports			{
			if (varset($2, NULL, 0, 0) == -1)
				YYERROR;
		}
		;

varfilename	: TFILE STRING '=' STRING		{
			if (varset($2, NULL, 0, 0) == -1)
				YYERROR;
		}
		;

comma		: ','
		;

optnl		: '\n' optnl
		| /*empty */
		;

nl		: '\n' optnl		/* one newline or more */
		;

modules		: module_l
		;

module_l	: module_l module
		| module
		;

module		: alfmodule
		| sfsmodule
		| sbmodule
		| vsmodule
		;

		/*
		 * ALF
		 */
alfmodule	: ALF optnl '{' optnl alfruleset_l '}'
		;

alfruleset_l	: alfruleset_l alfruleset
		| alfruleset
		;

alfruleset	: app_l				{
		} optnl '{' optnl alfrules '}' nl
		;

alfrules	: alfrule_l
		;

alfrule_l	: alfrule_l alfrule nl
		| alfrule nl
		;

alfrule		: alffilterrule
		| alfcaprule
		| alfdefault
		| ctxrule
		;

alfspecs	: alffilterspec
		| capability
		| ctxspec
		;

alffilterrule	: action alffilterspec
		;

alffilterspec	: netaccspec log af proto hosts
		| '$' STRING			{
			struct var	*var;

			if ((var = varget($2)) == NULL)
				YYERROR;
		}
		;

netaccspec	: '{' netaccess_l '}'
		| netaccess
		| ANY
		;

netaccess_l	: netaccess_l comma netaccess
		| netaccess
		;

netaccess	: CONNECT
		| ACCEPT
		;

af		: INET
		| INET6
		| /* empty */
		;

proto		: UDP
		| TCP
		;

hosts		: FROM hostspec portspec TO hostspec portspec
		| ALL
		;

hostspec	: '{' host_l '}'
		| host
		| ANY
		| '$' STRING			{
			struct var	*var;

			if ((var = varget($2)) == NULL)
				YYERROR;
		}
		;

host_l		: host_l comma host
		| host
		;

host		: not address
		;

address		: STRING
		| STRING '/' NUMBER
		;

portspec	: PORT ports
		| /* empty */
		;

ports		: '{' port_l '}'
		| port
		| '$' STRING			{
			struct var	*var;

			if ((var = varget($2)) == NULL)
				YYERROR;
		}
		;

port_l		: port_l comma port
		| port
		;

port		: portval
		;

portval		: STRING
		| NUMBER
		;

alfcaprule	: action capability
		;

capability	: RAW
		| OTHER
		| ALL
		;

alfdefault	: defaultrule;
		;

		/*
		 * SFS
		 */
sfsmodule	: SFS optnl '{' optnl sfsruleset_l '}'
		;

sfsruleset_l	: sfsruleset_l sfsruleset
		| sfsruleset
		;

sfsruleset	: app_l				{
		} optnl '{' optnl sfsrules '}' nl
		;
		;

sfsrule_l	: sfsrule_l sfsrule nl
		| sfsrule nl
		;

sfsrules	: sfsrule_l
		;

sfsrule		: sfscheckrule
		| sfsdefault
		;

sfscheckrule	: action sfsspec
		;

sfsspec		: STRING
		;

sfsdefault	: defaultrule
		;

		/*
		 * SB
		 */
sbmodule	: SB optnl '{' optnl sbruleset_l '}'
		;

sbruleset_l	: sbruleset_l sbruleset
		| sbruleset
		;

sbruleset	: app_l				{
		} optnl '{' optnl sbrules '}' nl
		;

sbrules		: sbrule_l
		;

sbrule_l	: sbrule_l sbrule nl
		| sbrule nl
		;

sbrule		: sbfilterrule
		| sbdefaultrule
		| ctxrule
		;

sbfilterrule	: action sbfilterspec
		;

sbfilterspec	: fsaccspec log pathspec

fsaccspec	: '{' faccess_l '}'
		| faccess
		| ANY
		;

faccess_l	: faccess_l comma faccess
		| faccess
		;

faccess		: READ
		| WRITE
		| EXEC
		| CHMOD
		;

pathspec	: '{' path_l '}'
		| path
		| ANY
		;

path_l		: path_l comma path
		| path
		;

path		: STRING
		;

sbdefaultrule	: defaultrule
		;

		/*
		 * VS
		 */
vsmodule	: VS optnl '{' optnl vsruleset_l '}'
		;

vsruleset_l	: vsruleset_l vsruleset
		| vsruleset
		;

vsruleset	: app_l				{
		} optnl '{' optnl vsrules '}' nl
		;

vsrules		: vsrule_l

vsrule_l	: vsrule_l vsrule nl
		| vsrule nl
		;

vsrule		: action vsspec
		;

vsspec		: STRING
		;

		/*
		 * Context
		 */
ctxrule		: CONTEXT ctxspec
		;

ctxspec		: NEW app_l
		;

		/*
		 * Default rule
		 */
defaultrule	: DEFAULT defaultspec
		;

defaultspec	: action
		| '$' STRING			{
			struct var	*var;

			if ((var = varget($2)) == NULL)
				YYERROR;
		}
		;

		/*
		 * Common elements
		 */
app_l		: app_l comma optnl app
		| app
		| ANY
		;

app		: STRING hashspec		{
			struct application	*app;

			if ((app = calloc(1, sizeof(struct application)))
			    == NULL) {
				free($1);
				YYERROR;
			}
			if ((app->name = strdup($1)) == NULL) {
				free($1);
				free(app);
				YYERROR;
			}
			app->hashtype = $2.type;
			bcopy($2.value, app->hashvalue, sizeof(app->hashvalue));

			free($1);

			$$ = app;
		}
		| '$' STRING			{
			struct var	*var;

			if ((var = varget($2)) == NULL) {
				free($2);
				YYERROR;
			}

			free($2);
		}
		;

hashspec	: hashtype STRING		{

			if (validate_hash($1, $2)) {
				free($2);
				YYERROR;
			}
			$$.type = $1;

			if (str2hash($2, $$.value, sizeof($$.value)) == -1) {
				free($2);
				YYERROR;
			}

			free($2);
		}
		;

hashtype	: SHA256			{ $$ = APN_HASH_SHA256; }
		| /* empty */			{ $$ = APN_HASH_SHA256; }
		;

action		: ALLOW
		| DENY
		| ASK
		;

log		: LOG
		| /* empty */
		;

not		: NOT
		| /* empty */
		;
%%

struct keywords {
	const char	*k_name;
	int		 k_val;
};

int
yyerror(const char *fmt, ...)
{
	va_list		 ap;

	file->errors++;
	va_start(ap, fmt);
	fprintf(stderr, "%s: %d: ", file->name, yylval.lineno);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	return (0);
}

int
kw_cmp(const void *k, const void *e)
{
	return (strcmp(k, ((const struct keywords *)e)->k_name));
}

int
lookup(char *s)
{
	/* this has to be sorted always */
	static const struct keywords keywords[] = {
		{ "accept",	ACCEPT },
		{ "alf",	ALF },
		{ "all",	ALL },
		{ "allow",	ALLOW },
		{ "any",	ANY },
		{ "application", APPLICATION },
		{ "ask",	ASK },
		{ "cap",	CAP },
		{ "chmod",	CHMOD },
		{ "connect",	CONNECT },
		{ "context",	CONTEXT },
		{ "default",	DEFAULT },
		{ "deny",	DENY },
		{ "exec",	EXEC },
		{ "file",	TFILE },
		{ "from",	FROM },
		{ "host",	HOST },
		{ "inet",	INET },
		{ "inet6",	INET6 },
		{ "log",	LOG },
		{ "new",	NEW },
		{ "not",	NOT },
		{ "other",	OTHER },
		{ "port",	PORT },
		{ "raw",	RAW },
		{ "read",	READ },
		{ "rule",	RULE },
		{ "sb",		SB },
		{ "sfs",	SFS },
		{ "sha256",	SHA256 },
		{ "tcp",	TCP },
		{ "to",		TO },
		{ "udp",	UDP },
		{ "vs",		VS },
		{ "write",	WRITE },
	};
	const struct keywords	*p;

	p = bsearch(s, keywords, sizeof(keywords)/sizeof(keywords[0]),
	    sizeof(keywords[0]), kw_cmp);

	if (p) {
		if (debug > 1)
			fprintf(stderr, "%s: %d\n", s, p->k_val);
		return (p->k_val);
	} else {
		if (debug > 1)
			fprintf(stderr, "string: %s\n", s);
		return (STRING);
	}
}

#define MAXPUSHBACK	128

char	*parsebuf;
int	 parseindex;
char	 pushback_buffer[MAXPUSHBACK];
int	 pushback_index = 0;

int
lgetc(int quotec)
{
	int		c, next;

	if (parsebuf) {
		/* Read character from the parsebuffer instead of input. */
		if (parseindex >= 0) {
			c = parsebuf[parseindex++];
			if (c != '\0')
				return (c);
			parsebuf = NULL;
		} else
			parseindex++;
	}

	if (pushback_index)
		return (pushback_buffer[--pushback_index]);

	if (quotec) {
		if ((c = getc(file->stream)) == EOF) {
			yyerror("reached end of file while parsing quoted "
			    "string");
			if (popfile() == EOF)
				return (EOF);
			return (quotec);
		}
		return (c);
	}

	while ((c = getc(file->stream)) == '\\') {
		next = getc(file->stream);
		if (next != '\n') {
			c = next;
			break;
		}
		yylval.lineno = file->lineno;
		file->lineno++;
	}

	while (c == EOF) {
		if (popfile() == EOF)
			return (EOF);
		c = getc(file->stream);
	}
	return (c);
}

int
lungetc(int c)
{
	if (c == EOF)
		return (EOF);
	if (parsebuf) {
		parseindex--;
		if (parseindex >= 0)
			return (c);
	}
	if (pushback_index < MAXPUSHBACK-1)
		return (pushback_buffer[pushback_index++] = c);
	else
		return (EOF);
}

int
findeol(void)
{
	int	c;

	parsebuf = NULL;
	pushback_index = 0;

	/* skip to either EOF or the first real EOL */
	while (1) {
		c = lgetc(0);
		if (c == '\n') {
			file->lineno++;
			break;
		}
		if (c == EOF)
			break;
	}
	return (ERROR);
}

int
yylex(void)
{
	char	 buf[8096];
	char	*p;
	int	 quotec, next, c;
	int	 token;

	p = buf;
	while ((c = lgetc(0)) == ' ' || c == '\t')
		; /* nothing */

	yylval.lineno = file->lineno;
	if (c == '#')
		while ((c = lgetc(0)) != '\n' && c != EOF)
			; /* nothing */

	switch (c) {
	case '\'':
	case '"':
		quotec = c;
		while (1) {
			if ((c = lgetc(quotec)) == EOF)
				return (0);
			if (c == '\n') {
				file->lineno++;
				continue;
			} else if (c == '\\') {
				if ((next = lgetc(quotec)) == EOF)
					return (0);
				if (next == quotec || c == ' ' || c == '\t')
					c = next;
				else if (next == '\n')
					continue;
				else
					lungetc(next);
			} else if (c == quotec) {
				*p = '\0';
				break;
			}
			if (p + 1 >= buf + sizeof(buf) - 1) {
				yyerror("string too long");
				return (findeol());
			}
			*p++ = (char)c;
		}
		yylval.v.string = strdup(buf);
		if (yylval.v.string == NULL)
			err(1, "yylex: strdup");
		return (STRING);
	}

#define allowed_to_end_number(x) \
	(isspace(x) || x == ')' || x ==',' || x == '/' || x == '}' || x == '=')

	if (c == '-' || isdigit(c)) {
		do {
			*p++ = c;
			if ((unsigned)(p-buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(0)) != EOF && isdigit(c));
		lungetc(c);
		if (p == buf + 1 && buf[0] == '-')
			goto nodigits;
		if (c == EOF || allowed_to_end_number(c)) {
			const char *errstr = NULL;

			*p = '\0';
			yylval.v.number = strtonum(buf, LLONG_MIN,
			    LLONG_MAX, &errstr);
			if (errstr) {
				yyerror("\"%s\" invalid number: %s",
				    buf, errstr);
				return (findeol());
			}
			return (NUMBER);
		} else {
nodigits:
			while (p > buf + 1)
				lungetc(*--p);
			c = *--p;
			if (c == '-')
				return (c);
		}
	}

#define allowed_in_string(x) \
	(isalnum(x) || (ispunct(x) && x != '(' && x != ')' && \
	x != '{' && x != '}' && x != '<' && x != '>' && \
	x != '!' && x != '=' && x != '#' && x != ','))

	if (isalnum(c) || c == '/') {
		do {
			*p++ = c;
			if ((unsigned)(p-buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(0)) != EOF && (allowed_in_string(c)));
		lungetc(c);
		*p = '\0';
		if ((token = lookup(buf)) == STRING)
			if ((yylval.v.string = strdup(buf)) == NULL)
				err(1, "yylex: strdup");
		return (token);
	}
	if (c == '\n') {
		yylval.lineno = file->lineno;
		file->lineno++;
	}
	if (c == EOF)
		return (0);
	return (c);
}

int
check_file_secrecy(int fd, const char *fname)
{
	struct stat	st;

	if (fstat(fd, &st)) {
		warn("cannot stat %s", fname);
		return (-1);
	}
	if (st.st_uid != 0 && st.st_uid != getuid()) {
		warnx("%s: owner not root or current user", fname);
		return (-1);
	}
	if (st.st_mode & (S_IRWXG | S_IRWXO)) {
		warnx("%s: group/world readable/writeable", fname);
		return (-1);
	}
	return (0);
}

struct file *
pushfile(const char *name, int secret)
{
	struct file	*nfile;

	if ((nfile = calloc(1, sizeof(struct file))) == NULL ||
	    (nfile->name = strdup(name)) == NULL) {
		warn("malloc");
		return (NULL);
	}
	if (TAILQ_FIRST(&files) == NULL && strcmp(nfile->name, "-") == 0) {
		nfile->stream = stdin;
		free(nfile->name);
		if ((nfile->name = strdup("stdin")) == NULL) {
			warn("strdup");
			free(nfile);
			return (NULL);
		}
	} else if ((nfile->stream = fopen(nfile->name, "r")) == NULL) {
		warn("%s", nfile->name);
		free(nfile->name);
		free(nfile);
		return (NULL);
	} else if (secret &&
	    check_file_secrecy(fileno(nfile->stream), nfile->name)) {
		fclose(nfile->stream);
		free(nfile->name);
		free(nfile);
		return (NULL);
	}
	nfile->lineno = 1;
	TAILQ_INSERT_TAIL(&files, nfile, entry);
	return (nfile);
}

int
popfile(void)
{
	struct file	*prev;

	if ((prev = TAILQ_PREV(file, files, entry)) != NULL) {
		prev->errors += file->errors;
		TAILQ_REMOVE(&files, file, entry);
		fclose(file->stream);
		free(file->name);
		free(file);
		file = prev;
		return (0);
	}
	return (EOF);
}

int
parse_rules(const char *filename, struct apnruleset *apnrspx)
{
	int		 errors = 0;

	apnrsp = apnrspx;

	if ((file = pushfile(filename, 1)) == NULL) {
		return (-1);
	}

	yyparse();
	errors = file->errors;
	popfile();

	return (errors ? -1 : 0);
}

/*
 * Add an opaque object of a designated size and a specified type to the
 * variable structure.  The variable is identified by the specified name.
 *
 * XXX HJH If the variable already exists it will be replaced.
 * XXX HJH Good idea?
 */
int
varset(const char *name, void *value, size_t valsize, int type)
{
	struct var	*var;

	for (var = TAILQ_FIRST(&apnrsp->var_queue);
	    var && strcmp(name, var->name); var = TAILQ_NEXT(var, entry))
		;	/* nothing */

	if (var != NULL) {
		free(var->name);
		free(var->value); /* XXX HJH more needed for complex structs! */
		TAILQ_REMOVE(&apnrsp->var_queue, var, entry);
		free(var);
	}
	if ((var = calloc(1, sizeof(*var))) == NULL)
		return (-1);

	var->name = strdup(name);
	if (var->name == NULL) {
		free(var);
		return (-1);
	}
	var->value = value;
	var->valsize = valsize;
	var->used = 0;
	var->type = type;

	TAILQ_INSERT_TAIL(&apnrsp->var_queue, var, entry);

	return (0);
}

struct var *
varget(const char *name)
{
	struct var	*var;

	TAILQ_FOREACH(var, &apnrsp->var_queue, entry)
		if (strcmp(name, var->name) == 0) {
			var->used = 1;
			return (var);
		}
	return (NULL);
}

int
str2hash(const char *s, char *dest, size_t max_len)
{
	unsigned	i;
	char		t[3];

	if (strlen(s) / 2 > max_len) {
		yyerror("hash too long");
		return (-1);
	}

	if (strlen(s) % 2) {
		yyerror("hash must be of even length");
		return (-1);
	}

	for (i = 0; i < strlen(s) / 2; i++) {
		t[0] = s[2 * i];
		t[1] = s[2 * i + 1];
		t[2] = 0;
		if (!isxdigit(t[0]) || !isxdigit(t[1])) {
			yyerror("hash must be specified in hex");
			return (-1);
		}
		dest[i] = strtoul(t, NULL, 16);
	}

	return (0);
}

int
validate_hash(int type, const char *s)
{
	switch (type) {
	case APN_HASH_SHA256:
		if (strlen(s) != APN_HASH_SHA256_LEN * 2) {
			yyerror("wrong hash length");
			return (-1);
		}
		break;
	default:
		yyerror("unknown hash type %d", type);
		return (-1);
	}

	return (0);
}

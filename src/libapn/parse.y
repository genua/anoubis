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
#include <sys/socket.h>
#ifndef LINUX
#include <sys/queue.h>
#else
#include <queue.h>
#endif	/* !LINUX */
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#ifdef LINUX
#include <bsdcompat.h>
#endif	/* LINUX */

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
int		 host(const char *, struct apn_addr *);
int		 host_v4(const char *, struct apn_addr *);
int		 host_v6(const char *, struct apn_addr *);
int		 validate_host(const struct apn_host *);
int		 portbyname(const char *, u_int16_t *);
int		 portbynumber(int64_t, u_int16_t *);
int		 validate_alffilterspec(struct apn_afiltspec *);
int		 validate_hostlist(struct apn_host *);
void		 freehost(struct apn_host *);
void		 freeport(struct apn_port *);
void		 clearfilter(struct apn_afiltspec *);
void		 clearapp(struct apn_app *);
void		 clearalfrule(struct apn_alfrule *);

static struct apn_ruleset	*apnrsp = NULL;
static int			 debug = 0;

typedef struct {
	union {
		int64_t			 number;
		char			*string;
		int			 hashtype;
		int			 netaccess;
		int			 af;
		int			 proto;
		int			 action;
		int			 log;
		struct {
			int		 type;
			char		 value[MAX_APN_HASH_LEN];
		} hashspec;
		struct {
			struct apn_host	*fromhost;
			struct apn_port	*fromport;
			struct apn_host	*tohost;
			struct apn_port	*toport;
		} hosts;
		struct apn_afiltspec	 afspec;
		struct apn_afiltrule	 afrule;
		struct apn_acaprule	 acaprule;
		struct apn_default	 dfltrule;
		struct apn_context	 ctxrule;
		struct apn_addr		 addr;
		struct apn_app		*app;
		struct apn_port		*port;
		struct apn_host		*host;
		struct apn_alfrule	*alfrule;
		struct apn_rule		*ruleset;
	} v;
	int lineno;
} YYSTYPE;
#define YYSTYPE_IS_DECLARED

%}

%token	ALLOW DENY ALF SFS SB VS CAP CONTEXT RAW ALL OTHER LOG CONNECT ACCEPT
%token	INET INET6 FROM TO PORT ANY SHA256 TCP UDP DEFAULT NEW ASK ALERT
%token	READ WRITE EXEC CHMOD ERROR APPLICATION RULE HOST TFILE
%token	<v.string>		STRING
%token	<v.number>		NUMBER
%type	<v.app>			app app_l apps
%type	<v.hashtype>		hashtype
%type	<v.hashspec>		hashspec
%type	<v.addr>		address
%type	<v.host>		host host_l hostspec
%type	<v.port>		port port_l ports portspec
%type	<v.hosts>		hosts
%type	<v.number>		not capability defaultspec
%type	<v.netaccess>		netaccess
%type	<v.af>			af
%type	<v.proto>		proto
%type	<v.afspec>		alffilterspec
%type	<v.action>		action
%type	<v.log>			log
%type	<v.afrule>		alffilterrule
%type	<v.alfrule>		alfrule alfrule_l
%type	<v.ruleset>		alfruleset
%type	<v.acaprule>		alfcaprule
%type	<v.dfltrule>		defaultrule alfdefault sfsdefault sbdefault;
%type	<v.ctxrule>		ctxrule
%%

grammar		: /* empty */
		| grammar '\n'
		| grammar module_l '\n'
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

varapp		: APPLICATION STRING '=' apps		{
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
			if (varset($2, NULL, 0, 0) == -1) {
				free($2);
				YYERROR;
			}
			free($2);
		}
		;

varsfsrule	: RULE STRING '=' sfsspec		{
			if (varset($2, NULL, 0, 0) == -1)
				YYERROR;
		}
		;

varhost		: HOST STRING '=' hostspec		{
			if (validate_hostlist($4) == -1) {
				free($2);
				YYERROR;
			}
			if (varset($2, NULL, 0, 0) == -1) {
				free($2);
				YYERROR;
			}
			free($2);
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

alfruleset	: apps optnl '{' optnl alfrule_l '}' nl {
			struct apn_rule	*rule;

			if ((rule = calloc(1, sizeof(struct apn_rule)))
			    == NULL) {
				clearapp($1);
				clearalfrule($5);
				YYERROR;
			}

			rule->app = $1;
			rule->rule.alf = $5;
			rule->tail = rule;
			rule->type = APN_ALF;

			if (apn_add_alfrule(rule, apnrsp) != 0) {
				YYERROR;
			}
		}
		;

alfrule_l	: alfrule_l alfrule nl		{
			if ($2 == NULL)
				$$ = $1;
			else if ($1 == NULL)
				$$ = $2;
			else {
				$1->tail->next = $2;
				$1->tail = $2->tail;
				$$ = $1;
			}
		}
		| alfrule nl			{ $$ = $1; }
		;

alfrule		: alffilterrule			{
			struct apn_alfrule	*rule;

			rule = calloc(1, sizeof(struct apn_alfrule));
			if (rule == NULL) {
				clearfilter(&$1.filtspec);
				YYERROR;
			}

			rule->type = APN_ALF_FILTER;
			rule->rule.afilt = $1;
			rule->tail = rule;

			$$ = rule;
		}
		| alfcaprule			{
			struct apn_alfrule	*rule;

			rule = calloc(1, sizeof(struct apn_alfrule));
			if (rule == NULL)
				YYERROR;

			rule->type = APN_ALF_CAPABILITY;
			rule->rule.acap = $1;
			rule->tail = rule;

			$$ = rule;
		}
		| alfdefault			{
			struct apn_alfrule	*rule;

			rule = calloc(1, sizeof(struct apn_alfrule));
			if (rule == NULL)
				YYERROR;

			rule->type = APN_ALF_DEFAULT;
			rule->rule.apndefault = $1;
			rule->tail = rule;

			$$ = rule;
		}
		| ctxrule			{
			struct apn_alfrule	*rule;

			rule = calloc(1, sizeof(struct apn_alfrule));
			if (rule == NULL)
				YYERROR;

			rule->type = APN_ALF_CTX;
			rule->rule.apncontext = $1;
			rule->tail = rule;

			$$ = rule;
		}
		;

alfspecs	: alffilterspec
		| capability
		;

alffilterrule	: action alffilterspec		{
			$$.action = $1;
			$$.filtspec = $2;
		}
		;

alffilterspec	: netaccess log af proto hosts	{
			$$.log = $2;
			$$.af = $3;
			$$.proto = $4;
			$$.fromhost = $5.fromhost;
			$$.fromport = $5.fromport;
			$$.tohost = $5.tohost;
			$$.toport = $5.toport;

			if (validate_alffilterspec(&$$) == -1) {
				clearfilter(&$$);
				YYERROR;
			}
		}
		| '$' STRING			{
			struct var	*var;

			if ((var = varget($2)) == NULL)
				YYERROR;
		}
		;

netaccess	: CONNECT			{ $$ = APN_CONNECT; }
		| ACCEPT			{ $$ = APN_ACCEPT; }
		;

af		: INET				{ $$ = AF_INET; }
		| INET6				{ $$ = AF_INET6; }
		| /* empty */			{ $$ = AF_UNSPEC; }
		;

proto		: UDP				{ $$ = IPPROTO_UDP; }
		| TCP				{ $$ = IPPROTO_TCP; }
		;

hosts		: FROM hostspec portspec TO hostspec portspec	{
			$$.fromhost = $2;
			$$.fromport = $3;
			$$.tohost = $5;
			$$.toport = $6;
		}
		| ALL				{
			bzero(&$$, sizeof($$));
		}
		;

hostspec	: '{' host_l '}'		{ $$ = $2; }
		| host				{ $$ = $1; }
		| ANY				{ $$ = NULL; }
		| '$' STRING			{
			struct var	*var;

			if ((var = varget($2)) == NULL)
				YYERROR;
		}
		;

host_l		: host_l comma host		{
			if ($3 == NULL)
				$$ = $1;
			else if ($1 == NULL)
				$$ = $3;
			else {
				$1->tail->next = $3;
				$1->tail = $3->tail;
				$$ = $1;
			}
		}
		| host				{ $$ = $1; }
		;

host		: not address			{
			struct apn_host	*host;

			host = calloc(1, sizeof(struct apn_host));
			if (host == NULL)
				YYERROR;

			host->not = $1;
			host->addr = $2;
			host->tail = host;

			$$ = host;
		}
		;

address		: STRING			{
			if (!host($1, &$$)) {
				free($1);
				yyerror("could not parse address");
				YYERROR;
			}
			free($1);
		}

portspec	: PORT ports			{ $$ = $2; }
		| /* empty */			{ $$ = NULL; }
		;

ports		: '{' port_l '}'		{ $$ = $2; }
		| port				{ $$ = $1; }
		| '$' STRING			{
			struct var	*var;

			if ((var = varget($2)) == NULL)
				YYERROR;
		}
		;

port_l		: port_l comma port		{
			if ($3 == NULL)
				$$ = $1;
			else if ($1 == NULL)
				$$ = $3;
			else {
				$1->tail->next = $3;
				$1->tail = $3->tail;
				$$ = $1;
			}
		}
		| port				{ $$ = $1; }
		;

port		: STRING			{
			struct apn_port	*port;

			port = calloc(1, sizeof(struct apn_port));
			if (port == NULL)
				YYERROR;

			port->tail = port;

			if (portbyname($1, &port->port) == -1) {
				free($1);
				free(port);
				YYERROR;
			}
			free($1);

			$$ = port;
		}
		| NUMBER			{
			struct apn_port	*port;

			port = calloc(1, sizeof(struct apn_port));
			if (port == NULL)
				YYERROR;

			port->tail = port;

			if (portbynumber($1, &port->port) == -1) {
				free(port);
				YYERROR;
			}

			$$ = port;
		}
		;

alfcaprule	: action capability		{
			$$.action = $1;
			$$.capability = $2;
		}
		;

capability	: RAW				{ $$ = APN_ALF_CAPRAW; }
		| OTHER				{ $$ = APN_ALF_CAPOTHER; }
		| ALL				{ $$ = APN_ALF_CAPALL; }
		;

alfdefault	: defaultrule			{ $$ = $1; }
		;

		/*
		 * SFS
		 */
sfsmodule	: SFS optnl '{' optnl sfsruleset_l '}'
		;

sfsruleset_l	: sfsruleset_l sfsruleset
		| sfsruleset
		;

sfsruleset	: apps				{
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

sbruleset	: apps				{
		} optnl '{' optnl sbrules '}' nl
		;

sbrules		: sbrule_l
		;

sbrule_l	: sbrule_l sbrule nl
		| sbrule nl
		;

sbrule		: sbfilterrule
		| sbdefault
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

sbdefault	: defaultrule
		;

		/*
		 * VS
		 */
vsmodule	: VS optnl '{' optnl vsruleset_l '}'
		;

vsruleset_l	: vsruleset_l vsruleset
		| vsruleset
		;

vsruleset	: apps				{
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
ctxrule		: CONTEXT NEW apps		{
			$$.application = $3;
		}
		;

		/*
		 * Default rule
		 */
defaultrule	: DEFAULT defaultspec		{
			$$.action = $2;
		}
		;


defaultspec	: action			{ $$ = $1; }
		| '$' STRING			{
			struct var      *var;
			if ((var = varget($2)) == NULL)
				YYERROR;
		}
		;

		/*
		 * Common elements
		 */
apps		: '{' app_l '}'			{ $$ = $2; }
		| app				{ $$ = $1; }
		| ANY				{ $$ = NULL; }
		| '$' STRING			{
			struct var	*var;

			if ((var = varget($2)) == NULL)
				YYERROR;
		}
		;

app_l		: app_l comma optnl app		{
			if ($4 == NULL)
				$$ = $1;
			else if ($1 == NULL)
				$$ = $4;
			else {
				$1->tail->next = $4;
				$1->tail = $4->tail;
				$$ = $1;
			}
		}
		| app				{ $$ = $1; }
		;

app		: STRING hashspec		{
			struct apn_app	*app;

			if ((app = calloc(1, sizeof(struct apn_app)))
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
			app->tail = app;

			free($1);

			$$ = app;
		}
		;

hashspec	: hashtype STRING		{
			if (validate_hash($1, $2) == -1) {
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

action		: ALLOW				{ $$ = APN_ACTION_ALLOW; }
		| DENY				{ $$ = APN_ACTION_DENY; }
		| ASK				{ $$ = APN_ACTION_ASK; }
		;

log		: LOG				{ $$ = APN_LOG_NORMAL; }
		| ALERT				{ $$ = APN_LOG_ALERT; }
		| /* empty */			{ $$ = APN_LOG_NONE; }
		;

not		: '!'				{ $$ = 1; }
		| /* empty */			{ $$ = 0; }
		;
%%

struct keywords {
	const char	*k_name;
	int		 k_val;
};

int
yyerror(const char *fmt, ...)
{
	struct apn_errmsg	*msg;
	va_list			 ap;
	char			*s1, *s2;

	if ((msg = calloc(1, sizeof(struct apn_errmsg))) == NULL)
		return (-1);

	file->errors++;

	va_start(ap, fmt);
	if (vasprintf(&s1, fmt, ap) == -1) {
		free(msg);
		return (-1);
	}
	va_end(ap);

	if (asprintf(&s2, "%s: %d: %s", file->name, yylval.lineno, s1) == -1) {
		free(msg);
		free(s1);
		return (-1);
	}
	free(s1);

	msg->msg = s2;
	TAILQ_INSERT_TAIL(&apnrsp->err_queue, msg, entry);

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
		{ "alert",	ALERT },
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

	if (isalnum(c) || c == ':' || c == '/') {
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
parse_rules(const char *filename, struct apn_ruleset *apnrspx)
{
	int errors = 0;

	apnrsp = apnrspx;

	if ((file = pushfile(filename, 1)) == NULL) {
		return (-1);
	}

	yyparse();
	errors = file->errors;
	popfile();

	if (errors) {
		/* XXX clean up rule sets */
	}

	fclose(file->stream);
	free(file->name);
	free(file);

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

/*
 * Verify a hash has a size matching its type.
 */
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

/*
 * Regarding host_v[46]():
 * As discussed with mpf and mfriedl this is the way to go right now.
 * In a long term a solution based on solely getaddrinfo(3) might be better.
 */
int
host(const char *s, struct apn_addr *h)
{
	int done = 0;

	bzero(h, sizeof(struct apn_addr));

	/* IPv4 address? */
	done = host_v4(s, h);

	if (!done)
		done = host_v6(s, h);

	return (done);
}

int
host_v4(const char *s, struct apn_addr *h)
{
	struct in_addr	 ina;
	int		 bits = 32;

	bzero(&ina, sizeof(struct in_addr));
	if (strrchr(s, '/') != NULL) {
		if ((bits = inet_net_pton(AF_INET, s, &ina, sizeof(ina))) == -1)
			return (0);
	} else {
		if (inet_pton(AF_INET, s, &ina) != 1)
			return (0);
	}

	h->af = AF_INET;
	h->apa.v4.s_addr = ina.s_addr;
	h->len = bits;

	return (1);
}

int
host_v6(const char *s, struct apn_addr *h)
{
	char		*p, *ps;
	const char	*errstr;
	int		 bits = 128;
	struct addrinfo	hints, *res;

	if ((p = strrchr(s, '/')) != NULL) {
		bits = strtonum(p + 1, 0, 128, &errstr);
		if (errstr) {
			yyerror("prefixlen is %s: %s", errstr, p + 1);
			return (-1);
		}
		if ((ps = malloc(strlen(s) - strlen(p) + 1)) == NULL)
			return (-1);
		strlcpy(ps, s, strlen(s) - strlen(p) + 1);
	} else {
		if ((ps = strdup(s)) == NULL)
			return (-1);
	}

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;	/* dummy */
	hints.ai_flags = AI_NUMERICHOST;
	if (getaddrinfo(ps, NULL, &hints, &res) == 0) {
		h->af = AF_INET6;
		bcopy(&((struct sockaddr_in6 *)res->ai_addr)->sin6_addr,
		    &h->apa.v6, sizeof(struct sockaddr_in6));
		h->len = bits;

		freeaddrinfo(res);
		free(ps);
		return (1);
	}

	free(ps);

	return (0);
}

/*
 * Verify that a host address has a sane address family, correct size and
 * '!'  flag.
 *
 * Returns -1 on failure, 0 otherwise.
 */
int
validate_host(const struct apn_host *host)
{
	switch (host->addr.af) {
	case AF_INET:
		if (host->addr.len > 32)
			return (-1);
		break;

	case AF_INET6:
		if (host->addr.len > 128)
			return (-1);
		break;

	default:
		return (-1);
	}

	if (!(0 <= host->not && host->not <= 1))
		return (-1);

	return (0);
}

int
portbyname(const char *ports, u_int16_t *portp)
{
	struct servent	*s;

	if ((s = getservbyname(ports, "tcp")) == NULL &&
	    (s = getservbyname(ports, "udp")) == NULL) {
		yyerror("could not parse port: %s", ports);
		return (-1);
	}

	*portp = s->s_port;

	return (0);
}

int
portbynumber(int64_t port, u_int16_t *portp)
{
	if (!(0 <= port && port <= USHRT_MAX)) {
		yyerror("port out of range: %ld", port);
		return (-1);
	}

	*portp = htons(port);

	return (0);
}

/*
 * Verify that a filterspecification uses the same address family
 * throughout, that only allowed protocols are specified and that host
 * addresses are sane.
 *
 * Returns -1 on failure, otherwise 0.
 */
int
validate_alffilterspec(struct apn_afiltspec *afspec)
{
	int	affrom, afto;

	if (!(APN_LOG_NONE <= afspec->log && afspec->log <= APN_LOG_ALERT))
		return (-1);

	if (!(APN_CONNECT <= afspec->netaccess && afspec->netaccess <=
	    APN_ACCEPT))
		return (-1);

	switch (afspec->af) {
	case AF_INET:
	case AF_INET6:
	case AF_UNSPEC:
		break;
	default:
		return (-1);
	}

	switch (afspec->proto) {
	case IPPROTO_UDP:
	case IPPROTO_TCP:
		break;
	default:
		return (-1);
	}

	if ((affrom = validate_hostlist(afspec->fromhost)) == -1)
		return (-1);
	if ((afto = validate_hostlist(afspec->tohost)) == -1)
		return (-1);

	switch (afspec->af) {
	case AF_UNSPEC:
		if (affrom == AF_UNSPEC || afto == AF_UNSPEC)
			break;
		if (affrom != afto) {
			yyerror("address family mismatch");
			return (-1);
		}
		break;
	case AF_INET:
		if (affrom == AF_INET6 || afto == AF_INET6) {
			yyerror("address family mismatch");
			return (-1);
		}
		break;
	case AF_INET6:
		if (affrom == AF_INET || afto == AF_INET) {
			yyerror("address family mismatch");
			return (-1);
		}
		break;
	default:
		/* NOTREACHED */
		return (-1);
	}

	return (0);
}

/*
 * Verfiy that a list of hosts is of the same address family, ie. either
 * only AF_INET or AF_INET6.  An empty list is allowed.  The list may be
 * just one single host.
 *
 * Returns -1 on failure, otherwise the address family is returned.
 */
int
validate_hostlist(struct apn_host *hostlist)
{
	struct apn_host	*hp;
	int		 inet, inet6;

	/* In case of "any". */
	if (hostlist == NULL)
		return (AF_UNSPEC);

	inet = inet6 = 0;
	for (hp = hostlist; hp; hp = hp->next) {
		if (validate_host(hp) == -1)
			return (-1);

		switch (hp->addr.af) {
		case AF_INET:
			inet++;
			break;

		case AF_INET6:
			inet6++;
			break;

		default:
			return (-1);
		}
	}

	if (inet != 0 && inet6 != 0) {
		yyerror("address family mismatch");
		return (-1);
	} else if (inet != 0)
		return (AF_INET);
	else
		return (AF_INET6);
}

void
freehost(struct apn_host *host)
{
	struct apn_host *hp, *tmp;

	if (host == NULL)
		return;

	hp = host;
	while (hp) {
		tmp = hp->next;
		free(hp);
		hp = tmp;
	}
}

void
freeport(struct apn_port *port)
{
	struct apn_port *hp, *tmp;

	if (port == NULL)
		return;

	hp = port;
	while (hp) {
		tmp = hp->next;
		free(hp);
		hp = tmp;
	}
}

void
clearfilter(struct apn_afiltspec *filtspec)
{
	if (filtspec == NULL)
		return;

	freehost(filtspec->fromhost);
	freehost(filtspec->tohost);
	freeport(filtspec->fromport);
	freeport(filtspec->toport);
}

void
clearapp(struct apn_app *app)
{
	struct apn_app *hp, *tmp;

	if (app == NULL)
		return;

	hp = app;
	while (hp) {
		tmp = hp->next;
		free(hp->name);
		free(hp);
		hp = tmp;
	}
}

void
clearalfrule(struct apn_alfrule *rule)
{
	struct apn_alfrule *hp, *tmp;

	if (rule == NULL)
		return;

	hp = rule;
	while (hp) {
		tmp = hp->next;

		switch (hp->type) {
		case APN_ALF_FILTER:
			clearfilter(&hp->rule.afilt.filtspec);
			break;

		case APN_ALF_CAPABILITY:
		case APN_ALF_DEFAULT:
		default:
			break;
		}

		free(hp);
		hp = tmp;
	}
}

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
#include <sys/uio.h>
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
#include <assert.h>

#ifdef LINUX
#include <bsdcompat.h>
#endif	/* LINUX */

#include "apn.h"
#include "apninternals.h"

#define APN_FILE	1
#define APN_IOVEC	2

static struct file {
	TAILQ_ENTRY(file)	 entry;
	int type;
	union {
		FILE			*u_stream;
		struct {
			struct iovec	*vec;
			int		 count;
			int		 cvec;
			int		 cidx;
		} u_iov;
	} u;
	char			*name;
	int			 lineno;
	int			 errors;
} *file;
TAILQ_HEAD(files, file)		 files;


int		 parse_rules(const char *, struct apn_ruleset *);
int		 parse_rules_iovec(const char *, struct iovec *, int count,
		     struct apn_ruleset *);
int		 __parse_rules_common(struct apn_ruleset *apnrspx);
struct file	*pushfile(const char *);
struct file	*pushiov(const char *, struct iovec *, int count);
int		 popfile(void);
int		 check_file_secrecy(int);
int		 yyparse(void);
int		 yylex(void);
int		 yyerror(const char *, ...);
int		 kw_cmp(const void *, const void *);
int		 lookup(char *);
int		 lgetc(int);
int		 llgetc(struct file *);
int		 lungetc(int);
int		 findeol(void);
int		 str2hash(const char *, u_int8_t *, size_t);
int		 host(const char *, struct apn_addr *);
int		 host_v4(const char *, struct apn_addr *);
int		 host_v6(const char *, struct apn_addr *);
int		 validate_host(const struct apn_host *);
int		 portbyname(const char *, u_int16_t *);
int		 portbynumber(int64_t, u_int16_t *);
int		 validate_alffilterspec(struct apn_afiltspec *);
int		 validate_hostlist(struct apn_host *);
char		*normalize_path(char *);

static struct apn_ruleset	*apnrsp = NULL;

struct hosthead {
	struct apn_host *head;
	struct apn_host *tail;
};

struct apphead {
	struct apn_app *head;
	struct apn_app *tail;
};

struct porthead {
	struct apn_port *head;
	struct apn_port *tail;
};

typedef struct {
	union {
		int64_t			 number;
		char			*string;
		unsigned int		 timeout;
		int			 netaccess;
		int			 proto;
		int			 action;
		int			 sfsaction;
		int			 log;
		struct {
			struct apn_host	*fromhost;
			struct apn_port	*fromport;
			struct apn_host	*tohost;
			struct apn_port	*toport;
		} hosts;
		struct apn_afiltspec	 afspec;
		struct apn_afiltrule	 afrule;
		struct apn_acaprule	 acaprule;
		struct apn_sbaccess	 sbaccess;
		struct apn_default	 dfltrule;
		struct apn_subject	 subject;
		struct apn_context	 ctxruleapps;
		struct apn_sfsaccess	 sfsaccess;
		struct apn_sfsdefault	 sfsdefault;
		struct apn_addr		 addr;
		struct apn_app		*app;
		struct apphead		 apphead;
		struct apn_port		*port;
		struct porthead		 porthead;
		struct apn_host		*host;
		struct hosthead		 hosthead;
		struct apn_rule		*rule;
		struct apn_chain	*rulehead;
		struct apn_scope	*scope;
	} v;
	int lineno;
} YYSTYPE;
#define YYSTYPE_IS_DECLARED

%}
%expect 0

%token	ALLOW DENY ALF SFS SB VS CAP CONTEXT RAW ALL OTHER LOG CONNECT ACCEPT
%token	INET INET6 FROM TO PORT ANY SHA256 TCP UDP DEFAULT NEW ASK ALERT OPEN
%token	READ WRITE EXEC CHMOD ERROR APPLICATION RULE HOST TFILE BOTH SEND
%token	RECEIVE TASK UNTIL COLON PATH KEY UID CSUM
%token	SELF SIGNEDSELF VALID INVALID UNKNOWN CONTINUE BORROW
%token	<v.string>		STRING
%destructor {
	free($$);
}				STRING
%token	<v.number>		NUMBER
%type	<v.app>			app apps
%destructor {
	apn_free_app($$);
}				app apps
%type	<v.apphead>		app_l
%destructor {
	apn_free_app($$.head);
}				app_l
%type	<v.addr>		address
%type	<v.host>		host hostspec
%destructor {
	apn_free_host($$);
}				host hostspec
%type	<v.hosthead>		host_l
%destructor {
	apn_free_host($$.head);
}				host_l
%type	<v.port>		port ports portspec
%destructor {
	apn_free_port($$);
}				port ports portspec
%type	<v.porthead>		port_l
%destructor {
	apn_free_port($$.head);
}				port_l
%type	<v.hosts>		hosts
%destructor {
	apn_free_port($$.fromport);
	apn_free_host($$.fromhost);
	apn_free_port($$.toport);
	apn_free_host($$.tohost);
}				hosts
%type	<v.number>		not capability defaultspec ruleid sbrwx
%type	<v.string>		sfspath
%destructor {
	free($$);
}				sfspath
%type	<v.netaccess>		netaccess
%type	<v.proto>		proto
%type	<v.afspec>		alffilterspec
%destructor {
	apn_free_filter(&$$);
}				alffilterspec
%type	<v.action>		action
%type	<v.log>			log
%type	<v.rule>		alfrule sfsrule sbrule ctxrule
%destructor {
	apn_free_one_rule($$, NULL);
}				alfrule sfsrule sbrule ctxrule
%type	<v.rulehead>		alfrule_l sfsrule_l sbrule_l ctxrule_l
%destructor {
	apn_free_chain($$, NULL);
	free($$);
}				alfrule_l sfsrule_l sbrule_l ctxrule_l
%type	<v.afrule>		alffilterrule
%destructor {
	apn_free_filter(&$$.filtspec);
}				alffilterrule
%type	<v.acaprule>		alfcaprule
%type	<v.sbaccess>		sbaccess sbpred sbpath sbuid sbkey
%destructor {
	apn_free_sbaccess(&$$);
}				sbaccess sbpred sbpath sbuid sbkey
%type	<v.dfltrule>		defaultrule alfdefault sbdefault
%type	<v.ctxruleapps>		ctxruleapps
%destructor {
	apn_free_app($$.application);
}				ctxruleapps
%type	<v.sfsaccess>		sfsaccessrule
%destructor {
	apn_free_sfsaccess(&$$);
}				sfsaccessrule
%type	<v.dfltrule>		sfsvalid sfsinvalid sfsunknown
%type	<v.subject>		sfssubject
%destructor {
	apn_free_subject(&$$);
}				sfssubject
%type	<v.sfsaction>		sfsaction
%type	<v.sfsdefault>		sfsdefaultrule
%destructor {
	apn_free_sfsdefault(&$$);
}				sfsdefaultrule
%type	<v.scope>		scope
%destructor {
	free($$);
}				scope
%%

grammar		: /* empty */
		| grammar '\n'
		| grammar module_l '\n'
		| grammar error '\n'		{ file->errors++; }
		;

comma		: ','
		;

minus		: '-'
		;

ruleid		: NUMBER COLON			{
			$$ = $1;
		}
		| /* empty */			{
			$$ = 0;
		}
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
		| ctxmodule
		;

		/*
		 * ALF
		 */
alfmodule	: ALF optnl '{' optnl alfruleset_l '}'
		| ALF optnl '{' optnl '}'
		;

alfruleset_l	: alfruleset_l alfruleset
		| alfruleset
		;

alfruleset	: ruleid apps optnl '{' optnl alfrule_l '}' nl {
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				apn_free_app($2);
				apn_free_chain($6, NULL);
				free($6);
				yyerror("Out of memory");
				YYERROR;
			}

			rule->app = $2;
			rule->scope = NULL;
			rule->apn_type = APN_ALF;
			rule->apn_id = $1;
			rule->userdata = NULL;
			rule->pchain = NULL;
			TAILQ_INIT(&rule->rule.chain);
			while(!TAILQ_EMPTY($6)) {
				struct apn_rule *arule;
				arule = TAILQ_FIRST($6);
				TAILQ_REMOVE($6, arule, entry);
				TAILQ_INSERT_TAIL(&rule->rule.chain,
				    arule, entry);
				arule->pchain = &rule->rule.chain;
			}
			free($6);

			if (apn_add_alfblock(apnrsp, rule, file->name,
			    yylval.lineno) != 0) {
				/* Account for errors in apn_add_alfblock */
				file->errors++;
				apn_free_one_rule(rule, NULL);
				YYERROR;
			}
		}
		;

alfrule_l	: alfrule_l alfrule nl		{
			if ($2 == NULL)
				$$ = $1;
			else {
				TAILQ_INSERT_TAIL($1, $2, entry);
				$2->pchain = $1;
				$$ = $1;
			}
		}
		| /* Empty */			{
			$$ = calloc(1, sizeof(struct apn_chain));
			if ($$ == NULL) {
				yyerror("Out of memory");
				YYERROR;
			}
			TAILQ_INIT($$);
		}
		;

scope		: /* Empty */ {
			$$ = NULL;
		}
		| TASK NUMBER {
			struct apn_scope *scope;

			if (apnrsp->flags & APN_FLAG_NOSCOPE) {
				yyerror("Scopes not permitted");
				YYERROR;
			}
			scope = calloc(1, sizeof(struct apn_scope));
			if (scope == NULL) {
				yyerror("Out of memory");
				YYERROR;
			}
			scope->task = $2;
			scope->timeout = 0;

			$$ = scope;
		}
		| TASK NUMBER UNTIL NUMBER {
			struct apn_scope *scope;

			if (apnrsp->flags & APN_FLAG_NOSCOPE) {
				yyerror("Scopes not permitted");
				YYERROR;
			}
			scope = calloc(1, sizeof(struct apn_scope));
			if (scope == NULL) {
				yyerror("Out of memory");
				YYERROR;
			}
			scope->task = $2;
			scope->timeout = $4;

			$$ = scope;
		}
		| UNTIL NUMBER TASK NUMBER {
			struct apn_scope *scope;

			if (apnrsp->flags & APN_FLAG_NOSCOPE) {
				yyerror("Scopes not permitted");
				YYERROR;
			}
			scope = calloc(1, sizeof(struct apn_scope));
			if (scope == NULL) {
				yyerror("Out of memory");
				YYERROR;
			}
			scope->task = $4;
			scope->timeout = $2;

			$$ = scope;
		}
		| UNTIL NUMBER {
			struct apn_scope *scope;

			if (apnrsp->flags & APN_FLAG_NOSCOPE) {
				yyerror("Scopes not permitted");
				YYERROR;
			}
			scope = calloc(1, sizeof(struct apn_scope));
			if (scope == NULL) {
				yyerror("Out of memory");
				YYERROR;
			}
			scope->task = 0;
			scope->timeout = $2;

			$$ = scope;
		}
		;

alfrule		: ruleid alffilterrule	scope		{
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				apn_free_filter(&$2.filtspec);
				free($3);
				yyerror("Out of memory");
				YYERROR;
			}

			rule->apn_type = APN_ALF_FILTER;
			rule->rule.afilt = $2;
			rule->apn_id = $1;
			rule->scope = $3;
			rule->userdata = NULL;
			rule->pchain = NULL;
			rule->app = NULL;

			$$ = rule;
		}
		| ruleid alfcaprule scope		{
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				free($3);
				yyerror("Out of memory");
				YYERROR;
			}

			rule->apn_type = APN_ALF_CAPABILITY;
			rule->rule.acap = $2;
			rule->apn_id = $1;
			rule->scope = $3;
			rule->userdata = NULL;
			rule->pchain = NULL;
			rule->app = NULL;

			$$ = rule;
		}
		| ruleid alfdefault scope		{
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				free($3);
				yyerror("Out of memory");
				YYERROR;
			}

			rule->apn_type = APN_DEFAULT;
			rule->rule.apndefault = $2;
			rule->apn_id = $1;
			rule->scope = $3;
			rule->userdata = NULL;
			rule->pchain = NULL;
			rule->app = NULL;

			$$ = rule;
		}
		;

alffilterrule	: action alffilterspec		{
			$$.action = $1;
			$$.filtspec = $2;
		}
		;

alffilterspec	: netaccess log proto hosts {
			$$.netaccess = $1;
			$$.log = $2;
			$$.proto = $3;
			$$.fromhost = $4.fromhost;
			$$.fromport = $4.fromport;
			$$.tohost = $4.tohost;
			$$.toport = $4.toport;

			if (validate_alffilterspec(&$$) == -1) {
				apn_free_filter(&$$);
				YYERROR;
			}
		}
		;

netaccess	: CONNECT			{ $$ = APN_CONNECT; }
		| ACCEPT			{ $$ = APN_ACCEPT; }
		| SEND				{ $$ = APN_SEND; }
		| RECEIVE			{ $$ = APN_RECEIVE; }
		| BOTH				{ $$ = APN_BOTH; }
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

hostspec	: '{' host_l '}'		{ $$ = $2.head; }
		| host				{ $$ = $1; }
		| ANY				{ $$ = NULL; }
		;

host_l		: host_l comma host		{
			if ($3 == NULL)
				$$ = $1;
			else {
				if ($1.tail) {
					$1.tail->next = $3;
				} else {
					$1.head = $3;
				}
				$1.tail = $3;
				$$ = $1;
			}
		}
		| host				{
			$$.head = $$.tail = $1;
		}
		;

host		: not address			{
			struct apn_host	*host;

			host = calloc(1, sizeof(struct apn_host));
			if (host == NULL) {
				yyerror("Out of memory");
				YYERROR;
			}

			host->negate = $1;
			host->addr = $2;

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

ports		: '{' port_l '}'		{ $$ = $2.head; }
		| port				{ $$ = $1; }
		;

port_l		: port_l comma port		{
			if ($3 == NULL)
				$$ = $1;
			else {
				if ($1.tail) {
					$1.tail->next = $3;
				} else {
					$1.head = $3;
				}
				$1.tail = $3;
				$$ = $1;
			}
		}
		| port				{
			$$.head = $$.tail = $1;
		}
		;

port		: NUMBER minus NUMBER		{
			struct apn_port *port;
			port = calloc(1, sizeof(struct apn_port));
			if (port == NULL) {
				yyerror("Out of memory");
				YYERROR;
			}

			if (portbynumber($1, &port->port) == -1) {
				free(port);
				YYERROR;
			}

			if (portbynumber($3, &port->port2) == -1) {
				free(port);
				YYERROR;
			}

			if ($3 < $1) {
				yyerror("portrange invalid: %ld-%ld", $1, $3);
				free(port);
				YYERROR;
			}

			$$ = port;
		}
		| STRING			{
			struct apn_port	*port;

			port = calloc(1, sizeof(struct apn_port));
			if (port == NULL) {
				free($1);
				yyerror("Out of memory");
				YYERROR;
			}

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
			if (port == NULL) {
				yyerror("Out of memory");
				YYERROR;
			}

			if (portbynumber($1, &port->port) == -1) {
				free(port);
				YYERROR;
			}

			$$ = port;
		}
		;

alfcaprule	: action log capability		{
			$$.action = $1;
			$$.log = $2;
			$$.capability = $3;
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
sfsmodule	: SFS optnl '{' optnl sfsrule_l '}'	{
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				apn_free_chain($5, NULL);
				free($5);
				yyerror("Out of memory");
				YYERROR;
			}

			rule->apn_type = APN_SFS;
			rule->apn_id = 0;
			rule->app = NULL;
			rule->userdata = NULL;
			rule->pchain = NULL;
			TAILQ_INIT(&rule->rule.chain);
			while (!TAILQ_EMPTY($5)) {
				struct apn_rule *srule;
				srule = TAILQ_FIRST($5);
				TAILQ_REMOVE($5, srule, entry);
				TAILQ_INSERT_TAIL(&rule->rule.chain,
				    srule, entry);
				srule->pchain = &rule->rule.chain;
			}
			free($5);
			if (apn_add_sfsblock(apnrsp, rule, file->name,
			    yylval.lineno) != 0) {
				apn_free_one_rule(rule, NULL);
				YYERROR;
			}
		}
		;

sfsrule_l	: sfsrule_l sfsrule nl		{
			if ($2 == NULL)
				$$ = $1;
			else {
				TAILQ_INSERT_TAIL($1, $2, entry);
				$2->pchain = $1;
				$$ = $1;
			}
		}
		| /* Empty */			{
			$$ = calloc(1, sizeof(struct apn_chain));
			if ($$ == NULL) {
				yyerror("Out of memory");
				YYERROR;
			}
			TAILQ_INIT($$);
		}
		;

sfsrule		: ruleid sfsaccessrule scope {
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				apn_free_sfsaccess(&$2);
				yyerror("Out of memory");
				YYERROR;
			}
			rule->apn_type = APN_SFS_ACCESS;
			rule->scope = $3;
			rule->rule.sfsaccess = $2;
			rule->userdata = NULL;
			rule->pchain = NULL;
			rule->apn_id = $1;

			$$ = rule;
		}
		| ruleid sfsdefaultrule scope {
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				apn_free_sfsdefault(&$2);
				yyerror("Out of memory");
				YYERROR;
			}

			rule->apn_type = APN_SFS_DEFAULT;
			rule->scope = $3;
			rule->rule.sfsdefault = $2;
			rule->userdata = NULL;
			rule->pchain = NULL;
			rule->apn_id = $1;

			$$ = rule;
		}
		;

sfsdefaultrule	: DEFAULT sfspath log action {
			$$.path = $2;
			$$.log = $3;
			$$.action = $4;
		}
		;

sfsaccessrule	: sfspath sfssubject sfsvalid sfsinvalid sfsunknown {
			$$.path    = $1;
			$$.subject = $2;
			$$.valid   = $3;
			$$.invalid = $4;
			$$.unknown = $5;
		}
		;

sfsvalid	: VALID log sfsaction {
			$$.log = $2;
			$$.action = $3;
		}
		| /* Empty */			{
			$$.log = APN_LOG_NONE;
			$$.action = APN_ACTION_ALLOW;
		}
		;

sfsinvalid	: INVALID log sfsaction {
			$$.log = $2;
			$$.action = $3;
		}
		| /* Empty */			{
			$$.log = APN_LOG_NORMAL;
			$$.action = APN_ACTION_DENY;
		}
		;

sfsunknown	: UNKNOWN log sfsaction {
			$$.log = $2;
			$$.action = $3;
		}
		| /* Empty */			{
			$$.log = APN_LOG_NONE;
			$$.action = APN_ACTION_CONTINUE;
		}
		;

sfsaction	: CONTINUE			{ $$ = APN_ACTION_CONTINUE; }
		| action			{ $$ = $1; }
		;

sfspath		: ANY		{ $$ = NULL; }
		| PATH STRING	{ $$ = normalize_path($2); }
		;

sfssubject	: SELF		{
			$$.type = APN_CS_UID_SELF;
		}
		| SIGNEDSELF {
			$$.type = APN_CS_KEY_SELF;
		}
		| UID NUMBER {
			$$.type = APN_CS_UID;
			$$.value.uid = $2;
		}
		| KEY STRING {
			$$.type = APN_CS_KEY;
			$$.value.keyid = $2;
		}
		;

		/*
		 * SB
		 */
sbmodule	: SB optnl '{' optnl sbruleset_l '}'
		| SB optnl '{' optnl '}'
		;

sbruleset_l	: sbruleset_l sbruleset
		| sbruleset
		;

sbruleset	: ruleid apps optnl '{' optnl sbrule_l '}' nl {
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				apn_free_app($2);
				apn_free_chain($6, NULL);
				free($6);
				yyerror("Out of memory");
				YYERROR;
			}

			rule->app = $2;
			rule->scope = NULL;
			rule->apn_type = APN_SB;
			rule->apn_id = $1;
			rule->userdata = NULL;
			rule->pchain = NULL;
			TAILQ_INIT(&rule->rule.chain);
			while(!TAILQ_EMPTY($6)) {
				struct apn_rule *sbrule;
				sbrule = TAILQ_FIRST($6);
				TAILQ_REMOVE($6, sbrule, entry);
				TAILQ_INSERT_TAIL(&rule->rule.chain,
				    sbrule, entry);
				sbrule->pchain = &rule->rule.chain;
			}
			free($6);

			if (apn_add_sbblock(apnrsp, rule, file->name,
			    yylval.lineno) != 0) {
				/* Account for errors in apn_add_sbblock */
				file->errors++;
				apn_free_one_rule(rule, NULL);
				YYERROR;
			}
		}
		;

sbrule_l	: sbrule_l sbrule nl		{
			if ($2 == NULL)
				$$ = $1;
			else {
				TAILQ_INSERT_TAIL($1, $2, entry);
				$2->pchain = $1;
				$$ = $1;
			}
		}
		| /* Empty */			{
			$$ = calloc(1, sizeof(struct apn_chain));
			if ($$ == NULL) {
				yyerror("Out of memory");
				YYERROR;
			}
			TAILQ_INIT($$);
		}
		;

sbrule		: ruleid sbaccess scope {
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				apn_free_sbaccess(&$2);
				free($3);
				yyerror("Out of memory");
				YYERROR;
			}

			rule->apn_type = APN_SB_ACCESS;
			rule->rule.sbaccess = $2;
			rule->apn_id = $1;
			rule->scope = $3;
			rule->userdata = NULL;
			rule->pchain = NULL;
			rule->app = NULL;

			$$ = rule;
		}
		| ruleid sbdefault scope {
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				free($3);
				yyerror("Out of memory");
				YYERROR;
			}

			rule->apn_type = APN_DEFAULT;
			rule->rule.apndefault = $2;
			rule->apn_id = $1;
			rule->scope = $3;
			rule->userdata = NULL;
			rule->pchain = NULL;
			rule->app = NULL;

			$$ = rule;
		}
		;

sbaccess	: action log sbpred sbrwx {
			$$ = $3;
			$$.action = $1;
			$$.log = $2;
			$$.amask = $4;
		}
		;

sbpred		: ANY {
			$$.cs.type = APN_CS_NONE;
			$$.cs.value.keyid = NULL;	/* Just to be sure */
			$$.path = NULL;
		}
		| sbpath {
			$$ = $1;
		}
		| sbkey {
			$$ = $1;
		}
		| sbuid {
			$$ = $1;
		}
		| sbpath sbkey {
			$$ = $1;
			$$.cs.type = $2.cs.type;
			$$.cs.value.keyid = $2.cs.value.keyid;
		}
		| sbpath sbuid {
			$$ = $1;
			$$.cs.type = $2.cs.type;
			$$.cs.value.uid = $2.cs.value.uid;
		}
		;

sbpath		: PATH STRING {
			$$.path = normalize_path($2);
			$$.cs.type = APN_CS_NONE;
			$$.cs.value.keyid = NULL;
		}
		;

sbkey		: KEY STRING {
			$$.path = NULL;
			$$.cs.type = APN_CS_KEY;
			$$.cs.value.keyid = $2;
		}
		| SIGNEDSELF {
			$$.path = NULL;
			$$.cs.type = APN_CS_KEY_SELF;
			$$.cs.value.keyid = NULL;
		}
		;

sbuid		: UID NUMBER {
			if ($2 < 0) {
				yyerror("Invalid uid %d", $2);
				YYERROR;
			}
			$$.path = NULL;
			$$.cs.type = APN_CS_UID;
			$$.cs.value.uid = $2;
		}
		| SELF {
			$$.path = NULL;
			$$.cs.type = APN_CS_UID_SELF;
			$$.cs.value.uid = 0;
		}
		;

sbrwx		: STRING {
			int i;
			$$ = 0;
			for (i=0; $1[i]; ++i) {
				int nm = 0;
				switch($1[i]) {
				case 'r': case 'R':
					nm = APN_SBA_READ;
					break;
				case 'w': case 'W':
					nm = APN_SBA_WRITE;
					break;
				case 'x': case 'X':
					nm = APN_SBA_EXEC;
					break;
				default:
					free($1);
					yyerror("Bad character %c "
					    "in permission string", $1[i]);
					YYERROR;
				}
				if ($$ & nm) {
					yyerror("Duplicate character %c "
					    "in permission string", $1[i]);
					free($1);
					YYERROR;
				}
				$$ |= nm;
			}
			free($1);
		}
		;

sbdefault	: defaultrule
		;


ctxmodule	: CONTEXT optnl '{' optnl ctxruleset_l '}'
		| CONTEXT optnl '{' optnl '}'
		;

ctxruleset_l	: ctxruleset_l ctxruleset
		| ctxruleset
		;

ctxruleset	: ruleid apps optnl '{' optnl ctxrule_l '}' nl {
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				apn_free_app($2);
				apn_free_chain($6, NULL);
				free($6);
				yyerror("Out of memory");
				YYERROR;
			}

			rule->app = $2;
			rule->scope = NULL;
			rule->apn_type = APN_CTX;
			rule->userdata = NULL;
			rule->pchain = NULL;
			rule->apn_id = $1;
			TAILQ_INIT(&rule->rule.chain);
			while(!TAILQ_EMPTY($6)) {
				struct apn_rule *arule;
				arule = TAILQ_FIRST($6);
				TAILQ_REMOVE($6, arule, entry);
				TAILQ_INSERT_TAIL(&rule->rule.chain,
				    arule, entry);
				arule->pchain = &rule->rule.chain;
			}
			free($6);

			if (apn_add_ctxblock(apnrsp, rule, file->name,
			    yylval.lineno) != 0) {
				/* Account for errors in apn_add_alfblock */
				file->errors++;
				apn_free_one_rule(rule, NULL);
				YYERROR;
			}
		}
		;

ctxrule_l	: ctxrule_l ctxrule nl		{
			if ($2 == NULL)
				$$ = $1;
			else {
				TAILQ_INSERT_TAIL($1, $2, entry);
				$2->pchain = $1;
				$$ = $1;
			}
		}
		| /* Empty */			{
			$$ = calloc(1, sizeof(struct apn_chain));
			if ($$ == NULL) {
				yyerror("Out of memory");
				YYERROR;
			}
			TAILQ_INIT($$);
		}
		;

ctxrule		: ruleid ctxruleapps scope			{
			struct apn_rule	*rule;

			rule = calloc(1, sizeof(struct apn_rule));
			if (rule == NULL) {
				free($3);
				apn_free_app($2.application);
				yyerror("Out of memory");
				YYERROR;
			}

			rule->apn_type = APN_CTX_RULE;
			rule->rule.apncontext = $2;
			rule->apn_id = $1;
			rule->scope = $3;
			rule->userdata = NULL;
			rule->pchain = NULL;
			rule->app = NULL;

			$$ = rule;
		}
		;

ctxruleapps	: CONTEXT NEW apps		{
			$$.application = $3;
			$$.type = APN_CTX_NEW;
		}
		| CONTEXT OPEN apps		{
			$$.application = $3;
			$$.type = APN_CTX_OPEN;
		}
		| CONTEXT BORROW apps		{
			$$.application = $3;
			$$.type = APN_CTX_BORROW;
		}
		;


		/*
		 * VS
		 */
vsmodule	: VS optnl '{' optnl vsruleset_l '}'
		| VS optnl '{' optnl '}'
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

vsspec		: STRING			{ free($1); }
		;

		/*
		 * Default rule
		 */
defaultrule	: DEFAULT log defaultspec	{
			$$.log = $2;
			$$.action = $3;
		}
		;


defaultspec	: action			{ $$ = $1; }
		;

		/*
		 * Common elements
		 */
apps		: '{' app_l '}'			{ $$ = $2.head; }
		| app				{ $$ = $1; }
		| ANY				{ $$ = NULL; }
		;

app_l		: app_l comma optnl app		{
			if ($4 == NULL)
				$$ = $1;
			else {
				if ($1.tail) {
					$1.tail->next = $4;
				} else {
					$1.head = $4;
				}
				$1.tail = $4;
				$$ = $1;
			}
		}
		| app				{
			$$.head = $$.tail = $1;
		}
		;

app		: STRING sfssubject		{
			struct apn_app	*app;
			app = calloc(1, sizeof(struct apn_app));
			if (app == NULL) {
				free($1);
				apn_free_subject(&$2);
				yyerror("Out of memory");
				YYERROR;
			}
			app->name = normalize_path($1);
			app->subject = $2;
			$$ = app;
		}
		| STRING			{
			struct apn_app	*app;
			app = calloc(1, sizeof(struct apn_app));
			if (app == NULL) {
				free($1);
				yyerror("Out of memory");
				YYERROR;
			}
			app->name = normalize_path($1);
			app->subject.type = APN_CS_NONE;
			$$ = app;
		}
		;

action		: ALLOW				{ $$ = APN_ACTION_ALLOW; }
		| DENY				{ $$ = APN_ACTION_DENY; }
		| ASK				{
			if (apnrsp->flags & APN_FLAG_NOASK) {
				yyerror("Action ``ASK'' not permitted");
				YYERROR;
			}
			$$ = APN_ACTION_ASK;
		}
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
	va_list			 ap;
	int ret;

	file->errors++;
	va_start(ap, fmt);
	ret = apn_verror(apnrsp, file->name, yylval.lineno, fmt, ap);
	va_end(ap);
	return ret;
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
		{ ":",		COLON },
		{ "accept",	ACCEPT },
		{ "alert",	ALERT },
		{ "alf",	ALF },
		{ "all",	ALL },
		{ "allow",	ALLOW },
		{ "any",	ANY },
		{ "application", APPLICATION },
		{ "ask",	ASK },
		{ "borrow",	BORROW },
		{ "both",	BOTH },
		{ "cap",	CAP },
		{ "chmod",	CHMOD },
		{ "connect",	CONNECT },
		{ "context",	CONTEXT },
		{ "continue",	CONTINUE },
		{ "csum",	CSUM },
		{ "default",	DEFAULT },
		{ "deny",	DENY },
		{ "exec",	EXEC },
		{ "file",	TFILE },
		{ "from",	FROM },
		{ "host",	HOST },
		{ "inet",	INET },
		{ "inet6",	INET6 },
		{ "invalid",	INVALID },
		{ "key",	KEY },
		{ "log",	LOG },
		{ "new",	NEW },
		{ "open",	OPEN },
		{ "other",	OTHER },
		{ "path",	PATH },
		{ "port",	PORT },
		{ "raw",	RAW },
		{ "read",	READ },
		{ "receive",	RECEIVE },
		{ "rule",	RULE },
		{ "sandbox",	SB },
		{ "sb",		SB },
		{ "self",	SELF },
		{ "send",	SEND },
		{ "sfs",	SFS },
		{ "sha256",	SHA256 },
		{ "signed-self",	SIGNEDSELF },
		{ "task",	TASK },
		{ "tcp",	TCP },
		{ "to",		TO },
		{ "udp",	UDP },
		{ "uid",	UID },
		{ "unknown",	UNKNOWN },
		{ "until",	UNTIL },
		{ "valid",	VALID },
		{ "vs",		VS },
		{ "write",	WRITE },
		/* the above list has to be sorted always */
	};
	const struct keywords	*p;

	p = bsearch(s, keywords, sizeof(keywords)/sizeof(keywords[0]),
	    sizeof(keywords[0]), kw_cmp);

	if (p)
		return (p->k_val);
	else
		return (STRING);
}

#define MAXPUSHBACK	128

char	*parsebuf;
int	 parseindex;
char	 pushback_buffer[MAXPUSHBACK];
int	 pushback_index = 0;

int llgetc(struct file * file)
{
	switch(file->type) {
	case APN_FILE:
		return getc(file->u.u_stream);
	case APN_IOVEC:
		while(1) {
			int vec, idx;
			struct iovec *iov;

			vec = file->u.u_iov.cvec;
			if (vec >= file->u.u_iov.count)
				return EOF;
			iov = &file->u.u_iov.vec[vec];
			idx = file->u.u_iov.cidx;
			if ((unsigned int)idx >= iov->iov_len) {
				file->u.u_iov.cidx = 0;
				file->u.u_iov.cvec++;
				continue;
			}
			file->u.u_iov.cidx++;
			return ((unsigned char*)(iov->iov_base))[idx];
		}
	}
	return EOF;
}

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
		if ((c = llgetc(file)) == EOF) {
			yyerror("reached end of file while parsing quoted "
			    "string");
			if (popfile() == EOF)
				return (EOF);
			return (quotec);
		}
		return (c);
	}

	while ((c = llgetc(file)) == '\\') {
		next = llgetc(file);
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
		c = llgetc(file);
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
			yyerror("yylex: strdup failed");
		return (STRING);
	}

#define allowed_to_end_number(x) \
	(isspace(x) || x == ')' || x ==',' || x == '/' || x == '}' \
	 || x == ':' || x == '=')

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
				yyerror("yylex: strdup failed");
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

struct file *
pushfile(const char *name)
{
	struct file	*nfile;

	if ((nfile = calloc(1, sizeof(struct file))) == NULL ||
	    (nfile->name = strdup(name)) == NULL) {
		return (NULL);
	}
	nfile->type = APN_FILE;
	if (TAILQ_FIRST(&files) == NULL && strcmp(nfile->name, "-") == 0) {
		nfile->u.u_stream = stdin;
		free(nfile->name);
		if ((nfile->name = strdup("stdin")) == NULL) {
			free(nfile);
			return (NULL);
		}
	} else if ((nfile->u.u_stream = fopen(nfile->name, "r")) == NULL) {
		free(nfile->name);
		free(nfile);
		return (NULL);
	}
	nfile->lineno = 1;
	TAILQ_INSERT_TAIL(&files, nfile, entry);
	return (nfile);
}

struct file *
pushiov(const char *name, struct iovec *vec, int count)
{
	struct file	*nfile;

	if ((nfile = calloc(1, sizeof(struct file))) == NULL ||
	    (nfile->name = strdup(name)) == NULL) {
		return NULL;
	}
	nfile->type = APN_IOVEC;
	nfile->lineno = 1;
	nfile->u.u_iov.vec = vec;
	nfile->u.u_iov.count = count;
	nfile->u.u_iov.cvec = 0;
	nfile->u.u_iov.cidx = 0;
	TAILQ_INSERT_TAIL(&files, nfile, entry);
	return nfile;
}

int
popfile(void)
{
	struct file	*prev;

	if ((prev = TAILQ_PREV(file, files, entry)) != NULL) {
		prev->errors += file->errors;
		TAILQ_REMOVE(&files, file, entry);
		if (file->type == APN_FILE)
			fclose(file->u.u_stream);
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
	TAILQ_INIT(&files);
	if ((file = pushfile(filename)) == NULL) {
		apn_error(apnrspx, filename, 0, "couldn't read file");
		return (1);
	}
	return __parse_rules_common(apnrspx);
}

int
parse_rules_iovec(const char *filename, struct iovec *iovec, int count,
    struct apn_ruleset *apnrspx)
{
	TAILQ_INIT(&files);
	if ((file = pushiov(filename, iovec, count)) == NULL) {
		apn_error(apnrspx, filename, 0, "Out of memory");
		return 1;
	}
	return __parse_rules_common(apnrspx);
}

int
__parse_rules_common(struct apn_ruleset *apnrspx)
{
	int errors = 0;

	apnrsp = apnrspx;

	yyparse();
	errors = file->errors;
	popfile();

	if (file->type == APN_FILE)
		fclose(file->u.u_stream);
	free(file->name);
	free(file);

	apn_assign_ids(apnrspx);

	return (errors ? 1 : 0);
}

int
str2hash(const char *s, u_int8_t *dest, size_t max_len)
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
		    &h->apa.v6, sizeof(struct in6_addr));
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
			return -1;
		break;

	case AF_INET6:
		if (host->addr.len > 128)
			return -1;
		break;

	default:
		return -1;
	}

	if (!(0 <= host->negate && host->negate <= 1))
		return -1;

	return 0;
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
	if (!(APN_LOG_NONE <= afspec->log && afspec->log <= APN_LOG_ALERT))
		return (-1);

	if (!(APN_CONNECT <= afspec->netaccess && afspec->netaccess <=
	    APN_BOTH))
		return (-1);

	switch (afspec->proto) {
	case IPPROTO_UDP:
		if (afspec->netaccess != APN_SEND && afspec->netaccess !=
		    APN_RECEIVE && afspec->netaccess != APN_BOTH) {
			yyerror("invalid connection state");
			return (-1);
		}
		break;

	case IPPROTO_TCP:
		if (afspec->netaccess != APN_CONNECT && afspec->netaccess !=
		    APN_ACCEPT && afspec->netaccess != APN_BOTH) {
			yyerror("invalid connection state");
			return (-1);
		}
		break;

	default:
		return (-1);
	}

	if (validate_hostlist(afspec->fromhost) < 0) {
		yyerror("invalid source host");
		return -1;
	}
	if (validate_hostlist(afspec->tohost) < 0) {
		yyerror("invalid destination host");
		return -1;
	}
	return (0);
}

/*
 * Verfiy that all hosts in a list have valid adress family and netmask.
 *
 * Returns -1 on failure, zero otherwise.
 */
int
validate_hostlist(struct apn_host *hostlist)
{
	struct apn_host	*hp;

	for (hp = hostlist; hp; hp = hp->next) {
		if (validate_host(hp) == -1)
			return -1;
	}
	return 0;
}

char *
normalize_path(char *path)
{
	int	src = 0, dst = 0;
	if (path[0] != '/') {
		yyerror("Invalid path %s", path);
		free(path);
		return NULL;
	}
	while(path[src]) {
		if (path[src] != '/') {
			path[dst++] = path[src++];
			continue;
		}
		/* path[src] is a slash an is not yet copied. */

		/* Skip consecutive/trailing slashes */
		if (path[src+1] == '/' || path[src+1] == 0) {
			src++;
			continue;
		}
		/* Skip /./ and /. at end of path */
		if (path[src+1] == '.'
		    && (path[src+2] == '/' || path[src+2] == 0)) {
			src += 2;
			continue;
		}
		/* Handle /../ and /.. at end of path. */
		if (path[src+1] == '.' && path[src+2] == '.'
		    && (path[src+3] == '/' || path[src+3] == 0)) {
			src += 3;
			while(dst) {
				dst--;
				if (path[dst] == '/')
					break;
			}
			continue;
		}
		/* No special case matched, copy the slash. */
		path[dst++] = path[src++];
	}
	/* Remove a trailing slash (just to be sure, should never happen) */
	if (dst && path[dst-1] == '/')
		dst--;
	/* A zero length path is not allowed. Change it to "/" */
	if (dst == 0)
		path[dst++] = '/';
	/* NUL termination */
	path[dst] = 0;
	return path;
}

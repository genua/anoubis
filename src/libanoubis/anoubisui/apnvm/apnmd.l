%option nounput
%option noinput
%option noyywrap
%option prefix="apnmd_"

%x MD
%s OUT

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

#include <sys/queue.h>

#include <stdlib.h>
#include <string.h>

#include "anoubis_apnvm_md.h"
#include "anoubis_apnvm_util.h"

#define ECHO {}

/* A key-value pair */
struct apnmd_kv {
	char	*key;
	char	*value;
	LIST_ENTRY(apnmd_kv) entry;
};

/* Head of key-value-list */
LIST_HEAD(apnmd_kv_list, apnmd_kv);

struct _apnmd {
	struct apnmd_kv_list kv_list;
};

/*
 * apnmd-structure used by the lexer.
 */
static struct _apnmd *apnmd_md = NULL;

/**
 * Creates an empty key-value-list.
 */
static struct apnmd_kv *
apnmd_kvcreate(const char *key)
{
	struct apnmd_kv *kv = malloc(sizeof(struct apnmd_kv));

	if (kv != NULL) {
		kv->key = strdup(key);
		kv->value = NULL;
	}

	return (kv);
}

/**
 * Searches for a key-value-pair with the specified key in the list pointed by
 * head. If no such entry exists and create is set to true, the key-value-pair
 * is created and appended to the list.
 */
static struct apnmd_kv *
apnmd_getkv(struct apnmd_kv_list *head, const char *key, int create)
{
	struct apnmd_kv *kv = NULL;

	LIST_FOREACH(kv, head, entry) {
		if (strcmp(kv->key, key) == 0)
			break; /* Found, leave loop */
	}

	if ((kv == NULL) && create) { /* Create, if requested */
		kv = apnmd_kvcreate(key);

		if (kv != NULL)
			LIST_INSERT_HEAD(head, kv, entry);
	}

	return (kv);
}

%}

%%

"<apnvm-metadata>" {
	BEGIN(MD);
}

<MD>"</apnvm-metadata>" {
	BEGIN(OUT);
}

<MD>^([[:space:]]*)([[:alnum:]]+)([[:space:]]*)":="(.*)$ {
	char *key, *value, *p;

	/* Split key, value at ":=" */
	p = strstr(yytext, ":=");
	*p = '\0';

	key = apnvm_trim(yytext);
	value = apnvm_trim(p + 2);

	/* Append of meta-database */
	apnmd_put(apnmd_md, key, value);
}

<MD>^([[:space:]]*)([[:alnum:]]+)([[:space:]]*)"+="(.*)$ {
	char *key, *value, *p;

	/* Split key, value at "+=" */
	p = strstr(yytext, "+=");
	*p = '\0';

	key = apnvm_trim(yytext);
	value = apnvm_trim(p + 2);

	/* Append to meta-database */
	apnmd_add(apnmd_md, key, value);
}

. {}

%%

apnmd *
apnmd_parse(const char *content)
{
	struct _apnmd *md;

	if ((md = malloc(sizeof(struct _apnmd))) == NULL)
		return (NULL);

	// cppcheck-suppress uninitdata
	LIST_INIT(&md->kv_list);

	/* Parse only, if you have something to parse */
	if ((content != NULL) && (strlen(content) > 0)) {
		apnmd_md = md;
		apnmd__scan_string(content);
		BEGIN(INITIAL);
		apnmd_lex();
		apnmd_md = NULL;
#ifdef LINUX
		apnmd_lex_destroy();
#endif
	}

	return (md);
}

void
apnmd_destroy(apnmd *md)
{
	if (md == NULL)
		return;

	while (!LIST_EMPTY(&md->kv_list)) {
		struct apnmd_kv *kv = LIST_FIRST(&md->kv_list);
		LIST_REMOVE(kv, entry);

		free(kv->key);
		free(kv->value);
		free(kv);
	}

	free(md);
}

char *
apnmd_serialize(apnmd *md)
{
	char		*content = NULL;
	char		*new_content;
	struct apnmd_kv	*kv;

	if (md == NULL)
		return (NULL);

	content = apnvm_appendline(content, "<apnvm-metadata>");
	if (content == NULL)
		return (NULL);

	LIST_FOREACH(kv, &md->kv_list, entry) {
		char *p, *nl;
		int r;

		p = kv->value;
		r = 0;

		while (1) {
			nl = index(p, '\n');

			new_content = apnvm_appendline(content, kv->key);

			if (new_content == NULL) {
				free(content);
				return (NULL);
			} else
				content = new_content;

			if (r == 0) /* First round always := */
				new_content = apnvm_append(content, " := ", 4);
			else /* Any following rounds: append something */
				new_content = apnvm_append(content, " += ", 4);

			if (new_content == NULL) {
				free(content);
				return (NULL);
			} else
				content = new_content;

			if (nl != NULL)
				new_content =
				    apnvm_append(content, p, (nl - p));
			else /* Last line */
				new_content =
				    apnvm_append(content, p, strlen(p));

			if (new_content == NULL) {
				free(content);
				return (NULL);
			} else
				content = new_content;

			if (nl != NULL) {
				p = nl + 1;
				r++;
			}
			else
				break;
		}
	}

	new_content = apnvm_appendline(content, "</apnvm-metadata>");

	if (new_content == NULL) {
		free(content);
		return (NULL);
	} else
		content = new_content;

	return content;
}

int
apnmd_count(apnmd *md)
{
	struct apnmd_kv	*kv;
	int		count = 0;

	if (md == NULL)
		return (0);

	LIST_FOREACH(kv, &md->kv_list, entry)
		count++;

	return (count);
}

char *
apnmd_get(apnmd *md, const char *key)
{
	struct apnmd_kv *kv;

	if ((md == NULL) || (key == NULL))
		return (NULL);

	kv = apnmd_getkv(&md->kv_list, key, 0);
	return (kv != NULL) ? kv->value : NULL;
}

int
apnmd_get_int(apnmd *md, const char *key)
{
	char *result = apnmd_get(md, key);
	return (result != NULL) ? atoi(result) : 0;
}

void
apnmd_put(apnmd *md, const char *key, const char *value)
{
	struct apnmd_kv *kv;

	if ((md == NULL) || (key == NULL) || (value == NULL))
		return;

	if ((kv = apnmd_getkv(&md->kv_list, key, 1)) == NULL)
		return;

	if (kv->value != NULL) {
		/* Destroy already existing value */
		free(kv->value);
		kv->value = NULL;
	}

	kv->value = strdup(value);
}

void
apnmd_put_int(apnmd *md, const char *key, int value)
{
	char s[12];

	snprintf(s, sizeof(s), "%i", value);
	apnmd_put(md, key, s);
}

void
apnmd_add(apnmd *md, const char *key, const char *value)
{
	struct apnmd_kv *kv;

	if ((md == NULL) || (key == NULL) || (value == NULL))
		return;

	if ((kv = apnmd_getkv(&md->kv_list, key, 1)) == NULL)
		return;

	if (kv->value != NULL)	/* Append value */
		kv->value = apnvm_appendline(kv->value, value);
	else			/* Assign value */
		kv->value = strdup(value);
}

void
apnmd_add_int(apnmd *md, const char *key, int value)
{
	char s[12];

	snprintf(s, sizeof(s), "%i", value);
	apnmd_add(md, key, s);
}

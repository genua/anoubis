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
#include <string.h>
#include <stdlib.h>

#include "apnvmutil.h"

char*
apnvm_trim(char *s)
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

char *
apnvm_emptystring()
{
	char *s = malloc(1);
	*s = '\0';

	return (s);
}

char *
apnvm_append(char *s, const char *l, size_t n)
{
	size_t	ns, nl;
	char	*r;

	/* Len of s */
	ns = (s != NULL) ? strlen(s) : 0;

	/* Len of l */
	if (l != NULL)
		nl= (strlen(l) < n) ? strlen(l) : n;
	else
		nl = 0;

	r = realloc(s, ns + nl + 1);

	if (ns > 0)
		strncat(r, l, nl);
	else
		strncpy(r, l, nl);

	return (r);
}

char *
apnvm_appendline(char *s, const char *l)
{
	char	*newstr	= s;
	size_t	oldsize, newsize;

	oldsize = (s != NULL) ? strlen(s) : 0;
	newsize = oldsize + ((l != NULL) ? strlen(l) : 0) + 2;

	newstr = realloc(newstr, newsize);

	if (oldsize > 0) {
		strcat(newstr, "\n");
		strcat(newstr, l);
	}
	else
		strcpy(newstr, l);

	return (newstr);
}

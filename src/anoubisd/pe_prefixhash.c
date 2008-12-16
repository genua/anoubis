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

/*
 * This file implements a generic hash for apn_rule structures that
 * is used to speed up policy matching of path base policies (Sandbox/SFS).
 *
 * Each rule in a given ruleset either has a path prefix and matching
 * of that rule is restricted to paths below that prefix _or_ the rule
 * simply matches independant of the path.
 * Now each rule can be stored in a hash based on this prefix where
 * a NULL path corresponds to "no preifx restriction". In addition
 * to the rule itself we also strore the index of the rule within the
 * block. The hash will use this as a sort key. Note that the actual
 * path prefix is not directly stored in the hash, it is only used when
 * inserting into the hash.
 *
 * No assume that an event for the file /a/b/c occurs. We only have to
 * consider rules with the following prefixes for this event:
 *     NULL, "/", "/a", "/a/b" and "/a/b/c"
 * The function pe_prefixhash_getrules does this for you: Given a
 * path and a prefixhash it locates all slots that might contain rules
 * with a prefix relevant for the path in the event. Any rules found
 * that way are sorted according to their index and duplicates are
 * eliminiated.
 *
 * Note that the rules returned by pe_prefixhash_getrules are _candidates_
 * that might or might not match the actual path. In particular note that
 * upon retrival of rules there is no string comparision or prefix match
 * done by the cache.
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <anoubisd.h>
#include <pe.h>

/*
 * A single entry in the hash
 */
struct entry {
	struct entry		*next;
	struct apn_rule		*rule;
	int			 idx;
};

/*
 * The hash datastructure itself. The size of the hash table is
 * determined by the parameter to pe_prefixhash_create.
 */
struct pe_prefixhash {
	unsigned int		 tabsize;
	struct entry		*tab[0];
};


#define PREFIXHASH_SHIFT	(24)
#define PREFIXHASH_MASK		((1UL<<PREFIXHASH_SHIFT)-1)

static unsigned long
hash_fn(const char *data, int cnt)
{
	unsigned long	ret = 0;
	int		i;

	for (i=0; i<cnt; ++i,++data) {
		ret <<= 1;
		ret ^= *data;
		ret = (ret ^ (ret >> PREFIXHASH_SHIFT)) & PREFIXHASH_MASK;
	}
	return ret & PREFIXHASH_MASK;
}

/*
 * Create a new prefix hash with tabsize entries in the hash table.
 */
struct pe_prefixhash *
pe_prefixhash_create(unsigned int tabsize)
{
	struct pe_prefixhash	*ret;
	unsigned int			 i;

	if (tabsize > PREFIXHASH_MASK)
		tabsize = PREFIXHASH_MASK + 1;
	if (tabsize < 10)
		tabsize = 10;
	ret = malloc(sizeof(struct pe_prefixhash)
	    + tabsize * sizeof(struct entry *));
	if (!ret)
		return NULL;
	ret->tabsize = tabsize;
	for (i=0; i<tabsize; ++i)
		ret->tab[i] = 0;
	return ret;
}

/*
 * Free all memory associated with the prefixhash @hash.
 */
void
pe_prefixhash_destroy(struct pe_prefixhash *hash)
{
	unsigned int	i;

	for (i=0; i<hash->tabsize; ++i) {
		struct entry	*tmp, *tmp2;
		tmp = hash->tab[i];
		while(tmp) {
			tmp2 = tmp;
			tmp = tmp->next;
			free(tmp2);
		}
		hash->tab[i] = NULL;
	}
	free(hash);
}

/*
 * Add the rule @rule with index @idx to the hash slot corresponding to
 * @str in the hash @hash.
 */
int
pe_prefixhash_add(struct pe_prefixhash *hash, const char *str,
    struct apn_rule *rule, int idx)
{
	unsigned int	  hv;
	struct entry	**pp;
	struct entry	 *n;

	if (str)
		hv = hash_fn(str, strlen(str)) % hash->tabsize;
	else
		hv = hash_fn(NULL, 0) % hash->tabsize;
	pp = &hash->tab[hv];
	while (*pp) {
		pp = &(*pp)->next;
	}
	n = malloc(sizeof(struct entry));
	if (!n)
		return -ENOMEM;
	n->next = NULL;
	n->idx = idx;
	n->rule = rule;
	(*pp) = n;
	return 0;
}

/*
 * Comparision function for qsort.
 */
static int
cmp_entries(const void *a, const void *b)
{
	struct entry	*e1 = *(struct entry**)a;
	struct entry	*e2 = *(struct entry**)b;

	if (e1->idx < e2->idx)
		return -1;
	if (e1->idx == e2->idx)
		return 0;
	return 1;
}

#define ADDSLOT(ENTRIES, CNT, STR, LEN) do {				\
	struct entry	**nentries;					\
	struct entry	 *list, *tmp;					\
	int		  hv, ncnt = 0;					\
	hv = hash_fn((STR), (LEN)) % hash->tabsize;			\
	list = hash->tab[hv];						\
	if (!list)							\
		break;							\
	for (tmp = list; tmp; tmp = tmp->next)				\
		ncnt++;							\
	nentries = realloc((ENTRIES),					\
	    ((CNT) + ncnt) * sizeof(struct entry *));			\
	if (nentries == NULL) {						\
		free(ENTRIES);						\
		return -ENOMEM;						\
	}								\
	(ENTRIES) = nentries;						\
	for (tmp = list; tmp; tmp = tmp->next) {			\
		(ENTRIES)[(CNT)] = tmp;					\
		(CNT)++;						\
	}								\
} while (0);

int
pe_prefixhash_getrules(struct pe_prefixhash *hash, const char *str,
    struct apn_rule ***rulesp, int *rulecnt)
{
	int			  i, j, len = 0, cnt = 0, rcnt = 0;
	struct entry		**entries = NULL;
	struct apn_rule		**rules;

	if (str && str[0] != '/')
		return -EINVAL;
	/* Empty string: No path required at all. */
	ADDSLOT(entries, cnt, NULL, 0);
	if (str) {
		len = strlen(str);
		/* The whole string */
		ADDSLOT(entries, cnt, str, len);
		/* A single slash ('/') is always a valid prefix. */
		ADDSLOT(entries, cnt, str, 1);
	}
	while(len) {
		if (str[len] == '/')
			ADDSLOT(entries, cnt, str, len);
		len--;
	}
	qsort(entries, cnt, sizeof(struct entry *), cmp_entries);
	/*
	 * Kill duplicate rules. These can appear if two prefixes of
	 * a path hash to the same slot. Additionally this handles
	 * cases like '/' nicely.
	 */
	if (cnt) {
		rcnt = 1;
		for (i=1; i<cnt; ++i)
			if (entries[i]->idx != entries[i-1]->idx)
				rcnt++;
	}
	/* Avoid zero sized allocations just to be sure. */
	if (rcnt)
		rules = malloc(rcnt * sizeof(struct apn_rule *));
	else
		rules = malloc(sizeof(struct apn_rule *));
	for (i=j=0; i<cnt; ++i) {
		if (i && (entries[i-1]->idx == entries[i]->idx))
			continue;
		rules[j] = entries[i]->rule;
		j++;
	}
	free(entries);
	(*rulesp) = rules;
	(*rulecnt) = rcnt;
	return 0;
}

#undef ADDSLOT

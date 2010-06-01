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

/**
 * \file
 * This file implements a generic hash for apn_rule structures that
 * is used to speed up policy matching of path base policies (Sandbox/SFS).
 *
 * Each rule in a given ruleset either has a path prefix and matching
 * of that rule is restricted to paths below that prefix _or_ the rule
 * simply matches independant of the path.
 * Now, each rule can be stored in a hash based on this prefix where
 * a NULL path corresponds to "no preifx restriction". In addition
 * to the rule itself we also strore the index of the rule within the
 * block. The hash will use this as a sort key. Note that the actual
 * path prefix is not directly stored in the hash, it is only used when
 * inserting into the hash.
 *
 * Now, assume that an event for the file /a/b/c occurs. We only have to
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
#include <anoubis_alloc.h>
#include <pe.h>

/*
 * A single entry in the hash
 */
struct entry {
	struct entry		*next;
	struct apn_rule		*rule;
	int			 idx;
};
#include <prefixhash_entry_array.h>


/**
 * The hash datastructure itself. The size of the hash table is
 * determined by the parameter to pe_prefixhash_create.
 */
struct pe_prefixhash {
	unsigned int		 tabsize;
	struct entryarr_array	 tab;
};


#define PREFIXHASH_SHIFT	(24)
#define PREFIXHASH_MASK		((1UL<<PREFIXHASH_SHIFT)-1)

/**
 * Calculate a path name hash. The hash consideres the first @cnt
 * bytes of the data. The return value is between zero and PREFIXHASH_MASK
 * inclusive.
 *
 * @param data The data that should be hashed.
 * @cnt The length of the data to hash.
 */
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

/**
 * Create a new prefix hash with @tabsize entries in the hash table.
 *
 * @param tabsize The number of entries in the hash table.
 * @return A new pe_prefixhash structure or NULL in case of an error.
 */
struct pe_prefixhash *
pe_prefixhash_create(unsigned int tabsize)
{
	struct pe_prefixhash		*ret;
	unsigned int			 i;

	if (tabsize > PREFIXHASH_MASK)
		tabsize = PREFIXHASH_MASK + 1;
	if (tabsize < 10)
		tabsize = 10;
	ret = abuf_alloc_type(struct pe_prefixhash);
	if (!ret)
		return NULL;
	ret->tab = entryarr_alloc(tabsize);
	if (entryarr_size(ret->tab) != tabsize) {
		pe_prefixhash_destroy(ret);
		return NULL;
	}
	ret->tabsize = tabsize;
	for (i=0; i<tabsize; ++i)
		entryarr_access(ret->tab, i) = NULL;
	return ret;
}

/**
 * Free all memory associated with the prefixhash @hash.
 *
 * @param hash The prefix hash.
 * @return None.
 */
void
pe_prefixhash_destroy(struct pe_prefixhash *hash)
{
	unsigned int	i;

	for (i=0; i<hash->tabsize; ++i) {
		struct entry	*tmp, *tmp2;
		tmp = entryarr_access(hash->tab, i);
		while(tmp) {
			tmp2 = tmp;
			tmp = tmp->next;
			abuf_free_type(tmp2, struct entry);
		}
		entryarr_access(hash->tab, i) = NULL;
	}
	entryarr_free(hash->tab);
	abuf_free_type(hash, struct pe_prefixhash);
}

/**
 * Add the rule @rule to the prefix hash.
 *
 * @param hash The prefix hash.
 * @param str The path prefix associated with the rule (may be NULL).
 * @param rule The actual rule.
 * @param idx The index of the rule. This index is used to sort rule
 *     candidates.
 * @return Zero in case of success, a negative error code in case of
 *     an error.
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
	pp = &entryarr_access(hash->tab, hv);
	while (*pp) {
		pp = &(*pp)->next;
	}
	n = abuf_alloc_type(struct entry);
	if (!n)
		return -ENOMEM;
	n->next = NULL;
	n->idx = idx;
	n->rule = rule;
	(*pp) = n;
	return 0;
}

/**
 * Compare two prefix hash entries according to their respective indexes.
 * This is a callback function for qsort, both arguments point to
 * struct sfs_entry structures.
 *
 * @param a The first entry.
 * @param b The second entry.
 * @return The result of the comparison.
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

/**
 * Add all entries in a particular hash slot to the array @arr.
 *
 * @param hash The prefix hash.
 * @param arr The result array that contains the rules. The array must be
 *     empty at the first call to this function. Memory will be allocated
 *     and the array grows as neccessary. The caller is responsible for the
 *     array and must free it if the function returns successfully.
 * @param str The path prefix that determines the hash slot. This pointer
 *     can be NULL, @len should be zero in this case.
 * @param len The length of the path prefix. Only the first @len bytes are
 *     hashed. The path name must be at least @len bytes long but it may
 *     be longer.
 * @return Zero on success, a negative error code in case of an error.
 *     In case of an error, the memory associated with the array is freed.
 */
static inline int
addslot(struct pe_prefixhash *hash, struct entryarr_array *arr,
    const char *str, size_t len)
{
	int		 hv;
	size_t		 oldcnt = entryarr_size(*arr);
	size_t		 ncnt = oldcnt;
	struct entry	*list, *tmp;

	hv = hash_fn(str, len) % hash->tabsize;
	list = entryarr_access(hash->tab, hv);
	if (!list)
		return 0;
	for (tmp = list; tmp; tmp = tmp->next)
		ncnt++;
	if (oldcnt == 0) {
		(*arr) = entryarr_alloc(ncnt);
	} else {
		(*arr) = entryarr_resize(*arr, ncnt);
	}
	if (entryarr_size(*arr) == 0)
		return -ENOMEM;
	for (tmp = list; tmp; tmp = tmp->next, oldcnt++)
		entryarr_access(*arr, oldcnt) = tmp;
	return 0;
}

/**
 * Find all rules registered in the preifx hash that match a given path
 * name. This function does _not_ check if the rules actually match. However,
 * it guarantees that all rules that match the path name are within the
 * list of rule candidates.
 *
 * @param hash The prefix hash.
 * @param str The path name string.
 * @param rules An apnarr_array that contains pointers to struct apn_rules.
 *     Each rule is a candidate that might match the path @str. The caller
 *     must free the array in case of success. No memory is allocated if the
 *     function returns an error.
 * @return Zero in case of success, a negative error code in case of an error.
 */
int
pe_prefixhash_getrules(struct pe_prefixhash *hash, const char *str,
    struct apnarr_array *rules)
{
	int			 len = 0;
	struct entryarr_array	 entries = entryarr_EMPTY;
	size_t			 i, j, rcnt = 0;
	struct entry		*prev;

	(*rules) = apnarr_EMPTY;
	if (str && str[0] != '/')
		return -EINVAL;
	/* Empty string: No path required at all. */
	if (addslot(hash, &entries, NULL, 0) < 0)
		return -ENOMEM;
	if (str) {
		len = strlen(str);
		/* The whole string */
		if (addslot(hash, &entries, str, len) < 0)
			return -ENOMEM;
		/* A single slash ('/') is always a valid prefix. */
		if (addslot(hash, &entries, str, 1) < 0)
			return -ENOMEM;
	}
	while(len) {
		if (str[len] == '/')
			if (addslot(hash, &entries, str, len) < 0)
				return -ENOMEM;
		len--;
	}
	if (entryarr_size(entries) == 0)
		return 0;
	entryarr_qsort(entries, cmp_entries);
	/*
	 * Kill duplicate rules. These can appear if two prefixes of
	 * a path hash to the same slot. Additionally this handles
	 * cases like '/' nicely.
	 */
	rcnt = 1;
	for (i=1, prev = entryarr_access(entries, 0);
	    i<entryarr_size(entries); ++i) {
		struct entry	*cur;
		cur = entryarr_access(entries, i);
		if (cur->idx != prev->idx)
			rcnt++;
		prev = cur;
	}

	(*rules) = apnarr_alloc(rcnt);
	for (i=j=0; i<entryarr_size(entries); ++i) {
		if (i && entryarr_access(entries, i-1)->idx
		    == entryarr_access(entries, i)->idx)
			continue;
		apnarr_access((*rules), j) = entryarr_access(entries, i)->rule;
		j++;
	}
	entryarr_free(entries);
	return 0;
}

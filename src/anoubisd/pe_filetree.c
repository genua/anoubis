/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <pe_filetree.h>

#define FT_HASH_SHIFT	(24)
#define FT_HASH_MASK	((1UL<<FT_HASH_SHIFT)-1)

static int hashcmp(struct pe_file_node *, struct pe_file_node *);
static unsigned long hash_fn(const char *data, int cnt);

RB_PROTOTYPE_STATIC(rb_file_tree, pe_file_node, entry, hashcmp);
RB_GENERATE_STATIC(rb_file_tree, pe_file_node, entry, hashcmp);

static int
hashcmp(struct pe_file_node *e1, struct pe_file_node *e2)
{
	if (e1->idx == e2->idx)
		return strcmp(e1->path, e2->path);
	return (e1->idx - e2->idx);
}

static unsigned long
hash_fn(const char *data, int cnt)
{
	unsigned long	ret = 0;
	int		i;

	if (data == NULL)
		return 0;

	for (i=0; i<cnt; ++i,++data) {
		ret <<= 1;
		ret ^= *data;
		ret = (ret ^ (ret >> FT_HASH_SHIFT)) & FT_HASH_MASK;
	}
	return ret & FT_HASH_MASK;
}

/* Initilaize the tree */
struct pe_file_tree *
pe_init_filetree(void)
{
	struct pe_file_tree *tree;

	tree = malloc(sizeof(*tree));
	if (tree == NULL)
		return NULL;
	RB_INIT(&tree->head);
	return tree;
}

/* insert a node to the tree */
int
pe_insert_node(struct pe_file_tree *f, char *path, anoubis_cookie_t cookie)
{
	struct pe_file_node *n = NULL;

	if (f == NULL || path == NULL)
		return -EINVAL;
	n = malloc(sizeof(struct pe_file_node));
	if (n == NULL)
		return -ENOMEM;
	n->path = strdup(path);
	if (n->path == NULL) {
		free(n);
		return -ENOMEM;
	}
	n->idx = hash_fn(path, strlen(path));
	n->task_cookie = cookie;
	if (RB_INSERT(rb_file_tree, &f->head, n) != NULL) {
		free(n->path);
		free(n);
		return -EINVAL;
	}
	return 0;
}

/* deletes a node by a path name */
void
pe_delete_file(struct pe_file_tree *f, char *path)
{
	struct pe_file_node *n;
	if (f == NULL || path == NULL)
		return;
	n = pe_find_file(f, path);
	pe_delete_node(f, n);
}

/* deletes a node by a struct node */
void
pe_delete_node(struct pe_file_tree *f, struct pe_file_node *n)
{
	if (f == NULL || n == NULL)
		return;
	if (RB_REMOVE(rb_file_tree, &f->head, n) == NULL)
		return;
	free(n->path);
	free(n);
}

/* finds a node in a tree */
struct pe_file_node *
pe_find_file(struct pe_file_tree *f, char *path)
{
	struct pe_file_node n, *res = NULL;

	if (f == NULL || path == NULL)
		return NULL;
	n.path = path;
	n.idx = hash_fn(path, strlen(path));
	res = RB_FIND(rb_file_tree, &f->head, &n);
	return res;
}

/* gets a first node in the tree. this can be used to
 * initlize a iterator:
 * for (np = pe_get_start(f); np != NULL; np = pe_get_next(f, np)) {
 *	...
 * }
 */
struct pe_file_node *
pe_get_start(struct pe_file_tree *f)
{
	if (f == NULL)
		return NULL;
	return RB_MIN(rb_file_tree, &f->head);
}

/* gets the next node in the tree from the 'last node'
 * it will return NULL if the 'last node' was the last node
 */
struct pe_file_node *
pe_get_next(struct pe_file_tree *f, struct pe_file_node *last)
{
	if (f == NULL || last == NULL)
		return NULL;
	return RB_NEXT(rb_file_tree, &f->head, last);
}

/* clean up the tree */
void
pe_filetree_destroy(struct pe_file_tree *f)
{
	struct pe_file_node *var, *nxt;

	if (f == NULL)
		return;
	for (var = RB_MIN(rb_file_tree, &f->head); var != NULL; var = nxt) {
		nxt = RB_NEXT(rb_file_tree, &f->head, var);
		RB_REMOVE(rb_file_tree, &f->head, var);
		free(var->path);
		free(var);
	}
	free(f);
}

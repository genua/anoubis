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

static int hashcmp(struct pe_filetree_node *, struct pe_filetree_node *);
static unsigned long hash_fn(const char *str);

RB_PROTOTYPE_STATIC(rb_file_tree, pe_filetree_node, entry, hashcmp);
RB_GENERATE_STATIC(rb_file_tree, pe_filetree_node, entry, hashcmp);

/**
 * Compare two nodes in a file tree. The comparision is first done
 * based on the hash value. Nodes with equal hash values are
 * compared using strcmp. This function assumes that the hash
 * values of the paths are stored in the idx fields of the nodes.
 *
 * This function is used by the RED/BLACK tree implementation.
 *
 * @param e1 The first node.
 * @param d2 The second node.
 * @return The result of the comparison (negative, zero, positive as in
 *     strcmp).
 */
static int
hashcmp(struct pe_filetree_node *e1, struct pe_filetree_node *e2)
{
	if (e1->idx == e2->idx)
		return strcmp(e1->path, e2->path);
	return (e1->idx - e2->idx);
}

/**
 * Calculate the hash value of the argument string.
 *
 * @param str The string.
 * @return The hash value.
 */
static unsigned long
hash_fn(const char *str)
{
	unsigned long	ret = 0;

	if (str == NULL)
		return 0;

	for (; *str; str++) {
		ret <<= 1;
		ret ^= *str;
		ret = (ret ^ (ret >> FT_HASH_SHIFT)) & FT_HASH_MASK;
	}
	return ret & FT_HASH_MASK;
}

/**
 * Create and initialize a new file tree. The tree structure is
 * allocated dynamically and must be freed by the caller.
 *
 * @return The new tree or NULL if out of memory.
 */
struct pe_file_tree *
pe_filetree_create(void)
{
	struct pe_file_tree *tree;

	tree = malloc(sizeof(*tree));
	if (tree == NULL)
		return NULL;
	RB_INIT(&tree->head);
	return tree;
}

/**
 * Insert a path into a file tree. If a node with the same path already exists
 * the node is not inserted. However, if the cookie of the new node has the
 * special value 0, the cookie in the existing node is modified.
 *
 * @param f The file tree.
 * @param path The path to insert. The memory of the path is copied, i.e.
 *     the caller must properly free its own copy.
 * @param cookie The new cookie. This value is stored in the tree together
 *     with the path name. If the value is zero, the cookie of an existing
 *     node for the same path is set to zero, too. Otherwise an existing
 *     node for the same path is not modified.
 * @return Zero in case of success, a negative error code in case of an
 *     error.
 */
int
pe_filetree_insert(struct pe_file_tree *f, char *path, anoubis_cookie_t cookie)
{
	struct pe_filetree_node *n = NULL, *other;

	if (f == NULL || path == NULL)
		return -EINVAL;
	n = malloc(sizeof(struct pe_filetree_node));
	if (n == NULL)
		return -ENOMEM;
	n->path = strdup(path);
	if (n->path == NULL) {
		free(n);
		return -ENOMEM;
	}
	n->idx = hash_fn(path);
	n->task_cookie = cookie;
	other = RB_INSERT(rb_file_tree, &f->head, n);
	if (other) {
		free(n->path);
		free(n);
		if (cookie == 0)
			other->task_cookie = 0;
		return -EINVAL;
	}
	return 0;
}

/**
 * Delete a node from the file tree. The node and the memory for the
 * path name in the node is freed. The node must exist and must be part
 * of the given file tree.
 *
 * @param f The file tree.
 * @param n The node to delete.
 */
void
pe_filetree_remove(struct pe_file_tree *f, struct pe_filetree_node *n)
{
	if (f == NULL || n == NULL)
		return;
	if (RB_REMOVE(rb_file_tree, &f->head, n) == NULL)
		return;
	free(n->path);
	free(n);
}

/**
 * Find the node for a given path in the file tree. The node is
 * still owned by the tree and must not be freed or modified.
 *
 * @param f The file tree.
 * @param path The path to search for.
 * @return The node for the path name or NULL if the path name was not
 *     found.
 */
struct pe_filetree_node *
pe_filetree_find(struct pe_file_tree *f, char *path)
{
	struct pe_filetree_node	n, *res = NULL;

	if (f == NULL || path == NULL)
		return NULL;
	n.path = path;
	n.idx = hash_fn(path);
	res = RB_FIND(rb_file_tree, &f->head, &n);
	return res;
}

/**
 * Return a pointer to the first node in the file tree. This can
 * be used together with pe_filetree_next to implement an iterator.
 * like this:
 * <code>
 * for (np = pe_filetree_start(f); np != NULL; np = pe_filetree_next(f, np)) {
 *	...
 * }
 * </code>
 * @param f The file tree.
 * @return The first node in the file tree. NULL if the tree is empty.
 */
struct pe_filetree_node *
pe_filetree_start(struct pe_file_tree *f)
{
	if (f == NULL)
		return NULL;
	return RB_MIN(rb_file_tree, &f->head);
}

/**
 * Return the node that immediately follows the node given node in the
 * file tree. The given node must be part of the file tree.
 *
 * @param f The file tree.
 * @param last The previous node.
 * @return The node that immediately follows last.
 */
struct pe_filetree_node *
pe_filetree_next(struct pe_file_tree *f, struct pe_filetree_node *last)
{
	if (f == NULL || last == NULL)
		return NULL;
	return RB_NEXT(rb_file_tree, &f->head, last);
}

/**
 * Destroy a file tree and all its nodes.
 *
 * @param f The file tree.
 */
void
pe_filetree_destroy(struct pe_file_tree *f)
{
	struct pe_filetree_node *var, *nxt;

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

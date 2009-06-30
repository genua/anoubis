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
#ifndef RBTREE_H
#define RBTREE_H

/*
 * Generic RED/BLACK Tree Implementation for Anoubis.
 * For a complete overview of red/black tree see wikipedia or any
 * computer science text book. We will only give a short overview of
 * the properties of a red/black tree:
 *
 * A red/black tree is a binary search tree that fullfills the following
 * additional restrictions:
 * - Each node in the tree has a color (either red or black)
 * - The root of the tree is always black
 * - Both children of a red node are either NULL or black.
 * - Each path from the root to a leaf node contains the same number of
 *   black nodes.
 *
 * These properties guarantee that the tree depth is no more then
 * 2*lg(N). Thus finding a node for a given key is O(lg N).
 *
 * Insertion and removal of nodes maintains this property. Both operations
 * are implemented such that their running time is O(lg N).
 *
 * Implementation note:
 *   This implementation never allocates or frees an rb_entry structure.
 *   Addtionally it never copies the data contents of one structure to
 *   another one, not even during removal of an element.
 *   This implies that an rb_entry structure can safely be embedded into
 *   a surrounding structure.
 *
 */

#include <sys/cdefs.h>

#define ARB_RED		1
#define ARB_BLACK	2

/*
 * The following restrictions apply to the access of the rb_entry fields:
 *
 * Exclive access by rbtree.c:		@left, @right, @parent, @col
 * Never accessed by rbtree.c:		@data, @dtype
 * Read only access by rbtree.c:	@key
 *
 * @key must be set by the caller and it must not be modified while
 * the rb_entry is inside a tree.
 */

struct rb_entry  {
	unsigned long key;
	struct rb_entry *left, *right, *parent;
	void *data;
	short col;
	short dtype;
};

__BEGIN_DECLS

struct rb_entry	*rb_find(struct rb_entry *root, unsigned long key);
int		 rb_insert_entry(struct rb_entry **root, struct rb_entry *n);
int		 rb_remove_entry(struct rb_entry **root, struct rb_entry *e);
int		 rb_remove(struct rb_entry **root, unsigned long key);
__END_DECLS

#endif

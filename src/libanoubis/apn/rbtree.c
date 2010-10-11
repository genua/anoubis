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
#include <stdlib.h>
#include "anoubis_rbtree.h"

/*
 * Rotate left, i.e. e->right replaces e:
 *         e                   r
 *        / \                 / \
 *       l   r      ==>      e   rr
 *          / \             / \
 *         rl rr           l  rl
 */
static void
rotate_left(struct rb_entry **root, struct rb_entry *e)
{
	struct rb_entry *l, *r, *rl, *rr, *p;

	p = e->parent;
	l = e->left;
	r = e->right;
	rl = r->left;
	rr = r->right;

	e->left = l;
	e->right = rl;
	if (l)
		l->parent = e;
	if (rl)
		rl->parent = e;

	r->left = e;
	r->right = rr;
	e->parent = r;
	if (rr)
		rr->parent = r;

	r->parent = p;
	if (p == NULL) {
		*root = r;
	} else {
		if (p->left == e)
			p->left = r;
		else
			p->right = r;
	}
}

/*
 * Rotate right, i.e. e->left replaces e:
 *         e                   l
 *        / \                 / \
 *       l   r      ==>     ll   e
 *      / \                     / \
 *    ll  lr                  lr   r
 */
static void
rotate_right(struct rb_entry **root, struct rb_entry *e)
{
	struct rb_entry *l, *r, *ll, *lr, *p;

	p = e->parent;
	l = e->left;
	r = e->right;
	ll = l->left;
	lr = l->right;

	e->left = lr;
	e->right = r;
	if (r)
		r->parent = e;
	if (lr)
		lr->parent = e;

	l->left = ll;
	l->right = e;
	if (ll)
		ll->parent = l;
	e->parent = l;

	l->parent = p;
	if (p == NULL) {
		*root = l;
	} else {
		if (p->left == e)
			p->left = l;
		else
			p->right = l;
	}
}

/**
 * Locate an entry
 *
 * @param root The root of the tree
 * @param key The key which identifies the entry
 * @return an entry or NULL if none was found 
 */
struct rb_entry *
rb_find(struct rb_entry *root, unsigned long key)
{
	while(root) {
		if (root->key ==  key)
			return root;
		if (root->key > key)
			root = root->left;
		else
			root = root->right;
	}
	return NULL;
}

/**
 * Insert an entry into the tree
 *
 * @param root The root of the tree
 * @param n The entry
 * @return 1 when successful, 0 if the entry already part of the tree 
 */
int
rb_insert_entry(struct rb_entry **root, struct rb_entry *n)
{
	struct rb_entry *parent = NULL, *gp, *r;
	struct rb_entry **rp = root;

	/* Find proper place in tree and link object in. */
	while (*rp) {
		unsigned long rkey = (*rp)->key;
		if (rkey == n->key)
			return 0;
		parent = *rp;
		if (rkey > n->key)
			rp = &(*rp)->left;
		else
			rp = &(*rp)->right;
	}
	(*rp) = n;
	n->left = n->right = NULL;
	n->parent = parent;
	n->col = ARB_RED;

fixup:

	/*
	 * At this point we have a node n and its parent where potentially
	 * both of them are red. Fix up.
	 */

	/* Try to assign a color without rotation */
	if (!parent) {
		n->col = ARB_BLACK;
		return 1;
	}
	if (parent->col == ARB_BLACK) {
		return 1;
	}
	/*
	 * parent exists and parent->col must be red. This also
	 * implies that parent->parent cannot be the root because the root
	 * is block. Thus parent->parent is not NULL.
	 * Additionally parent->parent cannot be red because one of its
	 * children is red.
	 */
	gp = parent->parent;
	/* gp cannot be NULL because the root is black but parent is red */
	if (gp->left && gp->right
	    && gp->left->col == ARB_RED && gp->right->col == ARB_RED) {
		if (gp != *root)
			gp->col = ARB_RED;
		gp->left->col = gp->right->col = ARB_BLACK;
		n = gp;
		parent = gp->parent;
		goto fixup;
	}
	/*
	 * The new node and its parent are both red. Rotate such that
	 * the two red nodes are either gp->left and gp->left->left or
	 * gp->right and gp->right->right.
	 */
	if (gp->left == parent && parent->right == n) {
		rotate_left(root, parent);
	} else if (gp->right == parent && parent->left == n) {
		rotate_right(root, parent);
	}
	/*
	 * Rotate the grand parent such that it is replaced by its red
	 * child. Then fixup colours.
	 */
	if (gp->left && gp->left->col == ARB_RED) {
		r = gp->left;
		rotate_right(root, gp);
	} else {
		r = gp->right;
		rotate_left(root, gp);
	}
	r->col = ARB_BLACK;
	r->left->col = r->right->col = ARB_RED;
	(*root)->col = ARB_BLACK;
	return 1;
}

/**
 * Swap two rb_entry structures in the tree. This must NOT copy the user
 * data but must instead relink the affected structures.
 * 
 * @param root The root of the tree
 * @param e1 The first entry
 * @param e2 The second entry
 */
static void
rb_swap(struct rb_entry **root, struct rb_entry *e1, struct rb_entry *e2)
{
	struct rb_entry *tmp;
	int e1root, e2root, e1left, e2left;
	short tmpcol;

	if (e1->parent == e2) {
		tmp = e1; e1 = e2; e2 = tmp;
	}

#define RBSWAP(FIELD) tmp = e1->FIELD; e1->FIELD = e2->FIELD; e2->FIELD = tmp;

	e1root = (e1->parent == NULL);
	e1left = (e1->parent && e1->parent->left == e1);
	e2root = (e2->parent == NULL);
	e2left = (e2->parent && e2->parent->left == e2);

	/* Color */
	tmpcol = e1->col;
	e1->col = e2->col;
	e2->col = tmpcol;

	if (e2->parent == e1) {
		/* Parent */
		e2->parent = e1->parent;
		e1->parent = e2;
		/* Left nodes */
		if (e1->left == e2) {
			e1->left = e2->left;
			e2->left = e1;
		} else {
			RBSWAP(left)
		}
		/* Right nodes */
		if (e1->right == e2) {
			e1->right = e2->right;
			e2->right = e1;
		} else {
			RBSWAP(right);
		}
	} else {
		/* Neither e2->parent == e1 nor e1->parent == e2 */
		RBSWAP(parent)
		RBSWAP(left)
		RBSWAP(right)
	}
#undef RBSWAP

	/* Pointers in e1 and e2 are properly setup. Fixup neighbours. */

	/* Parent nodes and root */
	if (e1root) {
		(*root) = e2;
	} else if (e1left) {
		e2->parent->left = e2;
	} else {
		e2->parent->right = e2;
	}
	if (e2root) {
		(*root) = e1;
	} else if (e2left) {
		e1->parent->left = e1;
	} else {
		e1->parent->right = e1;
	}
	if (e1->left)
		e1->left->parent = e1;
	if (e1->right)
		e1->right->parent = e1;
	if (e2->left)
		e2->left->parent = e2;
	if (e2->right)
		e2->right->parent = e2;
}

/**
 * Check if the children of an entry are black.
 * 
 * @param rb The entry which should be checked
 * @return 1 if both children are black, 0 otherwise
 */
static int
children_black(struct rb_entry *rb)
{
	if (rb->left && rb->left->col == ARB_RED)
		return 0;
	if (rb->right && rb->right->col == ARB_RED)
		return 0;
	return 1;
}

/**
 * Remove an entry from the tree.
 * 
 * @param root The root of the tree
 * @param e The entry which should be removed
 * @return 1 if the removal succeeds.
 */
int
rb_remove_entry(struct rb_entry **root, struct rb_entry *e)
{
	struct rb_entry		*swp, *cld, *p;
	int pleft;

	if (e->left) {
		swp = e->left;
		while(swp->right)
			swp = swp->right;
		rb_swap(root, e, swp);
	}
	/*
	 * At this point either e->left or e->right is NULL.
	 * Due to the restriction on black nodes on each path this
	 * means that the other child is either a single red node or NULL
	 * as well.
	 */
	/* Unlink e from the tree. At least e->left or e->right is NULL */
	cld = e->left;
	if (!cld)
		cld = e->right;
	if (cld)
		cld->parent = e->parent;
	if (e->parent == NULL) {
		*root = cld;
		pleft = -1;
	} else if (e->parent->left == e) {
		e->parent->left = cld;
		pleft = 1;
	} else {
		e->parent->right = cld;
		pleft = 0;
	}
	p = e->parent;
	/* If e was red we are done. */
	if (e->col == ARB_RED)
		return 1;
	/* If cld is red color it black and we are done. */
	if (cld && cld->col == ARB_RED) {
		cld->col = ARB_BLACK;
		return 1;
	}
	/* If the tree is now empty we are done. */
	if (*root == NULL)
		return 1;
	/*
	 * If we just replaced the root with its _single_ child color
	 * the root black and are done.
	 */
	if (*root == cld) {
		cld->col = ARB_BLACK;
		return 1;
	}

fixup:

	/*
	 * At this point we have a parent p where all the paths in one
	 * child tree lack one black node. pleft tells us if the left or
	 * the right subtree lacks nodes.
	 *
	 * Note that pleft implies p->right != NULL and !pleft implies
	 * p->left != NULL because the subtree of p that does not lack
	 * a black node on its paths must have at least one black real
	 * black node.
	 */

	/*
	 * Case 1: The other child is RED. Make it black.
	 * Note the a red child implies that p is BLACK. Thus the
	 * following rotations and recoloring does not change the black
	 * depths of any path.
	 */
	if (pleft && p->right->col == ARB_RED) {
		p->right->col = ARB_BLACK;
		p->col = ARB_RED;
		rotate_left(root, p);
	} else if (!pleft && p->left->col == ARB_RED) {
		p->left->col = ARB_BLACK;
		p->col = ARB_RED;
		rotate_right(root, p);
	}
	/*
	 * Case 2: The other child must be black. Handle the case where
	 *   its children are black, too.
	 */
	if (pleft && p->right->col == ARB_BLACK && children_black(p->right)) {
		if (p->col == ARB_BLACK) {
			p->right->col = ARB_RED;
			goto retry;
		} else {
			p->right->col = ARB_RED;
			p->col = ARB_BLACK;
			return 1;
		}
	} else if (!pleft && p->left->col == ARB_BLACK
	    && children_black(p->left)) {
		if (p->col == ARB_BLACK) {
			p->left->col = ARB_RED;
			goto retry;
		} else {
			p->left->col = ARB_RED;
			p->col = ARB_BLACK;
			return 1;
		}
	}
	/*
	 * Case 3: At this point we already know that the other child is
	 *   is black and that at least one of its children is not black.
	 */
	/*
	 * Case 3a: If necessary rotate such that the red sub child faces away
	 *   from the tree with lacking black nodes.
	 */
	if (pleft && (!p->right->right || p->right->right->col == ARB_BLACK)) {
		rotate_right(root, p->right);
		p->right->col = ARB_BLACK;
		p->right->right->col = ARB_RED;
	} else if (!pleft
	    && (!p->left->left || p->left->left->col == ARB_BLACK)) {
		rotate_left(root, p->left);
		p->left->col = ARB_BLACK;
		p->left->left->col = ARB_RED;
	}
	/*
	 * Case 3b: The other child is black and has a red child facing
	 *   away from the subtree with lacking black nodes.
	 */
	if (pleft) {
		p->right->right->col = ARB_BLACK;
		p->right->col = p->col;
		p->col = ARB_BLACK;
		rotate_left(root, p);
	} else {
		p->left->left->col = ARB_BLACK;
		p->left->col = p->col;
		p->col = ARB_BLACK;
		rotate_right(root, p);
	}
	if (*root)
		(*root)->col = ARB_BLACK;
	return 1;
retry:
	if (!p->parent)
		return 1;
	if (p->parent->left == p)
		pleft = 1;
	else
		pleft = 0;
	p = p->parent;
	goto fixup;
}

/**
 * Locate and remove an entry from the tree.
 * 
 * @param root The root of the tree
 * @param key The key which identifies the entry
 * @return 0 if no entry was found, 1 if the removal succeeds.
 */
int
rb_remove(struct rb_entry **root, unsigned long key)
{
	struct rb_entry *e = rb_find(*root, key);
	if (!e)
		return 0;
	return rb_remove_entry(root, e);
}

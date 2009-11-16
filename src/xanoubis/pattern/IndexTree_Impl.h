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

#ifndef _INDEXTREE_H_
#error Never include this file directly. Use IndexTree.h instead.
#endif

template<typename T>
IndexTree<T>::~IndexTree(void)
{
	free_link(root_);
	root_ = NULL;
}

template<typename T>
void
IndexTree<T>::free_link(rb_link *node)
{
	if (!node)
		return;
	free_link(node->left);
	free_link(node->right);
	delete(node);
}

template<typename T>
void
IndexTree<T>::fixup_size_one(rb_link *node)
{
	node->size = 1;
	if (node->left)
		node->size += node->left->size;
	if (node->right)
		node->size += node->right->size;
}

template<typename T>
void
IndexTree<T>::fixup_size(rb_link *node)
{
	while(node) {
		fixup_size_one(node);
		node = node->parent;
	}
}

/*
 * Rotate left, i.e. e->right replaces e:
 *         e                   r
 *        / \                 / \
 *       l   r      ==>      e   rr
 *          / \             / \
 *         rl rr           l  rl
 */
template<typename T>
void
IndexTree<T>::rotate_left(rb_link *e)
{
	rb_link *l, *r, *rl, *rr, *p;

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
		root_ = r;
	} else {
		if (p->left == e)
			p->left = r;
		else
			p->right = r;
	}
	fixup_size_one(e);
	fixup_size_one(r);
}

/*
 * Rotate right, i.e. e->left replaces e:
 *         e                   l
 *        / \                 / \
 *       l   r      ==>     ll   e
 *      / \                     / \
 *    ll  lr                  lr   r
 */

template<typename T>
void
IndexTree<T>::rotate_right(rb_link *e)
{
	rb_link *l, *r, *ll, *lr, *p;

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
		root_ = l;
	} else {
		if (p->left == e)
			p->left = l;
		else
			p->right = l;
	}
	fixup_size_one(e);
	fixup_size_one(l);
}

template<typename T>
T *
IndexTree<T>::find(const T *key) const
{
	rb_link	*root = root_;
	while(root) {
		int result = Cmp(key, root->data);
		if (result == 0)
			return root->data;
		if (result < 0)
			root = root->left;
		else
			root = root->right;
	}
	return NULL;
}

template<typename T>
bool
IndexTree<T>::insert(T *ndata)
{
	rb_link *parent = NULL, *gp, *r, *n;
	rb_link **rp = &root_;

	/* Find proper place in tree and link object in. */
	while (*rp) {
		int result = Cmp(ndata, ((*rp)->data));
		if (result == 0)
			return 0;
		parent = *rp;
		if (result < 0)
			rp = &(*rp)->left;
		else
			rp = &(*rp)->right;
	}
	n = new rb_link;
	n->data = ndata;
	(*rp) = n;
	n->left = n->right = NULL;
	n->parent = parent;
	n->col = RED;
	n->size = 1;
	fixup_size(n);

fixup:

	/*
	 * At this point we have a node n and its parent where potentially
	 * both of them are red. Fix up.
	 */

	/* Try to assign a color without rotation */
	if (!parent) {
		n->col = BLACK;
		return 1;
	}
	if (parent->col == BLACK) {
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
	    && gp->left->col == RED && gp->right->col == RED) {
		if (gp != root_)
			gp->col = RED;
		gp->left->col = gp->right->col = BLACK;
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
		rotate_left(parent);
	} else if (gp->right == parent && parent->left == n) {
		rotate_right(parent);
	}
	/*
	 * Rotate the grand parent such that it is replaced by its red
	 * child. Then fixup colours.
	 */
	if (gp->left && gp->left->col == RED) {
		r = gp->left;
		rotate_right(gp);
	} else {
		r = gp->right;
		rotate_left(gp);
	}
	r->col = BLACK;
	r->left->col = r->right->col = RED;
	root_->col = BLACK;
	return 1;
}

template<typename T>
bool
IndexTree<T>::children_black(rb_link *rb)
{
	if (rb->left && rb->left->col == RED)
		return false;
	if (rb->right && rb->right->col == RED)
		return false;
	return true;
}

template<typename T>
bool
IndexTree<T>::remove(const T *edata)
{
	rb_link		*swp, *cld, *p, *e = root_;
	int		 pleft;

	while(e) {
		int	result = Cmp(edata, e->data);
		if (result == 0)
			break;
		if (result < 0)
			e = e->left;
		else
			e = e->right;
	}
	if (e == NULL || e->data != edata)
		return false;
	if (e->left) {
		T	*tmp;
		swp = e->left;
		while(swp->right)
			swp = swp->right;
		tmp = e->data;
		e->data = swp->data;
		swp->data = tmp;
		e = swp;
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
		root_ = cld;
		pleft = -1;
	} else if (e->parent->left == e) {
		e->parent->left = cld;
		pleft = 1;
	} else {
		e->parent->right = cld;
		pleft = 0;
	}
	p = e->parent;
	if (p)
		fixup_size(p);
	/* If e was red we are done. */
	if (e->col == RED) {
		delete(e);
		return 1;
	}
	delete(e);
	/* If cld is red color it black and we are done. */
	if (cld && cld->col == RED) {
		cld->col = BLACK;
		return 1;
	}
	/* If the tree is now empty we are done. */
	if (root_ == NULL)
		return 1;
	/*
	 * If we just replaced the root with its _single_ child color
	 * the root black and are done.
	 */
	if (root_ == cld) {
		cld->col = BLACK;
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
	if (pleft && p->right->col == RED) {
		p->right->col = BLACK;
		p->col = RED;
		rotate_left(p);
	} else if (!pleft && p->left->col == RED) {
		p->left->col = BLACK;
		p->col = RED;
		rotate_right(p);
	}
	/*
	 * Case 2: The other child must be black. Handle the case where
	 *   its children are black, too.
	 */
	if (pleft && p->right->col == BLACK && children_black(p->right)) {
		if (p->col == BLACK) {
			p->right->col = RED;
			goto retry;
		} else {
			p->right->col = RED;
			p->col = BLACK;
			return 1;
		}
	} else if (!pleft && p->left->col == BLACK
	    && children_black(p->left)) {
		if (p->col == BLACK) {
			p->left->col = RED;
			goto retry;
		} else {
			p->left->col = RED;
			p->col = BLACK;
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
	if (pleft && (!p->right->right || p->right->right->col == BLACK)) {
		rotate_right(p->right);
		p->right->col = BLACK;
		p->right->right->col = RED;
	} else if (!pleft
	    && (!p->left->left || p->left->left->col == BLACK)) {
		rotate_left(p->left);
		p->left->col = BLACK;
		p->left->left->col = RED;
	}
	/*
	 * Case 3b: The other child is black and has a red child facing
	 *   away from the subtree with lacking black nodes.
	 */
	if (pleft) {
		p->right->right->col = BLACK;
		p->right->col = p->col;
		p->col = BLACK;
		rotate_left(p);
	} else {
		p->left->left->col = BLACK;
		p->left->col = p->col;
		p->col = BLACK;
		rotate_right(p);
	}
	if (root_)
		(root_)->col = BLACK;
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

template<typename T>
T *
IndexTree<T>::get(long int idx) const
{
	long int	 lsize;
	rb_link		*node = root_;

	if (!node || idx >= node->size || idx < 0)
		return NULL;

	while(1) {
		lsize = 0;
		if (node->left)
			lsize += node->left->size;
		if (idx < lsize) {
			node = node->left;
		} else if (idx == lsize) {
			return node->data;
		} else {
			idx -= lsize + 1;
			node = node->right;
		}
	}
}

template<typename T>
long int
IndexTree<T>::index_of(const T *entry) const
{
	long int	 before = 0;
	rb_link		*node = root_;

	while(node) {
		int result = Cmp(entry, node->data);
		if (result == 0) {
			if (node->left)
				before += node->left->size;
			return before;
		} else if (result < 0) {
			node = node->left;
		} else {
			if (node->left)
				before += node->left->size;
			before++;
			node = node->right;
		}
	}
	/* If we end up here, @entry was not found in the tree. */
	return -1;
}

template<typename T>
long int
IndexTree<T>::verify_one(const rb_link *t, const T *mi, const T *ma) const
{
	long int d1, d2;
	long int sz;
	if (!t)
		return 1;
	assert(mi == NULL || Cmp(mi, t->data) < 0);
	assert(ma == NULL || Cmp(t->data, ma) < 0);
	assert(t->col == RED || t->col == BLACK);
	if (t->left)
		assert(t->left->parent == t);
	if (t->right)
		assert(t->right->parent == t);
	if (t->col == RED) {
		assert(!t->left || t->left->col == BLACK);
		assert(!t->right || t->right->col == BLACK);
	}
	d1 = verify_one(t->left, mi, t->data);
	d2 = verify_one(t->right, t->data, ma);
	assert(d1 == d2);
	if (t->col == BLACK)
		d1++;
	sz = 1;
	if (t->left)
		sz += t->left->size;
	if (t->right)
		sz += t->right->size;
	assert(sz == t->size);
	return d1;
}

template<typename T>
void
IndexTree<T>::verify(void) const
{
	int	i = 0;
	if (!root_)
		return;
	assert(root_->parent == NULL);
	assert(root_->col == BLACK);
	verify_one(root_, NULL, NULL);
	while(1) {
		T * e = get(i);
		if (e == NULL)
			break;
		assert(index_of(e) == i);
		assert(find(e) == e);
		i++;
	}
	assert(i == size());
}

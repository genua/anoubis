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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <anoubis_rbtree.h>

static int
verify_one(struct rb_entry *t, unsigned long mi, unsigned long ma)
{
	int d1, d2;
	if (!t)
		return 1;
	assert(mi <= t->key && t->key <= ma);
	assert(t->col == ARB_RED || t->col == ARB_BLACK);
	if (t->left)
		assert(t->left->parent == t);
	if (t->right)
		assert(t->right->parent == t);
	if (t->col == ARB_RED) {
		assert(!t->left || t->left->col == ARB_BLACK);
		assert(!t->right || t->right->col == ARB_BLACK);
	}
	d1 = verify_one(t->left, mi, t->key-1);
	d2 = verify_one(t->right, t->key+1, ma);
	assert(d1 == d2);
	if (t->col == ARB_BLACK)
		d1++;
	return d1;
}

static void
rb_verify(struct rb_entry *root)
{
	if (!root)
		return;
	assert(root->parent == NULL);
	assert(root->col == ARB_BLACK);
	verify_one(root, 0, (unsigned long)-1);
}

static int
rb_count(struct rb_entry *root)
{
	if (!root)
		return 0;
	return 1 + rb_count(root->left) + rb_count(root->right);
}

#define NENT	10000
int main()
{
	struct rb_entry *root = NULL;
	struct rb_entry e[NENT];
	int cnt = 0, try;
	int i;

	rb_verify(root);
	for (i=0; i<NENT; ++i) {
		e[i].key = -1;
	}
	for (try = 0; cnt || try < 1000000; ++try) {
		int idx = rand() % NENT;
		if (try > 1000000)
			for (; e[idx].key == (unsigned int)-1;
			    idx = (idx+1) % NENT);
		if (e[idx].key == (unsigned int)-1) {
			e[idx].key = idx;
			assert(rb_insert_entry(&root, e+idx));
			cnt++;
		} else {
			assert(rb_remove_entry(&root, e+idx));
			e[idx].key = -1;
			cnt--;
		}
		if (cnt < 1000 || try % 100 == 0) {
			rb_verify(root);
			assert(rb_count(root) == cnt);
		}
	}
	return 0;
}

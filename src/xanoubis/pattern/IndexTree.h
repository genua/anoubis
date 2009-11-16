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
#define _INDEXTREE_H_

#include <cstdlib>

template<typename T>
class IndexTree {
private:
	enum color {
		RED,
		BLACK,
	};
	class rb_link {
	public:
		T *data;
		rb_link *left, *right, *parent;
		color col;
		long int size;
	};
public:
	/**
	 * Constructor.
	 * Komplexity: O(1)
	 */
	IndexTree(void) : root_(NULL) {
	};

	/**
	 * Destructor. This only frees meta data maintained by the tree,
	 * it does not free the objects themselves.
	 * Komplexity: O(N)
	 */
	virtual ~IndexTree(void);

	/**
	 * Insert the Entry pointed to be @entry into the tree. The caller
	 * is responsible for the allocation of T.
	 * @param[in] 1st The new entry.
	 * @return True if inserting was successful.
	 * Komplexity: O(log(N))
	 */
	bool insert(T *entry);

	/**
	 * Remove the entry pointed to by @entry from the tree. The object
	 * pointed to by @entry must be in the tree. It is _not_ sufficient
	 * if some other object that compares equal to @entry is in the tree.
	 * This function does not free the object itself. This is the
	 * responsibility of the caller.
	 * @param[in] 1st The entry.
	 * @return True if the object could be removed from the tree.
	 * Komplexit: O(log(N))
	 */
	bool remove(const T *entry);

	/**
	 * Find an object that compares equal to @entry in the tree. Return
	 * a pointer to the object fount.
	 * @param[in] 1st The entry that the function should find.
	 * @return A pointer to the object in the tree or NULL.
	 * Komplexity: O(log(N))
	 */
	T *find(const T *entry) const;

	/**
	 * Find the object with number @idx in the tree. Counting starts
	 * at 0.
	 * @param[in] 1st The index.
	 * @return The object if the index is within range or NULL.
	 * Komplexity: O(log(N))
	 */
	T *get(long int idx) const;

	/**
	 * Find the index of the entry in the tree that compares equal to
	 * @entry.
	 * @param[in] 1st The object.
	 * @return The index starting at 0 or -1 if the object was not found.
	 * Komplexity: O(log(N))
	 */
	long int index_of(const T *entry) const;

	/**
	 * Return the total number of elements in the tree.
	 * @param None.
	 * @return The Total number of elements.
	 * Komplexity: O(1)
	 */
	long int size(void) const {
		return root_?root_->size:0;
	}

	/**
	 * Verify the integrity of the tree. Throws an assertion if the
	 * integrity is violated.
	 * Komplexity: O(N*log(N))
	 */
	void verify(void) const;

	/* Simple iterator class. */
	class iterator {
	private:
		rb_link	*node_;
	public:
		iterator(void) : node_(NULL) { };
		iterator(rb_link *n) : node_(n) { };
		T * operator*() const {
			return node_->data;
		};
		iterator &operator++() {
			if (node_ == NULL)
				return *this;
			if (node_->right) {
				node_ = node_->right;
				while(node_->left)
					node_ = node_->left;
			} else {
				while(node_->parent
				    && node_->parent->right == node_) {
					node_ = node_->parent;
				}
				node_ = node_->parent;
			}
			return *this;
		};
		bool operator==(const iterator &other) const {
			return other.node_ == this->node_;
		};
		bool operator!=(const iterator &other) const {
			return other.node_ != this->node_;
		}
	};

	/*
	 * Return an iterator that points to the first element in the
	 * container.
	 * Komplexity: O(log(N))
	 */
	iterator begin(void) {
		rb_link	*node = root_;
		while(node && node->left)
			node = node->left;
		return iterator(node);
	}

	/*
	 * Return an iterator that points one past the end of the last
	 * element in the container.
	 * Komplexity: O(1)
	 */
	iterator end(void) {
		return iterator(NULL);
	}

private:
	/**
	 * Private, pure virtual comparision function. Override this
	 * in your derived class with  a suitable comparision function.
	 */
	virtual int Cmp(const T *a, const T *b) const = 0;

	/**
	 * Verify the integrity of all nodes in the tree rooted at @entry.
	 * @param[in] 1st The root of the tree.
	 * @param[in] 2nd If not NULL all objects in the subtree must
	 *     be strictly greater than this object.
	 * @param[in] 3rd If not NULL all objects in the subtree must
	 *     be strictly smaller than this object.
	 * Komplexity: O(#Entries in sub tree)
	 */
	long int verify_one(const rb_link *entry, const T *, const T *) const;

	/* Left and right rotation of the tree rooted at @root_ */
	void rotate_left(rb_link *node);
	void rotate_right(rb_link *node);
	bool children_black(rb_link *node);

	/* root of the red black tree. */
	rb_link *root_;

	/* Helper function for the destructor. */
	void free_link(rb_link *);

	/* Functions to fixup tree size after modifications. */
	void fixup_size(rb_link *);
	void fixup_size_one(rb_link *);
};

#include "IndexTree_Impl.h"

#endif /* _INDEXTREE_H_ */

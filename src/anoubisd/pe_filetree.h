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

#ifndef _PE_FILETREE_H_
#define _PE_FILETREE_H_

#include <sys/tree.h>
#include "pe.h"

/**
 * A node in file tree. The node contains a path name and the task cookie
 * of the task that modified the file. The path name is dynamically
 * allocated.
 */
struct pe_filetree_node {
	/**
	 * The link for the node in the surrounding file tree.
	 */
	RB_ENTRY(pe_filetree_node)	 entry;

	/**
	 * The path name of the file.
	 */
	char			*path;

	/**
	 * The task cookie of the task that modified the file.
	 */
	anoubis_cookie_t	 task_cookie;

	/**
	 * A hash value of the file name. This is used to simplify
	 * file name comparisions.
	 */
	unsigned long		 idx;
};

/**
 * The root of a file tree. A file tree contains file names associated
 * with task cookies in a red black tree that allows efficient access
 * based on file names.
 */
struct pe_file_tree {
	RB_HEAD(rb_file_tree, pe_filetree_node) head;
};

/* Prototypes */
struct pe_file_tree	*pe_filetree_create(void);
void			 pe_filetree_destroy(struct pe_file_tree *);
int			 pe_filetree_insert(struct pe_file_tree *,
			     char *, anoubis_cookie_t);
void			 pe_filetree_remove(struct pe_file_tree *,
			     struct pe_filetree_node *);
struct pe_filetree_node	*pe_filetree_find(struct pe_file_tree *, char *);
struct pe_filetree_node	*pe_filetree_start(struct pe_file_tree *);
struct pe_filetree_node	*pe_filetree_next(struct pe_file_tree *,
			     struct pe_filetree_node *);

#endif	/* _PE_FILETREE_H_ */

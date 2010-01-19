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

struct pe_file_node {
	RB_ENTRY(pe_file_node)	 entry;
	char			*path;
	anoubis_cookie_t	 task_cookie;
	unsigned long		 idx;
};

struct pe_file_tree {
	RB_HEAD(rb_file_tree, pe_file_node) head;
};

struct pe_file_tree *pe_init_filetree(void);
int pe_insert_node(struct pe_file_tree *, char *, anoubis_cookie_t);
struct pe_file_node *pe_find_file(struct pe_file_tree *, char *);
struct pe_file_node *pe_filetree_next(struct pe_file_tree *,
    struct pe_file_node *);
struct pe_file_node *pe_filetree_start(struct pe_file_tree *);
void pe_delete_file(struct pe_file_tree *, char *);
void pe_delete_node(struct pe_file_tree *, struct pe_file_node *);
void pe_filetree_destroy(struct pe_file_tree *);

#endif	/* _PE_FILETREE_H_ */

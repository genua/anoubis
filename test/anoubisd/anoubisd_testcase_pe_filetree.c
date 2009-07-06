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

#include <config.h>
#include <pe_filetree.h>
#include <check.h>
#include <string.h>

START_TEST(tc_filetree_nomal)
{
	struct pe_file_tree *f = NULL;
	struct pe_file_node *n = NULL;
	anoubis_cookie_t c = 1234;
	char name[] = "/var/lib/bla";
	int rc = 0;

	f = pe_init_filetree();
	fail_if(f == NULL, "pe_init_filetree failed");

	rc = pe_insert_node(f, name, c);
	fail_if (rc < 0, "pe_insert_node failed");

	n = pe_find_file(f, name);
	fail_if(n == NULL, "Couldn't find file in tree");

	pe_filetree_destroy(f);
}
END_TEST

START_TEST(tc_filetree_collision)
{
	struct pe_file_tree *f = NULL;
	struct pe_file_node *n = NULL;
	anoubis_cookie_t c = 1234;
	int rc = 0;

	f = pe_init_filetree();
	fail_if(f == NULL, "pe_init_filetree failed");

	/* These strings have all the same hashvalue with our
	 * hash_fn and should be able to be insert and find.
	 */
	rc = pe_insert_node(f, "cy", c);
	fail_if (rc < 0, "pe_insert_node failed");

	rc = pe_insert_node(f, "dw", c);
	fail_if (rc < 0, "pe_insert_node failed");

	rc = pe_insert_node(f, "eu", c);
	fail_if (rc < 0, "pe_insert_node failed");

	n = pe_find_file(f, "cy");
	fail_if(n == NULL, "couldn't finde 'cy' in tree");

	n = pe_find_file(f, "dw");
	fail_if(n == NULL, "couldn't find 'dw' in tree");

	n = pe_find_file(f, "eu");
	fail_if(n == NULL, "couldn't find eu in tree");

	/* where else the same string shouldn't be insert */
	rc = pe_insert_node(f, "cy", c);
	fail_if(rc == 0, "cy allready in the tree and shouldn't be inserted");

	pe_filetree_destroy(f);
}
END_TEST

START_TEST(tc_filetree_failcases)
{
	struct pe_file_tree *f = NULL;
	struct pe_file_node *n = NULL;
	anoubis_cookie_t c = 1234;
	char name[] = "/var/lib/bla";
	int rc = 0;

	f = pe_init_filetree();
	fail_if(f == NULL, "pe_init_filetree failed");

	rc = pe_insert_node(f, NULL, c);
	fail_if (rc == 0, "pe_insert_node should fail");

	rc = pe_insert_node(NULL, name, c);
	fail_if (rc == 0, "pe_insert_node should fail");

	rc = pe_insert_node(NULL, NULL, c);
	fail_if (rc == 0, "pe_insert_node should fail");

	rc = pe_insert_node(f, name, c);
	fail_if (rc < 0, "pe_insert_node failed");

	n = pe_find_file(NULL, name);
	fail_if(n != NULL, "Shouldn't find file in tree");

	n = pe_find_file(f, NULL);
	fail_if(n != NULL, "Shouldn't find file in tree");

	n = pe_find_file(NULL, NULL);
	fail_if(n != NULL, "Shouldn't find file in tree");

	n = pe_find_file(f, name);
	fail_if(n == NULL, "pe_find_file failed");

	pe_filetree_destroy(f);
}
END_TEST

START_TEST(tc_filetree_iteration)
{
	struct pe_file_tree *f = NULL;
	struct pe_file_node *n = NULL;
	anoubis_cookie_t c = 1234;
	int rc = 0, i = 0,
	    cy = 0, dw = 0, eu = 0;

	f = pe_init_filetree();
	fail_if(f == NULL, "pe_init_filetree failed");

	rc = pe_insert_node(f, "cy", c);
	fail_if (rc < 0, "pe_insert_node failed");

	rc = pe_insert_node(f, "dw", c);
	fail_if (rc < 0, "pe_insert_node failed");

	rc = pe_insert_node(f, "eu", c);
	fail_if (rc < 0, "pe_insert_node failed");

	for (n = pe_filetree_start(f); n != NULL; n = pe_filetree_next(f, n)) {
		fail_if(n == NULL, "Error while iteration");
		if (!strcmp(n->path, "cy"))
			cy++;
		else if (!strcmp(n->path, "dw"))
			dw++;
		else if (!strcmp(n->path, "eu"))
			eu++;
		else
			fail("Unexpected path %s", n->path);
		i++;
	}
	fail_if(i != 3, "Couldn't iterated trought the tree %d", i);
	fail_if(cy != 1, "cy should be 1 not %d", cy);
	fail_if(dw != 1, "dw should be 1 not %d", cy);
	fail_if(eu != 1, "eu should be 1 not %d", cy);

	n = pe_filetree_start(NULL);
	fail_if(n != NULL, "Shouldn't get a start node");

	n = pe_filetree_next(f, NULL);
	fail_if(n != NULL, "Shouldn't get a next node");

	pe_filetree_destroy(f);
}
END_TEST

TCase *
anoubisd_testcase_pe_filetree(void)
{
	TCase *tc_filetree = tcase_create("File Tree");

	tcase_set_timeout(tc_filetree, 10);
	tcase_add_test(tc_filetree, tc_filetree_nomal);
	tcase_add_test(tc_filetree, tc_filetree_collision);
	tcase_add_test(tc_filetree, tc_filetree_failcases);
	tcase_add_test(tc_filetree, tc_filetree_iteration);

	return (tc_filetree);
}

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

#include <check.h>
#include "PlaygroundFileEntry.h"

START_TEST(entry_init_verify)
{
	/* create an instance */
	wxString path1 = wxT("Hello");
	wxString path2 = wxT("World");
	PlaygroundFileEntry* entry =
	    new PlaygroundFileEntry(1, 2, 3);
	entry->addPath(path1);
	entry->addPath(path2);

	/* make sure that all getters return what we created */
	fail_if(entry->getPgid() != 1);
	fail_if(entry->getDevice() != 2);
	fail_if(entry->getInode() != 3);
	fail_if(entry->getPaths()[0].Cmp(path1) != 0);
	fail_if(entry->getPaths()[1].Cmp(path2) != 0);

	delete(entry);
}
END_TEST

TCase *
getTc_PlaygroundFileEntry(void)
{
	TCase *testCase;

	testCase = tcase_create("PlaygroundFileEntry");
	tcase_add_test(testCase, entry_init_verify);

	return (testCase);
}

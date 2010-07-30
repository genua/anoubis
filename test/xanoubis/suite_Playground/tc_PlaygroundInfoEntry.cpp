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
#include "PlaygroundInfoEntry.h"


START_TEST(entry_init_verify)
{
	/* create an instance */
	wxDateTime t = wxDateTime::Now();
	wxString path = wxT("Hello world");
	PlaygroundInfoEntry* entry = new PlaygroundInfoEntry(
	    1, 2, t, true, 4, path);

	/* make sure that all getters return what we created */
	fail_if(entry->getPgid() != 1);
	fail_if(entry->getUid() != 2);
	fail_if(! entry->getStarttime().IsEqualTo(t));
	fail_if(entry->isActive() != true);
	fail_if(entry->getNumFiles() != 4);
	fail_if(entry->getPath().Cmp(path) != 0);

	delete(entry);
}
END_TEST

TCase *
getTc_PlaygroundInfoEntry(void)
{
	TCase *testCase;

	testCase = tcase_create("PlaygroundInfoEntry");
	tcase_add_test(testCase, entry_init_verify);

	return (testCase);
}
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

#include <sys/wait.h>

#include <check.h>
#include <stdlib.h>

#include <model/SfsDirectory.h>

char		sfsdir[64];
wxString	wxSfsDir;

static int
tc_exec(const char* cmd, ...)
{
	va_list ap;
	char str[1024], syscmd[2048];
	int rc;

	va_start(ap, cmd);
	vsnprintf(str, sizeof(str), cmd, ap);
	va_end(ap);

	snprintf(syscmd, sizeof(syscmd), "(%s) >/dev/null 2>&1", str);

	rc = system(syscmd);
	return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

void
setup()
{
	char *s;

	strcpy(sfsdir, "/tmp/sfsdir_XXXXXX");
	s = mkdtemp(sfsdir);

	tc_exec("touch \"%s/file1\"", sfsdir);
	tc_exec("touch \"%s/file2\"", sfsdir);
	tc_exec("touch \"%s/file3\"", sfsdir);
	tc_exec("touch \"%s/fiel4\"", sfsdir);
	tc_exec("mkdir \"%s/sub\"", sfsdir);
	tc_exec("touch \"%s/sub/file1\"", sfsdir);
	tc_exec("touch \"%s/sub/file2\"", sfsdir);
	tc_exec("touch \"%s/sub/file3\"", sfsdir);
	tc_exec("touch \"%s/sub/file4\"", sfsdir);

	wxSfsDir = wxString(sfsdir, wxConvFile);
}

void
teardown()
{
	tc_exec("rm -rf \"%s\"", sfsdir);
}

START_TEST(SfsDir_existing)
{
	SfsDirectory	dir;
	bool		result;
	int		num;

	result = dir.setPath(wxSfsDir);
	fail_if(result == false, "Path has not changed");

	num = dir.getNumEntries();
	fail_if(num != 4, "Three entries expected, but have %i", num);
}
END_TEST

START_TEST(SfsDir_non_existing)
{
	SfsDirectory	dir;
	bool		result;
	int		num;

	result = dir.setPath(wxSfsDir + wxT("/baz"));
	fail_if(result == false, "Path has not changed");

	num = dir.getNumEntries();
	fail_if(num > 0, "No entries expected, but have %i", num);
}
END_TEST

START_TEST(SfsDir_filter_match)
{
	SfsDirectory	dir;
	bool		result;
	int		num;

	result = dir.setPath(wxSfsDir);
	fail_if(result == false, "Path has not changed");

	result = dir.setFilter(wxT("file"));
	fail_if(result == false, "Filter has not changed");

	num = dir.getNumEntries();
	fail_if(num != 3, "Three entries expected, but have %i", num);
}
END_TEST

START_TEST(SfsDir_filter_no_match)
{
	SfsDirectory	dir;
	bool		result;
	int		num;

	result = dir.setPath(wxSfsDir);
	fail_if(result == false, "Path has not changed");

	result = dir.setFilter(wxT("jhasfas"));
	fail_if(result == false, "Filter has not changed");

	num = dir.getNumEntries();
	fail_if(num != 0, "No entries expected, but have %i", num);
}
END_TEST

START_TEST(SfsDir_inverse_filter_match)
{
	SfsDirectory	dir;
	bool		result;
	int		num;

	result = dir.setPath(wxSfsDir);
	fail_if(result == false, "Path has not changed");

	result = dir.setFilter(wxT("file"));
	fail_if(result == false, "Filter has not changed");

	result = dir.setFilterInversed(true);
	fail_if(result == false, "Filter-inverse-setting has not changed");

	num = dir.getNumEntries();
	fail_if(num != 1, "One entry expected, but have %i", num);
}
END_TEST

START_TEST(SfsDir_inverse_filter_no_match)
{
	SfsDirectory	dir;
	bool		result;
	int		num;

	result = dir.setPath(wxSfsDir);
	fail_if(result == false, "Path has not changed");

	result = dir.setFilter(wxT("fi"));
	fail_if(result == false, "Filter has not changed");

	result = dir.setFilterInversed(true);
	fail_if(result == false, "Filter-inverse-setting has not changed");

	num = dir.getNumEntries();
	fail_if(num != 0, "No entries expected, but have %i", num);
}
END_TEST

START_TEST(SfsDir_entry_order)
{
	SfsDirectory	dir;
	bool		result;
	unsigned int	i;
	wxString	prevName = wxEmptyString;

	result = dir.setPath(wxSfsDir);
	fail_if(result == false, "Path has not changed");

	for (i = 0; i < dir.getNumEntries(); i++) {
		SfsEntry &entry = dir.getEntry(i);

		if (entry.getFileName() <= prevName)
			fail("Entries on directory are not correct order");

		prevName = entry.getFileName();
	}
}
END_TEST

START_TEST(SfsDir_recursive)
{
	SfsDirectory	dir;
	bool		result;
	bool		fileFound[8];

	result = dir.setPath(wxSfsDir);
	fail_unless(result, "Path has not changed");

	result = dir.setDirTraversal(true);
	fail_unless(result, "Failed to enabled dir-traversal");

	fail_unless(dir.getNumEntries() == 8,
	    "Unexpected number of entries\n"
	    "Is: %i\n"
	    "Expected: 8", dir.getNumEntries());

	for (unsigned int i = 0; i < sizeof(fileFound); i++)
		fileFound[i] = false;

	for (unsigned int i = 0; i < dir.getNumEntries(); i++) {
		SfsEntry &entry = dir.getEntry(i);

		if (entry.getRelativePath(wxSfsDir) == wxT("file1"))
			fileFound[0] = true;
		else if (entry.getRelativePath(wxSfsDir) == wxT("file2"))
			fileFound[1] = true;
		else if (entry.getRelativePath(wxSfsDir) == wxT("file3"))
			fileFound[2] = true;
		else if (entry.getRelativePath(wxSfsDir) == wxT("fiel4"))
			fileFound[3] = true;
		else if (entry.getRelativePath(wxSfsDir) == wxT("sub/file1"))
			fileFound[4] = true;
		else if (entry.getRelativePath(wxSfsDir) == wxT("sub/file2"))
			fileFound[5] = true;
		else if (entry.getRelativePath(wxSfsDir) == wxT("sub/file3"))
			fileFound[6] = true;
		else if (entry.getRelativePath(wxSfsDir) == wxT("sub/file4"))
			fileFound[7] = true;
		else
			fail("Unexpected entry fetched: %s",
			    (const char*)entry.getPath().fn_str());
	}

	for (unsigned int i = 0; i < sizeof(fileFound); i++)
		fail_unless(fileFound[i], "File at index %i not found", i);
}
END_TEST

START_TEST(SfsDir_recursive_filtered)
{
	SfsDirectory	dir;
	bool		result;
	bool		fileFound[2];

	result = dir.setPath(wxSfsDir);
	fail_unless(result, "Path has not changed");

	result = dir.setDirTraversal(true);
	fail_unless(result, "Failed to enabled dir-traversal");

	result = dir.setFilter(wxT("1"));
	fail_unless(result, "Failed to update the filter");

	fail_unless(dir.getNumEntries() == 2,
	    "Unexpected number of entries\n"
	    "Is: %i\n"
	    "Expected: 2", dir.getNumEntries());

	for (unsigned int i = 0; i < sizeof(fileFound); i++)
		fileFound[i] = false;

	for (unsigned int i = 0; i < dir.getNumEntries(); i++) {
		SfsEntry &entry = dir.getEntry(i);

		if (entry.getRelativePath(wxSfsDir) == wxT("file1"))
			fileFound[0] = true;
		else if (entry.getRelativePath(wxSfsDir) == wxT("sub/file1"))
			fileFound[1] = true;
		else
			fail("Unexpected entry fetched: %s",
			    (const char*)entry.getPath().fn_str());
	}

	for (unsigned int i = 0; i < sizeof(fileFound); i++)
		fail_unless(fileFound[i], "File at index %i not found", i);
}
END_TEST

TCase *
getTc_SfsDir(void)
{
	TCase *testCase;

	testCase = tcase_create("SfsDir");
	tcase_add_checked_fixture(testCase, setup, teardown);

	tcase_add_test(testCase, SfsDir_existing);
	tcase_add_test(testCase, SfsDir_non_existing);
	tcase_add_test(testCase, SfsDir_filter_match);
	tcase_add_test(testCase, SfsDir_filter_no_match);
	tcase_add_test(testCase, SfsDir_inverse_filter_match);
	tcase_add_test(testCase, SfsDir_inverse_filter_no_match);
	tcase_add_test(testCase, SfsDir_entry_order);
	tcase_add_test(testCase, SfsDir_recursive);
	tcase_add_test(testCase, SfsDir_recursive_filtered);

	return (testCase);
}

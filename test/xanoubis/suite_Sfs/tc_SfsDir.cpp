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

#include <wx/app.h>

#include <model/SfsDirectory.h>
#include <model/SfsEntry.h>

char		sfsdir[64];
wxString	wxSfsDir;
wxApp		*tcApp;

class tc_SfsDir_ScanHandler : public SfsDirectoryScanHandler
{
	public:
		tc_SfsDir_ScanHandler(SfsDirectory *dir, bool abort)
		{
			dir_ = dir;
			abort_ = abort;

			startInvocations_ = 0;
			finishedInvocations_ = 0;
			progressInvocations_ = 0;
			scanAborted_ = false;
			maxVisited_ = 0;
			maxTotal_ = 0;
		}

		void scanStarts()
		{
			startInvocations_++;

			if (dir_ && abort_) {
				dir_->abortScan();
			}
		}

		void scanFinished(bool aborted)
		{
			finishedInvocations_++;
			scanAborted_ = aborted;
		}

		void scanProgress(unsigned long visited, unsigned long total)
		{
			progressInvocations_++;

			if (visited > maxVisited_)
				maxVisited_ = visited;
			if (total > maxTotal_)
				maxTotal_ = total;
		}

		unsigned int startInvocations_;
		unsigned int finishedInvocations_;
		unsigned int progressInvocations_;
		bool scanAborted_;
		unsigned long maxVisited_;
		unsigned long maxTotal_;

	private:
		SfsDirectory *dir_;
		bool abort_;
};

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
	tc_exec("ln -s \"%s/file1\" \"%s/link_file1\"", sfsdir, sfsdir);
	tc_exec("mkdir \"%s/sub\"", sfsdir);
	tc_exec("touch \"%s/sub/file1\"", sfsdir);
	tc_exec("touch \"%s/sub/file2\"", sfsdir);
	tc_exec("touch \"%s/sub/file3\"", sfsdir);
	tc_exec("touch \"%s/sub/file4\"", sfsdir);

	wxSfsDir = wxString(sfsdir, wxConvFile);

	tcApp = new wxApp;
	wxApp::SetInstance(tcApp);
}

void
teardown()
{
	wxApp::SetInstance(0);
	delete tcApp;

	tc_exec("rm -rf \"%s\"", sfsdir);
}

START_TEST(SfsDir_existing)
{
	SfsDirectory	dir;
	bool		result;
	int		num;

	result = dir.setPath(wxSfsDir);
	fail_if(result == false, "Path has not changed");

	dir.scanLocalFilesystem();

	num = dir.getNumEntries();
	fail_if(num != 5, "Five entries expected, but have %i", num);
}
END_TEST

START_TEST(SfsDir_non_existing)
{
	SfsDirectory	dir;
	bool		result;
	int		num;

	result = dir.setPath(wxSfsDir + wxT("/baz"));
	fail_if(result == false, "Path has not changed");

	dir.scanLocalFilesystem();

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

	dir.scanLocalFilesystem();

	num = dir.getNumEntries();
	fail_if(num != 4, "Four entries expected, but have %i", num);
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

	dir.scanLocalFilesystem();

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

	dir.scanLocalFilesystem();

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

	dir.scanLocalFilesystem();

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

	dir.scanLocalFilesystem();

	for (i = 0; i < dir.getNumEntries(); i++) {
		SfsEntry *entry = dir.getEntry(i);

		if (entry->getFileName() <= prevName)
			fail("Entries on directory are not correct order");

		prevName = entry->getFileName();
	}
}
END_TEST

START_TEST(SfsDir_recursive)
{
	SfsDirectory	dir;
	bool		result;
	bool		fileFound[9];

	result = dir.setPath(wxSfsDir);
	fail_unless(result, "Path has not changed");

	result = dir.setDirTraversal(true);
	fail_unless(result, "Failed to enabled dir-traversal");

	dir.scanLocalFilesystem();

	fail_unless(dir.getNumEntries() == 9,
	    "Unexpected number of entries\n"
	    "Is: %i\n"
	    "Expected: 9", dir.getNumEntries());

	for (unsigned int i = 0; i < sizeof(fileFound); i++)
		fileFound[i] = false;

	for (unsigned int i = 0; i < dir.getNumEntries(); i++) {
		SfsEntry *entry = dir.getEntry(i);

		if (entry->getRelativePath(wxSfsDir) == wxT("file1"))
			fileFound[0] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("file2"))
			fileFound[1] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("file3"))
			fileFound[2] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("fiel4"))
			fileFound[3] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("link_file1"))
			fileFound[4] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("sub/file1"))
			fileFound[5] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("sub/file2"))
			fileFound[6] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("sub/file3"))
			fileFound[7] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("sub/file4"))
			fileFound[8] = true;
		else
			fail("Unexpected entry fetched: %ls",
			    entry->getPath().c_str());
	}

	for (unsigned int i = 0; i < sizeof(fileFound); i++)
		fail_unless(fileFound[i], "File at index %i not found", i);
}
END_TEST

START_TEST(SfsDir_recursive_filtered)
{
	SfsDirectory	dir;
	bool		result;
	bool		fileFound[3];

	result = dir.setPath(wxSfsDir);
	fail_unless(result, "Path has not changed");

	result = dir.setDirTraversal(true);
	fail_unless(result, "Failed to enabled dir-traversal");

	result = dir.setFilter(wxT("1"));
	fail_unless(result, "Failed to update the filter");

	dir.scanLocalFilesystem();

	fail_unless(dir.getNumEntries() == 3,
	    "Unexpected number of entries\n"
	    "Is: %i\n"
	    "Expected: 3", dir.getNumEntries());

	for (unsigned int i = 0; i < sizeof(fileFound); i++)
		fileFound[i] = false;

	for (unsigned int i = 0; i < dir.getNumEntries(); i++) {
		SfsEntry *entry = dir.getEntry(i);

		if (entry->getRelativePath(wxSfsDir) == wxT("file1"))
			fileFound[0] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("link_file1"))
			fileFound[1] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("sub/file1"))
			fileFound[2] = true;
		else
			fail("Unexpected entry fetched: %ls",
			    entry->getPath().c_str());
	}

	for (unsigned int i = 0; i < sizeof(fileFound); i++)
		fail_unless(fileFound[i], "File at index %i not found", i);
}
END_TEST

START_TEST(SfsDir_resolve_link_ok)
{
	SfsDirectory	dir;
	bool		result;

	result = dir.setPath(wxSfsDir);
	fail_unless(result, "Path has not changed");

	dir.scanLocalFilesystem();

	int idx = dir.getIndexOf(wxSfsDir + wxT("/link_file1"));
	fail_unless(idx >= 0, "File not found in model");

	SfsEntry *entry = dir.getEntry(idx);
	wxString resolved = entry->resolve();
	wxString expected = wxSfsDir + wxT("/file1");

	fail_unless(resolved == expected,
	    "Unexpected resolve-result\n"
	    "Is: %ls\n"
	    "Expected: %ls",
	    resolved.c_str(), expected.c_str());
}
END_TEST

START_TEST(SfsDir_resolve_link_plain_file)
{
	SfsDirectory	dir;
	bool		result;

	result = dir.setPath(wxSfsDir);
	fail_unless(result, "Path has not changed");

	dir.scanLocalFilesystem();

	int idx = dir.getIndexOf(wxSfsDir + wxT("/file1"));
	fail_unless(idx >= 0, "File not found in model");

	SfsEntry *entry = dir.getEntry(idx);
	wxString resolved = entry->resolve();

	fail_unless(resolved.IsEmpty(),
	    "Unexpected resolve-result\n"
	    "Empty string expected\n"
	    "Is: %ls", resolved.c_str());
}
END_TEST

START_TEST(SfsDir_filter_rel_path_only)
{
	/* @see Bugzilla #1078 */
	SfsDirectory	dir;
	bool		result;
	bool		fileFound[4];

	result = dir.setPath(wxSfsDir);
	fail_unless(result, "Path has not changed");

	result = dir.setDirTraversal(true);
	fail_unless(result, "Failed to enabled dir-traversal");

	result = dir.setFilter(wxT("s"));
	fail_unless(result, "Failed to update the filter");

	dir.scanLocalFilesystem();

	fail_unless(dir.getNumEntries() == 4,
	    "Unexpected number of entries\n"
	    "Is: %i\n"
	    "Expected: 4", dir.getNumEntries());

	for (unsigned int i = 0; i < sizeof(fileFound); i++)
		fileFound[i] = false;

	for (unsigned int i = 0; i < dir.getNumEntries(); i++) {
		SfsEntry *entry = dir.getEntry(i);

		if (entry->getRelativePath(wxSfsDir) == wxT("sub/file1"))
			fileFound[0] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("sub/file2"))
			fileFound[1] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("sub/file3"))
			fileFound[2] = true;
		else if (entry->getRelativePath(wxSfsDir) == wxT("sub/file4"))
			fileFound[3] = true;
		else
			fail("Unexpected entry fetched: %ls",
			    entry->getPath().c_str());
	}

	for (unsigned int i = 0; i < sizeof(fileFound); i++)
		fail_unless(fileFound[i], "File at index %i not found", i);
}
END_TEST

START_TEST(SfsDir_check_handler)
{
	SfsDirectory		dir;
	tc_SfsDir_ScanHandler	handler(&dir, false);
	bool			result;

	dir.setScanHandler(&handler);
	result = dir.setPath(wxSfsDir);
	fail_unless(result, "Path has not changed");

	dir.scanLocalFilesystem();

	fail_unless(dir.getNumEntries() == 5,
	    "Unexpected number of sfs-entries (%i)",
	    dir.getNumEntries());
	fail_unless(handler.startInvocations_ == 1,
	    "Unexpected number of scanStarts()-invocations (%i)",
	    handler.startInvocations_);
	fail_unless(handler.finishedInvocations_ == 1,
	    "Unexpected number of scanFinished()-invocations (%i)",
	    handler.finishedInvocations_);
	fail_unless(handler.scanAborted_ == false,
	    "The filesystem-scan was aborted");
	fail_unless(handler.maxTotal_ == 1,
	    "Unexpected number of total directories (%li)",
	    handler.maxTotal_);
	fail_unless(handler.maxVisited_ == 1,
	    "Unexpected number of visited directories (%li)",
	    handler.maxVisited_);
}
END_TEST

START_TEST(SfsDir_check_handler_abort)
{
	SfsDirectory		dir;
	tc_SfsDir_ScanHandler	handler(&dir, true);
	bool			result;

	dir.setScanHandler(&handler);
	result = dir.setPath(wxSfsDir);
	fail_unless(result, "Path has not changed");

	dir.scanLocalFilesystem();

	fail_unless(dir.getNumEntries() == 0,
	    "Unexpected number of sfs-entries (%i)",
	    dir.getNumEntries());
	fail_unless(handler.startInvocations_ == 1,
	    "Unexpected number of scanStarts()-invocations (%i)",
	    handler.startInvocations_);
	fail_unless(handler.finishedInvocations_ == 1,
	    "Unexpected number of scanFinished()-invocations (%i)",
	    handler.finishedInvocations_);
	fail_unless(handler.scanAborted_ == true,
	    "The filesystem-scan was aborted");
	fail_unless(handler.maxTotal_ == 0,
	    "Unexpected number of total directories (%li)",
	    handler.maxTotal_);
	fail_unless(handler.maxVisited_ == 0,
	    "Unexpected number of visited directories (%li)",
	    handler.maxVisited_);
}
END_TEST

START_TEST(SfsDir_insert_entry)
{
	SfsDirectory	dir;
	SfsEntry	*entry;

	entry = dir.insertEntry(wxT("foo"));
	fail_unless(entry != 0, "Failed to insert path");
	fail_unless(entry->getPath() == wxT("foo"),
	    "Unexptected path (%ls)", entry->getPath().c_str());

	entry = dir.insertEntry(wxT("bar"));
	fail_unless(entry != 0, "Failed to insert path");
	fail_unless(entry->getPath() == wxT("bar"),
	    "Unexptected path (%ls)", entry->getPath().c_str());

	fail_unless(dir.getNumEntries() == 2,
	    "Unexpected number of entries (%i)", dir.getNumEntries());

	entry = dir.getEntry(0);
	fail_unless(entry != 0, "Failed to fetch entry");
	fail_unless(entry->getPath() == wxT("bar"),
	    "Unexptected path (%ls)", entry->getPath().c_str());

	entry = dir.getEntry(1);
	fail_unless(entry != 0, "Failed to fetch entry");
	fail_unless(entry->getPath() == wxT("foo"),
	    "Unexptected path (%ls)", entry->getPath().c_str());
}
END_TEST

START_TEST(SfsDir_insert_double_entry)
{
	SfsDirectory	dir;
	SfsEntry	*entry, *foo_entry;

	foo_entry = dir.insertEntry(wxT("foo"));
	fail_unless(foo_entry != 0, "Failed to insert path");
	fail_unless(foo_entry->getPath() == wxT("foo"),
	    "Unexptected path (%ls)", foo_entry->getPath().c_str());

	entry = dir.insertEntry(wxT("bar"));
	fail_unless(entry != 0, "Failed to insert path");
	fail_unless(entry->getPath() == wxT("bar"),
	    "Unexptected path (%ls)", entry->getPath().c_str());

	entry = dir.insertEntry(wxT("foo"));
	fail_unless(entry != 0, "Failed to insert path");
	fail_unless(entry->getPath() == wxT("foo"),
	    "Unexptected path (%ls)", entry->getPath().c_str());

	fail_unless(entry == foo_entry, "Wrong entry fetched");

	fail_unless(dir.getNumEntries() == 2,
	    "Unexpected number of entries (%i)", dir.getNumEntries());

	entry = dir.getEntry(0);
	fail_unless(entry != 0, "Failed to fetch entry");
	fail_unless(entry->getPath() == wxT("bar"),
	    "Unexptected path (%ls)", entry->getPath().c_str());

	entry = dir.getEntry(1);
	fail_unless(entry != 0, "Failed to fetch entry");
	fail_unless(entry->getPath() == wxT("foo"),
	    "Unexptected path (%ls)", entry->getPath().c_str());
}
END_TEST

START_TEST(SfsDir_remove_entry)
{
	SfsDirectory	dir;
	SfsEntry	*entry;

	entry = dir.insertEntry(wxT("foo"));
	fail_unless(entry != 0, "Failed to insert path");
	fail_unless(entry->getPath() == wxT("foo"),
	    "Unexptected path (%ls)", entry->getPath().c_str());

	entry = dir.insertEntry(wxT("bar"));
	fail_unless(entry != 0, "Failed to insert path");
	fail_unless(entry->getPath() == wxT("bar"),
	    "Unexptected path (%ls)", entry->getPath().c_str());

	fail_unless(dir.getNumEntries() == 2,
	    "Unexpected number of entries (%i)", dir.getNumEntries());

	dir.removeEntry(0);

	fail_unless(dir.getNumEntries() == 1,
	    "Unexpected number of entries (%i)", dir.getNumEntries());

	entry = dir.getEntry(0);
	fail_unless(entry != 0, "Failed to fetch entry");
	fail_unless(entry->getPath() == wxT("foo"),
	    "Unexptected path (%ls)", entry->getPath().c_str());

	
}
END_TEST

START_TEST(SfsDir_removeall_entries)
{
	SfsDirectory	dir;
	SfsEntry	*entry;

	entry = dir.insertEntry(wxT("foo"));
	fail_unless(entry != 0, "Failed to insert path");
	fail_unless(entry->getPath() == wxT("foo"),
	    "Unexptected path (%ls)", entry->getPath().c_str());

	entry = dir.insertEntry(wxT("bar"));
	fail_unless(entry != 0, "Failed to insert path");
	fail_unless(entry->getPath() == wxT("bar"),
	    "Unexptected path (%ls)", entry->getPath().c_str());

	fail_unless(dir.getNumEntries() == 2,
	    "Unexpected number of entries (%i)", dir.getNumEntries());

	dir.removeAllEntries();

	fail_unless(dir.getNumEntries() == 0,
	    "Unexpected number of entries (%i)", dir.getNumEntries());
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
	tcase_add_test(testCase, SfsDir_resolve_link_ok);
	tcase_add_test(testCase, SfsDir_resolve_link_plain_file);
	tcase_add_test(testCase, SfsDir_filter_rel_path_only);
	tcase_add_test(testCase, SfsDir_check_handler);
	tcase_add_test(testCase, SfsDir_check_handler_abort);
	tcase_add_test(testCase, SfsDir_insert_entry);
	tcase_add_test(testCase, SfsDir_insert_double_entry);
	tcase_add_test(testCase, SfsDir_remove_entry);
	tcase_add_test(testCase, SfsDir_removeall_entries);

	return (testCase);
}

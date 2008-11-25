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

#include <ctype.h>
#include <check.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <apncvs.h>

#define TEST_TIMEOUT	20

char cvsroot[32];
char workdir[32];

struct apncvs cvs;

static int
cvs_tc_test_revision(int revno)
{
	char file[128];
	char content[64], exp_content[64];
	FILE *f;
	size_t nelems, exp_nelems;

	snprintf(file, sizeof(file), "%s/%s/file", cvs.workdir, cvs.module);
	if ((f = fopen(file, "r")) == NULL)
		return (-1);

	nelems = fread(content, 1, sizeof(content), f);
	content[nelems] = '\0';

	fclose(f);

	switch (revno) {
	case 0:
		exp_nelems = 8;
		strcpy(exp_content, "Zeile 1\n");
		break;
	case 1:
		exp_nelems = 16;
		strcpy(exp_content, "Zeile 1\nZeile 2\n");
		break;
	case 2:
		exp_nelems = 24;
		strcpy(exp_content, "Zeile 1\nZeile 2\nZeile 3\n");
		break;
	case 3:
		exp_nelems = 32;
		strcpy(exp_content, "Zeile 1\nZeile 2\nZeile 3\nZeile 4\n");
		break;
	default:
		return (-4);
	}

	if (nelems != exp_nelems)
		return (-2);

	if (strcmp(content, exp_content) != 0)
		return (-3);


	return (1);
}

static int
cvs_tc_exec(const char* cmd, ...)
{
	va_list ap;
	char str[128], syscmd[128];
	int rc;

	va_start(ap, cmd);
	vsnprintf(str, sizeof(str), cmd, ap);
	va_end(ap);

	snprintf(syscmd, sizeof(syscmd), "(%s) >/dev/null 2>&1", str);

	rc = system(syscmd);
	return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

void cvs_setup(void)
{
	char *s;

	strcpy(cvsroot, "/tmp/tc_cvs_XXXXXX");
	strcpy(workdir, "/tmp/tc_cvs_XXXXXX");
	s = mkdtemp(cvsroot);
	s = mkdtemp(workdir);

	cvs_tc_exec("cvs -d \"%s\" init", cvsroot);
	cvs_tc_exec("mkdir \"%s/foo\"", cvsroot);
	cvs_tc_exec("chmod 700 \"%s/foo\"", cvsroot);
	cvs_tc_exec("cd \"%s\" && cvs -d \"%s\" checkout foo",
	    workdir, cvsroot);
	cvs_tc_exec("echo \"Zeile 1\" > \"%s/foo/file\"", workdir);
	cvs_tc_exec("cd \"%s\" && cvs -d \"%s\" add foo/file",
	    workdir, cvsroot);
	cvs_tc_exec(
	"cd \"%s\" && cvs -d \"%s\" commit -m \"1st revision\" foo/file",
	workdir, cvsroot);
	cvs_tc_exec("echo \"Zeile 2\" >> \"%s/foo/file\"", workdir);
	cvs_tc_exec(
	"cd \"%s\" && cvs -d \"%s\" commit -m \"2nd revision\" foo/file",
	workdir, cvsroot);
	cvs_tc_exec("echo \"Zeile 3\" >> \"%s/foo/file\"", workdir);
	cvs_tc_exec(
	"cd \"%s\" && cvs -d \"%s\" commit -m \"3rd revision\" foo/file",
	workdir, cvsroot);

	cvs.cvsroot = cvsroot;
	cvs.workdir = workdir;
	cvs.module = "foo";
}

void cvs_teardown(void)
{
	cvs_tc_exec("rm -rf \"%s\"", cvsroot);
	cvs_tc_exec("rm -rf \"%s\"", workdir);
}

START_TEST(cvs_tc_checkout)
{
	int result;

	result = apncvs_checkout(&cvs);
	fail_if(result != 0, "checkout operation failed with %i", result);

	/* Make a fresh checkout into empty working directory */
	cvs_tc_exec("rm -rf \"%s\"", cvs.workdir);
	cvs_tc_exec("mkdir -p \"%s\"", cvs.workdir);

	result = apncvs_checkout(&cvs);
	fail_if(result != 0, "checkout operation failed with %i", result);
}
END_TEST

START_TEST(cvs_tc_checkout_wrong_repository)
{
	int result;

	cvs.cvsroot = "/xyz";
	result = apncvs_checkout(&cvs);
	fail_if(result == 0, "checkout operation failed with %i", result);
}
END_TEST

START_TEST(cvs_tc_checkout_wrong_workdir)
{
	int result;

	cvs.workdir = "xyz";
	result = apncvs_checkout(&cvs);
	fail_if(result == 0, "checkout operation failed with %i", result);
}
END_TEST

START_TEST(cvs_tc_checkout_wrong_module)
{
	int result;

	cvs.module = "xyz";
	result = apncvs_checkout(&cvs);
	fail_if(result == 0, "checkout operation failed with %i", result);
}
END_TEST

START_TEST(cvs_tc_update_head)
{
	int result;

	result = apncvs_update(&cvs, "file", NULL);
	fail_if(result != 0, "update operation failed with %i", result);
	result = cvs_tc_test_revision(2);
	fail_if(result < 0, "Unexpected file content, result=%i", result);
}
END_TEST

START_TEST(cvs_tc_update_rev)
{
	int result;

	result = apncvs_update(&cvs, "file", "1.1");
	fail_if(result != 0, "update operation failed with %i", result);
	result = cvs_tc_test_revision(0);
	fail_if(result < 0, "Unexpected file content, result=%i", result);

	result = apncvs_update(&cvs, "file", "1.2");
	fail_if(result != 0, "update operation failed with %i", result);
	result = cvs_tc_test_revision(1);
	fail_if(result < 0, "Unexpected file content, result=%i", result);

	result = apncvs_update(&cvs, "file", "1.3");
	fail_if(result != 0, "update operation failed with %i", result);
	result = cvs_tc_test_revision(2);
	fail_if(result < 0, "Unexpected file content, result=%i", result);
}
END_TEST

START_TEST(cvs_tc_update_wrong_repository)
{
	int result;

	cvs.cvsroot = "/something_other";

	result = apncvs_update(&cvs, "file", "1.1");
	fail_if(result == 0, "Unexpected result from update command: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_update_wrong_workdir)
{
	int result;

	cvs.workdir = "/something_other";

	result = apncvs_update(&cvs, "file", "1.1");
	fail_if(result == 0, "Unexpected result from update command: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_update_wrong_module)
{
	int result;

	cvs.module = "something_other";

	result = apncvs_update(&cvs, "file", "1.1");
	fail_if(result == 0, "Unexpected result from update command: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_update_wrong_file)
{
	int result;

	result = apncvs_update(&cvs, "xyz", "1.1");
	fail_if(result == 0, "Unexpected result from update command: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_update_wrong_rev)
{
	int result;

	result = apncvs_update(&cvs, "file", "1.10");
	fail_if(result == 0, "Unexpected result from update command: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_log)
{
	int result;
	char str[128];
	struct apncvs_log log;
	struct apncvs_rev *rev;

	result = apncvs_log(&cvs, "file", &log);
	fail_if(result != 0, "cvs log command failed with %i", result);

	snprintf(str, sizeof(str), "%s/%s/file,v", cvs.cvsroot, cvs.module);
	result = strcmp(log.rcs_file, str);
	fail_if(result != 0, "Unexpected rcs_file: %s", log.rcs_file);

	snprintf(str, sizeof(str), "%s/file", cvs.module);
	result = strcmp(log.working_file, str);
	fail_if(result != 0, "Unexpected working_file: %s", log.working_file);

	result = strcmp(log.head, "1.3");
	fail_if(result != 0, "Unexpected head: %s", log.head);

	result = strcmp(log.branch, "");
	fail_if(result != 0, "Unexpected branch: %s", log.branch);

	result = strcmp(log.locks, "strict");
	fail_if(result != 0, "Unexpected locks: %s", log.locks);

	result = strcmp(log.access_list, "");
	fail_if(result != 0, "Unexpected access_list: %s", log.access_list);

	result = strcmp(log.symbolic_names, "");
	fail_if(result != 0, "Unexpected symbolic_names: %s",
	    log.symbolic_names);

	result = strcmp(log.keyword_substitution, "kv");
	fail_if(result != 0, "Unexpected keyword_substitution: %s",
	    log.keyword_substitution);

	fail_if(log.total_revisions != 3, "Unexpected total_revisions: %i",
	    log.total_revisions);
	fail_if(log.selected_revisions != 3,
	    "Unexpected selected_revisions: %i", log.total_revisions);

	result = strcmp(log.description, "");
	fail_if(result != 0, "Unexpected description: %s", log.description);

	/* Rev 1.3 */
	/*rev = log.revs[0];*/
	rev = TAILQ_FIRST(&log.rev_queue);

	result = strcmp(rev->revno, "1.3");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	fail_if(rev->date <= 0, "Unexpected time: %li", rev->date);
	fail_if(strlen(rev->author) == 0, "Unexpected author: %s", rev->author);

	result = strcmp(rev->state, "Exp");
	fail_if(result != 0, "Unexpected state: %s", rev->state);

	result = strcmp(rev->comment, "3rd revision");
	fail_if(result != 0, "Unexpected comment: %s", rev->comment);

	/* Rev 1.2 */
	/*rev = log.revs[1];*/
	rev = TAILQ_NEXT(rev, entry);

	result = strcmp(rev->revno, "1.2");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	fail_if(rev->date <= 0, "Unexpected time: %li", rev->date);
	fail_if(strlen(rev->author) == 0, "Unexpected author: %s", rev->author);

	result = strcmp(rev->state, "Exp");
	fail_if(result != 0, "Unexpected state: %s", rev->state);

	result = strcmp(rev->comment, "2nd revision");
	fail_if(result != 0, "Unexpected comment: %s", rev->comment);

	/* Rev 1.1 */
	/*rev = log.revs[2];*/
	rev = TAILQ_NEXT(rev, entry);

	result = strcmp(rev->revno, "1.1");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	fail_if(rev->date <= 0, "Unexpected time: %li", rev->date);
	fail_if(strlen(rev->author) == 0, "Unexpected author: %s", rev->author);

	result = strcmp(rev->state, "Exp");
	fail_if(result != 0, "Unexpected state: %s", rev->state);

	result = strcmp(rev->comment, "1st revision");
	fail_if(result != 0, "Unexpected comment: %s", rev->comment);

	/* End of list */
	rev = TAILQ_NEXT(rev, entry);
	fail_if(rev != NULL, "End of revision-list not reached!");

	apncvs_log_destroy(&log);
}
END_TEST

START_TEST(cvs_tc_log_wrong_repository)
{
	int result;
	struct apncvs_log log;

	cvs.cvsroot = "/something_else_which_does_not_exist";
	result = apncvs_log(&cvs, "file", &log);
	fail_if(result == 0, "Unexpected result from log command: %i", result);
}
END_TEST

START_TEST(cvs_tc_log_wrong_workdir)
{
	int result;
	struct apncvs_log log;

	cvs.workdir = "/something_else_which_does_not_exist";
	result = apncvs_log(&cvs, "file", &log);
	fail_if(result == 0, "Unexpected result from log command: %i", result);
}
END_TEST

START_TEST(cvs_tc_log_wrong_module)
{
	int result;
	struct apncvs_log log;

	cvs.module = "something_else_which_does_not_exist";
	result = apncvs_log(&cvs, "file", &log);
	fail_if(result == 0, "Unexpected result from log command: %i", result);
}
END_TEST

START_TEST(cvs_tc_log_wrong_file)
{
	int result;
	struct apncvs_log log;

	result = apncvs_log(&cvs, "something_else", &log);
	fail_if(result == 0, "Unexpected result from log command: %i", result);
}
END_TEST

START_TEST(cvs_tc_commit)
{
	int result;
	struct apncvs_log log;
	struct apncvs_rev *rev;

	cvs_tc_exec("echo \"Zeile 4\" >> \"%s/%s/file\"",
	    cvs.workdir, cvs.module);

	result = apncvs_commit(&cvs, "file", "4th revision");
	fail_if(result != 0, "Unexpected result from commit command: %i",
	    result);

	result = apncvs_log(&cvs, "file", &log);
	fail_if(result != 0, "Unexpected result from log command: %i", result);

	fail_if(log.total_revisions != 4, "Unexpected total_revisions: %i",
	    log.total_revisions);
	fail_if(log.selected_revisions != 4,
	    "Unexpected selected_revisions: %i", log.selected_revisions);

	/* Check new revision */

	rev = TAILQ_FIRST(&log.rev_queue);
	result = strcmp(rev->revno, "1.4");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	fail_if(rev->date <= 0, "Unexpected time: %li", rev->date);
	fail_if(strlen(rev->author) == 0, "Unexpected author: %s", rev->author);

	result = strcmp(rev->state, "Exp");
	fail_if(result != 0, "Unexpected state: %s", rev->state);

	result = strcmp(rev->comment, "4th revision");
	fail_if(result != 0, "Unexpected comment: %s", rev->comment);

	apncvs_log_destroy(&log);

	/* Check content of revision */
	result = apncvs_update(&cvs, "file", "1.4");
	fail_if(result != 0, "Unexpected result from update command: %i",
	    result);

	result = cvs_tc_test_revision(3);
	fail_if(result < 0, "Unexpected file content, result=%i", result);
}
END_TEST

START_TEST(cvs_tc_commit_no_comment)
{
	int result;
	struct apncvs_log log;
	struct apncvs_rev *rev;

	cvs_tc_exec("echo \"Zeile 4\" >> \"%s/%s/file\"",
	    cvs.workdir, cvs.module);

	result = apncvs_commit(&cvs, "file", "");
	fail_if(result != 0, "Unexpected result from commit command: %i",
	    result);

	result = apncvs_log(&cvs, "file", &log);
	fail_if(result != 0, "Unexpected result from log command: %i", result);

	fail_if(log.total_revisions != 4, "Unexpected total_revisions: %i",
	    log.total_revisions);
	fail_if(log.selected_revisions != 4,
	    "Unexpected selected_revisions: %i", log.selected_revisions);

	/* Check new revision */

	rev = TAILQ_FIRST(&log.rev_queue);
	result = strcmp(rev->revno, "1.4");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	fail_if(rev->date <= 0, "Unexpected time: %li", rev->date);
	fail_if(strlen(rev->author) == 0, "Unexpected author: %s",
	    rev->author);

	result = strcmp(rev->state, "Exp");
	fail_if(result != 0, "Unexpected state: %s", rev->state);

	result = strcmp(rev->comment, "*** empty log message ***");
	fail_if(result != 0, "Unexpected comment: %s", rev->comment);

	apncvs_log_destroy(&log);

	/* Check content of revision */
	result = apncvs_update(&cvs, "file", "1.4");
	fail_if(result != 0, "Unexpected result from update command: %i",
	    result);

	result = cvs_tc_test_revision(3);
	fail_if(result < 0, "Unexpected file content, result=%i", result);
}
END_TEST

START_TEST(cvs_tc_commit_wrong_repository)
{
	int result;

	cvs.cvsroot = "/somewhere_else";

	result = apncvs_commit(&cvs, "file", "A silly comment");
	fail_if(result == 0, "Unexpected result from commit command: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_commit_wrong_workdir)
{
	int result;

	cvs.workdir = "/somewhere_else";

	result = apncvs_commit(&cvs, "file", "A silly comment");
	fail_if(result == 0, "Unexpected result from commit command: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_commit_wrong_module)
{
	int result;

	cvs.module = "xyz";

	result = apncvs_commit(&cvs, "file", "A silly comment");
	fail_if(result == 0, "Unexpected result from commit command: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_commit_wrong_file)
{
	int result;

	result = apncvs_commit(&cvs, "xyz", "A silly comment");
	fail_if(result == 0, "Unexpected result from commit command: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_commit_long_comment)
{
	int result;

	result = apncvs_commit(&cvs, "file", "Here you need a long comment.\n\
Very long...\n\
Even longer...\n\
It is not enough, need much more content!\n\
What should be the content of a comment?\n\
It depends on you!\n\
The version management does not take care about the comment.\n\
Try to describe the new revision!");

	fail_if(result != 0, "Unexpected result from commit command: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_add)
{
	char str[128];
	int result;
	struct apncvs_log log;
	struct apncvs_rev *rev;

	cvs_tc_exec("echo \"Ein bisschen Frieden...\" > %s/%s/anotherfile",
	    cvs.workdir, cvs.module);

	result = apncvs_add(&cvs, "anotherfile");
	fail_if(result != 0, "Unexpected result from add command: %i", result);

	result = apncvs_commit(&cvs, "anotherfile", "...");
	fail_if(result != 0, "Unexpected result from commit command: %i",
	    result);

	result = apncvs_log(&cvs, "anotherfile", &log);
	fail_if(result != 0, "Unexpected result from log command: %i", result);

	/* Check rev/revision */
	snprintf(str, sizeof(str), "%s/%s/anotherfile,v",
	    cvs.cvsroot, cvs.module);
	result = strcmp(log.rcs_file, str);
	fail_if(result != 0, "Unexpected rcs_file: %s", log.rcs_file);

	snprintf(str, sizeof(str), "%s/anotherfile", cvs.module);
	result = strcmp(log.working_file, str);
	fail_if(result != 0, "Unexpected working_file: %s", log.working_file);

	result = strcmp(log.head, "1.1");
	fail_if(result != 0, "Unexpected head: %s", log.head);

	result = strcmp(log.branch, "");
	fail_if(result != 0, "Unexpected branch: %s", log.branch);

	result = strcmp(log.locks, "strict");
	fail_if(result != 0, "Unexpected locks: %s", log.locks);

	result = strcmp(log.access_list, "");
	fail_if(result != 0, "Unexpected access_list: %s", log.access_list);

	result = strcmp(log.symbolic_names, "");
	fail_if(result != 0, "Unexpected symbolic_names: %s",
	    log.symbolic_names);

	result = strcmp(log.keyword_substitution, "kv");
	fail_if(result != 0, "Unexpected keyword_substitution: %s",
	    log.keyword_substitution);

	fail_if(log.total_revisions != 1, "Unexpected total_revisions: %i",
	    log.total_revisions);
	fail_if(log.selected_revisions != 1,
	    "Unexpected selected_revisions: %i", log.total_revisions);

	result = strcmp(log.description, "");
	fail_if(result != 0, "Unexpected description: %s", log.description);

	/* Rev 1.1 */
	rev = TAILQ_FIRST(&log.rev_queue);

	result = strcmp(rev->revno, "1.1");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	fail_if(rev->date <= 0, "Unexpected time: %li", rev->date);
	fail_if(strlen(rev->author) == 0, "Unexpected author: %s", rev->author);

	result = strcmp(rev->state, "Exp");
	fail_if(result != 0, "Unexpected state: %s", rev->state);

	result = strcmp(rev->comment, "...");
	fail_if(result != 0, "Unexpected comment: %s", rev->comment);

	apncvs_log_destroy(&log);
}
END_TEST

START_TEST(cvs_tc_add_no_file)
{
	int result;

	result = apncvs_add(&cvs, "anotherfile");
	fail_if(result == 0, "Unexpected result from add command: %i", result);
}
END_TEST

START_TEST(cvs_tc_add_added_file)
{
	int result;

	result = apncvs_add(&cvs, "file");
	fail_if(result == 0, "Unexpected result from add command: %i", result);
}
END_TEST

START_TEST(cvs_tc_add_wrong_repository)
{
	int result;

	cvs_tc_exec("echo \"Ein bisschen Frieden...\" > %s/%s/anotherfile",
	    cvs.workdir, cvs.module);

	cvs.cvsroot = "/somewhere_else";

	result = apncvs_add(&cvs, "anotherfile");
	fail_if(result == 0, "Unexpected result from add command: %i", result);
}
END_TEST

START_TEST(cvs_tc_add_wrong_workdir)
{
	int result;

	cvs_tc_exec("echo \"Ein bisschen Frieden...\" > %s/%s/anotherfile",
	    cvs.workdir, cvs.module);

	cvs.workdir = "/somewhere_else";

	result = apncvs_add(&cvs, "anotherfile");
	fail_if(result == 0, "Unexpected result from add command: %i", result);
}
END_TEST

START_TEST(cvs_tc_add_wrong_module)
{
	int result;

	cvs_tc_exec("echo \"Ein bisschen Frieden...\" > %s/%s/anotherfile",
	    cvs.workdir, cvs.module);

	cvs.module = "xyz";

	result = apncvs_add(&cvs, "anotherfile");
	fail_if(result == 0, "Unexpected result from add command: %i", result);
}
END_TEST

START_TEST(cvs_tc_remrev_1_1)
{
	int			result;
	struct apncvs_log	log;
	struct apncvs_rev	*rev;

	result = apncvs_remrev(&cvs, "file", "1.1");
	fail_if(result != 0, "Failed to remove revision: %i", result);

	result = apncvs_log(&cvs, "file", &log);
	fail_if(result != 0, "Unexpected result from log command: %i", result);

	fail_if(log.total_revisions != 2, "Unexpected total_revisions;: %i",
	    log.total_revisions);
	fail_if(log.selected_revisions != 2,
	    "Unexpected selected_revisions;: %i", log.selected_revisions);

	/* Revision list */
	rev = TAILQ_FIRST(&log.rev_queue);
	result = strcmp(rev->revno, "1.3");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	rev = TAILQ_NEXT(rev, entry);
	result = strcmp(rev->revno, "1.2");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	/* Content of revisions */

	result = apncvs_update(&cvs, "file", "1.3");
	fail_if(result != 0, "Unexpected result from update command: %i",
	    result);

	result = cvs_tc_test_revision(2);
	fail_if(result < 0, "Unexpected file content, result=%i", result);

	result = apncvs_update(&cvs, "file", "1.2");
	fail_if(result != 0, "Unexpected result from update command: %i",
	    result);

	result = cvs_tc_test_revision(1);
	fail_if(result < 0, "Unexpected file content, result=%i", result);

	apncvs_log_destroy(&log);
}
END_TEST

START_TEST(cvs_tc_remrev_1_2)
{
	int			result;
	struct apncvs_log	log;
	struct apncvs_rev	*rev;

	result = apncvs_remrev(&cvs, "file", "1.2");
	fail_if(result != 0, "Failed to remove revision: %i", result);

	result = apncvs_log(&cvs, "file", &log);
	fail_if(result != 0, "Unexpected result from log command: %i", result);

	fail_if(log.total_revisions != 2, "Unexpected total_revisions;: %i",
	    log.total_revisions);
	fail_if(log.selected_revisions != 2,
	    "Unexpected selected_revisions;: %i", log.selected_revisions);

	/* Revision list */
	rev = TAILQ_FIRST(&log.rev_queue);
	result = strcmp(rev->revno, "1.3");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	rev = TAILQ_NEXT(rev, entry);
	result = strcmp(rev->revno, "1.1");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	/* Content of revisions */

	result = apncvs_update(&cvs, "file", "1.3");
	fail_if(result != 0, "Unexpected result from update command: %i",
	    result);

	result = cvs_tc_test_revision(2);
	fail_if(result < 0, "Unexpected file content, result=%i", result);

	result = apncvs_update(&cvs, "file", "1.1");
	fail_if(result != 0, "Unexpected result from update command: %i",
	    result);

	result = cvs_tc_test_revision(0);
	fail_if(result < 0, "Unexpected file content, result=%i", result);

	apncvs_log_destroy(&log);
}
END_TEST

START_TEST(cvs_tc_remrev_1_3)
{
	int			result;
	struct apncvs_log	log;
	struct apncvs_rev	*rev;

	result = apncvs_remrev(&cvs, "file", "1.3");
	fail_if(result != 0, "Failed to remove revision: %i", result);

	result = apncvs_log(&cvs, "file", &log);
	fail_if(result != 0, "Unexpected result from log command: %i", result);

	fail_if(log.total_revisions != 2, "Unexpected total_revisions;: %i",
	    log.total_revisions);
	fail_if(log.selected_revisions != 2,
	    "Unexpected selected_revisions;: %i", log.selected_revisions);

	/* Revision list */
	rev = TAILQ_FIRST(&log.rev_queue);
	fail_if(rev == NULL, "End of revision list reached");
	result = strcmp(rev->revno, "1.2");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	rev = TAILQ_NEXT(rev, entry);
	fail_if(rev == NULL, "End of revision list reached");
	result = strcmp(rev->revno, "1.1");
	fail_if(result != 0, "Unexpected revno: %s", rev->revno);

	/* Content of revisions */

	result = apncvs_update(&cvs, "file", "1.2");
	fail_if(result != 0, "Unexpected result from update command: %i",
	    result);

	result = cvs_tc_test_revision(1);
	fail_if(result < 0, "Unexpected file content, result=%i", result);

	result = apncvs_update(&cvs, "file", "1.1");
	fail_if(result != 0, "Unexpected result from update command: %i",
	    result);

	result = cvs_tc_test_revision(0);
	fail_if(result < 0, "Unexpected file content, result=%i", result);

	apncvs_log_destroy(&log);
}
END_TEST

START_TEST(cvs_tc_remrev_all)
{
	int result;

	result = apncvs_remrev(&cvs, "file", "1.3");
	fail_if(result != 0, "Failed to remove revision: %i", result);

	result = apncvs_remrev(&cvs, "file", "1.2");
	fail_if(result != 0, "Failed to remove revision: %i", result);

	result = apncvs_remrev(&cvs, "file", "1.1");
	fail_if(result == 0, "Unexpected result from remove operation: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_remrev_wrong_rev)
{
	int result;

	result = apncvs_remrev(&cvs, "file", "1.10");
	fail_if(result == 0, "Unexpected result from remove operation: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_remrev_wrong_file)
{
	int result;

	result = apncvs_remrev(&cvs, "xyz", "1.1");
	fail_if(result == 0, "Unexpected result from remove operation: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_remrev_wrong_repository)
{
	int result;

	cvs.cvsroot = "/somewhere_else";

	result = apncvs_remrev(&cvs, "file", "1.1");
	fail_if(result == 0, "Unexpected result from remove operation: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_remrev_wrong_workdir)
{
	int result;

	cvs.workdir = "/somewhere_else";

	result = apncvs_remrev(&cvs, "file", "1.1");
	fail_if(result == 0, "Unexpected result from remove operation: %i",
	    result);
}
END_TEST

START_TEST(cvs_tc_remrev_wrong_module)
{
	int result;

	cvs.module = "xyz";

	result = apncvs_remrev(&cvs, "file", "1.1");
	fail_if(result == 0, "Unexpected result from remove operation: %i",
	    result);
}
END_TEST

TCase *
apnvm_tc_cvs(void)
{
	TCase *tc = tcase_create("CVS");

	tcase_add_checked_fixture(tc, cvs_setup, cvs_teardown);
	tcase_set_timeout(tc, TEST_TIMEOUT);

	tcase_add_test(tc, cvs_tc_checkout);
	tcase_add_test(tc, cvs_tc_checkout_wrong_repository);
	tcase_add_test(tc, cvs_tc_checkout_wrong_workdir);
	tcase_add_test(tc, cvs_tc_checkout_wrong_module);
	tcase_add_test(tc, cvs_tc_update_head);
	tcase_add_test(tc, cvs_tc_update_rev);
	tcase_add_test(tc, cvs_tc_update_wrong_repository);
	tcase_add_test(tc, cvs_tc_update_wrong_workdir);
	tcase_add_test(tc, cvs_tc_update_wrong_module);
	tcase_add_test(tc, cvs_tc_update_wrong_file);
	tcase_add_test(tc, cvs_tc_update_wrong_rev);
	tcase_add_test(tc, cvs_tc_log);
	tcase_add_test(tc, cvs_tc_log_wrong_repository);
	tcase_add_test(tc, cvs_tc_log_wrong_workdir);
	tcase_add_test(tc, cvs_tc_log_wrong_module);
	tcase_add_test(tc, cvs_tc_log_wrong_file);
	tcase_add_test(tc, cvs_tc_commit);
	tcase_add_test(tc, cvs_tc_commit_no_comment);
	tcase_add_test(tc, cvs_tc_commit_wrong_repository);
	tcase_add_test(tc, cvs_tc_commit_wrong_workdir);
	tcase_add_test(tc, cvs_tc_commit_wrong_module);
	tcase_add_test(tc, cvs_tc_commit_wrong_file);
	tcase_add_test(tc, cvs_tc_commit_long_comment);
	tcase_add_test(tc, cvs_tc_add);
	tcase_add_test(tc, cvs_tc_add_no_file);
	tcase_add_test(tc, cvs_tc_add_added_file);
	tcase_add_test(tc, cvs_tc_add_wrong_repository);
	tcase_add_test(tc, cvs_tc_add_wrong_workdir);
	tcase_add_test(tc, cvs_tc_add_wrong_module);
	tcase_add_test(tc, cvs_tc_remrev_1_1);
	tcase_add_test(tc, cvs_tc_remrev_1_2);
	tcase_add_test(tc, cvs_tc_remrev_1_3);
	tcase_add_test(tc, cvs_tc_remrev_all);
	tcase_add_test(tc, cvs_tc_remrev_wrong_rev);
	tcase_add_test(tc, cvs_tc_remrev_wrong_file);
	tcase_add_test(tc, cvs_tc_remrev_wrong_repository);
	tcase_add_test(tc, cvs_tc_remrev_wrong_workdir);
	tcase_add_test(tc, cvs_tc_remrev_wrong_module);

	return (tc);
}

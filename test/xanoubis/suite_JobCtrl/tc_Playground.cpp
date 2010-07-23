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

#ifdef LINUX

#include <sys/types.h>
#include <sys/wait.h>
#include <check.h>

#include <wx/app.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/module.h>

#include <PlaygroundFilesTask.h>
#include <PlaygroundListTask.h>
#include <JobCtrl.h>

#include "dummyDaemon.h"
#include "JobCtrlEventSpy.h"
#include "TaskEventSpy.h"
#include "utils.h"

#include <anoubis_errno.h>
#include <anoubis_playground.h>

static void
setup(void)
{
	setup_dummyDaemon();

	for (int i = 0; i < 2; i++) {
		const char *playground[] = {
			"/bin/bash", "-c", "echo hallo > xxx"
		};

		int pid = playground_start_fork((char **)playground);
		fail_unless(pid > 0,
		    "Failed to fork playground-process: %d (%s)",
		    -pid, strerror(-pid));
		waitpid(pid, NULL, 0);
	}
}

static void
teardown(void)
{
	teardown_dummyDaemon();
}

START_TEST(fetch_list)
{
	JobCtrl *jobCtrl = JobCtrl::getInstance();
	TaskEventSpy listSpy(jobCtrl, anTASKEVT_PG_LIST);
	TaskEventSpy filesSpy(jobCtrl, anTASKEVT_PG_FILES);
	PlaygroundListTask listTask;
	PlaygroundFilesTask filesTask;

	jobCtrl->addTask(&listTask);
	listSpy.waitForInvocation(1);

	fail_unless(listTask.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to fetch playground-list");

	listTask.resetRecordIterator();
	int numRecords = 0;
	while (listTask.readNextRecord()) {
		fail_unless(listTask.getCommand() == wxT("/bin/bash"),
		    "Wrong command: %hs", listTask.getCommand().c_str());
		fail_unless(listTask.getUID() == (int)getuid(),
		    "Wrong uid: %i", listTask.getUID());
		fail_unless(listTask.getNumFiles() == 1,
		    "Wrong number of files: %i", listTask.getNumFiles());

		numRecords++;

		/* Fetch files for the current playground */
		filesTask.setRequestedPGID(listTask.getPGID());
		jobCtrl->addTask(&filesTask);
		filesSpy.waitForInvocation(numRecords);

		filesTask.resetRecordIterator();
		int numFiles = 0;
		while (filesTask.readNextRecord()) {
			fail_unless(filesTask.getPath().EndsWith(wxT("xxx")));
			numFiles++;
		}

		fail_unless(numFiles == 1,
		    "Wrong number of files: %i", numFiles);
	}

	fail_unless(numRecords == 2,
	    "Unexpected number of playgrounds: %i", numRecords);
}
END_TEST

TCase *
getTc_JobCtrlPlayground(void)
{
	TCase *tcase = tcase_create("JobCtrl_Playground");

	tcase_set_timeout(tcase, 10);
	tcase_add_checked_fixture(tcase, setup, teardown);

	tcase_add_test(tcase, fetch_list);

	return (tcase);
}

#endif /* LINUX */

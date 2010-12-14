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

#include <AnRowProvider.h>
#include <JobCtrl.h>
#include <PlaygroundCtrl.h>
#include <PlaygroundFileEntry.h>
#include <PlaygroundFilesTask.h>
#include <PlaygroundInfoEntry.h>
#include <PlaygroundListTask.h>
#include <PlaygroundUnlinkTask.h>

#include "dummyDaemon.h"
#include "JobCtrlEventSpy.h"
#include "TaskEventSpy.h"
#include "utils.h"

#include <anoubis_errno.h>
#include <anoubis_playground.h>

#define CHECK_REGENT(name) \
	do { \
		struct stat _sb; \
		if (stat(name, &_sb) != 0 || !S_ISREG(_sb.st_mode)) { \
			fail("stat(%s) failed or it's not a file.", name); \
		} \
	} while (0)

#define CHECK_DIRENT(name) \
	do { \
		struct stat _sb; \
		if (stat(name, &_sb) != 0 || !S_ISDIR(_sb.st_mode)) { \
			fail("stat(%s) failed or it's not a directory.", \
			    name); \
		} \
	} while (0)

#define CHECK_NOENT(name) \
	do { \
		struct stat _sb; \
		if (stat(name, &_sb) == 0 || errno != ENOENT) { \
			fail("Unlink failed: file [%s] still exists.", name); \
		} \
	} while (0)

#undef DEBUG_DUMP
#ifdef DEBUG_DUMP
#  define DUMP_PLAYGROUND	dump_playground()
#else
#  define DUMP_PLAYGROUND
#endif /* DEBUG_DUMP */

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

	const char *playground3[] = {
		"/bin/bash", "-c", "echo h1 > h1; mkdir pgTest; cd pgTest;"
			" for i in 1 2 3 4; do echo h$i > h$i; done;"
	};
	int pid = playground_start_fork((char**)playground3);
	fail_unless(pid > 0, "Failed to fork playground-process: %d (%s)",
	    -pid, strerror(-pid));
	waitpid(pid, NULL, 0);

	const char *playground4[] = {
		"/bin/bash", "-c", "echo h > xxx; sleep 80 &"
	};
	pid = playground_start_fork((char**)playground4);

	fail_unless(pid > 0, "Failed to fork playground-process: %d (%s)",
	    -pid, strerror(-pid));
	/* Note: we do not wait for this shell to finish, no cleanup is done */
}

static void
teardown(void)
{
	teardown_dummyDaemon();
}

#ifdef DEBUG_DUMP
static AnRowProvider*
getPlaygroundInfos(void)
{
	JobCtrl *jobctl = JobCtrl::instance();
	PlaygroundCtrl *pgctl = PlaygroundCtrl::instance();

	/* get all playgrounds */
	AnRowProvider *pg_info = (AnRowProvider*)pgctl->getInfoProvider();

	pgctl->updatePlaygroundInfo();

	while (pg_info->getSize() < 1) {
		jobctl->ProcessPendingEvents();
		pg_info->ProcessPendingEvents();
	}

	return pg_info;
}

static void
dump_playground(void)
{
	/* get all playgrounds */
	AnRowProvider *pg_info = getPlaygroundInfos();

	for (int i=0; i<pg_info->getSize(); i++) {
		AnListClass *row = pg_info->getRow(i);
		PlaygroundInfoEntry *entry =
		    dynamic_cast<PlaygroundInfoEntry *>(row);

		fprintf(stderr, "playground: '%ls' pgid=%llx"
		    " uid=%u files=%d active=%d\n",
		    (const wchar_t*)entry->getPath().c_str(), entry->getPgid(),
		    entry->getUid(), entry->getNumFiles(), entry->isActive());
	}
}
#endif /* DEBUG_DUMP */

START_TEST(fetch_list)
{
	JobCtrl *jobCtrl = JobCtrl::instance();
	TaskEventSpy listSpy(jobCtrl, anTASKEVT_PG_LIST);
	TaskEventSpy filesSpy(jobCtrl, anTASKEVT_PG_FILES);
	PlaygroundListTask listTask;
	PlaygroundFilesTask *filesTask;

	jobCtrl->addTask(&listTask);
	listSpy.waitForInvocation(1);

	fail_unless(listTask.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to fetch playground-list");

	listTask.resetRecordIterator();
	int numRecords = 0;
	while (listTask.readNextRecord()) {
		if (listTask.getPGID() > 2) {
			continue;
		}
		fail_unless(listTask.getCommand() == wxT("/bin/bash"),
		    "Wrong command: %hs", listTask.getCommand().c_str());
		fail_unless(listTask.getUID() == (int)getuid(),
		    "Wrong uid: %i", listTask.getUID());
		fail_unless(listTask.getNumFiles() == 1,
		    "Wrong number of files: %i", listTask.getNumFiles());

		numRecords++;

		/* Fetch files for the current playground */
		filesTask = new PlaygroundFilesTask(listTask.getPGID());
		jobCtrl->addTask(filesTask);
		filesSpy.waitForInvocation(numRecords);

		filesTask->resetRecordIterator();
		int numFiles = 0;
		while (filesTask->readNextRecord()) {
			fail_unless(filesTask->getPath().EndsWith(wxT("xxx")));
			numFiles++;
		}
		delete filesTask;

		fail_unless(numFiles == 1,
		    "Wrong number of files: %i", numFiles);
	}

	fail_unless(numRecords == 2,
	    "Unexpected number of playgrounds: %i", numRecords);
}
END_TEST

START_TEST(unlink_activePg)
{
	JobCtrl *jobCtrl = JobCtrl::instance();
	TaskEventSpy eventSpy(jobCtrl, anTASKEVT_PG_UNLINK);
	PlaygroundUnlinkTask unlinkTask(8);

	DUMP_PLAYGROUND;
	jobCtrl->addTask(&unlinkTask);
	eventSpy.waitForInvocation(1);
	mark_point();

	if (unlinkTask.getComTaskResult() != ComTask::RESULT_REMOTE_ERROR ||
	    unlinkTask.getResultDetails() != EBUSY) {
		fail("UnlinkTask returned with %d/%s - but %d/%s expected",
		    unlinkTask.getResultDetails(),
		    anoubis_strerror(unlinkTask.getResultDetails()),
		    EBUSY, strerror(EBUSY));
	}
}
END_TEST

START_TEST(unlink_noentPg)
{
	JobCtrl *jobCtrl = JobCtrl::instance();
	TaskEventSpy eventSpy(jobCtrl, anTASKEVT_PG_UNLINK);
	PlaygroundUnlinkTask unlinkTask(888);

	DUMP_PLAYGROUND;
	jobCtrl->addTask(&unlinkTask);
	eventSpy.waitForInvocation(1);
	mark_point();

	if (unlinkTask.getComTaskResult() != ComTask::RESULT_REMOTE_ERROR ||
	    unlinkTask.getResultDetails() != ESRCH) {
		fail("UnlinkTask returned with %d/%s - but %d/%s expected",
		    unlinkTask.getResultDetails(),
		    anoubis_strerror(unlinkTask.getResultDetails()),
		    ESRCH, strerror(ESRCH));
	}
}
END_TEST

START_TEST(unlink_completeSimple)
{
	JobCtrl *jobCtrl = JobCtrl::instance();
	TaskEventSpy eventSpy(jobCtrl, anTASKEVT_PG_UNLINK);
	PlaygroundUnlinkTask unlinkTask(1);

	DUMP_PLAYGROUND;
	jobCtrl->addTask(&unlinkTask);
	eventSpy.waitForInvocation(1);
	mark_point();

	fail_unless(unlinkTask.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "UnlinkTask failed with %d/%s", unlinkTask.getResultDetails(),
	    anoubis_strerror(unlinkTask.getResultDetails()));

	CHECK_NOENT("/home/u2000/.plgr.1.xxx");
}
END_TEST

START_TEST(unlink_completeComplex)
{
	JobCtrl *jobCtrl = JobCtrl::instance();
	TaskEventSpy eventSpy(jobCtrl, anTASKEVT_PG_UNLINK);
	PlaygroundUnlinkTask unlinkTask(15);

	DUMP_PLAYGROUND;
	jobCtrl->addTask(&unlinkTask);
	eventSpy.waitForInvocation(1);
	mark_point();

	fail_unless(unlinkTask.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "UnlinkTask failed with %d/%s", unlinkTask.getResultDetails(),
	    anoubis_strerror(unlinkTask.getResultDetails()));

	CHECK_NOENT("/home/u2000/.plgr.f.h1");
	CHECK_NOENT("/home/u2000/.plgr.f.pgTest");
	CHECK_NOENT("/home/u2000/.plgr.f.pgTest/.plgr.f.h1");
	CHECK_NOENT("/home/u2000/.plgr.f.pgTest/.plgr.f.h2");
	CHECK_NOENT("/home/u2000/.plgr.f.pgTest/.plgr.f.h3");
	CHECK_NOENT("/home/u2000/.plgr.f.pgTest/.plgr.f.h4");
}
END_TEST

START_TEST(unlink_listEmpty)
{
	std::vector<PlaygroundFileEntry *> list;
	JobCtrl *jobCtrl = JobCtrl::instance();
	TaskEventSpy eventSpy(jobCtrl, anTASKEVT_PG_UNLINK);
	PlaygroundUnlinkTask unlinkTask(0x13);

	DUMP_PLAYGROUND;
	list.clear();
	unlinkTask.addMatchList(list);
	jobCtrl->addTask(&unlinkTask);
	eventSpy.waitForInvocation(1);
	mark_point();

	if (unlinkTask.getComTaskResult() != ComTask::RESULT_SUCCESS) {
		fail("UnlinkTask returned with %d/%s - but success expected",
		    unlinkTask.getResultDetails(),
		    anoubis_strerror(unlinkTask.getResultDetails()));
	}
	CHECK_REGENT("/home/u2000/.plgr.13.h1");
	CHECK_DIRENT("/home/u2000/.plgr.13.pgTest");
	CHECK_REGENT("/home/u2000/.plgr.13.pgTest/.plgr.13.h1");
	CHECK_REGENT("/home/u2000/.plgr.13.pgTest/.plgr.13.h2");
	CHECK_REGENT("/home/u2000/.plgr.13.pgTest/.plgr.13.h3");
	CHECK_REGENT("/home/u2000/.plgr.13.pgTest/.plgr.13.h4");
}
END_TEST

START_TEST(unlink_listComplex)
{
	std::vector<PlaygroundFileEntry *> list;
	PlaygroundFileEntry *entry = NULL;
	AnRowProvider *pg_files = NULL;
	JobCtrl *jobCtrl = JobCtrl::instance();
	PlaygroundCtrl *playgroundCtrl = PlaygroundCtrl::instance();
	TaskEventSpy eventSpy(jobCtrl, anTASKEVT_PG_UNLINK);
	PlaygroundUnlinkTask unlinkTask(0xb);

	DUMP_PLAYGROUND;
	pg_files = (AnRowProvider*)playgroundCtrl->getFileProvider();
	playgroundCtrl->updatePlaygroundFiles(0xb);
	while (pg_files->getSize() < 6) {
		jobCtrl->ProcessPendingEvents();
		pg_files->ProcessPendingEvents();
	}
	mark_point();

	for (int i=0; i<=6; i++) {
		entry = dynamic_cast<PlaygroundFileEntry *>(
		    pg_files->getRow(i));
		if (entry == NULL) {
			continue;
		}
		if (entry->getPaths()[0] != wxT("/home/u2000/h1")) {
			list.push_back(entry);
		}
	}
	mark_point();

	unlinkTask.addMatchList(list);
	jobCtrl->addTask(&unlinkTask);
	eventSpy.waitForInvocation(1);
	mark_point();

	fail_unless(unlinkTask.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "UnlinkTask failed with %d/%s", unlinkTask.getResultDetails(),
	    anoubis_strerror(unlinkTask.getResultDetails()));

	CHECK_REGENT("/home/u2000/.plgr.b.h1");
	CHECK_NOENT("/home/u2000/.plgr.b.pgTest");
	CHECK_NOENT("/home/u2000/.plgr.b.pgTest/.plgr.b.h1");
	CHECK_NOENT("/home/u2000/.plgr.b.pgTest/.plgr.b.h2");
	CHECK_NOENT("/home/u2000/.plgr.b.pgTest/.plgr.b.h3");
	CHECK_NOENT("/home/u2000/.plgr.b.pgTest/.plgr.b.h4");
}
END_TEST

START_TEST(unlink_errorList)
{
	JobCtrl *jobCtrl = JobCtrl::instance();
	TaskEventSpy eventSpy(jobCtrl, anTASKEVT_PG_UNLINK);
	PlaygroundUnlinkTask unlinkTask(3);
	std::map<std::pair<uint64_t, uint64_t>, int> list;

	DUMP_PLAYGROUND;
	chmod("/home/u2000/.plgr.3.pgTest", (S_IRUSR | S_IXUSR));
	jobCtrl->addTask(&unlinkTask);
	eventSpy.waitForInvocation(1);
	mark_point();

	fail_if(unlinkTask.hasErrorList() == 0, "List unexpectedly empty");
	list = unlinkTask.getErrorList();
	fail_if(list.size() != 5, "Unexpected size of error list.");
	fail_if(list.begin()->second != ENOTEMPTY, "Unexpected errno");

	if (unlinkTask.getComTaskResult() != ComTask::RESULT_LOCAL_ERROR ||
	    unlinkTask.getResultDetails() != EBUSY) {
		fail("UnlinkTask returned with %d/%s - but %d/%s expected",
		    unlinkTask.getResultDetails(),
		    anoubis_strerror(unlinkTask.getResultDetails()),
		    EBUSY, strerror(EBUSY));
	}

	chmod("/home/u2000/.plgr.3.pgTest", (S_IRUSR|S_IWUSR|S_IXUSR));
	CHECK_NOENT("/home/u2000/.plgr.3.h1");
	CHECK_DIRENT("/home/u2000/.plgr.3.pgTest");
	CHECK_REGENT("/home/u2000/.plgr.3.pgTest/.plgr.3.h1");
	CHECK_REGENT("/home/u2000/.plgr.3.pgTest/.plgr.3.h2");
	CHECK_REGENT("/home/u2000/.plgr.3.pgTest/.plgr.3.h3");
	CHECK_REGENT("/home/u2000/.plgr.3.pgTest/.plgr.3.h4");
}
END_TEST

START_TEST(unlink_force)
{
	JobCtrl *jobCtrl = JobCtrl::instance();
	TaskEventSpy eventSpy(jobCtrl, anTASKEVT_PG_UNLINK);
	PlaygroundUnlinkTask unlinkTask(5, true);
	PlaygroundUnlinkTask checkTask(5);
	std::map<std::pair<uint64_t, uint64_t>, int> list;

	DUMP_PLAYGROUND;
	jobCtrl->addTask(&unlinkTask);
	eventSpy.waitForInvocation(1);
	mark_point();

	fail_unless(!unlinkTask.hasErrorList());
	list = unlinkTask.getErrorList();
	fail_unless(list.size() == 0);
	fail_unless(unlinkTask.getComTaskResult() == ComTask::RESULT_SUCCESS);

	/*
	 * Removing the same playground again will result in a ESRCH because
	 * the playground is alredy removed.
	 */
	jobCtrl->addTask(&checkTask);
	eventSpy.waitForInvocation(2);

	fail_if(checkTask.getComTaskResult() == ComTask::RESULT_SUCCESS);
	fail_unless(checkTask.getResultDetails() == ESRCH);
}
END_TEST

TCase *
getTc_JobCtrlPlayground(void)
{
	TCase *tcase = tcase_create("JobCtrl_Playground");

	tcase_set_timeout(tcase, 10);
	tcase_add_checked_fixture(tcase, setup, teardown);

	tcase_add_test(tcase, fetch_list);

	tcase_add_test(tcase, unlink_activePg);
	tcase_add_test(tcase, unlink_noentPg);
	tcase_add_test(tcase, unlink_completeSimple);
	tcase_add_test(tcase, unlink_completeComplex);
	tcase_add_test(tcase, unlink_listEmpty);
	tcase_add_test(tcase, unlink_listComplex);
	tcase_add_test(tcase, unlink_errorList);
	tcase_add_test(tcase, unlink_force);

	return (tcase);
}

#endif /* LINUX */

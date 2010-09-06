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

#include <sys/wait.h>

#include <check.h>
#include <stdio.h>
#include <JobCtrl.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "dummyDaemon.h"
#include "utils.h"
#include "CmdEventSpy.h"
#include "PlaygroundCtrl.h"
#include "PlaygroundInfoEntry.h"
#include "PlaygroundFileEntry.h"

#include <anoubis_errno.h>
#include <anoubis_playground.h>

static void
run_pg_cmd(char** cmd) {
	int pid = playground_start_fork(cmd);
	fail_unless(pid > 0,
	    "Failed to fork playground-process: %d (%s)",
	    -pid, strerror(-pid));
	waitpid(pid, NULL, 0);
}

static void
setup(void)
{
	setup_dummyDaemon();

	const char *playground1[] = {
	    "/bin/bash", "-c", "sleep 20 &"
	};
	int pid = playground_start_fork((char**)playground1);
	fail_unless(pid > 0, "Failed to fork playground-process: %d (%s)",
	    -pid, strerror(-pid));
	/* Note: we do not wait for this shell to finish, no cleanup is done */

	const char *playground2[] = {
	    "/bin/bash", "-c", "echo hallo > xxx"
	};
	run_pg_cmd((char**)playground2);

	const char *playground3[] = {
	    "/bin/bash", "-c", "touch foo; ln foo foo1; ln foo foo2; rm foo"
	};
	run_pg_cmd((char**)playground3);

	/* make sure commands end before we continue */
	sleep(1);
}

static void
teardown(void)
{
	teardown_dummyDaemon();
}

static AnRowProvider*
getPlaygroundInfos(void)
{
	JobCtrl *jobctl = JobCtrl::instance();
	PlaygroundCtrl *pgctl = PlaygroundCtrl::instance();

	/* get all playgrounds */
	AnRowProvider *pg_info = (AnRowProvider*)pgctl->getInfoProvider();
	CmdEventSpy info_spy(pg_info, anEVT_ROW_SIZECHANGE);

	pgctl->updatePlaygroundInfo();

	while (info_spy.getNumInvocations() < 1) {
		jobctl->ProcessPendingEvents();
		pg_info->ProcessPendingEvents();
	}

	return pg_info;
}

static AnRowProvider*
getPlaygroundFiles(uint64_t pgid) {
	JobCtrl *jobctl = JobCtrl::instance();
	PlaygroundCtrl *pgctl = PlaygroundCtrl::instance();

	/* get this playgrounds files */

	AnRowProvider *pg_files =
	    (AnRowProvider*)pgctl->getFileProvider();
	CmdEventSpy file_spy(pg_files, anEVT_ROW_SIZECHANGE);

	pgctl->updatePlaygroundFiles(pgid);

	while (file_spy.getNumInvocations() < 1) {
		jobctl->ProcessPendingEvents();
		pg_files->ProcessPendingEvents();
	}

	return pg_files;
}

/* dumps all playground and their files to stdout, mainly for debugging */
void dump_playgrounds(void) {
	/* get all playgrounds */
	AnRowProvider *pg_info = getPlaygroundInfos();

	for (int i=0; i<pg_info->getSize(); i++) {
		AnListClass *row = pg_info->getRow(i);
		PlaygroundInfoEntry *entry =
		    dynamic_cast<PlaygroundInfoEntry *>(row);

		printf("playground: '%ls' pgid=%"PRIx64
		    " uid=%u files=%d active=%d\n",
		    (const wchar_t*)entry->getPath().c_str(), entry->getPgid(),
		    entry->getUid(), entry->getNumFiles(), entry->isActive());

		/* get this playgrounds files */
		AnRowProvider *pg_files = getPlaygroundFiles(entry->getPgid());

		for (int j=0; j<pg_files->getSize(); j++) {
			AnListClass *row = pg_files->getRow(j);
			PlaygroundFileEntry *file =
			    dynamic_cast<PlaygroundFileEntry *>(row);

			for (unsigned int k=0; k<file->getPaths().size(); k++)
			{
				printf("  '%ls' pgid=%"PRIx64" dev=%"PRIx64
				    " inode=%"PRIx64"\n", (const wchar_t*)
				    file->getPaths()[k].c_str(),
				    file->getPgid(), file->getDevice(),
				    file->getInode());
			}
		}
	}
}


START_TEST(fetch_file_list)
{
	/*
	 * Attention:
	 * This test uses hardcoded filenames for the playground files, this
	 * is not nice but there is no effective way to do better right row.
	 */

	dump_playgrounds();

	printf("starting test:\n");

	AnRowProvider *pg_info;
	AnRowProvider *pg_files;
	PlaygroundInfoEntry *info_entry;
	PlaygroundFileEntry *file_entry;

	/* get playgrounds */
	pg_info = getPlaygroundInfos();
	fail_unless(pg_info->getSize() == 3);


	/* check 1st playground */
	info_entry = dynamic_cast<PlaygroundInfoEntry *>(pg_info->getRow(2));
	fail_unless(info_entry != 0);
	fail_unless(info_entry->getPath().Cmp(wxT("/bin/bash")) == 0);
	fail_unless(info_entry->getNumFiles() == 0);
	fail_unless(info_entry->isActive() == 1);

	/* check files of 1st playground */
	pg_files = getPlaygroundFiles(info_entry->getPgid());
	fail_unless(pg_files->getSize() == 0);


	/* check 2nd playground */
	info_entry = dynamic_cast<PlaygroundInfoEntry *>(pg_info->getRow(1));
	fail_unless(info_entry != 0);
	fail_unless(info_entry->getPath().Cmp(wxT("/bin/bash")) == 0);
	fail_unless(info_entry->getNumFiles() == 1);
	fail_unless(info_entry->isActive() == 0);

	/* check files of 2nd playground */
	pg_files = getPlaygroundFiles(info_entry->getPgid());
	fail_unless(pg_files->getSize() == 1);

	file_entry = dynamic_cast<PlaygroundFileEntry *>(pg_files->getRow(0));
	fail_unless(file_entry != 0);
	fail_unless(file_entry->getPaths().size() == 1);
	fail_unless(file_entry->getPaths()[0].Cmp(
	    wxT("/home/u2000/xxx")) == 0);


	/** check 3rd playground */
	info_entry = dynamic_cast<PlaygroundInfoEntry *>(pg_info->getRow(0));
	fail_unless(info_entry != 0);
	fail_unless(info_entry->getPath().Cmp(wxT("/bin/bash")) == 0);
	fail_unless(info_entry->getNumFiles() == 1);
	fail_unless(info_entry->isActive() == 0);

	/* check files of 3rd playground */
	pg_files = getPlaygroundFiles(info_entry->getPgid());
	fail_unless(pg_files->getSize() == 1);

	file_entry = dynamic_cast<PlaygroundFileEntry *>(pg_files->getRow(0));
	fail_unless(file_entry != 0);
	fail_unless(file_entry->getPaths().size() == 2);
	fail_unless(file_entry->getPaths()[0].Cmp(
	    wxT("/home/u2000/foo1")) == 0);
	fail_unless(file_entry->getPaths()[1].Cmp(
	    wxT("/home/u2000/foo2")) == 0);

	/* we did it ! */
	printf("test done\n");
}
END_TEST

TCase *
getTc_CtrlPlayground(void)
{
	TCase *tcase = tcase_create("CtrlPlayground");

	tcase_set_timeout(tcase, 10);
	tcase_add_checked_fixture(tcase, setup, teardown);

	tcase_add_test(tcase, fetch_file_list);

	return (tcase);
}

#endif /* LINUX */

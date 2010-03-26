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

#include <check.h>

#include <wx/app.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/module.h>

#include <ComSfsListTask.h>
#include <JobCtrl.h>
#include <KeyCtrl.h>

#include "JobCtrlEventSpy.h"
#include "TaskEventSpy.h"
#include "utils.h"

#include <anoubis_errno.h>

static wxApp *app = 0;
static JobCtrl *jobCtrl = 0;
static char testDir[PATH_MAX];

static bool
create_directory(const char *format, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, format);
	vsnprintf(path, sizeof(path), format, ap);
	va_end(ap);

	return (mkdir(path, S_IRWXU|S_IRWXG|S_IRWXO) == 0);
}

static bool
create_file(const char *format, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, format);
	vsnprintf(path, sizeof(path), format, ap);
	va_end(ap);

	FILE *f = fopen(path, "w");

	if (f == 0)
		return (false);
	if (fflush(f) != 0)
		return (false);
	fclose(f);

	return (true);
}

static void
setup(void)
{
	/* wxWidgets-infrastructure */
	app = new wxApp;
	wxApp::SetInstance(app);
	wxPendingEventsLocker = new wxCriticalSection;

	wxModule::RegisterModules();
	fail_unless(wxModule::InitializeModules(),
	    "Failed to initialize wxModules");

	/* Object to be tested */
	jobCtrl = JobCtrl::getInstance();
	fail_unless(jobCtrl->start(), "Failed to start JobCtrl");

	JobCtrlEventSpy spy(jobCtrl);
	fail_unless(jobCtrl->connect(), "Connection request failed");

	spy.waitForInvocation(1);
	fail_unless(spy.getLastState() == JobCtrl::CONNECTED,
	    "Unexpected connection-state; is: %i", spy.getLastState());

	LocalCertificate &cert = KeyCtrl::getInstance()->getLocalCertificate();
	cert.setFile(wxFileName::GetHomeDir() + wxT("/pubkey"));

	fail_unless(cert.load(), "Failed to load certificate");

	/* Test directory */
	strcpy(testDir, "/tmp/sfslist.XXXXXX");
	char *s = mkdtemp(testDir);
	fail_if(s == 0,
	    "Failed to create test-directory (%s)", anoubis_strerror(errno));

	fail_unless(create_directory("%s/empty", testDir));
	fail_unless(create_directory("%s/sub", testDir));
	fail_unless(create_file("%s/1", testDir));
	fail_unless(create_file("%s/2", testDir));
	fail_unless(create_file("%s/3", testDir));
	fail_unless(create_file("%s/sub/1", testDir));
	fail_unless(create_file("%s/sub/2", testDir));
	fail_unless(create_file("%s/sub/3", testDir));

	fail_unless(JobCtrl_execute("/t/sfssig -r add %s", testDir));
	fail_unless(JobCtrl_execute(
	    "/t/sfssig --sig -k ~/privkey -c ~/pubkey add %s/sub/1", testDir));
	fail_unless(JobCtrl_execute(
	    "/t/sfssig --sig -k ~/privkey -c ~/pubkey add %s/sub/2", testDir));
}

static void
teardown(void)
{
	JobCtrl_execute("/t/sfssig -r del %s", testDir);
	JobCtrl_execute(
	    "/t/sfssig --sig -r -k ~/privkey -c ~/pubkey del %s", testDir);
	JobCtrl_execute("rm -rf %s", testDir);

	/* Destry test-object */
	JobCtrlEventSpy *spy = new JobCtrlEventSpy(jobCtrl);

	jobCtrl->disconnect();
	spy->waitForInvocation(1);
	fail_unless(spy->getLastState() == JobCtrl::DISCONNECTED,
	    "Unexpected connection-state; is: %i", spy->getLastState());

	delete spy;

	jobCtrl->stop();
	delete jobCtrl;
	jobCtrl = 0;

	/* Destroy wxWidgets-infrastructure */
	wxModule::CleanUpModules();
	wxApp::SetInstance(0);

	delete app;
	delete wxPendingEventsLocker;
	wxPendingEventsLocker = 0;
}

START_TEST(test_csum_nonempty)
{
	TaskEventSpy spy(jobCtrl, anTASKEVT_SFS_LIST);
	ComSfsListTask task;
	task.setRequestParameter(getuid(), wxString(testDir, wxConvFile));

	jobCtrl->addTask(&task);
	spy.waitForInvocation(1);

	fail_unless(task.haveKeyId() == false, "A key-id is assigned");

	wxArrayString result = task.getFileList();

	int idx = result.Index(wxT("1"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"1\" not found");
	result.RemoveAt(idx);

	idx = result.Index(wxT("2"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"2\" not found");
	result.RemoveAt(idx);

	idx = result.Index(wxT("3"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"3\" not found");
	result.RemoveAt(idx);

	fail_unless(result.Count() == 0,
	    "After removing all expected entries, an empty list is expected\n"
	    "Is: %i\n", result.Count());
}
END_TEST

START_TEST(test_csum_empty)
{
	TaskEventSpy spy(jobCtrl, anTASKEVT_SFS_LIST);
	ComSfsListTask task;
	task.setRequestParameter(
	    getuid(), wxString(testDir, wxConvFile) + wxT("/empty"));

	jobCtrl->addTask(&task);
	spy.waitForInvocation(1);

	fail_unless(task.haveKeyId() == false, "A key-id is assigned");

	wxArrayString result = task.getFileList();
	fail_unless(result.Count() == 0,
	    "Unexpected # of entries in sfs-list\n"
	    "Expected: 0\n"
	    "Is: %i\n", result.Count());
}
END_TEST

START_TEST(test_csum_recursive)
{
	TaskEventSpy spy(jobCtrl, anTASKEVT_SFS_LIST);
	ComSfsListTask task;
	task.setRequestParameter(getuid(), wxString(testDir, wxConvFile));
	task.setRecursive(true);

	jobCtrl->addTask(&task);
	spy.waitForInvocation(1);

	fail_unless(task.haveKeyId() == false, "A key-id is assigned");

	wxArrayString result = task.getFileList();

	int idx = result.Index(wxT("1"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"1\" not found");
	result.RemoveAt(idx);

	idx = result.Index(wxT("2"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"2\" not found");
	result.RemoveAt(idx);

	idx = result.Index(wxT("3"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"3\" not found");
	result.RemoveAt(idx);

	idx = result.Index(wxT("sub/1"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"sub/1\" not found");
	result.RemoveAt(idx);

	idx = result.Index(wxT("sub/2"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"sub/2\" not found");
	result.RemoveAt(idx);

	idx = result.Index(wxT("sub/3"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"sub/3\" not found");
	result.RemoveAt(idx);

	fail_unless(result.Count() == 0,
	    "After removing all expected entries, an empty list is expected\n"
	    "Is: %i\n", result.Count());
}
END_TEST

START_TEST(test_sig_nonempty)
{
	TaskEventSpy spy(jobCtrl, anTASKEVT_SFS_LIST);
	ComSfsListTask task;
	task.setRequestParameter(
	    getuid(), wxString(testDir, wxConvFile) + wxT("/sub"));

	LocalCertificate &cert = KeyCtrl::getInstance()->getLocalCertificate();
	struct anoubis_sig *raw_cert = cert.getCertificate();
	fail_unless(task.setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");

	jobCtrl->addTask(&task);
	spy.waitForInvocation(1);

	wxArrayString result = task.getFileList();

	int idx = result.Index(wxT("1"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"1\" not found");
	result.RemoveAt(idx);

	idx = result.Index(wxT("2"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"2\" not found");
	result.RemoveAt(idx);

	fail_unless(result.Count() == 0,
	    "After removing all expected entries, an empty list is expected\n"
	    "Is: %i\n", result.Count());
}
END_TEST

START_TEST(test_sig_empty)
{
	TaskEventSpy spy(jobCtrl, anTASKEVT_SFS_LIST);
	ComSfsListTask task;
	task.setRequestParameter(getuid(), wxString(testDir, wxConvFile));

	LocalCertificate &cert = KeyCtrl::getInstance()->getLocalCertificate();
	struct anoubis_sig *raw_cert = cert.getCertificate();
	fail_unless(task.setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");

	jobCtrl->addTask(&task);
	spy.waitForInvocation(1);

	wxArrayString result = task.getFileList();
	fail_unless(result.Count() == 0,
	    "Unexpected # of entries in sfs-list\n"
	    "Expected: 0\n"
	    "Is: %i\n", result.Count());
}
END_TEST

START_TEST(test_sig_recursive)
{
	TaskEventSpy spy(jobCtrl, anTASKEVT_SFS_LIST);
	ComSfsListTask task;
	task.setRequestParameter(getuid(), wxString(testDir, wxConvFile));
	task.setRecursive(true);

	LocalCertificate &cert = KeyCtrl::getInstance()->getLocalCertificate();
	struct anoubis_sig *raw_cert = cert.getCertificate();
	fail_unless(task.setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");

	jobCtrl->addTask(&task);
	spy.waitForInvocation(1);

	wxArrayString result = task.getFileList();

	int idx = result.Index(wxT("sub/1"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"sub/1\" not found");
	result.RemoveAt(idx);

	idx = result.Index(wxT("sub/2"));
	fail_unless(idx != wxNOT_FOUND, "Entry \"sub/2\" not found");
	result.RemoveAt(idx);

	fail_unless(result.Count() == 0,
	    "After removing all expected entries, an empty list is expected\n"
	    "Is: %i\n", result.Count());
}
END_TEST

TCase *
getTc_JobCtrlSfsList(void)
{
	TCase *tcase = tcase_create("JobCtrl_SfsList");

	tcase_set_timeout(tcase, 10);
	tcase_add_checked_fixture(tcase, setup, teardown);

	tcase_add_test(tcase, test_csum_nonempty);
	tcase_add_test(tcase, test_csum_empty);
	tcase_add_test(tcase, test_csum_recursive);
	tcase_add_test(tcase, test_sig_nonempty);
	tcase_add_test(tcase, test_sig_empty);
	tcase_add_test(tcase, test_sig_recursive);

	return (tcase);
}

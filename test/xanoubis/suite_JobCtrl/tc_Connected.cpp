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
#include <wx/ffile.h>
#include <wx/module.h>

#include <apn.h>

#include <ComCsumAddTask.h>
#include <ComCsumDelTask.h>
#include <ComCsumGetTask.h>
#include <ComPolicyRequestTask.h>
#include <ComPolicySendTask.h>
#include <JobCtrl.h>
#include <KeyCtrl.h>
#include <PolicyRuleSet.h>

#include "JobCtrlEventSpy.h"
#include "TaskEventSpy.h"
#include "utils.h"

static wxApp *app = 0;
static JobCtrl *jobCtrl = 0;

extern "C" int
privkey_1234_cb(char *buf, int size, int, void *)
{
	char pw[] = "1234";
	int len = strlen(pw);
	if (len > size)
		len = size;
	memcpy(buf, pw, len);
	return len;
}

static wxString
anoubisctl_dump(void)
{
	wxFFile policyFile;
	wxString policyFileName = wxFileName::CreateTempFileName(
	    wxT("policy"), &policyFile);
	wxString policy;

	wxString cmd = wxString::Format(
	    wxT("/t/anoubisctl dump %ls"), policyFileName.c_str());

	if (system(cmd.To8BitData()) == 0) {
		policyFile.ReadAll(&policy);

		/* Cut the leading comment */
		int idx = policy.Find(wxT("apnversion"));
		if (idx != wxNOT_FOUND) {
			policy = policy.Mid(idx);
		}
	}

	policyFile.Close();
	wxRemoveFile(policyFileName);

	return (policy);
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

	/* The private key */
	PrivKey &privKey = KeyCtrl::getInstance()->getPrivateKey();
	privKey.setFile(wxFileName::GetHomeDir() + wxT("/privkey"));

	if (!privKey.isLoaded()) {
		fail_unless(
		    privKey.load(privkey_1234_cb) == PrivKey::ERR_PRIV_OK,
		    "Failed to load private key");
	}

	/* Certificate */
	LocalCertificate &cert = KeyCtrl::getInstance()->getLocalCertificate();
	cert.setFile(wxFileName::GetHomeDir() + wxT("/pubkey"));

	fail_unless(cert.load(), "Failed to load certificate");

	/* Object to be tested */
	jobCtrl = JobCtrl::getInstance();
	fail_unless(jobCtrl->start(), "Failed to start JobCtrl");

	JobCtrlEventSpy spy(jobCtrl);
	fail_unless(jobCtrl->connect(), "Connection request failed");

	spy.waitForInvocation(1);
	fail_unless(spy.getLastState() == JobCtrl::CONNECTED,
	    "Unexpected connection-state; is: %i", spy.getLastState());
}

static void
teardown(void)
{
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

START_TEST(test_fetchversion)
{
	TaskEventSpy spy(jobCtrl, anTASKEVT_VERSION);

	ComVersionTask task;
	jobCtrl->addTask(&task);

	spy.waitForInvocation(1);

	fail_unless(task.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to receive versionlist: %i\n", task.getComTaskResult());

	fail_unless(task.getProtocolVersion() == 5,
	    "Unexpected protocol version received (%i)\n",
	    task.getProtocolVersion());
	fail_unless(task.getApnVersion() == 65538,
	    "Unexpected APN version received (%i.%i)\n",
	    APN_PARSER_MAJOR(task.getApnVersion()),
	    APN_PARSER_MINOR(task.getApnVersion()));
}
END_TEST

START_TEST(test_policyrequest)
{
	wxString policy_in, policy_out;

	/* Read policy from daemon (using anoubisctl) */
	policy_in = anoubisctl_dump();
	fail_if(policy_in.IsEmpty(), "Failed to fetch policy from daemon");

	/* Read policy from daemon (using JobCtrl) */
	TaskEventSpy spy(jobCtrl, anTASKEVT_POLICY_REQUEST);
	ComPolicyRequestTask task;
	task.setRequestParameter(1, getuid());

	jobCtrl->addTask(&task);

	spy.waitForInvocation(1);

	fail_unless(task.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to receive a policy: %i\n", task.getComTaskResult());
	fail_unless(task.getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(task.getResultDetails()), task.getResultDetails());

	PolicyRuleSet *rs = task.getPolicy();
	rs->toString(policy_out);
	delete rs;

	fail_unless(policy_in == policy_out,
	    "Unexpected policy fetched\nis:\n%ls\nexpected:\n%ls",
	    policy_out.c_str(), policy_in.c_str());
}
END_TEST

START_TEST(test_policysend_empty)
{
	TaskEventSpy spy(jobCtrl, anTASKEVT_POLICY_SEND);

	ComPolicySendTask task;
	task.setPrivateKey(KeyCtrl::getInstance()->getPrivateKey().getKey());
	jobCtrl->addTask(&task);

	spy.waitForInvocation(1);

	fail_unless(task.getComTaskResult() == ComTask::RESULT_INIT,
	    "Task in wrong state: %i\n", task.getComTaskResult());

	fail_unless(task.getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(task.getResultDetails()), task.getResultDetails());
}
END_TEST

START_TEST(test_policysend)
{
	TaskEventSpy spy(jobCtrl, anTASKEVT_POLICY_SEND);

	struct apn_ruleset *rs;
	wxString policyFile = wxFileName::GetHomeDir() + wxT("/policy");
	fail_unless(apn_parse(policyFile.fn_str(), &rs, 0) == 0,
	    "Failed to parse the policy");

	ComPolicySendTask task;
	task.setPolicy(rs, getuid(), 1);
	task.setPrivateKey(KeyCtrl::getInstance()->getPrivateKey().getKey());

	apn_free_ruleset(rs);

	jobCtrl->addTask(&task);
	spy.waitForInvocation(1);

	fail_unless(task.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to send a policy: %i\n", task.getComTaskResult());
	fail_unless(task.getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(task.getResultDetails()), task.getResultDetails());
}
END_TEST

START_TEST(test_csum)
{
	size_t		len;

	wxString fileName = JobCtrl_tempfile();
	fail_if(fileName.IsEmpty(), "Failed to create file");

	u_int8_t cs_in[ANOUBIS_CS_LEN];
	fail_unless(JobCtrl_calculate_checksum(fileName, cs_in, sizeof(cs_in)),
	    "Failed to calculate checksum");

	/* Register checksum: Success */
	TaskEventSpy add_spy(jobCtrl, anTASKEVT_CSUM_ADD);
	ComCsumAddTask add_task;
	add_task.addPath(fileName);

	jobCtrl->addTask(&add_task);
	add_spy.waitForInvocation(1);

	fail_unless(add_task.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to add a checksum: %i\n", add_task.getComTaskResult());
	fail_unless(add_task.getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n", strerror(add_task.getResultDetails()),
	    add_task.getResultDetails());

	/* Receive checksum: Success */
	TaskEventSpy get_spy(jobCtrl, anTASKEVT_CSUM_GET);
	ComCsumGetTask *get_task = new ComCsumGetTask;
	get_task->addPath(fileName);

	jobCtrl->addTask(get_task);
	get_spy.waitForInvocation(1);

	fail_unless(get_task->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to get a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i (%s)\n",
	    get_task->getComTaskResult(), get_task->getResultDetails(),
	    strerror(get_task->getResultDetails()));
	fail_unless(get_task->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n", strerror(get_task->getResultDetails()),
	    get_task->getResultDetails());
	fail_unless(get_task->haveKeyId() == false, "A key-id is assigned");
	len = get_task->getChecksumLen(0, ANOUBIS_SIG_TYPE_CS);
	fail_unless(len == ANOUBIS_CS_LEN,
	    "Unexpected csum-len\nIs: %ld\nExpected: %d\n",
	    len, ANOUBIS_CS_LEN);

	const u_int8_t *cs_out;
	bool ok = get_task->getChecksumData(0, ANOUBIS_SIG_TYPE_CS,
	    cs_out, len);
	fail_unless(ok && len == ANOUBIS_CS_LEN,
	    "Unexpected checksum received!");
	fail_unless(memcmp(cs_in, cs_out, ANOUBIS_CS_LEN) == 0,
	    "Unexpected checksum fetched from daemon");

	/* Remove checksum: Success */
	TaskEventSpy del_spy(jobCtrl, anTASKEVT_CSUM_DEL);
	ComCsumDelTask del_task;
	del_task.setPath(fileName);

	jobCtrl->addTask(&del_task);
	del_spy.waitForInvocation(1);

	fail_unless(del_task.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to remove a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i\n",
	    del_task.getComTaskResult(), del_task.getResultDetails());
	fail_unless(del_task.haveKeyId() == false, "A key-id is assigned");
	fail_unless(del_task.getResultDetails() == 0,
	    "ResultDetails: %s (%i)", strerror(del_task.getResultDetails()),
	    del_task.getResultDetails());

	delete get_task;
	get_task = new ComCsumGetTask;
	get_task->addPath(fileName);
	/* Fetch checksum again: Failure */
	jobCtrl->addTask(get_task);
	get_spy.waitForInvocation(2);

	fail_unless(get_task->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Checksum request failed\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i\n",
	    get_task->getComTaskResult(), get_task->getResultDetails());
	fail_unless(get_task->getChecksumError(0) == ENOENT,
	    "Checksum error: %s (%i)\n",
	    strerror(get_task->getChecksumError(0)),
	    get_task->getChecksumError(0));
	fail_unless(get_task->haveKeyId() == false, "A key-id is assigned");
	len = get_task->getChecksumLen(0, ANOUBIS_SIG_TYPE_CS);
	fail_unless(len == 0, "Task contains a checksum");
	ok = get_task->getChecksumData(0, ANOUBIS_SIG_TYPE_CS, cs_out, len);
	fail_unless(!ok && len == 0, "Task contains a checksum!\n");

	wxRemoveFile(fileName);
}
END_TEST

START_TEST(test_csum_nosuchfile)
{
	size_t		len;
	TaskEventSpy spy(jobCtrl, anTASKEVT_CSUM_GET);

	wxString file = wxFileName::GetTempDir() + wxT("/blablubb");
	fail_if(wxFileExists(file), "File already exists");

	ComCsumGetTask *task = new ComCsumGetTask;
	task->addPath(file);

	jobCtrl->addTask(task);
	spy.waitForInvocation(1);

	fail_unless(task->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Checksum request failed!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i\n",
	    task->getComTaskResult(), task->getResultDetails());
	fail_unless(task->getChecksumError(0) == ENOENT,
	    "Checksum error: %s (%i)\n",
	    strerror(task->getChecksumError(0)), task->getChecksumError(0));
	fail_unless(task->haveKeyId() == false, "A key-id is assigned");
	len = task->getChecksumLen(0, ANOUBIS_SIG_TYPE_CS);
	fail_unless(len == 0, "Task contains a checksum");

	const u_int8_t *cs;
	bool ok = task->getChecksumData(0, ANOUBIS_SIG_TYPE_CS, cs, len);
	fail_unless(!ok && len == 0, "Task contains a checksum!\n");
}
END_TEST

START_TEST(test_csum_orphaned)
{
	size_t		len;
	/* Create a new file */
	wxString fileName = JobCtrl_tempfile();
	fail_if(fileName.IsEmpty(), "Failed to create file");

	u_int8_t cs_in[ANOUBIS_CS_LEN];
	fail_unless(JobCtrl_calculate_checksum(fileName, cs_in, sizeof(cs_in)),
	    "Failed to calculate checksum");

	/* Register checksum: Success */
	TaskEventSpy add_spy(jobCtrl, anTASKEVT_CSUM_ADD);
	ComCsumAddTask add_task;
	add_task.addPath(fileName);

	jobCtrl->addTask(&add_task);
	add_spy.waitForInvocation(1);

	fail_unless(add_task.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to add a checksum: %i\n", add_task.getComTaskResult());
	fail_unless(add_task.getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n", strerror(add_task.getResultDetails()),
	    add_task.getResultDetails());
	fail_unless(add_task.haveSignatures() == false, "A key-id is assigned");

	/* Remove file -> orphaned */
	fail_unless(wxRemoveFile(fileName), "Failed to remove file");

	/* Receive checksum: Success */
	TaskEventSpy get_spy(jobCtrl, anTASKEVT_CSUM_GET);
	ComCsumGetTask *get_task = new ComCsumGetTask;
	get_task->addPath(fileName);

	jobCtrl->addTask(get_task);
	get_spy.waitForInvocation(1);

	fail_unless(get_task->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to get a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i (%s)\n",
	    get_task->getComTaskResult(), get_task->getResultDetails(),
	    strerror(get_task->getResultDetails()));
	fail_unless(get_task->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n", strerror(get_task->getResultDetails()),
	    get_task->getResultDetails());
	fail_unless(get_task->haveKeyId() == false, "A key-id is assigned");
	len = get_task->getChecksumLen(0, ANOUBIS_SIG_TYPE_CS);
	fail_unless(len == ANOUBIS_CS_LEN,
	    "Unexpected csum-len\nIs: %i\nExpected: %i\n",
	    len, ANOUBIS_CS_LEN);

	const u_int8_t *cs_out;
	bool ok = get_task->getChecksumData(0, ANOUBIS_SIG_TYPE_CS,
	    cs_out, len);
	fail_unless(ok && len == ANOUBIS_CS_LEN,
	    "Unexpected checksum received!");
	fail_unless(memcmp(cs_in, cs_out, ANOUBIS_CS_LEN) == 0,
	    "Unexpected checksum fetched from daemon");
}
END_TEST

START_TEST(test_csum_symlink)
{
	/* Create a new file */
	wxString fileName = JobCtrl_tempfile();
	fail_if(fileName.IsEmpty(), "Failed to create file");

	u_int8_t cs_in[ANOUBIS_CS_LEN];
	fail_unless(JobCtrl_calculate_checksum(fileName, cs_in, sizeof(cs_in)),
	    "Failed to calculate checksum");

	/* Register checksum: Success */
	TaskEventSpy add_spy(jobCtrl, anTASKEVT_CSUM_ADD);
	ComCsumAddTask add_task;
	add_task.addPath(fileName);

	jobCtrl->addTask(&add_task);
	add_spy.waitForInvocation(1);

	fail_unless(add_task.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to add a checksum: %i\n", add_task.getComTaskResult());
	fail_unless(add_task.getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n", strerror(add_task.getResultDetails()),
	    add_task.getResultDetails());
	fail_unless(add_task.haveSignatures() == false, "A key-id is assigned");

	/* Symlink the file */
	wxString symlinkName = wxFileName::GetTempDir() + wxT("/csumsymlink");
	fail_if(wxFileExists(symlinkName), "Symlink already exists");
	fail_unless(
	    symlink(fileName.To8BitData(), symlinkName.To8BitData()) == 0,
	    "Failed to create symlink (%s)", strerror(errno));

	/* Receive checksum: Success */
	TaskEventSpy get_spy(jobCtrl, anTASKEVT_CSUM_GET);
	ComCsumGetTask *get_task = new ComCsumGetTask;
	get_task->addPath(symlinkName);

	jobCtrl->addTask(get_task);
	get_spy.waitForInvocation(1);

	fail_unless(get_task->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to get a checksum!\nComTaskResult = %i\n"
	    "ResultDetails = %s\n", get_task->getComTaskResult(),
	    strerror(get_task->getResultDetails()));
	fail_unless(get_task->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n", strerror(get_task->getResultDetails()),
	    get_task->getResultDetails());
	fail_unless(get_task->haveKeyId() == false, "A key-id is assigned");
	size_t len = get_task->getChecksumLen(0, ANOUBIS_SIG_TYPE_CS);
	/*
	 * No checksum expected, get_task is for symlinkName while
	 * add_task is for filename.
	 */
	fail_unless(len == 0,
	    "Unexpected csum-len\nIs: %i\nExpected: 0\n", len);

	const u_int8_t *cs_out;
	bool ok = get_task->getChecksumData(0, ANOUBIS_SIG_TYPE_CS,
	    cs_out, len);
	fail_unless(!ok && len == 0, "Unexpected checksum received!");

	unlink(symlinkName.To8BitData());
	wxRemoveFile(fileName);
}
END_TEST

START_TEST(test_csum_symlink_link)
{
	/* Create a new file */
	mark_point();
	wxString fileName = JobCtrl_tempfile();
	fail_if(fileName.IsEmpty(), "Failed to create file");
	mark_point();

	u_int8_t cs_in[ANOUBIS_CS_LEN];
	fail_unless(JobCtrl_calculate_checksum_buffer(
	    fileName.To8BitData(), fileName.Len(), cs_in, sizeof(cs_in)),
	    "Failed to calculate checksum");

	mark_point();
	/* Symlink the file */
	wxString symlinkName = wxFileName::GetTempDir() + wxT("/csumsymlink");
	fail_if(wxFileExists(symlinkName), "Symlink already exists");
	fail_unless(
	    symlink(fileName.To8BitData(), symlinkName.To8BitData()) == 0,
	    "Failed to create symlink (%s)", strerror(errno));
	mark_point();

	/* Register symlink at daemon: Success */
	TaskEventSpy add_spy(jobCtrl, anTASKEVT_CSUM_ADD);
	mark_point();
	ComCsumAddTask add_task;
	add_task.addPath(symlinkName);
	mark_point();

	jobCtrl->addTask(&add_task);
	add_spy.waitForInvocation(1);
	mark_point();

	fail_unless(add_task.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to add a checksum: %i\n", add_task.getComTaskResult());
	fail_unless(add_task.getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n", strerror(add_task.getResultDetails()),
	    add_task.getResultDetails());
	fail_unless(add_task.haveSignatures() == false, "A key-id is assigned");
	mark_point();

	/* Receive from daemon: success */
	TaskEventSpy get_spy(jobCtrl, anTASKEVT_CSUM_GET);
	ComCsumGetTask *get_task = new ComCsumGetTask;
	get_task->addPath(symlinkName);
	mark_point();

	jobCtrl->addTask(get_task);
	get_spy.waitForInvocation(1);
	mark_point();

	fail_unless(get_task->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to get a checksum!\nComTaskResult = %i\n"
	    "ResultDetails = %s\n", get_task->getComTaskResult(),
	    strerror(get_task->getResultDetails()));
	fail_unless(get_task->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n", strerror(get_task->getResultDetails()),
	    get_task->getResultDetails());
	fail_unless(get_task->haveKeyId() == false, "A key-id is assigned");
	mark_point();
	size_t len = get_task->getChecksumLen(0, ANOUBIS_SIG_TYPE_CS);
	mark_point();
	fail_unless(len == ANOUBIS_CS_LEN,
	    "Unexpected csum-len\n"
	    "Is: %i\n"
	    "Expected: %i\n", len, ANOUBIS_CS_LEN);

	mark_point();
	const u_int8_t *cs_out;
	bool ok = get_task->getChecksumData(0, ANOUBIS_SIG_TYPE_CS,
	    cs_out, len);
	fail_unless(ok && len == ANOUBIS_CS_LEN,
	    "Unexpected checksum received!");
	fail_unless(memcmp(cs_in, cs_out, ANOUBIS_CS_LEN) == 0,
	    "Unexpected checksum fetched from daemon");
	mark_point();

	wxRemoveFile(symlinkName);
	wxRemoveFile(fileName);
	mark_point();
}
END_TEST

START_TEST(test_signature)
{
	mark_point();
	LocalCertificate &cert = KeyCtrl::getInstance()->getLocalCertificate();
	struct anoubis_sig *raw_cert = cert.getCertificate();

	mark_point();
	/* Create a new file */
	wxString fileName = JobCtrl_tempfile();
	fail_if(fileName.IsEmpty(), "Failed to create file");

	mark_point();
	u_int8_t cs_in[ANOUBIS_CS_LEN];
	fail_unless(JobCtrl_calculate_checksum(fileName, cs_in, sizeof(cs_in)),
	    "Failed to calculate checksum");

	mark_point();
	/* Register signature: Success */
	TaskEventSpy add_spy(jobCtrl, anTASKEVT_CSUM_ADD);

	mark_point();
	ComCsumAddTask add_task;
	add_task.addPath(fileName);
	fail_unless(add_task.setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");
	add_task.setPrivateKey(
	    KeyCtrl::getInstance()->getPrivateKey().getKey());
	mark_point();

	jobCtrl->addTask(&add_task);
	add_spy.waitForInvocation(1);

	mark_point();
	/* Receive signature: Success */
	TaskEventSpy get_spy(jobCtrl, anTASKEVT_CSUM_GET);

	mark_point();
	ComCsumGetTask *get_task = new ComCsumGetTask;
	get_task->addPath(fileName);
	fail_unless(get_task->setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");

	mark_point();
	jobCtrl->addTask(get_task);
	get_spy.waitForInvocation(1);

	mark_point();
	fail_unless(get_task->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to get a checksum!\nComTaskResult = %i\n"
	    "ResultDetails = %i\n", get_task->getComTaskResult(),
	    get_task->getResultDetails());
	fail_unless(get_task->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n", strerror(get_task->getResultDetails()),
	    get_task->getResultDetails());
	fail_unless(get_task->haveKeyId(), "No key-id is assigned");
	size_t len = get_task->getChecksumLen(0, ANOUBIS_SIG_TYPE_SIG);
	fail_unless(len > ANOUBIS_CS_LEN, "Unexpected csum-len\nIs: %i", len);
	mark_point();

	/* Remove signature: Success */
	TaskEventSpy del_spy(jobCtrl, anTASKEVT_CSUM_DEL);
	ComCsumDelTask del_task;
	del_task.setPath(fileName);
	fail_unless(del_task.setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");
	mark_point();

	jobCtrl->addTask(&del_task);
	del_spy.waitForInvocation(1);
	mark_point();

	fail_unless(del_task.getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to remove a checksum!\nComTaskResult = %i\n"
	    "ResultDetails = %i\n", del_task.getComTaskResult(),
	    del_task.getResultDetails());
	fail_unless(del_task.getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n", strerror(del_task.getResultDetails()),
	    del_task.getResultDetails());
	fail_unless(del_task.haveKeyId(), "No key-id is assigned");

	mark_point();
	delete get_task;
	get_task = new ComCsumGetTask;
	get_task->addPath(fileName);
	mark_point();
	fail_unless(get_task->setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");
	mark_point();
	/* Fetch signature again: Failure */
	jobCtrl->addTask(get_task);
	get_spy.waitForInvocation(2);

	mark_point();
	fail_unless(get_task->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Checksum request failed!\nComTaskResult = %i\nResultDetails = %i",
	    get_task->getComTaskResult(), get_task->getResultDetails());
	fail_unless(get_task->getSignatureError(0) == ENOENT,
	    "Signature error: %s (%i)\n",
	    strerror(get_task->getSignatureError(0)),
	    get_task->getSignatureError(0));
	len = get_task->getChecksumLen(0, ANOUBIS_SIG_TYPE_SIG);
	fail_unless(len == 0, "Task contains a signature");
	mark_point();

	wxRemoveFile(fileName);
	mark_point();
}
END_TEST

TCase *
getTc_JobCtrlConnected(void)
{
	TCase *tcase = tcase_create("JobCtrl_Connected");

	tcase_set_timeout(tcase, 10);
	tcase_add_checked_fixture(tcase, setup, teardown);

	tcase_add_test(tcase, test_fetchversion);
	tcase_add_test(tcase, test_policyrequest);
	tcase_add_test(tcase, test_policysend_empty);
	tcase_add_test(tcase, test_policysend);
	tcase_add_test(tcase, test_csum);
	tcase_add_test(tcase, test_csum_nosuchfile);
	tcase_add_test(tcase, test_csum_orphaned);
	tcase_add_test(tcase, test_csum_symlink);
	tcase_add_test(tcase, test_csum_symlink_link);
	tcase_add_test(tcase, test_signature);

	return (tcase);
}

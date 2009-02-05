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

#include <errno.h>

#include <wx/ffile.h>
#include <wx/file.h>
#include <wx/filename.h>

#include <apn.h>
#include <anoubis_sig.h>

#include <ComCsumAddTask.h>
#include <ComCsumDelTask.h>
#include <ComCsumGetTask.h>
#include <ComPolicyRequestTask.h>
#include <ComPolicySendTask.h>
#include <ComRegistrationTask.h>
#include <ComSfsListTask.h>
#include <JobCtrl.h>
#include <KeyCtrl.h>

#include "TcComTask.h"

#define trace(format, args...) fprintf (stderr, format , ##args)

#define assertUnless(expr, msg, args...)	\
	if (!expr) {				\
		fprintf(stderr, msg, ##args);	\
		result_ = __LINE__;		\
		exit_ = true;			\
		return;				\
	}

const wxString policyCsum =
    wxT("09c2012a32bb5b776c8e0adc186df5b45279dd87dda249e877ec876b33d190f9");

const wxString symlink_get_1_csum =
    wxT("750f16659ec9fd87a2951f7538233b6d6bb2f4b285c9666c918de1af87a07465");

const wxString l1_csum =
    wxT("b66a213422e0bd251d5d900018086fcaa23581c2e46ca3ccf586ac8db32d0854");

const wxString l2_csum =
    wxT("5a0d272136895946c76efb7c98aa0a80cff7e5811a995ac52a6899ec5b45aa26");

TcComTask::TcComTask()
{
	this->testCounter_ = 0;
	this->exit_ = false;
	this->result_ = 0;

	trace("TcComTask::TcComTask finished\n");
}

TcComTask::~TcComTask()
{
	trace("TcComTask::~TcComTask finished\n");
}

void
TcComTask::startTest()
{
	nextTest();
}

bool
TcComTask::canExitTest() const
{
	return (exit_);
}

int
TcComTask::getTestResult() const
{
	return (result_);
}

void
TcComTask::nextTest()
{
	JobCtrl *jobCtrl = JobCtrl::getInstance();

	jobCtrl->Disconnect(anTASKEVT_REGISTER);
	jobCtrl->Disconnect(anTASKEVT_POLICY_REQUEST);
	jobCtrl->Disconnect(anTASKEVT_POLICY_SEND);
	jobCtrl->Disconnect(anTASKEVT_CSUM_ADD);
	jobCtrl->Disconnect(anTASKEVT_CSUM_GET);
	jobCtrl->Disconnect(anTASKEVT_CSUM_DEL);
	jobCtrl->Disconnect(anTASKEVT_SFS_LIST);

	testCounter_++;

	switch (testCounter_) {
	case 1:
		setupTestRegister();
		break;
	case 2:
		setupTestPolicyRequest();
		break;
	case 3:
		setupTestPolicySend();
		break;
	case 4:
		setupTestPolicyRequest();
		break;
	case 5:
		setupTestCsumAdd();
		break;
	case 6:
		setupTestCsumAddSymlink();
		break;
	case 7:
		setupTestCsumAddSymlinkLink();
		break;
	case 8:
		setupTestCsumGet();
		break;
	case 9:
		setupTestCsumGetNoSuchFile();
		break;
	case 10:
		setupTestCsumGetSymlink();
		break;
	case 11:
		setupTestCsumGetSymlinkLink();
		break;
	case 12:
		setupTestSfsListNotEmpty();
		break;
	case 13:
		setupTestCsumDel();
		break;
	case 14:
		setupTestCsumDelSymlink();
		break;
	case 15:
		setupTestCsumDelSymlinkLink();
		break;
	case 16:
		setupTestSfsListEmpty();
		break;
	case 17:
		setupTestSfsListRecursive();
		break;
	case 18:
		setupTestSigAdd();
		break;
	case 19:
		setupTestSigGet();
		break;
	case 20:
		setupTestSigListNotEmpty();
		break;
	case 21:
		setupTestSigDel();
		break;
	case 22:
		setupTestSigListEmpty();
		break;
	case 23:
		setupTestUnregister();
		break;
	default:
		exit_ = true;
		break;
	}
}

void
TcComTask::setupTestRegister(void)
{
	trace("Enter TcComTask::setupTestRegister\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_REGISTER,
	    wxTaskEventHandler(TcComTask::onTestRegister), NULL, this);

	ComRegistrationTask *t = new ComRegistrationTask;
	t->setAction(ComRegistrationTask::ACTION_REGISTER);

	trace("Scheduling ComRegistrationTask: %p\n", t);
	JobCtrl::getInstance()->addTask(t);

	trace("Leaving TcComTask::setupTestRegister\n");
}

void
TcComTask::onTestRegister(TaskEvent &event)
{
	trace("Enter TcComTask::onTestRegister\n");

	ComRegistrationTask *t =
	    dynamic_cast<ComRegistrationTask*>(event.getTask());
	trace("ComRegistrationTask = %p\n", t);

	assertUnless(t->getAction() == ComRegistrationTask::ACTION_REGISTER,
	    "No registration-task!\n");

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Registration failed!\n"
	    " * ComTaskResult = %i\n"
	    " * ResultDetails = %i\n",
	    t->getComTaskResult(), t->getResultDetails());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	delete t;

	trace("Leaving TcComTask::onTestRegister\n");
	nextTest();
}

void
TcComTask::setupTestUnregister(void)
{
	trace("Enter TcComTask::setupTestUnregister\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_REGISTER,
	    wxTaskEventHandler(TcComTask::onTestUnregister), NULL, this);

	ComRegistrationTask *t = new ComRegistrationTask;
	t->setAction(ComRegistrationTask::ACTION_UNREGISTER);

	trace("Scheduling ComRegistrationTask: %p\n", t);
	JobCtrl::getInstance()->addTask(t);

	trace("Leaving TcComTask::setupTestUnregister\n");
}

void
TcComTask::onTestUnregister(TaskEvent &event)
{
	trace("Enter TcComTask::onTestUnregister\n");

	ComRegistrationTask *t =
	    dynamic_cast<ComRegistrationTask*>(event.getTask());
	trace("ComRegistrationTask = %p\n", t);

	assertUnless(t->getAction() == ComRegistrationTask::ACTION_UNREGISTER,
	    "No unregistration-task!\n");

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Registration failed!\n"
	    " * ComTaskResult = %i\n"
	    " * ResultDetails = %i\n",
	    t->getComTaskResult(), t->getResultDetails());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	delete t;

	trace("TcComTask::onTestUnregister\n");
	nextTest();
}

void
TcComTask::setupTestPolicyRequest(void)
{
	trace("Enter TcComTask::setupTestPolicyRequest\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_POLICY_REQUEST,
	    wxTaskEventHandler(TcComTask::onTestPolicyRequest), NULL, this);

	ComPolicyRequestTask *next = new ComPolicyRequestTask;
	next->setRequestParameter(1, getuid());

	trace("Scheduling ComPolicyRequestTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestPolicyRequest\n");
}

void
TcComTask::onTestPolicyRequest(TaskEvent &event)
{
	trace("Enter TcComTask::onTestPolicyRequest\n");

	ComPolicyRequestTask *t =
	    dynamic_cast<ComPolicyRequestTask*>(event.getTask());
	trace("ComPolicyRequestTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to receive a policy: %i\n", t->getComTaskResult());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	struct apn_ruleset *ruleset = t->getPolicyApn();
	wxString content = getPolicyAsString(ruleset);
	apn_free_ruleset(ruleset);
	trace("Apn extracted from task\n");

	wxString userPolicy = wxFileName::GetHomeDir() + wxT("/policy");
	wxString cmp = getFileContent(userPolicy);
	trace("Apn extracted from ~/policy\n");

	delete t;

	assertUnless(
	    content.Strip(wxString::both) == cmp.Strip(wxString::both),
	    "Unexpected policy received!\n%s\n",
	    (const char*)content.fn_str());

	trace("Leaving TcComTask::onTestPolicyRequest\n");
	nextTest();
}

void
TcComTask::setupTestPolicySend(void)
{
	trace("Enter TcComTask::setupTestPolicySend\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_POLICY_SEND,
	    wxTaskEventHandler(TcComTask::onTestPolicySend), NULL, this);

	wxString file = wxFileName::GetHomeDir() + wxT("/policy");

	ComPolicySendTask *next = new ComPolicySendTask;
	next->setPolicy(getPolicyFromFile(file), getuid(), 1);

	trace("Scheduling ComPolicySendTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestPolicySend\n");
}

void
TcComTask::onTestPolicySend(TaskEvent &event)
{
	trace("Enter TcComTask::onTestPolicySend\n");

	ComPolicySendTask *t =
	    dynamic_cast<ComPolicySendTask*>(event.getTask());
	trace("ComPolicySendTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to send a policy: %i\n", t->getComTaskResult());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	delete t;

	trace("Leaving TcComTask::onTestPolicySend\n");
	nextTest();
}

void
TcComTask::setupTestCsumAdd(void)
{
	trace("Enter TcComTask::setupTestCsumAdd\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_ADD,
	    wxTaskEventHandler(TcComTask::onTestCsumAdd), NULL, this);

	wxString file = wxFileName::GetHomeDir() + wxT("/policy");

	ComCsumAddTask *next = new ComCsumAddTask;
	next->setPath(file);

	trace("Scheduling ComCsumAddTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestCsumAdd\n");
}

void
TcComTask::onTestCsumAdd(TaskEvent &event)
{
	trace("Enter TcComTask::onTestCsumAdd\n");

	ComCsumAddTask *t = dynamic_cast<ComCsumAddTask*>(event.getTask());
	trace("ComCsumAddTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to add a checksum: %i\n", t->getComTaskResult());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");
	assertUnless(t->havePrivateKey() == false,
	    "A private key is assigned");

	delete t;

	trace("Leaving TcComTask::onTestCsumAdd\n");
	nextTest();
}

void
TcComTask::setupTestCsumAddSymlink(void)
{
	trace("Enter TcComTask::setupTestCsumAddSymlink\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_ADD,
	    wxTaskEventHandler(TcComTask::onTestCsumAddSymlink), NULL, this);

	wxString file = wxFileName::GetHomeDir() + wxT("/l_add_1");

	ComCsumAddTask *next = new ComCsumAddTask;
	next->setPath(file);

	trace("Scheduling ComCsumAddTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestCsumAddSymlink\n");
}

void
TcComTask::onTestCsumAddSymlink(TaskEvent &event)
{
	trace("Enter TcComTask::onTestCsumAddSymlink\n");

	ComCsumAddTask *t = dynamic_cast<ComCsumAddTask*>(event.getTask());
	trace("ComCsumAddTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to add a checksum: %i\n", t->getComTaskResult());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");
	assertUnless(t->havePrivateKey() == false,
	    "A private key is assigned");

	delete t;

	wxString path;
	long result;

	path = wxFileName::GetHomeDir() + wxT("/symlink_add_1");
	result = wxExecute(wxT("/t/sfssig get ") + path, wxEXEC_SYNC);
	assertUnless((result == 0),
	    "No checksum for the reference is registered!\n");

	path = wxFileName::GetHomeDir() + wxT("/l_add_1");
	result = wxExecute(wxT("/t/sfssig -l get ") + path, wxEXEC_SYNC);
	assertUnless((result != 0), "A checksum for the link is registered!\n");

	trace("Leaving TcComTask::onTestCsumAddSymlink\n");
	nextTest();
}

void
TcComTask::setupTestCsumAddSymlinkLink(void)
{
	trace("Enter TcComTask::setupTestCsumAddSymlinkLink\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_ADD,
	  wxTaskEventHandler(TcComTask::onTestCsumAddSymlinkLink), NULL, this);

	wxString file = wxFileName::GetHomeDir() + wxT("/l_add_2");

	ComCsumAddTask *next = new ComCsumAddTask;
	next->setPath(file);
	next->setCalcLink(true);

	trace("Scheduling ComCsumAddTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestCsumAddSymlinkLink\n");
}

void
TcComTask::onTestCsumAddSymlinkLink(TaskEvent &event)
{
	trace("Enter TcComTask::onTestCsumAddSymlinkLink\n");

	ComCsumAddTask *t = dynamic_cast<ComCsumAddTask*>(event.getTask());
	trace("ComCsumAddTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to add a checksum: %i\n", t->getComTaskResult());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");
	assertUnless(t->havePrivateKey() == false,
	    "A private key is assigned");

	delete t;

	wxString path;
	long result;

	path = wxFileName::GetHomeDir() + wxT("/symlink_add_2");
	result = wxExecute(wxT("/t/sfssig get ") + path, wxEXEC_SYNC);
	assertUnless((result != 0),
	    "No checksum for the reference is registered!\n");

	path = wxFileName::GetHomeDir() + wxT("/l_add_2");
	result = wxExecute(wxT("/t/sfssig -l get ") + path, wxEXEC_SYNC);
	assertUnless((result == 0), "A checksum for the link is registered!\n");

	trace("Leaving TcComTask::onTestCsumAddSymlinkLink\n");
	nextTest();
}

void
TcComTask::setupTestCsumGet(void)
{
	trace("Enter TcComTask::setupTestCsumGet\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_GET,
	    wxTaskEventHandler(TcComTask::onTestCsumGet), NULL, this);

	wxString file = wxFileName::GetHomeDir() + wxT("/policy");

	ComCsumGetTask *next = new ComCsumGetTask;
	next->setPath(file);

	trace("Scheduling ComCsumGetTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestCsumGet\n");
}

void
TcComTask::onTestCsumGet(TaskEvent &event)
{
	trace("TcComTask::onTestCsumGet\n");

	ComCsumGetTask *t = dynamic_cast<ComCsumGetTask*>(event.getTask());
	trace("ComCsumGetTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to get a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i (%s)\n",
	    t->getComTaskResult(), t->getResultDetails(),
	    strerror(t->getResultDetails()));

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");

	u_int8_t cs[ANOUBIS_CS_LEN];
	assertUnless(t->getCsum(cs, ANOUBIS_CS_LEN) == ANOUBIS_CS_LEN,
	    "Unexpected checksum received!");

	wxString csum = t->getCsumStr();
	trace("Received checksum from task\n");

	assertUnless(csum == policyCsum,
	    "Unexpected checksum received!\n"
	    "Is      : %s\n"
	    "Expected: %s\n",
	    (const char*)csum.fn_str(), (const char*)policyCsum.fn_str());

	delete t;
	trace("Leaving TcComTask::onTestCsumGet\n");
	nextTest();
}

void
TcComTask::setupTestCsumGetNoSuchFile(void)
{
	trace("Enter TcComTask::setupTestCsumGetNoSuchFile\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_GET,
	    wxTaskEventHandler(TcComTask::onTestCsumGetNoSuchFile),
	    NULL, this);

	wxString file = wxFileName::GetHomeDir() + wxT("/PpolicyY");

	ComCsumGetTask *next = new ComCsumGetTask;
	next->setPath(file);

	trace("Scheduling ComCsumGetTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestCsumGetNoSuchFile\n");
}

void
TcComTask::onTestCsumGetNoSuchFile(TaskEvent &event)
{
	trace("TcComTask::onTestCsumGetNoSuchFile\n");

	ComCsumGetTask *t = dynamic_cast<ComCsumGetTask*>(event.getTask());
	trace("ComCsumGetTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_REMOTE_ERROR,
	    "Failed to get a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i\n",
	    t->getComTaskResult(), t->getResultDetails());

	assertUnless(t->getResultDetails() == 2,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");

	u_int8_t cs[ANOUBIS_CS_LEN];
	assertUnless(t->getCsum(cs, ANOUBIS_CS_LEN) == 0,
	   "Task contains a checksum!\n");

	assertUnless(t->getCsumStr().IsEmpty(),
	   "Task contains a checksum-string!\n");

	delete t;
	trace("Leaving TcComTask::onTestCsumGetNoSuchFile\n");
	nextTest();
}

void
TcComTask::setupTestCsumGetSymlink(void)
{
	trace("Enter TcComTask::setupTestCsumGetSymlink\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_GET,
	    wxTaskEventHandler(TcComTask::onTestCsumGetSymlink),
	    NULL, this);

	wxString file = wxFileName::GetHomeDir() + wxT("/l_get_1");

	ComCsumGetTask *next = new ComCsumGetTask;
	next->setPath(file);

	trace("Scheduling ComCsumGetTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestCsumGetSymlink\n");
}

void
TcComTask::onTestCsumGetSymlink(TaskEvent &event)
{
	trace("TcComTask::onTestCsumGetSymlink\n");

	ComCsumGetTask *t = dynamic_cast<ComCsumGetTask*>(event.getTask());
	trace("ComCsumGetTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to get a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i\n",
	    t->getComTaskResult(), t->getResultDetails());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");

	//u_int8_t cs[ANOUBIS_CS_LEN];
	//assertUnless(t->getCsum(cs, ANOUBIS_CS_LEN) == 0,
	//   "Task contains a checksum!\n");

	assertUnless((t->getCsumStr() == symlink_get_1_csum),
	   "Checksum does not match\n"
	   "Expected: %ls\n"
	   "Is: %ls\n", symlink_get_1_csum.c_str(), t->getCsumStr().c_str());

	delete t;
	trace("Leaving TcComTask::onTestCsumGetSymlink\n");
	nextTest();
}

void
TcComTask::setupTestCsumGetSymlinkLink(void)
{
	trace("Enter TcComTask::setupTestCsumGetSymlinkLink\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_GET,
	    wxTaskEventHandler(TcComTask::onTestCsumGetSymlinkLink),
	    NULL, this);

	wxString file = wxFileName::GetHomeDir() + wxT("/l_get_2");

	ComCsumGetTask *next = new ComCsumGetTask;
	next->setPath(file);
	next->setCalcLink(true);

	trace("Scheduling ComCsumGetTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestCsumGetSymlinkLink\n");
}

void
TcComTask::onTestCsumGetSymlinkLink(TaskEvent &event)
{
	trace("TcComTask::onTestCsumGetSymlinkLink\n");

	ComCsumGetTask *t = dynamic_cast<ComCsumGetTask*>(event.getTask());
	trace("ComCsumGetTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to get a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i\n",
	    t->getComTaskResult(), t->getResultDetails());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");

	//u_int8_t cs[ANOUBIS_CS_LEN];
	//assertUnless(t->getCsum(cs, ANOUBIS_CS_LEN) == 0,
	//   "Task contains a checksum!\n");

	assertUnless((t->getCsumStr() == l2_csum),
	   "Checksum does not match\n"
	   "Expected: %ls\n"
	   "Is: %ls\n", l2_csum.c_str(), t->getCsumStr().c_str());

	delete t;
	trace("Leaving TcComTask::onTestCsumGetSymlinkLink\n");
	nextTest();
}

void
TcComTask::setupTestSfsListNotEmpty(void)
{
	trace("Enter TcComTask::setupTestSfsListNotEmpty\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(TcComTask::onTestSfsListNotEmpty), NULL, this);

	ComSfsListTask *next = new ComSfsListTask;
	next->setRequestParameter(getuid(), wxFileName::GetHomeDir());

	trace("Scheduling ComSfsListTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestSfsListNotEmpty\n");
}

void
TcComTask::onTestSfsListNotEmpty(TaskEvent &event)
{
	trace("TcComTask::onTestSfsListNotEmpty\n");

	ComSfsListTask *t = dynamic_cast<ComSfsListTask*>(event.getTask());
	trace("ComSfsListTask = %p\n", t);

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");

	wxArrayString result = t->getFileList();
	trace("sfs-list-size: %i\n", result.Count());

	/*
	 * XXX
	 * Will return ENOENT if result.Count() is 0.
	 * Known error in anoubisd (#924)
	 */
	/*if (t->getResultDetails() != 0) {
		trace("ResultDetails: %s (%i)\n",
		    strerror(t->getResultDetails()), t->getResultDetails());

		result_ = __LINE__;
		exit_ = true;
		return;
	}*/

	delete t;

	/* One entry expected */
	assertUnless(result.Count() == 1,
	    "Unexpected # of entries in sfs-list\n"
	    "Expected: 1\n"
	    "Is: %i\n", result.Count());

	wxString first = result[0];
	wxString second = wxT("policy");
	assertUnless(first == second,
	    "Unexpected content of sfs-list: %s\n",
	    (const char*)result[0].fn_str());

	trace("Leaving TcComTask::onTestSfsListNotEmpty\n");
	nextTest();
}

void
TcComTask::setupTestCsumDel(void)
{
	trace("Enter TcComTask::setupTestCsumDel\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_DEL,
	    wxTaskEventHandler(TcComTask::onTestCsumDel), NULL, this);

	wxString file = wxFileName::GetHomeDir() + wxT("/policy");

	ComCsumDelTask *next = new ComCsumDelTask;
	next->setPath(file);

	trace("Scheduling ComCsumDelTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestCsumDel\n");
}

void
TcComTask::onTestCsumDel(TaskEvent &event)
{
	trace("Enter TcComTask::onTestCsumDel\n");

	ComCsumDelTask *t = dynamic_cast<ComCsumDelTask*>(event.getTask());
	trace("ComCsumDelTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to remove a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i\n",
	    t->getComTaskResult(), t->getResultDetails());

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	delete t;

	trace("Leaving TcComTask::onTestCsumDel\n");
	nextTest();
}

void
TcComTask::setupTestCsumDelSymlink(void)
{
	trace("Enter TcComTask::setupTestCsumDelSymlink\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_DEL,
	    wxTaskEventHandler(TcComTask::onTestCsumDelSymlink), NULL, this);

	wxString file = wxFileName::GetHomeDir() + wxT("/l_del_1");

	ComCsumDelTask *next = new ComCsumDelTask;
	next->setPath(file);

	trace("Scheduling ComCsumDelTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestCsumDelSymlink\n");
}

void
TcComTask::onTestCsumDelSymlink(TaskEvent &event)
{
	trace("Enter TcComTask::onTestCsumDel\n");

	ComCsumDelTask *t = dynamic_cast<ComCsumDelTask*>(event.getTask());
	trace("ComCsumDelTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to remove a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i\n",
	    t->getComTaskResult(), t->getResultDetails());

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	delete t;

	wxString path;
	long result;

	path = wxFileName::GetHomeDir() + wxT("/symlink_del_1");
	result = wxExecute(wxT("/t/sfssig get ") + path, wxEXEC_SYNC);
	assertUnless((result != 0),
	    "A checksum for the reference is registered!\n");

	path = wxFileName::GetHomeDir() + wxT("/l_del_1");
	result = wxExecute(wxT("/t/sfssig -l get ") + path, wxEXEC_SYNC);
	assertUnless((result != 0), "A checksum for the link is registered!\n");

	trace("Leaving TcComTask::onTestCsumDel\n");
	nextTest();
}

void
TcComTask::setupTestCsumDelSymlinkLink(void)
{
	trace("Enter TcComTask::setupTestCsumDelSymlinkLink\n");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_DEL,
	  wxTaskEventHandler(TcComTask::onTestCsumDelSymlinkLink), NULL, this);

	wxString file = wxFileName::GetHomeDir() + wxT("/l_del_2");

	ComCsumDelTask *next = new ComCsumDelTask;
	next->setPath(file);
	next->setCalcLink(true);

	trace("Scheduling ComCsumDelTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestCsumDelSymlinkLink\n");
}

void
TcComTask::onTestCsumDelSymlinkLink(TaskEvent &event)
{
	trace("Enter TcComTask::onTestCsumDelSymlinkLink\n");

	ComCsumDelTask *t = dynamic_cast<ComCsumDelTask*>(event.getTask());
	trace("ComCsumDelTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to remove a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i\n",
	    t->getComTaskResult(), t->getResultDetails());

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	delete t;

	wxString path;
	long result;

	path = wxFileName::GetHomeDir() + wxT("/symlink_del_2");
	result = wxExecute(wxT("/t/sfssig get ") + path, wxEXEC_SYNC);
	assertUnless((result != 0),
	    "A checksum for the reference is registered!\n");

	path = wxFileName::GetHomeDir() + wxT("/l_del_2");
	result = wxExecute(wxT("/t/sfssig -l get ") + path, wxEXEC_SYNC);
	assertUnless((result != 0), "A checksum for the link is registered!\n");

	trace("Leaving TcComTask::onTestCsumDelSymlinkLink\n");
	nextTest();
}

void
TcComTask::setupTestSfsListEmpty(void)
{
	trace("Enter TcComTask::setupTestSfsListEmpty\n");
	wxString path = wxFileName::GetHomeDir() + wxT("/csums/empty");

	JobCtrl::getInstance()->Connect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(TcComTask::onTestSfsListEmpty), NULL, this);

	ComSfsListTask *next = new ComSfsListTask;
	next->setRequestParameter(getuid(), path);

	trace("Scheduling ComSfsListTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestSfsListEmpty\n");
}

void
TcComTask::onTestSfsListEmpty(TaskEvent &event)
{
	trace("TcComTask::onTestSfsListEmpty\n");

	ComSfsListTask *t = dynamic_cast<ComSfsListTask*>(event.getTask());
	trace("ComSfsListTask = %p\n", t);

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");

	wxArrayString result = t->getFileList();
	trace("sfs-list-size: %i\n", result.Count());

	/*
	 * XXX
	 * Will return ENOENT if result.Count() is 0.
	 * Known error in anoubisd (#924)
	 */
	/*if (t->getResultDetails() != 0) {
		trace("ResultDetails: %s (%i)\n",
		    strerror(t->getResultDetails()), t->getResultDetails());

		result_ = __LINE__;
		exit_ = true;
		return;
	}*/

	delete t;

	/* Empty list expected */
	assertUnless(result.Count() == 0,
	    "Unexpected # of entries in sfs-list\n"
	    "Expected: 0\n"
	    "Is: %i\n", result.Count());

	trace("Leaving TcComTask::onTestSfsListEmpty\n");
	nextTest();
}

void
TcComTask::setupTestSfsListRecursive(void)
{
	trace("Enter TcComTask::setupTestSfsListRecursive\n");
	wxString path = wxFileName::GetHomeDir() + wxT("/csums");

	JobCtrl::getInstance()->Connect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(TcComTask::onTestSfsListRecursive), NULL, this);

	ComSfsListTask *next = new ComSfsListTask;
	next->setRequestParameter(getuid(), path);
	next->setRecursive(true);

	trace("Scheduling ComSfsListTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestSfsListRecursive\n");
}

void
TcComTask::onTestSfsListRecursive(TaskEvent &event)
{
	trace("TcComTask::onTestSfsListRecursive\n");

	ComSfsListTask *t = dynamic_cast<ComSfsListTask*>(event.getTask());
	trace("ComSfsListTask = %p\n", t);

	assertUnless(t->haveKeyId() == false, "A key-id is assigned");

	wxArrayString result = t->getFileList();
	trace("sfs-list-size: %i\n", result.Count());

	delete t;

	int idx;

	idx = result.Index(wxT("1"));
	assertUnless((idx != wxNOT_FOUND),
	    "Entry \"1\" not found in sfs-list\n");
	result.RemoveAt(idx);

	idx = result.Index(wxT("2"));
	assertUnless((idx != wxNOT_FOUND),
	    "Entry \"2\" not found in sfs-list\n");
	result.RemoveAt(idx);

	idx = result.Index(wxT("3"));
	assertUnless((idx != wxNOT_FOUND),
	    "Entry \"3\" not found in sfs-list\n");
	result.RemoveAt(idx);

	idx = result.Index(wxT("sub1/"));
	assertUnless((idx != wxNOT_FOUND),
	    "Entry \"sub1/\" not found in sfs-list\n");
	result.RemoveAt(idx);

	idx = result.Index(wxT("sub1/1"));
	assertUnless((idx != wxNOT_FOUND),
	    "Entry \"sub1/1\" not found in sfs-list\n");
	result.RemoveAt(idx);

	idx = result.Index(wxT("sub1/2"));
	assertUnless((idx != wxNOT_FOUND),
	    "Entry \"sub1/2\" not found in sfs-list\n");
	result.RemoveAt(idx);

	idx = result.Index(wxT("sub2/"));
	assertUnless((idx != wxNOT_FOUND),
	    "Entry \"sub2/\" not found in sfs-list\n");
	result.RemoveAt(idx);

	idx = result.Index(wxT("sub2/1"));
	assertUnless((idx != wxNOT_FOUND),
	    "Entry \"sub2/1\" not found in sfs-list\n");
	result.RemoveAt(idx);

	idx = result.Index(wxT("sub2/2"));
	assertUnless((idx != wxNOT_FOUND),
	    "Entry \"sub2/2\" not found in sfs-list\n");
	result.RemoveAt(idx);

	idx = result.Index(wxT("sub2/3"));
	assertUnless((idx != wxNOT_FOUND),
	    "Entry \"sub2/3\" not found in sfs-list\n");
	result.RemoveAt(idx);

	for (unsigned int i = 0; i < result.Count(); i++) {
		trace("* (%i): %s\n", i, (const char *)result[i].fn_str());
	}

	assertUnless((result.Count() == 0),
	    "After removing all expected entries, an empty list is expected\n"
	    "Is: %i\n", result.Count());

	trace("Leaving TcComTask::onTestSfsListRecursive\n");
	nextTest();
}

void
TcComTask::setupTestSigAdd(void)
{
	trace("Enter TcComTask::setupTestSigAdd\n");
	wxString certFile = wxFileName::GetHomeDir() + wxT("/pubkey");
	wxString keyFile = wxFileName::GetHomeDir() + wxT("/privkey");
	wxString file = wxFileName::GetHomeDir() + wxT("/policy");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_ADD,
	    wxTaskEventHandler(TcComTask::onTestSigAdd), NULL, this);

	KeyCtrl *keyCtrl = KeyCtrl::getInstance();

	LocalCertificate &cert = keyCtrl->getLocalCertificate();
	cert.setFile(certFile);

	assertUnless(cert.load(),
	    "Failed to load certificate from %s\n",
	    (const char *)certFile.fn_str());

	trace(" * KeyId: %s\n", (const char *)cert.getKeyId().fn_str());
	trace(" * Fingerprint: %s\n",
	    (const char *)cert.getFingerprint().fn_str());
	trace(" * DN: %s\n",
	    (const char *)cert.getDistinguishedName().fn_str());

	PrivKey &privKey = keyCtrl->getPrivateKey();
	privKey.setFile(keyFile);

	assertUnless(privKey.load(wxT("1234")),
	    "Failed to load private key from %s\n",
	    (const char *)keyFile.fn_str());

	struct anoubis_sig *raw_cert = cert.getCertificate();

	ComCsumAddTask *next = new ComCsumAddTask;
	next->setPath(file);
	assertUnless(next->setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");
	next->setPrivateKey(privKey.getKey());

	trace("Scheduling ComCsumAddTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestSigAdd\n");
}

void
TcComTask::onTestSigAdd(TaskEvent &event)
{
	trace("Enter TcComTask::onTestSigAdd\n");

	ComCsumAddTask *t = dynamic_cast<ComCsumAddTask*>(event.getTask());
	trace("ComCsumAddTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to add a signed checksum: %i\n", t->getComTaskResult());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	assertUnless(t->haveKeyId(), "No key-id is assigned");

	delete t;

	trace("Leaving TcComTask::onTestSigAdd\n");
	nextTest();
}

void
TcComTask::setupTestSigGet(void)
{
	trace("Enter TcComTask::setupTestSigGet\n");
	wxString certFile = wxFileName::GetHomeDir() + wxT("/pubkey");
	wxString file = wxFileName::GetHomeDir() + wxT("/policy");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_GET,
	    wxTaskEventHandler(TcComTask::onTestSigGet), NULL, this);

	KeyCtrl *keyCtrl = KeyCtrl::getInstance();

	LocalCertificate &cert = keyCtrl->getLocalCertificate();
	cert.setFile(certFile);

	assertUnless(cert.load(),
	    "Failed to load certificate from %s\n",
	    (const char *)certFile.fn_str());

	trace(" * KeyId: %s\n", (const char *)cert.getKeyId().fn_str());
	trace(" * Fingerprint: %s\n",
	    (const char *)cert.getFingerprint().fn_str());
	trace(" * DN: %s\n",
	    (const char *)cert.getDistinguishedName().fn_str());

	struct anoubis_sig *raw_cert = cert.getCertificate();

	ComCsumGetTask *next = new ComCsumGetTask;
	next->setPath(file);
	assertUnless(next->setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");

	trace("Scheduling ComCsumGetTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestSigGet\n");
}

void
TcComTask::onTestSigGet(TaskEvent &event)
{
	trace("TcComTask::onTestSigGet\n");

	ComCsumGetTask *t = dynamic_cast<ComCsumGetTask*>(event.getTask());
	trace("ComCsumGetTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to get a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i\n",
	    t->getComTaskResult(), t->getResultDetails());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	assertUnless(t->haveKeyId(), "No key-id is assigned");

	u_int8_t cs[ANOUBIS_CS_LEN];
	assertUnless(t->getCsum(cs, ANOUBIS_CS_LEN) == ANOUBIS_CS_LEN,
	    "Unexpected checksum received!");

	wxString csum = t->getCsumStr();
	trace("Received checksum from task\n");

	assertUnless(csum == policyCsum,
	    "Unexpected checksum received!\n"
	    "Is      : %s\n"
	    "Expected: %s\n",
	    (const char*)csum.fn_str(), (const char*)policyCsum.fn_str());

	delete t;
	trace("Leaving TcComTask::onTestSigGet\n");
	nextTest();
}

void
TcComTask::setupTestSigListNotEmpty(void)
{
	trace("Enter TcComTask::setupTestSigListNotEmpty\n");
	wxString certFile = wxFileName::GetHomeDir() + wxT("/pubkey");

	JobCtrl::getInstance()->Connect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(TcComTask::onTestSigListNotEmpty), NULL, this);

	KeyCtrl *keyCtrl = KeyCtrl::getInstance();

	LocalCertificate &cert = keyCtrl->getLocalCertificate();
	cert.setFile(certFile);

	assertUnless(cert.load(),
	    "Failed to load certificate from %s\n",
	    (const char *)certFile.fn_str());

	trace(" * KeyId: %s\n", (const char *)cert.getKeyId().fn_str());
	trace(" * Fingerprint: %s\n",
	    (const char *)cert.getFingerprint().fn_str());
	trace(" * DN: %s\n",
	    (const char *)cert.getDistinguishedName().fn_str());

	struct anoubis_sig *raw_cert = cert.getCertificate();

	ComSfsListTask *next = new ComSfsListTask;
	next->setRequestParameter(getuid(), wxFileName::GetHomeDir());
	assertUnless(next->setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");

	trace("Scheduling ComSfsListTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestSigListNotEmpty\n");
}

void
TcComTask::onTestSigListNotEmpty(TaskEvent &event)
{
	trace("TcComTask::onTestSigListNotEmpty\n");

	ComSfsListTask *t = dynamic_cast<ComSfsListTask*>(event.getTask());
	trace("ComSfsListTask = %p\n", t);

	assertUnless(t->haveKeyId(), "No key-id is assigned");

	wxArrayString result = t->getFileList();
	trace("sfs-list-size: %i\n", result.Count());

	/*
	 * XXX
	 * Will return ENOENT if result.Count() is 0.
	 * Known error in anoubisd (#924)
	 */
	/*if (t->getResultDetails() != 0) {
		trace("ResultDetails: %s (%i)\n",
		    strerror(t->getResultDetails()), t->getResultDetails());

		result_ = __LINE__;
		exit_ = true;
		return;
	}*/

	delete t;

	/* One entry expected */
	assertUnless(result.Count() == 1,
	    "Unexpected # of entries in sfs-list\n"
	    "Expected: 1\n"
	    "Is: %i\n", result.Count());

	wxString first = result[0];
	wxString second = wxT("policy");
	assertUnless(first == second,
	    "Unexpected content of sfs-list: %s\n",
	    (const char*)result[0].fn_str());

	trace("Leaving TcComTask::onTestSigListNotEmpty\n");
	nextTest();
}

void
TcComTask::setupTestSigDel(void)
{
	trace("Enter TcComTask::setupTestSigDel\n");
	wxString certFile = wxFileName::GetHomeDir() + wxT("/pubkey");
	wxString file = wxFileName::GetHomeDir() + wxT("/policy");

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_DEL,
	    wxTaskEventHandler(TcComTask::onTestSigDel), NULL, this);

	KeyCtrl *keyCtrl = KeyCtrl::getInstance();

	LocalCertificate &cert = keyCtrl->getLocalCertificate();
	cert.setFile(certFile);

	assertUnless(cert.load(),
	    "Failed to load certificate from %s\n",
	    (const char *)certFile.fn_str());

	trace(" * KeyId: %s\n", (const char *)cert.getKeyId().fn_str());
	trace(" * Fingerprint: %s\n",
	    (const char *)cert.getFingerprint().fn_str());
	trace(" * DN: %s\n",
	    (const char *)cert.getDistinguishedName().fn_str());

	struct anoubis_sig *raw_cert = cert.getCertificate();

	ComCsumDelTask *next = new ComCsumDelTask;
	next->setPath(file);
	assertUnless(next->setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");

	trace("Scheduling ComCsumDelTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestSigDel\n");
}

void
TcComTask::onTestSigDel(TaskEvent &event)
{
	trace("Enter TcComTask::onTestSigDel\n");

	ComCsumDelTask *t = dynamic_cast<ComCsumDelTask*>(event.getTask());
	trace("ComCsumDelTask = %p\n", t);

	assertUnless(t->getComTaskResult() == ComTask::RESULT_SUCCESS,
	    "Failed to remove a checksum!\n"
	    "ComTaskResult = %i\n"
	    "ResultDetails = %i\n",
	    t->getComTaskResult(), t->getResultDetails());

	assertUnless(t->getResultDetails() == 0,
	    "ResultDetails: %s (%i)\n",
	    strerror(t->getResultDetails()), t->getResultDetails());

	assertUnless(t->haveKeyId(), "No key-id is assigned");

	delete t;

	trace("Leaving TcComTask::onTestSigDel\n");
	nextTest();
}

void
TcComTask::setupTestSigListEmpty(void)
{
	trace("Enter TcComTask::setupTestSigListEmpty\n");
	wxString path = wxFileName::GetHomeDir() + wxT("/csums/empty");
	wxString certFile = wxFileName::GetHomeDir() + wxT("/pubkey");

	JobCtrl::getInstance()->Connect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(TcComTask::onTestSigListEmpty), NULL, this);

	KeyCtrl *keyCtrl = KeyCtrl::getInstance();

	LocalCertificate &cert = keyCtrl->getLocalCertificate();
	cert.setFile(certFile);

	assertUnless(cert.load(),
	    "Failed to load certificate from %s\n",
	    (const char *)certFile.fn_str());

	trace(" * KeyId: %s\n", (const char *)cert.getKeyId().fn_str());
	trace(" * Fingerprint: %s\n",
	    (const char *)cert.getFingerprint().fn_str());
	trace(" * DN: %s\n",
	    (const char *)cert.getDistinguishedName().fn_str());

	struct anoubis_sig *raw_cert = cert.getCertificate();

	ComSfsListTask *next = new ComSfsListTask;
	next->setRequestParameter(getuid(), path);
	assertUnless(next->setKeyId(raw_cert->keyid, raw_cert->idlen),
	    "Failed to setup task with key-id.");

	trace("Scheduling ComSfsListTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::setupTestSigListEmpty\n");
}

void
TcComTask::onTestSigListEmpty(TaskEvent &event)
{
	trace("TcComTask::onTestSigListEmpty\n");

	ComSfsListTask *t = dynamic_cast<ComSfsListTask*>(event.getTask());
	trace("ComSfsListTask = %p\n", t);

	assertUnless(t->haveKeyId(), "No key-id is assigned");

	wxArrayString result = t->getFileList();
	trace("sfs-list-size: %i\n", result.Count());

	/*
	 * XXX
	 * Will return ENOENT if result.Count() is 0.
	 * Known error in anoubisd (#924)
	 */
	/*if (t->getResultDetails() != 0) {
		trace("ResultDetails: %s (%i)\n",
		    strerror(t->getResultDetails()), t->getResultDetails());

		result_ = __LINE__;
		exit_ = true;
		return;
	}*/

	delete t;

	/* Empty list expected */
	assertUnless(result.Count() == 0,
	    "Unexpected # of entries in sfs-list\n"
	    "Expected: 0\n"
	    "Is: %i\n", result.Count());

	trace("Leaving TcComTask::onTestSigListEmpty\n");
	nextTest();
}

wxString
TcComTask::getFileContent(const wxString &fileName)
{
	wxString	content;
	char		buf[256];
	FILE		*f;

	f = fopen(fileName.fn_str(), "r");
	if (f == 0) {
		fprintf(stderr, "Failed to open %s (%s)!\n",
		    (const char*)fileName.fn_str(), strerror(errno));
		return (content);
	}

	while (!feof(f)) {
		int nread = fread(buf, 1, sizeof(buf), f);

		if (nread >= 0) {
			wxString append(buf, wxConvFile, nread);
			content += append;
		}
		else {
			fprintf(stderr, "Failed to read from temp. file!\n");
			return content;
		}
	}

	fclose(f);

	return (content);
}

wxString
TcComTask::getPolicyAsString(struct apn_ruleset *rs)
{
	wxString tmpFile, result;
	FILE *f;

	tmpFile = wxFileName::CreateTempFileName(wxT(""));

	f = fopen(tmpFile.fn_str(), "w");
	apn_print_ruleset(rs, 0, f);
	fclose(f);

	result = getFileContent(tmpFile);

	wxRemoveFile(tmpFile);

	return (result);
}

struct apn_ruleset *
TcComTask::getPolicyFromFile(const wxString &file)
{
	struct apn_ruleset *rs;

	apn_parse(file.fn_str(), &rs, 0);

	return (rs);
}

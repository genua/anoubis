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

#include <ComCsumAddTask.h>
#include <ComCsumDelTask.h>
#include <ComCsumGetTask.h>
#include <ComPolicyRequestTask.h>
#include <ComPolicySendTask.h>
#include <ComRegistrationTask.h>
#include <ComSfsListTask.h>
#include <JobCtrl.h>

#include "TcComTask.h"

#define trace(format, args...) fprintf (stderr, format , ##args)

const wxString policyCsum =
    wxT("09c2012a32bb5b776c8e0adc186df5b45279dd87dda249e877ec876b33d190f9");

TcComTask::TcComTask()
{
	this->policyRequestCounter_ = 0;
	this->csumListCounter_ = 0;
	this->exit_ = false;
	this->result_ = 0;

	JobCtrl::getInstance()->Connect(anTASKEVT_REGISTER,
	    wxTaskEventHandler(TcComTask::OnRegister), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_POLICY_REQUEST,
	    wxTaskEventHandler(TcComTask::OnPolicyReceived), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_POLICY_SEND,
	    wxTaskEventHandler(TcComTask::OnPolicySend), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_ADD,
	    wxTaskEventHandler(TcComTask::OnCsumAdd), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_GET,
	    wxTaskEventHandler(TcComTask::OnCsumGet), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_DEL,
	    wxTaskEventHandler(TcComTask::OnCsumDel), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(TcComTask::OnSfsList), NULL, this);

	trace("TcComTask::TcComTask finished\n");
}

TcComTask::~TcComTask()
{
	trace("TcComTask::~TcComTask finished\n");
}

void
TcComTask::startTest()
{
	trace("Enter TcComTask::startTest\n");

	ComRegistrationTask *t = new ComRegistrationTask;
	t->setAction(ComRegistrationTask::ACTION_REGISTER);

	trace("Scheduling ComRegistrationTask: %p\n", t);
	JobCtrl::getInstance()->addTask(t);
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
TcComTask::OnRegister(TaskEvent &event)
{
	trace("Enter TcComTask::OnRegister\n");

	ComRegistrationTask *t =
	    dynamic_cast<ComRegistrationTask*>(event.getTask());
	trace("ComRegistrationTask = %p\n", t);

	if (t->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		trace("ComTaskResult == %i\n", t->getComTaskResult());

		switch (t->getAction()) {
		case ComRegistrationTask::ACTION_REGISTER:
			trace("Registration failed: %i\n",
			    t->getComTaskResult());
			break;
		case ComRegistrationTask::ACTION_UNREGISTER:
			trace("Unregistration failed: %i\n",
			    t->getComTaskResult());
			break;
		}

		result_ = __LINE__;
		exit_ = true;
		return;
	}

	if (t->getResultDetails() != 0) {
		trace("ResultDetails: %s (%i)\n",
		    strerror(t->getResultDetails()), t->getResultDetails());

		result_ = __LINE__;
		exit_ = true;
		return;
	}

	exit_ = (t->getAction() != ComRegistrationTask::ACTION_REGISTER);
	delete t;

	trace("exit_ = %i\n", exit_);

	if (!exit_) {
		/*
		 * Start with ComPolicyRequestTask
		 */
		ComPolicyRequestTask *next = new ComPolicyRequestTask;
		next->setRequestParameter(1, getuid());

		trace("Scheduling ComPolicyRequestTask: %p\n", next);
		JobCtrl::getInstance()->addTask(next);
	}

	trace("Leaving TcComTask::OnRegister\n");
}

void
TcComTask::OnPolicyReceived(TaskEvent &event)
{
	trace("Enter TcComTask::OnPolicyReceived\n");

	policyRequestCounter_++;
	trace("policyRequestCounter_ = %i\n", policyRequestCounter_);

	ComPolicyRequestTask *t =
	    dynamic_cast<ComPolicyRequestTask*>(event.getTask());
	trace("ComPolicyRequestTask = %p\n", t);

	if (t->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		trace("Failed to receive a policy: %i\n",
		    t->getComTaskResult());
		result_ = __LINE__;
		exit_ = true;

		delete (t);
		return;
	}

	if (t->getResultDetails() != 0) {
		trace("ResultDetails: %s (%i)\n",
		    strerror(t->getResultDetails()), t->getResultDetails());

		result_ = __LINE__;
		exit_ = true;
		return;
	}

	struct apn_ruleset *ruleset = t->getPolicyApn();
	wxString content = getPolicyAsString(ruleset);
	apn_free_ruleset(ruleset);
	trace("Apn extracted from task\n");

	wxString userPolicy = wxFileName::GetHomeDir() + wxT("/policy");
	wxString cmp = getFileContent(userPolicy);
	trace("Apn extracted from ~/policy\n");

	delete t;

	if (content.Strip(wxString::both) != cmp.Strip(wxString::both)) {
		trace("Unexpected policy received!\n%s\n",
			(const char*)content.fn_str());
		result_ = __LINE__;
		exit_ = true;
		return;
	}

	if (policyRequestCounter_ == 1) {
		/*
		 * Continue with ComPolicySendTask
		 */
		wxString file = wxFileName::GetHomeDir() + wxT("/policy");

		ComPolicySendTask *next = new ComPolicySendTask;
		next->setPolicy(getPolicyFromFile(file), getuid(), 1);

		trace("Scheduling ComPolicySendTask: %p\n", next);
		JobCtrl::getInstance()->addTask(next);
	}
	else {
		/*
		 * Continue with ComCsumAddTask
		 */
		wxString file = wxFileName::GetHomeDir() + wxT("/policy");

		ComCsumAddTask *next = new ComCsumAddTask;
		next->setFile(file);

		trace("Scheduling ComCsumAddTask: %p\n", next);
		JobCtrl::getInstance()->addTask(next);
	}

	trace("Leaving TcComTask::OnPolicyReceived\n");
}

void
TcComTask::OnPolicySend(TaskEvent &event)
{
	trace("Enter TcComTask::OnPolicySend\n");

	ComPolicySendTask *t =
	    dynamic_cast<ComPolicySendTask*>(event.getTask());
	trace("ComPolicySendTask = %p\n", t);

	if (t->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		trace("Failed to send a policy: %i\n", t->getComTaskResult());
		result_ = __LINE__;
		exit_ = true;

		delete (t);
		return;
	}

	if (t->getResultDetails() != 0) {
		trace("ResultDetails: %s (%i)\n",
		    strerror(t->getResultDetails()), t->getResultDetails());

		result_ = __LINE__;
		exit_ = true;
		return;
	}

	delete t;

	/*
	 * Receive the policy again
	 */
	ComPolicyRequestTask *next = new ComPolicyRequestTask;
	next->setRequestParameter(1, getuid());

	trace("Scheduling ComPolicyRequestTask; %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::OnPolicySend\n");
}

void
TcComTask::OnCsumAdd(TaskEvent &event)
{
	trace("Enter TcComTask::OnCsumAdd\n");

	ComCsumAddTask *t = dynamic_cast<ComCsumAddTask*>(event.getTask());
	trace("ComCsumAddTask = %p\n", t);

	if (t->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		trace("Failed to add a checksum: %i\n", t->getComTaskResult());
		result_ = __LINE__;
		exit_ = true;

		delete (t);
		return;
	}

	if (t->getResultDetails() != 0) {
		trace("ResultDetails: %s (%i)\n",
		    strerror(t->getResultDetails()), t->getResultDetails());

		result_ = __LINE__;
		exit_ = true;
		return;
	}

	delete t;

	/*
	 * Receive the checksum again
	 */
	wxString file = wxFileName::GetHomeDir() + wxT("/policy");

	ComCsumGetTask *next = new ComCsumGetTask;
	next->setFile(file);

	trace("Scheduling ComCsumGetTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::OnCsumAdd\n");
}

void
TcComTask::OnCsumGet(TaskEvent &event)
{
	trace("TcComTask::OnCsumGet\n");

	ComCsumGetTask *t = dynamic_cast<ComCsumGetTask*>(event.getTask());
	trace("ComCsumGetTask = %p\n", t);

	if (t->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		trace("Failed to get a checksum: %i\n", t->getComTaskResult());
		result_ = __LINE__;
		exit_ = true;

		delete (t);
		return;
	}

	if (t->getResultDetails() != 0) {
		trace("ResultDetails: %s (%i)\n",
		    strerror(t->getResultDetails()), t->getResultDetails());

		result_ = __LINE__;
		exit_ = true;
		return;
	}

	wxString csum = t->getCsumStr();
	trace("Received checksum from task\n");
	delete t;

	if (csum != policyCsum) {
		trace("Unexpected checksum received!\n");
		trace("Is      : %s\n", (const char*)csum.fn_str());
		trace("Expected: %s\n", (const char*)policyCsum.fn_str());

		result_ = __LINE__;
		exit_ = true;
		return;
	}

	/*
	 * Receive the sfs-list
	 */
	ComSfsListTask *next = new ComSfsListTask;
	next->setRequestParameter(getuid(), wxFileName::GetHomeDir());

	trace("Scheduling ComSfsListTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::OnCsumGet\n");
}

void
TcComTask::OnCsumDel(TaskEvent &event)
{
	trace("Enter TcComTask::OnCsumDel\n");

	ComCsumDelTask *t = dynamic_cast<ComCsumDelTask*>(event.getTask());
	trace("ComCsumDelTask = %p\n", t);

	ComTask::ComTaskResult result = t->getComTaskResult();
	trace("ComTaskResult = %i\n", result);

	delete t;

	if (result != ComTask::RESULT_SUCCESS) {
		trace("Failed to remove a checksum!\n");
		result_ = __LINE__;
		exit_ = true;
		return;
	}

	if (t->getResultDetails() != 0) {
		trace("ResultDetails: %s (%i)\n",
		    strerror(t->getResultDetails()), t->getResultDetails());

		result_ = __LINE__;
		exit_ = true;
		return;
	}

	/*
	 * Receive the sfs-list
	 */
	ComSfsListTask *next = new ComSfsListTask;
	next->setRequestParameter(getuid(), wxFileName::GetHomeDir());

	trace("Scheduling ComSfsListTask: %p\n", next);
	JobCtrl::getInstance()->addTask(next);

	trace("Leaving TcComTask::OnCsumDel\n");
}

void
TcComTask::OnSfsList(TaskEvent &event)
{
	trace("TcComTask::OnSfsList\n");

	ComSfsListTask *t = dynamic_cast<ComSfsListTask*>(event.getTask());
	trace("ComSfsListTask = %p\n", t);

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

	csumListCounter_++;
	trace("csumListCounter_ = %i\n", csumListCounter_);

	if (csumListCounter_ == 1) {
		/* One entry expected */
		if (result.Count() != 1) {
			trace("Unexpected # of entries in sfs-list\n");
			trace("Expected: 1\n");
			trace("Is: %i\n", result.Count());
			result_ = __LINE__;
			exit_ = true;
			return;
		}

		if (result[0] != wxT("policy")) {
			trace("Unexpected content of sfs-list: %s\n",
			    (const char*)result[0].fn_str());
			result_ = __LINE__;
			exit_ = true;
			return;
		}

		/* Remove the checksum */
		wxString file = wxFileName::GetHomeDir() + wxT("/policy");

		ComCsumDelTask *next = new ComCsumDelTask;
		next->setFile(file);

		trace("Scheduling ComCsumDelTask: %p\n", next);
		JobCtrl::getInstance()->addTask(next);
	} else {
		/* Empty list expected */
		if (result.Count() != 0) {
			trace("Unexpected # of entries in sfs-list\n");
			trace("Expected: 0\n");
			trace("Is: %i\n", result.Count());
			result_ = __LINE__;
			exit_ = true;
			return;
		}

		/*
		 * Finally send UnregistrationTask
		 */
		ComRegistrationTask *next = new ComRegistrationTask;
		next->setAction(ComRegistrationTask::ACTION_UNREGISTER);

		trace("Scheduling ComRegistrationTask: %p\n", next);
		JobCtrl::getInstance()->addTask(next);
	}

	trace("Leaving TcComTask::OnSfsList\n");
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

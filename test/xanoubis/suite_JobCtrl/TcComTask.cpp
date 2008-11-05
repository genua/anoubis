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
#include <ComCsumGetTask.h>
#include <ComPolicyRequestTask.h>
#include <ComPolicySendTask.h>
#include <ComRegistrationTask.h>
#include <ComSfsListTask.h>
#include <JobCtrl.h>

#include "TcComTask.h"

const wxString policyCsum =
    wxT("09c2012a32bb5b776c8e0adc186df5b45279dd87dda249e877ec876b33d190f9");

TcComTask::TcComTask()
{
	this->policyRequestCounter_ = 0;
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
	JobCtrl::getInstance()->Connect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(TcComTask::OnSfsList), NULL, this);
}

TcComTask::~TcComTask()
{
}

void
TcComTask::startTest()
{
	ComRegistrationTask *t = new ComRegistrationTask;
	t->setAction(ComRegistrationTask::ACTION_REGISTER);

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
	ComRegistrationTask *t =
	    dynamic_cast<ComRegistrationTask*>(event.getTask());

	if (t->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		switch (t->getAction()) {
		case ComRegistrationTask::ACTION_REGISTER:
			fprintf(stderr, "Registration failed: %i\n",
			    t->getComTaskResult());
			break;
		case ComRegistrationTask::ACTION_UNREGISTER:
			fprintf(stderr, "Unregistration failed: %i\n",
			    t->getComTaskResult());
			break;
		}

		result_ = 4711;
		exit_ = true;
	}

	exit_ = (t->getAction() != ComRegistrationTask::ACTION_REGISTER);
	delete t;

	if (!exit_) {
		/*
		 * Start with ComPolicyRequestTask
		 */
		ComPolicyRequestTask *next = new ComPolicyRequestTask;
		next->setRequestParameter(1, getuid());

		JobCtrl::getInstance()->addTask(next);
	}
}

void
TcComTask::OnPolicyReceived(TaskEvent &event)
{
	policyRequestCounter_++;

	ComPolicyRequestTask *t =
	    dynamic_cast<ComPolicyRequestTask*>(event.getTask());

	if (t->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		fprintf(stderr, "Failed to receive a policy: %i\n",
		    t->getComTaskResult());
		result_ = 4711;
		exit_ = true;

		delete (t);
		return;
	}

	struct apn_ruleset *ruleset = t->getPolicyApn();
	wxString content = getPolicyAsString(ruleset);
	apn_free_ruleset(ruleset);

	wxString userPolicy = wxFileName::GetHomeDir() + wxT("/policy");
	wxString cmp = getFileContent(userPolicy);

	delete t;

	if (content.Strip(wxString::both) != cmp.Strip(wxString::both)) {
		fprintf(stderr, "Unexpected policy received!\n%s\n",
			(const char*)content.fn_str());
		result_ = 4711;
		exit_ = true;
	}

	if (policyRequestCounter_ == 1) {
		/*
		 * Continue with ComPolicySendTask
		 */
		wxString file = wxFileName::GetHomeDir() + wxT("/policy");

		ComPolicySendTask *next = new ComPolicySendTask;
		next->setPolicy(getPolicyFromFile(file), getuid(), 1);

		JobCtrl::getInstance()->addTask(next);
	}
	else {
		/*
		 * Continue with ComCsumAddTask
		 */
		wxString file = wxFileName::GetHomeDir() + wxT("/policy");

		ComCsumAddTask *next = new ComCsumAddTask;
		next->setFile(file);

		JobCtrl::getInstance()->addTask(next);
	}
}

void
TcComTask::OnPolicySend(TaskEvent &event)
{
	ComPolicySendTask *t =
	    dynamic_cast<ComPolicySendTask*>(event.getTask());

	if (t->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		fprintf(stderr, "Failed to send a policy: %i\n",
		    t->getComTaskResult());
		result_ = 4711;
		exit_ = true;

		delete (t);
		return;
	}

	delete t;

	/*
	 * Receive the policy again
	 */
	ComPolicyRequestTask *next = new ComPolicyRequestTask;
	next->setRequestParameter(1, getuid());

	JobCtrl::getInstance()->addTask(next);
}

void
TcComTask::OnCsumAdd(TaskEvent &event)
{
	ComCsumAddTask *t = dynamic_cast<ComCsumAddTask*>(event.getTask());

	if (t->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		fprintf(stderr, "Failed to add a checksum: %i\n",
		    t->getComTaskResult());
		result_ = 4711;
		exit_ = true;

		delete (t);
		return;
	}

	delete t;

	/*
	 * Receive the checksum again
	 */
	wxString file = wxFileName::GetHomeDir() + wxT("/policy");

	ComCsumGetTask *next = new ComCsumGetTask;
	next->setFile(file);

	JobCtrl::getInstance()->addTask(next);
}

void
TcComTask::OnCsumGet(TaskEvent &event)
{
	ComCsumGetTask *t = dynamic_cast<ComCsumGetTask*>(event.getTask());

	if (t->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		fprintf(stderr, "Failed to get a checksum: %i\n",
		    t->getComTaskResult());
		result_ = 4711;
		exit_ = true;

		delete (t);
		return;
	}

	wxString csum = t->getCsumStr();
	delete t;

	if (csum != policyCsum) {
		fprintf(stderr, "Unexpected checksum received!\n");
		fprintf(stderr, "Is      : %s\n", (const char*)csum.fn_str());
		fprintf(stderr, "Expected: %s\n",
		    (const char*)policyCsum.fn_str());

		result_ = 4711;
		exit_ = true;
		return;
	}

	/*
	 * Receive the sfs-list
	 */
	ComSfsListTask *next = new ComSfsListTask;
	next->setRequestParameter(getuid(), wxFileName::GetHomeDir());
	JobCtrl::getInstance()->addTask(next);
}

void
TcComTask::OnSfsList(TaskEvent &event)
{
	ComSfsListTask *t = dynamic_cast<ComSfsListTask*>(event.getTask());
	wxArrayString result = t->getFileList();
	delete t;

	if (result.Count() != 1) {
		fprintf(stderr, "Unexpected # of entries in sfs-list: %i\n",
		    result.Count());
		result_ = 4711;
		exit_ = true;
		return;
	}

	if (result[0] != wxT("policy")) {
		fprintf(stderr, "Unexpected content of sfs-list: %s\n",
		    (const char*)result[0].fn_str());
		result_ = 4711;
		exit_ = true;
		return;
	}

	/*
	 * Finally send UnregistrationTask
	 */
	ComRegistrationTask *next = new ComRegistrationTask;
	next->setAction(ComRegistrationTask::ACTION_UNREGISTER);

	JobCtrl::getInstance()->addTask(next);
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

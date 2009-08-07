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

#include <JobCtrl.h>

#include <wx/cmdline.h>

#include "JobCtrlApp.h"
#include "TcComTask.h"
#include "TcCsumCalcTask.h"
#include "TestHandler.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(TestHandlerList);

IMPLEMENT_APP_NO_MAIN(JobCtrlApp);

bool
JobCtrlApp::OnInit()
{
	if (!wxAppConsole::OnInit())
		return (false);
	wxPendingEventsLocker = new wxCriticalSection;

	JobCtrl *jobCtrl = JobCtrl::getInstance();

	if (testcase_ == wxT("tc_jobctrl_nodaemon")) {
		/* Setup an unknown socket-path -> connection will fail */
		jobCtrl->setSocketPath(wxT("blablubb"));
	}

	if (!jobCtrl->start()) {
		fprintf(stderr, "Failed to start JobCtrl\n");
		return (false);
	}

	handlerList_.push_back(new TcCsumCalcTask);
	handlerList_.push_back(new TcComTask);

	return (true);
}

int
JobCtrlApp::OnRun()
{
	TestHandlerList::const_iterator it = handlerList_.begin();
	for (; it != handlerList_.end(); ++it) {
		TestHandler *h = (*it);
		h->startTest();
	}

	while (!canExit()) {
		ProcessPendingEvents();
	}

	int result = getResult();
	/*
	 * Within the nodaemon testcase the connection test failed.
	 * The result is the line number of the check at onTestConnect().
	 * On modifications of TcComTask.cpp, this value must be adjusted.
	 */
	if (testcase_ == wxT("tc_jobctrl_nodaemon") && result == 245) {
		/*
		 * Expected result for nodaemon-testcase because establishing
		 * of the connection failed.
		 */
		return (0);
	} else
		return (result);
}

int
JobCtrlApp::OnExit()
{
	while (!handlerList_.empty()) {
		TestHandler *h = handlerList_.front();
		handlerList_.pop_front();

		delete h;
	}

	JobCtrl *jobCtrl = JobCtrl::getInstance();

	jobCtrl->disconnect();
	jobCtrl->stop();
	delete jobCtrl;

	delete wxPendingEventsLocker;
	wxPendingEventsLocker = 0;

	return (0);
}

void
JobCtrlApp::OnInitCmdLine(wxCmdLineParser &parser)
{
	static const wxCmdLineEntryDesc cmdLineDescription[] = {
		{
			wxCMD_LINE_OPTION,
			wxT("t"),
			wxT("tc"),
			_("Name of testcase"),
			wxCMD_LINE_VAL_STRING,
			wxCMD_LINE_OPTION_MANDATORY
		}, {
			wxCMD_LINE_NONE,
			NULL,
			NULL,
			NULL,
			wxCMD_LINE_VAL_NONE,
			0
		}
	};

	parser.SetDesc(cmdLineDescription);
	parser.SetSwitchChars(wxT("-"));
}

bool
JobCtrlApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
	parser.Found(wxT("t"), &this->testcase_);
	fprintf(stderr, "Testcase: %s\n", (const char*)testcase_.fn_str());
	return (true);
}

bool
JobCtrlApp::canExit() const
{
	TestHandlerList::const_iterator it = handlerList_.begin();
	for (; it != handlerList_.end(); ++it) {
		TestHandler *h = (*it);

		if (!h->canExitTest())
			return (false);
	}

	return (true);
}

int
JobCtrlApp::getResult() const
{
	int result = 0;

	TestHandlerList::const_iterator it = handlerList_.begin();
	for (; it != handlerList_.end(); ++it) {
		TestHandler *h = (*it);
		result += h->getTestResult();
	}

	return (result);
}

int jobCtrlAppEntry(int argc, char **argv)
{
	return wxEntry(argc, argv);
}

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

	if (!jobCtrl->start()) {
		fprintf(stderr, "Failed to start JobCtrl\n");
		return (false);
	}

	if (!jobCtrl->connect()) {
		fprintf(stderr, "No connection to anoubisd\n");
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

	return (getResult());
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

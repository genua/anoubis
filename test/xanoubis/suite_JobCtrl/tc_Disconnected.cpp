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
#include <wx/module.h>

#include <JobCtrl.h>

#include "JobCtrlEventSpy.h"

static wxApp *app = 0;
static JobCtrl *jobCtrl;

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
}

static void
teardown(void)
{
	/* Destry test-object */
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

START_TEST(test_noconnection)
{
	JobCtrlEventSpy spy(jobCtrl);

	fail_if(wxFileExists(wxT("foo")), "Domain-socket already exists");
	jobCtrl->setSocketPath(wxT("foo"));

	fail_unless(jobCtrl->connect(), "Connection request failed");

	spy.waitForInvocation(1);
	fail_if(spy.getLastState() == JobCtrl::CONNECTED,
	    "Unexpected connection-state; is: %i", spy.getLastState());
}
END_TEST

TCase *
getTc_JobCtrlDisconnected(void)
{
	TCase *tcase = tcase_create("JobCtrl_Disconnected");

	tcase_set_timeout(tcase, 10);
	tcase_add_checked_fixture(tcase, setup, teardown);

	tcase_add_test(tcase, test_noconnection);

	return (tcase);
}

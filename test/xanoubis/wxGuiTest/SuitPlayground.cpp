/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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


#include <wxGuiTest/WxGuiTestHelper.h>
#include <wxGuiTest/EventSimulationHelper.h>
#include <wxGuiTest/ModalDialogTimer.h>
#include <wxGuiTest/TempInteractive.h>

#include <ModPlaygroundPanelsBase.h>

#include "SuitPlayground.h"

/* Register test suite. */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(SuitPlayground, "WxGuiTest");

void
SuitPlayground::startPlayground(void)
{
	wxWindow *mainFrame;
	wxWindow *toolBar;
	wxComboBox *appComboBox;
	wxWindow *startButton;

	mainFrame = wxWindow::FindWindowById(ID_MAINFRAME);
	CPPUNIT_ASSERT_MESSAGE("Can't find 'MainFrame'", mainFrame != NULL);

	/* Get tool bar and press PG button. */
	toolBar = mainFrame->FindWindow(ID_TOOLBAR);
	CPPUNIT_ASSERT_MESSAGE("Can't find toolbar!", toolBar != NULL);
	wxTst::EventSimulationHelper::ToggleToolOnlyEvent(15003, true, toolBar);
	wxTst::WxGuiTestHelper::FlushEventQueue();

	/* Enter test app. */
	appComboBox = dynamic_cast<wxComboBox *>(
	    wxWindow::FindWindowById(PG_APP_COMBOBOX));
	CPPUNIT_ASSERT_MESSAGE("Can't find app combo box", appComboBox != NULL);
	appComboBox->SetValue(wxT("/tmp/2000.test.sh"));

	/* Get start button and press it. */
	startButton = wxWindow::FindWindowById(PG_APP_STARTBUTTON);
	CPPUNIT_ASSERT_MESSAGE("Can't find start button", startButton != NULL);
	wxTst::EventSimulationHelper::ClickButton(startButton->GetId(),
	    startButton);
	wxTst::WxGuiTestHelper::FlushEventQueue();
}

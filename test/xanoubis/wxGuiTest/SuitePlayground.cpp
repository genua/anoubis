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

#include <unistd.h>

#include <wxGuiTest/WxGuiTestHelper.h>
#include <wxGuiTest/EventSimulationHelper.h>
#include <wxGuiTest/ModalDialogTimer.h>
#include <wxGuiTest/TempInteractive.h>

#include <AnListCtrl.h>
#include <ModPlaygroundPanelsBase.h>
#include <ModAnoubisPanelsBase.h>

#include "SuitePlayground.h"

/* Register test suite. */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(SuitePlayground, "WxGuiTest");

void
SuitePlayground::do_playground_testsuite(void)
{
	/* Note: these 'testcases' are _not_ independent */
	connect();
	startPlayground();
	checkPlaygroundInfo();
	checkPlaygroundFiles();
}

void
SuitePlayground::connect(void)
{
	wxStaticText *status_text = NULL;

	do {
		/* press connect button */
		wxWindow *connect_button =
		    wxWindow::FindWindowById(MAIN_CONNECTBUTTON);
		CPPUNIT_ASSERT_MESSAGE("Can't find connect button",
		    connect_button != NULL);
		wxTst::EventSimulationHelper::ClickButton(
		    connect_button->GetId(), connect_button);
		wxTst::WxGuiTestHelper::FlushEventQueue();

		/* check connection status */
		status_text = dynamic_cast<wxStaticText*>
		    (wxWindow::FindWindowById(ID_CONNECTIONSTATUS));
		CPPUNIT_ASSERT_MESSAGE("Can't find Connection Status label",
		    status_text != NULL);

		sleep(1);
		fprintf(stderr, "waiting for connect: %ls ...\n",
		    (const wchar_t*)status_text->GetLabel().c_str());
	}
	while (status_text->GetLabel().CmpNoCase(wxT("ok")) != 0);
}

void
SuitePlayground::startPlayground(void)
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
	appComboBox = dynamic_cast<wxComboBox*>(
	    wxWindow::FindWindowById(PG_APP_COMBOBOX));
	CPPUNIT_ASSERT_MESSAGE("Can't find app combo box", appComboBox != NULL);
	appComboBox->SetValue(wxT("/tmp/2000.test.sh"));

	/* Get start button and press it. */
	startButton = wxWindow::FindWindowById(PG_APP_STARTBUTTON);
	CPPUNIT_ASSERT_MESSAGE("Can't find start button", startButton != NULL);
	wxTst::EventSimulationHelper::ClickButton(startButton->GetId(),
	    startButton);
	wxTst::WxGuiTestHelper::FlushEventQueue();

	/* wait some time to let the script do its job */
	sleep(1);
}

wxString
SuitePlayground::getListValue(AnListCtrl* list, long id, int col) {
	/* get the value of the specified row/column of list */
	wxListItem item;
	item.SetId(id);
	item.SetColumn(col);
	list->GetItem(item);
	/* Note: return 'by value' because src string is in 'item' and
	 * will be removed from stack once function ends */
	return item.GetText();
}

void
SuitePlayground::checkPlaygroundInfo(void) {
	fprintf(stderr, "now testing the playground info list\n");

	/* refresh the list */
	fprintf(stderr, "now refreshing the list\n");
	wxWindow *refreshButton =
	    wxWindow::FindWindowById(PG_VIEW_REFRESHBUTTON);
	CPPUNIT_ASSERT_MESSAGE("Can't find refresh button",
	    refreshButton != NULL);
	wxTst::EventSimulationHelper::ClickButton(
	    refreshButton->GetId(), refreshButton);
	wxTst::WxGuiTestHelper::FlushEventQueue();
	sleep(1);

	/* get the grid */
	fprintf(stderr, "now getting view\n");
	wxListCtrl *pg_info_wx = dynamic_cast<wxListCtrl*>
	    (wxWindow::FindWindowById(PG_VIEW_LIST));
	CPPUNIT_ASSERT_MESSAGE("Can't find playground info list",
	    pg_info_wx != NULL);
	AnListCtrl *pg_info = dynamic_cast<AnListCtrl*>(pg_info_wx);

	/* dump rows for debugging */
	long item_id = -1;
	fprintf(stderr, "playground info list:\n");
	while ((item_id = pg_info->GetNextItem(item_id)) != -1) {
		fprintf(stderr, "row %ld: ", item_id);
		for (int col=1; col<7; col++) {
			fprintf(stderr, "%ls ", (const wchar_t*)
			    getListValue(pg_info, item_id, col).c_str());
		}
		fprintf(stderr, "\n");
	}

	/* check that content meets our expectations */
	CPPUNIT_ASSERT_MESSAGE("column 'Benutzer' contains wrong value",
	    getListValue(pg_info, 0, 5).Cmp(wxT("u2000")) == 0);
	CPPUNIT_ASSERT_MESSAGE("column 'Status' contains wrong value",
	    getListValue(pg_info, 0, 3).Cmp(wxT("active")) == 0);
	CPPUNIT_ASSERT_MESSAGE("column 'Dateien' contains wrong value",
	    getListValue(pg_info, 0, 4).Cmp(wxT("1")) == 0);
	CPPUNIT_ASSERT_MESSAGE("column 'Command' contains wrong value",
	    getListValue(pg_info, 0, 2).Cmp(wxT("/tmp/2000.test.sh")) == 0);

	fprintf(stderr, "playground info list is ok\n");
}

void
SuitePlayground::checkPlaygroundFiles(void) {
	/* Select the 1st row in the playground info list */
	fprintf(stderr, "now selecting playground\n");
	wxListCtrl *pg_info = dynamic_cast<wxListCtrl*>
	    (wxWindow::FindWindowById(PG_VIEW_LIST));
	CPPUNIT_ASSERT_MESSAGE("Can't find playground info list",
	    pg_info != NULL);
	pg_info->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	/* click commit files button */
	fprintf(stderr, "now clicking commit button\n");
	wxWindow *commit_button =
	    wxWindow::FindWindowById(PG_VIEW_COMMITBUTTON);
	CPPUNIT_ASSERT_MESSAGE("Can't find commit button",
	    commit_button != NULL);
	wxTst::EventSimulationHelper::ClickButton(
	    commit_button->GetId(), commit_button);
	wxTst::WxGuiTestHelper::FlushEventQueue();

	/* get newly opened window */
	fprintf(stderr, "now getting playground file list\n");
	wxGrid *file_list = dynamic_cast<wxGrid*>
	    (wxWindow::FindWindowById(PG_COMMIT_LIST));
	CPPUNIT_ASSERT_MESSAGE("Can't find playground file list",
	    file_list != NULL);

	/* dump rows for debugging */
	fprintf(stderr, "playground file list:\n");
	for (int row=0; row<file_list->GetNumberRows(); row++) {
		fprintf(stderr, "%ls\n", (const wchar_t*)
		    file_list->GetCellValue(row, 0).c_str());
	}

	/* check content */
	CPPUNIT_ASSERT_MESSAGE("column 'File name' contains wrong value",
	    file_list->GetCellValue(0, 0).Cmp(wxT("/tmp/.plgr.1.foo")) == 0);

	fprintf(stderr, "playground file list is ok\n");
}

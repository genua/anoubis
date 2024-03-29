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
#include <ModAnoubis.h>
#include <ModAnoubisPanelsBase.h>

#include "JobCtrl.h"
#include "SuitePSList.h"

/* Register test suite. */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(SuitePSList, "WxGuiTest");

void
SuitePSList::do_pslist_testsuite(void)
{
	/* Note: these 'testcases' are _not_ independent */
	fprintf(stderr, "----------------------------------------\n");
	connect();
	switch_ps_tab();
	check_ps_list();
}

void
SuitePSList::connect(void)
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
SuitePSList::switch_ps_tab(void)
{
	/* get main, find toolbar, press anoubis button */
	wxWindow *mainFrame;
	mainFrame = wxWindow::FindWindowById(ID_MAINFRAME);
	CPPUNIT_ASSERT_MESSAGE("Can't find 'MainFrame'", mainFrame != NULL);

	wxWindow *toolbar;
	toolbar = mainFrame->FindWindow(ID_TOOLBAR);
	CPPUNIT_ASSERT_MESSAGE("Can't find toolbar!", toolbar != NULL);
	wxTst::EventSimulationHelper::ToggleToolOnlyEvent(
	    MODANOUBIS_ID_TOOLBAR, true, toolbar);
	wxTst::WxGuiTestHelper::FlushEventQueue();

	/* select the process notebook page */
	wxNotebook *notebook;
	notebook = dynamic_cast<wxNotebook*>
	    (wxWindow::FindWindowById(ID_ANOUBIS_MAIN_NOTEBOOK));
	CPPUNIT_ASSERT_MESSAGE("Can't find main notebook", notebook != NULL);
	wxTst::EventSimulationHelper::SelectNotebookPage(notebook, 3);
	wxTst::WxGuiTestHelper::FlushEventQueue();
}

void
SuitePSList::check_ps_list(void)
{
	/* get the list */
	AnListCtrl *pslist;
	pslist = dynamic_cast<AnListCtrl*>
	    //(mainFrame->FindWindow(ID_PSLIST));
	    (wxWindow::FindWindowById(ID_PSLIST));
	CPPUNIT_ASSERT_MESSAGE("Can't find process list", pslist != NULL);

	/* check the content */
	long item_id = -1;
	fprintf(stderr, "process list:\n");
	while ((item_id = pslist->GetNextItem(item_id)) != -1) {
		fprintf(stderr, "process: ");
		for (int i=0; i<7; i++) {
			fprintf(stderr, "%ls ",
			    getListValue(pslist, item_id, i).c_str());
		}
		fprintf(stderr, "\n");
	}

	/* test process 1 */
	long proc1 = getProcessRow(pslist, wxT("2000_1.sh"));

	CPPUNIT_ASSERT_MESSAGE("process 1 does not exist", proc1 != -1);
	CPPUNIT_ASSERT_MESSAGE("process 1 invalid user",
	    getListValue(pslist, proc1, 1).Cmp(wxT("u2000")) == 0);
	CPPUNIT_ASSERT_MESSAGE("process 1 has pg id",
	    getListValue(pslist, proc1, 5).Cmp(wxT("")) == 0);

	pslist->SetItemState(proc1,
	    wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	assertTextValue(ID_LABEL_PS_COMMAND, "/bin/sh /tmp/2000_1.sh");
	assertTextValue(ID_LABEL_PS_UID, "u2000");
	assertTextValue(ID_LABEL_PS_GID, "users");
	assertTextValue(ID_LABEL_PS_PGID, "no");
	assertTextValue(ID_LABEL_PS_PROCESSPATH, "/tmp/2000_1.sh");
	assertTextValue(ID_LABEL_PS_USERPATH, "/tmp/2000_1.sh");
	assertTextValue(ID_LABEL_PS_ADMINPATH, "/tmp/2000_1.sh");

	assertRuleValue(ID_TEXT_PS_ALF_USER,  "2: ");
	assertRuleValue(ID_TEXT_PS_ALF_ADMIN, "2: ");
	assertRuleValue(ID_TEXT_PS_SB_USER,   "6: ");
	assertRuleValue(ID_TEXT_PS_SB_ADMIN,  "8: ");
	assertRuleValue(ID_TEXT_PS_CTX_USER,  "13: ");
	assertRuleValue(ID_TEXT_PS_CTX_ADMIN, "11: ");
	pslist->SetItemState(proc1, 0, wxLIST_STATE_SELECTED);

	/* test process 2 */
	long proc2 = getProcessRow(pslist, wxT("2000_2.sh"));
	CPPUNIT_ASSERT_MESSAGE("process 2 does not exist", proc2 != -1);
	CPPUNIT_ASSERT_MESSAGE("process 2 invalid user",
	    getListValue(pslist, proc2, 1).Cmp(wxT("u2000")) == 0);
	CPPUNIT_ASSERT_MESSAGE("process 2 no playground id",
	    getListValue(pslist, proc2, 5).Cmp(wxT("")) != 0);

	pslist->SetItemState(proc2,
	    wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	assertTextValue(ID_LABEL_PS_COMMAND, "/bin/sh /tmp/2000_2.sh");
	assertTextValue(ID_LABEL_PS_UID, "u2000");
	assertTextValue(ID_LABEL_PS_GID, "users");
	assertTextValue(ID_LABEL_PS_PGID, "yes (ID: 1)");
	assertTextValue(ID_LABEL_PS_PROCESSPATH, "/tmp/2000_2.sh");
	assertTextValue(ID_LABEL_PS_USERPATH, "/tmp/2000_2.sh");
	assertTextValue(ID_LABEL_PS_ADMINPATH, "/tmp/2000_2.sh");

	assertRuleValue(ID_TEXT_PS_ALF_USER,  "4: ");
	assertRuleValue(ID_TEXT_PS_ALF_ADMIN, "3: ");
	assertRuleValue(ID_TEXT_PS_SB_USER,   "10: ");
	assertRuleValue(ID_TEXT_PS_SB_ADMIN,  "16: ");
	assertRuleValue(ID_TEXT_PS_CTX_USER,  "12: ");
	assertRuleValue(ID_TEXT_PS_CTX_ADMIN, "14: ");

	pslist->SetItemState(proc2, 0, wxLIST_STATE_SELECTED);
}

void
SuitePSList::assertRuleValue(long id, const char* str) {
	wxTextCtrl* text;
	text = dynamic_cast<wxTextCtrl*>(wxWindow::FindWindowById(id));

	CPPUNIT_ASSERT_MESSAGE("Can't find text object", text);

	fprintf(stderr, "label %ld:\n%ls\n", id,
	    text->GetValue().c_str());

	CPPUNIT_ASSERT_MESSAGE(
	    "Text value does not match",
	    text->GetValue().StartsWith(wxString::From8BitData(str)));
}

void
SuitePSList::assertTextValue(long id, const char* str) {
	wxStaticText *text;
	text = dynamic_cast<wxStaticText*>(wxWindow::FindWindowById(id));
	CPPUNIT_ASSERT_MESSAGE("Can't find text object", text);

	fprintf(stderr, "label %ld has value '%ls'\n", id,
	    text->GetLabel().c_str());
	CPPUNIT_ASSERT_MESSAGE(
	    "Text value does not match",
	    text->GetLabel().Cmp(wxString::From8BitData(str)) == 0);
}

wxString
SuitePSList::getListValue(AnListCtrl* list, long id, int col) {
	/* get the value of the specified row/column of list */
	wxListItem item;
	item.SetId(id);
	item.SetColumn(col);
	list->GetItem(item);
	/* Note: return 'by value' because src string is in 'item' and
	 * will be removed from stack once function ends */
	return item.GetText();
}

long
SuitePSList::getProcessRow(AnListCtrl* pslist, wxString command) {
	long item_id = -1;
	while ((item_id = pslist->GetNextItem(item_id)) != -1) {
		if (getListValue(pslist, item_id, 6).Cmp(command) == 0) {
			return item_id;
		}
	}
	return -1;
}

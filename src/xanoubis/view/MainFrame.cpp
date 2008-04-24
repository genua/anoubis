/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/app.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

#include "AnEvents.h"
#include "AnShortcuts.h"
#include "AnStatusBar.h"
#include "MainFrameBase.h"
#include "Module.h"
#include "ModAnoubis.h"
#include "ModAnoubisMainPanelImpl.h"

#include "main.h"

#include "MainFrame.h"

MainFrame::MainFrame(wxWindow *parent) : MainFrameBase(parent)
{
	shortcuts_ = new AnShortcuts(this);
	messageAlertCount_ = 0;
	messageEscalationCount_ = 0;

	Connect(anEVT_COM_REMOTESTATION,
	    wxCommandEventHandler(MainFrame::OnRemoteStation), NULL, this);
	Connect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(MainFrame::onLogViewerShow), NULL, this);
	Connect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(MainFrame::onRuleEditorShow), NULL, this);
	Connect(anEVT_MAINFRAME_SHOW,
	    wxCommandEventHandler(MainFrame::onMainFrameShow), NULL, this);
	Connect(anEVT_OPEN_ALERTS,
	    wxCommandEventHandler(MainFrame::OnOpenAlerts), NULL, this);
	Connect(anEVT_OPEN_ESCALATIONS,
	    wxCommandEventHandler(MainFrame::OnOpenEscalations), NULL, this);
}

MainFrame::~MainFrame()
{
	SetStatusBar(NULL);
	delete an_statusbar;
}

void
MainFrame::OnInit(void)
{
	an_statusbar = new AnStatusBar(this);

	setMessageString();
	setConnectionString(wxT("none"));

	SetStatusBar(an_statusbar);
	GetStatusBar()->Show();
	PositionStatusBar();
	Layout();
}

void
MainFrame::addModules(Module* modules[ANOUBIS_MODULESNO])
{
	wxString	 name;
	wxIcon		*icon;
	int		 i;
	int		 id;

	for (i=0; i<ANOUBIS_MODULESNO; i++) {
		name = modules[i]->getNick();
		icon = modules[i]->getIcon();
		id   = modules[i]->getToolbarId();

		/* add module to toolbar and connect selection event */
		tb_LeftToolbarModule->AddRadioTool(id, name, *(icon));
		tb_LeftToolbarModule->Realize();
		this->Connect(id, wxEVT_COMMAND_MENU_SELECTED,
		    wxCommandEventHandler(MainFrame::OnTbModuleSelect));
	}

	/* add overview module as firt been displayed */
	sz_mainframeMain->Add(modules[OVERVIEW]->getMainPanel(), 1,
	    wxEXPAND, 5);
	sz_mainframeMain->Layout();
}

void
MainFrame::onRuleEditorShow(wxCommandEvent& event)
{
	an_menubar->Check(ID_MITOOLSRULEEDITOR, event.GetInt());
}

void
MainFrame::onLogViewerShow(wxCommandEvent& event)
{
	if(!event.GetInt())
		messageAlertCount_ = 0;
	setMessageString();

	an_menubar->Check(ID_MITOOLSLOGVIEWER, event.GetInt());
}

void
MainFrame::onMainFrameShow(wxCommandEvent& event)
{
	this->Show();
}

void
MainFrame::setDaemonConnection(bool state)
{
	an_menubar->Check(ID_MIFILECONNECT, state);
}

void
MainFrame::setConnectionString(wxString host)
{
	wxString label;

	if  (!host.Cmp(wxT("none"))) {
		label = wxT("not connected");
		an_menubar->Check(ID_MIFILECONNECT, false);
	} else {
		label = wxT("connected with\n");
		label += host;
		an_menubar->Check(ID_MIFILECONNECT, true);
	}
	tx_connected->SetLabel(label);
	Layout();
}

void
MainFrame::setMessageString(void)
{
	wxString label;

	label = wxT("Messages: ");
	if (messageEscalationCount_ > 0) {
		label += wxString::Format(wxT("%d\n"), messageEscalationCount_);
	}
	if (messageEscalationCount_ == 0 && messageAlertCount_ > 0) {
		label += wxString::Format(wxT("%d\n"), messageAlertCount_);
	} else {
		label = wxT("No messages\n");
	}

	tx_messages->SetLabel(label);
	Layout();
}

void
MainFrame::OnTbModuleSelect(wxCommandEvent& event)
{
	int id;
	id = event.GetId() - MODULEID_OFFSET_TOOLBAR + \
	     MODULEID_OFFSET_MAINPANEL;

	wxPanel *modulePanel = (wxPanel *)FindWindow(id);

	/* hide and detach the SECOND item of the mainframe sizer */
	sz_mainframeMain->GetItem(1)->GetWindow()->Hide();
	sz_mainframeMain->Detach(1);

	sz_mainframeMain->Add(modulePanel, 1, wxEXPAND, 5);
	modulePanel->Show();
	sz_mainframeMain->Layout();

	if (id == MODANOUBIS_ID_MAINPANEL) {
		((ModAnoubisMainPanelImpl *)modulePanel)->update();
	}
}

void
MainFrame::OnRemoteStation(wxCommandEvent& event)
{
	setConnectionString(*(wxString *)(event.GetClientObject()));
	event.Skip();
}

void
MainFrame::OnOpenAlerts(wxCommandEvent& event)
{
	messageAlertCount_ = event.GetInt();
	setMessageString();
}

void
MainFrame::OnOpenEscalations(wxCommandEvent& event)
{
	messageEscalationCount_ = event.GetInt();
	setMessageString();
}

void
MainFrame::OnMbHelpAboutSelect(wxCommandEvent& event)
{
	wxMessageBox(
	    _("Anoubis GUI\n\nAuthors:\nAndreas Fiessler\nChristian Hiesl"),
	    _("About"), wxOK, this);
}

void
MainFrame::OnMbFileCloseSelect(wxCommandEvent&)
{
	this->Hide();
}

void
MainFrame::OnMbFileImportSelect(wxCommandEvent& event)
{
	wxString	caption = _("Choose a policy file:");
	wxString	wildcard = wxT("*");
	wxString	defaultDir = wxGetApp().getDataDir();
	wxString	defaultFilename = wxEmptyString;
	wxFileDialog	fileDlg(NULL, caption, defaultDir, defaultFilename,
			    wildcard, wxOPEN);

	if (fileDlg.ShowModal() == wxID_OK) {
		wxGetApp().importPolicyFile(fileDlg.GetPath());
	}
}

void
MainFrame::OnMbFileQuitSelect(wxCommandEvent& event)
{
	wxGetApp().quit();
}

bool
MainFrame::OnQuit(void)
{
	uint8_t answer = wxMessageBox(_("xanoubis is going to be closed."),
	    _("Confirm"), wxYES_NO, this);

	if (answer == wxYES) {
		return true;
	} else {
		return false;
	}
}

void
MainFrame::OnMbToolsRuleEditorSelect(wxCommandEvent& event)
{
	wxCommandEvent  showEvent(anEVT_RULEEDITOR_SHOW);
	showEvent.SetInt(event.IsChecked());
	wxGetApp().sendEvent(showEvent);
}

void
MainFrame::OnMbToolsLogViewerSelect(wxCommandEvent& event)
{
	wxCommandEvent  showEvent(anEVT_LOGVIEWER_SHOW);
	showEvent.SetInt(event.IsChecked());
	wxGetApp().sendEvent(showEvent);
}

void
MainFrame::OnMbHelpHelpSelect(wxCommandEvent& event)
{
	printf("Menu Item Help->Help selected\n");
}

void
MainFrame::OnMbFileConnectSelect(wxCommandEvent& event)
{
	wxGetApp().connectCommunicator(event.IsChecked());
}

void
MainFrame::OnMbEditPreferencesSelect(wxCommandEvent& event)
{
	printf("Menu Item Edit->Preferences selected\n");
}

void
MainFrame::OnClose(wxCloseEvent& event)
{
	this->Hide();
}

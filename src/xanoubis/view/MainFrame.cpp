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
#include <wx/msgdlg.h>

#include "AnShortcuts.h"
#include "AnStatusBar.h"
#include "MainFrameBase.h"
#include "Module.h"

#include "main.h"

#include "MainFrame.h"

MainFrame::MainFrame(wxWindow *parent) : MainFrameBase(parent)
{
	shortcuts_ = new AnShortcuts(this);
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
}

void
MainFrame::OnMbHelpAboutSelect(wxCommandEvent& event)
{
	wxMessageBox(
	    _("Anoubis GUI\n\nAuthors:\nAndreas Fiessler\nChristian Hiesl"),
	    _("About"), wxOK, this);
}

void
MainFrame::OnMbFileQuitSelect(wxCommandEvent& event)
{
	uint8_t answer = wxMessageBox(_("Really Quit Anoubis?"), _("Confirm"),
	    wxYES_NO, this);
	if ( answer == wxYES ) {
		this->Close(true);
		(wxGetApp()).close();
	}
}

void
MainFrame::OnMbHelpHelpSelect(wxCommandEvent& event)
{
	printf("Menu Item Help->Help selected\n");
}

void
MainFrame::OnMbFileConnectSelect(wxCommandEvent& event)
{
	printf("Menu Item File->Connect selected\n");
}

void
MainFrame::OnMbEditPreferencesSelect(wxCommandEvent& event)
{
	printf("Menu Item Edit->Preferences selected\n");
}

void
MainFrame::OnSbLogToggle(wxCommandEvent& event)
{
	printf("Statusbar Log Button toggled\n");
}

void
MainFrame::OnSbRuleeditToggle(wxCommandEvent& event)
{
	printf("Statusbar Ruleedit Button toggled\n");
}

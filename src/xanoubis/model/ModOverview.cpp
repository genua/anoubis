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
#  include "config.h"
#endif

#include <wx/stdpaths.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include "Module.h"
#include "ModOverview.h"
#include "ModOverviewMainPanelImpl.h"

ModOverview::ModOverview(wxWindow *parent) : Module()
{
	name_ = wxString(wxT("Overview"));
	nick_ = wxString(wxT("Overview"));
	mainPanel_ = new ModOverviewMainPanelImpl(parent,
	    MODOVERVIEW_ID_MAINPANEL);

	loadIcon(wxT("ModOverview_black_48.png"));
	mainPanel_->Hide();
}

int
ModOverview::getBaseId(void)
{
	return (MODOVERVIEW_ID_BASE);
}

int
ModOverview::getToolbarId(void)
{
	return (MODOVERVIEW_ID_TOOLBAR);
}

void
ModOverview::update(void)
{
}

void
ModOverview::addModules(Module* modules[ANOUBIS_MODULESNO])
{
	int		 i;
	wxBoxSizer	*sz_v;
	wxSize		 length;

	sz_v = new wxBoxSizer(wxVERTICAL);

	for (i=0; i<ANOUBIS_MODULESNO; i++) {
		if ((modules[i] == NULL) or (modules[i] == this))
			continue;

		wxBoxSizer *sz_h = new wxBoxSizer(wxHORIZONTAL);

		wxStaticText *tx_modLabel = new wxStaticText(mainPanel_,
		    wxID_ANY, modules[i]->getName());
		length.SetWidth(750 - (tx_modLabel->GetSize()).GetWidth());
		wxStaticLine *sl_line = new wxStaticLine(mainPanel_, wxID_ANY,
		    wxDefaultPosition, length, wxLI_HORIZONTAL|wxEXPAND);

		sz_h->Add(tx_modLabel, 0, wxFIXED_MINSIZE|wxLEFT|wxRIGHT|wxTOP,
		     5);
		sz_h->Add(sl_line, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5);

		sz_v->Add(sz_h);
		sz_v->Add(modules[i]->getOverviewPanel(), 0, wxEXPAND, 5);

		modules[i]->getOverviewPanel()->Reparent(mainPanel_);
		modules[i]->getOverviewPanel()->Show(true);
	}

	mainPanel_->GetSizer()->Add(sz_v);
	mainPanel_->Show(true);
	mainPanel_->Layout();
}

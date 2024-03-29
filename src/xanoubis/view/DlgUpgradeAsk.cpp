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

#include <wx/button.h>
#include <wx/sizer.h>

#include "AnEvents.h"
#include "DlgUpgradeAsk.h"

DlgUpgradeAsk::DlgUpgradeAsk(wxWindow *parent)
    : AnMessageDialog(parent, _("The system has been upgraded. Therefore the "
    "checksums of all related files have changed. Please check the checksums "
    "of your signed files in the SFS Browser!"),
    _("Anoubis - System Update"), wxOK | wxICON_INFORMATION)
{
	/* Insert a new button */
	wxButton *sfsButton = new wxButton(
	    this, -1, _("Open SFS Browser now!"));
	sfsButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(
	    DlgUpgradeAsk::onSfsBrowserShow), NULL, this);
	sfsButton->SetFocus();
	buttonSizer_->Add(sfsButton, 0, wxALL, 5);

	onNotifyCheck(wxT("/Options/ShowUpgradeMessage"));

	this->Layout();
	GetSizer()->Fit(this);
}

DlgUpgradeAsk::~DlgUpgradeAsk(void)
{
	/* Destructor */
}

void
DlgUpgradeAsk::onSfsBrowserShow(wxCommandEvent& WXUNUSED(event))
{
	wxCommandEvent event(anEVT_SFSBROWSER_SHOW);
	event.SetInt(1);
	wxPostEvent(AnEvents::instance(), event);
	EndModal(wxID_OK);
}

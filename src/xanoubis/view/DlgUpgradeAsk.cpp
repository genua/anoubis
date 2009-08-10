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

#include "DlgUpgradeAsk.h"
#include <wx/msgdlg.h>


DlgUpgradeAsk::DlgUpgradeAsk(wxWindow *) : DlgUpgradeAskBase(NULL)
{
	/* Constructor */
	userOptions_ = wxGetApp().getUserOptions();
	upgradeMessage_ = userOptions_->Read(wxT
	    ("/Options/ShowUpgradeMessage"),&ShowUpgradeMessage_);
	showAgainCheckBox->SetValue(upgradeMessage_);
}

DlgUpgradeAsk::~DlgUpgradeAsk(void)
{
	/* Destructor */
}

void
DlgUpgradeAsk::close(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}

void
DlgUpgradeAsk::openSFS(wxCommandEvent& WXUNUSED(event))
{
	/* XXX DZ: Further work has to be done here; Switch to SfsBrowser */
	wxMessageBox(_("SFS Browser"),_("SFS Browser"), wxOK, this);
}

void
DlgUpgradeAsk::onUpgradeNotifyCheck(wxCommandEvent& WXUNUSED(event))
{
	 userOptions_->Write(wxT("/Options/ShowUpgradeMessage"),
	     showAgainCheckBox->IsChecked());
}

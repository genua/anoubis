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

#include "DlgProfileSelection.h"
#include "PolicyCtrl.h"

DlgProfileSelection::DlgProfileSelection(const wxString &selection,
    wxWindow *parent) : ModAnoubisProfileSelectionDialogBase(parent)
{
	PolicyCtrl *policyCtrl = PolicyCtrl::instance();

	/* Filter writeable profiles */
	for (int i = 0; i < policyCtrl->getSize(); i++) {
		if (policyCtrl->isProfileWritable
		    (policyCtrl->getProfile(i)->getProfileName()))
			profilesCombo->Append
			    (policyCtrl->getProfile(i)->getProfileName());
	}

	if (policyCtrl->isProfileWritable(selection)) {
		profilesCombo->SetValue(selection);
		buttonSizerOK->Enable(selection != wxEmptyString);
	} else
		buttonSizerOK->Enable(false);
}

wxString
DlgProfileSelection::getSelectedProfile(void) const
{
	return (profilesCombo->GetValue());
}

void
DlgProfileSelection::OnTextChanged(wxCommandEvent &)
{
	buttonSizerOK->Enable(profilesCombo->GetValue() != wxEmptyString);
}

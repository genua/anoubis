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

#include <sys/types.h>
#include <pwd.h>

#include <wx/app.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

#include "DlgBackupPolicyImpl.h"
#include "PolicyRuleSet.h"
#include "main.h"

DlgBackupPolicy::DlgBackupPolicy(wxIcon *icon, PolicyRuleSet *rs)
    : DlgBackupPolicyBase(NULL)
{
	struct passwd		*pwd;
	wxString		 policytext = wxT("(unknown)");
	wxString		 user;

	ruleset_ = rs;
	homedir_ = wxT("/");
	pwd = getpwuid(ruleset_->getUid());
	if (pwd && pwd->pw_dir) {
	}
	homedir_ = wxGetApp().getDataDir();
	user = wxGetApp().getUserNameById(rs->getUid());
	if (user.IsEmpty()) {
		user = wxString::Format(_("uid %d"), rs->getUid());
	}
	if (pwd && pwd->pw_name != NULL) {
		if (ruleset_->isAdmin()) {
			policytext = wxString::Format(_("Admin Policy of %ls"),
			    user.c_str());
		} else {
			policytext = wxString::Format(_("User Policy of %ls"),
			    user.c_str());
		}
	}
	tx_policyright->SetLabel(policytext);
	if (icon)
		bm_info->SetIcon(*icon);
}

void
DlgBackupPolicy::onDiscardButton(wxCommandEvent &)
{
	EndModal(wxID_CANCEL);
}

void
DlgBackupPolicy::onSaveButton(wxCommandEvent &)
{
	bool		ok;
	wxString	filename;
	wxFileDialog	fileDlg(this, wxEmptyString, wxEmptyString,
			    wxEmptyString, wxEmptyString,
			    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	wxBeginBusyCursor();
	fileDlg.SetDirectory(homedir_);
	fileDlg.SetFilename(wxT(""));
	fileDlg.SetWildcard(wxT("*"));
	wxEndBusyCursor();

	if (fileDlg.ShowModal() == wxID_OK) {
		filename = fileDlg.GetPath();
		if (!filename.IsEmpty()) {
			wxBeginBusyCursor();
			ok = ruleset_->exportToFile(filename);
			wxEndBusyCursor();
			if (ok)
				EndModal(wxID_OK);
			else
				wxMessageBox(_("Failed to save the ruleset "
				    "into a file."), _("Save ruleset"),
				    wxOK|wxICON_ERROR, this);
		}
	}
}

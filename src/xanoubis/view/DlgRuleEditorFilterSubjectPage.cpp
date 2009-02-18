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

#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/string.h>

#include "DlgRuleEditorFilterSubjectPage.h"

DlgRuleEditorFilterSubjectPage::DlgRuleEditorFilterSubjectPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorPage(),
    DlgRuleEditorFilterSubjectPageBase(parent, id, pos, size, style)
{
	sfsPolicy_ = NULL;
	sbPolicy_  = NULL;
}

void
DlgRuleEditorFilterSubjectPage::update(Subject *subject)
{
	if (subject == sfsPolicy_) {
		/* This is our sfs policy. */
		showSfsPath();
		showSubject();
	}
	if (subject == sbPolicy_) {
		/* This is our sb policy. */
		showSbPath();
		showSubject();
	}
}

void
DlgRuleEditorFilterSubjectPage::select(Policy *policy)
{
	if (policy->IsKindOf(CLASSINFO(SfsFilterPolicy))) {
		sfsPolicy_ = wxDynamicCast(policy, SfsFilterPolicy);
		DlgRuleEditorPage::select(policy);
		Show();
	}
	if (policy->IsKindOf(CLASSINFO(SbAccessFilterPolicy))) {
		sbPolicy_ = wxDynamicCast(policy, SbAccessFilterPolicy);
		DlgRuleEditorPage::select(policy);
		Show();
	}
}

void
DlgRuleEditorFilterSubjectPage::deselect(void)
{
	sfsPolicy_ = NULL;
	sbPolicy_  = NULL;
	DlgRuleEditorPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterSubjectPage::showSfsPath(void)
{
	if (sfsPolicy_ == NULL) {
		return;
	}
	pathTextCtrl->ChangeValue(sfsPolicy_->getPath());
}

void
DlgRuleEditorFilterSubjectPage::showSbPath(void)
{
	if (sbPolicy_ == NULL) {
		return;
	}
	pathTextCtrl->ChangeValue(sbPolicy_->getPath());
}

void
DlgRuleEditorFilterSubjectPage::showSubject(void)
{
	int	 type;
	wxString value;

	if (sfsPolicy_ != NULL) {
		type  = sfsPolicy_->getSubjectTypeNo();
		value = sfsPolicy_->getSubjectName();
		if (!selfRadioButton->IsShown()) {
			selfRadioButton->Show();
		}
		if (!selfSignedRadioButton->IsShown()) {
			selfSignedRadioButton->Show();
		}
		csumRadioButton->Hide();
		csumTextCtrl->Hide();
	} else if (sbPolicy_ != NULL) {
		type  = sbPolicy_->getSubjectTypeNo();
		value = sbPolicy_->getSubjectName();
		selfRadioButton->Hide();
		selfSignedRadioButton->Hide();
		if (!csumRadioButton->IsShown()) {
			csumRadioButton->Show();
		}
		if (!csumTextCtrl->IsShown()) {
			csumTextCtrl->Show();
		}
	} else {
		/* Neither sfs nor sb - abort */
		return;
	}

	/*
	 * Leave handling of the *TextCtrl widgets within switch cases,
	 * because on Disable() they loose the focus. On a hit to <enter>
	 * this will lead to a KillFocus event, which is processed just
	 * after the Clear() but before the value update. In the end the
	 * result is a empty input instead to the expected value.
	 */
	switch (type) {
	case APN_CS_UID_SELF:
		uidTextCtrl->Clear();
		uidTextCtrl->Disable();
		keyTextCtrl->Clear();
		keyTextCtrl->Disable();
		csumTextCtrl->Clear();
		csumTextCtrl->Disable();
		selfRadioButton->SetValue(true);
		break;
	case APN_CS_KEY_SELF:
		uidTextCtrl->Clear();
		uidTextCtrl->Disable();
		keyTextCtrl->Clear();
		keyTextCtrl->Disable();
		csumTextCtrl->Clear();
		csumTextCtrl->Disable();
		selfSignedRadioButton->SetValue(true);
		break;
	case APN_CS_UID:
		keyTextCtrl->Clear();
		keyTextCtrl->Disable();
		csumTextCtrl->Clear();
		csumTextCtrl->Disable();
		value.Replace(wxT("uid "), wxEmptyString);
		if (value.Cmp(uidTextCtrl->GetValue()) != 0) {
			uidTextCtrl->ChangeValue(value);
		}
		if (uidRadioButton->GetValue() != true) {
			uidRadioButton->SetValue(true);
		}
		if (!uidTextCtrl->IsEnabled()) {
			uidTextCtrl->Enable();
		}
		break;
	case APN_CS_KEY:
		uidTextCtrl->Clear();
		uidTextCtrl->Disable();
		csumTextCtrl->Clear();
		csumTextCtrl->Disable();
		value.Replace(wxT("key "), wxEmptyString);
		if (value.Cmp(keyTextCtrl->GetValue()) != 0) {
			keyTextCtrl->ChangeValue(value);
		}
		if (keyRadioButton->GetValue() != true) {
			keyRadioButton->SetValue(true);
		}
		if (!keyTextCtrl->IsEnabled()) {
			keyTextCtrl->Enable();
		}
		break;
	case APN_CS_CSUM:
		uidTextCtrl->Clear();
		uidTextCtrl->Disable();
		keyTextCtrl->Clear();
		keyTextCtrl->Disable();
		value.Replace(wxT("csum "), wxEmptyString);
		if (value.Cmp(csumTextCtrl->GetValue()) != 0) {
			csumTextCtrl->ChangeValue(value);
		}
		if (csumRadioButton->GetValue() != true) {
			csumRadioButton->SetValue(true);
		}
		if (!csumTextCtrl->IsEnabled()) {
			csumTextCtrl->Enable();
		}
		break;
	case APN_CS_NONE:
		anyRadioButton->SetValue(true);
		break;
	}

	Layout();
	Refresh();
}

void
DlgRuleEditorFilterSubjectPage::onPathTextEnter(wxCommandEvent & event)
{
	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setPath(event.GetString());
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setPath(event.GetString());
	}
}

void
DlgRuleEditorFilterSubjectPage::onModifyButton(wxCommandEvent &)
{
	wxFileName	 defaultPath;
	wxFileDialog	*fileDlg;

	if (sfsPolicy_ != NULL) {
		defaultPath.Assign(sfsPolicy_->getPath());
	}
	if (sbPolicy_ != NULL) {
		defaultPath.Assign(sbPolicy_->getPath());
	}

	wxBeginBusyCursor();
	fileDlg = new wxFileDialog(this);
	fileDlg->SetDirectory(defaultPath.GetPath());
	fileDlg->SetFilename(defaultPath.GetFullName());
	fileDlg->SetWildcard(wxT("*"));
	wxEndBusyCursor();

	if (fileDlg->ShowModal() == wxID_OK) {
		if (sfsPolicy_ != NULL) {
			sfsPolicy_->setPath(fileDlg->GetPath());
		}
		if (sbPolicy_ != NULL) {
			sbPolicy_->setPath(fileDlg->GetPath());
		}
	}

	delete fileDlg;
}

void
DlgRuleEditorFilterSubjectPage::onAnyRadioButton(wxCommandEvent &)
{
	/* XXX ch: this must be implemented, too */
}

void
DlgRuleEditorFilterSubjectPage::onSelfRadioButton(wxCommandEvent &)
{
	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setSubjectSelf(false);
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectSelf(false);
	}
}

void
DlgRuleEditorFilterSubjectPage::onSelfSignedRadioButton(wxCommandEvent &)
{
	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setSubjectSelf(true);
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectSelf(true);
	}
}

void
DlgRuleEditorFilterSubjectPage::onUidRadioButton(wxCommandEvent &)
{
	uidTextCtrl->Enable();
	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setSubjectUid(geteuid());
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectUid(geteuid());
	}
}

void
DlgRuleEditorFilterSubjectPage::onUidTextEnter(wxCommandEvent & event)
{
	unsigned long uid;

	event.GetString().ToULong(&uid);

	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setSubjectUid((uid_t)uid);
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectUid((uid_t)uid);
	}
}

void
DlgRuleEditorFilterSubjectPage::onUidTextKillFocus(wxFocusEvent &)
{
	unsigned long uid;

	uidTextCtrl->GetValue().ToULong(&uid);

	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setSubjectUid(uid);
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectUid(uid);
	}
}

void
DlgRuleEditorFilterSubjectPage::onKeyRadioButton(wxCommandEvent &)
{
	keyTextCtrl->Enable();
	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setSubjectKey(wxEmptyString);
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectKey(wxEmptyString);
	}
}

void
DlgRuleEditorFilterSubjectPage::onKeyTextEnter(wxCommandEvent & event)
{
	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setSubjectKey(event.GetString());
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectKey(event.GetString());
	}
}

void
DlgRuleEditorFilterSubjectPage::onKeyTextKillFocus(wxFocusEvent &)
{
	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setSubjectKey(keyTextCtrl->GetValue());
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectKey(keyTextCtrl->GetValue());
	}
}

void
DlgRuleEditorFilterSubjectPage::onCsumRadioButton(wxCommandEvent &)
{
	csumTextCtrl->Enable();
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectCsum(wxEmptyString);
	}
}

void
DlgRuleEditorFilterSubjectPage::onCsumTextEnter(wxCommandEvent & event)
{
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectCsum(event.GetString());
	}
}

void
DlgRuleEditorFilterSubjectPage::onCsumTextKillFocus(wxFocusEvent &)
{
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectCsum(csumTextCtrl->GetValue());
	}
}

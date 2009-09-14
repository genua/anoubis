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

#include "ContextFilterPolicy.h"
#include "AnPickFromFs.h"
#include "DlgRuleEditorFilterSubjectPage.h"

DlgRuleEditorFilterSubjectPage::DlgRuleEditorFilterSubjectPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorPage(),
    DlgRuleEditorFilterSubjectPageBase(parent, id, pos, size, style)
{
	sfsPolicy_ = NULL;
	sbPolicy_  = NULL;
	appPolicy_ = NULL;
	ctxPolicy_ = NULL;

	addSubject(pathPicker);
	pathPicker->setMode(AnPickFromFs::MODE_BOTH);
	pathPicker->setTitle(_("Path:"));
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
	if (subject == appPolicy_) {
		/* This is our app policy. */
		showAppPath();
		showSubject();
		if (!appPolicy_->isAnyBlock())
			Enable(enable_);
	}
	if (subject == ctxPolicy_) {
		/* This is our ctx policy */
		showCtxPath();
		showSubject();
		if (!ctxPolicy_->isAny())
			Enable(enable_);
	}
	if (subject == pathPicker) {
		if (sfsPolicy_ != NULL) {
			sfsPolicy_->setPath(pathPicker->getFileName());
		}
		if (sbPolicy_ != NULL) {
			sbPolicy_->setPath(pathPicker->getFileName());
		}
		if (appPolicy_ != NULL) {
			appPolicy_->setBinaryName(pathPicker->getFileName(),
			    binaryIndex_);
		}
		if (ctxPolicy_ != NULL) {
			ctxPolicy_->setBinaryName(pathPicker->getFileName(),
			    binaryIndex_);
		}
	}
}

void
DlgRuleEditorFilterSubjectPage::select(Policy *policy)
{
	if (policy->IsKindOf(CLASSINFO(SfsFilterPolicy))) {
		sfsPolicy_ = wxDynamicCast(policy, SfsFilterPolicy);
		DlgRuleEditorPage::select(policy);
		pathPicker->setMode(AnPickFromFs::MODE_BOTH);
		pathPicker->setTitle(_("Path:"));
		Enable(enable_);
		Show();
	}
	if (policy->IsKindOf(CLASSINFO(SbAccessFilterPolicy))) {
		sbPolicy_ = wxDynamicCast(policy, SbAccessFilterPolicy);
		DlgRuleEditorPage::select(policy);
		pathPicker->setMode(AnPickFromFs::MODE_BOTH);
		pathPicker->setTitle(_("Path:"));
		Enable(enable_);
		Show();
	}
	if (policy->IsKindOf(CLASSINFO(AppPolicy))) {
		appPolicy_ = wxDynamicCast(policy, AppPolicy);
		DlgRuleEditorPage::select(policy);
		pathPicker->setMode(AnPickFromFs::MODE_FILE);
		pathPicker->setTitle(_("Binary:"));
		/* Do not allow entries in an any Policy. Must use "Add". */
		if (appPolicy_->isAnyBlock())
			Enable(false);
		else
			Enable(enable_);
		Show();
	}
	if (policy->IsKindOf(CLASSINFO(ContextFilterPolicy))) {
		ctxPolicy_ = wxDynamicCast(policy, ContextFilterPolicy);
		DlgRuleEditorPage::select(policy);
		pathPicker->setMode(AnPickFromFs::MODE_FILE);
		pathPicker->setTitle(_("Binary:"));
		/* Do not allow entries in an any Policy. Must use "Add". */
		if (ctxPolicy_->isAny())
			Enable(false);
		else
			Enable(enable_);
		Show();
	}
}

void
DlgRuleEditorFilterSubjectPage::deselect(void)
{
	sfsPolicy_ = NULL;
	sbPolicy_  = NULL;
	appPolicy_ = NULL;
	ctxPolicy_ = NULL;
	DlgRuleEditorPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterSubjectPage::showSfsPath(void)
{
	if (sfsPolicy_ == NULL) {
		return;
	}
	pathPicker->setFileName(sfsPolicy_->getPath());
}

void
DlgRuleEditorFilterSubjectPage::showSbPath(void)
{
	if (sbPolicy_ == NULL) {
		return;
	}
	pathPicker->setFileName(sbPolicy_->getPath());
}

void
DlgRuleEditorFilterSubjectPage::showAppPath(void)
{
	if (appPolicy_ == NULL)
		return;
	pathPicker->setFileName(appPolicy_->getBinaryName(binaryIndex_));
}

void
DlgRuleEditorFilterSubjectPage::showCtxPath(void)
{
	if (ctxPolicy_ == NULL)
		return;
	pathPicker->setFileName(ctxPolicy_->getBinaryName(binaryIndex_));
}

void
DlgRuleEditorFilterSubjectPage::showSubject(void)
{
	int	 type;
	wxString value;

	if (sfsPolicy_ != NULL) {
		type  = sfsPolicy_->getSubjectTypeNo();
		value = sfsPolicy_->getSubjectName();
		anyRadioButton->Hide();
	} else if (sbPolicy_ != NULL) {
		type  = sbPolicy_->getSubjectTypeNo();
		value = sbPolicy_->getSubjectName();
		if (!anyRadioButton->IsShown()) {
			anyRadioButton->Show();
		}
	} else if (appPolicy_ != NULL) {
		anyRadioButton->Show();
		type = appPolicy_->getSubjectTypeNo(binaryIndex_);
		value = appPolicy_->getSubjectName(binaryIndex_);
	} else if (ctxPolicy_ != NULL) {
		anyRadioButton->Show();
		type = ctxPolicy_->getSubjectTypeNo(binaryIndex_);
		value = ctxPolicy_->getSubjectName(binaryIndex_);
	} else {
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
		selfRadioButton->SetValue(true);
		break;
	case APN_CS_KEY_SELF:
		uidTextCtrl->Clear();
		uidTextCtrl->Disable();
		keyTextCtrl->Clear();
		keyTextCtrl->Disable();
		selfSignedRadioButton->SetValue(true);
		break;
	case APN_CS_UID:
		keyTextCtrl->Clear();
		keyTextCtrl->Disable();
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
	case APN_CS_NONE:
		uidTextCtrl->Clear();
		uidTextCtrl->Disable();
		keyTextCtrl->Clear();
		keyTextCtrl->Disable();
		anyRadioButton->SetValue(true);
		break;
	}

	Layout();
	Refresh();
}

void
DlgRuleEditorFilterSubjectPage::onAnyRadioButton(wxCommandEvent &)
{
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectNone();
	}
	if (appPolicy_ != NULL) {
		appPolicy_->setSubjectNone(binaryIndex_);
	}
	if (ctxPolicy_ != NULL) {
		ctxPolicy_->setSubjectNone(binaryIndex_);
	}
}

void
DlgRuleEditorFilterSubjectPage::setSubjectSelf(bool sign)
{
	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setSubjectSelf(sign);
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectSelf(sign);
	}
	if (appPolicy_ != NULL) {
		appPolicy_->setSubjectSelf(binaryIndex_, sign);
	}
	if (ctxPolicy_ != NULL) {
		ctxPolicy_->setSubjectSelf(binaryIndex_, sign);
	}
}

void
DlgRuleEditorFilterSubjectPage::onSelfRadioButton(wxCommandEvent &)
{
	setSubjectSelf(false);
}

void
DlgRuleEditorFilterSubjectPage::onSelfSignedRadioButton(wxCommandEvent &)
{
	setSubjectSelf(true);
}

void
DlgRuleEditorFilterSubjectPage::setSubjectUid(uid_t uid)
{
	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setSubjectUid(uid);
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectUid(uid);
	}
	if (appPolicy_ != NULL) {
		appPolicy_->setSubjectUid(binaryIndex_, uid);
	}
	if (ctxPolicy_ != NULL) {
		ctxPolicy_->setSubjectUid(binaryIndex_, uid);
	}
}

void
DlgRuleEditorFilterSubjectPage::onUidRadioButton(wxCommandEvent &)
{
	uidTextCtrl->Enable();
	setSubjectUid(geteuid());
}

void
DlgRuleEditorFilterSubjectPage::onUidTextEnter(wxCommandEvent & event)
{
	unsigned long uid;

	event.GetString().ToULong(&uid);
	setSubjectUid((uid_t)uid);
}

void
DlgRuleEditorFilterSubjectPage::onUidTextKillFocus(wxFocusEvent &)
{
	unsigned long uid;

	if (uidTextCtrl->IsModified()) {
		/* Mark as clean */
		uidTextCtrl->DiscardEdits();
		uidTextCtrl->GetValue().ToULong(&uid);
		setSubjectUid((uid_t)uid);
	}
}

void
DlgRuleEditorFilterSubjectPage::setSubjectKey(wxString key)
{
	if (sfsPolicy_ != NULL) {
		sfsPolicy_->setSubjectKey(key);
	}
	if (sbPolicy_ != NULL) {
		sbPolicy_->setSubjectKey(key);
	}
	if (appPolicy_ != NULL) {
		appPolicy_->setSubjectKey(binaryIndex_, key);
	}
	if (ctxPolicy_ != NULL) {
		ctxPolicy_->setSubjectKey(binaryIndex_, key);
	}
}

void
DlgRuleEditorFilterSubjectPage::onKeyRadioButton(wxCommandEvent &)
{
	keyTextCtrl->Enable();
	setSubjectKey(wxEmptyString);
}

void
DlgRuleEditorFilterSubjectPage::onKeyTextEnter(wxCommandEvent & event)
{
	setSubjectKey(event.GetString());
}

void
DlgRuleEditorFilterSubjectPage::onKeyTextKillFocus(wxFocusEvent &)
{
	if (keyTextCtrl->IsModified()) {
		/* Mark as clean */
		keyTextCtrl->DiscardEdits();
		setSubjectKey(keyTextCtrl->GetValue());
	}
}

void
DlgRuleEditorFilterSubjectPage::setBinaryIndex(unsigned int idx)
{
	binaryIndex_ = idx;
}

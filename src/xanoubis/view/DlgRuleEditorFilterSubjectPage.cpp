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

/**
 * Client-data passed to subjectChoice.
 *
 * Contains the APN_CS_-type. Each entry in subjectChoice represents a specific
 * subject-type.
 */
class SubjectChoiceData
{
	public:
		SubjectChoiceData(int type)
		{
			this->type_ = type;
		}

		int type_;
};

/*
 * The different client-data-instances, which are passed to subjectChoice.
 *
 * They can be made static because the instances are never modified.
 */
static SubjectChoiceData cs_none(APN_CS_NONE);
static SubjectChoiceData cs_uid_self(APN_CS_UID_SELF);
static SubjectChoiceData cs_key_self(APN_CS_KEY_SELF);
static SubjectChoiceData cs_uid(APN_CS_UID);
static SubjectChoiceData cs_key(APN_CS_KEY);

DlgRuleEditorFilterSubjectPage::DlgRuleEditorFilterSubjectPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorPage(),
    DlgRuleEditorFilterSubjectPageBase(parent, id, pos, size, style)
{
	sfsPolicy_ = NULL;
	sbPolicy_  = NULL;
	appPolicy_ = NULL;
	ctxPolicy_ = NULL;

	/* Fill subjectChoice with all possible values */
	subjectChoice->Append(_("any"), &cs_none);
	subjectChoice->Append(_("self"), &cs_uid_self);
	subjectChoice->Append(_("self-signed"), &cs_key_self);
	subjectChoice->Append(_("uid"), &cs_uid);
	subjectChoice->Append(_("key"), &cs_key);

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
	if (dynamic_cast<SfsFilterPolicy*>(policy)) {
		sfsPolicy_ = dynamic_cast<SfsFilterPolicy*>(policy);
		DlgRuleEditorPage::select(policy);
		pathPicker->setMode(AnPickFromFs::MODE_BOTH);
		pathPicker->setTitle(_("Path:"));
		Enable(enable_);
		Show();
	}
	if (dynamic_cast<SbAccessFilterPolicy*>(policy)) {
		sbPolicy_ = dynamic_cast<SbAccessFilterPolicy*>(policy);
		DlgRuleEditorPage::select(policy);
		pathPicker->setMode(AnPickFromFs::MODE_BOTH);
		pathPicker->setTitle(_("Path:"));
		Enable(enable_);
		Show();
	}
	if (dynamic_cast<AppPolicy*>(policy)) {
		appPolicy_ = dynamic_cast<AppPolicy*>(policy);
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
	if (dynamic_cast<ContextFilterPolicy*>(policy)) {
		ctxPolicy_ = dynamic_cast<ContextFilterPolicy*>(policy);
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
		updateSubjectChoice(type, false);
	} else if (sbPolicy_ != NULL) {
		type  = sbPolicy_->getSubjectTypeNo();
		value = sbPolicy_->getSubjectName();
		updateSubjectChoice(type, true);
	} else if (appPolicy_ != NULL) {
		type = appPolicy_->getSubjectTypeNo(binaryIndex_);
		value = appPolicy_->getSubjectName(binaryIndex_);
		updateSubjectChoice(type, true);
	} else if (ctxPolicy_ != NULL) {
		type = ctxPolicy_->getSubjectTypeNo(binaryIndex_);
		value = ctxPolicy_->getSubjectName(binaryIndex_);
		updateSubjectChoice(type, true);
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
		updateSubjectTextCtrl(false, wxEmptyString);
		break;
	case APN_CS_KEY_SELF:
		updateSubjectTextCtrl(false, wxEmptyString);
		break;
	case APN_CS_UID:
		value.Replace(wxT("uid "), wxEmptyString);
		updateSubjectTextCtrl(true, value);
		break;
	case APN_CS_KEY:
		value.Replace(wxT("key "), wxEmptyString);
		updateSubjectTextCtrl(true, value);
		break;
	case APN_CS_NONE:
		updateSubjectTextCtrl(false, wxEmptyString);
		break;
	}

	Layout();
	Refresh();
}

void
DlgRuleEditorFilterSubjectPage::onSubjectSelected(wxCommandEvent &)
{
	int selection = subjectChoice->GetSelection();
	SubjectChoiceData *data = static_cast<SubjectChoiceData *>(
	    subjectChoice->GetClientData(selection));

	if (data->type_ == APN_CS_NONE) {
		/* any */
		if (sbPolicy_ != NULL) {
			sbPolicy_->setSubjectNone();
		}
		if (appPolicy_ != NULL) {
			appPolicy_->setSubjectNone(binaryIndex_);
		}
		if (ctxPolicy_ != NULL) {
			ctxPolicy_->setSubjectNone(binaryIndex_);
		}
	} else if (data->type_ == APN_CS_UID_SELF) {
		/* self */
		setSubjectSelf(false);
	} else if (data->type_ == APN_CS_KEY_SELF) {
		/* self-signed */
		setSubjectSelf(true);
	} else if (data->type_ == APN_CS_UID) {
		/* uid */
		setSubjectUid(geteuid());
	} else if (data->type_ == APN_CS_KEY) {
		/* key */
		setSubjectKey(wxEmptyString);
	}

	showSubject();
}

void
DlgRuleEditorFilterSubjectPage::onSubjectTextEnter(wxCommandEvent &event)
{
	int selection = subjectChoice->GetSelection();
	SubjectChoiceData *data = static_cast<SubjectChoiceData *>(
	    subjectChoice->GetClientData(selection));

	if (data->type_ == APN_CS_UID) {
		/* uid */
		unsigned long uid;

		if (event.GetString().ToULong(&uid))
			setSubjectUid((uid_t)uid);
	} else if (data->type_ == APN_CS_KEY) {
		/* key */
		setSubjectKey(event.GetString());
	}
}

void
DlgRuleEditorFilterSubjectPage::onSubjectTextKillFocus(wxFocusEvent &)
{
	int selection = subjectChoice->GetSelection();
	SubjectChoiceData *data = static_cast<SubjectChoiceData *>(
	    subjectChoice->GetClientData(selection));

	if (data->type_ == APN_CS_UID) {
		/* uid */
		unsigned long uid;

		if (subjectTextCtrl->IsModified()) {
			/* Mark as clean */
			subjectTextCtrl->DiscardEdits();
			if (subjectTextCtrl->GetValue().ToULong(&uid))
				setSubjectUid((uid_t)uid);
		}
	} else if (data->type_ == APN_CS_KEY) {
		/* key */
		if (subjectTextCtrl->IsModified()) {
			/* Mark as clean */
			subjectTextCtrl->DiscardEdits();
			setSubjectKey(subjectTextCtrl->GetValue());
		}
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
DlgRuleEditorFilterSubjectPage::updateSubjectChoice(int type, bool showAny)
{
	int count = subjectChoice->GetCount();

	if (count > 4 && !showAny) {
		/* Remove any-entry */
		subjectChoice->Delete(0);
	} else if (count <= 4 && showAny) {
		/* Insert any-entry */
		subjectChoice->Insert(_("any"), 0, &cs_none);
	}

	/* Update the selection according to the type */
	for (unsigned int i = 0; i < subjectChoice->GetCount(); i++) {
		SubjectChoiceData *data = static_cast<SubjectChoiceData *>(
		    subjectChoice->GetClientData(i));
		if (data->type_ == type) {
			subjectChoice->SetSelection(i);
		}
	}
}

void DlgRuleEditorFilterSubjectPage::updateSubjectTextCtrl(bool enabled,
	const wxString &value)
{
	subjectTextCtrl->Enable(enabled);
	subjectTextCtrl->SetEditable(enabled);

	if (value.Cmp(subjectTextCtrl->GetValue()) != 0) {
		subjectTextCtrl->ChangeValue(value);
	}

}

void
DlgRuleEditorFilterSubjectPage::setBinaryIndex(unsigned int idx)
{
	binaryIndex_ = idx;
}

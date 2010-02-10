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

#include <SbModel.h>

#include "RuleWizardSandboxReadPage.h"

RuleWizardSandboxReadPage::RuleWizardSandboxReadPage(wxWindow *parent,
    RuleWizardHistory *history) : RuleWizardSandboxPermissionPageBase(parent)
{
	history_ = history;

	switch (history_->getSandboxPermission(SbEntry::READ)) {
	case RuleWizardHistory::PERM_ALLOW_ALL:
		allowAllRadioButton->SetValue(true);
		break;
	case RuleWizardHistory::PERM_RESTRICT_USER:
		restrictedRadioButton->SetValue(true);
		break;
	case RuleWizardHistory::PERM_RESTRICT_DEFAULT:
		/* FALLTHROUGH */
	default:
		defaultRadioButton->SetValue(true);
		break;
	}

	defaultRadioButton->Enable(
	    history_->getSandboxFileList()->canAssignDefaults());

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardSandboxReadPage::onPageChanged),
	    NULL, this);
}

void
RuleWizardSandboxReadPage::onPageChanged(wxWizardEvent &)
{
	wxString text;

	text.Printf(_("Choose permissions of application \"%ls\"\n"
	    "of READING file access:"), history_->getProgram().c_str());
	questionLabel->SetLabel(text);

	updateNavi();
}

void
RuleWizardSandboxReadPage::onAllowAllRadioButton(wxCommandEvent &)
{
	history_->setSandboxPermission(SbEntry::READ,
	    RuleWizardHistory::PERM_ALLOW_ALL);
	updateNavi();
}

void
RuleWizardSandboxReadPage::onDefaultRadioButton(wxCommandEvent &)
{
	history_->setSandboxPermission(SbEntry::READ,
	    RuleWizardHistory::PERM_RESTRICT_DEFAULT);
	updateNavi();
}

void
RuleWizardSandboxReadPage::onRestrictedRadioButton(wxCommandEvent &)
{
	history_->setSandboxPermission(SbEntry::READ,
	    RuleWizardHistory::PERM_RESTRICT_USER);
	updateNavi();
}

void
RuleWizardSandboxReadPage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, false);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillAlfNavi(this, naviSizer, false);
	history_->fillSandboxNavi(this, naviSizer, true);
	Layout();
	Refresh();
}

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

#include "RuleWizardAlfClientPage.h"

RuleWizardAlfClientPage::RuleWizardAlfClientPage(wxWindow *parent,
    RuleWizardHistory *history) : RuleWizardAlfPermissionPageBase(parent)
{
	history_ = history;

	switch (history_->getAlfClientPermission()) {
	case RuleWizardHistory::PERM_ALLOW_ALL:
		yesRadioButton->SetValue(true);
		break;
	case RuleWizardHistory::PERM_RESTRICT_DEFAULT:
		defaultRadioButton->SetValue(true);
		break;
	case RuleWizardHistory::PERM_RESTRICT_USER:
		restrictedRadioButton->SetValue(true);
		break;
	case RuleWizardHistory::PERM_DENY_ALL:
		/* FALLTHROUGH */
	default:
		noRadioButton->SetValue(true);
		break;
	}

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardAlfClientPage::onPageChanged),
	    NULL, this);
}

void
RuleWizardAlfClientPage::onPageChanged(wxWizardEvent &)
{
	wxString text;

	text.Printf(_("Allow the application \"%ls\" access to\n"
	    "network services (client functionality)?"),
	    history_->getProgram().c_str());
	questionLabel->SetLabel(text);

	updateNavi();
}

void
RuleWizardAlfClientPage::onYesRadioButton(wxCommandEvent &)
{
	history_->setAlfClientPermission(RuleWizardHistory::PERM_ALLOW_ALL);
	updateNavi();
}

void
RuleWizardAlfClientPage::onDefaultRadioButton(wxCommandEvent &)
{
	history_->setAlfClientPermission(
	    RuleWizardHistory::PERM_RESTRICT_DEFAULT);
	updateNavi();
}

void
RuleWizardAlfClientPage::onRestrictedRadioButton(wxCommandEvent &)
{
	history_->setAlfClientPermission(
	    RuleWizardHistory::PERM_RESTRICT_USER);
	updateNavi();
}

void
RuleWizardAlfClientPage::onNoRadioButton(wxCommandEvent &)
{
	history_->setAlfClientPermission(RuleWizardHistory::PERM_DENY_ALL);
	updateNavi();
}

void
RuleWizardAlfClientPage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, false);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillAlfNavi(this, naviSizer, true);
	history_->fillSandboxNavi(this, naviSizer, false);
	Layout();
	Refresh();
}

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

#include "RuleWizardAlfOverwritePage.h"

#include "main.h"
#include "AppPolicy.h"
#include "PolicyCtrl.h"
#include "PolicyRuleSet.h"

RuleWizardAlfOverwritePage::RuleWizardAlfOverwritePage(wxWindow *parent,
    RuleWizardHistory *history) : RuleWizardOverwritePolicyPageBase(parent)
{
	wxString	 text;
	wxIcon		*icon;

	history_ = history;

	text = wxT("Application Level Firewall settings:");
	headLineLabel->SetLabel(text);

	text = _("Alf polices already exists and you have to choose"
	    " between two options: keep or overwrite.\n\n"
	    "Choose \"No\" if you don't want to alter the existing policies."
	    " You'll skip the alf section and proceed to sandbox page.\n\n"
	    "Choose \"Yes\" if you want to erase all policies of this"
	    " application. Thereby you may specify new alf policies.");
	helpLabel->SetLabel(text);

	icon = wxGetApp().loadIcon(wxT("General_problem_48.png"));
	alertIcon->SetIcon(*icon);

	text = _("For this application\nalf policies\nalready exists.");
	alertLabel->SetLabel(text);

	if (history_->shallOverwriteAlfPolicy() ==
	    RuleWizardHistory::OVERWRITE_YES) {
		yesRadioButton->SetValue(true);
	} else {
		noRadioButton->SetValue(true);
	}

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardAlfOverwritePage::onPageChanged),
	    NULL, this);
}

void
RuleWizardAlfOverwritePage::onPageChanged(wxWizardEvent &)
{
	fillPolicy();
	updateNavi();
}

void
RuleWizardAlfOverwritePage::onYesRadioButton(wxCommandEvent &)
{
	history_->setOverwriteAlfPolicy(RuleWizardHistory::OVERWRITE_YES);
	updateNavi();
}

void
RuleWizardAlfOverwritePage::onNoRadioButton(wxCommandEvent &)
{
	history_->setOverwriteAlfPolicy(RuleWizardHistory::OVERWRITE_NO);
	updateNavi();
}

void
RuleWizardAlfOverwritePage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, false);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillAlfNavi(this, naviSizer, true);
	history_->fillSandboxNavi(this, naviSizer, false);
	Layout();
	Refresh();
}

void
RuleWizardAlfOverwritePage::fillPolicy(void)
{
	wxString	 text;
	AppPolicy	*app;
	PolicyCtrl	*policyCtrl;
	PolicyRuleSet	*ruleSet;

	text.Printf(_("Policy of \"%ls\":"), history_->getProgram().c_str());
	policyLabel->SetLabel(text);
	text = wxEmptyString;

	policyCtrl = PolicyCtrl::getInstance();
	ruleSet = policyCtrl->getRuleSet(policyCtrl->getUserId());
	if (ruleSet != NULL) {
		app = ruleSet->searchAlfAppPolicy(history_->getProgram());
		if (app != NULL) {
			text = app->toString();
		}
	}

	policyTextCtrl->Clear();
	policyTextCtrl->AppendText(text);
}

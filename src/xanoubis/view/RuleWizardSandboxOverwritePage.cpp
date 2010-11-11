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

#include "AnIconList.h"
#include "PolicyCtrl.h"
#include "PolicyRuleSet.h"
#include "RuleWizardSandboxOverwritePage.h"

RuleWizardSandboxOverwritePage::RuleWizardSandboxOverwritePage(wxWindow *parent,
    RuleWizardHistory *history) : RuleWizardOverwritePolicyPageBase(parent)
{
	wxString	 text;
	wxIcon		*icon;

	history->get();
	history_ = history;

	text = wxT("Sandbox settings:");
	headLineLabel->SetLabel(text);

	text = _("Sandbox policies already exist and you have to choose"
	    " between two options: keep or overwrite.\n\n"
	    "Choose \"No\" if you don't want to alter the existing policies."
	    " You'll skip the sandbox section and proceed to final page.\n\n"
	    "Choose \"Yes\" if you want to erase all policies of this"
	    " application. Thereby you may specify new sandbox policies.");
	helpLabel->SetLabel(text);

	icon = AnIconList::instance()->getIcon(AnIconList::ICON_PROBLEM_48);
	alertIcon->SetIcon(*icon);

	text = _("For this application\nsandbox policies\nalready exist.");
	alertLabel->SetLabel(text);

	if (history_->shallOverwriteSandboxPolicy() ==
	    RuleWizardHistory::OVERWRITE_YES) {
		yesRadioButton->SetValue(true);
	} else {
		noRadioButton->SetValue(true);
	}

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardSandboxOverwritePage::onPageChanged),
	    NULL, this);
}

RuleWizardSandboxOverwritePage::~RuleWizardSandboxOverwritePage(void)
{
	RuleWizardHistory::put(history_);
}

void
RuleWizardSandboxOverwritePage::onPageChanged(wxWizardEvent &)
{
	fillPolicy();
	updateNavi();
}

void
RuleWizardSandboxOverwritePage::onYesRadioButton(wxCommandEvent &)
{
	/*
	 * Must call setSandbox(...), too because the page that selects
	 * this value is skipped if sandbox policies exist.
	 */
	history_->setOverwriteSandboxPolicy(RuleWizardHistory::OVERWRITE_YES);
	history_->setSandbox(RuleWizardHistory::PERM_RESTRICT_USER);
	updateNavi();
}

void
RuleWizardSandboxOverwritePage::onNoRadioButton(wxCommandEvent &)
{
	/*
	 * Must call setSandbox(...), too because the page that selects
	 * this value is skipped if sandbox policies exist.
	 */
	history_->setOverwriteSandboxPolicy(RuleWizardHistory::OVERWRITE_NO);
	history_->setSandbox(RuleWizardHistory::PERM_NONE);
	updateNavi();
}

void
RuleWizardSandboxOverwritePage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, false);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillSandboxNavi(this, naviSizer, true);
	Layout();
	Refresh();
}

void
RuleWizardSandboxOverwritePage::fillPolicy(void)
{
	wxString	 text;
	AppPolicy	*app;
	PolicyCtrl	*policyCtrl;
	PolicyRuleSet	*ruleSet;

	text.Printf(_("Policy of \"%ls\":"), history_->getProgram().c_str());
	policyLabel->SetLabel(text);
	text = wxEmptyString;

	policyCtrl = PolicyCtrl::instance();
	ruleSet = policyCtrl->getRuleSet(policyCtrl->getUserId());
	if (ruleSet != NULL) {
		app = ruleSet->searchSandboxAppPolicy(history_->getProgram());
		if (app != NULL) {
			text = app->toString();
		}
	}

	policyTextCtrl->Clear();
	policyTextCtrl->AppendText(text);
}

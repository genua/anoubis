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

#include "DlgRuleEditorFilterActionPage.h"

#include "PolicyRuleSet.h"
#include "AlfFilterPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "SbAccessFilterPolicy.h"
#include "DefaultFilterPolicy.h"
#include "SfsDefaultFilterPolicy.h"

DlgRuleEditorFilterActionPage::DlgRuleEditorFilterActionPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorPage(),
    DlgRuleEditorFilterActionPageBase(parent, id, pos, size, style)
{
}

void
DlgRuleEditorFilterActionPage::update(Subject *subject)
{
	if (subject == policy_) {
		/* This is our policy. */
		showAction();
		showLog();
	}
}

void
DlgRuleEditorFilterActionPage::select(FilterPolicy *policy)
{
	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy)) ||
	    policy->IsKindOf(CLASSINFO(AlfCapabilityFilterPolicy)) ||
	    policy->IsKindOf(CLASSINFO(SbAccessFilterPolicy)) ||
	    policy->IsKindOf(CLASSINFO(DefaultFilterPolicy)) ||
	    policy->IsKindOf(CLASSINFO(SfsDefaultFilterPolicy)) ) {
	    	PolicyRuleSet	*ruleset = policy->getParentRuleSet();
		policy_ = policy;
		DlgRuleEditorPage::select(policy);
		if (ruleset && ruleset->isAdmin() && geteuid() != 0) {
			askRadioButton->Disable();
		} else {
			askRadioButton->Enable();
		}
		Show();
	}
}

void
DlgRuleEditorFilterActionPage::deselect(void)
{
	policy_ = NULL;
	DlgRuleEditorPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterActionPage::showAction(void)
{
	if (policy_ == NULL) {
		return;
	}

	switch (policy_->getActionNo()) {
	case APN_ACTION_ALLOW:
		allowRadioButton->SetValue(true);
		break;
	case APN_ACTION_DENY:
		denyRadioButton->SetValue(true);
		break;
	case APN_ACTION_ASK:
		askRadioButton->SetValue(true);
		break;
	default:
		allowRadioButton->SetValue(false);
		denyRadioButton->SetValue(false);
		askRadioButton->SetValue(false);
	}
}

void
DlgRuleEditorFilterActionPage::showLog(void)
{
	if (policy_ == NULL) {
		return;
	}

	switch (policy_->getLogNo()) {
	case APN_LOG_NONE:
		noneRadioButton->SetValue(true);
		break;
	case APN_LOG_NORMAL:
		normalRadioButton->SetValue(true);
		break;
	case APN_LOG_ALERT:
		alertRadioButton->SetValue(true);
		break;
	default:
		noneRadioButton->SetValue(false);
		normalRadioButton->SetValue(false);
		alertRadioButton->SetValue(false);
	}
}

void
DlgRuleEditorFilterActionPage::onAllowRadioButton(wxCommandEvent &)
{
	if (policy_ != NULL) {
		policy_->setActionNo(APN_ACTION_ALLOW);
	}
}

void
DlgRuleEditorFilterActionPage::onDenyRadioButton(wxCommandEvent &)
{
	if (policy_ != NULL) {
		policy_->setActionNo(APN_ACTION_DENY);
	}
}

void
DlgRuleEditorFilterActionPage::onAskRadioButton(wxCommandEvent &)
{
	if (policy_ != NULL) {
		policy_->setActionNo(APN_ACTION_ASK);
	}
}

void
DlgRuleEditorFilterActionPage::onNoneRadioButton(wxCommandEvent &)
{
	if (policy_ != NULL) {
		policy_->setLogNo(APN_LOG_NONE);
	}
}

void
DlgRuleEditorFilterActionPage::onNormalRadioButton(wxCommandEvent &)
{
	if (policy_ != NULL) {
		policy_->setLogNo(APN_LOG_NORMAL);
	}
}

void
DlgRuleEditorFilterActionPage::onAlertRadioButton(wxCommandEvent &)
{
	if (policy_ != NULL) {
		policy_->setLogNo(APN_LOG_ALERT);
	}
}

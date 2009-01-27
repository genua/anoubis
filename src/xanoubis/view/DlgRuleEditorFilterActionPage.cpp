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

#include "AlfFilterPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "SbAccessFilterPolicy.h"
#include "DefaultFilterPolicy.h"
#include "SfsDefaultFilterPolicy.h"

DlgRuleEditorFilterActionPage::DlgRuleEditorFilterActionPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorFilterPage(),
    DlgRuleEditorFilterActionPageBase(parent, id, pos, size, style)
{
}

void
DlgRuleEditorFilterActionPage::update(Subject *subject)
{
	if (subject == policy_) {
		/* This is our policy. */
		showAction(policy_->getActionNo());
		showLog(policy_->getLogNo());
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
		DlgRuleEditorFilterPage::select(policy);
		Show();
	}
}

void
DlgRuleEditorFilterActionPage::deselect(void)
{
	DlgRuleEditorFilterPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterActionPage::clear(void)
{
	showAction(-1);
	showLog(-1);
}

void
DlgRuleEditorFilterActionPage::showAction(int action)
{
	actionLabel->Enable();
	allowRadioButton->Enable();
	denyRadioButton->Enable();
	askRadioButton->Enable();

	switch (action) {
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
		denyRadioButton->SetValue(true);
		actionLabel->Disable();
		allowRadioButton->Disable();
		denyRadioButton->Disable();
		askRadioButton->Disable();
	}
}

void
DlgRuleEditorFilterActionPage::showLog(int log)
{
	logLabel->Enable();
	noneRadioButton->Enable();
	normalRadioButton->Enable();
	alertRadioButton->Enable();

	switch (log) {
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
		logLabel->Disable();
		noneRadioButton->Disable();
		noneRadioButton->SetValue(false);
		normalRadioButton->Disable();
		normalRadioButton->SetValue(false);
		alertRadioButton->Disable();
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

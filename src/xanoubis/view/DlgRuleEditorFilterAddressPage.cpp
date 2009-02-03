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

#include "DlgRuleEditorFilterAddressPage.h"

DlgRuleEditorFilterAddressPage::DlgRuleEditorFilterAddressPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorFilterPage(),
    DlgRuleEditorFilterAddressPageBase(parent, id, pos, size, style)
{
}

void
DlgRuleEditorFilterAddressPage::update(Subject *subject)
{
	if (subject == policy_) {
		/* This is our policy. */
		showSource();
		showDestination();
	}
}

void
DlgRuleEditorFilterAddressPage::select(FilterPolicy *policy)
{
	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		policy_ = wxDynamicCast(policy, AlfFilterPolicy);
		DlgRuleEditorFilterPage::select(policy);
		Show();
	}
}

void
DlgRuleEditorFilterAddressPage::deselect(void)
{
	policy_ = NULL;
	DlgRuleEditorFilterPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterAddressPage::showSource(void)
{
	if (policy_ != NULL) {
		sourceAddressTextCtrl->ChangeValue(policy_->getFromHostName());
		sourcePortTextCtrl->ChangeValue(policy_->getFromPortName());
	}
}

void
DlgRuleEditorFilterAddressPage::showDestination(void)
{
	if (policy_ != NULL) {
		destinationAddressTextCtrl->ChangeValue(
		    policy_->getToHostName());
		destinationPortTextCtrl->ChangeValue(policy_->getToPortName());
	}
}

void
DlgRuleEditorFilterAddressPage::onSourceAddressTextEnter(wxCommandEvent & event)
{
	if (policy_ != NULL) {
		policy_->setFromHostName(event.GetString());
	}
}

void
DlgRuleEditorFilterAddressPage::onSourceAddressTextKillFocus(wxFocusEvent &)
{
	if (policy_ != NULL) {
		policy_->setFromHostName(sourceAddressTextCtrl->GetValue());
	}
}

void
DlgRuleEditorFilterAddressPage::onSourcePortTextEnter(wxCommandEvent & event)
{
	if (policy_ != NULL) {
		policy_->setFromPortName(event.GetString());
	}
}

void
DlgRuleEditorFilterAddressPage::onSourcePortTextKillFocus(wxFocusEvent &)
{
	if (policy_ != NULL) {
		policy_->setFromPortName(sourcePortTextCtrl->GetValue());
	}
}

void
DlgRuleEditorFilterAddressPage::onDestinationAddressTextEnter(
    wxCommandEvent & event)
{
	if (policy_ != NULL) {
		policy_->setToHostName(event.GetString());
	}
}

void
DlgRuleEditorFilterAddressPage::onDestinationAddressTextKillFocus(
    wxFocusEvent &)
{
	if (policy_ != NULL) {
		policy_->setToHostName(destinationAddressTextCtrl->GetValue());
	}
}

void
DlgRuleEditorFilterAddressPage::onDestinationPortTextEnter(
    wxCommandEvent & event)
{
	if (policy_ != NULL) {
		policy_->setToPortName(event.GetString());
	}
}

void
DlgRuleEditorFilterAddressPage::onDestinationPortTextKillFocus(
    wxFocusEvent &)
{
	if (policy_ != NULL) {
		policy_->setToPortName(destinationPortTextCtrl->GetValue());
	}
}

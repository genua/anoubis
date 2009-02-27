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
    : DlgRuleEditorPage(),
    DlgRuleEditorFilterAddressPageBase(parent, id, pos, size, style)
{
	filterPolicy_ = NULL;
}

void
DlgRuleEditorFilterAddressPage::update(Subject *subject)
{
	if (subject == filterPolicy_) {
		/* This is our policy. */
		showSource();
		showDestination();
	}
}

void
DlgRuleEditorFilterAddressPage::select(Policy *policy)
{
	if (policy->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		filterPolicy_ = wxDynamicCast(policy, AlfFilterPolicy);
		DlgRuleEditorPage::select(policy);
		Enable(enable_);
		Show();
	}
}

void
DlgRuleEditorFilterAddressPage::deselect(void)
{
	filterPolicy_ = NULL;
	DlgRuleEditorPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterAddressPage::showSource(void)
{
	if (filterPolicy_ != NULL) {
		sourceAddressTextCtrl->ChangeValue(
		    filterPolicy_->getFromHostName());
		sourcePortTextCtrl->ChangeValue(
		    filterPolicy_->getFromPortName());
	}
}

void
DlgRuleEditorFilterAddressPage::showDestination(void)
{
	if (filterPolicy_ != NULL) {
		destinationAddressTextCtrl->ChangeValue(
		    filterPolicy_->getToHostName());
		destinationPortTextCtrl->ChangeValue(
		    filterPolicy_->getToPortName());
	}
}

void
DlgRuleEditorFilterAddressPage::onSourceAddressTextEnter(wxCommandEvent & event)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setFromHostName(event.GetString());
	}
}

void
DlgRuleEditorFilterAddressPage::onSourceAddressTextKillFocus(wxFocusEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setFromHostName(
		    sourceAddressTextCtrl->GetValue());
	}
}

void
DlgRuleEditorFilterAddressPage::onSourcePortTextEnter(wxCommandEvent & event)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setFromPortName(event.GetString());
	}
}

void
DlgRuleEditorFilterAddressPage::onSourcePortTextKillFocus(wxFocusEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setFromPortName(sourcePortTextCtrl->GetValue());
	}
}

void
DlgRuleEditorFilterAddressPage::onDestinationAddressTextEnter(
    wxCommandEvent & event)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setToHostName(event.GetString());
	}
}

void
DlgRuleEditorFilterAddressPage::onDestinationAddressTextKillFocus(
    wxFocusEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setToHostName(
		    destinationAddressTextCtrl->GetValue());
	}
}

void
DlgRuleEditorFilterAddressPage::onDestinationPortTextEnter(
    wxCommandEvent & event)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setToPortName(event.GetString());
	}
}

void
DlgRuleEditorFilterAddressPage::onDestinationPortTextKillFocus(
    wxFocusEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setToPortName(
		    destinationPortTextCtrl->GetValue());
	}
}

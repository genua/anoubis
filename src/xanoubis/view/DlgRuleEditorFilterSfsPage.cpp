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

#include "DlgRuleEditorFilterSfsPage.h"

DlgRuleEditorFilterSfsPage::DlgRuleEditorFilterSfsPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorFilterPage(),
    DlgRuleEditorFilterSfsPageBase(parent, id, pos, size, style)
{
}

void
DlgRuleEditorFilterSfsPage::update(Subject *subject)
{
	if (subject == policy_) {
		/* This is our policy. */
		showValid();
		showInvalid();
		showUnknown();
	}
}

void
DlgRuleEditorFilterSfsPage::select(FilterPolicy *policy)
{
	if (policy->IsKindOf(CLASSINFO(SfsFilterPolicy))) {
		policy_ = wxDynamicCast(policy, SfsFilterPolicy);
		DlgRuleEditorFilterPage::select(policy);
		Show();
	}
}

void
DlgRuleEditorFilterSfsPage::deselect(void)
{
	policy_ = NULL;
	DlgRuleEditorFilterPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterSfsPage::showValid(void)
{
	if (policy_ != NULL) {
		validActionRadioBox->SetSelection(policy_->getValidActionNo());
		validLogRadioBox->SetSelection(policy_->getValidLogNo());
	}
}

void
DlgRuleEditorFilterSfsPage::showInvalid(void)
{
	if (policy_ != NULL) {
		invalidActionRadioBox->SetSelection(
		    policy_->getInvalidActionNo());
		invalidLogRadioBox->SetSelection(policy_->getInvalidLogNo());
	}
}

void
DlgRuleEditorFilterSfsPage::showUnknown(void)
{
	if (policy_ != NULL) {
		unknownActionRadioBox->SetSelection(
		    policy_->getUnknownActionNo());
		unknownLogRadioBox->SetSelection(policy_->getUnknownLogNo());
	}
}

void
DlgRuleEditorFilterSfsPage::onValidActionRadioBox(wxCommandEvent & event)
{
	if (policy_ != NULL) {
		policy_->setValidAction((int)event.GetSelection());
	}
}

void
DlgRuleEditorFilterSfsPage::onValidLogRadioBox(wxCommandEvent & event)
{
	if (policy_ != NULL) {
		policy_->setValidLog((int)event.GetSelection());
	}
}

void
DlgRuleEditorFilterSfsPage::onInvalidActionRadioBox(wxCommandEvent & event)
{
	if (policy_ != NULL) {
		policy_->setInvalidAction((int)event.GetSelection());
	}
}

void
DlgRuleEditorFilterSfsPage::onInvalidLogRadioBox(wxCommandEvent & event)
{
	if (policy_ != NULL) {
		policy_->setInvalidLog((int)event.GetSelection());
	}
}

void
DlgRuleEditorFilterSfsPage::onUnknownActionRadioBox(wxCommandEvent & event)
{
	if (policy_ != NULL) {
		policy_->setUnknownAction((int)event.GetSelection());
	}
}

void
DlgRuleEditorFilterSfsPage::onUnknownLogRadioBox(wxCommandEvent & event)
{
	if (policy_ != NULL) {
		policy_->setUnknownLog((int)event.GetSelection());
	}
}

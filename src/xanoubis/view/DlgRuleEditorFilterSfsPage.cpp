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
    : DlgRuleEditorPage(),
    DlgRuleEditorFilterSfsPageBase(parent, id, pos, size, style)
{
	filterPolicy_ = NULL;
}

void
DlgRuleEditorFilterSfsPage::update(Subject *subject)
{
	if (subject == filterPolicy_) {
		/* This is our policy. */
		showValid();
		showInvalid();
		showUnknown();
	}
}

void
DlgRuleEditorFilterSfsPage::select(Policy *policy)
{
	if (policy->IsKindOf(CLASSINFO(SfsFilterPolicy))) {
		filterPolicy_ = wxDynamicCast(policy, SfsFilterPolicy);
		DlgRuleEditorPage::select(policy);
		Enable(enable_);
		Show();
	}
}

void
DlgRuleEditorFilterSfsPage::deselect(void)
{
	filterPolicy_ = NULL;
	DlgRuleEditorPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterSfsPage::showValid(void)
{
	if (filterPolicy_ != NULL) {
		validActionRadioBox->SetSelection(
		    filterPolicy_->getValidActionNo());
		validLogRadioBox->SetSelection(
		    filterPolicy_->getValidLogNo());
	}
}

void
DlgRuleEditorFilterSfsPage::showInvalid(void)
{
	if (filterPolicy_ != NULL) {
		invalidActionRadioBox->SetSelection(
		    filterPolicy_->getInvalidActionNo());
		invalidLogRadioBox->SetSelection(
		    filterPolicy_->getInvalidLogNo());
	}
}

void
DlgRuleEditorFilterSfsPage::showUnknown(void)
{
	if (filterPolicy_ != NULL) {
		unknownActionRadioBox->SetSelection(
		    filterPolicy_->getUnknownActionNo());
		unknownLogRadioBox->SetSelection(
		    filterPolicy_->getUnknownLogNo());
	}
}

void
DlgRuleEditorFilterSfsPage::onValidActionRadioBox(wxCommandEvent & event)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setValidAction((int)event.GetSelection());
	}
}

void
DlgRuleEditorFilterSfsPage::onValidLogRadioBox(wxCommandEvent & event)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setValidLog((int)event.GetSelection());
	}
}

void
DlgRuleEditorFilterSfsPage::onInvalidActionRadioBox(wxCommandEvent & event)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setInvalidAction((int)event.GetSelection());
	}
}

void
DlgRuleEditorFilterSfsPage::onInvalidLogRadioBox(wxCommandEvent & event)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setInvalidLog((int)event.GetSelection());
	}
}

void
DlgRuleEditorFilterSfsPage::onUnknownActionRadioBox(wxCommandEvent & event)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setUnknownAction((int)event.GetSelection());
	}
}

void
DlgRuleEditorFilterSfsPage::onUnknownLogRadioBox(wxCommandEvent & event)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setUnknownLog((int)event.GetSelection());
	}
}

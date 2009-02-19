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

#include "DlgRuleEditorFilterCapabilityPage.h"

DlgRuleEditorFilterCapabilityPage::DlgRuleEditorFilterCapabilityPage(
    wxWindow *parent, wxWindowID id, const wxPoint & pos, const wxSize & size,
    long style) : DlgRuleEditorPage(),
    DlgRuleEditorFilterCapabilityPageBase(parent, id, pos, size, style)
{
	filterPolicy_ = NULL;
}

void
DlgRuleEditorFilterCapabilityPage::update(Subject *subject)
{
	if (subject == filterPolicy_) {
		/* This is not our policy! */
		showCapability();
	}
}

void
DlgRuleEditorFilterCapabilityPage::select(Policy *policy)
{
	if (policy->IsKindOf(CLASSINFO(AlfCapabilityFilterPolicy))) {
		filterPolicy_ = wxDynamicCast(policy,
		    AlfCapabilityFilterPolicy);
		DlgRuleEditorPage::select(policy);
		Show();
	}
}

void
DlgRuleEditorFilterCapabilityPage::deselect(void)
{
	filterPolicy_ = NULL;
	DlgRuleEditorPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterCapabilityPage::showCapability(void)
{
	if (filterPolicy_ == NULL) {
		return;
	}

	switch (filterPolicy_->getCapabilityTypeNo()) {
	case APN_ALF_CAPRAW:
		rawRadioButton->SetValue(true);
		break;
	case APN_ALF_CAPOTHER:
		otherRadioButton->SetValue(true);
		break;
	case APN_ALF_CAPALL:
		allRadioButton->SetValue(true);
		break;
	default:
		rawRadioButton->SetValue(false);
		otherRadioButton->SetValue(false);
		allRadioButton->SetValue(false);
		break;
	}
}

void
DlgRuleEditorFilterCapabilityPage::onRawRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setCapabilityTypeNo(APN_ALF_CAPRAW);
	}
}

void
DlgRuleEditorFilterCapabilityPage::onOtherRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setCapabilityTypeNo(APN_ALF_CAPOTHER);
	}
}

void
DlgRuleEditorFilterCapabilityPage::onAllRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setCapabilityTypeNo(APN_ALF_CAPALL);
	}
}

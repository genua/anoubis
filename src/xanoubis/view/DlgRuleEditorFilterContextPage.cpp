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

#include "DlgRuleEditorFilterContextPage.h"
#include "DlgRuleEditorAppPage.h"

DlgRuleEditorFilterContextPage::DlgRuleEditorFilterContextPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorPage(),
    DlgRuleEditorFilterContextPageBase(parent, id, pos, size, style)
{
	filterPolicy_ = NULL;
}

void
DlgRuleEditorFilterContextPage::update(Subject *subject)
{
	if (subject == filterPolicy_) {
		/* This is not our policy! */
		showContextType();
	}
}

void
DlgRuleEditorFilterContextPage::select(Policy *policy)
{
	if (dynamic_cast<ContextFilterPolicy*>(policy)) {
		filterPolicy_ = dynamic_cast<ContextFilterPolicy*>(policy);
		DlgRuleEditorPage::select(policy);
		appPage->setBinaryIndex(binaryIndex_);
		appPage->select(policy);
		Enable(enable_);
		Show();
	}
}

void
DlgRuleEditorFilterContextPage::deselect(void)
{
	filterPolicy_ = NULL;
	appPage->deselect();
	DlgRuleEditorPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterContextPage::setBinaryIndex(unsigned int index)
{
	binaryIndex_ = index;
}

void
DlgRuleEditorFilterContextPage::showContextType(void)
{
	if (filterPolicy_ == NULL) {
		return;
	}

	switch (filterPolicy_->getContextTypeNo()) {
	case APN_CTX_NEW:
		newRadioButton->SetValue(true);
		break;
	case APN_CTX_OPEN:
		openRadioButton->SetValue(true);
		break;
	default:
		newRadioButton->SetValue(false);
		openRadioButton->SetValue(false);
		break;
	}
}

void
DlgRuleEditorFilterContextPage::onNewRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setContextTypeNo(APN_CTX_NEW);
	}
}

void
DlgRuleEditorFilterContextPage::onOpenRadioButton(wxCommandEvent &)
{
	if (filterPolicy_ != NULL) {
		filterPolicy_->setContextTypeNo(APN_CTX_OPEN);
	}
}

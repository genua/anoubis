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

#include "DlgRuleEditorFilterPermissionPage.h"

DlgRuleEditorFilterPermissionPage::DlgRuleEditorFilterPermissionPage(
    wxWindow *parent, wxWindowID id, const wxPoint & pos, const wxSize & size,
    long style) : DlgRuleEditorPage(),
    DlgRuleEditorFilterPermissionPageBase(parent, id, pos, size, style)
{
	filterPolicy_ = NULL;
}

void
DlgRuleEditorFilterPermissionPage::update(Subject *subject)
{
	if (subject == filterPolicy_) {
		/* This is not our policy! */
		showPermission();
	}
}

void
DlgRuleEditorFilterPermissionPage::select(Policy *policy)
{
	if (dynamic_cast<SbAccessFilterPolicy*>(policy)) {
		filterPolicy_ = dynamic_cast<SbAccessFilterPolicy*>(policy);
		DlgRuleEditorPage::select(policy);
		Enable(enable_);
		Show();
	}
}

void
DlgRuleEditorFilterPermissionPage::deselect(void)
{
	filterPolicy_ = NULL;
	DlgRuleEditorPage::deselect();
	Hide();
}

void
DlgRuleEditorFilterPermissionPage::showPermission(void)
{
	unsigned int mask;

	mask = filterPolicy_->getAccessMaskNo();
	if ((mask & APN_SBA_READ) != 0) {
		readCheckBox->SetValue(true);
	}
	if ((mask & APN_SBA_WRITE) != 0) {
		writeCheckBox->SetValue(true);
	}
	if ((mask & APN_SBA_EXEC) != 0) {
		executeCheckBox->SetValue(true);
	}
}

void
DlgRuleEditorFilterPermissionPage::onReadCheckBox(wxCommandEvent & event)
{
	unsigned int mask;

	if (filterPolicy_ != NULL) {
		mask  = filterPolicy_->getAccessMaskNo();
		if  (event.IsChecked()) {
			mask |= APN_SBA_READ;
		} else {
			mask &= ~APN_SBA_READ;
		}
		filterPolicy_->setAccessMask(mask);
	}
}

void
DlgRuleEditorFilterPermissionPage::onWriteCheckBox(wxCommandEvent & event)
{
	unsigned int mask;

	if (filterPolicy_ != NULL) {
		mask  = filterPolicy_->getAccessMaskNo();
		if  (event.IsChecked()) {
			mask |= APN_SBA_WRITE;
		} else {
			mask &= ~APN_SBA_WRITE;
		}
		filterPolicy_->setAccessMask(mask);
	}
}

void
DlgRuleEditorFilterPermissionPage::onExecuteCheckBox(wxCommandEvent & event)
{
	unsigned int mask;

	if (filterPolicy_ != NULL) {
		mask  = filterPolicy_->getAccessMaskNo();
		if (event.IsChecked()) {
			mask |= APN_SBA_EXEC;
		} else {
			mask &= ~APN_SBA_EXEC;
		}
		filterPolicy_->setAccessMask(mask);
	}
}

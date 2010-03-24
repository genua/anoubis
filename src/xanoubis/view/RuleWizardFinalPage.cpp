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

#include "RuleWizardFinalPage.h"
#include "JobCtrl.h"

RuleWizardFinalPage::RuleWizardFinalPage(wxWindow *parent,
    RuleWizardHistory *history) : RuleWizardFinalPageBase(parent)
{
	history->get();
	history_ = history;

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardFinalPage::onPageChanged),
	    NULL, this);
}

RuleWizardFinalPage::~RuleWizardFinalPage(void)
{
	RuleWizardHistory::put(history_);
}

void
RuleWizardFinalPage::onPageChanged(wxWizardEvent &)
{
	bool connected;

	connected = JobCtrl::getInstance()->isConnected();

	history_->setActivatePolicy(connected);
	activatePolicyCheckbox->SetValue(connected);
	activatePolicyCheckbox->Enable(connected);

	updateNavi();
}

void
RuleWizardFinalPage::onActivatePolicyCheckBox(wxCommandEvent & event)
{
	history_->setActivatePolicy(event.GetInt());
	updateNavi();
}

void
RuleWizardFinalPage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, false);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillAlfNavi(this, naviSizer, false);
	history_->fillSandboxNavi(this, naviSizer, false);
	Layout();
	Refresh();
}

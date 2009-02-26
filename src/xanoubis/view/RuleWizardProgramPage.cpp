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

#include <wx/filedlg.h>
#include <wx/filename.h>

#include "RuleWizardProgramPage.h"

RuleWizardProgramPage::RuleWizardProgramPage(wxWindow *parent,
    RuleWizardHistory *history) : RuleWizardProgramPageBase(parent)
{
	history_ = history;

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGING,
	    wxWizardEventHandler(RuleWizardProgramPage::onPageChanging),
	    NULL, this);
	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardProgramPage::onPageChanged),
	    NULL, this);
}

void
RuleWizardProgramPage::onPageChanging(wxWizardEvent &event)
{
	/* If no program was set, we'll not proceed to the next page. */
	if (history_->getProgram().IsEmpty()) {
		event.Veto();
	}
}

void
RuleWizardProgramPage::onPageChanged(wxWizardEvent &)
{
	updateNavi();
}

void
RuleWizardProgramPage::onProgramTextKillFocus(wxFocusEvent &)
{
	history_->setProgram(programTextCtrl->GetValue());
	updateNavi();
}

void
RuleWizardProgramPage::onProgramTextEnter(wxCommandEvent &event)
{
	history_->setProgram(event.GetString());
	updateNavi();
}

void
RuleWizardProgramPage::onPickButton(wxCommandEvent &)
{
	wxFileName	defaultPath;
	wxFileDialog	fileDlg(this);

	defaultPath.Assign(history_->getProgram());

	wxBeginBusyCursor();
	fileDlg.SetDirectory(defaultPath.GetPath());
	fileDlg.SetFilename(defaultPath.GetFullName());
	fileDlg.SetWildcard(wxT("*"));
	wxEndBusyCursor();

	if (fileDlg.ShowModal() == wxID_OK) {
		history_->setProgram(fileDlg.GetPath());
		programTextCtrl->ChangeValue(fileDlg.GetPath());
		updateNavi();
	}
}

void
RuleWizardProgramPage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, true);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillAlfNavi(this, naviSizer, false);
	Layout();
	Refresh();
}

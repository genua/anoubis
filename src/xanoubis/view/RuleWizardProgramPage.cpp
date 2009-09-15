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

#include <cstdlib>

#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

#include "AnPickFromFs.h"
#include "RuleWizardProgramPage.h"

RuleWizardProgramPage::RuleWizardProgramPage(wxWindow *parent,
    RuleWizardHistory *history) : Observer(NULL),
    RuleWizardProgramPageBase(parent)
{
	history_ = history;

	addSubject(programPicker);
	programPicker->setTitle(wxT("")); /* Title shown as extra label. */
	programPicker->setMode(AnPickFromFs::MODE_FILE);

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGING,
	    wxWizardEventHandler(RuleWizardProgramPage::onPageChanging),
	    NULL, this);
	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardProgramPage::onPageChanged),
	    NULL, this);
}

RuleWizardProgramPage::~RuleWizardProgramPage(void)
{
}

void
RuleWizardProgramPage::update(Subject *subject)
{
	if (subject == programPicker) {
		setProgram(programPicker->getFileName());
		updateNavi();
	}
}

void
RuleWizardProgramPage::updateDelete(Subject *)
{
	removeSubject(programPicker);
}

void
RuleWizardProgramPage::onPageChanging(wxWizardEvent &event)
{
	wxString message;

	message = wxEmptyString;

	/* If no program was set, we'll not proceed to the next page. */
	if (history_->getProgram().IsEmpty()) {
		message = _("Please choose a program first.");
	}

	if (!message.IsEmpty()) {
		wxMessageBox(message, _("Rule Wizard"), wxOK | wxICON_ERROR,
		    this);
		event.Veto();
	}
}

void
RuleWizardProgramPage::onPageChanged(wxWizardEvent &)
{
	updateNavi();
}

void
RuleWizardProgramPage::setProgram(const wxString &binary)
{
	/* Store binary */
	if (binary == history_->getProgram()) {
		return;
	}

	history_->setProgram(binary);

	Layout();
	Refresh();
}

void
RuleWizardProgramPage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, true);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillAlfNavi(this, naviSizer, false);
	history_->fillSandboxNavi(this, naviSizer, false);
	Layout();
	Refresh();
}

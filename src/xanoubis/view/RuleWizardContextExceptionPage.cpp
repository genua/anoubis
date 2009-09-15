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

#include <config.h>

#include <wx/filedlg.h>
#include <wx/arrstr.h>
#include <wx/msgdlg.h>

#include "RuleWizardContextExceptionPage.h"
#include "PolicyUtils.h"

RuleWizardContextExceptionPage::RuleWizardContextExceptionPage(wxWindow *parent,
    RuleWizardHistory *history) : RuleWizardContextExceptionPageBase(parent)
{
	history_ = history;

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGING, wxWizardEventHandler(
	    RuleWizardContextExceptionPage::onPageChanging), NULL, this);
	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardContextExceptionPage::onPageChanged),
	    NULL, this);
}

void
RuleWizardContextExceptionPage::onPageChanging(wxWizardEvent &)
{
	/* If no program was set, we'll not proceed to the next page. */
	/* XXX ch: do we really want to veto if list is empty? */
}

void
RuleWizardContextExceptionPage::onPageChanged(wxWizardEvent &)
{
	updateNavi();
	if (!history_->haveContextException()) {
		exceptionListBox->Clear();
	}
}

void
RuleWizardContextExceptionPage::onAddButton(wxCommandEvent &)
{
	int		selection;
	wxArrayString	list;
	wxFileDialog	fileDlg(this);

	wxBeginBusyCursor();
	fileDlg.SetDirectory(wxT("/usr/bin"));
	fileDlg.SetFilename(wxEmptyString);
	fileDlg.SetWildcard(wxT("*"));
	wxEndBusyCursor();

	if (fileDlg.ShowModal() == wxID_OK) {
		wxString	path = fileDlg.GetPath();

		wxBeginBusyCursor();

		history_->addContextException(path);
		selection = exceptionListBox->Append(path);
		exceptionListBox->Select(selection);
		updateNavi();
		wxEndBusyCursor();
	}
}

void
RuleWizardContextExceptionPage::onDeleteButton(wxCommandEvent &)
{
	int selection;

	selection = exceptionListBox->GetSelection();
	if (selection != wxNOT_FOUND) {
		exceptionListBox->Delete(selection);
		history_->delContextException(selection);
		if (!exceptionListBox->IsEmpty() &&
		    (selection >= (int)exceptionListBox->GetCount())) {
			/* In case we deleted the last element, adjust index. */
			selection--;
		}
		exceptionListBox->Select(selection);
		updateNavi();
	}
}

void
RuleWizardContextExceptionPage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, false);
	history_->fillContextNavi(this, naviSizer, true);
	history_->fillAlfNavi(this, naviSizer, false);
	history_->fillSandboxNavi(this, naviSizer, false);
	Layout();
	Refresh();
}

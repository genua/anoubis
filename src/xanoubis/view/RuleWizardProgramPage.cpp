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

#include "RuleWizardProgramPage.h"

RuleWizardProgramPage::RuleWizardProgramPage(wxWindow *parent,
    RuleWizardHistory *history) : RuleWizardProgramPageBase(parent)
{
	history_ = history;

	csumValue->SetLabel(wxEmptyString);

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGING,
	    wxWizardEventHandler(RuleWizardProgramPage::onPageChanging),
	    NULL, this);
	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardProgramPage::onPageChanged),
	    NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUMCALC,
	    wxTaskEventHandler(RuleWizardProgramPage::onCsumCalcTask),
	    NULL, this);
}

RuleWizardProgramPage::~RuleWizardProgramPage(void)
{
	JobCtrl::getInstance()->Disconnect(anTASKEVT_CSUMCALC,
	    wxTaskEventHandler(RuleWizardProgramPage::onCsumCalcTask),
	    NULL, this);
}

void
RuleWizardProgramPage::onPageChanging(wxWizardEvent &event)
{
	wxString message;

	message = wxEmptyString;

	/*
	 * If no program was set or the checkum is not calculated yet,
	 * we'll not proceed to the next page.
	 */
	if (history_->getProgram().IsEmpty()) {
		message = _("Please choose a program first.");
	} else if (history_->getChecksum().IsEmpty()) {
		message = _("No checkum is calculated yet.");
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
RuleWizardProgramPage::onProgramTextKillFocus(wxFocusEvent &)
{
	if (programTextCtrl->IsModified()) {
		/* Mark as clean */
		programTextCtrl->DiscardEdits();
		setProgram(programTextCtrl->GetValue());
		updateNavi();
	}
}

void
RuleWizardProgramPage::onProgramTextEnter(wxCommandEvent &event)
{
	setProgram(event.GetString());
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
		programTextCtrl->ChangeValue(fileDlg.GetPath());
		setProgram(fileDlg.GetPath());
		updateNavi();
	}
}

void
RuleWizardProgramPage::onCsumCalcTask(TaskEvent &event)
{
	wxString	 message;
	CsumCalcTask	*task;

	task = dynamic_cast<CsumCalcTask*>(event.getTask());
	if (task == 0) {
		/* No ComCsumGetTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (task != &calcTask_) {
		/* Belongs to someone other, ignore it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	if (task->getResult() != 0) {
		/* Calculation failed */
		message = wxString::Format(
		    _("Failed to calculate the checksum for %ls: %hs"),
		    task->getPath().c_str(),
		    strerror(task->getResult()));
		wxMessageBox(message, _("Rule Wizard"), wxOK | wxICON_ERROR,
		    this);
		csumValue->SetLabel(_("(unknown)"));
		return;
	}

	history_->setChecksum(task->getCsumStr());
	csumValue->SetLabel(history_->getChecksum());
	Layout();
	Refresh();
}

void
RuleWizardProgramPage::setProgram(const wxString &paramBinary)
{
	char		real[PATH_MAX], *res;
	wxString	binary;

	/* Store binary */
	if (paramBinary == history_->getProgram()) {
		return;
	}

	if (paramBinary == wxT("") || paramBinary == wxT("any")) {
		programInfo->SetLabel(wxT(""));
		binary = paramBinary;
	} else {
		res = realpath(paramBinary.fn_str(), real);
		if (res) {
			binary = wxString::From8BitData(res, strlen(res));
			if (binary != paramBinary) {
				programTextCtrl->ChangeValue(binary);
				programInfo->SetLabel(
				    _("Symbolic link was resolved"));
			} else {
				programInfo->SetLabel(wxT(""));
			}
		} else {
			binary = paramBinary;
			programInfo->SetLabel(
			    _("Failure to resolve symbolic link"));
		}
	}
	history_->setProgram(binary);
	history_->setChecksum(wxEmptyString);

	/* Start csum calculation */
	if (!binary.IsEmpty() && binary != wxT("any")) {
		calcTask_.setPath(binary);
		calcTask_.setCalcLink(false);
		JobCtrl::getInstance()->addTask(&calcTask_);

		csumValue->SetLabel(_("calculating..."));
	}
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

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
#include <wx/msgdlg.h>
#include <wx/string.h>

#include "DlgRuleEditorAppPage.h"
#include "AnUtils.h"

DlgRuleEditorAppPage::DlgRuleEditorAppPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorPage(),
    DlgRuleEditorAppPageBase(parent, id, pos, size, style)
{
	binaryIndex_ = 0;
	csumCache_ = wxEmptyString;
	appPolicy_ = NULL;
	ctxPolicy_ = NULL;

	JobCtrl::getInstance()->Connect(anTASKEVT_CSUMCALC,
	    wxTaskEventHandler(DlgRuleEditorAppPage::onCsumCalcTask),
	    NULL, this);
}

DlgRuleEditorAppPage::~DlgRuleEditorAppPage(void)
{
	JobCtrl::getInstance()->Disconnect(anTASKEVT_CSUMCALC,
	    wxTaskEventHandler(DlgRuleEditorAppPage::onCsumCalcTask),
	    NULL, this);
}

void
DlgRuleEditorAppPage::update(Subject *subject)
{
	if ((subject == appPolicy_) || (subject == ctxPolicy_)) {
		/* This is our policy. */
		showBinary();
		showCsum();
		showStatus();
	}
}

void
DlgRuleEditorAppPage::select(Policy *policy)
{
	if (policy->IsKindOf(CLASSINFO(AppPolicy))) {
		appPolicy_ = wxDynamicCast(policy, AppPolicy);
		DlgRuleEditorPage::select(policy);
	}
	if (policy->IsKindOf(CLASSINFO(ContextFilterPolicy))) {
		ctxPolicy_ = wxDynamicCast(policy, ContextFilterPolicy);
		DlgRuleEditorPage::select(policy);
	}
}

void
DlgRuleEditorAppPage::deselect(void)
{
	appPolicy_ = NULL;
	ctxPolicy_ = NULL;
	DlgRuleEditorPage::deselect();
}

void
DlgRuleEditorAppPage::setBinaryIndex(unsigned int index)
{
	binaryIndex_ = index;
}

void
DlgRuleEditorAppPage::showBinary(void)
{
	if (appPolicy_ != NULL) {
		binaryTextCtrl->ChangeValue(
		    appPolicy_->getBinaryName(binaryIndex_));
	}
	if (ctxPolicy_ != NULL) {
		binaryTextCtrl->ChangeValue(
		    ctxPolicy_->getBinaryName(binaryIndex_));
	}
	Layout();
}

void
DlgRuleEditorAppPage::showCsum(void)
{
	wxString csum;

	csum = wxEmptyString;

	if (appPolicy_ != NULL) {
		csum = appPolicy_->getHashValueName(binaryIndex_);
	}
	if (ctxPolicy_ != NULL) {
		csum = ctxPolicy_->getHashValueName(binaryIndex_);
	}

	csumRegisteredText->SetLabel(csum);
	csumCurrentText->SetLabel(csumCache_);
	Layout();
}

void
DlgRuleEditorAppPage::showStatus(void)
{
	unsigned int	count;
	wxString	registered;

	count = 0;
	registered = wxEmptyString;

	if (appPolicy_ != NULL) {
		count = appPolicy_->getBinaryCount();
		registered = appPolicy_->getHashValueName(binaryIndex_);
	}
	if (ctxPolicy_ != NULL) {
		count = ctxPolicy_->getBinaryCount();
		registered = ctxPolicy_->getHashValueName(binaryIndex_);
	}

	if (count < 1) {
		deleteButton->Disable();
	} else {
		deleteButton->Enable();
	}

	if (csumCache_.IsEmpty()) {
		statusText->SetLabel(_("(unknown)"));
		Layout();
		return;
	}

	if (registered.Cmp(csumCache_) == 0) {
		statusText->SetLabel(_("match"));
	} else {
		statusText->SetLabel(_("mismatch"));
	}
	Layout();
}

void
DlgRuleEditorAppPage::setBinary(wxString binary)
{
	int		 selection;
	wxFileName	 baseName;
	wxNotebook	*parentNotebook;

	if (appPolicy_ != NULL) {
		if (appPolicy_->getBinaryCount() == 0) {
			appPolicy_->addBinary(binary);
		} else {
			appPolicy_->setBinaryName(binary, binaryIndex_);
		}
	}
	if (ctxPolicy_ != NULL) {
		ctxPolicy_->setBinaryName(binary, binaryIndex_);
	}

	parentNotebook = wxDynamicCast(GetParent(), wxNotebook);
	if (parentNotebook != NULL) {
		selection = parentNotebook->GetSelection();
		baseName.Assign(binary);
		parentNotebook->SetPageText(selection, baseName.GetFullName());
	}
}

void
DlgRuleEditorAppPage::onBinaryTextKillFocus(wxFocusEvent &)
{
	setBinary(binaryTextCtrl->GetValue());
}

void
DlgRuleEditorAppPage::onBinaryTextEnter(wxCommandEvent &event)
{
	setBinary(event.GetString());
}

void
DlgRuleEditorAppPage::onPickButton(wxCommandEvent &)
{
	wxFileName	defaultPath;
	wxFileDialog	fileDlg(this);

	if (appPolicy_ != NULL) {
		defaultPath.Assign(appPolicy_->getBinaryName(binaryIndex_));
	}
	if (ctxPolicy_ != NULL) {
		defaultPath.Assign(ctxPolicy_->getBinaryName(binaryIndex_));
	}

	wxBeginBusyCursor();
	fileDlg.SetDirectory(defaultPath.GetPath());
	fileDlg.SetFilename(defaultPath.GetFullName());
	fileDlg.SetWildcard(wxT("*"));
	wxEndBusyCursor();

	if (fileDlg.ShowModal() == wxID_OK) {
		setBinary(fileDlg.GetPath());
	}
}

void
DlgRuleEditorAppPage::onValidateButton(wxCommandEvent &)
{
	wxString binary;

	binary = wxEmptyString;

	if (appPolicy_ != NULL) {
		binary = appPolicy_->getBinaryName(binaryIndex_);
	}
	if (ctxPolicy_ != NULL) {
		binary = ctxPolicy_->getBinaryName(binaryIndex_);
	}

	if (!binary.IsEmpty()) {
		calcTask_.setPath(binary);
		calcTask_.setCalcLink(true);
		JobCtrl::getInstance()->addTask(&calcTask_);
		statusText->SetLabel(_("calculating..."));
		Layout();
	}
}

void
DlgRuleEditorAppPage::onUpdateButton(wxCommandEvent &)
{
	if (appPolicy_ != NULL) {
		appPolicy_->setHashValueString(csumCache_, binaryIndex_);
	}
	if (ctxPolicy_ != NULL) {
		ctxPolicy_->setHashValueString(csumCache_, binaryIndex_);
	}
}

void
DlgRuleEditorAppPage::onDeleteButton(wxCommandEvent &event)
{
	/* Send the index of the binary to delete to AnPolicyNotebook. */
	event.SetInt(binaryIndex_);
	event.Skip();
}

void
DlgRuleEditorAppPage::onCsumCalcTask(TaskEvent &event)
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
		    _("Failed to calculate the checksum for %s: %s"),
		    task->getPath().c_str(),
		    wxStrError(task->getResult()).c_str());
		wxMessageBox(message, _("Rule Editor"), wxOK | wxICON_ERROR,
		    this);
		statusText->SetLabel(_("(unknown)"));
		return;
	}

	csumCache_ = task->getCsumStr();
	showCsum();
	showStatus();
}
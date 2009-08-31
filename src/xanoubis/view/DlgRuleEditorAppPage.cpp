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

#include <climits>
#include <cstdlib>

#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/string.h>

#include "AnPickFromFs.h"
#include "DlgRuleEditorAppPage.h"

DlgRuleEditorAppPage::DlgRuleEditorAppPage(wxWindow *parent,
    wxWindowID id, const wxPoint & pos, const wxSize & size, long style)
    : DlgRuleEditorPage(),
    DlgRuleEditorAppPageBase(parent, id, pos, size, style)
{
	binaryIndex_ = 0;
	automaticOnNew_ = false;
	csumCache_ = wxEmptyString;
	appPolicy_ = NULL;
	ctxPolicy_ = NULL;

	addSubject(binaryPicker);
	binaryPicker->setMode(AnPickFromFs::MODE_FILE);
	binaryPicker->setTitle(_("Binary:"));

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
	if (subject == binaryPicker) {
		setBinary(binaryPicker->getFileName());
	}
}

void
DlgRuleEditorAppPage::select(Policy *policy)
{
	if (policy->IsKindOf(CLASSINFO(AppPolicy))) {
		appPolicy_ = wxDynamicCast(policy, AppPolicy);
		DlgRuleEditorPage::select(policy);
		if (appPolicy_->getTypeID() == APN_SFS) {
			enable_ = false;
		}
		Enable(enable_);
	}
	if (policy->IsKindOf(CLASSINFO(ContextFilterPolicy))) {
		ctxPolicy_ = wxDynamicCast(policy, ContextFilterPolicy);
		DlgRuleEditorPage::select(policy);
		Enable(enable_);
	}
	showInfo(wxT(""));
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
		binaryPicker->setFileName(
		    appPolicy_->getBinaryName(binaryIndex_));
	}
	if (ctxPolicy_ != NULL) {
		binaryPicker->setFileName(
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
	bool		isAny;
	wxString	registered;

	isAny = false;
	registered = wxEmptyString;

	if (appPolicy_ != NULL) {
		isAny = appPolicy_->isAnyBlock();
		registered = appPolicy_->getHashValueName(binaryIndex_);
	}
	if (ctxPolicy_ != NULL) {
		isAny = ctxPolicy_->isAny();
		registered = ctxPolicy_->getHashValueName(binaryIndex_);
	}

	if (isAny) {
		deleteButton->Disable();
		validateButton->Disable();
	} else {
		deleteButton->Enable();
		validateButton->Enable();
	}

	if (csumCache_.IsEmpty()) {
		statusText->SetLabel(_("(unknown)"));
		updateButton->Disable();
		Layout();
		Refresh();
		return;
	}

	if (registered.Cmp(csumCache_) == 0) {
		statusText->SetLabel(_("match"));
		updateButton->Disable();
	} else {
		statusText->SetLabel(_("mismatch"));
		updateButton->Enable();
	}
	Layout();
	Refresh();
}

void
DlgRuleEditorAppPage::setBinary(wxString binary)
{
	int		 selection;
	wxString	 current;
	wxFileName	 baseName;
	wxNotebook	*parentNotebook;

	if (appPolicy_ != NULL) {
		if (appPolicy_->isAnyBlock()) {
			appPolicy_->addBinary(binary);
			automaticOnNew_ = true;
		} else {
			current = appPolicy_->getBinaryName(binaryIndex_);
			if (binary.Cmp(current) == 0) {
				return;
			}
			appPolicy_->setBinaryName(binary, binaryIndex_);
			automaticOnNew_ = true;
		}
	}
	if (ctxPolicy_ != NULL) {
		if (ctxPolicy_->isAny()) {
			ctxPolicy_->addBinary(binary);
			automaticOnNew_ = true;
		} else {
			current = ctxPolicy_->getBinaryName(binaryIndex_);
			if (binary.Cmp(current) == 0) {
				return;
			}
			ctxPolicy_->setBinaryName(binary, binaryIndex_);
			automaticOnNew_ = true;
		}
	}

	parentNotebook = wxDynamicCast(GetParent(), wxNotebook);
	if ((parentNotebook == NULL) && (GetGrandParent() != NULL)) {
		/*
		 * This might be the case when we're integrated within
		 * a FilterContextPage.
		 */
		parentNotebook = wxDynamicCast(GetGrandParent()->GetParent(),
		    wxNotebook);
	}
	if (parentNotebook != NULL) {
		selection = parentNotebook->GetSelection();
		baseName.Assign(binary);
		parentNotebook->SetPageText(selection, baseName.GetFullName());
	}

	if (binary != wxT("") && binary != wxT("any") &&
	    !wxFileName::IsFileExecutable(binary)) {
		showInfo(_("File does not exist or is not executable"));
	} else {
		showInfo(wxEmptyString);
	}

	if (automaticOnNew_ == true) {
		csumCache_.Empty();
		startCalculation();
	}
}

void
DlgRuleEditorAppPage::startCalculation(void)
{
	wxString binary;

	binary = wxEmptyString;

	if (appPolicy_ != NULL) {
		binary = appPolicy_->getBinaryName(binaryIndex_);
	}
	if (ctxPolicy_ != NULL) {
		binary = ctxPolicy_->getBinaryName(binaryIndex_);
	}

	if (!binary.IsEmpty() && binary != wxT("any")) {
		calcTask_.setPath(binary);
		calcTask_.setCalcLink(true);
		JobCtrl::getInstance()->addTask(&calcTask_);
		statusText->SetLabel(_("calculating..."));
		Layout();
	}
}

void
DlgRuleEditorAppPage::doCsumUpdate(void)
{
	if (appPolicy_ != NULL) {
		appPolicy_->setHashValueString(csumCache_, binaryIndex_);
	}
	if (ctxPolicy_ != NULL) {
		ctxPolicy_->setHashValueString(csumCache_, binaryIndex_);
	}
}

void
DlgRuleEditorAppPage::onValidateButton(wxCommandEvent &)
{
	startCalculation();
}

void
DlgRuleEditorAppPage::onUpdateButton(wxCommandEvent &)
{
	doCsumUpdate();
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
		    _("Failed to calculate the checksum for %ls: %hs"),
		    task->getPath().c_str(),
		    strerror(task->getResult()));
		wxMessageBox(message, _("Rule Editor"), wxOK | wxICON_ERROR,
		    this);
		csumCache_.Empty();
	} else {
		csumCache_ = task->getCsumStr();
	}

	if (automaticOnNew_ == true) {
		automaticOnNew_ = false;
		doCsumUpdate();
	}

	showCsum();
	showStatus();
}

void
DlgRuleEditorAppPage::showInfo(const wxString &string)
{
	if (string == wxT("")) {
		infoLeft->Hide();
		infoRight->Hide();
	} else {
		infoRight->SetLabel(string);
		infoLeft->Show();
		infoRight->Show();
	}
	Layout();
	Refresh();
}

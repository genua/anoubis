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
#include <wx/dirdlg.h>
#include <wx/filename.h>

#include <AnListCtrl.h>
#include <SbEntry.h>
#include <SbModel.h>
#include <SbModelRowProvider.h>

#include "RuleWizardSandboxProperty.h"
#include "RuleWizardSandboxWriteFilesPage.h"

RuleWizardSandboxWriteFilesPage::RuleWizardSandboxWriteFilesPage(
    wxWindow *parent, RuleWizardHistory *history)
    : RuleWizardSandboxFilesPageBase(parent)
{
	history->get();
	history_ = history;

	/* The initial column width */
	int width = fileListCtrl->GetClientSize().GetWidth() / 3;

	/* Create columns */
	fileListCtrl->addColumn(new RuleWizardSandboxProperty(
	    RuleWizardSandboxProperty::PATH), width);
	fileListCtrl->addColumn(new RuleWizardSandboxProperty(
	    RuleWizardSandboxProperty::FILE), width);
	fileListCtrl->addColumn(new RuleWizardSandboxProperty(
	    RuleWizardSandboxProperty::STD), width);

	/* Assign row-provider to list */
	rowProvider_ = new SbModelRowProvider(
	    history_->getSandboxFileList(), SbEntry::WRITE);
	fileListCtrl->setRowProvider(rowProvider_);

	defaultsButton->Enable(
	    history_->getSandboxFileList()->canAssignDefaults());
	validCheckBox->SetLabel(
	    _("always deny access on valid checksum / signature"));

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED, wxWizardEventHandler(
	    RuleWizardSandboxWriteFilesPage::onPageChanged), NULL, this);
}

RuleWizardSandboxWriteFilesPage::~RuleWizardSandboxWriteFilesPage(void)
{
	fileListCtrl->setRowProvider(0);
	delete rowProvider_;
	RuleWizardHistory::put(history_);
}

void
RuleWizardSandboxWriteFilesPage::onPageChanged(wxWizardEvent &)
{
	wxString text;

	text.Printf(_("Options for WRITE access of application \"%ls\":"),
	    history_->getProgram().c_str());
	questionLabel->SetLabel(text);

	fileListLabel->SetLabel(_("Grant WRITE access:"));

	updateNavi();
}

void
RuleWizardSandboxWriteFilesPage::onAddFileButton(wxCommandEvent &)
{
	wxFileName	path(history_->getProgram());
	wxFileDialog	fileDlg(this);

	wxBeginBusyCursor();
	fileDlg.SetDirectory(path.GetPath());
	fileDlg.SetFilename(path.GetFullName());
	fileDlg.SetWildcard(wxT("*"));
	wxEndBusyCursor();

	if (fileDlg.ShowModal() == wxID_OK) {
		SbEntry *entry = history_->getSandboxFileList()->getEntry(
		    fileDlg.GetPath(), true);
		entry->setPermission(SbEntry::WRITE, true);

		updateNavi();
	}
}

void
RuleWizardSandboxWriteFilesPage::onAddDirectoryButton(wxCommandEvent &)
{
	wxFileName	path(history_->getProgram());
	wxDirDialog	dirDlg(this);

	wxBeginBusyCursor();
	dirDlg.SetPath(path.GetPath());
	wxEndBusyCursor();

	if (dirDlg.ShowModal() == wxID_OK) {
		SbEntry *entry = history_->getSandboxFileList()->getEntry(
		    dirDlg.GetPath(), true);
		entry->setPermission(SbEntry::WRITE, true);

		updateNavi();
	}
}

void
RuleWizardSandboxWriteFilesPage::onDefaultsButton(wxCommandEvent &)
{
	history_->getSandboxFileList()->assignDefaults(SbEntry::WRITE);

	updateNavi();
}

void
RuleWizardSandboxWriteFilesPage::onDeleteButton(wxCommandEvent &)
{
	std::list<SbEntry *>	removeList;
	int			index = -1;

	/*
	 * AnListCtrl restores selected internally, if the content of the list
	 * changes. That's why you have to collect the selected services
	 * before removing them from the model.
	 */
	while (true) {
		if ((index = fileListCtrl->getNextSelection(index)) == -1)
			break;

		SbEntry *entry = rowProvider_->getEntryAt(index);

		if (entry != 0)
			removeList.push_back(entry);
	}

	/*
	 * Now it's save to remove the services from the model as you are
	 * independent from the selection
	 */
	for (std::list<SbEntry *>::const_iterator it = removeList.begin();
	    it != removeList.end(); ++it) {
		SbEntry *entry = (*it);
		entry->setPermission(SbEntry::WRITE, false);
	}

	updateNavi();
}

void
RuleWizardSandboxWriteFilesPage::onFileListSelect(wxListEvent &)
{
	deleteButton->Enable();
}

void
RuleWizardSandboxWriteFilesPage::onFileListDeselect(wxListEvent &)
{
	/* Was the last one deselected? */
	if (fileListCtrl->GetSelectedItemCount() == 0) {
		deleteButton->Disable();
	}
}

void
RuleWizardSandboxWriteFilesPage::onAskCheckBox(wxCommandEvent & event)
{
	history_->setSandboxAsk(SbEntry::WRITE, event.GetInt());
	updateNavi();
}

void
RuleWizardSandboxWriteFilesPage::onValidCheckBox(wxCommandEvent & event)
{
	history_->setSandboxValidSignature(SbEntry::WRITE, event.GetInt());
	updateNavi();
}

void
RuleWizardSandboxWriteFilesPage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, false);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillAlfNavi(this, naviSizer, false);
	history_->fillSandboxNavi(this, naviSizer, true);
	Layout();
	Refresh();
}

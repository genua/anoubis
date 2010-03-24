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

#include <AnListColumn.h>
#include <AnListCtrl.h>
#include <SbEntry.h>
#include <SbModel.h>
#include <SbModelRowProvider.h>

#include "RuleWizardSandboxExecuteFilesPage.h"
#include "RuleWizardSandboxProperty.h"

#define ALL_FILES_ENTRY	_("(all files)")

RuleWizardSandboxExecuteFilesPage::RuleWizardSandboxExecuteFilesPage(
    wxWindow *parent, RuleWizardHistory *history)
    : RuleWizardSandboxFilesPageBase(parent)
{
	history->get();
	history_ = history;

	/* The initial column width */
	int width = fileListCtrl->GetClientSize().GetWidth() / 3;

	AnListColumn *col;

	/* Create columns */
	col = fileListCtrl->addColumn(new RuleWizardSandboxProperty(
	    RuleWizardSandboxProperty::PATH));
	col->setWidth(width);
	col = fileListCtrl->addColumn(new RuleWizardSandboxProperty(
	    RuleWizardSandboxProperty::FILE));
	col->setWidth(width);
	col = fileListCtrl->addColumn(new RuleWizardSandboxProperty(
	    RuleWizardSandboxProperty::STD));
	col->setWidth(width);

	/* Assign row-provider to list */
	rowProvider_ = new SbModelRowProvider(
	    history_->getSandboxFileList(), SbEntry::EXECUTE);
	fileListCtrl->setRowProvider(rowProvider_);

	defaultsButton->Enable(
	    history_->getSandboxFileList()->canAssignDefaults());

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED, wxWizardEventHandler(
	    RuleWizardSandboxExecuteFilesPage::onPageChanged), NULL, this);
}

RuleWizardSandboxExecuteFilesPage::~RuleWizardSandboxExecuteFilesPage(void)
{
	fileListCtrl->setRowProvider(0);
	delete rowProvider_;
	RuleWizardHistory::put(history_);
}

void
RuleWizardSandboxExecuteFilesPage::onPageChanged(wxWizardEvent &)
{
	wxString text;

	text.Printf(_("Options for EXECUTE access of application \"%ls\":"),
	    history_->getProgram().c_str());
	questionLabel->SetLabel(text);

	fileListLabel->SetLabel(_("Grant EXECUTE access:"));

	updateNavi();
}

void
RuleWizardSandboxExecuteFilesPage::onAddFileButton(wxCommandEvent &)
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
		entry->setPermission(SbEntry::EXECUTE, true);

		updateNavi();
	}
}

void
RuleWizardSandboxExecuteFilesPage::onAddDirectoryButton(wxCommandEvent &)
{
	wxFileName	path(history_->getProgram());
	wxDirDialog	dirDlg(this);

	wxBeginBusyCursor();
	dirDlg.SetPath(path.GetPath());
	wxEndBusyCursor();

	if (dirDlg.ShowModal() == wxID_OK) {
		SbEntry *entry = history_->getSandboxFileList()->getEntry(
		    dirDlg.GetPath(), true);
		entry->setPermission(SbEntry::EXECUTE, true);

		updateNavi();
	}
}

void
RuleWizardSandboxExecuteFilesPage::onDefaultsButton(wxCommandEvent &)
{
	history_->getSandboxFileList()->assignDefaults(SbEntry::EXECUTE);

	updateNavi();
}

void
RuleWizardSandboxExecuteFilesPage::onDeleteButton(wxCommandEvent &)
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
		entry->setPermission(SbEntry::EXECUTE, false);
	}

	updateNavi();
}

void
RuleWizardSandboxExecuteFilesPage::onFileListSelect(wxListEvent &)
{
	deleteButton->Enable();
}

void
RuleWizardSandboxExecuteFilesPage::onFileListDeselect(wxListEvent &)
{
	/* Was the last one deselected? */
	if (fileListCtrl->GetSelectedItemCount() == 0) {
		deleteButton->Disable();
	}
}

void
RuleWizardSandboxExecuteFilesPage::onAskCheckBox(wxCommandEvent & event)
{
	history_->setSandboxAsk(SbEntry::EXECUTE, event.GetInt());
	updateNavi();
}

void
RuleWizardSandboxExecuteFilesPage::onValidCheckBox(wxCommandEvent & event)
{
	history_->setSandboxValidSignature(SbEntry::EXECUTE, event.GetInt());
	updateNavi();
}

void
RuleWizardSandboxExecuteFilesPage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, false);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillAlfNavi(this, naviSizer, false);
	history_->fillSandboxNavi(this, naviSizer, true);
	Layout();
	Refresh();
}

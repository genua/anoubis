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

#include "RuleWizardSandboxReadFilesPage.h"

#define ALL_FILES_ENTRY	_("(all files)")

RuleWizardSandboxReadFilesPage::RuleWizardSandboxReadFilesPage(wxWindow *parent,
    RuleWizardHistory *history) : RuleWizardSandboxFilesPageBase(parent)
{
	int width;

	history_ = history;

	/* Create columns */
	fileListCtrl->InsertColumn(COLUMN_PATH, _("Path"));
	fileListCtrl->InsertColumn(COLUMN_FILE, _("File"));
	fileListCtrl->InsertColumn(COLUMN_STD, _("Standard"));

	/* Set initial column width */
	width = fileListCtrl->GetClientSize().GetWidth() / 3;
	fileListCtrl->SetColumnWidth(COLUMN_PATH, width);
	fileListCtrl->SetColumnWidth(COLUMN_FILE, width);
	fileListCtrl->SetColumnWidth(COLUMN_STD, width);

	parent->Connect(wxEVT_WIZARD_PAGE_CHANGED,
	    wxWizardEventHandler(RuleWizardSandboxReadFilesPage::onPageChanged),
	    NULL, this);
}

void
RuleWizardSandboxReadFilesPage::onPageChanged(wxWizardEvent &)
{
	wxString text;

	text.Printf(_("Options for READING access of application \"%ls\":"),
	    history_->getProgram().c_str());
	questionLabel->SetLabel(text);

	fileListLabel->SetLabel(_("Grand READING access:"));

	updateNavi();
}

void
RuleWizardSandboxReadFilesPage::onAddFileButton(wxCommandEvent &)
{
	long		index;
	wxFileName	path;
	wxFileDialog	fileDlg(this);

	path.Assign(history_->getProgram());

	wxBeginBusyCursor();
	fileDlg.SetDirectory(path.GetPath());
	fileDlg.SetFilename(path.GetFullName());
	fileDlg.SetWildcard(wxT("*"));
	wxEndBusyCursor();

	if (fileDlg.ShowModal() == wxID_OK) {
		path.Assign(fileDlg.GetPath());
		index = fileListCtrl->GetItemCount();
		fileListCtrl->InsertItem(index, wxEmptyString);
		fileListCtrl->SetItem(index, COLUMN_PATH, path.GetPath());
		fileListCtrl->SetItem(index, COLUMN_FILE, path.GetFullName());
		storeFileList();
		updateNavi();
	}
}

void
RuleWizardSandboxReadFilesPage::onAddDirectoryButton(wxCommandEvent &)
{
	long		index;
	wxFileName	path;
	wxDirDialog	dirDlg(this);

	path.Assign(history_->getProgram());

	wxBeginBusyCursor();
	dirDlg.SetPath(path.GetPath());
	wxEndBusyCursor();

	if (dirDlg.ShowModal() == wxID_OK) {
		index = fileListCtrl->GetItemCount();
		fileListCtrl->InsertItem(index, wxEmptyString);
		fileListCtrl->SetItem(index, COLUMN_PATH, dirDlg.GetPath());
		fileListCtrl->SetItem(index, COLUMN_FILE, ALL_FILES_ENTRY);
		storeFileList();
		updateNavi();
	}
}

void
RuleWizardSandboxReadFilesPage::onDeleteButton(wxCommandEvent &)
{
	long index;

	index = fileListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL,
	    wxLIST_STATE_SELECTED);
	while (index != wxNOT_FOUND) {
		fileListCtrl->SetItemState(index, 0, wxLIST_STATE_SELECTED);
		fileListCtrl->DeleteItem(index);
		index = fileListCtrl->GetNextItem(index - 1, wxLIST_NEXT_ALL,
		    wxLIST_STATE_SELECTED);
	}
	storeFileList();
	updateNavi();
}

void
RuleWizardSandboxReadFilesPage::onFileListSelect(wxListEvent &)
{
	deleteButton->Enable();
}

void
RuleWizardSandboxReadFilesPage::onFileListDeselect(wxListEvent &)
{
	/* Was the last one deselected? */
	if (fileListCtrl->GetSelectedItemCount() == 0) {
		deleteButton->Disable();
	}
}

void
RuleWizardSandboxReadFilesPage::onAskCheckBox(wxCommandEvent & event)
{
	history_->setSandboxReadAsk(event.GetInt());
	updateNavi();
}

void
RuleWizardSandboxReadFilesPage::onValidCheckBox(wxCommandEvent & event)
{
	history_->setSandboxReadValidSignature(event.GetInt());
	updateNavi();
}

void
RuleWizardSandboxReadFilesPage::storeFileList(void) const
{
	long		index;
	wxString	compound;
	wxListItem	line;
	wxArrayString	list;

	for (index=0; index<fileListCtrl->GetItemCount(); index++) {
		/* Get Path */
		line.SetId(index);
		line.SetColumn(COLUMN_PATH);
		fileListCtrl->GetItem(line);
		compound = line.GetText();

		/* Get File */
		line.SetId(index);
		line.SetColumn(COLUMN_FILE);
		fileListCtrl->GetItem(line);
		if (line.GetText() != ALL_FILES_ENTRY) {
			compound.Append(wxFILE_SEP_PATH);
			compound.Append(line.GetText());
		}

		list.Add(compound);
	}

	history_->setSandboxReadFileList(list);
}

void
RuleWizardSandboxReadFilesPage::updateNavi(void)
{
	naviSizer->Clear(true);
	history_->fillProgramNavi(this, naviSizer, false);
	history_->fillContextNavi(this, naviSizer, false);
	history_->fillAlfNavi(this, naviSizer, false);
	history_->fillSandboxNavi(this, naviSizer, true);
	Layout();
	Refresh();
}

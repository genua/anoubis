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

#include <JobCtrl.h>
#include <SfsCtrl.h>
#include <SfsEntry.h>

#include "main.h"
#include "ModSfsDetailsDlg.h"
#include "ModSfsListCtrl.h"

ModSfsListCtrl::ModSfsListCtrl(wxWindow *parent, wxWindowID id,
    const wxPoint &pos, const wxSize &size, long style)
    : wxListCtrl(parent, id, pos, size, style), SfsDirectoryScanHandler(40)
{
	sfsCtrl_ = 0;
	currentSelection_ = -1;

	okIcon_ = wxGetApp().loadIcon(wxT("General_ok_16.png"));
	warnIcon_ = wxGetApp().loadIcon(wxT("General_problem_16.png"));
	errorIcon_ = wxGetApp().loadIcon(wxT("General_error_16.png"));
	symlinkIcon_ = wxGetApp().loadIcon(wxT("General_symlink_16.png"));

	imageList_.Add(wxNullIcon);
	imageList_.Add(*okIcon_);
	imageList_.Add(*warnIcon_);
	imageList_.Add(*errorIcon_);
	imageList_.Add(*symlinkIcon_);

	/* Assign icons to list */
	SetImageList(&imageList_, wxIMAGE_LIST_SMALL);

	/* Insert columns */
	InsertColumn(COLUMN_FILE, _("File"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
	InsertColumn(COLUMN_CHECKSUM, _("Checksum"),
	    wxLIST_FORMAT_CENTRE, wxLIST_AUTOSIZE_USEHEADER);
	InsertColumn(COLUMN_SIGNATURE, _("Signature"),
	    wxLIST_FORMAT_CENTRE, wxLIST_AUTOSIZE_USEHEADER);

	/* Adjust initial width of COLUMN_FILE */
	/* Adjust column width to fill space. (Bug #1321). */
	SetColumnWidth(COLUMN_FILE, 360);

	/* Initialize popup-menu */

	wxMenuItem *showDetailsItem = popupMenu_.Append(
	    wxNewId(), _("Show details"));
	wxMenuItem *resolveLinkItem = popupMenu_.Append(
	    wxNewId(), _("Jump to link target"));

	popupMenu_.Connect(showDetailsItem->GetId(),
	    wxEVT_COMMAND_MENU_SELECTED,
	    wxCommandEventHandler(ModSfsListCtrl::OnPopupShowDetailsSelected),
	    NULL, this);
	popupMenu_.Connect(resolveLinkItem->GetId(),
	    wxEVT_COMMAND_MENU_SELECTED,
	    wxCommandEventHandler(ModSfsListCtrl::OnPopupResolveLinkSelected),
	    NULL, this);

	Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
	    wxListEventHandler(ModSfsListCtrl::OnListItemActivated),
	    NULL, this);
	Connect(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
	    wxListEventHandler(ModSfsListCtrl::OnListItemRightClicked),
	    NULL, this);

	scanProgressDlg_ = 0;
}

ModSfsListCtrl::~ModSfsListCtrl(void)
{
	if (sfsCtrl_ != 0) {
		SfsDirectory &dir = sfsCtrl_->getSfsDirectory();
		dir.setScanHandler(0);
	}

	delete okIcon_;
	delete warnIcon_;
	delete errorIcon_;
	delete symlinkIcon_;
}

SfsCtrl *
ModSfsListCtrl::getSfsCtrl(void) const
{
	return (this->sfsCtrl_);
}

void
ModSfsListCtrl::setSfsCtrl(SfsCtrl *sfsCtrl)
{
	if (sfsCtrl != 0) {
		this->sfsCtrl_ = sfsCtrl;

		/* Assign this class for monitoring filesystem-scans */
		SfsDirectory &dir = sfsCtrl->getSfsDirectory();
		dir.setScanHandler(this);
	} else
		this->sfsCtrl_ = 0;
}

IndexArray
ModSfsListCtrl::getSelectedIndexes(void) const
{
	int		selection = -1;
	IndexArray	selectionArray;

	while (true) {
		selection = GetNextItem(selection, wxLIST_NEXT_ALL,
		    wxLIST_STATE_SELECTED);

		if (selection == -1) {
			/* No selection */
			break;
		}

		selectionArray.Add(selection);
	}

	return (selectionArray);
}

void
ModSfsListCtrl::refreshList(void)
{
	if (sfsCtrl_ == 0)
		return;

	SfsDirectory &dir = sfsCtrl_->getSfsDirectory();

	DeleteAllItems();

	for (unsigned int i = 0; i < dir.getNumEntries(); i++) {
		int idx = GetItemCount();

		InsertItem(idx, wxEmptyString);
		refreshEntry(idx);
	}
}

void
ModSfsListCtrl::refreshEntry(unsigned int idx)
{
	if (sfsCtrl_ == 0)
		return;

	SfsDirectory	&dir = sfsCtrl_->getSfsDirectory();
	wxString	checksumInfo, signatureInfo;
	int		fileIconIndex = 0, checksumIconIndex = 0,
			signatureIconIndex = 0;

	/* Receive entry */
	SfsEntry	*entry = dir.getEntry(idx);
	wxString	baseDir = dir.getPath();

	if (entry->isSymlink())
		fileIconIndex = 4;

	switch (entry->getChecksumState(SfsEntry::SFSENTRY_CHECKSUM)) {
	case SfsEntry::SFSENTRY_NOT_VALIDATED:
		checksumInfo = _("???");
		checksumIconIndex = 0;
		break;
	case SfsEntry::SFSENTRY_MISSING:
		checksumInfo = _("not registered");
		checksumIconIndex = 0;
		break;
	case SfsEntry::SFSENTRY_INVALID:
		checksumInfo = _("invalid");
		checksumIconIndex = 2;
		break;
	case SfsEntry::SFSENTRY_NOMATCH:
		checksumInfo = wxEmptyString;
		checksumIconIndex = 3;
		break;
	case SfsEntry::SFSENTRY_MATCH:
		checksumInfo = wxEmptyString;
		checksumIconIndex = 1;
		break;
	case SfsEntry::SFSENTRY_ORPHANED:
		checksumInfo = _("orphaned");
		checksumIconIndex = 2;
		break;
	}

	switch (entry->getChecksumState(SfsEntry::SFSENTRY_SIGNATURE)) {
	case SfsEntry::SFSENTRY_NOT_VALIDATED:
		signatureInfo = _("???");
		signatureIconIndex = 0;
		break;
	case SfsEntry::SFSENTRY_MISSING:
		signatureInfo = _("not registered");
		signatureIconIndex = 0;
		break;
	case SfsEntry::SFSENTRY_INVALID:
		signatureInfo = _("invalid");
		signatureIconIndex = 2;
		break;
	case SfsEntry::SFSENTRY_NOMATCH:
		switch(entry->getChecksumState(SfsEntry::SFSENTRY_UPGRADE)) {
		case SfsEntry::SFSENTRY_MATCH:
			signatureIconIndex = 2;
			signatureInfo = _("upgraded");
			break;
		default:
			signatureIconIndex = 3;
		}
		break;
	case SfsEntry::SFSENTRY_MATCH:
		signatureIconIndex = 1;
		break;
	case SfsEntry::SFSENTRY_ORPHANED:
		signatureInfo = _("orphaned");
		signatureIconIndex = 2;
		break;
	}

	SetItem(idx, COLUMN_FILE,
	    entry->getRelativePath(baseDir), fileIconIndex);
	SetItem(idx, COLUMN_CHECKSUM,
	    checksumInfo, checksumIconIndex);
	SetItem(idx, COLUMN_SIGNATURE,
	    signatureInfo, signatureIconIndex);
}

void
ModSfsListCtrl::OnListItemActivated(wxListEvent &event)
{
	if (sfsCtrl_ == 0)
		return;

	int idx = event.GetIndex();

	SfsDirectory &dir = sfsCtrl_->getSfsDirectory();
	SfsEntry *entry = dir.getEntry(idx);

	/* Display details-dialog */
	ModSfsDetailsDlg dlg(entry, this);
	dlg.ShowModal();
}

void
ModSfsListCtrl::OnListItemRightClicked(wxListEvent &event)
{
	/* Show popup-menu */
	if (sfsCtrl_ == 0)
		return;

	currentSelection_ = event.GetIndex();
	int idx = event.GetIndex();

	SfsDirectory &dir = sfsCtrl_->getSfsDirectory();
	SfsEntry *entry = dir.getEntry(idx);

	wxMenuItemList &menuItems = popupMenu_.GetMenuItems();

	/* Update menu-items of the popup-menu */
	menuItems[0]->Enable(true); /* Show details */
	menuItems[1]->Enable(entry->isSymlink()); /* Resolve symlink */
	PopupMenu(&popupMenu_, ScreenToClient(wxGetMousePosition()));
}

void
ModSfsListCtrl::OnPopupShowDetailsSelected(wxCommandEvent &)
{
	if (sfsCtrl_ == 0)
		return;

	if (currentSelection_ == -1)
		return;

	SfsDirectory &dir = sfsCtrl_->getSfsDirectory();
	SfsEntry *entry = dir.getEntry(currentSelection_);

	/* Display details-dialog */
	ModSfsDetailsDlg dlg(entry, this);
	dlg.ShowModal();
}

void
ModSfsListCtrl::OnPopupResolveLinkSelected(wxCommandEvent &)
{
	if (sfsCtrl_ == 0)
		return;

	if (currentSelection_ == -1)
		return;

	SfsDirectory &dir = sfsCtrl_->getSfsDirectory();
	SfsEntry *entry = dir.getEntry(currentSelection_);

	wxString path = entry->resolve();
	if (path.IsEmpty())
		return;

	/* Select the directory */
	wxFileName fn(path);
	sfsCtrl_->setPath(fn.GetPath());

	/* At least select the filename in the list */
	int idx = dir.getIndexOf(path);
	if (idx == -1)
		return;

	SetItemState(idx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	EnsureVisible(idx);
}

void
ModSfsListCtrl::scanFinished(bool)
{
	if (scanProgressDlg_ != 0) {
		delete scanProgressDlg_;
		scanProgressDlg_ = 0;
	}
}

void
ModSfsListCtrl::scanUpdate(unsigned int visited, unsigned int total)
{
	/* Create the progress-dialog, if not already created. */
	if (scanProgressDlg_ == 0) {
		scanProgressDlg_ = new wxProgressDialog(
		    _("Filesystem is scanned..."),
		    _("Please wait while the filesystem is scanned."),
		    total, this, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_CAN_ABORT);
	}

	/* Update progress-dialog */
	bool accepted = scanProgressDlg_->Update(visited);
	if (!accepted) {
		/* Cancel pressed, abort filesystem-scan */
		SfsDirectory &dir = sfsCtrl_->getSfsDirectory();
		dir.abortScan();
	}
}

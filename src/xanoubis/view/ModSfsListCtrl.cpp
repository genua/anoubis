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

#include <wx/filename.h>

#include <AnListColumn.h>
#include <SfsCtrl.h>
#include <SfsEntry.h>

#include "ModSfsDetailsDlg.h"
#include "ModSfsListCtrl.h"
#include "ModSfsListProperty.h"

ModSfsListCtrl::ModSfsListCtrl(wxWindow *parent, wxWindowID id,
    const wxPoint &pos, const wxSize &size, long style)
    : AnListCtrl(parent, id, pos, size, style), SfsDirectoryScanHandler(40)
{
	sfsCtrl_ = 0;
	currentSelection_ = -1;

	/* Add columns to list */
	AnListColumn *col;

	col = addColumn(new ModSfsListProperty(ModSfsListProperty::PATH));
	col->setWidth(360);

	addColumn(new ModSfsListProperty(ModSfsListProperty::CHECKSUM));
	addColumn(new ModSfsListProperty(ModSfsListProperty::SIGNATURE));

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

		setRowProvider(&dir); /* Assign row-provider */
	} else {
		setRowProvider(0); /* Remove row-provider */

		this->sfsCtrl_ = 0;
	}
}

IndexArray
ModSfsListCtrl::getSelectedIndexes(void) const
{
	int		selection = -1;
	IndexArray	selectionArray;

	while (true) {
		selection = getNextSelection(selection);

		if (selection == -1) {
			/* No selection */
			break;
		}

		selectionArray.Add(selection);
	}

	return (selectionArray);
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

	if (event.GetIndex() == -1)
		return; /* No item selected */

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
		    _("Scanning filesystem..."),
		    _("Please wait while the filesystem is being scanned."),
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

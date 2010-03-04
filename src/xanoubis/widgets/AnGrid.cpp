/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include <wx/defs.h>	/* mandatory but missing in choicdlg.h */
#include <wx/choicdlg.h>
#include <wx/dynarray.h>

#include "AnGrid.h"
#include "AnTable.h"

AnGrid::AnGrid(wxWindow *parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style, const wxString& name) : wxGrid(
    parent, id, pos, size, style, name)
{
	isCursorVisible_ = true;

	Connect(wxEVT_GRID_LABEL_RIGHT_CLICK, wxGridEventHandler(
	    AnGrid::onLabelRightClick), NULL, this);
	Connect(wxEVT_GRID_COL_SIZE, wxGridSizeEventHandler(
	    AnGrid::onColumnSize), NULL, this);
}

AnGrid::~AnGrid(void)
{
	Disconnect(wxEVT_GRID_LABEL_RIGHT_CLICK, wxGridEventHandler(
	    AnGrid::onLabelRightClick), NULL, this);
	Disconnect(wxEVT_GRID_COL_SIZE, wxGridSizeEventHandler(
	    AnGrid::onColumnSize), NULL, this);
}

void
AnGrid::setCursorVisibility(bool isVisible)
{
	isCursorVisible_ = isVisible;
}

bool
AnGrid::isCursorVisible(void) const
{
	return (isCursorVisible_);
}

void
AnGrid::DrawCellHighlight(wxDC &dc, const wxGridCellAttr *attribute)
{
	if (isCursorVisible_) {
		wxGrid::DrawCellHighlight(dc, attribute);
	}
}

void
AnGrid::onLabelRightClick(wxGridEvent &)
{
	size_t			 i;
	wxArrayString		 choices;
	wxArrayInt		 selections;
	AnTable			*table;
	wxMultiChoiceDialog	*multiChoiceDlg;

	table = wxDynamicCast(GetTable(), AnTable);
	if (table == NULL) {
		return;
	}

	/* Assemble information for choice dialog. */
	choices = table->getColumnName();
	for (i=0; i<table->getColumnCount(); i++) {
		if (table->isColumnVisible(i)) {
			selections.Add(i);
		}
	}
	multiChoiceDlg = new wxMultiChoiceDialog(this, _("Table columns"),
	    _("Please select the columns to be shown"), choices);
	multiChoiceDlg->SetSelections(selections);
	multiChoiceDlg->SetSize(400,350);
	multiChoiceDlg->Layout();

	if (multiChoiceDlg->ShowModal() == wxID_OK) {
		selections = multiChoiceDlg->GetSelections();
		for (i=0; i<table->getColumnCount(); i++) {
			if (selections.Index(i) == wxNOT_FOUND) {
				table->setColumnVisability(i, false);
			} else {
				table->setColumnVisability(i, true);
			}
		}
		table->assignColumnWidth();
		ForceRefresh();
	}

	multiChoiceDlg->Destroy();
}

void
AnGrid::onColumnSize(wxGridSizeEvent & event)
{
	int	 idx;
	AnTable	*table;

	event.Skip();

	idx = event.GetRowOrCol();
	table = wxDynamicCast(GetTable(), AnTable);
	if (table == NULL) {
		return;
	}

	table->setColumnWidth(idx, GetColSize(idx));
}

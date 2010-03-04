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

#include "AnTable.h"


AnTable::AnTable(const wxString & configPath)
{
	configPath_  = configPath;
	rowProvider_ = NULL;
}

AnTable::~AnTable(void)
{
	AnGridColumn	*column = NULL;

	rowProvider_ = NULL;
	visibilityList_.clear();
	/* clear() does not destroy object, thus we do it by hand. */
	while (!columnList_.empty()) {
		column = columnList_.back();
		columnList_.pop_back();
		delete column;
	}
}

wxArrayString
AnTable::getColumnName(void) const
{
	wxArrayString					 list;
	std::vector<AnGridColumn *>::const_iterator	 it;
	AnListProperty					*property;

	for (it=columnList_.begin(); it!=columnList_.end(); it++) {
		property = (*it)->getProperty();
		list.Add(property->getHeader());
	}

	return (list);
}

void
AnTable::addProperty(AnListProperty *property, int width)
{
	wxString	 key;
	AnGridColumn	*column;

	if (!configPath_.EndsWith(wxT("/"))) {
		configPath_.Append(wxT("/"));
	}
	key = wxString::Format(wxT("%ls%d/"), configPath_.c_str(),
	    columnList_.size());
	column = new AnGridColumn(property, key, width);

	columnList_.push_back(column);
	if (column->isVisible()) {
		visibilityList_.push_back(columnList_.size() - 1);
		fireRowsInserted((*visibilityList_.end()), 1);
	}
}

size_t
AnTable::getColumnCount(void) const
{
	return (columnList_.size());
}

bool
AnTable::isColumnVisible(unsigned int idx) const
{
	if (idx >= columnList_.size()) {
		return (false);
	}
	return (columnList_[idx]->isVisible());
}

void
AnTable::setColumnVisability(unsigned int idx, bool visibility)
{
	std::vector<int>::iterator	it;

	if (idx >= columnList_.size()) {
		return;
	}

	if (visibility == columnList_[idx]->isVisible()) {
		/* Nothing to do. */
		return;
	}

	if (!visibility) {
		/* Visability turn off. Remove column. */
		for (it=visibilityList_.begin();
		     it!=visibilityList_.end();
		     it++) {
			if ((*it) == (int)idx) {
				columnList_[idx]->setVisibility(visibility);
				visibilityList_.erase(it);
				fireColsRemoved((*it), 1);
				return;
			}
		}
	} else {
		/* Visability turn on. Add column. */
		columnList_[idx]->setVisibility(visibility);
		for (it=visibilityList_.begin();
		     it!=visibilityList_.end();
		     it++) {
			if ((*it) > (int)idx) {
				visibilityList_.insert(it, idx);
				fireColsInserted((*it), 1);
				return;
			}
		}
		/* Didn't find matching element within list. Appending... */
		it = visibilityList_.end();
		visibilityList_.insert(it, idx);
		fireColsInserted((*it), 1);
	}
}

void
AnTable::setColumnWidth(unsigned int idx, int width)
{
	if (idx >= columnList_.size()) {
		return;
	}
	columnList_[idx]->setWidth(width);
}

void
AnTable::assignColumnWidth(void) const
{
	int	 width	= 0;
	wxGrid	*grid	= NULL;

	grid = GetView();
	if (grid == NULL) {
		return;
	}

	grid->BeginBatch();
	for (unsigned int i=0; i<visibilityList_.size(); i++) {
		width = columnList_[visibilityList_.at(i)]->getWidth();
		grid->SetColSize(i, width);
	}
	grid->EndBatch();
}

void
AnTable::setRowProvider(AnRowProvider *rowProvider)
{
	wxGrid	*grid;
	int	 oldSize;
	int	 newSize;

	rowProvider_ = rowProvider;
	grid = GetView();

	if (grid == NULL) {
		return;
	}

	oldSize = grid->GetNumberRows();
	newSize = rowProvider_->getSize();

	if (oldSize == newSize) {
		/* Nothing to do. */
		grid->ForceRefresh();
		return;
	}

	if (oldSize < newSize) {
		fireRowsInserted(0, (newSize - oldSize));
	} else {
		fireRowsRemoved(0, (oldSize - newSize));
	}

	grid->ForceRefresh();
}

AnRowProvider *
AnTable::getRowProvider(void) const
{
	return (rowProvider_);
}

int
AnTable::GetNumberRows(void)
{
	int	size;

	if (rowProvider_ != NULL) {
		size = rowProvider_->getSize();
	} else {
		size = 0;
	}

	return (size);
}

int
AnTable::GetNumberCols(void)
{
	return (visibilityList_.size());
}

wxString
AnTable::GetColLabelValue(int i)
{
	unsigned int	 idx = 0;
	AnListProperty	*property = NULL;

	if ((i < 0) || (i >= (int)visibilityList_.size())) {
		return (wxEmptyString);
	}
	idx = visibilityList_[i];

	if (idx >= columnList_.size()) {
		return (wxEmptyString);
	}
	property = columnList_[idx]->getProperty();

	return (property->getHeader());
}

bool
AnTable::IsEmptyCell(int row, int col)
{
	wxString cell;

	cell = GetValue(row, col);

	if ((cell == wxEmptyString) || (cell == _("???"))) {
		return (true);
	} else {
		return (false);
	}
}

wxString
AnTable::GetValue(int row, int col)
{
	int		 idx  = -1;
	AnListClass	*item = NULL;
	AnListProperty	*property = NULL;

	if ((row < 0) || (row > GetNumberRows())) {
		return (_("???"));
	}
	if ((col < 0) || (col > (int)visibilityList_.size())) {
		return (_("???"));
	}
	if (rowProvider_ == NULL) {
		return (_("???"));
	}

	idx  = visibilityList_[col];
	item = rowProvider_->getRow(row);
	property = columnList_[idx]->getProperty();

	return (property->getText(item));
}

void
AnTable::SetValue(int, int, const wxString &)
{
	/* Our tables are read only. */
}

void
AnTable::fireRowsInserted(int start, int count)
{
	if (GetView()) {
		wxGridTableMessage msg(
		    this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED, start, count);
		GetView()->ProcessTableMessage(msg);
	}
}

void
AnTable::fireRowsRemoved(int start, int count)
{
	if (GetView()) {
		wxGridTableMessage msg(
		    this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, start, count);
		GetView()->ProcessTableMessage(msg);
	}
}

void
AnTable::fireColsInserted(int start, int count)
{
	if (GetView()) {
		wxGridTableMessage msg(
		    this, wxGRIDTABLE_NOTIFY_COLS_INSERTED, start, count);
		GetView()->ProcessTableMessage(msg);
	}
}

void
AnTable::fireColsRemoved(int start, int count)
{
	if (GetView()) {
		wxGridTableMessage msg(
		    this, wxGRIDTABLE_NOTIFY_COLS_DELETED, start, count);
		GetView()->ProcessTableMessage(msg);
	}
}

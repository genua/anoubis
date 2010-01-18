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

#include "AnListClass.h"
#include "AnListClassProperty.h"
#include "AnListColumn.h"
#include "AnListCtrl.h"
#include "AnListProperty.h"

#include "AnRowProvider.h"
#include "AnRowViewer.h"

AnListCtrl::AnListCtrl(wxWindow *parent, wxWindowID id, const wxPoint &pos,
    const wxSize &size, long style, const wxValidator &validator,
    const wxString &name)
    : wxListCtrl(parent, id, pos, size, style, validator, name)
{
	this->stateKey_ = wxEmptyString;
	this->rowProperty_ = 0;
	this->itemAttr_ = new wxListItemAttr;
	hasSelectionResult_ = 0;
	/*
	 * XXX CEH: Implement a default provider with the contents of the
	 * XXX CEH: old rows_ and reverserows_ implementation.
	 */
	rowProvider_ = NULL;

	SetImageList(AnIconList::getInstance(), wxIMAGE_LIST_SMALL);
}

void
AnListCtrl::setRowProvider(AnRowProvider *provider)
{
	if (provider)
		provider->setViewer(this);
	rowProvider_ = provider;
}

AnListCtrl::~AnListCtrl(void)
{
	if (this->rowProperty_ != 0)
		delete this->rowProperty_;

	delete this->itemAttr_;

	while (!columnList_.empty()) {
		AnListColumn *col = columnList_.back();
		columnList_.pop_back();

		delete col;
	}
}

wxString
AnListCtrl::getStateKey(void) const
{
	return (this->stateKey_);
}

void
AnListCtrl::setStateKey(const wxString &key)
{
	this->stateKey_ = key;
}

AnListColumn *
AnListCtrl::addColumn(AnListProperty *property)
{
	AnListColumn *col = new AnListColumn(property, this);
	columnList_.push_back(col);

	return (col);
}

AnListColumn *
AnListCtrl::getColumn(unsigned int idx) const
{
	if (idx < columnList_.size())
		return (columnList_[idx]);
	else
		return (0);
}

bool
AnListCtrl::removeColumn(unsigned int idx)
{
	if (idx < columnList_.size()) {
		std::vector<AnListColumn *>::iterator it = columnList_.begin();
		it += idx;

		AnListColumn *col = (*it);
		columnList_.erase(it);
		delete col;

		/* Adapt position of any following columns */
		for (unsigned int i = idx; i < columnList_.size(); i++) {
			AnListColumn *col = columnList_[i];
			col->setIndex(col->getIndex() - 1);
		}

		return (true);
	} else
		return (false);
}

unsigned int
AnListCtrl::getColumnCount(void) const
{
	return (columnList_.size());
}

unsigned int
AnListCtrl::getRowCount(void) const
{
	return rowProvider_->getSize();
}

bool
AnListCtrl::isColumnVisible(const AnListColumn *col) const
{
	for (unsigned int idx = 0; idx < visibleColumns_.size(); idx++) {
		if (visibleColumns_[idx] == col->getIndex())
			return (true);
	}

	return (false);
}

void
AnListCtrl::setColumnVisible(AnListColumn *col, bool visible)
{
	if (isColumnVisible(col) == visible) {
		/* Nothing to do */
		return;
	}

	if (visible) {
		/* Insert into list of visible columns */
		unsigned int idx = insertVisible(col->getIndex());

		/* Insert column into view */
		InsertColumn(idx, col->columnInfo_);
	} else {
		/* Remove from list of visible columns */
		int idx = removeVisible(col->getIndex());

		/* Remove column from view */
		if (idx >= 0)
			DeleteColumn(idx);
	}
}

AnListClassProperty *
AnListCtrl::getRowProperty(void) const
{
	return this->rowProperty_;
}

void
AnListCtrl::setRowProperty(AnListClassProperty *property)
{
	this->rowProperty_ = property;
}

bool
AnListCtrl::hasSelection(void)
{
	int	result;

	/*
	 * Search for a selected item starting immediately before the
	 * last item that was found to be selected but do not start after
	 * the end of the list.
	 */
	if (hasSelectionResult_ >= GetItemCount())
		hasSelectionResult_ = 0;
	result = getNextSelection(hasSelectionResult_ - 1);
	/*
	 * If no selected item was found and the search did not start
	 * at the beginning of the list, restart from the beginning and
	 * erase the saved value in hasSelectionResult_.
	 */
	if (result == -1 && hasSelectionResult_ != 0) {
		hasSelectionResult_ = 0;
		result = getFirstSelection();
	}
	/*
	 * If we found a selected item save its index in hasSelectionResult_.
	 */
	if (result >= 0) {
		hasSelectionResult_ = result;
		return true;
	}
	return false;
}

int
AnListCtrl::getFirstSelection(void) const
{
	return (GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED));
}

int
AnListCtrl::getNextSelection(int previous) const
{
	return (GetNextItem(previous, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED));
}

wxString
AnListCtrl::OnGetItemText(long row, long column) const
{
	unsigned int columnIndex = visibleColumns_[column];
	AnListColumn *col = getColumn(columnIndex);

	if (col != 0) {
		AnListProperty *property = col->getProperty();
		AnListClass *item = rowProvider_->getRow(row);

		if (property && item)
			return property->getText(item);
	}
	return wxEmptyString;
}

int
AnListCtrl::OnGetItemColumnImage(long row, long column) const
{
	unsigned int columnIndex = visibleColumns_[column];
	AnListColumn *col = getColumn(columnIndex);

	if (col != 0) {
		AnListProperty *property = col->getProperty();
		AnListClass *item = rowProvider_->getRow(row);

		if (property && item)
			return property->getIcon(item);
	}
	return (AnIconList::ICON_NONE);
}

wxListItemAttr *
AnListCtrl::OnGetItemAttr(long row) const
{
	if (rowProperty_ != 0) {
		AnListClass *item = rowProvider_->getRow(row);

		if (item != 0) {
			/*
			 * XXX CEH: Nur ein itemAttr fuer alle Zeilen,
			 * XXX CEH: ob das wirklich gut geht???
			 */
			itemAttr_->SetTextColour(rowProperty_->getColor(item));
			itemAttr_->SetBackgroundColour(
			    rowProperty_->getBackgroundColor(item));
			itemAttr_->SetFont(rowProperty_->getFont(item));

			return itemAttr_;
		}
	}

	return (0);
}

unsigned int
AnListCtrl::insertVisible(unsigned int value)
{
	/* Insert into list of visible columns. Take care of correct order! */
	for (unsigned int idx = 0; idx < visibleColumns_.size(); idx++) {
		if (visibleColumns_[idx] > value) {
			std::vector<unsigned int>::iterator it =
			    visibleColumns_.begin();
			it += idx;

			visibleColumns_.insert(it, value);
			return (idx);
		}
	}

	visibleColumns_.push_back(value);
	return (visibleColumns_.size() - 1);
}

int
AnListCtrl::removeVisible(unsigned int value)
{
	for (unsigned int idx = 0; idx < visibleColumns_.size(); idx++) {
		if (visibleColumns_[idx] == value) {
			std::vector<unsigned int>::iterator it =
			    visibleColumns_.begin();
			it += idx;

			visibleColumns_.erase(it);

			return (idx);
		}
	}

	return (-1);
}

long
AnListCtrl::getSelectedIndex(void)
{
	return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}


/*
 * AnRowViewer implementation
 */

void
AnListCtrl::changeSize(int newSize)
{
	SetItemCount(newSize);
}

void
AnListCtrl::updateRows(int from, int to)
{
	if (to == -1) {
		int	newSize;

		if (rowProvider_ == NULL)
			return;
		newSize = rowProvider_->getSize();
		changeSize(newSize);
		to = newSize-1;
	}
	if (0 <= from && from <= to)
		RefreshItems(from, to);
}

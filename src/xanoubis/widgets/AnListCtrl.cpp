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
#include "AnListColumn.h"
#include "AnListCtrl.h"
#include "AnListProperty.h"
#include "Subject.h"

AnListCtrl::AnListCtrl(wxWindow *parent, wxWindowID id, const wxPoint &pos,
    const wxSize &size, long style, const wxValidator &validator,
    const wxString &name)
    : wxListCtrl(parent, id, pos, size, style, validator, name), Observer(0)
{
	SetImageList(AnIconList::getInstance(), wxIMAGE_LIST_SMALL);
}

AnListCtrl::~AnListCtrl(void)
{
	while (!columnList_.empty()) {
		AnListColumn *col = columnList_.back();
		columnList_.pop_back();

		delete col;
	}
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

const std::vector<AnListClass *>
AnListCtrl::getRowData(void) const
{
	return (this->rows_);
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

void
AnListCtrl::setRowData(const std::vector<AnListClass *> &rowData)
{
	clearRows();

	rows_.reserve(rowData.size());
	std::vector<AnListClass *>::const_iterator it = rowData.begin();
	for (; it != rowData.end(); ++it) {
		AnListClass *c = (*it);
		addSubject(c); /* Register at observer */

		rows_.push_back(c);
		reverseRows_[c] = rows_.size() - 1; /* Last index */
	}

	SetItemCount(rows_.size());
	refreshVisible();
}

void
AnListCtrl::setRowData(const std::list<AnListClass *> &rowData)
{
	clearRows();

	rows_.reserve(rowData.size());
	std::list<AnListClass *>::const_iterator it = rowData.begin();
	for (; it != rowData.end(); ++it) {
		AnListClass *c = (*it);
		addSubject(c); /* Register at observer */

		rows_.push_back(c);
		reverseRows_[c] = rows_.size() - 1; /* Last index */
	}

	SetItemCount(rows_.size());
	refreshVisible();
}

void
AnListCtrl::addRow(AnListClass *row)
{
	rows_.push_back(row);
	reverseRows_[row] = rows_.size() - 1; /* Last index of the vector */

	addSubject(row); /* Register at observer */

	SetItemCount(rows_.size());
	refreshVisible();
}

void
AnListCtrl::addRow(AnListClass *row, unsigned int before)
{
	if (before < rows_.size()) {
		std::vector<AnListClass *>::iterator it = rows_.begin();
		it += before;

		rows_.insert(it, row);
		reverseRows_[row] = before;

		addSubject(row); /* Register at observer */

		/* Any following rows are advanced by one position */
		for (unsigned int i = before + 1; i < rows_.size(); i++) {
			AnListClass *c = rows_[i];
			reverseRows_[c] = reverseRows_[c] + 1;
		}

		SetItemCount(rows_.size());
		refreshVisible();
	} else {
		/* Out of range, append to the end */
		addRow(row);
	}
}

bool
AnListCtrl::removeRow(unsigned int idx)
{
	if (idx < rows_.size()) {
		removeSubject(rows_[idx]); /* Deregister from observer */

		std::vector<AnListClass *>::iterator it = rows_.begin();
		it += idx;

		rows_.erase(it);

		for (unsigned int i = idx; i < rows_.size(); i++) {
			AnListClass *c = rows_[idx];
			reverseRows_[c] = reverseRows_[c] - 1;
		}

		SetItemCount(rows_.size());
		refreshVisible();

		return (true);
	} else
		return (false);
}

bool
AnListCtrl::removeRows(unsigned int first, unsigned int last)
{
	if ((first <= last) && (last < rows_.size())) {
		/* Deregister from observer */
		for (unsigned int idx = first; idx <= last; idx++) {
			removeSubject(rows_[idx]);
		}

		std::vector<AnListClass *>::iterator it1 = rows_.begin();
		std::vector<AnListClass *>::iterator it2 = rows_.begin();

		it1 += first;
		it2 += last;

		rows_.erase(it1, it2);

		for (unsigned int idx = last; idx < rows_.size(); idx++) {
			AnListClass *c = rows_[idx];
			reverseRows_[c] = reverseRows_[c] - 1;
		}

		SetItemCount(rows_.size());
		refreshVisible();

		return (true);
	} else
		return (false);
}

void
AnListCtrl::clearRows(void)
{
	while (!rows_.empty()) {
		AnListClass *c = rows_.back();
		rows_.pop_back();

		removeSubject(c); /* Deregister from observer */
	}

	reverseRows_.clear();

	SetItemCount(rows_.size());
	refreshVisible();
}

AnListClass *
AnListCtrl::getRowAt(unsigned int idx) const
{
	if (idx < rows_.size())
		return rows_[idx];
	else
		return (0);
}

int
AnListCtrl::getRowIndex(AnListClass *row) const
{
	if (reverseRows_.count(row) > 0) {
		std::map<AnListClass *, unsigned int>::const_iterator it =
		    reverseRows_.find(row);

		return ((*it).second);
	} else
		return (-1);
}

void
AnListCtrl::refreshVisible(void)
{
	long first = GetTopItem();
	long last = first + GetCountPerPage() - 1;
	long index = getSelectedIndex();

	RefreshItems(first, last);

	if (index != -1) {
		SetItemState(index, wxLIST_STATE_SELECTED,
		    wxLIST_STATE_SELECTED);
	}

	EnsureVisible(first);
	EnsureVisible(last);

	if (index != -1)
		EnsureVisible(index);
}

wxString
AnListCtrl::OnGetItemText(long item, long column) const
{
	unsigned int columnIndex = visibleColumns_[column];
	AnListColumn *col = getColumn(columnIndex);

	if (col != 0) {
		AnListProperty *property = col->getProperty();

		if (property != 0) {
			AnListClass *c = rows_[item];
			return (property->getText(c));
		}
	}

	return (wxEmptyString);
}

int
AnListCtrl::OnGetItemColumnImage(long item, long column) const
{
	unsigned int columnIndex = visibleColumns_[column];
	AnListColumn *col = getColumn(columnIndex);

	if (col != 0) {
		AnListProperty *property = col->getProperty();

		if (property != 0) {
			AnListClass *c = rows_[item];
			return (property->getIcon(c));
		}
	}

	return (AnIconList::ICON_NONE);
}

void
AnListCtrl::update(Subject *subject)
{
	AnListClass *c = wxDynamicCast(subject, AnListClass);

	if (c != 0) {
		refreshVisible();
	}
}

void
AnListCtrl::updateDelete(Subject *subject)
{
	AnListClass *c = wxDynamicCast(subject, AnListClass);

	if (c != 0) {
		int idx = getRowIndex(c);

		if (idx != -1) {
			/*
			 * AnListClass-instance removed,
			 * remove also from view.
			 */
			removeRow(idx);
		}
	}
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

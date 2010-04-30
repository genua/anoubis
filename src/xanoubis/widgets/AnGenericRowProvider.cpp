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

#include "AnGenericRowProvider.h"

AnGenericRowProvider::AnGenericRowProvider(void) : Observer(NULL)
{
	rows_.clear();
	reverseRows_.clear();
}

AnGenericRowProvider::~AnGenericRowProvider(void)
{
	unsigned int	i;
	for (i=0; i<rows_.size(); ++i) {
		removeSubject(rows_[i]);
	}
	rows_.clear();
	reverseRows_.clear();
}

void
AnGenericRowProvider::setRowData(const std::vector<AnListClass *> &rowData)
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
	rowChangeEvent(0, -1);
}

void
AnGenericRowProvider::setRowData(const std::list<AnListClass *> &rowData)
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
	rowChangeEvent(0, -1);
}

void
AnGenericRowProvider::addRow(AnListClass *row)
{
	int	idx = rows_.size();

	rows_.push_back(row);
	reverseRows_[row] = idx;
	addSubject(row); /* Register at observer */
	rowChangeEvent(idx, -1);
}

void
AnGenericRowProvider::addRow(AnListClass *row, unsigned int before)
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

		rowChangeEvent(before, -1);
	} else {
		/* Out of range, append to the end */
		addRow(row);
	}
}

bool
AnGenericRowProvider::removeRow(unsigned int idx)
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

		rowChangeEvent(idx, -1);

		return (true);
	} else
		return (false);
}

bool
AnGenericRowProvider::removeRows(unsigned int first, unsigned int last)
{
	int	diff = last - first  + 1;
	if (diff > 0 && last < rows_.size()) {
		/* Deregister from observer */
		for (unsigned int idx = first; idx <= last; idx++) {
			removeSubject(rows_[idx]);
		}

		std::vector<AnListClass *>::iterator it1 = rows_.begin();
		std::vector<AnListClass *>::iterator it2 = rows_.begin();

		it1 += first;
		it2 += last;

		rows_.erase(it1, it2);

		for (unsigned int idx = first; idx < rows_.size(); idx++) {
			AnListClass *c = rows_[idx];
			reverseRows_[c] = reverseRows_[c] - diff;
		}

		rowChangeEvent(first, -1);

		return (true);
	} else
		return (false);
}

void
AnGenericRowProvider::clearRows(void)
{
	while (!rows_.empty()) {
		AnListClass *c = rows_.back();
		rows_.pop_back();

		removeSubject(c); /* Deregister from observer */
	}
	reverseRows_.clear();
	sizeChangeEvent(0);
}

int
AnGenericRowProvider::getRowIndex(AnListClass *row) const
{
	if (reverseRows_.count(row) > 0) {
		std::map<AnListClass *, unsigned int>::const_iterator it =
		    reverseRows_.find(row);

		return ((*it).second);
	} else
		return (-1);
}

AnListClass *
AnGenericRowProvider::getRow(unsigned int idx) const
{
	if (idx < rows_.size())
		return rows_[idx];
	return NULL;
}

int
AnGenericRowProvider::getSize(void) const
{
	return rows_.size();
}

void
AnGenericRowProvider::update(Subject *subject)
{
	AnListClass *c = dynamic_cast<AnListClass*>(subject);

	if (c != 0) {
		int	idx = getRowIndex(c);
		rowChangeEvent(idx, idx);
	}
}

void
AnGenericRowProvider::updateDelete(Subject *subject)
{
	AnListClass *c = dynamic_cast<AnListClass*>(subject);

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

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

#include <algorithm>

#include "AnGenericRowProvider.h"

AnGenericRowProvider::AnGenericRowProvider(void) : Observer(NULL)
{
	allrows_.clear();
	visiblerows_.clear();
	sortProperty_ = NULL;
	filterProperty_ = NULL;
	filterString_ = wxEmptyString;
}

AnGenericRowProvider::~AnGenericRowProvider(void)
{
	for (unsigned int i=0; i<allrows_.size(); ++i) {
		removeSubject(allrows_[i]);
	}
	allrows_.clear();
	visiblerows_.clear();
	/* Using the setter methods ensures that removeSubject is called: */
	setSortProperty(NULL);
	setFilterProperty(NULL);
}

void
AnGenericRowProvider::setRowData(const std::vector<AnListClass *> &rowData)
{
	clearRows();

	std::vector<AnListClass *>::const_iterator	it;
	for (it = rowData.begin(); it != rowData.end(); ++it) {
		addSubject(*it);
		allrows_.push_back(*it);
	}
	updateVisible();
}

void
AnGenericRowProvider::addRow(AnListClass *row)
{
	allrows_.push_back(row);
	addSubject(row); /* Register at observer */
	updateVisible();
}

bool
AnGenericRowProvider::removeRawRow(unsigned int idx)
{
	if (idx >= allrows_.size())
		return false;
	removeSubject(allrows_[idx]);
	allrows_.erase(allrows_.begin()+idx);
	updateVisible();
	return true;
}

void
AnGenericRowProvider::clearRows(void)
{
	for (unsigned int i=0; i<allrows_.size(); ++i)
		removeSubject(allrows_[i]);
	allrows_.clear();
	visiblerows_.clear();
	sizeChangeEvent(0);
}

AnListClass *
AnGenericRowProvider::getRow(unsigned int idx) const
{
	if (idx < visiblerows_.size())
		return allrows_[visiblerows_[idx]];
	return NULL;
}

int
AnGenericRowProvider::getSize(void) const
{
	return visiblerows_.size();
}

/* NOTE: Does not change row order. */
void
AnGenericRowProvider::update(Subject *subject)
{
	AnListClass	*row = dynamic_cast<AnListClass*>(subject);

	if (row == NULL)
		return;
	for (unsigned int i=0; i<visiblerows_.size(); ++i) {
		if (allrows_[visiblerows_[i]] == row) {
			rowChangeEvent(i, i);
			break;
		}
	}
}

void
AnGenericRowProvider::updateDelete(Subject *subject)
{
	AnListClass		*row;

	/* NOTE: Both sortProperty_ == filterProperty_ is possible! */
	if (subject == sortProperty_)
		sortProperty_ = NULL;
	if (subject == filterProperty_)
		filterProperty_ = NULL;
	row = dynamic_cast<AnListClass*>(subject);
	if (row == NULL)
		return;
	for (unsigned int idx=0; idx < allrows_.size(); ++idx) {
		if (allrows_[idx] == row) {
			removeRawRow(idx);
			break;
		}
	}
}

void
AnGenericRowProvider::setFilterProperty(AnListProperty *filter)
{
	if (filterProperty_)
		removeSubject(filterProperty_);
	filterProperty_ = filter;
	if (filterProperty_)
		addSubject(filterProperty_);
	filterString_ = wxEmptyString;
	updateVisible();
}

void
AnGenericRowProvider::setFilterString(const wxString &filter)
{
	filterString_ = filter.Lower();
	updateVisible();
}

void
AnGenericRowProvider::setSortProperty(AnListProperty *sortprop)
{
	if (sortProperty_)
		removeSubject(sortProperty_);
	sortProperty_ = sortprop;
	if (sortProperty_)
		addSubject(sortProperty_);
	updateVisible();
}

class AnGenericRowProviderComp {
private:
	const AnGenericRowProvider	&provider_;
public:
	AnGenericRowProviderComp(const AnGenericRowProvider *p)
	    : provider_(*p) { }
	bool operator() (unsigned int i, unsigned int j) const {
		return provider_(i, j);
	}
};

void
AnGenericRowProvider::updateVisible(void)
{
	AnGenericRowProviderComp	comp(this);

	visiblerows_.clear();
	/* Apply the filter (if any) */
	if (filterProperty_ && filterString_ != wxEmptyString) {
		for (unsigned int i=0; i<allrows_.size(); ++i) {
			wxString		text;

			text = (filterProperty_->getText(allrows_[i])).Lower();
			if (text.Find(filterString_.c_str()) != wxNOT_FOUND)
				visiblerows_.push_back(i);
		}
	} else {
		for (unsigned int i=0; i<allrows_.size(); ++i)
			visiblerows_.push_back(i);
	}
	if (sortProperty_)
		sort(visiblerows_.begin(), visiblerows_.end(), comp);
	rowChangeEvent(0, -1);
}

bool
AnGenericRowProvider::operator() (unsigned int i, unsigned int j) const
{
	wxString	s1, s2;

	if (sortProperty_ == NULL)
		return i<j;
	s1 = (sortProperty_->getText(allrows_[i])).Lower();
	s2 = (sortProperty_->getText(allrows_[j])).Lower();
	return (s1.Cmp(s2) < 0);
}

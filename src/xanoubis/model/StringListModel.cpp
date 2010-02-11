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

#include "StringListModel.h"

StringListModel::~StringListModel(void)
{
	clear();
}

void
StringListModel::add(const wxString &str)
{
	stringList_.push_back(new StringWrapper(str));
	sizeChangeEvent(stringList_.size());
}

void
StringListModel::add(const wxString &str, unsigned int idx)
{
	if (idx < stringList_.size())
		stringList_.insert(stringList_.begin() + idx,
		    new StringWrapper(str));
	else
		stringList_.push_back(new StringWrapper(str));

	sizeChangeEvent(stringList_.size());
}

void
StringListModel::remove(const wxString &str)
{
	int idx = find(str);

	if (idx != -1) {
		StringWrapper *wrapper = stringList_[idx];
		stringList_.erase(stringList_.begin() + idx);

		delete wrapper;

		sizeChangeEvent(stringList_.size());
	}
}

void
StringListModel::remove(unsigned int idx)
{
	if (idx < stringList_.size()) {
		StringWrapper *wrapper = stringList_[idx];
		stringList_.erase(stringList_.begin() + idx);

		delete wrapper;

		sizeChangeEvent(stringList_.size());
	}
}

void
StringListModel::clear(void)
{
	bool empty = stringList_.empty();

	while (!stringList_.empty()) {
		StringWrapper *wrapper = stringList_.back();
		stringList_.pop_back();

		delete wrapper;
	}

	if (!empty)
		sizeChangeEvent(stringList_.size());
}

int
StringListModel::find(const wxString &str) const
{
	for (unsigned int i = 0; i < stringList_.size(); i++) {
		if (stringList_[i]->str() == str)
			return (i);
	}

	return (-1);
}

unsigned int
StringListModel::count(void) const
{
	return (stringList_.size());
}

wxString
StringListModel::get(unsigned int idx) const
{
	if (idx < stringList_.size())
		return (stringList_[idx]->str());
	else
		return (wxEmptyString);
}

wxString
StringListModel::operator[](unsigned int idx) const
{
	return (get(idx));
}

AnListClass *
StringListModel::getRow(unsigned int idx) const
{
	return (idx < stringList_.size() ? stringList_[idx] : 0);
}

int
StringListModel::getSize(void) const
{
	return (count());
}

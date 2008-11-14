/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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

#include "SfsDirectory.h"

SfsDirectory::SfsDirectory()
{
	this->path_ = wxEmptyString;
	this->filter_ = wxEmptyString;
	this->inverseFilter_ = false;
}

wxString
SfsDirectory::getPath() const
{
	return (this->path_);
}

bool
SfsDirectory::setPath(const wxString &path)
{
	if (path != this->path_) {
		this->path_ = path;
		updateEntryList();

		return (true);
	}
	else
		return (false);
}

wxString
SfsDirectory::getFilter() const
{
	return (this->filter_);
}

bool
SfsDirectory::setFilter(const wxString &filter)
{
	if (filter != this->filter_) {
		this->filter_ = filter;
		updateEntryList();

		return (true);
	}
	else
		return (false);
}

bool
SfsDirectory::isFilterInversed() const
{
	return (this->inverseFilter_);
}

bool
SfsDirectory::setFilterInversed(bool inverse)
{
	if (inverse != this->inverseFilter_) {
		this->inverseFilter_ = inverse;
		updateEntryList();

		return (true);
	}
	else
		return (false);
}

unsigned int
SfsDirectory::getNumEntries() const
{
	return (entryList_.size());
}

int
SfsDirectory::getIndexOf(const wxString &filename, bool path) const
{
	for (unsigned int i = 0; i < entryList_.size(); i++) {
		if (path && (entryList_[i].getPath() == filename))
			return (i);
		if (!path && (entryList_[i].getFileName() == filename))
			return (i);
	}

	return (-1);
}

SfsEntry &
SfsDirectory::getEntry(unsigned int idx)
{
	return (entryList_[idx]);
}

void
SfsDirectory::updateEntryList()
{
	wxDir dir(this->path_);

	/* Clear list before traversion starts */
	entryList_.clear();
	dir.Traverse(*this);
}

wxDirTraverseResult
SfsDirectory::OnDir(const wxString &)
{
	/* No recursion */
	return (wxDIR_IGNORE);
}

wxDirTraverseResult
SfsDirectory::OnFile(const wxString &filename)
{
	if (this->inverseFilter_ ^ filename.Contains(this->filter_)) {
		SfsEntry entry(filename);
		entryList_.push_back(entry);
	}

	return (wxDIR_CONTINUE);
}

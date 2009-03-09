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

#include "wx/utils.h"
#include "SfsDirectory.h"

SfsDirectory::SfsDirectory()
{
	this->path_ = wxEmptyString;
	this->recursive_ = false;
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

bool
SfsDirectory::isDirTraversalEnabled(void) const
{
	return (this->recursive_);
}

bool
SfsDirectory::setDirTraversal(bool enabled)
{
	if (enabled != this->recursive_) {
		this->recursive_ = enabled;
		updateEntryList();

		return (true);
	} else
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
SfsDirectory::getIndexOf(const wxString &filename) const
{
	for (unsigned int i = 0; i < entryList_.size(); i++) {
		if (entryList_[i].getPath() == filename)
			return (i);
	}

	return (-1);
}

SfsEntry &
SfsDirectory::getEntry(unsigned int idx)
{
	return (entryList_[idx]);
}

SfsEntry &
SfsDirectory::insertEntry(const wxString &path)
{
	int idx;

	if (entryList_.empty()) {
		/*
		 * An empty list. This is easy. Append the new SfsEntry to the
		 * empty list. A list with one item is always in correct order.
		 */
		SfsEntry newEntry(path);
		entryList_.push_back(newEntry);
		idx = 0;
	} else {
		/* Find the correct position and insert the item. */
		idx = insertEntry(path, 0, entryList_.size() - 1);
	}

	return (entryList_[idx]);
}

bool
SfsDirectory::cleanup(void)
{
	std::vector<SfsEntry>::iterator it = entryList_.begin();
	int numRemoved = 0;

	while (it != entryList_.end()) {
		SfsEntry &e = (*it);

		if (!e.fileExists()) {
			/* File does not exist */
			entryList_.erase(it);
			numRemoved++;
		} else if (!isDirTraversalEnabled() &&
		    e.getRelativePath(path_).Find(wxT("/")) != wxNOT_FOUND) {
			/*
			 * Directory traversal is disabled but this entry comes
			 * from a sub-directory.
			 */
			entryList_.erase(it);
			numRemoved++;
		} else {
			/* All tests passed, skip to next entry */
			it++;
		}
	}

	/* Model has changed, if at least one entry was removed */
	return (numRemoved > 0);
}

void
SfsDirectory::updateEntryList()
{
	wxDir dir(this->path_);

	/* Clear list before traversion starts */
	entryList_.clear();
	wxBeginBusyCursor();
	dir.Traverse(*this);
	wxEndBusyCursor();
}

int
SfsDirectory::insertEntry(const wxString &path, unsigned int start,
    unsigned int end)
{
	unsigned int mid = start + (unsigned int)((end - start) / 2);
	int result = entryList_[mid].getPath().Cmp(path);

	if (start == end) {
		/*
		 * This is the position, where the SfSEntry should be inserted.
		 */
		SfsEntry newEntry(path);

		if (result > 0) {
			/*
			 * The entry at start is "greater" that the new entry.
			 * Insert it before the item at start.
			 */
			std::vector<SfsEntry>::iterator it = entryList_.begin();
			it += start; /* Advance to the correct position */

			entryList_.insert(it, newEntry);
		} else {
			/*
			 * This special case can happen, if the end of the list
			 * is reached. The new item is "greater" that the last
			 * item in the list. In the case simply append the new
			 * SfsEntry at the list.
			 */
			entryList_.push_back(newEntry);
		}

		/* This is the position, where the item was inserted */
		return (start);
	}

	if (result < 0) {
		/*
		 * The new item is "greater" than the middle item.
		 * Continue the search right to mid.
		 */
		return insertEntry(path, mid + 1, end);
	} else if (result > 0) {
		/*
		 * The middle item is "greater" than the middle item.
		 * Continue the search left to mid.
		 */
		return insertEntry(path, start, mid);
	} else {
		/* Try to insert an already existing path, abort */
		return (mid);
	}
}

wxDirTraverseResult
SfsDirectory::OnDir(const wxString &)
{
	return (this->recursive_ ? wxDIR_CONTINUE : wxDIR_IGNORE);
}

wxDirTraverseResult
SfsDirectory::OnFile(const wxString &filename)
{
	/* Search for recursive part only  */
	wxString path;

	/* ::StartsWith puts the remaining part (after path_) into path */
	filename.StartsWith(this->path_, &path);
	if (path.StartsWith(wxT("/")))
		path = path.Mid(1);

	if (this->inverseFilter_ ^ (path.Find(this->filter_) != wxNOT_FOUND)) {
		/*
		 * Insert into directory
		 * Entries are sorted in alphabetic order
		 */
		insertEntry(filename);
	}

	return (wxDIR_CONTINUE);
}

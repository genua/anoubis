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

#include <wx/dir.h>

#include "main.h"
#include "SfsEntry.h"
#include "SfsDirectory.h"

#define callHandler(method) if (scanHandler_ != 0) { scanHandler_->method; }

SfsDirectory::SfsDirectory()
{
	this->path_ = wxT("/");
	this->recursive_ = false;
	this->filter_ = wxEmptyString;
	this->inverseFilter_ = false;
	this->scanHandler_ = 0;
	this->abortScan_ = false;
	this->totalDirectories_ = 0;
	this->visitedDirectories_ = 0;
}

SfsDirectory::~SfsDirectory()
{
	removeAllEntries();
}

SfsDirectoryScanHandler *
SfsDirectory::getScanHandler(void) const
{
	return (this->scanHandler_);
}

void
SfsDirectory::setScanHandler(SfsDirectoryScanHandler *scanHandler)
{
	this->scanHandler_ = scanHandler;
}

void
SfsDirectory::abortScan(void)
{
	this->abortScan_ = true;
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
		if (entryList_[i]->getPath() == filename)
			return (i);
	}

	return (-1);
}

SfsEntry *
SfsDirectory::getEntry(unsigned int idx)
{
	return (entryList_[idx]);
}

SfsEntry *
SfsDirectory::insertEntry(const wxString &path)
{
	int idx;

	if (!canInsert(path))
		return (0);

	if (entryList_.empty()) {
		/*
		 * An empty list. This is easy. Append the new SfsEntry to the
		 * empty list. A list with one item is always in correct order.
		 */
		SfsEntry *newEntry = new SfsEntry(path);
		entryList_.push_back(newEntry);
		idx = 0;
	} else {
		/* Find the correct position and insert the item. */
		idx = insertEntry(path, 0, entryList_.size() - 1);
	}

	return (entryList_[idx]);
}

void
SfsDirectory::removeEntry(unsigned int idx)
{
	if (idx < entryList_.size()) {
		std::vector<SfsEntry *>::iterator it = entryList_.begin();
		it += idx;

		SfsEntry *entry = (*it);

		entryList_.erase(it);
		delete entry;
	}
}

void
SfsDirectory::removeAllEntries(void)
{
	while (!entryList_.empty()) {
		SfsEntry *e = entryList_.back();
		entryList_.pop_back();

		delete e;
	}
}

void
SfsDirectory::scanLocalFilesystem()
{
	wxDir dir(this->path_);

	/* Reset variables used by the scanner */
	abortScan_ = false;
	totalDirectories_ = getDirectoryCount(this->path_);
	visitedDirectories_ = 0;

	/* Scan starts now */
	callHandler(scanStarts());

	/* Clear list before traversion starts */
	removeAllEntries();
	dir.Traverse(*this);

	/* Scan is finished */
	if (!abortScan_) {
		/* 100% is reached */
		callHandler(
		    scanProgress(totalDirectories_, totalDirectories_));
	}
	callHandler(scanFinished(abortScan_));
}

bool
SfsDirectory::canInsert(const wxString &filename) const
{
	/* Search for recursive part only  */
	wxString path;

	/* ::StartsWith puts the remaining part (after path_) into path */
	filename.StartsWith(this->path_, &path);
	if (path.StartsWith(wxT("/")))
		path = path.Mid(1);

	return (this->inverseFilter_ ^
	    (path.Find(this->filter_) != wxNOT_FOUND));
}

int
SfsDirectory::insertEntry(const wxString &path, unsigned int start,
    unsigned int end)
{
	unsigned int mid = start + (unsigned int)((end - start) / 2);
	int result = entryList_[mid]->getPath().Cmp(path);

	/*
	 * Try to insert an already existing path, abort. Do this before
	 * we insert because we might not have compared the new string to
	 * entryList_[mid] yet (we do know that start == 0 or that the new
	 * element is strictly greater than start, though.
	 */
	if (result == 0)
		return mid;

	if (start == end) {
		/*
		 * This is the position, where the SfSEntry should be inserted.
		 */
		SfsEntry *newEntry = new SfsEntry(path);

		if (result > 0) {
			/*
			 * The entry at start is "greater" that the new entry.
			 * Insert it before the item at start.
			 */
			std::vector<SfsEntry *>::iterator it =
			    entryList_.begin();
			it += start; /* Advance to the correct position */

			entryList_.insert(it, newEntry);
			/* This is the position, where the item was inserted */
			return start;
		} else {
			/*
			 * This special case can happen, if the end of the list
			 * is reached. The new item is "greater" than the last
			 * item in the list. In this case simply append the new
			 * SfsEntry to the list.
			 */
			entryList_.push_back(newEntry);
			/* This is the position, where the item was inserted */
			return  entryList_.size() - 1;
		}
	}

	if (result < 0) {
		/*
		 * The new item is "greater" than the middle item.
		 * Continue the search right to mid.
		 */
		return insertEntry(path, mid + 1, end);
	} else {	/* result > 0 */
		/*
		 * The middle item is "greater" than the middle item.
		 * Continue the search left to mid.
		 */
		return insertEntry(path, start, mid);
	}
}

wxDirTraverseResult
SfsDirectory::OnDir(const wxString &dir)
{
	if (abortScan_) {
		/* Abort requested, stop traversing */
		return (wxDIR_STOP);
	}

	if (this->recursive_) {
		/* Number of directories, you will visit later */
		totalDirectories_ += getDirectoryCount(dir);
	}

	/* A progress: visitedDirectories_ [0 ... totalDirectories_ - 1] */
	callHandler(scanProgress(visitedDirectories_, totalDirectories_));
	visitedDirectories_++;

	return (this->recursive_ ? wxDIR_CONTINUE : wxDIR_IGNORE);
}

wxDirTraverseResult
SfsDirectory::OnFile(const wxString &filename)
{
	if (abortScan_) {
		/* Abort requested, stop traversing */
		return (wxDIR_STOP);
	}

	/*
	 * Insert into directory
	 * Entries are sorted in alphabetic order
	 */
	insertEntry(filename);
	wxGetApp().ProcessPendingEvents();

	return (wxDIR_CONTINUE);
}

unsigned int
SfsDirectory::getDirectoryCount(const wxString &path)
{
	wxDir dir(path);
	wxString filename;
	unsigned int count = 0;

	if (dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS|wxDIR_HIDDEN))
		count = 1;
	else
		return (count);

	while (dir.GetNext(&filename))
		count++;

	return (count);
}

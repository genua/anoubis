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

int
SfsEntryList::Cmp(const SfsEntry *a, const SfsEntry *b) const
{
	return a->getPath().Cmp(b->getPath());
}

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
	SfsEntry tmp(filename);
	return entryList_.index_of(&tmp);
}

SfsEntry *
SfsDirectory::getEntry(unsigned int idx)
{
	return entryList_.get(idx);
}

SfsEntry *
SfsDirectory::insertEntry(const wxString &path)
{
	if (!canInsert(path))
		return (0);

	SfsEntry *newEntry = new SfsEntry(path);
	if (entryList_.insert(newEntry)) {
		return newEntry;
	} else {
		SfsEntry	*ret = entryList_.find(newEntry);
		delete newEntry;
		return ret;
	}
}

void
SfsDirectory::removeEntry(unsigned int idx)
{
	SfsEntry *entry = entryList_.get(idx);

	if (entry) {
		entryList_.remove(entry);
		delete entry;
	}
}

void
SfsDirectory::removeAllEntries(void)
{
	SfsEntryList::iterator	it = entryList_.begin();
	while (it != entryList_.end()) {
		SfsEntry	*e = *it;
		++it;
		entryList_.remove(e);
		delete(e);
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

wxDirTraverseResult
SfsDirectory::OnDir(const wxString &dir)
{
	if (abortScan_) {
		/* Abort requested, stop traversing */
		return (wxDIR_STOP);
	}

	if (this->recursive_) {
		struct stat fstat;
		/* Skip symlinks pointing to directories to avoid loops */
		if (lstat(dir.fn_str(), &fstat) == 0
		    && S_ISLNK(fstat.st_mode))
			return wxDIR_IGNORE;
		/* Total number of directories to scan. */
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

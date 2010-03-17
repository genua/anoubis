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


SfsDirectoryScanHandler::SfsDirectoryScanHandler(int limit)
{
	limit_ = limit;
	total_ = 0;
	levels_.clear();
	curstate_.clear();
}

SfsDirectoryScanHandler::~SfsDirectoryScanHandler(void)
{
}

void
SfsDirectoryScanHandler::scanStarts(void)
{
	total_ = 0;
	levels_.clear();
	curstate_.clear();
}

void
SfsDirectoryScanHandler::scanPush(unsigned long count)
{
	/*
	 * The +1 is needed to compensate for the implict scanProgress that
	 * is done when poping a level.
	 */
	levels_.push_back(count+1);
	curstate_.push_back(0);
	total_ += count;
}

void
SfsDirectoryScanHandler::scanProgress(unsigned long count)
{
	/*
	 * Advance the top level by at most count steps. The level completes
	 * advance the lower level by the remaining number of steps and add
	 * one for the completed level.
	 */
	while(count && levels_.size()) {
		unsigned int	idx = levels_.size() - 1;
		unsigned int	done = levels_[idx] - curstate_[idx];

		if (done > count)
			done = count;
		curstate_[idx] += done;
		if (curstate_[idx] >= levels_[idx]) {
			levels_.pop_back();
			curstate_.pop_back();
			/*
			 * We completed a pushed level. Advance the lower
			 * level by one.
			 */
			count++;
		}
		count -= done;
	}
	/*
	 * If progress bar updates are neccessary, calculate the current
	 * progress and call scanUpdate.
	 */
	if (total_ >= limit_) {
		double		percent = 0.0;
		double		unit = 1.0;
		unsigned long	steps;
		for (unsigned int idx = 0; idx < levels_.size(); ++idx) {
			unit /= (double)levels_[idx];
			percent += unit * (double)curstate_[idx];
		}
		steps = (unsigned long)(percent * 100000.0);
		scanUpdate(steps, 100000);
	}
}

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
	changeInProgress_ = 0;
}

SfsDirectory::~SfsDirectory()
{
	removeAllEntries(false);
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
	SfsEntry tmp(const_cast<SfsDirectory *>(this), filename);
	return entryList_.index_of(&tmp);
}

SfsEntry *
SfsDirectory::getEntry(unsigned int idx) const
{
	return entryList_.get(idx);
}

SfsEntry *
SfsDirectory::insertEntry(const wxString &path)
{
	if (!canInsert(path))
		return (0);

	SfsEntry *newEntry = new SfsEntry(this, path);
	if (entryList_.insert(newEntry)) {
		if (!changeInProgress_) {
			unsigned int	idx = entryList_.index_of(newEntry);
			rowChangeEvent(idx, -1);
		}
		return newEntry;
	} else {
		delete newEntry;
		return NULL;
	}
}

void
SfsDirectory::removeEntry(unsigned int idx)
{
	SfsEntry *entry = entryList_.get(idx);

	if (entry) {
		if (!changeInProgress_) {
			entryList_.remove(entry);
			rowChangeEvent(idx, -1);
		}
		delete entry;
	}
}

void
SfsDirectory::removeAllEntries(void)
{
	removeAllEntries(true);
}

void
SfsDirectory::removeAllEntries(bool sendEvent)
{
	bool empty = (entryList_.size() == 0);

	SfsEntryList::iterator	it = entryList_.begin();
	while (it != entryList_.end()) {
		SfsEntry	*e = *it;
		++it;
		entryList_.remove(e);
		delete(e);
	}

	if (!empty && sendEvent)
		sizeChangeEvent(0);
}

void
SfsDirectory::scanLocalFilesystem()
{
	wxDir dir(this->path_);

	/* Reset variables used by the scanner */
	abortScan_ = false;
	int thiscnt = getDirectoryCount(this->path_);

	/* Scan starts now */
	callHandler(scanStarts());
	callHandler(scanPush(thiscnt));

	/* Clear list before traversion starts */
	removeAllEntries(false);
	beginChange();
	dir.Traverse(*this);
	endChange();

	callHandler(scanFinished(abortScan_));
}

AnListClass *
SfsDirectory::getRow(unsigned int idx) const
{
	unsigned int size = entryList_.size();
	return (size > 0 && idx < size ? entryList_.get(idx) : 0);
}

int
SfsDirectory::getSize(void) const
{
	return (entryList_.size());
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
		    && !S_ISDIR(fstat.st_mode)) {
			callHandler(scanProgress(1));
			return wxDIR_IGNORE;
		}
		/* Total number of directories to scan. */
		callHandler(scanPush(getDirectoryCount(dir)));
	}
	callHandler(scanProgress(1));

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
	unsigned int count;

	if (!dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS|wxDIR_HIDDEN))
		return 0;

	count = 1;
	while (dir.GetNext(&filename))
		count++;

	return count;
}

void
SfsDirectory::beginChange(void)
{
	changeInProgress_++;
}

void
SfsDirectory::endChange(void)
{
	if (--changeInProgress_ == 0)
		rowChangeEvent(0, -1);
}

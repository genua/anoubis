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

#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/textfile.h>

#include <AnEvents.h>

#include "SbModel.h"

SbModel::~SbModel(void)
{
	while (!entries_.empty()) {
		SbEntry *entry = entries_.back();
		entries_.pop_back();
		delete entry;
	}
}

unsigned int
SbModel::getEntryCount(void) const
{
	return (this->entries_.size());
}

unsigned int
SbModel::getEntryCount(SbEntry::Permission permission) const
{
	unsigned int count = 0;

	for (std::vector<SbEntry *>::const_iterator it = entries_.begin();
	    it != entries_.end(); ++it) {
		SbEntry *entry = (*it);

		if (entry->hasPermission(permission))
			count++;
	}

	return (count);
}

SbEntry *
SbModel::getEntry(unsigned int idx) const
{
	if (!this->entries_.empty() && idx < this->entries_.size())
		return (this->entries_[idx]);
	else
		return (0);
}

SbEntry *
SbModel::getEntry(const wxString &path) const
{
	std::vector<SbEntry *>::const_iterator it = entries_.begin();

	/* Search for SbEntry with the given path */
	for (; it != entries_.end(); ++it) {
		SbEntry *entry = (*it);
		if (entry->getPath() == path)
			return (entry);
	}

	return (0);
}

SbEntry *
SbModel::getEntry(const wxString &path, bool create)
{
	std::vector<SbEntry *>::const_iterator it = entries_.begin();

	/* Search for SbEntry with the given path */
	for (; it != entries_.end(); ++it) {
		SbEntry *entry = (*it);
		if (entry->getPath() == path)
			return (entry);
	}

	/* No such entry, but creation was requested */
	if (create) {
		SbEntry *entry = new SbEntry(this);
		entry->setPath(path);
		entries_.push_back(entry);

		fireSbModelChanged();

		return (entry);
	}

	return (0);
}

bool
SbModel::removeEntry(unsigned int idx)
{
	if (!entries_.empty() && idx < entries_.size()) {
		SbEntry *entry = entries_[idx];
		entries_.erase(entries_.begin() + idx);

		delete entry;

		fireSbModelChanged();

		return (true);
	} else
		return (false);
}

bool
SbModel::removeEntry(const wxString &path)
{
	std::vector<SbEntry *>::iterator it = entries_.begin();

	for (; it != entries_.end(); ++it) {
		SbEntry *entry = (*it);
		if (entry->getPath() == path) {
			entries_.erase(it);
			delete entry;

			fireSbModelChanged();

			return (true);
		}
	}

	return (false);
}

bool
SbModel::canAssignDefaults(void) const
{
	wxString file = wxStandardPaths::Get().GetDataDir() +
	    wxT("/profiles/wizard/sandbox");
	return (wxFileName::IsFileReadable(file));
}

void
SbModel::assignDefaults(SbEntry::Permission permission)
{
	wxString	path;
	wxString	perm;
	wxString	line;
	wxTextFile	file;
	wxArrayString	list;

	file.Open(wxStandardPaths::Get().GetDataDir() +
	    wxT("/profiles/wizard/sandbox"));

	if (!file.IsOpened()) {
		/* sandbox defaults file does not exists - return empty list */
		return;
	}

	line = file.GetFirstLine().BeforeFirst('#');
	line.Trim(true);
	line.Trim(false);
	while (!file.Eof()) {
		if (!line.IsEmpty()) {
			/* Slice the string */
			line.Replace(wxT("\t"), wxT(" "));
			path = line.BeforeFirst(' ');
			perm = line.AfterLast(' ');

			if (comparePermission(perm, permission)) {
				/*
				 * Permission-string & SbEntry-permission
				 * matches, append path to model
				 */
				SbEntry *entry = getEntry(path, true);
				entry->default_ = true;
				entry->setPermission(permission, true);
			}
		}

		line = file.GetNextLine().BeforeFirst('#');
		line.Trim(true);
		line.Trim(false);
	}

	file.Close();
}

inline bool
SbModel::comparePermission(const wxString &ps, SbEntry::Permission p) const
{
	return ((ps.Find(wxT("r")) != wxNOT_FOUND) && (p == SbEntry::READ)) ||
	    ((ps.Find(wxT("w")) != wxNOT_FOUND) && (p == SbEntry::WRITE)) ||
	    ((ps.Find(wxT("x")) != wxNOT_FOUND) && (p == SbEntry::EXECUTE));
}

void
SbModel::fireSbModelChanged(void)
{
	wxCommandEvent event(anEVT_ROW_SIZECHANGE);
	event.SetInt(entries_.size());
	ProcessEvent(event);
}

void
SbModel::fireSbEntryChanged(SbEntry *entry)
{
	if (entry == 0)
		return;

	/* Find position of entry */
	for (unsigned int i = 0; i < entries_.size(); i++) {
		if (entries_[i] == entry) {
			/* Entry found in list, fire event */
			wxCommandEvent  event(anEVT_ROW_UPDATE);
			event.SetInt(i);
			event.SetExtraLong(i);

			ProcessEvent(event);

			return;
		}
	}
}

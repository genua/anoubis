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

#include <wx/filename.h>

#include "SfsEntry.h"

SfsEntry::SfsEntry()
{
	setPath(wxEmptyString);
	reset();
}

SfsEntry::SfsEntry(const wxString &path)
{
	setPath(path);
	reset();
}

SfsEntry::SfsEntry(const SfsEntry &other)
{
	this->path_ = other.path_;
	this->filename_ = other.filename_;
	this->haveLocalCsum_ = other.haveLocalCsum_;
	memcpy(this->localCsum_, other.localCsum_, ANOUBIS_CS_LEN);
	memcpy(this->csum_[SFSENTRY_CHECKSUM],
	    other.csum_[SFSENTRY_CHECKSUM],
	    ANOUBIS_CS_LEN);
	memcpy(this->csum_[SFSENTRY_SIGNATURE],
	    other.csum_[SFSENTRY_SIGNATURE],
	    ANOUBIS_CS_LEN);
	this->assigned_[SFSENTRY_CHECKSUM] = other.assigned_[SFSENTRY_CHECKSUM];
	this->assigned_[SFSENTRY_SIGNATURE] =
	    other.assigned_[SFSENTRY_SIGNATURE];
	this->state_[SFSENTRY_CHECKSUM] = other.state_[SFSENTRY_CHECKSUM];
	this->state_[SFSENTRY_SIGNATURE] = other.state_[SFSENTRY_SIGNATURE];
}

wxString
SfsEntry::getPath() const
{
	return (this->path_);
}

wxString
SfsEntry::getRelativePath(const wxString &basePath) const
{
	wxString path = this->path_;

	if (this->path_.StartsWith(basePath, &path)) {
		if (path.StartsWith(wxT("/")))
			path = path.Mid(1);
	}

	return (path);
}

void
SfsEntry::setPath(const wxString &path)
{
	this->path_ = path;

	int pos = path.Find('/', true);
	if (pos != wxNOT_FOUND)
		this->filename_ = path.Mid(pos + 1);
	else
		this->filename_ = path;
}

wxString
SfsEntry::getFileName(void) const
{
	return (this->filename_);
}

wxDateTime
SfsEntry::getLastModified(void) const
{
	wxDateTime dt;

	if (wxFileExists(this->path_)) {
		wxFileName fn(this->path_);
		fn.GetTimes(0, &dt, 0);
	}

	return (dt);
}

SfsEntry::ChecksumState
SfsEntry::getChecksumState(ChecksumType type) const
{
	return (state_[type]);
}

wxString
SfsEntry::getChecksum(ChecksumType type) const
{
	wxString result;

	if (assigned_[type])
		result = cs2str(csum_[type]);

	return (result);
}

bool
SfsEntry::setChecksum(ChecksumType type, const u_int8_t *cs)
{
	memcpy(csum_[type], cs, ANOUBIS_CS_LEN);
	assigned_[type] = true;

	return (validateChecksum(type));
}

bool
SfsEntry::setChecksumMissing(ChecksumType type)
{
	memset(csum_[type], 0, ANOUBIS_CS_LEN);
	assigned_[type] = false;

	if (state_[type] != SFSENTRY_MISSING) {
		state_[type] = SFSENTRY_MISSING;
		return (true);
	} else
		return (false);
}

bool
SfsEntry::setChecksumInvalid(ChecksumType type)
{
	memset(csum_[type], 0, ANOUBIS_CS_LEN);
	assigned_[type] = false;

	if (state_[type] != SFSENTRY_INVALID) {
		state_[type] = SFSENTRY_INVALID;
		return (true);
	} else
		return (false);
}

bool
SfsEntry::haveLocalCsum(void) const
{
	return (this->haveLocalCsum_);
}

wxString
SfsEntry::getLocalCsum(void) const
{
	wxString result;

	if (haveLocalCsum_)
		result = cs2str(localCsum_);

	return (result);
}

bool
SfsEntry::setLocalCsum(const u_int8_t *cs)
{
	bool c1, c2, c3;

	int result = memcmp(localCsum_, cs, ANOUBIS_CS_LEN);
	c1 = (result != 0);

	memcpy(localCsum_, cs, ANOUBIS_CS_LEN);
	haveLocalCsum_ = true;

	c2 = validateChecksum(SFSENTRY_CHECKSUM);
	c3 = validateChecksum(SFSENTRY_SIGNATURE);

	return (c1 || c2 || c3);
}

bool
SfsEntry::reset(void)
{
	bool c1, c2, c3;

	c1 = haveLocalCsum_;
	c2 = reset(SFSENTRY_CHECKSUM);
	c3 = reset(SFSENTRY_SIGNATURE);

	haveLocalCsum_ = false;
	memset(localCsum_, 0, ANOUBIS_CS_LEN);

	return (c1 || c2 || c3);
}

bool
SfsEntry::reset(ChecksumType type)
{
	memset(csum_[type], 0, ANOUBIS_CS_LEN);
	assigned_[type] = false;

	if (state_[type] != SFSENTRY_NOT_VALIDATED) {
		state_[type] = SFSENTRY_NOT_VALIDATED;
		return (true);
	} else
		return (false);
}

bool
SfsEntry::validateChecksum(ChecksumType type)
{
	ChecksumState newState = SFSENTRY_NOT_VALIDATED;

	if (haveLocalCsum_ && assigned_[type]) {
		int result = memcmp(localCsum_, csum_[type], ANOUBIS_CS_LEN);
		newState = (result == 0) ? SFSENTRY_MATCH : SFSENTRY_NOMATCH;
	}

	if (state_[type] != newState) {
		state_[type] = newState;
		return (true);
	} else
		return (false);
}

wxString
SfsEntry::cs2str(const u_int8_t *cs)
{
	wxString str;

	for (int i = 0; i < ANOUBIS_CS_LEN; i++) {
		str += wxString::Format(wxT("%2.2x"), cs[i]);
	}

	return (str);
}

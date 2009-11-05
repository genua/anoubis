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
	this->csum_[SFSENTRY_CHECKSUM] = 0;
	this->csum_[SFSENTRY_SIGNATURE] = 0;
	this->csum_[SFSENTRY_UPGRADE] = 0;

	setPath(wxEmptyString);
	reset();
}

SfsEntry::SfsEntry(const wxString &path)
{
	this->csum_[SFSENTRY_CHECKSUM] = 0;
	this->csum_[SFSENTRY_SIGNATURE] = 0;
	this->csum_[SFSENTRY_UPGRADE] = 0;

	setPath(path);
	reset();
}

SfsEntry::~SfsEntry(void)
{
	releaseChecksum(SFSENTRY_CHECKSUM);
	releaseChecksum(SFSENTRY_SIGNATURE);
	releaseChecksum(SFSENTRY_UPGRADE);
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

bool
SfsEntry::fileExists(void) const
{
	struct stat fstat;

	if (lstat(this->path_.fn_str(), &fstat) == 0)
		return (S_ISREG(fstat.st_mode) || S_ISLNK(fstat.st_mode));
	else
		return (false);
}

bool
SfsEntry::isSymlink(void) const
{
	struct stat fstat;

	if (lstat(this->path_.fn_str(), &fstat) == 0)
		return (S_ISLNK(fstat.st_mode));
	else
		return (false);
}

wxString
SfsEntry::resolve(void) const
{
	if (isSymlink()) {
		char resolved_path[PATH_MAX];

		if (realpath(path_.fn_str(), resolved_path) != 0)
			return wxString::FromAscii(resolved_path);
	}

	return (wxEmptyString);
}

bool
SfsEntry::canHaveChecksum(bool resolve) const
{
	struct stat fstat;

	if (lstat(path_.fn_str(), &fstat) == 0) {
		if (S_ISLNK(fstat.st_mode)) {
			if (resolve) {
				/* Resolve link, needs to be a regular file */
				struct stat link_stat;

				if (stat(path_.fn_str(), &link_stat) == 0)
					return (S_ISREG(link_stat.st_mode));
				else
					return (false);
			} else
				return (true);
		} else
			return (S_ISREG(fstat.st_mode));
	} else {
		return (false);
	}
}

wxDateTime
SfsEntry::getLastModified(void) const
{
	wxDateTime dt;
	struct stat fstat;

	if (stat(this->path_.fn_str(), &fstat) == 0)
		dt.Set(fstat.st_mtime);

	return (dt);
}

SfsEntry::ChecksumState
SfsEntry::getChecksumState(ChecksumType type) const
{
	return (state_[type]);
}

bool
SfsEntry::haveChecksum(ChecksumType type) const
{
	return (csum_[type] != 0);
}

bool
SfsEntry::haveChecksum(void) const
{
	return haveChecksum(SFSENTRY_CHECKSUM) ||
	    haveChecksum(SFSENTRY_SIGNATURE);
}

bool
SfsEntry::isChecksumChanged(ChecksumType type) const
{
	return (state_[type] == SFSENTRY_NOMATCH);
}

bool
SfsEntry::isChecksumChanged(void) const
{
	return isChecksumChanged(SFSENTRY_CHECKSUM) ||
	    isChecksumChanged(SFSENTRY_SIGNATURE);
}

size_t
SfsEntry::getChecksumLength(ChecksumType type) const
{
	return (csumLen_[type]);
}

size_t
SfsEntry::getChecksum(ChecksumType type, u_int8_t *csum, size_t size) const
{
	if ((csum != 0) && (csum_[type] != 0) && (size >= csumLen_[type])) {
		memcpy(csum, csum_[type], csumLen_[type]);
		return (csumLen_[type]);
	} else
		return (0);
}

wxString
SfsEntry::getChecksum(ChecksumType type) const
{
	wxString result;

	if (csum_[type] != 0)
		result = cs2str(csum_[type], csumLen_[type]);

	return (result);
}

bool
SfsEntry::setChecksum(ChecksumType type, const u_int8_t *cs, size_t size)
{
	copyChecksum(type, cs, size);

	return (validateChecksum(type));
}

bool
SfsEntry::setChecksumMissing(ChecksumType type)
{
	bool	ret = false;

	if (type == SFSENTRY_SIGNATURE)
		ret = setChecksumMissing(SFSENTRY_UPGRADE);
	releaseChecksum(type);

	if (state_[type] != SFSENTRY_MISSING) {
		state_[type] = SFSENTRY_MISSING;
		return (true);
	} else
		return ret;
}

bool
SfsEntry::setChecksumInvalid(ChecksumType type)
{
	bool	ret = false;

	if (type == SFSENTRY_SIGNATURE)
		ret = setChecksumInvalid(SFSENTRY_UPGRADE);
	releaseChecksum(type);

	if (state_[type] != SFSENTRY_INVALID) {
		state_[type] = SFSENTRY_INVALID;
		return (true);
	} else
		return ret;
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
		result = cs2str(localCsum_, ANOUBIS_CS_LEN);

	return (result);
}

bool
SfsEntry::setLocalCsum(const u_int8_t *cs)
{
	bool c1, c2, c3, c4;

	if (cs != 0) {
		int result = memcmp(localCsum_, cs, ANOUBIS_CS_LEN);
		c1 = (result != 0);

		memcpy(localCsum_, cs, ANOUBIS_CS_LEN);
		haveLocalCsum_ = true;
	} else {
		/* Reset local checksum
		 * Local checksum state has changed, if a checksum was assigned
		 */
		c1 = haveLocalCsum_;
		haveLocalCsum_ = false;
	}

	c2 = validateChecksum(SFSENTRY_CHECKSUM);
	c3 = validateChecksum(SFSENTRY_SIGNATURE);
	c4 = validateChecksum(SFSENTRY_UPGRADE);

	return (c1 || c2 || c3 || c4);
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
	bool	ret = false;

	if (type == SFSENTRY_SIGNATURE)
		ret = reset(SFSENTRY_UPGRADE);
	releaseChecksum(type);

	if (state_[type] != SFSENTRY_NOT_VALIDATED) {
		state_[type] = SFSENTRY_NOT_VALIDATED;
		return (true);
	} else
		return ret;
}

void
SfsEntry::copyChecksum(ChecksumType type, const u_int8_t *cs, size_t len)
{
	releaseChecksum(type);

	if ((cs != 0) && (len > 0)) {
		csum_[type] = (u_int8_t *)malloc(len);
		memcpy(csum_[type], cs, len);
		csumLen_[type] = len;
	}
}

void
SfsEntry::releaseChecksum(ChecksumType type)
{
	if (csum_[type] != 0) {
		free(csum_[type]);
		csum_[type] = 0;
		csumLen_[type] = 0;
	}
}

bool
SfsEntry::validateChecksum(ChecksumType type)
{
	ChecksumState newState = SFSENTRY_MISSING;

	if (haveLocalCsum_ && haveChecksum(type)) {
		int result = memcmp(localCsum_, csum_[type], ANOUBIS_CS_LEN);
		newState = (result == 0) ? SFSENTRY_MATCH : SFSENTRY_NOMATCH;
	} else if (!haveLocalCsum_ && haveChecksum(type)) {
		/* no local checksum + registered checksum := orphaned? */
		if (fileExists()) {
			newState = SFSENTRY_INVALID;
		} else {
			newState = SFSENTRY_ORPHANED;
		}
	}

	if (state_[type] != newState) {
		state_[type] = newState;
		return (true);
	} else
		return (false);
}

wxString
SfsEntry::cs2str(const u_int8_t *cs, size_t len)
{
	wxString str;

	for (unsigned int i = 0; i < len; i++) {
		str += wxString::Format(wxT("%2.2x"), cs[i]);
	}

	return (str);
}

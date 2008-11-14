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
	this->haveDaemonCsum_ = other.haveDaemonCsum_;
	memcpy(this->localCsum_, other.localCsum_, ANOUBIS_CS_LEN);
	memcpy(this->daemonCsum_, other.daemonCsum_, ANOUBIS_CS_LEN);
	this->checksumAttr_ = other.checksumAttr_;
	this->signatureAttr_ = other.signatureAttr_;
}

wxString
SfsEntry::getPath() const
{
	return (this->path_);
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

SfsEntry::ChecksumAttr
SfsEntry::getChecksumAttr() const
{
	return (this->checksumAttr_);
}

SfsEntry::SignatureAttr
SfsEntry::getSignatureAttr() const
{
	return (this->signatureAttr_);
}

bool
SfsEntry::setNoChecksum(void)
{
	/* Remove already assigned checksums */
	bool b1 = setLocalCsum(0);
	bool b2 = setDaemonCsum(0);

	return (b1 || b2); /* Track any change in the model */
}

bool
SfsEntry::setLocalCsum(const u_int8_t *cs)
{
	if (cs != 0) {
		/* Copy */
		memcpy(localCsum_, cs, ANOUBIS_CS_LEN);
		haveLocalCsum_ = true;
	}
	else {
		/* Reset */
		memset(localCsum_, 0, ANOUBIS_CS_LEN);
		haveLocalCsum_ = false;
	}

	/* Checksum has changed, update checksumAttr */
	return (updateChecksumAttr());
}

bool
SfsEntry::setDaemonCsum(const u_int8_t *cs)
{
	if (cs != 0) {
		/* Copy */
		memcpy(daemonCsum_, cs, ANOUBIS_CS_LEN);
		haveDaemonCsum_ = true;
	}
	else {
		memset(daemonCsum_, 0, ANOUBIS_CS_LEN);
		haveDaemonCsum_ = false;
	}

	/* Checksum has changed, update checksumAttr */
	return (updateChecksumAttr());
}

void
SfsEntry::reset(void)
{
	haveLocalCsum_ = false;
	haveDaemonCsum_ = false;
	memset(localCsum_, 0, ANOUBIS_CS_LEN);
	memset(daemonCsum_, 0, ANOUBIS_CS_LEN);
	checksumAttr_ = SFSENTRY_CHECKSUM_NOT_VALIDATED;
	signatureAttr_ = SFSENTRY_SIGNATURE_UNKNOWN;
}

bool
SfsEntry::updateChecksumAttr(void)
{
	ChecksumAttr newValue;

	if (haveLocalCsum_ && haveDaemonCsum_) {
		int result = memcmp(localCsum_, daemonCsum_, ANOUBIS_CS_LEN);

		if (result == 0)
			newValue = SFSENTRY_CHECKUM_MATCH;
		else
			newValue = SFSENTRY_CHECKSUM_NOMATCH;
	}
	else
		newValue = SFSENTRY_CHECKSUM_UNKNOWN;

	if (checksumAttr_ != newValue) {
		/* Attribute value has changed */
		checksumAttr_ = newValue;
		return (true);
	}
	else
		return (false);
}

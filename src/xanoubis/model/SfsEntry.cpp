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
	this->path_ = wxEmptyString;
	this->checksumAttr_ = SFSENTRY_CHECKSUM_UNKNOWN;
	this->signatureAttr_ = SFSENTRY_SIGNATURE_UNKNOWN;
}

SfsEntry::SfsEntry(const wxString &path)
{
	this->path_ = path;
	this->checksumAttr_ = SFSENTRY_CHECKSUM_UNKNOWN;
	this->signatureAttr_ = SFSENTRY_SIGNATURE_UNKNOWN;
}

SfsEntry::SfsEntry(const wxString &path, ChecksumAttr csAttr,
    SignatureAttr sigAttr)
{
	this->path_ = path;
	this->checksumAttr_ = csAttr;
	this->signatureAttr_ = sigAttr;
}

SfsEntry::SfsEntry(const SfsEntry &other)
{
	this->path_ = other.path_;
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
}

SfsEntry::ChecksumAttr
SfsEntry::getChecksumAttr() const
{
	return (this->checksumAttr_);
}

void
SfsEntry::setChecksumAttr(ChecksumAttr attr)
{
	this->checksumAttr_ = attr;
}

SfsEntry::SignatureAttr
SfsEntry::getSignatureAttr() const
{
	return (this->signatureAttr_);
}

void
SfsEntry::setSignatureAttr(SignatureAttr attr)
{
	this->signatureAttr_ = attr;
}

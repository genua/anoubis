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

#include "ApnVersion.h"

ApnVersion::ApnVersion(struct apnvm_version *version, const char *username)
{
	this->no_ = version->no;
	this->tstamp_ = version->tstamp;
	this->user_ = wxString(username, wxConvLibc);
	this->comment_ = wxString(version->comment, wxConvLibc);
	this->auto_store_ = (version->auto_store != 0);
}

ApnVersion::ApnVersion(const ApnVersion &other) : AnListClass()
{
	this->no_ = other.no_;
	this->tstamp_ = other.tstamp_;
	this->user_ = other.user_;
	this->comment_ = other.comment_;
	this->auto_store_ = other.auto_store_;
}

int
ApnVersion::getVersionNo(void) const
{
	return (this->no_);
}

wxDateTime
ApnVersion::getTimestamp(void) const
{
	return (this->tstamp_);
}

wxString
ApnVersion::getUsername(void) const
{
	return (this->user_);
}

wxString
ApnVersion::getComment(void) const
{
	return (this->comment_);
}

bool
ApnVersion::isAutoStore(void) const
{
	return (this->auto_store_);
}

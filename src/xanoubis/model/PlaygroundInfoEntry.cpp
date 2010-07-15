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

#include "PlaygroundInfoEntry.h"

PlaygroundInfoEntry::PlaygroundInfoEntry(uint64_t pgid, uid_t uid,
    wxDateTime starttime, bool isactive, unsigned int numfiles, wxString path)
{
	this->pgid_ = pgid;
	this->uid_ = uid;
	this->starttime_ = starttime;
	this->isactive_ = isactive;
	this->numfiles_ = numfiles;
	this->path_ = path;
}

uint64_t
PlaygroundInfoEntry::getPgid() const
{
	return this->pgid_;
}

uid_t
PlaygroundInfoEntry::getUid() const
{
	return this->uid_;
}

wxDateTime
PlaygroundInfoEntry::getStarttime() const
{
	return this->starttime_;
}

bool
PlaygroundInfoEntry::isActive() const
{
	return this->isactive_;
}

unsigned int
PlaygroundInfoEntry::getNumFiles() const
{
	return this->numfiles_;
}

wxString
PlaygroundInfoEntry::getPath() const
{
	return this->path_;
}

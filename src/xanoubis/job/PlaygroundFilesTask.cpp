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

#include "PlaygroundFilesTask.h"
#include "TaskEvent.h"

PlaygroundFilesTask::PlaygroundFilesTask(void)
    : PlaygroundIteratorTask<Anoubis_PgFileRecord>(ANOUBIS_PGREC_FILELIST, 0)
{
}

PlaygroundFilesTask::PlaygroundFilesTask(uint64_t pgid)
    : PlaygroundIteratorTask<Anoubis_PgFileRecord>(ANOUBIS_PGREC_FILELIST, pgid)
{
}

wxEventType
PlaygroundFilesTask::getEventType(void) const
{
	return (anTASKEVT_PG_FILES);
}

uint64_t
PlaygroundFilesTask::getRequestedPGID(void) const
{
	return (pgid_);
}

void
PlaygroundFilesTask::setRequestedPGID(uint64_t pgid)
{
	pgid_ = pgid;
}

uint64_t
PlaygroundFilesTask::getPGID(void) const
{
	if (getRecord())
		return get_value(getRecord()->pgid);
	else
		return 0;
}

uint64_t
PlaygroundFilesTask::getDevice(void) const
{
	if (getRecord())
		return (get_value(getRecord()->dev));
	else
		return (0);
}

uint64_t
PlaygroundFilesTask::getInode(void) const
{
	if (getRecord())
		return (get_value(getRecord()->ino));
	else
		return (0);
}

wxString
PlaygroundFilesTask::getPath(void) const
{
	if (getRecord() && getRecord()->path != 0)
		return (wxString::FromAscii(getRecord()->path));
	else
		return (wxEmptyString);
}

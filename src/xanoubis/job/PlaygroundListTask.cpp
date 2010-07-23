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

#include "PlaygroundListTask.h"
#include "TaskEvent.h"

PlaygroundListTask::PlaygroundListTask(void)
    : PlaygroundTask(ANOUBIS_PGREC_PGLIST, 0)
{
}

wxEventType
PlaygroundListTask::getEventType(void) const
{
	return (anTASKEVT_PG_LIST);
}

void
PlaygroundListTask::resetRecordIterator(void)
{
	it_ = iterator<Anoubis_PgInfoRecord>(result_);
}

bool
PlaygroundListTask::readNextRecord(void)
{
	return (it_.next());
}

uint64_t
PlaygroundListTask::getPGID(void) const
{
	if (it_.current())
		return (get_value(it_.current()->pgid));
	else
		return (0);
}

int
PlaygroundListTask::getUID(void) const
{
	if (it_.current())
		return (get_value(it_.current()->uid));
	else
		return (0);
}

bool
PlaygroundListTask::isActive(void) const
{
	if (it_.current())
		return (get_value(it_.current()->nrprocs) > 0);
	else
		return (false);
}

int
PlaygroundListTask::getNumFiles(void) const
{
	if (it_.current())
		return (get_value(it_.current()->nrfiles));
	else
		return (0);
}

wxDateTime
PlaygroundListTask::getTime(void) const
{
	wxDateTime dt;

	if (it_.current()) {
		time_t t = get_value(it_.current()->starttime);
		dt.Set(t);
	}

	return (dt);
}

wxString
PlaygroundListTask::getCommand(void) const
{
	if (it_.current())
		return (wxString::FromAscii(it_.current()->path));
	else
		return (wxEmptyString);
}

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

#include "anoubis_errno.h"
#include "PlaygroundUnlinkTask.h"

PlaygroundUnlinkTask::PlaygroundUnlinkTask(void) : Task(Task::TYPE_FS)
{
	reset();
}

void
PlaygroundUnlinkTask::addFile(PlaygroundFileEntry *entry)
{
	fileList_.insert(entry);
}

wxEventType
PlaygroundUnlinkTask::getEventType(void) const
{
	return (anTASKEVT_PG_UNLINK);
}

void
PlaygroundUnlinkTask::exec(void)
{
	std::vector<wxString> pathList;

	std::vector<wxString>::iterator		 pIt;
	std::set<PlaygroundFileEntry*>::iterator eIt;

	for (eIt=fileList_.begin(); eIt!=fileList_.end(); eIt++) {
		pathList = (*eIt)->getPaths();
		if (pathList.size() == 0) {
			result_ = -EINVAL;
			return;
		}
		for (pIt=pathList.begin(); pIt!=pathList.end(); pIt++) {
			if (unlink((*pIt).fn_str()) == 0 ||
			    (errno == EISDIR && rmdir((*pIt).fn_str()) == 0)) {
				result_ = 0;
			} else {
				result_ = -errno;
				return;
			}
		}
	}
}

int
PlaygroundUnlinkTask::getResult(void) const
{
	return (result_);
}

void
PlaygroundUnlinkTask::reset(void)
{
	PlaygroundFileEntry			 *entry;
	std::set<PlaygroundFileEntry*>::iterator  it;

	for (it=fileList_.begin(); it!=fileList_.end(); it++) {
		entry = *it;
		fileList_.erase(it);
		delete entry;
	}

	result_ = -EINVAL;

}

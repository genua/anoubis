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

#include <anoubis_errno.h>

#include "AnEvents.h"
#include "JobCtrl.h"
#include "PlaygroundCtrl.h"
#include "PlaygroundInfoEntry.h"

#include "Singleton.cpp"
template class Singleton<PlaygroundCtrl>;

PlaygroundCtrl::~PlaygroundCtrl(void)
{
	JobCtrl::getInstance()->Disconnect(anTASKEVT_PG_LIST,
	    wxTaskEventHandler(PlaygroundCtrl::OnPlaygroundListArrived),
	    NULL, this);

	/* Remove remaining tasks */
	for (std::set<Task *>::iterator it = taskList_.begin();
	    it != taskList_.end(); it++) {
		Task *t = *it;
		taskList_.erase(it);
		delete t;
	}

	clearPlaygroundList();
}

const AnRowProvider *
PlaygroundCtrl::getInfoProvider(void) const
{
	return (&playgroundList_);
}

bool
PlaygroundCtrl::updatePlaygroundList(void)
{
	return (createListTask());
}

const wxArrayString &
PlaygroundCtrl::getErrors(void) const
{
	return (errorList_);
}

PlaygroundCtrl::PlaygroundCtrl(void) : Singleton<PlaygroundCtrl>()
{
	JobCtrl::getInstance()->Connect(anTASKEVT_PG_LIST,
	    wxTaskEventHandler(PlaygroundCtrl::OnPlaygroundListArrived),
	    NULL, this);
}

void
PlaygroundCtrl::OnPlaygroundListArrived(TaskEvent & event)
{
	PlaygroundListTask *task = NULL;

	task = dynamic_cast<PlaygroundListTask *>(event.getTask());
	if (task == NULL) {
		/* No PlaygroundListTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (taskList_.find(task) == taskList_.end()) {
		/* Belongs to someone other, ignore it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	/* Progress for the finished task. */
	switch (task->getComTaskResult()) {
	case ComTask::RESULT_COM_ERROR:
		errorList_.Add(wxString::Format(_("Communication error while "
		    "fetching list of playgrounds.")));
		sendErrorEvent();
		break;
	case ComTask::RESULT_REMOTE_ERROR:
		errorList_.Add(wxString::Format(_("Got error from daemon "
		    "while fetching list of playgrounds: %hs"),
		    anoubis_strerror(task->getResultDetails())));
		sendErrorEvent();
		break;
	case ComTask::RESULT_SUCCESS:
		errorList_.Clear();
		extractListTask(task);
		break;
	default:
		errorList_.Add(wxString::Format(_("Got unexpected result (%d) "
		    "fetching list of playgrounds."),
		    task->getComTaskResult()));
		sendErrorEvent();
		break;
	}

	taskList_.erase(task);
	delete task;
}

bool
PlaygroundCtrl::createListTask(void)
{
	PlaygroundListTask *task = NULL;

	task = new PlaygroundListTask();
	if (task == NULL) {
		return (false);
	}

	taskList_.insert(task);
	JobCtrl::instance()->addTask(task);

	return (true);
}

void
PlaygroundCtrl::extractListTask(PlaygroundListTask *task)
{
	PlaygroundInfoEntry *entry = NULL;

	clearPlaygroundList();
	while (task->setNextRecord()) {
		entry = new PlaygroundInfoEntry(task->getPGID(),
		    task->getUID(), task->getTime(), task->isActive(),
		    task->getNumFiles(), task->getCommand());
		playgroundList_.addRow(entry);
	}
}

void
PlaygroundCtrl::clearPlaygroundList(void)
{
	while (playgroundList_.getSize() > 0) {
		int idx = playgroundList_.getSize() - 1;
		AnListClass *obj = playgroundList_.getRow(idx);
		playgroundList_.removeRow(idx);

		delete obj;
	}
}

void
PlaygroundCtrl::sendErrorEvent(void)
{
	wxCommandEvent event(anEVT_PLAYGROUND_ERROR);
	event.SetEventObject(this);

	ProcessEvent(event);
}


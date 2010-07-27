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

#include <config.h>

#include <set>

#include "anoubis_errno.h"
#include "anoubis_playground.h"
#include "AnEvents.h"
#include "JobCtrl.h"
#include "PlaygroundCtrl.h"
#include "PlaygroundFileEntry.h"
#include "PlaygroundInfoEntry.h"
#include "anoubis_errno.h"

#include "Singleton.cpp"

template class Singleton<PlaygroundCtrl>;

PlaygroundCtrl::~PlaygroundCtrl(void)
{
	JobCtrl::instance()->Disconnect(anTASKEVT_PG_LIST,
	    wxTaskEventHandler(PlaygroundCtrl::OnPlaygroundListArrived),
	    NULL, this);

	JobCtrl::instance()->Disconnect(anTASKEVT_PG_FILES,
	    wxTaskEventHandler(PlaygroundCtrl::OnPlaygroundFilesArrived),
	    NULL, this);

	/* Remove remaining tasks */
	for (std::set<Task *>::iterator it = taskList_.begin();
	    it != taskList_.end(); it++) {
		Task *t = *it;
		taskList_.erase(it);
		delete t;
	}

	clearPlaygroundInfo();
	clearPlaygroundFiles();
}

AnRowProvider *
PlaygroundCtrl::getInfoProvider(void)
{
	return (&playgroundInfo_);
}

bool
PlaygroundCtrl::updatePlaygroundInfo(void)
{
	return (createListTask());
}

AnRowProvider *
PlaygroundCtrl::getFileProvider(void)
{
	return (&playgroundFiles_);
}

bool
PlaygroundCtrl::updatePlaygroundFiles(uint64_t pgid)
{
	return (createFileTask(pgid));
}

const wxArrayString &
PlaygroundCtrl::getErrors(void) const
{
	return (errorList_);
}

void
PlaygroundCtrl::clearErrors(void)
{
	errorList_.Clear();
}

PlaygroundCtrl::PlaygroundCtrl(void) : Singleton<PlaygroundCtrl>()
{
	JobCtrl::instance()->Connect(anTASKEVT_PG_LIST,
	    wxTaskEventHandler(PlaygroundCtrl::OnPlaygroundListArrived),
	    NULL, this);
	JobCtrl::instance()->Connect(anTASKEVT_PG_FILES,
	    wxTaskEventHandler(PlaygroundCtrl::OnPlaygroundFilesArrived),
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
	clearPlaygroundInfo();
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

void
PlaygroundCtrl::OnPlaygroundFilesArrived(TaskEvent & event)
{
	/* Note: this looks like OnPlaygroundListArrived but there
	 * is simply no way to remove redundancies here */
	PlaygroundFilesTask *task = NULL;

	task = dynamic_cast<PlaygroundFilesTask *>(event.getTask());
	if (task == NULL) {
		/* No PlaygroundFileTask -> stop propagating */
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
	clearPlaygroundFiles();
	switch (task->getComTaskResult()) {
	case ComTask::RESULT_COM_ERROR:
		errorList_.Add(wxString::Format(_("Communication error while "
		    "fetching files for playground.")));
		sendErrorEvent();
		break;
	case ComTask::RESULT_REMOTE_ERROR:
		errorList_.Add(wxString::Format(_("Got error from daemon "
		    "while fetching files for playgrounds: %hs"),
		    anoubis_strerror(task->getResultDetails())));
		sendErrorEvent();
		break;
	case ComTask::RESULT_SUCCESS:
		errorList_.Clear();
		extractFilesTask(task);
		break;
	default:
		errorList_.Add(wxString::Format(_("Got unexpected result (%d) "
		    "while fetching files for playground."),
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

bool
PlaygroundCtrl::createFileTask(uint64_t pgid)
{
	PlaygroundFilesTask *task = NULL;
	task = new PlaygroundFilesTask(pgid);
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

	task->resetRecordIterator();
	while (task->readNextRecord()) {
		entry = new PlaygroundInfoEntry(task->getPGID(),
		    task->getUID(), task->getTime(), task->isActive(),
		    task->getNumFiles(), task->getCommand());
		playgroundInfo_.addRow(entry);
	}
}

void
PlaygroundCtrl::extractFilesTask(PlaygroundFilesTask *task)
{
	/* declare list variable "files" */
	std::set<PlaygroundFileEntry*,
	    bool(*)(const PlaygroundFileEntry*, const PlaygroundFileEntry*)>
		files(&PlaygroundFileEntry::cmpSet);

	/* insert data from task into temporary set 'files' */
	task->resetRecordIterator();
	while (task->readNextRecord()) {
		/* assume that we have to create one */
		PlaygroundFileEntry *entry = new PlaygroundFileEntry(
		    task->getPGID(), task->getDevice(), task->getInode());

		std::pair<std::set<PlaygroundFileEntry*>::iterator,bool> ret;
		ret = files.insert(entry);

		if (!ret.second) {
			delete(entry);  /* element already exists */
		}

		/* compose the absolute, validated path */
		char *path_abs = NULL;
#ifdef LINUX
		int res = pgfile_composename(&path_abs,
		    task->getDevice(), task->getInode(), task->getPathData());
#else
		int res = -ENOSYS;
#endif

		switch (res) {
		case 0: {
			/* add absolute path to entry */
			/* ret.first contains the element from the list */
			PlaygroundFileEntry *cur = *ret.first;
			cur->addPath(wxString::FromAscii(path_abs));
			break;
		}
		case -EBUSY:
			/* filename invalid, ignore */
			break;
		case -EXDEV:
			/* device busy, ignore */
			break;
		case -ENOMEM:
			/* we should log an error here but there is no memory
			 * memory to add strings. simply do not add the file */
			break;
		default: {
			errorList_.Add(wxString::Format(_(
			    "Could not determine filename for playground-file: "
			    "%hs"), anoubis_strerror(res)));
			sendErrorEvent();
			break;
		} // end default:
		} // end switch

		if (path_abs != 0) {
			free(path_abs);
		}
	}

	/* create the actual provider */
	std::set<PlaygroundFileEntry*>::iterator cur = files.begin();
	while (cur != files.end()) {
		PlaygroundFileEntry *e = *cur;
		playgroundFiles_.addRow(e);
		cur++;
	}
}

void
PlaygroundCtrl::clearPlaygroundInfo(void)
{
	while (playgroundInfo_.getSize() > 0) {
		int idx = playgroundInfo_.getSize() - 1;
		AnListClass *obj = playgroundInfo_.getRow(idx);
		playgroundInfo_.removeRow(idx);

		delete obj;
	}
	/* Note: clearRows is not necessary but it triggers an event even if
	 * the list was empty before (and at least the test needs this). */
	playgroundInfo_.clearRows();
}

void
PlaygroundCtrl::clearPlaygroundFiles(void)
{
	while (playgroundFiles_.getSize() > 0) {
		int idx = playgroundFiles_.getSize() - 1;
		AnListClass *obj = playgroundFiles_.getRow(idx);
		playgroundFiles_.removeRow(idx);

		delete obj;
	}
	/* Note: clearRows is not necessary but it triggers an event even if
	 * the list was empty before (and at least the test needs this). */
	playgroundFiles_.clearRows();
}

void
PlaygroundCtrl::sendErrorEvent(void)
{
	wxCommandEvent event(anEVT_PLAYGROUND_ERROR);
	event.SetEventObject(this);

	ProcessEvent(event);
}

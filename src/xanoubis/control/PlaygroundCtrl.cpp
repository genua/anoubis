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
#include <wx/msgdlg.h>

#include "anoubis_errno.h"
#include "anoubis_playground.h"
#include "AnEvents.h"
#include "JobCtrl.h"
#include "PlaygroundCtrl.h"
#include "PlaygroundFileEntry.h"
#include "PlaygroundInfoEntry.h"
#include "PlaygroundCommitTask.h"
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

	JobCtrl::instance()->Disconnect(anTASKEVT_PG_COMMIT,
	    wxTaskEventHandler(PlaygroundCtrl::OnPlaygroundFilesCommitted),
	    NULL, this);

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
PlaygroundCtrl::updatePlaygroundFiles(uint64_t pgid, bool reportESRCH)
{
	return createFileTask(pgid, reportESRCH);
}

bool
PlaygroundCtrl::removePlayground(uint64_t pgid)
{
	/* to be implemented ... */
	(void)pgid;
	return true;
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
	JobCtrl::instance()->Connect(anTASKEVT_PG_COMMIT,
	    wxTaskEventHandler(PlaygroundCtrl::OnPlaygroundFilesCommitted),
	    NULL, this);
}

void
PlaygroundCtrl::handleComTaskResult(ComTask *task)
{
	switch (task->getComTaskResult()) {
	case ComTask::RESULT_COM_ERROR:
		errorList_.Add(wxString::Format(_("Communication error in "
		    "playground request.")));
		sendErrorEvent();
		break;
	case ComTask::RESULT_REMOTE_ERROR:
		errorList_.Add(wxString::Format(_("The daemon returned an "
		    "error for the playground request: %hs"),
		    anoubis_strerror(task->getResultDetails())));
		sendErrorEvent();
		break;
	case ComTask::RESULT_SUCCESS:
		errorList_.Clear();
		break;
	default:
		errorList_.Add(wxString::Format(_("Got unexpected result (%d) "
		    "for playground request"), task->getComTaskResult()));
		sendErrorEvent();
		break;
	}
}

void
PlaygroundCtrl::OnPlaygroundListArrived(TaskEvent &event)
{
	PlaygroundListTask *task = NULL;

	task = dynamic_cast<PlaygroundListTask *>(event.getTask());
	if (task == NULL) {
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	/* Progress for the finished task. */
	clearPlaygroundInfo();
	if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		handleComTaskResult(task);
	} else {
		extractListTask(task);
	}
	delete task;
}

void
PlaygroundCtrl::OnPlaygroundFilesArrived(TaskEvent & event)
{
	PlaygroundFilesTask *task = NULL;

	task = dynamic_cast<PlaygroundFilesTask *>(event.getTask());
	if (task == NULL) {
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	/* Progress for the finished task. */
	clearPlaygroundFiles();
	if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		handleComTaskResult(task);
	} else {
		extractFilesTask(task);
	}
	delete task;
}

wxString
PlaygroundCtrl::getCommitErrorMessage(int state, int err)
{
	switch (err) {
	case EXDEV:
		return _("The device is not mounted.");
	case EBUSY:
		return _("Cannot find all hard links to the file.");
	}
	switch (state) {
	case PlaygroundCommitTask::STATE_TODO:
		if (err == ENOTEMPTY) {
			return _("Parent directory must be committed first.");
		}
		return _("Did not try to commit the file.");
	case PlaygroundCommitTask::STATE_NEED_OVERWRITE:
		switch (err) {
		case EMFILE:
			return _("Target file exists and has multiple "
			    "hard links.");
		case EEXIST:
			return _("Target file exists.");
		}
		break;
	case PlaygroundCommitTask::STATE_SCAN_FAILED:
		return _("File scanners reported a problem with the file.");
	case PlaygroundCommitTask::STATE_RENAME_FAILED:
		return wxString::Format(_("File was removed from the "
		    "playground but could not be renamed (%hs)."),
		    anoubis_strerror(err));
	}
	return wxString::Format(wxT("%hs"), anoubis_strerror(err));
}

void
PlaygroundCtrl::OnPlaygroundFilesCommitted(TaskEvent &event)
{
	PlaygroundCommitTask	*task = NULL;
	bool			 errors = false;
	std::vector<uint64_t>	 devs, inos;

	task = dynamic_cast<PlaygroundCommitTask *>(event.getTask());
	if (task == NULL) {
		event.Skip();
		return;
	}
	/* Our task. Stop propagating. */
	event.Skip(false);
	if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		handleComTaskResult(task);
		sendCompletedEvent();
		updatePlaygroundFiles(task->getPgid(), false);
		delete task;
		return;
	}

	for (int i=0; i<task->getFileCount(); ++i) {
		int		s = task->getFileState(i);
		wxString	msg, fullmsg, file;

		if (s == PlaygroundCommitTask::STATE_COMPLETE)
			continue;
		msg = getCommitErrorMessage(s, task->getFileError(i));
		file = fileIdentification(task->getDevice(i),
		    task->getInode(i));
		fullmsg = wxString::Format(_("Commit for file(s) '%ls' "
		    "failed.\n\n%ls"), file.c_str(), msg.c_str());
		if (s == PlaygroundCommitTask::STATE_NEED_OVERWRITE) {
			fullmsg += _(" Commit anyway?");
			if (wxMessageBox(fullmsg, _("Playground Commit"),
			    wxYES_NO) == wxYES) {
				devs.push_back(task->getDevice(i));
				inos.push_back(task->getInode(i));
				continue;
			}
		} else {
			errorList_.Add(fullmsg);
		}
		errors = true;
	}
	if (errors)
		sendErrorEvent();
	if (devs.size() == 0) {
		sendCompletedEvent();
		updatePlaygroundFiles(task->getPgid(), false);
	} else {
		commitFiles(task->getPgid(), devs, inos, true);
	}
	delete task;
}

bool
PlaygroundCtrl::createListTask(void)
{
	PlaygroundListTask *task = NULL;

	task = new PlaygroundListTask();
	if (task == NULL)
		return false;
	JobCtrl::instance()->addTask(task);

	return true;
}

bool
PlaygroundCtrl::createFileTask(uint64_t pgid, bool reportESRCH)
{
	PlaygroundFilesTask *task = NULL;

	task = new PlaygroundFilesTask(pgid, reportESRCH);
	if (task == NULL)
		return false;
	JobCtrl::instance()->addTask(task);

	return true;
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
		PlaygroundFileEntry	*entry;
		char			*path_abs = NULL;
		std::pair<std::set<PlaygroundFileEntry*>::iterator, bool> ret;


		/* assume that we have to create one */
		entry = new PlaygroundFileEntry(task->getPGID(),
		    task->getDevice(), task->getInode());
		ret = files.insert(entry);

		/* element already exists */
		if (!ret.second)
			delete(entry);
		entry = *(ret.first);

#ifdef LINUX
		/* compose the absolute, validated path */
		int res = pgfile_composename(&path_abs,
		    task->getDevice(), task->getInode(), task->getPathData());
#else
		int res = -ENOSYS;
#endif

		switch (res) {
		case 0: {
			/* Normalize and add path to entry */
			pgfile_normalize_file(path_abs);
			entry->addPath(wxString::FromAscii(path_abs));
			free(path_abs);
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
		default:
			errorList_.Add(wxString::Format(_(
			    "Could not determine filename for playground-file: "
			    "%hs"), anoubis_strerror(res)));
			sendErrorEvent();
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

wxString
PlaygroundCtrl::fileIdentification(uint64_t dev, uint64_t ino)
{
	PlaygroundFileEntry	*e = NULL;
	int			 i;

	for (i=0; i<playgroundFiles_.getSize(); ++i) {
		AnListClass		*item = playgroundFiles_.getRow(i);

		if (item == NULL)
			continue;
		e = dynamic_cast<PlaygroundFileEntry *>(item);
		if (e == NULL)
			continue;
		if (e->getDevice() == dev && e->getInode() == ino)
			break;
	}
	if (i == playgroundInfo_.getSize()) {
		return wxString::Format(wxT("dev=%lld ino=%lld"),
		    (long long) dev, (long long)ino);
	}
	const std::vector<wxString>	 &paths = e->getPaths();
	wxString			  ret = wxT("");
	for (unsigned int j=0; j<paths.size(); ++j) {
		if (j)
			ret += wxT(", ");
		ret += paths[j];
	}
	return ret;
}

void
PlaygroundCtrl::sendErrorEvent(void)
{
	wxCommandEvent event(anEVT_PLAYGROUND_ERROR);
	event.SetEventObject(this);

	ProcessEvent(event);
}

void
PlaygroundCtrl::sendCompletedEvent(void)
{
	wxCommandEvent	event(anEVT_PLAYGROUND_COMPLETED);
	event.SetEventObject(this);

	ProcessEvent(event);
}

bool
PlaygroundCtrl::commitFiles(const std::vector<int> &files)
{
	AnRowProvider			*provider;
	std::vector<uint64_t>		 devs;
	std::vector<uint64_t>		 inos;
	uint64_t			 pgid = 0;

	provider = getFileProvider();
	for (unsigned int i=0; i<files.size(); ++i) {
		AnListClass		*item = provider->getRow(files[i]);
		PlaygroundFileEntry	*e;

		if (!item)
			continue;
		e = dynamic_cast<PlaygroundFileEntry *>(item);
		if (!e)
			continue;
		devs.push_back(e->getDevice());
		inos.push_back(e->getInode());
		if (pgid == 0)
			pgid = e->getPgid();
	}
	if (pgid == 0 || inos.size() == 0)
		return false;
	commitFiles(pgid, devs, inos, false);
	return true;
}

void
PlaygroundCtrl::commitFiles(uint64_t pgid, std::vector<uint64_t> devs,
    std::vector<uint64_t> inos, bool force)
{
	PlaygroundCommitTask		*ct;

	ct = new PlaygroundCommitTask(pgid, devs, inos);
	if (force)
		ct->setForceOverwrite();
	JobCtrl::instance()->addTask(ct);
}

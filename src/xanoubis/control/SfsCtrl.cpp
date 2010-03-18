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

#include <wx/intl.h>
#include <wx/progdlg.h>

#include <anoubis_sig.h>
#include <anoubis_csum.h>
#include <anoubis_errno.h>

#include "AnEvents.h"
#include "ComCsumAddTask.h"
#include "ComCsumDelTask.h"
#include "ComCsumGetTask.h"
#include "ComRegistrationTask.h"
#include "ComSfsListTask.h"
#include "CsumCalcTask.h"
#include "JobCtrl.h"
#include "KeyCtrl.h"
#include "SfsCtrl.h"
#include "main.h"	/* For wxGetApp().ProcessPendingEvents() */

#define SFSCTRL_PROGRESS_TIMER	1
BEGIN_EVENT_TABLE(SfsCtrl, wxEvtHandler)
	EVT_TIMER(SFSCTRL_PROGRESS_TIMER, SfsCtrl::onProgressTimer)
END_EVENT_TABLE()

SfsCtrl::SfsCtrl(void) : progressTimer_(this, SFSCTRL_PROGRESS_TIMER)
{
	this->entryFilter_ = FILTER_STD;
	this->comEnabled_ = false; /* Communication disabled */
	this->inProgress_ = 0; /* No SFS-Operation in Progress. */
	this->progress_ = NULL;
	this->sigEnabled_ = true; /* Signature support enabled */
	this->exportEnabled_ = false;
	this->importList_ = 0;

	JobCtrl::getInstance()->Connect(anTASKEVT_REGISTER,
	    wxTaskEventHandler(SfsCtrl::OnRegistration), NULL, this);
}

SfsCtrl::~SfsCtrl(void)
{
	clearImportEntries();
	clearExportEntries();
}

wxString
SfsCtrl::getPath(void) const
{
	return (sfsDir_.getPath());
}

void
SfsCtrl::setPath(const wxString &path)
{
	if (sfsDir_.setPath(path))
		refresh();
}

wxString
SfsCtrl::getFilter(void) const
{
	return (sfsDir_.getFilter());
}

void
SfsCtrl::setFilter(const wxString &filter)
{
	if (sfsDir_.setFilter(filter))
		refresh();
}

bool
SfsCtrl::isFilterInversed(void) const
{
	return (sfsDir_.isFilterInversed());
}

void
SfsCtrl::setFilterInversed(bool inversed)
{
	if (sfsDir_.setFilterInversed(inversed))
		refresh();
}

bool
SfsCtrl::isRecursive(void) const
{
	return (sfsDir_.isDirTraversalEnabled());
}

void
SfsCtrl::setRecursive(bool recursive)
{
	if (sfsDir_.setDirTraversal(recursive))
		refresh();
}

SfsCtrl::EntryFilter
SfsCtrl::getEntryFilter(void) const
{
	return (this->entryFilter_);
}

void
SfsCtrl::setEntryFilter(EntryFilter filter)
{
	if (this->entryFilter_ != filter) {
		this->entryFilter_ = filter;
		refresh();
	}
}

void
SfsCtrl::setEntryFilter(EntryFilter filter, bool recursive)
{
	if (sfsDir_.setDirTraversal(recursive)||
	    this->entryFilter_ != filter) {
		this->entryFilter_ = filter;
		refresh();
	}
}

bool
SfsCtrl::isSignatureEnabled(void) const
{
	KeyCtrl *keyCtrl = KeyCtrl::getInstance();
	return (this->sigEnabled_ && keyCtrl->canUseLocalKeys());
}

bool
SfsCtrl::setSignatureEnabled(bool enable)
{
	if (enable) {
		/*
		 * Signature-support should be enabled, you need a certificate
		 * and private key to be able to use the signature-support.
		 */
		if (KeyCtrl::getInstance()->canUseLocalKeys()) {
			this->sigEnabled_ = true;
			return (true);
		} else {
			/* Dependency mismatch */
			return (false);
		}
	} else {
		this->sigEnabled_ = false;
		return (true);
	}
}

SfsCtrl::CommandResult
SfsCtrl::refresh(void)
{
	if (!taskList_.empty() || inProgress_)
		return (RESULT_BUSY);

	sfsDir_.removeAllEntries();

	switch (entryFilter_) {
	case FILTER_STD:
		sfsDir_.scanLocalFilesystem();
		break;
	case FILTER_CHECKSUMS:
	case FILTER_CHANGED:
	case FILTER_ORPHANED:
	case FILTER_UPGRADED:
		/* Fetch sfslist from daemon */
		if (!comEnabled_)
			return (RESULT_NOTCONNECTED);
		startSfsOp(1);
		createSfsListTasks(getuid(), sfsDir_.getPath());
		endSfsOp();
		break;
	}

	sendDirChangedEvent();
	return (RESULT_EXECUTE);
}

SfsCtrl::CommandResult
SfsCtrl::validate(const IndexArray &arr)
{
	ComCsumGetTask		*task = NULL;
	if (!taskList_.empty() || inProgress_)
		return (RESULT_BUSY);

	if (comEnabled_) {
		int	numScheduled = 0;
		bool	doSig = isSignatureEnabled();

		startSfsOp(arr.Count());
		for (size_t i = 0; i < arr.Count(); i++) {
			unsigned int idx = arr.Item(i);

			if (idx >= sfsDir_.getNumEntries()) {
				/* Out of range */
				endSfsOp();
				return (RESULT_INVALIDARG);
			}

			SfsEntry *entry = sfsDir_.getEntry(idx);

			/*
			 * Remove any information from the entry, they are
			 * recreated now
			 */
			entry->reset();

			if (!task)
				task = createComCsumGetTask(doSig);
			task->addPath(entry->getPath());
			if (task->getPathCount() >= 1000) {
				pushTask(task);
				JobCtrl::getInstance()->addTask(task);
				task = NULL;
			}

			if (entry->canHaveChecksum(false)) {
				/* Ask for the local checksum */
				createCsumCalcTask(entry->getPath());
			}
			if (!updateSfsOp(1))
				break;
			numScheduled++;
		}
		if (task) {
			pushTask(task);
			JobCtrl::getInstance()->addTask(task);
			task = NULL;
		}
		endSfsOp();

		return (numScheduled > 0) ? RESULT_EXECUTE : RESULT_NOOP;
	} else
		return (RESULT_NOTCONNECTED);
}

SfsCtrl::CommandResult
SfsCtrl::validate(unsigned int idx)
{
	IndexArray arr;
	arr.Add(idx);

	return (validate(arr));
}

SfsCtrl::CommandResult
SfsCtrl::validateAll(void)
{
	/* Validate all entries in model */
	IndexArray arr;

	for (unsigned int idx = 0; idx < sfsDir_.getNumEntries(); idx++)
		arr.Add(idx);

	return (validate(arr));
}

SfsCtrl::CommandResult
SfsCtrl::registerChecksum(const IndexArray &arr)
{
	KeyCtrl::KeyResult	 keyRes;
	unsigned int		 idx;
	ComCsumAddTask		*task = NULL;

	if (!taskList_.empty() || inProgress_)
		return (RESULT_BUSY);

	if (comEnabled_) {
		bool	doSig = isSignatureEnabled();
		startSfsOp(arr.Count());
		for (size_t i = 0; i < arr.Count(); i++) {
			idx = arr.Item(i);

			if (idx >= sfsDir_.getNumEntries()) {
				/* Out of range */
				endSfsOp();
				return (RESULT_INVALIDARG);
			}

			if (doSig) {
				/* Need a loaded private key */
				KeyCtrl *keyCtrl = KeyCtrl::getInstance();
				keyRes = keyCtrl->loadPrivateKey();
				if (keyRes == KeyCtrl::RESULT_KEY_WRONG_PASS)
					return (RESULT_WRONG_PASS);
				else if (keyRes == KeyCtrl::RESULT_KEY_ERROR)
					return (RESULT_NEEDKEY);
				else if (keyRes == KeyCtrl::RESULT_KEY_ABORT)
					return (RESULT_NOOP);
			}

			SfsEntry *entry = sfsDir_.getEntry(idx);

			/* Send checksum to anoubisd */
			if (!task)
				task = createComCsumAddTask(doSig);
			task->addPath(entry->getPath());
			/* Add one to the progress bar for the new task*/
			if (task->getPathCount() >= 1000) {
				startSfsOp(1);
				if (!updateSfsOp(1)) {
					delete task;
					task = NULL;
					endSfsOp();
					break;
				}
				pushTask(task);
				JobCtrl::getInstance()->addTask(task);
				task = NULL;
			}
		}
		if (task) {
			startSfsOp(1);
			if (!updateSfsOp(1)) {
				delete task;
				task = NULL;
				endSfsOp();
			} else {
				pushTask(task);
				JobCtrl::getInstance()->addTask(task);
			}
		}
		endSfsOp();
		return (RESULT_EXECUTE);
	} else
		return (RESULT_NOTCONNECTED);
}

SfsCtrl::CommandResult
SfsCtrl::registerChecksum(unsigned int idx)
{
	IndexArray arr;
	arr.Add(idx);

	return (registerChecksum(arr));
}

SfsCtrl::CommandResult
SfsCtrl::unregisterChecksum(const IndexArray &arr)
{
	if (!taskList_.empty() || inProgress_)
		return (RESULT_BUSY);

	if (comEnabled_) {
		int	numScheduled = 0;
		bool	doSig = isSignatureEnabled();

		startSfsOp(arr.Count());
		for (size_t i = 0; i < arr.Count(); i++) {
			unsigned int idx = arr.Item(i);

			if (idx >= sfsDir_.getNumEntries()) {
				/* Out of range */
				endSfsOp();
				return (RESULT_INVALIDARG);
			}

			SfsEntry *entry = sfsDir_.getEntry(idx);

			/* Remove checksum from anoubisd */
			numScheduled += createComCsumDelTasks(entry, doSig);

			if (!doSig) {
				/* Reset signed checksums in model */
				entry->setChecksumMissing(
				    SfsEntry::SFSENTRY_SIGNATURE);
			}
			if (!updateSfsOp(1))
				break;
		}
		endSfsOp();
		return (numScheduled == 0) ? RESULT_NOOP : RESULT_EXECUTE;
	} else
		return (RESULT_NOTCONNECTED);
}

SfsCtrl::CommandResult
SfsCtrl::importChecksums(const wxString &path)
{
	ComCsumGetTask	*task;

	if (!taskList_.empty() || inProgress_)
		return (RESULT_BUSY);

	if (comEnabled_) {
		FILE *fh;
		struct sfs_entry *entries, *entry;
		int total = 0;
		bool	doSig = isSignatureEnabled();

		/*
		 * Wrap the file read in a dummy SfsOp because the caller
		 * relies on the "Done" event.
		 */
		startSfsOp(1);
		clearImportEntries();

		if ((fh = fopen(path.fn_str(), "r")) == 0) {
			errorList_.Add(wxString::Format(
			    _("Failed to open %ls for reading: %hs"),
			    path.c_str(), anoubis_strerror(errno)));
			sendErrorEvent();
			endSfsOp();
			return (RESULT_INVALIDARG);
		}

		if ((entries = import_csum(fh)) == 0) {
			errorList_.Add(wxString::Format(
			    _("Failed to parse %ls!"), path.c_str()));
			sendErrorEvent();
			fclose(fh);
			endSfsOp();
			return (RESULT_INVALIDARG);
		}
		endSfsOp();
		fclose(fh);

		entry = entries;
		for (entry = entries; entry; entry = entry->next)
			total++;

		entry = entries;
		startSfsOp(total);
		/* XXX CEH: Make better use of csmulti requests if possible */
		while (entry != 0) {
			createComCsumAddTask(entry);

			int idx = sfsDir_.getIndexOf(
			    wxString::FromAscii(entry->name));
			if (idx != -1) {
				/* Part of the model, perform a validation */
				SfsEntry *e = sfsDir_.getEntry(idx);

				if (!e->haveLocalCsum())
					createCsumCalcTask(e->getPath());

				task = createComCsumGetTask(doSig);
				task->addPath(e->getPath());
				pushTask(task);
				JobCtrl::getInstance()->addTask(task);
			}
			entry = entry->next;
			if (!updateSfsOp(1))
				break;
		}
		endSfsOp();
		return (RESULT_EXECUTE);
	} else
		return (RESULT_NOTCONNECTED);
}

SfsCtrl::CommandResult
SfsCtrl::exportChecksums(const IndexArray &arr, const wxString &path)
{
	ComCsumGetTask	*task;

	if (!taskList_.empty() || inProgress_)
		return (RESULT_BUSY);

	if (comEnabled_) {
		bool	doSig = isSignatureEnabled();
		/* Enable export */
		exportEnabled_ = true;
		exportFile_ = path;

		startSfsOp(arr.Count());
		for (size_t i = 0; i < arr.Count(); i++) {
			unsigned int idx = arr.Item(i);

			if (idx >= sfsDir_.getNumEntries()) {
				/* Out of range */
				endSfsOp();
				return (RESULT_INVALIDARG);
			}

			/*
			 * Ask for checksums, if received the checksums are
			 * dumped into exportFile_.
			 */
			SfsEntry *entry = sfsDir_.getEntry(idx);
			createCsumCalcTask(entry->getPath());
			task = createComCsumGetTask(doSig);
			task->addPath(entry->getPath());
			pushTask(task);
			JobCtrl::getInstance()->addTask(task);
			if (!updateSfsOp(1))
				break;
		}
		endSfsOp();
		return (RESULT_EXECUTE);
	} else
		return (RESULT_NOTCONNECTED);
}

SfsCtrl::CommandResult
SfsCtrl::unregisterChecksum(unsigned int idx)
{
	IndexArray arr;
	arr.Add(idx);

	return (unregisterChecksum(arr));
}

SfsDirectory &
SfsCtrl::getSfsDirectory()
{
	return (this->sfsDir_);
}

const wxArrayString &
SfsCtrl::getErrors(void) const
{
	return (errorList_);
}

void
SfsCtrl::OnRegistration(TaskEvent &event)
{
	ComRegistrationTask *task =
	    dynamic_cast<ComRegistrationTask*>(event.getTask());

	if (task->getComTaskResult() == ComTask::RESULT_SUCCESS) {
		switch (task->getAction()) {
		case ComRegistrationTask::ACTION_REGISTER:
			/*
			 * Registration successful,
			 * enable communication-part
			 */
			enableCommunication();
			break;
		case ComRegistrationTask::ACTION_UNREGISTER:
			/*
			 * Unregistration successful,
			 * disable communication-part again
			 */
			disableCommunication();
			break;
		}
	}

	event.Skip();
}

void
SfsCtrl::OnSfsListArrived(TaskEvent &event)
{
	ComSfsListTask	*task = dynamic_cast<ComSfsListTask*>(event.getTask());
	ComCsumGetTask	*getTask = NULL;
	wxArrayString	result; /* Files which has a checksum */
	PopTaskHelper	taskHelper(this, task);

	if (task == 0) {
		/* No ComCsumListTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (taskList_.find(task) == taskList_.end()) {
		/* Belongs to someone other, ignore it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	ComTask::ComTaskResult comResult = task->getComTaskResult();
	SfsEntry::ChecksumType type = task->haveKeyId() ?
	    SfsEntry::SFSENTRY_SIGNATURE : SfsEntry::SFSENTRY_CHECKSUM;

	if (comResult != ComTask::RESULT_SUCCESS) {
		ComTask::ComTaskResult comResult = task->getComTaskResult();
		wxString message;

		if (comResult == ComTask::RESULT_COM_ERROR) {
			if (type == SfsEntry::SFSENTRY_CHECKSUM)
				message = wxString::Format(_("Communication "
				    "error while fetching list of checksumed "
				    "files of %ls."),
				    task->getDirectory().c_str());
			else
				message = wxString::Format(_("Communication "
				    "error while fetching list of signed "
				    "files of %ls."),
				    task->getDirectory().c_str());
		} else if (comResult == ComTask::RESULT_REMOTE_ERROR) {
			const char *err =
				anoubis_strerror(task->getResultDetails());

			if (type == SfsEntry::SFSENTRY_CHECKSUM)
				message = wxString::Format(_("Got error from "
				    "daemon (%hs) while fetching the list of "
				    "checksumed files of %ls."),
				    err, task->getDirectory().c_str());
			else
				message = wxString::Format(_("Got error from "
				    "daemon (%hs) while fetching the list of "
				    "signed files of %ls."),
				    err, task->getDirectory().c_str());
		} else {
			if (type == SfsEntry::SFSENTRY_CHECKSUM)
				message = wxString::Format(_("An unexpected "
				    "error occured (%i) while fetching the "
				    "list of checksumed files of %ls."),
				    task->getComTaskResult(),
				    task->getDirectory().c_str());
			else
				message = wxString::Format(_("An unexpected "
				    "error occured (%i) while fetching the "
				    "list of signed files of %ls."),
				    task->getComTaskResult(),
				    task->getDirectory().c_str());
		}

		errorList_.Add(message);
	} else
		result = task->getFileList();

	wxString basePath = task->getDirectory();
	if (!basePath.EndsWith(wxT("/")))
		basePath += wxT("/");

	startSfsOp(result.Count());
	bool doSig = isSignatureEnabled();
	sfsDir_.beginChange();
	for (unsigned int idx = 0; idx < result.Count(); idx++) {
		SfsEntry *entry = sfsDir_.insertEntry(basePath + result[idx]);

		if (entry == 0) {
			/* Not inserted into the model */
			if (!updateSfsOp(1))
				break;
			continue;
		}

		if (entry->canHaveChecksum(false)) {
			/*
			 * Model-entry is able to hold a local checksum,
			 * calculate it now
			 */
			createCsumCalcTask(entry->getPath());
		} else {
			/*
			 * Remove a previous calculated local checksum, the
			 * file might be removed in the meanwhile.
			 */
			entry->setLocalCsum(0);
		}

		if (!getTask)
			getTask = createComCsumGetTask(doSig);
		getTask->addPath(entry->getPath());
		if (getTask->getPathCount() >= 1000) {
			pushTask(getTask);
			JobCtrl::getInstance()->addTask(getTask);
			getTask = NULL;
		}
		if (!updateSfsOp(1))
			break;
	}
	if (getTask) {
		pushTask(getTask);
		JobCtrl::getInstance()->addTask(getTask);
		getTask = NULL;
	}
	sfsDir_.endChange();
	endSfsOp();
	/* Directory listing is complete */
	sendDirChangedEvent();
}

void
SfsCtrl::OnCsumCalc(TaskEvent &event)
{
	CsumCalcTask	*task = dynamic_cast<CsumCalcTask*>(event.getTask());
	PopTaskHelper	taskHelper(this, task);

	if (task == 0) {
		/* No ComCsumCalcTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (taskList_.find(task) == taskList_.end()) {
		/* Belongs to someone other, ignore it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	/* Search for SfsEntry */
	int idx = sfsDir_.getIndexOf(task->getPath());
	if (idx == -1) {
		wxString message = wxString::Format(
		    _("%ls not found in file-list!"), task->getPath().c_str());
		errorList_.Add(message);

		return;
	}

	if (task->getResult() != 0) {
		/* Calculation failed */
		wxString message = wxString::Format(
		    _("Failed to calculate the checksum for %ls: %hs"),
		    task->getPath().c_str(),
		    anoubis_strerror(task->getResult()));
		errorList_.Add(message);

		return;
	}

	/* Copy checksum into SfsEntry */
	SfsEntry *entry = sfsDir_.getEntry(idx);
	entry->setLocalCsum(task->getCsum());
}

void
SfsCtrl::processOneCsumGet(ComCsumGetTask *task, unsigned int idx,
    SfsEntry *entry)
{
	const u_int8_t		*csdata;
	size_t			 cslen;
	SfsEntry::ChecksumType	 type;
	int			 error;
	wxString		 message;

	type = SfsEntry::SFSENTRY_CHECKSUM;
	error = task->getChecksumError(idx);
	if (error == 0) {
		if (task->getChecksumData(idx, ANOUBIS_SIG_TYPE_CS,
		    csdata, cslen) && cslen == ANOUBIS_CS_LEN) {
			entry->setChecksum(type, csdata, cslen);
			if (exportEnabled_)
				pushExportEntry(entry, type);
		} else {
			message = wxString::Format(_("An unexpected checksum "
			    "was fetched for %ls"), entry->getPath().c_str());
			errorList_.Add(message);
		}
	} else if (error == ENOENT) {
		error = entry->setChecksumMissing(type);
	} else if (error == EINVAL) {
		error = entry->setChecksumInvalid(type);
	} else {
		message = wxString::Format(_("Got error from daemon while "
		    "fetching the checksum for %ls: %hs"),
		    entry->getPath().c_str(), anoubis_strerror(error));
		errorList_.Add(message);
	}

	if (!task->haveKeyId())
		return;
	error = task->getSignatureError(idx);
	type = SfsEntry::SFSENTRY_SIGNATURE;
	if (error == ENOENT) {
		entry->setChecksumMissing(type);
	} else if (error == EINVAL) {
		entry->setChecksumInvalid(type);
	} else if (error) {
		message = wxString::Format(_("Got error from "
		    "daemon while fetching the signature for %ls: %hs"),
		    entry->getPath().c_str(), anoubis_strerror(error));
		errorList_.Add(message);
	}
	if (error)
		return;
	if (!task->getChecksumData(idx, ANOUBIS_SIG_TYPE_SIG, csdata, cslen)
	    || cslen < ANOUBIS_CS_LEN) {
		message = wxString::Format(_("An unexpected signature was "
		    "fetched for %ls"), entry->getPath().c_str());
		errorList_.Add(message);
		return;
	}
	entry->setChecksum(type, csdata, cslen);
	if (exportEnabled_) {
		pushExportEntry(entry, type);
		/* XXX CEH: Why no process UPGRADE for export? */
		return;
	}
	if (task->getChecksumData(idx, ANOUBIS_SIG_TYPE_UPGRADECS,
	    csdata, cslen) && cslen >= ANOUBIS_CS_LEN) {
		entry->setChecksum(SfsEntry::SFSENTRY_UPGRADE, csdata, cslen);
	} else {
		entry->reset(SfsEntry::SFSENTRY_UPGRADE);
	}
}

void
SfsCtrl::OnCsumGet(TaskEvent &event)
{
	ComCsumGetTask		*task =
	    dynamic_cast<ComCsumGetTask*>(event.getTask());
	PopTaskHelper		 taskHelper(this, task);
	ComTask::ComTaskResult	 taskResult;
	wxString		 message;
	const char		*err;

	if (task == 0) {
		/* No ComCsumGetTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (taskList_.find(task) == taskList_.end()) {
		/* Belongs to someone other, ignore it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	taskResult = task->getComTaskResult();
	if (taskResult == ComTask::RESULT_REMOTE_ERROR) {
		err = anoubis_strerror(task->getResultDetails());
		message = wxString::Format(_("Got error from daemon while "
		    "fetching checksums/signatures: %hs"), err);
		errorList_.Add(message);
		return;
	} else if (taskResult == ComTask::RESULT_COM_ERROR) {
		message = wxString::Format(_("Communication error while "
		    "fetching checksums/signatures"));
		errorList_.Add(message);
		return;
	} else if (taskResult == ComTask::RESULT_LOCAL_ERROR) {
		err = anoubis_strerror(task->getResultDetails());
		message = wxString::Format(_("Failed to retrieve "
		    "checksums/signatures: %hs"), err);
		errorList_.Add(message);
		return;
	} else if (taskResult != ComTask::RESULT_SUCCESS) {
		err = anoubis_strerror(task->getResultDetails());
		message = wxString::Format(_("An unexpected error occured "
		    "while fetching checksums/signatures: %hs"), err);
		errorList_.Add(message);
		return;
	}

	for (unsigned int idx = 0; idx < task->getPathCount(); ++idx) {
		int			 modelindex;
		SfsEntry		*entry;
		wxString		 path = task->getPath(idx);

		modelindex = sfsDir_.getIndexOf(path);
		if (modelindex < 0) {
			message = wxString::Format(
			    _("%ls not found in file-list!"), path.c_str());
			errorList_.Add(message);
			continue;
		}
		entry = sfsDir_.getEntry(modelindex);
		processOneCsumGet(task, idx, entry);
	}
}

void
SfsCtrl::OnCsumAdd(TaskEvent &event)
{
	ComCsumAddTask	*task = dynamic_cast<ComCsumAddTask*>(event.getTask());
	PopTaskHelper	 taskHelper(this, task);
	bool		 sigs;

	if (task == 0) {
		/* No ComCsumAddTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (taskList_.find(task) == taskList_.end()) {
		/* Belongs to someone other, ignore it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	if (task->getComTaskResult() == ComTask::RESULT_LOCAL_ERROR) {
		wxString message;

		message.Printf(_("Failed to add checksums/signatures: %hs"),
		    anoubis_strerror(task->getResultDetails()));
		errorList_.Add(message);
	} else if (task->getComTaskResult() == ComTask::RESULT_COM_ERROR) {
		wxString message;

		message = wxString::Format(_("Communication error "
		    "while register the checksums/signatures."));
		errorList_.Add(message);
	} else if (task->getComTaskResult() == ComTask::RESULT_REMOTE_ERROR) {
		wxString message;

		message = wxString::Format(_("Got error from daemon "
		    "(%hs) while registering checksums/signautres."),
		    anoubis_strerror(task->getResultDetails()));
		errorList_.Add(message);
	} else if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		wxString message;

		message = wxString::Format(_("An unexpected error "
		    "occured (%hs) while registering checksums/signatures."),
		    anoubis_strerror(task->getComTaskResult()));
		errorList_.Add(message);
	}

	sigs = task->haveSignatures();
	for (unsigned int i=0; i<task->getPathCount(); ++i) {
		int	error;

		error = task->getChecksumError(i);
		if (error) {
			wxString	message;

			message = wxString::Format(
			    _("Failed to add checksum for %ls: %hs"),
			    task->getPath(i).c_str(), strerror(error));
			errorList_.Add(message);
		}
		error = task->getSignatureError(i);
		if (error) {
			wxString	message;

			message = wxString::Format(
			    _("Failed to add signature for %ls: %hs"),
			    task->getPath(i).c_str(), strerror(error));
			errorList_.Add(message);
		}

		/*
		 * Search for SfsEntry. Do _not_ report an error if the
		 * index is not found. It may be from an import.
		 */
		int sfsidx = sfsDir_.getIndexOf(task->getPath(i));
		if (sfsidx == -1)
			continue;

		/*
		 * Add the data we got to the model, even in case of
		 * (partial) errors.
		 */
		SfsEntry *entry = sfsDir_.getEntry(sfsidx);
		u_int8_t cs[ANOUBIS_CS_LEN];

		if (task->getCsum(i, cs, sizeof(cs)) != ANOUBIS_CS_LEN)
			continue;
		entry->setLocalCsum(cs);
		if (task->getChecksumError(i) == 0)
			entry->setChecksum(SfsEntry::SFSENTRY_CHECKSUM, cs,
			    ANOUBIS_CS_LEN);
		if (sigs && task->getSignatureError(i) == 0)
			entry->setChecksum(SfsEntry::SFSENTRY_SIGNATURE,
			    cs, ANOUBIS_CS_LEN);
	}
	updateSfsOp(task->getPathCount());
	endSfsOp();
}

void
SfsCtrl::OnCsumDel(TaskEvent &event)
{
	ComCsumDelTask	*task = dynamic_cast<ComCsumDelTask*>(event.getTask());
	PopTaskHelper	taskHelper(this, task);

	if (task == 0) {
		/* No ComCsumAddTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (taskList_.find(task) == taskList_.end()) {
		/* Belongs to someone other, ignore it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */

	/* Search for SfsEntry */
	int idx = sfsDir_.getIndexOf(task->getPath());
	if (idx == -1) {
		wxString message = wxString::Format(
		    _("%ls not found in file-list!"), task->getPath().c_str());
		errorList_.Add(message);

		return;
	}

	SfsEntry::ChecksumType type = task->haveKeyId() ?
	    SfsEntry::SFSENTRY_SIGNATURE : SfsEntry::SFSENTRY_CHECKSUM;

	if (task->getComTaskResult() == ComTask::RESULT_COM_ERROR) {
		wxString message;

		if (type == SfsEntry::SFSENTRY_CHECKSUM)
			message = wxString::Format(_("Communication error "
			    "while removing the checksum for %ls."),
			    task->getPath().c_str());
		else
			message = wxString::Format(_("Communication error "
			    "while removing the signature for %ls."),
			    task->getPath().c_str());

		errorList_.Add(message);
	} else if (task->getComTaskResult() == ComTask::RESULT_REMOTE_ERROR) {
		wxString message;

		if (type == SfsEntry::SFSENTRY_CHECKSUM)
			message = wxString::Format(_("Got error from daemon "
			    "(%hs) while removing the checksum for %ls."),
			    anoubis_strerror(task->getResultDetails()),
			    task->getPath().c_str());
		else
			message = wxString::Format(_("Got error from daemon "
			    "(%hs) while removing the signature for %ls."),
			    anoubis_strerror(task->getResultDetails()),
			    task->getPath().c_str());

		errorList_.Add(message);
	} else if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		wxString message;

		if (type == SfsEntry::SFSENTRY_CHECKSUM)
			message = wxString::Format(_("An unexpected error "
			    "occured (%hs) while removing the checksum for "
			    "%ls."), anoubis_strerror(task->getComTaskResult()),
			    task->getPath().c_str());
		else
			message = wxString::Format(_("An unexpected error "
			    "occured (%hs) while removing the signature for "
			    "%ls."), anoubis_strerror(task->getComTaskResult()),
			    task->getPath().c_str());

		errorList_.Add(message);
	}

	/* Update model */
	SfsEntry *entry = sfsDir_.getEntry(idx);
	entry->setChecksumMissing(type);

	if (!entry->fileExists() && !entry->haveChecksum()) {
		sfsDir_.removeEntry(idx);
		sendDirChangedEvent();
	}
}

void
SfsCtrl::enableCommunication(void)
{
	comEnabled_ = true;

	JobCtrl::getInstance()->Connect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(SfsCtrl::OnSfsListArrived), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUMCALC,
	    wxTaskEventHandler(SfsCtrl::OnCsumCalc), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_GET,
	    wxTaskEventHandler(SfsCtrl::OnCsumGet), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_ADD,
	    wxTaskEventHandler(SfsCtrl::OnCsumAdd), NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_CSUM_DEL,
	    wxTaskEventHandler(SfsCtrl::OnCsumDel), NULL, this);
}

void
SfsCtrl::disableCommunication(void)
{
	comEnabled_ = false;

	JobCtrl::getInstance()->Disconnect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(SfsCtrl::OnSfsListArrived), NULL, this);
	JobCtrl::getInstance()->Disconnect(anTASKEVT_CSUMCALC,
	    wxTaskEventHandler(SfsCtrl::OnCsumCalc), NULL, this);
	JobCtrl::getInstance()->Disconnect(anTASKEVT_CSUM_GET,
	    wxTaskEventHandler(SfsCtrl::OnCsumGet), NULL, this);
	JobCtrl::getInstance()->Disconnect(anTASKEVT_CSUM_ADD,
	    wxTaskEventHandler(SfsCtrl::OnCsumAdd), NULL, this);
	JobCtrl::getInstance()->Disconnect(anTASKEVT_CSUM_DEL,
	    wxTaskEventHandler(SfsCtrl::OnCsumDel), NULL, this);
}

void
SfsCtrl::sendOperationFinishedEvent(void)
{
	wxCommandEvent event(anEVT_SFSOPERATION_FINISHED);
	event.SetEventObject(this);

	ProcessEvent(event);
}

void
SfsCtrl::sendDirChangedEvent(void)
{
	wxCommandEvent event(anEVT_SFSDIR_CHANGED);
	event.SetEventObject(this);

	ProcessEvent(event);
}

void
SfsCtrl::sendErrorEvent(void)
{
	wxCommandEvent event(anEVT_SFSENTRY_ERROR);
	event.SetEventObject(this);

	ProcessEvent(event);
}

ComCsumGetTask *
SfsCtrl::createComCsumGetTask(bool doSig)
{
	ComCsumGetTask		*task = new ComCsumGetTask;

	if (doSig) {
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &cert = keyCtrl->getLocalCertificate();
		struct anoubis_sig *raw_cert = cert.getCertificate();

		task->setKeyId(raw_cert->keyid, raw_cert->idlen);
	}
	return task;
}

ComCsumAddTask *
SfsCtrl::createComCsumAddTask(bool doSig)
{
	ComCsumAddTask *ret = new ComCsumAddTask;

	if (doSig) {
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &cert = keyCtrl->getLocalCertificate();
		PrivKey &privKey = keyCtrl->getPrivateKey();
		struct anoubis_sig *raw_cert = cert.getCertificate();

		ret->setKeyId(raw_cert->keyid, raw_cert->idlen);
		ret->setPrivateKey(privKey.getKey());
	}
	return ret;
}

void
SfsCtrl::createComCsumAddTask(struct sfs_entry *entry)
{
	if (entry->checksum || entry->signature) {
		ComCsumAddTask	*csTask = new ComCsumAddTask;

		csTask->setSfsEntry(entry);

		pushTask(csTask);
		JobCtrl::getInstance()->addTask(csTask);
	}
}

int
SfsCtrl::createComCsumDelTasks(SfsEntry *entry, bool doSig)
{
	int count = 0;

	SfsEntry::ChecksumState csState =
	    entry->getChecksumState(SfsEntry::SFSENTRY_CHECKSUM);
	if (csState != SfsEntry::SFSENTRY_MISSING) {
		ComCsumDelTask *csTask = new ComCsumDelTask;
		csTask->setPath(entry->getPath());
		csTask->setCalcLink(true);

		pushTask(csTask);
		JobCtrl::getInstance()->addTask(csTask);

		count++;
	}

	SfsEntry::ChecksumState sigState =
	    entry->getChecksumState(SfsEntry::SFSENTRY_SIGNATURE);
	if ((sigState != SfsEntry::SFSENTRY_MISSING) && doSig) {
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &cert = keyCtrl->getLocalCertificate();
		struct anoubis_sig *raw_cert = cert.getCertificate();

		ComCsumDelTask *sigTask = new ComCsumDelTask;
		sigTask->setPath(entry->getPath());
		sigTask->setCalcLink(true);
		sigTask->setKeyId(raw_cert->keyid, raw_cert->idlen);

		pushTask(sigTask);
		JobCtrl::getInstance()->addTask(sigTask);

		count++;
	}

	return (count);
}

void
SfsCtrl::createSfsListTasks(uid_t uid, const wxString &path)
{
	if (entryFilter_ != FILTER_UPGRADED) {
		ComSfsListTask *csTask = new ComSfsListTask;
		csTask->setRequestParameter(uid, path);
		csTask->setRecursive(isRecursive());
		csTask->setFetchOrphaned(entryFilter_ == FILTER_ORPHANED);

		pushTask(csTask);
		JobCtrl::getInstance()->addTask(csTask);
	}

	if (isSignatureEnabled()) {
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &cert = keyCtrl->getLocalCertificate();
		struct anoubis_sig *raw_cert = cert.getCertificate();

		ComSfsListTask *sigTask = new ComSfsListTask;
		sigTask->setRequestParameter(uid, path);
		sigTask->setRecursive(isRecursive());
		sigTask->setFetchOrphaned(entryFilter_ == FILTER_ORPHANED);
		sigTask->setFetchUpgraded(entryFilter_ == FILTER_UPGRADED);
		sigTask->setKeyId(raw_cert->keyid, raw_cert->idlen);

		pushTask(sigTask);
		JobCtrl::getInstance()->addTask(sigTask);
	}
}

void
SfsCtrl::createCsumCalcTask(const wxString &path)
{
	CsumCalcTask *task = new CsumCalcTask;
	task->setPath(path);
	task->setCalcLink(true);

	pushTask(task);
	JobCtrl::getInstance()->addTask(task);
}

void
SfsCtrl::pushExportEntry(SfsEntry *entry, SfsEntry::ChecksumType type)
{
	struct sfs_entry *se = 0; /* The export-entry */

	size_t csumLen = entry->getChecksumLength(type);
	u_int8_t csum[csumLen];

	entry->getChecksum(type, csum, csumLen);

	if (type == SfsEntry::SFSENTRY_CHECKSUM) {
		/* Create a sfs_entry, onl ycontains the checksum */
		se = anoubis_build_entry(entry->getPath().fn_str(),
		    csum, csumLen, NULL, 0, geteuid(), NULL, 0);
	} else if (isSignatureEnabled()) {
		/* Receive key-id of local-certificate */
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &lc = keyCtrl->getLocalCertificate();
		/* keyid part of anoubis_sig-structure */
		struct anoubis_sig *cert = lc.getCertificate();

		/* Create sfs_entry */
		se = anoubis_build_entry(entry->getPath().fn_str(),
		    0, 0, csum, csumLen,
		    geteuid(), cert->keyid, cert->idlen);
	}

	if (se != 0)
		exportList_.push_back(se);
}

void
SfsCtrl::dumpExportEntries(void)
{
	int i = 0;
	struct sfs_entry *entry_list[exportList_.size()];

	/* Prepare entry_list. Copy from exportList_ into entry_list */
	std::list<struct sfs_entry *>::const_iterator it;
	for (it = exportList_.begin(); it != exportList_.end(); ++it) {
		entry_list[i] = (*it);
		i++;
	}

	FILE *fh = fopen(exportFile_.fn_str(), "w");
	if (fh == 0) {
		errorList_.Add(wxString::Format(
		    _("Failed to open %ls for writing: %hs"),
		    exportFile_.c_str(), anoubis_strerror(errno)));
		return;
	}

	anoubis_print_entries(fh, entry_list, exportList_.size());

	if (fflush(fh) != 0) {
		errorList_.Add(wxString::Format(
		    _("Failed to flush the content of %ls: %hs"),
		    exportFile_.c_str(), anoubis_strerror(errno)));
	}

	fclose(fh);
}

void
SfsCtrl::clearExportEntries(void)
{
	while (!exportList_.empty()) {
		struct sfs_entry *se = exportList_.front();
		exportList_.pop_front();

		anoubis_entry_free(se);
	}
}

void
SfsCtrl::clearImportEntries(void)
{
	struct sfs_entry *entry = this->importList_;

	while (entry != 0) {
		struct sfs_entry *tmp_entry = entry;
		entry = entry->next;

		anoubis_entry_free(tmp_entry);
	}

	this->importList_ = 0;
}

void
SfsCtrl::pushTask(Task *task)
{
	if (taskList_.size() == 0 && !inProgress_) {
		/*
		 * This is the first task in the list -> an operation began.
		 * Clear the error-list.
		 * NOTE: This has been replaced in most cases by startSfsOp()
		 */
		errorList_.Clear();
	}

	taskList_[task] = true;
	if (taskList_.size() > 20)
		wxGetApp().ProcessPendingEvents();
}

void
SfsCtrl::popTask(Task *task)
{
	if (task) {
		std::map<Task *, bool>::iterator	it;

		it = taskList_.find(task);
		if (it != taskList_.end()) {
			taskList_.erase(it);
			delete task;
		}
	}

	if (taskList_.size() == 0 && !inProgress_) {
		/*
		 * This was the last task in the list -> the operation finished
		 */

		if (entryFilter_ == FILTER_CHANGED) {
			/*
			 * Remove all entries from model which has not
			 * changed
			 */
			unsigned int idx = 0;

			sfsDir_.beginChange();
			while (idx < sfsDir_.getNumEntries()) {
				SfsEntry *entry = sfsDir_.getEntry(idx);
				if (!entry->isChecksumChanged())
					sfsDir_.removeEntry(idx);
				else
					idx++;
			}
			sfsDir_.endChange();

			sendDirChangedEvent();
		}

		if (exportEnabled_) {
			/*
			 * Export is enabled
			 * All files and checksums are collected. Now dump into
			 * file.
			 */
			dumpExportEntries();

			/* Cleanup */
			clearExportEntries();
			exportEnabled_ = false;
		}

		sendOperationFinishedEvent();

		/* Check for errors */
		if (!errorList_.empty())
			sendErrorEvent();
	}
}

SfsCtrl::PopTaskHelper::PopTaskHelper(SfsCtrl *sfsCtrl, Task *task)
{
	this->sfsCtrl_ = sfsCtrl;
	this->task_ = task;
}

SfsCtrl::PopTaskHelper::~PopTaskHelper()
{
	if (task_ != 0) {
		sfsCtrl_->popTask(task_);
	}
}

SfsCtrl::PopTaskHelper::PopTaskHelper(SfsCtrl *sfsCtrl)
{
	sfsCtrl_ = sfsCtrl;
	task_ = NULL;
}

void
SfsCtrl::PopTaskHelper::setTask(Task *task)
{
	task_ = task;
}

void
SfsCtrl::startSfsOp(int steps)
{
	if (inProgress_++) {
		progressMax_ += steps;
	} else {
		errorList_.Clear();
		progressMax_ = steps;
		progressDone_ = 0;
		progressAbort_ = false;
	}
	if (!progress_ && progressMax_ > 40) {
		progress_ = new wxProgressDialog(_("SFS operation"),
		    _("SFS operation in Progress. Please wait ..."),
		    1000, NULL, wxPD_APP_MODAL|wxPD_AUTO_HIDE|wxPD_CAN_ABORT);
		progressTimer_.Start(1000, wxTIMER_CONTINUOUS);
	}
}

bool
SfsCtrl::updateSfsOp(int done)
{
	bool	ret = true;

	if (!inProgress_ || progressAbort_)
		return false;
	progressDone_ += done;
	if (progress_ && progressMax_) {
		int	reldone = (1000LL * progressDone_) / progressMax_;
		ret =  progress_->Update(reldone);
	}
	if (!ret) {
		progressAbort_ = true;
		abortAllTasks();
	}
	return ret;
}

void
SfsCtrl::endSfsOp(void)
{
	if (--inProgress_)
		return;
	if (progress_) {
		/* The Yield is required to avoid a BadWindow warning from X */
		progressTimer_.Stop();
		progress_->Hide();
		wxSafeYield(false);
		delete progress_;
		progress_ = NULL;
	}
	popTask(NULL);
}

void
SfsCtrl::abortAllTasks(void)
{
	std::map<Task *, bool>::iterator	it;

	for (it = taskList_.begin(); it != taskList_.end(); it++)
		it->first->abort();
}

void
SfsCtrl::onProgressTimer(wxTimerEvent &)
{
	/* Check for aborts. */
	updateSfsOp(0);
}

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

#include <anoubis_sig.h>
#include <csum/csum.h>

#include "AnEvents.h"
#include "ComCsumAddTask.h"
#include "ComCsumDelTask.h"
#include "ComCsumGetTask.h"
#include "ComRegistrationTask.h"
#include "ComSfsListTask.h"
#include "ComUpgradeListGetTask.h"
#include "CsumCalcTask.h"
#include "JobCtrl.h"
#include "KeyCtrl.h"
#include "SfsCtrl.h"

SfsCtrl::SfsCtrl(void)
{
	this->comEnabled_ = false; /* Communication disabled */
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
		sendDirChangedEvent();
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
		sendDirChangedEvent();
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
		sendDirChangedEvent();
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
		sendDirChangedEvent();
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
SfsCtrl::validate(const IndexArray &arr)
{
	if (!taskList_.empty())
		return (RESULT_BUSY);

	if (comEnabled_) {
		int numScheduled = 0;

		for (size_t i = 0; i < arr.Count(); i++) {
			unsigned int idx = arr.Item(i);

			if (idx >= sfsDir_.getNumEntries()) {
				/* Out of range */
				return (RESULT_INVALIDARG);
			}

			SfsEntry *entry = sfsDir_.getEntry(idx);

			/*
			 * Remove any information from the entry, they are
			 * recreated now
			 */
			entry->reset();

			/* In any case fetch the remote checksums */
			createComCsumGetTasks(entry->getPath(), true, true);

			if (entry->canHaveChecksum(false)) {
				/* Ask for the local checksum */
				createCsumCalcTask(entry->getPath());
			}

			numScheduled++;
		}

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
SfsCtrl::validateAll(bool orphaned)
{
	if (!taskList_.empty())
		return (RESULT_BUSY);

	if (comEnabled_) {
		if (!orphaned) {
			/*
			 * Switch back from orphaned to "normal" list.
			 * There might be entries in the model (non-existing
			 * files), which needs to be removed now.
			 */
			if (sfsDir_.cleanup())
				sendDirChangedEvent();
		}

		if (!isSignatureEnabled()) {
			/* Reset signed checksums in model */
			for (unsigned int idx = 0;
			    idx < sfsDir_.getNumEntries(); idx++) {
				SfsEntry *entry = sfsDir_.getEntry(idx);

				if (entry->setChecksumMissing(
				    SfsEntry::SFSENTRY_SIGNATURE))
					sendEntryChangedEvent(idx);
			}

		}

		/* Ask for sfs-list */
		createSfsListTasks(getuid(), sfsDir_.getPath(), orphaned);

		return (RESULT_EXECUTE);
	}
	else
		return (RESULT_NOTCONNECTED);
}

SfsCtrl::CommandResult
SfsCtrl::registerChecksum(const IndexArray &arr)
{
	KeyCtrl::KeyResult keyRes;
	unsigned int idx;

	if (!taskList_.empty())
		return (RESULT_BUSY);

	if (comEnabled_) {
		for (size_t i = 0; i < arr.Count(); i++) {
			idx = arr.Item(i);

			if (idx >= sfsDir_.getNumEntries()) {
				/* Out of range */
				return (RESULT_INVALIDARG);
			}

			if (isSignatureEnabled()) {
				/* Need a loaded private key */
				KeyCtrl *keyCtrl = KeyCtrl::getInstance();
				keyRes = keyCtrl->loadPrivateKey();
				if (keyRes == KeyCtrl::RESULT_KEY_WRONG_PASS)
					return (RESULT_WRONG_PASS);
				else if (keyRes == KeyCtrl::RESULT_KEY_ERROR)
					return (RESULT_NEEDKEY);
			}

			SfsEntry *entry = sfsDir_.getEntry(idx);

			/* Send checksum to anoubisd */
			createComCsumAddTasks(entry->getPath());

			if (!entry->haveLocalCsum()) {
				/*
				 * You also need a local checksum to be able to
				 * compare it with the remote one
				 */
				createCsumCalcTask(entry->getPath());
			}
		}

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
	if (!taskList_.empty())
		return (RESULT_BUSY);

	if (comEnabled_) {
		int numScheduled = 0;

		for (size_t i = 0; i < arr.Count(); i++) {
			unsigned int idx = arr.Item(i);

			if (idx >= sfsDir_.getNumEntries()) {
				/* Out of range */
				return (RESULT_INVALIDARG);
			}

			SfsEntry *entry = sfsDir_.getEntry(idx);

			/* Remove checksum from anoubisd */
			numScheduled += createComCsumDelTasks(entry);

			if (!isSignatureEnabled()) {
				/* Reset signed checksums in model */
				if (entry->setChecksumMissing(
				    SfsEntry::SFSENTRY_SIGNATURE))
					sendEntryChangedEvent(idx);
			}
		}

		return (numScheduled == 0) ? RESULT_NOOP : RESULT_EXECUTE;
	} else
		return (RESULT_NOTCONNECTED);
}

SfsCtrl::CommandResult
SfsCtrl::importChecksums(const wxString &path)
{
	if (!taskList_.empty())
		return (RESULT_BUSY);

	if (comEnabled_) {
		FILE *fh;
		struct sfs_entry *entries, *entry;

		errorList_.Clear();
		clearImportEntries();

		if ((fh = fopen(path.fn_str(), "r")) == 0) {
			errorList_.Add(wxString::Format(
			    _("Failed to open %ls for reading: %hs"),
			    path.c_str(), strerror(errno)));
			sendErrorEvent();
			return (RESULT_INVALIDARG);
		}

		if ((entries = import_csum(fh)) == 0) {
			errorList_.Add(wxString::Format(
			    _("Failed to parse %ls!"), path.c_str()));
			sendErrorEvent();
			fclose(fh);
			return (RESULT_INVALIDARG);
		}

		fclose(fh);

		entry = entries;

		while (entry != 0) {
			createComCsumAddTasks(entry);

			int idx = sfsDir_.getIndexOf(
			    wxString::FromAscii(entry->name));
			if (idx != -1) {
				/* Part of the model, perform a validation */
				SfsEntry *e = sfsDir_.getEntry(idx);

				if (!e->haveLocalCsum())
					createCsumCalcTask(e->getPath());

				createComCsumGetTasks(
				    e->getPath(), true, true);
			}

			entry = entry->next;
		}

		return (RESULT_EXECUTE);
	} else
		return (RESULT_NOTCONNECTED);
}

SfsCtrl::CommandResult
SfsCtrl::exportChecksums(const IndexArray &arr, const wxString &path)
{
	if (!taskList_.empty())
		return (RESULT_BUSY);

	if (comEnabled_) {
		/* Enable export */
		exportEnabled_ = true;
		exportFile_ = path;

		for (size_t i = 0; i < arr.Count(); i++) {
			unsigned int idx = arr.Item(i);

			if (idx >= sfsDir_.getNumEntries()) {
				/* Out of range */
				return (RESULT_INVALIDARG);
			}

			/*
			 * Ask for checksums, if received the checksums are
			 * dumped into exportFile_.
			 */
			SfsEntry *entry = sfsDir_.getEntry(idx);
			createCsumCalcTask(entry->getPath());
			createComCsumGetTasks(entry->getPath(), true, true);
		}

		return (RESULT_EXECUTE);
	} else
		return (RESULT_NOTCONNECTED);
}

SfsCtrl::CommandResult
SfsCtrl::fetchUpgradeList(void)
{
	if (!taskList_.empty()) {
		return (RESULT_BUSY);
	}

	if (comEnabled_) {
		createUpgradeListGetTask();
		return (RESULT_EXECUTE);
	} else {
		return (RESULT_NOTCONNECTED);
	}
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
	wxArrayString	result; /* Files which has a checksum */
	PopTaskHelper	taskHelper(this, task);

	if (task == 0) {
		/* No ComCsumGetTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (taskList_.IndexOf(task) == wxNOT_FOUND) {
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
			const char *err = strerror(task->getResultDetails());

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

	/* Go though each file in the model, each file needs to be updated */
	for (unsigned int idx = 0; idx < sfsDir_.getNumEntries(); idx++) {
		SfsEntry *entry = sfsDir_.getEntry(idx);

		/* Has file a checksum? */
		int csumIdx = result.Index(
		    entry->getRelativePath(sfsDir_.getPath()));

		if (csumIdx != wxNOT_FOUND) {
			/*
			 * Model-entry has a registered checksum, fetch it now
			 */
			createComCsumGetTasks(entry->getPath(),
			    !task->haveKeyId(), task->haveKeyId());

			/* Processed, remove from result-list */
			result.RemoveAt(csumIdx);
		} else {
			/* Model-entry has no checksum */
			if (entry->setChecksumMissing(type))
				sendEntryChangedEvent(idx);
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
			if (entry->setLocalCsum(0))
				sendEntryChangedEvent(idx);
		}
	}

	if (task->fetchOrphaned()) {
		/*
		 * If fetching of orphaned files was enabled, the remaining
		 * entries of the result-list are orphaned files, which needs
		 * to be inserted into the model.
		 */
		wxString basePath = task->getDirectory();

		if (!basePath.EndsWith(wxT("/")))
			basePath += wxT("/");

		for (unsigned int idx = 0; idx < result.Count(); idx++) {
			if (!result[idx].EndsWith(wxT("/"))) {
				SfsEntry *e = sfsDir_.insertEntry(
				    basePath + result[idx]);

				createComCsumGetTasks(e->getPath(),
				    !task->haveKeyId(), task->haveKeyId());
			}
		}
	}
}

void
SfsCtrl::OnCsumCalc(TaskEvent &event)
{
	CsumCalcTask	*task = dynamic_cast<CsumCalcTask*>(event.getTask());
	PopTaskHelper	taskHelper(this, task);

	if (task == 0) {
		/* No ComCsumGetTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (taskList_.IndexOf(task) == wxNOT_FOUND) {
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
		    strerror(task->getResult()));
		errorList_.Add(message);

		return;
	}

	/* Copy checksum into SfsEntry */
	SfsEntry *entry = sfsDir_.getEntry(idx);
	if (entry->setLocalCsum(task->getCsum())) {
		/* Checksum attribute has changed, inform any listener */
		sendEntryChangedEvent(idx);
	}
}

void
SfsCtrl::OnCsumGet(TaskEvent &event)
{
	ComCsumGetTask	*task = dynamic_cast<ComCsumGetTask*>(event.getTask());
	PopTaskHelper	taskHelper(this, task);

	if (task == 0) {
		/* No ComCsumGetTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (taskList_.IndexOf(task) == wxNOT_FOUND) {
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

	/* Recieve checksum from task */
	SfsEntry *entry = sfsDir_.getEntry(idx);
	ComTask::ComTaskResult taskResult = task->getComTaskResult();
	SfsEntry::ChecksumType type = task->haveKeyId() ?
	    SfsEntry::SFSENTRY_SIGNATURE : SfsEntry::SFSENTRY_CHECKSUM;

	if (taskResult == ComTask::RESULT_REMOTE_ERROR) {
		int err = task->getResultDetails();

		if (err == ENOENT) {
			/* No checksum registered */
			if (entry->setChecksumMissing(type))
				sendEntryChangedEvent(idx);
		} else if (err == EINVAL) {
			if (entry->setChecksumInvalid(type))
				sendEntryChangedEvent(idx);
		} else {
			const char *err = strerror(task->getResultDetails());
			wxString message;

			if (type == SfsEntry::SFSENTRY_CHECKSUM)
				message = wxString::Format(_("Got error from "
				    "daemon (%hs) while fetching the checksum "
				    "for %ls."), err, task->getPath().c_str());
			else
				message = wxString::Format(_("Got error from "
				    "daemon (%hs) while fetching the "
				    "signature for %ls."), err,
				    task->getPath().c_str());

			errorList_.Add(message);
		}

		return;
	} else if (taskResult == ComTask::RESULT_COM_ERROR) {
		wxString message;

		if (type == SfsEntry::SFSENTRY_CHECKSUM)
			message = wxString::Format(_("Communication error "
			    "while fetching the checksum for %ls."),
			    task->getPath().c_str());
		else
			message = wxString::Format(_("Communication error "
			    "while fetching the signature for %ls."),
			    task->getPath().c_str());

		errorList_.Add(message);
		return;
	} else if (taskResult == ComTask::RESULT_LOCAL_ERROR) {
		wxString message = wxString::Format(
		    _("Failed to resolve %ls"), task->getPath().c_str());

		errorList_.Add(message);
		return;
	} else if (taskResult != ComTask::RESULT_SUCCESS) {
		wxString message;

		if (type == SfsEntry::SFSENTRY_CHECKSUM)
			message = wxString::Format(_("An unexpected error "
			    "occured (%hs) while fetching the checksum for "
			    "%ls."), strerror(task->getComTaskResult()),
			    task->getPath().c_str());
		else
			message = wxString::Format(_("An unexpected error "
			    "occured (%hs) while fetching the signature for "
			    "%ls."), strerror(task->getComTaskResult()),
			    task->getPath().c_str());

		errorList_.Add(message);
		return;
	}

	size_t csumLen = task->getCsumLen();
	u_int8_t cs[csumLen];

	if (task->getCsum(cs, csumLen) == csumLen) {
		/* File has a checksum, copy into model */
		if (entry->setChecksum(type, cs, csumLen))
			sendEntryChangedEvent(idx);

		if (exportEnabled_) {
			/* Export is enabled, push entry into export-list */
			pushExportEntry(entry, type);
		} else if (type == SfsEntry::SFSENTRY_SIGNATURE) {
			if (task->getUpgradeCsum(cs, csumLen) == csumLen) {
				entry->setChecksum(
				    SfsEntry::SFSENTRY_UPGRADE, cs, csumLen);
			} else {
				entry->reset(SfsEntry::SFSENTRY_UPGRADE);
			}
			sendEntryChangedEvent(idx);
		}
	} else {
		wxString message;

		if (type == SfsEntry::SFSENTRY_CHECKSUM)
			message = _("An unexpected checksum was fetched.");
		else
			message = _("An unexpected signature was fetched.");

		errorList_.Add(message);
	}
}

void
SfsCtrl::OnCsumAdd(TaskEvent &event)
{
	ComCsumAddTask	*task = dynamic_cast<ComCsumAddTask*>(event.getTask());
	PopTaskHelper	taskHelper(this, task);

	if (task == 0) {
		/* No ComCsumAddTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (taskList_.IndexOf(task) == wxNOT_FOUND) {
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

	if (task->getComTaskResult() == ComTask::RESULT_LOCAL_ERROR) {
		wxString message;

		if (type == SfsEntry::SFSENTRY_CHECKSUM)
			message.Printf(_("Failed to calculate the checksum "
			    "for %ls: %hs"), task->getPath().c_str(),
			    strerror(task->getResultDetails()));
		else
			message.Printf(_("Failed to calculate the signature "
			    "for %ls: %hs"), task->getPath().c_str(),
			    strerror(task->getResultDetails()));

		errorList_.Add(message);
		return;
	} else if (task->getComTaskResult() == ComTask::RESULT_COM_ERROR) {
		wxString message;

		if (type == SfsEntry::SFSENTRY_CHECKSUM)
			message = wxString::Format(_("Communication error "
			    "while register the checksum for %ls."),
			    task->getPath().c_str());
		else
			message = wxString::Format(_("Communication error "
			    "while register the signature for %ls."),
			    task->getPath().c_str());

		errorList_.Add(message);
		return;
	} else if (task->getComTaskResult() == ComTask::RESULT_REMOTE_ERROR) {
		wxString message;

		if (type == SfsEntry::SFSENTRY_CHECKSUM)
			message = wxString::Format(_("Got error from daemon "
			    "(%hs) while register the checksum for %ls."),
			    strerror(task->getResultDetails()),
			    task->getPath().c_str());
		else
			message = wxString::Format(_("Got error from daemon "
			    "(%hs) while register the signature for %ls."),
			    strerror(task->getResultDetails()),
			    task->getPath().c_str());

		errorList_.Add(message);
		return;
	} else if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		wxString message;

		if (type == SfsEntry::SFSENTRY_CHECKSUM)
			message = wxString::Format(_("An unexpected error "
			    "occured (%hs) while register the checksum for "
			    "%ls."), strerror(task->getComTaskResult()),
			    task->getPath().c_str());
		else
			message = wxString::Format(_("An unexpected error "
			    "occured (%hs) while register the signature for "
			    "%ls."), strerror(task->getComTaskResult()),
			    task->getPath().c_str());

		errorList_.Add(message);
		return;
	}

	/* Update model */
	SfsEntry *entry = sfsDir_.getEntry(idx);
	u_int8_t cs[ANOUBIS_CS_LEN];

	if (task->getCsum(cs, sizeof(cs)) != ANOUBIS_CS_LEN) {
		/* TODO */
		return;
	}

	if (entry->setChecksum(type, cs, ANOUBIS_CS_LEN))
		sendEntryChangedEvent(idx);
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

	if (taskList_.IndexOf(task) == wxNOT_FOUND) {
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
			    strerror(task->getResultDetails()),
			    task->getPath().c_str());
		else
			message = wxString::Format(_("Got error from daemon "
			    "(%hs) while removing the signature for %ls."),
			    strerror(task->getResultDetails()),
			    task->getPath().c_str());

		errorList_.Add(message);
	} else if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		wxString message;

		if (type == SfsEntry::SFSENTRY_CHECKSUM)
			message = wxString::Format(_("An unexpected error "
			    "occured (%hs) while removing the checksum for "
			    "%ls."), strerror(task->getComTaskResult()),
			    task->getPath().c_str());
		else
			message = wxString::Format(_("An unexpected error "
			    "occured (%hs) while removing the signature for "
			    "%ls."), strerror(task->getComTaskResult()),
			    task->getPath().c_str());

		errorList_.Add(message);
	}

	/* Update model */
	SfsEntry *entry = sfsDir_.getEntry(idx);
	if (entry->setChecksumMissing(type))
		sendEntryChangedEvent(idx);

	if (!entry->fileExists()) {
		if (!entry->haveChecksum()) {
			sfsDir_.removeEntry(idx);
			sendDirChangedEvent();
		}
	}
}

void
SfsCtrl::OnUpgradeListArrived(TaskEvent &event)
{
	PopTaskHelper		 taskHelper(this);
	ComTask::ComTaskResult	 comResult;
	SfsEntry::ChecksumType	 type;
	int			 upgradeIdx;
	wxArrayString		 fileList;
	wxString		 errMsg;
	SfsEntry		*entry;
	ComUpgradeListGetTask	*task;

	task = dynamic_cast<ComUpgradeListGetTask*>(event.getTask());
	comResult = ComTask::RESULT_LOCAL_ERROR;
	type = SfsEntry::SFSENTRY_CHECKSUM;
	fileList.Clear();
	errMsg = wxEmptyString;

	if (task == 0) {
		/* No ComUpgradeListGetTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (taskList_.IndexOf(task) == wxNOT_FOUND) {
		/* Belongs to someone other, ignore it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */
	comResult = task->getComTaskResult();
	taskHelper.setTask(task);
	type = task->haveKeyId() ?
	    SfsEntry::SFSENTRY_SIGNATURE : SfsEntry::SFSENTRY_CHECKSUM;

	if (comResult != ComTask::RESULT_SUCCESS) {
		if (comResult == ComTask::RESULT_COM_ERROR) {
			errMsg = wxString::Format(_("Communication "
			    "error while fetching list of upgraded "
			    "files."));
		} else if (comResult == ComTask::RESULT_REMOTE_ERROR) {
			errMsg = wxString::Format(_("Got error "
			    "(%hs) from daemon while fetching list"
			    "of upgraded files."),
			    strerror(task->getResultDetails()));
		} else {
			errMsg = wxString::Format(_("An unexpected "
			    "error (%i) occured while while fetching list"
			    "of upgraded files."), task->getComTaskResult());
		}
		errorList_.Add(errMsg);
	} else {
		fileList = task->getFileList();
		for (unsigned int idx=0; idx<sfsDir_.getNumEntries(); idx++) {
			entry = sfsDir_.getEntry(idx);
			upgradeIdx = fileList.Index(entry->getPath());
			if (upgradeIdx != wxNOT_FOUND) {
				entry->setUpgraded();
				fileList.RemoveAt(upgradeIdx);
			}
		}
		for (unsigned int idx=0; idx<fileList.Count(); idx++) {
			sfsDir_.insertEntry(fileList.Item(idx));
		}
		sfsDir_.cleanup();
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
	JobCtrl::getInstance()->Connect(anTASKEVT_UPGRADE_LIST,
	    wxTaskEventHandler(SfsCtrl::OnUpgradeListArrived), NULL, this);
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
	JobCtrl::getInstance()->Disconnect(anTASKEVT_UPGRADE_LIST,
	    wxTaskEventHandler(SfsCtrl::OnUpgradeListArrived), NULL, this);
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
SfsCtrl::sendEntryChangedEvent(int idx)
{
	wxCommandEvent event(anEVT_SFSENTRY_CHANGED);
	event.SetEventObject(this);
	event.SetInt(idx);

	ProcessEvent(event);
}

void
SfsCtrl::sendErrorEvent(void)
{
	wxCommandEvent event(anEVT_SFSENTRY_ERROR);
	event.SetEventObject(this);

	ProcessEvent(event);
}

void
SfsCtrl::createComCsumGetTasks(const wxString &path, bool createCsum,
    bool createSig)
{
	if (createCsum) {
		/* Ask anoubisd for the checksum */
		ComCsumGetTask *csTask = new ComCsumGetTask;
		csTask->setPath(path);
		csTask->setCalcLink(true);

		pushTask(csTask);
		JobCtrl::getInstance()->addTask(csTask);
	}

	if (createSig && isSignatureEnabled()) {
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &cert = keyCtrl->getLocalCertificate();
		struct anoubis_sig *raw_cert = cert.getCertificate();

		ComCsumGetTask *sigTask = new ComCsumGetTask;
		sigTask->setPath(path);
		sigTask->setCalcLink(true);
		sigTask->setKeyId(raw_cert->keyid, raw_cert->idlen);

		pushTask(sigTask);
		JobCtrl::getInstance()->addTask(sigTask);
	}
}

void
SfsCtrl::createComCsumAddTasks(const wxString &path)
{
	ComCsumAddTask *csTask = new ComCsumAddTask;
	csTask->setPath(path);
	csTask->setCalcLink(true);

	pushTask(csTask);
	JobCtrl::getInstance()->addTask(csTask);

	if (isSignatureEnabled()) {
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &cert = keyCtrl->getLocalCertificate();
		PrivKey &privKey = keyCtrl->getPrivateKey();
		struct anoubis_sig *raw_cert = cert.getCertificate();

		ComCsumAddTask *sigTask = new ComCsumAddTask;
		sigTask->setPath(path);
		sigTask->setCalcLink(true);
		sigTask->setKeyId(raw_cert->keyid, raw_cert->idlen);
		sigTask->setPrivateKey(privKey.getKey());

		pushTask(sigTask);
		JobCtrl::getInstance()->addTask(sigTask);
	}
}

void
SfsCtrl::createComCsumAddTasks(struct sfs_entry *entry)
{
	if (entry->checksum != 0) {
		ComCsumAddTask *csTask = new ComCsumAddTask;
		csTask->setPath(wxString::FromAscii(entry->name));
		csTask->setCalcLink(true);
		csTask->setSfsEntry(entry, true);

		pushTask(csTask);
		JobCtrl::getInstance()->addTask(csTask);
	}

	if (entry->signature != 0) {
		/* Additionally send signature to daemon */
		ComCsumAddTask *sigTask = new ComCsumAddTask;
		sigTask->setPath(wxString::FromAscii(entry->name));
		sigTask->setCalcLink(true);
		sigTask->setSfsEntry(entry, false);

		pushTask(sigTask);
		JobCtrl::getInstance()->addTask(sigTask);
	}
}

int
SfsCtrl::createComCsumDelTasks(SfsEntry *entry)
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
	if ((sigState != SfsEntry::SFSENTRY_MISSING) && isSignatureEnabled()) {
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
SfsCtrl::createSfsListTasks(uid_t uid, const wxString &path, bool orphaned)
{
	ComSfsListTask *csTask = new ComSfsListTask;
	csTask->setRequestParameter(uid, path);
	csTask->setRecursive(isRecursive());
	csTask->setFetchOrphaned(orphaned);

	pushTask(csTask);
	JobCtrl::getInstance()->addTask(csTask);

	if (isSignatureEnabled()) {
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &cert = keyCtrl->getLocalCertificate();
		struct anoubis_sig *raw_cert = cert.getCertificate();

		ComSfsListTask *sigTask = new ComSfsListTask;
		sigTask->setRequestParameter(uid, path);
		sigTask->setRecursive(isRecursive());
		sigTask->setFetchOrphaned(orphaned);
		sigTask->setKeyId(raw_cert->keyid, raw_cert->idlen);

		pushTask(sigTask);
		JobCtrl::getInstance()->addTask(sigTask);
	}
}

void
SfsCtrl::createUpgradeListGetTask(void)
{
	ComUpgradeListGetTask *task = new ComUpgradeListGetTask();
	task->setRequestParameter(geteuid());

	pushTask(task);
	JobCtrl::getInstance()->addTask(task);
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
		    exportFile_.c_str(), strerror(errno)));
		return;
	}

	anoubis_print_entries(fh, entry_list, exportList_.size());

	if (fflush(fh) != 0) {
		errorList_.Add(wxString::Format(
		    _("Failed to flush the content of %ls: %hs"),
		    exportFile_.c_str(), strerror(errno)));
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

bool
SfsCtrl::setChecksumMissing(SfsEntry *entry)
{
	bool ret = false;

	ret |= entry->setChecksumMissing(SfsEntry::SFSENTRY_CHECKSUM);

	if (isSignatureEnabled()) {
		ret |= entry->setChecksumMissing(SfsEntry::SFSENTRY_SIGNATURE);
	}

	return (ret);
}

void
SfsCtrl::pushTask(Task *task)
{
	if (taskList_.size() == 0) {
		/*
		 * This is the first task in the list -> an operation began.
		 * Clear the error-list
		 */
		errorList_.Clear();
	}

	taskList_.push_back(task);
}

bool
SfsCtrl::popTask(Task *task)
{
	bool result = taskList_.DeleteObject(task);

	if (taskList_.size() == 0) {
		/*
		 * This was the last task in the list -> the operation finished
		 */

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

	return (result);
}

SfsCtrl::PopTaskHelper::PopTaskHelper(SfsCtrl *sfsCtrl, Task *task)
{
	this->sfsCtrl_ = sfsCtrl;
	this->task_ = task;
}

SfsCtrl::PopTaskHelper::~PopTaskHelper()
{
	if (task_ != 0) {
		if (sfsCtrl_->popTask(task_))
			delete task_;
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

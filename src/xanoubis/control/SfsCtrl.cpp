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

#include "AnEvents.h"
#include "AnUtils.h"
#include "ComCsumAddTask.h"
#include "ComCsumDelTask.h"
#include "ComCsumGetTask.h"
#include "ComRegistrationTask.h"
#include "ComSfsListTask.h"
#include "CsumCalcTask.h"
#include "JobCtrl.h"
#include "KeyCtrl.h"
#include "SfsCtrl.h"

SfsCtrl::SfsCtrl(void)
{
	this->comEnabled_ = false; /* Communication disabled */
	this->sigEnabled_ = true; /* Signature support enabled */

	JobCtrl::getInstance()->Connect(anTASKEVT_REGISTER,
	    wxTaskEventHandler(SfsCtrl::OnRegistration), NULL, this);
}

wxString
SfsCtrl::getPath() const
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
SfsCtrl::getFilter() const
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
SfsCtrl::isFilterInversed() const
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
		for (size_t i = 0; i < arr.Count(); i++) {
			unsigned int idx = arr.Item(i);

			if (idx >= sfsDir_.getNumEntries()) {
				/* Out of range */
				return (RESULT_INVALIDARG);
			}

			SfsEntry &entry = sfsDir_.getEntry(idx);
			entry.reset();

			/* Ask anoubisd for the checksums */
			createComCsumGetTasks(entry.getPath(), true, true);

			/* Calculate the checksum */
			createCsumCalcTask(entry.getPath());
		}

		return (RESULT_EXECUTE);
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
	if (!taskList_.empty())
		return (RESULT_BUSY);

	if (comEnabled_) {
		if (!isSignatureEnabled()) {
			/* Reset signed checksums in model */
			for (unsigned int idx = 0;
			    idx < sfsDir_.getNumEntries(); idx++) {
				SfsEntry &entry = sfsDir_.getEntry(idx);

				if (entry.reset(SfsEntry::SFSENTRY_SIGNATURE))
					sendEntryChangedEvent(idx);
			}

		}

		/* Ask for sfs-list */
		createSfsListTasks(getuid(), sfsDir_.getPath());

		return (RESULT_EXECUTE);
	}
	else
		return (RESULT_NOTCONNECTED);
}

SfsCtrl::CommandResult
SfsCtrl::registerChecksum(const IndexArray &arr)
{
	if (!taskList_.empty())
		return (RESULT_BUSY);

	if (comEnabled_) {
		for (size_t i = 0; i < arr.Count(); i++) {
			unsigned int idx = arr.Item(i);

			if (idx >= sfsDir_.getNumEntries()) {
				/* Out of range */
				return (RESULT_INVALIDARG);
			}

			if (isSignatureEnabled()) {
				/* Need a loaded private key */
				KeyCtrl *keyCtrl = KeyCtrl::getInstance();
				PrivKey &privKey = keyCtrl->getPrivateKey();

				if (!privKey.isLoaded())
					return (RESULT_NEEDPASS);
			}

			SfsEntry &entry = sfsDir_.getEntry(idx);

			/* Send checksum to anoubisd */
			createComCsumAddTasks(entry.getPath());

			if (!entry.haveLocalCsum()) {
				/*
				 * You also need a local checksum to be able to
				 * compare it with the remote one
				 */
				createCsumCalcTask(entry.getPath());
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
		for (size_t i = 0; i < arr.Count(); i++) {
			unsigned int idx = arr.Item(i);

			if (idx >= sfsDir_.getNumEntries()) {
				/* Out of range */
				return (RESULT_INVALIDARG);
			}

			SfsEntry &entry = sfsDir_.getEntry(idx);

			/* Remove checksum from anoubisd */
			createComCsumDelTasks(entry.getPath());
		}

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

SfsCtrl::CommandResult
SfsCtrl::updateChecksum(const IndexArray &arr)
{
	/* Equal to register a checksum */
	return (registerChecksum(arr));
}

SfsCtrl::CommandResult
SfsCtrl::updateChecksum(unsigned int idx)
{
	/* Equal to register a checksum */
	return (registerChecksum(idx));
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

	if (task->getComTaskResult() == ComTask::RESULT_COM_ERROR) {
		wxString message = wxString::Format(_(
		    "Communication error while fetching list of checksumed \
files of %s."), task->getDirectory().c_str());
		errorList_.Add(message);
	} else if (task->getComTaskResult() == ComTask::RESULT_REMOTE_ERROR) {
		wxString message = wxString::Format(
		    _("Got error from daemon (%s) while fetching the list of \
checksumed files of %s"),
		    wxStrError(task->getResultDetails()).c_str(),
		    task->getDirectory().c_str());
		errorList_.Add(message);
	} else if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		wxString message = wxString::Format(
		    _("An unexpected error occured (%i) while fetching the \
list of checksumed files of %s."),
		    task->getComTaskResult(),
		    task->getDirectory().c_str());
		errorList_.Add(message);
	} else
		result = task->getFileList();

	/* Go though each file in the model, each file needs to be updated */
	for (unsigned int idx = 0; idx < sfsDir_.getNumEntries(); idx++) {
		SfsEntry &entry = sfsDir_.getEntry(idx);

		/* Has file a checksum? */
		int csumIdx = result.Index(entry.getFileName());

		if (csumIdx != wxNOT_FOUND) {
			/* Ask anoubisd for the checksum */
			createComCsumGetTasks(entry.getPath(),
			    !task->haveKeyId(), task->haveKeyId());

			/* Calculate the checksum */
			createCsumCalcTask(entry.getPath());
		} else {
			/* Update checksum attribute to no-checksum */
			SfsEntry::ChecksumType type = task->haveKeyId() ?
			    SfsEntry::SFSENTRY_SIGNATURE :
			    SfsEntry::SFSENTRY_CHECKSUM;

			if (entry.setChecksumMissing(type))
				sendEntryChangedEvent(idx);
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
	int idx = sfsDir_.getIndexOf(task->getPath(), true);
	if (idx == -1) {
		wxString message = wxString::Format(
		    _("%s not found in file-list!"), task->getPath().c_str());
		errorList_.Add(message);

		return;
	}

	if (task->getResult() != 0) {
		/* Calculation failed */
		wxString message = wxString::Format(
		    _("Failed to calculate the checksum for %s: %s"),
		    task->getPath().c_str(),
		    wxStrError(task->getResult()).c_str());
		errorList_.Add(message);

		return;
	}

	/* Copy checksum into SfsEntry */
	SfsEntry &entry = sfsDir_.getEntry(idx);
	if (entry.setLocalCsum(task->getCsum())) {
		/* Checksum attribute has changed, inform any listener */
		sendEntryChangedEvent(idx);
	}
}

void
SfsCtrl::OnCsumGet(TaskEvent &event)
{
	ComCsumGetTask	*task = dynamic_cast<ComCsumGetTask*>(event.getTask());
	PopTaskHelper	taskHelper(this, task);
	u_int8_t	cs[ANOUBIS_CS_LEN];

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
	int idx = sfsDir_.getIndexOf(task->getFile(), true);
	if (idx == -1) {
		wxString message = wxString::Format(
		    _("%s not found in file-list!"), task->getFile().c_str());
		errorList_.Add(message);

		return;
	}

	/* Recieve checksum from task */
	SfsEntry &entry = sfsDir_.getEntry(idx);
	ComTask::ComTaskResult taskResult = task->getComTaskResult();
	SfsEntry::ChecksumType type = task->haveKeyId() ?
	    SfsEntry::SFSENTRY_SIGNATURE : SfsEntry::SFSENTRY_CHECKSUM;

	if (taskResult == ComTask::RESULT_REMOTE_ERROR) {
		int err = task->getResultDetails();

		if (err == ENOENT) {
			/* No checksum registered */
			if (entry.setChecksumMissing(type))
				sendEntryChangedEvent(idx);
		} else if (err == EINVAL) {
			if (entry.setChecksumInvalid(type))
				sendEntryChangedEvent(idx);
		} else {
			wxString message = wxString::Format(
			    _("Got error from daemon (%s) while fetching the "
			    "checksum for %s."),
			    wxStrError(task->getResultDetails()).c_str(),
			    task->getFile().c_str());
			errorList_.Add(message);
		}

		return;
	} else if (taskResult == ComTask::RESULT_COM_ERROR) {
		wxString message = wxString::Format(_(
		    "Communication error while fetching the checksum for %s."),
		    task->getFile().c_str());
		errorList_.Add(message);

		return;
	} else if (taskResult != ComTask::RESULT_SUCCESS) {
		wxString message = wxString::Format(
		    _("An unexpected error occured (%i) while fetching the "
		    "checksum for %s."), task->getComTaskResult(),
		    task->getFile().c_str());
		errorList_.Add(message);

		return;
	}

	if (task->getCsum(cs, ANOUBIS_CS_LEN) == ANOUBIS_CS_LEN) {
		/* File has a checksum, copy into model */
		if (entry.setChecksum(type, cs))
			sendEntryChangedEvent(idx);
	} else {
		wxString message = _("An unexpected checksum was fetched.");
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
	int idx = sfsDir_.getIndexOf(task->getFile(), true);
	if (idx == -1) {
		wxString message = wxString::Format(
		    _("%s not found in file-list!"), task->getFile().c_str());
		errorList_.Add(message);

		return;
	}

	if (task->getComTaskResult() == ComTask::RESULT_LOCAL_ERROR) {
		wxString message;
		message.Printf(
		    _("Failed to calculate the checksum for %s: %s"),
		    task->getFile().c_str(),
		    wxStrError(task->getResultDetails()).c_str());
		errorList_.Add(message);

		return;
	} else if (task->getComTaskResult() == ComTask::RESULT_COM_ERROR) {
		wxString message = wxString::Format(_(
		    "Communication error while register the checksum for %s."),
		    task->getFile().c_str());
		errorList_.Add(message);

		return;
	} else if (task->getComTaskResult() == ComTask::RESULT_REMOTE_ERROR) {
		wxString message = wxString::Format(
		    _("Got error from daemoon (%s) while register the \
checksum for %s."), wxStrError(task->getResultDetails()).c_str(),
		    task->getFile().c_str());
		errorList_.Add(message);

		return;
	} else if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		wxString message = wxString::Format(
		    _("An unexpected error occured (%i) while register the \
checksum for %s."), task->getComTaskResult(), task->getFile().c_str());
		errorList_.Add(message);

		return;
	}

	/* Update model */
	SfsEntry &entry = sfsDir_.getEntry(idx);
	SfsEntry::ChecksumType type = task->haveKeyId() ?
	    SfsEntry::SFSENTRY_SIGNATURE : SfsEntry::SFSENTRY_CHECKSUM;
	u_int8_t cs[ANOUBIS_CS_LEN];

	if (task->getCsum(cs, sizeof(cs)) != ANOUBIS_CS_LEN) {
		/* TODO */
		return;
	}

	if (entry.setChecksum(type, cs))
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
	int idx = sfsDir_.getIndexOf(task->getFile(), true);
	if (idx == -1) {
		wxString message = wxString::Format(
		    _("%s not found in file-list!"), task->getFile().c_str());
		errorList_.Add(message);

		return;
	}

	if (task->getComTaskResult() == ComTask::RESULT_COM_ERROR) {
		wxString message = wxString::Format(_(
		    "Communication error while removing the checksum for %s."),
		    task->getFile().c_str());
		errorList_.Add(message);
	} else if (task->getComTaskResult() == ComTask::RESULT_REMOTE_ERROR) {
		wxString message = wxString::Format(
		    _("Got error from daemoon (%s) while removing the \
checksum for %s."), wxStrError(task->getResultDetails()).c_str(),
		    task->getFile().c_str());
		errorList_.Add(message);
	} else if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		wxString message = wxString::Format(
		    _("An unexpected error occured (%i) while removing the \
checksum for %s."), task->getComTaskResult(), task->getFile().c_str());
		errorList_.Add(message);
	}

	/* Update model */
	SfsEntry &entry = sfsDir_.getEntry(idx);
	SfsEntry::ChecksumType type = task->haveKeyId() ?
	    SfsEntry::SFSENTRY_SIGNATURE : SfsEntry::SFSENTRY_CHECKSUM;

	if (entry.setChecksumMissing(type))
		sendEntryChangedEvent(idx);
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
		csTask->setFile(path);

		pushTask(csTask);
		JobCtrl::getInstance()->addTask(csTask);
	}

	if (createSig && isSignatureEnabled()) {
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &cert = keyCtrl->getLocalCertificate();
		struct anoubis_sig *raw_cert = cert.getCertificate();

		ComCsumGetTask *sigTask = new ComCsumGetTask;
		sigTask->setFile(path);
		sigTask->setKeyId(raw_cert->keyid, raw_cert->idlen);

		pushTask(sigTask);
		JobCtrl::getInstance()->addTask(sigTask);
	}
}

void
SfsCtrl::createComCsumAddTasks(const wxString &path)
{
	ComCsumAddTask *csTask = new ComCsumAddTask;
	csTask->setFile(path);

	pushTask(csTask);
	JobCtrl::getInstance()->addTask(csTask);

	if (isSignatureEnabled()) {
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &cert = keyCtrl->getLocalCertificate();
		PrivKey &privKey = keyCtrl->getPrivateKey();
		struct anoubis_sig *raw_cert = cert.getCertificate();

		ComCsumAddTask *sigTask = new ComCsumAddTask;
		sigTask->setFile(path);
		sigTask->setKeyId(raw_cert->keyid, raw_cert->idlen);
		sigTask->setPrivateKey(privKey.getKey());

		pushTask(sigTask);
		JobCtrl::getInstance()->addTask(sigTask);
	}
}

void
SfsCtrl::createComCsumDelTasks(const wxString &path)
{
	ComCsumDelTask *csTask = new ComCsumDelTask;
	csTask->setFile(path);

	pushTask(csTask);
	JobCtrl::getInstance()->addTask(csTask);

	if (isSignatureEnabled()) {
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &cert = keyCtrl->getLocalCertificate();
		struct anoubis_sig *raw_cert = cert.getCertificate();

		ComCsumDelTask *sigTask = new ComCsumDelTask;
		sigTask->setFile(path);
		sigTask->setKeyId(raw_cert->keyid, raw_cert->idlen);

		pushTask(sigTask);
		JobCtrl::getInstance()->addTask(sigTask);
	}
}

void
SfsCtrl::createSfsListTasks(uid_t uid, const wxString &path)
{
	ComSfsListTask *csTask = new ComSfsListTask;
	csTask->setRequestParameter(uid, path);

	pushTask(csTask);
	JobCtrl::getInstance()->addTask(csTask);

	if (isSignatureEnabled()) {
		KeyCtrl *keyCtrl = KeyCtrl::getInstance();
		LocalCertificate &cert = keyCtrl->getLocalCertificate();
		struct anoubis_sig *raw_cert = cert.getCertificate();

		ComSfsListTask *sigTask = new ComSfsListTask;
		sigTask->setRequestParameter(uid, path);
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

	pushTask(task);
	JobCtrl::getInstance()->addTask(task);
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
		 * Check for errors.
		 */
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

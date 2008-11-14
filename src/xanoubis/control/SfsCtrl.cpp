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

#include "AnEvents.h"
#include "ComCsumAddTask.h"
#include "ComCsumDelTask.h"
#include "ComCsumGetTask.h"
#include "ComRegistrationTask.h"
#include "CsumCalcTask.h"
#include "JobCtrl.h"
#include "SfsCtrl.h"

SfsCtrl::SfsCtrl(void)
{
	this->comEnabled_ = false; /* Communication disabled */

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
SfsCtrl::validate(unsigned int idx)
{
	if (comEnabled_) {
		if (idx >= sfsDir_.getNumEntries()) {
			/* Out of range */
			return (false);
		}

		SfsEntry &entry = sfsDir_.getEntry(idx);
		entry.reset();

		/* Ask anoubisd for the checksum */
		ComCsumGetTask *comTask = new ComCsumGetTask;
		comTask->setFile(entry.getPath());
		JobCtrl::getInstance()->addTask(comTask);

		/* Calculate the checksum */
		CsumCalcTask *calcTask = new CsumCalcTask;
		calcTask->setPath(entry.getPath());
		JobCtrl::getInstance()->addTask(calcTask);

		return (true);
	} else
		return (false);
}

bool
SfsCtrl::validateAll(void)
{
	if (comEnabled_) {
		/* Ask for sfs-list */
		sfsListTask_.setRequestParameter(getuid(), sfsDir_.getPath());
		JobCtrl::getInstance()->addTask(&sfsListTask_);

		return (true);
	}
	else
		return (false);
}

bool
SfsCtrl::registerChecksum(unsigned int idx)
{
	if (comEnabled_) {
		if (idx >= sfsDir_.getNumEntries()) {
			/* Out of range */
			return (false);
		}

		SfsEntry &entry = sfsDir_.getEntry(idx);

		/* Send checksum to anoubisd */
		ComCsumAddTask *task = new ComCsumAddTask;
		task->setFile(entry.getPath());
		JobCtrl::getInstance()->addTask(task);

		return (true);
	} else
		return (false);
}

bool
SfsCtrl::unregisterChecksum(unsigned int idx)
{
	if (comEnabled_) {
		if (idx >= sfsDir_.getNumEntries()) {
			/* Out of range */
			return (false);
		}

		SfsEntry &entry = sfsDir_.getEntry(idx);

		/* Remove checksum from anoubisd */
		ComCsumDelTask *task = new ComCsumDelTask;
		task->setFile(entry.getPath());
		JobCtrl::getInstance()->addTask(task);

		return (true);
	} else
		return (false);
}

bool
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
	if (event.getTask() == &sfsListTask_) {
		/* Internal task -> stop propagating */
		event.Skip(false);
	} else {
		/* Belongs to someone other */
		event.Skip();
		return;
	}

	/* Files which has a checksum */
	wxArrayString result;

	if (sfsListTask_.getComTaskResult() == ComTask::RESULT_SUCCESS)
		result = sfsListTask_.getFileList();

	/* Go though each file in the model, each file needs to be updated */
	for (unsigned int idx = 0; idx < sfsDir_.getNumEntries(); idx++) {
		SfsEntry &entry = sfsDir_.getEntry(idx);

		/* Has file a checksum? */
		int csumIdx = result.Index(entry.getFileName());

		if (csumIdx != wxNOT_FOUND) {
			/* Ask anoubisd for the checksum */
			ComCsumGetTask *comTask = new ComCsumGetTask;
			comTask->setFile(entry.getPath());
			JobCtrl::getInstance()->addTask(comTask);

			/* Calculate the checksum */
			CsumCalcTask *calcTask = new CsumCalcTask;
			calcTask->setPath(entry.getPath());
			JobCtrl::getInstance()->addTask(calcTask);
		} else {
			/* Update checksum attribute to no-checksum */
			if (entry.setNoChecksum())
				sendEntryChangedEvent(idx);
		}
	}
}

void
SfsCtrl::OnCsumCalc(TaskEvent &event)
{
	CsumCalcTask *task = dynamic_cast<CsumCalcTask*>(event.getTask());

	if (task == 0) {
		/* No ComCsumGetTask -> stop propagating */
		event.Skip(false);
		return;
	}

	/* Search for SfsEntry */
	int idx = sfsDir_.getIndexOf(task->getPath(), true);
	if (idx == -1) {
		/*
		 * No such file in model, continue propagating, maybe another
		 * is interesed in
		 */
		event.Skip();
		return;
	}

	/* This is "my" task -> stop propageting */
	event.Skip(false);

	if (task->getResult() != 0) {
		/* Calculation failed */
		return;
	}

	/* Copy checksum into SfsEntry */
	SfsEntry &entry = sfsDir_.getEntry(idx);
	if (entry.setLocalCsum(task->getCsum())) {
		/* Checksum attribute has changed, inform any listener */
		sendEntryChangedEvent(idx);
	}

	/* Created in OnSfsListArrived */
	delete task;
}

void
SfsCtrl::OnCsumGet(TaskEvent &event)
{
	ComCsumGetTask *task = dynamic_cast<ComCsumGetTask*>(event.getTask());
	u_int8_t cs[ANOUBIS_CS_LEN];

	if (task == 0) {
		/* No ComCsumGetTask -> stop propagating */
		event.Skip(false);
		return;
	}

	/* Search for SfsEntry */
	int idx = sfsDir_.getIndexOf(task->getFile(), true);
	if (idx == -1) {
		/*
		 * No such file in model, continue propagating, maybe another
		 * is interesed in
		 */
		event.Skip();
		return;
	}

	/* This is "my" task -> stop propagating */
	event.Skip(false);

	if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		/* Failure */
		return;
	}

	/* Recieve checksum from task */
	task->getCsum(cs, ANOUBIS_CS_LEN);

	/* Copy checksum intp SfsEntry */
	SfsEntry &entry = sfsDir_.getEntry(idx);
	if (entry.setDaemonCsum(cs)) {
		/* Checksum attribute has changed, inform any listener */
		sendEntryChangedEvent(idx);
	}

	/* Created in OnSfsListArrived */
	delete task;
}

void
SfsCtrl::OnCsumAdd(TaskEvent &event)
{
	ComCsumAddTask *task = dynamic_cast<ComCsumAddTask*>(event.getTask());

	if (task == 0) {
		/* No ComCsumAddTask -> stop propagating */
		event.Skip(false);
		return;
	}

	/* Search for SfsEntry */
	int idx = sfsDir_.getIndexOf(task->getFile(), true);
	if (idx == -1) {
		/*
		 * No such file in model, continue propagating, maybe another
		 * is interesed in
		 */
		event.Skip();
		return;
	}

	/* This is "my" task -> stop propagating */
	event.Skip(false);

	if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		/* Failure */
		return;
	}

	delete task;

	/* Update model */
	validate(idx);
}

void
SfsCtrl::OnCsumDel(TaskEvent &event)
{
	ComCsumDelTask *task = dynamic_cast<ComCsumDelTask*>(event.getTask());

	if (task == 0) {
		/* No ComCsumAddTask -> stop propagating */
		event.Skip(false);
		return;
	}

	/* Search for SfsEntry */
	int idx = sfsDir_.getIndexOf(task->getFile(), true);
	if (idx == -1) {
		/*
		 * No such file in model, continue propagating, maybe another
		 * is interesed in
		 */
		event.Skip();
		return;
	}

	/* This is "my" task -> stop propagating */
	event.Skip(false);

	delete task;

	/* Update model */
	validate(idx);
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

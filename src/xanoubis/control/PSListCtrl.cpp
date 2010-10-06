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

#include "PSListCtrl.h"
#include "JobCtrl.h"
#include "PSListTask.h"
#include "PSEntry.h"
#include "anoubis_protocol.h"
#include "anoubis_procinfo.h"
#include "anoubis_errno.h"

/* Explicit instantiation of singleton base class. */
#include <Singleton.cpp>
template class Singleton<PSListCtrl>;

PSListCtrl::PSListCtrl(void)
{
	psinfo_.clearRows();
	JobCtrl::instance()->Connect(anTASKEVT_PS_LIST,
	    wxTaskEventHandler(PSListCtrl::OnPSListArrived), NULL, this);
}

PSListCtrl::~PSListCtrl(void)
{
	if (JobCtrl::existingInstance()) {
		JobCtrl::existingInstance()->Disconnect(anTASKEVT_PS_LIST,
		    wxTaskEventHandler(PSListCtrl::OnPSListArrived), NULL,
		    this);
	}
	clearPSList();
}

void
PSListCtrl::clearPSList()
{
	while (psinfo_.getRawSize() > 0) {
		int			 idx = psinfo_.getRawSize()-1;
		AnListClass		*obj = psinfo_.getRawRow(idx);

		psinfo_.removeRawRow(idx);
		delete obj;
	}
	psinfo_.clearRows();
}

void
PSListCtrl::updatePSList(void)
{
	PSListTask		*task = new PSListTask();

	addTask(task);
	JobCtrl::instance()->addTask(task);
	/*
	 * Clear the process list now. This gives the user a visual
	 * feedback that the task was started.
	 */
	clearPSList();
}

AnRowProvider *
PSListCtrl::getPSListProvider(void)
{
	return &psinfo_;
}

const PSEntry *
PSListCtrl::getEntry(int idx) const
{
	AnListClass		*obj;

	if (idx < 0 || idx >= psinfo_.getSize())
		return NULL;
	obj = psinfo_.getRow(idx);
	return dynamic_cast<PSEntry *>(obj);
}

void
PSListCtrl::OnPSListArrived(TaskEvent &event)
{
	PSListTask		*task;

	task = dynamic_cast<PSListTask *>(event.getTask());
	if (!isValidTask(task)) {
		event.Skip();
		return;
	}
	event.Skip(false); /* Our task. */
	clearPSList(); /* Should already be empty, but better be sure. */
	switch (task->getComTaskResult()) {
	case ComTask::RESULT_COM_ERROR:
		addError(_("Communication error during process list request."));
		break;
	case ComTask::RESULT_REMOTE_ERROR:
		addError(wxString::Format(_("The daemon returned an "
		    "error for the process list request: %hs"),
		    anoubis_strerror(task->getResultDetails())));
		break;
	case ComTask::RESULT_LOCAL_ERROR:
		addError(wxString::Format(_("Error in process list request: "
		    "%hs"), anoubis_strerror(task->getResultDetails())));
		break;
	case ComTask::RESULT_SUCCESS:
		break;
	default:
		addError(wxString::Format(_("Got unexpected result (%d) "
		    "for process list request"), task->getComTaskResult()));
	}
	if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		removeTask(task);
		delete task;
		sendEvent(anEVT_PSLIST_ERROR);
		return;
	}
	const Anoubis_ProcRecord		*daemonproc;
	struct anoubis_proc			*sysproc;
	struct anoubis_proc_handle		*handle;

	handle = anoubis_proc_open();
	task->resetRecordIterator();
	while (task->readNextRecord()) {
		long		 pid;
		PSEntry		*entry;

		daemonproc = task->getProc();
		pid = get_value(daemonproc->pid);
		sysproc = anoubis_proc_get(handle, pid);
		entry = new PSEntry(daemonproc, sysproc);
		psinfo_.addRow(entry);
		if (sysproc)
			anoubis_proc_destroy(sysproc);
	}
	anoubis_proc_close(handle);
	removeTask(task);
	delete task;
}

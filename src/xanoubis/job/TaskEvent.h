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

#ifndef _TASKEVENT_H_
#define _TASKEVENT_H_

#include <wx/event.h>

/**
 * @name Event-types associated with TaskEvent.
 */
//@{
BEGIN_DECLARE_EVENT_TYPES()
	/**
	 * Sent by several tasks to indicate progress.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_PROGRESS, wxNewEventType())

	/**
	 * Sent by a dummy Task after execution.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_DUMMY, wxNewEventType())

	/**
	 * Event-type of TaskEvent when a checksum-calculation has finished.
	 * @see CsumCalcTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_CSUMCALC, wxNewEventType())

	/**
	 * Event-type of TaskEvent when the registration at anoubisd has
	 * finished.
	 * @see ComRegistrationTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_REGISTER, wxNewEventType())

	/**
	 * Event-type of TaskEvent when a policy was sent to anoubisd.
	 * @see ComPolicySendTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_POLICY_SEND, wxNewEventType())

	/**
	 * Event-type of TaskEvent when a policy-requested was answered by
	 * anoubisd.
	 * @see ComPolicyRequestTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_POLICY_REQUEST, wxNewEventType())

	/**
	 * Event-type of TaskEvent when a checksum was sent to anoubisd.
	 * @see ComCsumAddTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_CSUM_ADD, wxNewEventType())

	/**
	 * Event-type of TaskEvent when a checksum-request was answered by
	 * anoubisd.
	 * @see ComCsumGetTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_CSUM_GET, wxNewEventType())

	/**
	 * Event-type of TaskEvent when a checksum was removed from anoubisd.
	 * @see ComCsumDelTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_CSUM_DEL, wxNewEventType())

	/**
	 * Event-type of TaskEvent when a sfs-list was answered by anoubisd.
	 * @see ComSfsListTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_SFS_LIST, wxNewEventType())

	/**
	 * Event-type of TaskEvent when a version-list arrived from anoubisd.
	 * @see ComVersionTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_VERSION, wxNewEventType())

	/**
	 * Event-type of a TaskEvent when a list of playgrounds arrived from
	 * anoubisd.
	 * @see PlaygroundListTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_PG_LIST, wxNewEventType())

	/**
	 * Event-type of a TaskEvent when a list of file in a playground
	 * arrived from anoubisd.
	 * @see PlaygroundFilesTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_PG_FILES, wxNewEventType())

	/**
	 * Event-type of a TaskEvent when a list of processes arrived
	 * from anobuisd.
	 * @see PSListTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_PS_LIST, wxNewEventType())

	/**
	 * Event-type of TaskEvent when a list of files of a playground
	 * were deleted.
	 * @see PlaygroundUnlinkTask
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_PG_UNLINK, wxNewEventType())

	/**
	 * Event-type of a TaskEvent that is sennt after a playground
	 * commit task completes.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anTASKEVT_PG_COMMIT, wxNewEventType())
END_DECLARE_EVENT_TYPES()
//@}

class Task;

/**
 * Event is fired, when an Task was executed.
 */
class TaskEvent : public wxEvent
{
	public:
		TaskEvent(Task *, int);
		TaskEvent(Task *, int, int);
		TaskEvent(const TaskEvent &);

		wxEvent *Clone(void) const;

		/**
		 * Returns the task, which was executed.
		 * @return The task, which was exectued
		 */
		Task *getTask(void) const;

	private:
		Task		*task_;
};

typedef void (wxEvtHandler::*taskEventFunction)(TaskEvent&);

#define wxTaskEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction) \
    wxStaticCastEvent(taskEventFunction, &func)

#endif	/* _TASKEVENT_H_ */

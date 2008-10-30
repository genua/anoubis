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
#include "JobCtrl.h"
#include "JobThread.h"

JobThread::JobThread(JobCtrl *jobCtrl) : wxThread(wxTHREAD_JOINABLE)
{
	this->exitThread_ = false;
	this->jobCtrl_ = jobCtrl;
}

bool
JobThread::start(void)
{
	/*
	 * Create the thread
	 * Depending on the result, you need to start the thread.
	 */
	wxThreadError	result = Create();
	bool		success = true;

	if (result == wxTHREAD_NO_ERROR) {
		/* Thread was created, try to start it */
		if (Run() == wxTHREAD_NO_ERROR)
			success = true;
		else
			success = false;
	}
	else if (result != wxTHREAD_RUNNING)
		success = false;

	if (success) {
		/*
		 * Thread is running, invoke startHook() to complete the
		 * procedure.
		 */
		success = startHook();
	}

	return (success);
}

void
JobThread::stop(void)
{
	stopHook();

	/* Ask for quit and wait... */
	exitThread_ = true;
	Wait();
}

bool
JobThread::isRunning(void) const
{
	return (IsAlive());
}

bool
JobThread::startHook(void)
{
	return (true);
}

void
JobThread::stopHook(void)
{
}

bool
JobThread::exitThread(void) const
{
	return (this->exitThread_);
}

Task *
JobThread::getNextTask(Task::Type type)
{
	return jobCtrl_->popTask(type);
}

void
JobThread::sendEvent(wxEvent &event)
{
	jobCtrl_->AddPendingEvent(event);
}

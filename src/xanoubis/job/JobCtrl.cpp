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

#include <wx/thread.h>

#include "CsumCalcThread.h"
#include "JobCtrl.h"
#include "Singleton.cpp"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(JobThreadList);

JobCtrl::JobCtrl(void)
{
	/* Checksum calculation thread */
	CsumCalcThread *csumCalcThread = new CsumCalcThread(this);
	csumCalcThread->start();
	threadList_.push_back(csumCalcThread);
}

JobCtrl::~JobCtrl(void)
{
	/* Stop and destroy all background threads */
	while (!threadList_.empty()) {
		JobThread *thread = threadList_.front();
		threadList_.pop_front();

		thread->stop();
		delete thread;
	}
}

JobCtrl *
JobCtrl::getInstance(void)
{
	return (instance());
}

void
JobCtrl::addTask(Task *task)
{
	switch (task->getType()) {
	case Task::TYPE_CSUMCALC:
		pushCsumCalcTask(task);
		break;
	}
}

Task *
JobCtrl::popTask(Task::Type type)
{
	switch (type) {
	case Task::TYPE_CSUMCALC:
		return popCsumCalcTask();
	}

	return (0);
}

void
JobCtrl::pushCsumCalcTask(Task *task)
{
	wxMutexLocker locker(csumCalcTaskMutex_);
	csumCalcTaskQueue_.push(task);
}

Task *
JobCtrl::popCsumCalcTask(void)
{
	wxMutexLocker locker(csumCalcTaskMutex_);

	if (csumCalcTaskQueue_.empty())
		return (0);

	Task *task = csumCalcTaskQueue_.front();
	csumCalcTaskQueue_.pop();

	return (task);
}

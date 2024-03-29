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
#include "FsThread.h"
#include "Task.h"
#include "DummyTask.h"
#include "JobCtrl.h"

FsThread::FsThread(JobCtrl *jobCtrl) : JobThread(jobCtrl)
{
}

void *
FsThread::Entry(void)
{
	while (!exitThread()) {
		Task		*task = getNextTask(Task::TYPE_FS);
		DummyTask	*dummy;

		if (task == 0)
			continue;

		if (task->shallAbort()) {
			task->setTaskResultAbort();
		} else {
			task->exec();
			if (task->getType() != Task::TYPE_FS) {
				JobCtrl::instance()->addTask(task);
				continue;
			}
		}

		dummy = dynamic_cast<DummyTask *>(task);
		if (dummy) {
			delete dummy;
		} else {
			TaskEvent event(task, wxID_ANY);
			sendEvent(event);
		}
	}

	return (0);
}

void
FsThread::wakeup(bool isexit)
{
	if (isexit) {
		DummyTask	*task = new DummyTask(Task::TYPE_FS);
		JobCtrl::instance()->addTask(task);
	}
}

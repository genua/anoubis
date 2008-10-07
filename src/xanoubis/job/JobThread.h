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

#ifndef _JOBTHREAD_H_
#define _JOBTHREAD_H_

#include <wx/thread.h>

#include "Task.h"
#include "TaskEvent.h"

class JobCtrl;

/**
 * Base class for all JobCtrl-background-threads.
 *
 * The derivated class needs to implement wxThread::Entry(). The threads
 * main-loop should exit, if exitThread() returns true. The method
 * getNextTask() is used to receive a task from the execution-queue.
 *
 * <pre>
 * void *
 * MyJobThreadImpl::Entry()
 * {
 *     while (!exitThread()) {
 *         Task *t = getNextTask(...);
 *         if (t == 0)
 *             continue;
 *
 *         ...
 *     }
 * }
 * </pre>
 *
 * The thread is started with start() and stopped with stop().
 */
class JobThread : public wxThread
{
	public:
		/**
		 * Starts the thread.
		 * @return true on success, false otherwise.
		 */
		bool start(void);

		/**
		 * Stops the thread.
		 * The method blocks until the thread leaves the main-loop.
		 */
		void stop(void);

		/**
		 * Tests weather the thread is currently running.
		 * @return true if the thread is running, false otherwise.
		 */
		bool isRunning(void) const;

	protected:
		JobThread(JobCtrl *);

		/**
		 * Tests weather the thread should leave the main-loop.
		 * @return true if the thread should be exited, false
		 *         otherwise.
		 */
		bool exitThread(void) const;

		/**
		 * Receives a task from the execution-queue.
		 * @param type Type of requested task
		 * @return The requested task. If there is no scheduled task of
		 *         the requested type, 0 is returned.
		 */
		Task *getNextTask(Task::Type);

		/**
		 * Sends an event to the main event-loop.
		 * @param event The event to be send
		 */
		void sendEvent(wxEvent &);

	private:
		bool exitThread_;
		JobCtrl *jobCtrl_;
};

#endif	/* _JOBTHREAD_H_ */

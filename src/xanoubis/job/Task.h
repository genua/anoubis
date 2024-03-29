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

#ifndef _TASK_H_
#define _TASK_H_

#include <wx/event.h>

/**
 * A background-task controlled by JobCtrl.
 *
 * A task has a type (returned by Task::getType()). The type tells the JobCtrl,
 * which background-thread has to be used.
 *
 * The logic of the task is implemented in Task::exec(). The return-code of the
 * exec-method is the result of the operation. Together with
 * Task::getEventType(), an TaskEvent is created afterwards and posted to the
 * event-loop.
 */
class Task
{
	public:
		/**
		 * Type of task.
		 */
		enum Type
		{
			TYPE_FS = 0,	/*!< Filesystem ops like csum calc */
			TYPE_COM	/*!< Daemon-communication */
		};

		virtual ~Task(void);

		/**
		 * Returns the type of the task.
		 *
		 * The type is used to find the correct background-thread, i.e.
		 * for each type there is an separate background-thread.
		 *
		 * @return Type of task
		 */
		Type getType(void) const;

		/**
		 * Returns the event-type associated with this task.
		 *
		 * This event-type is used for sending a TaskEvent afterwards.
		 *
		 * @return Event-type of the task
		 * @see wxEvent::GetEventType()
		 */
		virtual wxEventType getEventType(void) const = 0;

		/**
		 * Executes the task.
		 *
		 * This method implements the logic of the task. It returns
		 * void, thus derivated classes should provide an interface
		 * to receive a task-specific result.
		 */
		virtual void exec(void) = 0;

		/**
		 * Abort this task.
		 * @param None.
		 * @return Nothing.
		 */
		void abort(void);

		/**
		 * Get abort status.
		 * @param None.
		 * @return True if abort was requested.
		 */
		bool shallAbort(void) const;

		/**
		 * Task must provide an implementation that sets a task
		 * result code as appropriate for an aborted task.
		 */
		virtual void setTaskResultAbort(void) = 0;

		/**
		 * Return the current progress text.
		 *
		 * @param None.
		 * @return The current progress text.
		 */
		wxString getProgressText(void) const {
			return progressText_;
		}

	protected:
		Task(Type);
		Type type_;

		/**
		 * Called by a derived class to indicate progress.
		 * This will send a task progress event and set the
		 * progress string to its parameter.
		 *
		 * @param msg The new progress text.
		 * @return None.
		 */
		void progress(wxString msg);
	private:
		/**
		 * Store abort request.
		 */
		bool abortRequest_;

		/**
		 * The current progress text.
		 */
		wxString	progressText_;
};

/**
 * A list of tasks.
 *
 * Maybe it is useful for someone...
 */
WX_DECLARE_LIST(Task, TaskList);

#endif	/* _TASK_H_ */

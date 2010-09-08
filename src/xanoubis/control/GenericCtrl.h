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

#ifndef _GENERICCTRL_H_
#define _GENERICCTRL_H_

#include <wx/event.h>

#include <set>
#include <vector>

#include "Task.h"

/**
 * Class to collet common things of different controllers.
 * In the main focus are controls dealing with tasks.
 */
class GenericCtrl : public wxEvtHandler
{
	public:
		/**
		 * Constructor.
		 * Just ensures clean lists.
		 * @param None.
		 */
		GenericCtrl(void);

		/**
		 * Destructor.
		 * Clean up lists.
		 * @param None.
		 */
		~GenericCtrl(void);

		/**
		 * Has errors.
		 * Use this method to test if a list of errors was accumulated.
		 * @param None.
		 * @return True if list of errors is not empty.
		 * @see getErrors()
		 * @see clearErrors()
		 * @see addError()
		 */
		bool hasErrors(void) const;

		/**
		 * Get errors.
		 * This method returns a list occured errors.
		 * @param None.
		 * @return List of errors.
		 * @see hasErrors()
		 * @see clearErrors()
		 * @see addError()
		 */
		const std::vector<wxString> getErrors(void) const;

		/**
		 * Clear list of errors.
		 * This will erase all elements from the list of errors. After
		 * a call of this method, hasErrors() will return false.
		 * @param None.
		 * @return Nothing.
		 * @see hasErrors()
		 * @see getErrors()
		 * @see addError()
		 */
		void clearErrors(void);

	protected:
		/**
		 * Add an error.
		 * Use this method to add a wxString with error description.
		 * @param[in] 1st The error description.
		 * @return True on success.
		 * @see hasErrors()
		 * @see getErrors()
		 * @see clearErrors()
		 */
		bool addError(const wxString &);

		/**
		 * Add task.
		 * Use this method to add a task to the list of tasks.
		 * @param[in] 1st The task to add.
		 * @return True on success.
		 * @see removeTask()
		 * @see isValidTask()
		 */
		bool addTask(Task *);

		/**
		 * Remove task.
		 * Use this method to remove a task from the list of tasks.
		 * @param[in] 1st The task to remove.
		 * @return True on success.
		 * @see addTask()
		 * @see isValidTask()
		 */
		bool removeTask(Task *);

		/**
		 * Is this a valid task?
		 * This method checks if the given task is valid. To be valid
		 * the task may not be NULL and is element of the task list
		 * (see addTask()).
		 * @param[in] 1st The task in question.
		 * @return True if task is valid.
		 * @see addTask()
		 * @see removeTask()
		 */
		bool isValidTask(Task *) const;

		/**
		 * Send event.
		 * This method will create and send an event of the given type.
		 * @param[in] 1st The type of the event.
		 * @return Nothing.
		 */
		void sendEvent(WXTYPE);

		/**
		 * Return true if the task list is currently empty.
		 *
		 * @param None.
		 * @return True if the task list is empty.
		 */
		bool taskListEmpty(void) const {
			return (taskList_.size() == 0);
		}

	private:
		/**
		 * Store multiple error descriptions.
		 */
		std::vector<wxString> errorList_;

		/**
		 * Store all our current running tasks.
		 */
		std::set<Task *> taskList_;
};

#endif	/* _GENERICCTRL_H_ */

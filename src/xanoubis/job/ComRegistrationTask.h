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

#ifndef _COMREGISTRATIONTASK_H_
#define _COMREGISTRATIONTASK_H_

#include "ComTask.h"

/**
 * A task to perform necessary steps to register GUI-communication with the
 * Anoubis-daemon.
 *
 * This task needs to be queued before any other communication is performed
 * with the Anoubis-daemon.
 */
class ComRegistrationTask : public ComTask
{
	public:
		/**
		 * Type of registration.
		 */
		enum Action
		{
			ACTION_REGISTER = 0, /*!< Registers at daemon */
			ACTION_UNREGISTER    /*!< Unregisters from daemon */
		};

		/**
		 * Creates the task.
		 *
		 * The action is set to ComRegistrationTask::ACTION_REGISTER.
		 */
		ComRegistrationTask(void);

		/**
		 * Creates a ComRegistrationTask with an already assigned
		 * action.
		 *
		 * @param action The registration action.
		 */
		ComRegistrationTask(Action);

		/**
		 * Returns the action of the task.
		 * @return Weather the action registers or unregisters from the
		 *         daemon.
		 */
		Action getAction(void) const;

		/**
		 * Updates the action of the task.
		 *
		 * This method meeds to be called <i>before</i> the task is
		 * executed.
		 *
		 * @param action The new action
		 */
		void setAction(Action);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

	private:
		Action	action_;

		/**
		 * exec() implements a state-machine.
		 * The following enumeration defines the states.
		 */
		enum RegState {
			STATE_REGISTER = 0,
			STATE_UNREGISTER,
			STATE_STAT_REGISTER,
			STATE_STAT_UNREGISTER,
			STATE_DONE
		};
};

#endif	/* _COMREGISTRATIONTASK_H_ */

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

#ifndef _COMTASK_H_
#define _COMTASK_H_

#include "Task.h"

class ComHandler;

/**
 * Base-class for communication-tasks.
 *
 * Any communication-related tasks should implement this class to get access to
 * an ComHandler.
 */
class ComTask : public Task
{
	public:
		/**
		 * A generic result-code of ComTask-implementations.
		 *
		 * A ComTask can have some generic states (independent from
		 * task-logic).
		 *
		 * - The task is initialized (not executed)
		 * - The task was successfully executed
		 * - The task could not obtain requested memory
		 * - A communication error occured
		 * - A local operation failed (e.g. some local calculation)
		 * - anoubisd could not perform the operation (nack-answer)
		 */
		enum ComTaskResult
		{
			RESULT_INIT = -1,
			RESULT_SUCCESS = 0,
			RESULT_OOM,
			RESULT_COM_ERROR,
			RESULT_LOCAL_ERROR,
			RESULT_REMOTE_ERROR
		};

		/**
		 * Returns the execution-state of the ComTask.
		 *
		 * This result shows weather the task was executed successfully
		 * nor not. Note, that derivated tasks might extend the
		 * ComTask-interface to return some more details.
		 *
		 * @return The execution-result of the task
		 */
		ComTaskResult getComTaskResult(void) const;

		/**
		 * Returns the ComHandler-instance assigned with the task.
		 *
		 * The derivated class have an access to message-handling with
		 * the Anoubis-daemon without having detail-knowledge about
		 * message-handling.
		 *
		 * @param The ComHandler-instance
		 */
		ComHandler *getComHandler(void) const;

		/**
		 * Updates the ComHandler-instance.
		 *
		 * This method is called from the JobCtrl background-thread.
		 * User of the task can ignore the method.
		 *
		 * @param comHandler The ComHandler-instance
		 */
		void setComHandler(ComHandler *);

	protected:
		/**
		 * Std-c'tor.
		 *
		 * The ComTaskResult-attribute is resetted
		 * (ComTask::resetComTaskResult()).
		 */
		ComTask(void);

		/**
		 * Updates the execution-result of the task.
		 *
		 * The exec()-method should call this method to update the
		 * result according to the task-logic.
		 *
		 * @param result The new result-code
		 */
		void setComTaskResult(ComTaskResult);

		/**
		 * Resets the result-code to its original state.
		 *
		 * The default-implementation sets the ComTaskResult-attribute
		 * to Comtask::RESULT_INIT.
		 *
		 * Derivated classes can overwrite the method to perform a
		 * task-specific reset-operation. Note: Do not forget to invoke
		 * the original implementation!
		 *
		 * The method should be invoked in the exec()-method!
		 */
		virtual void resetComTaskResult(void);

	private:
		ComTaskResult	result_;
		ComHandler	*comHandler_;
};

#endif	/* _COMTASK_H_ */
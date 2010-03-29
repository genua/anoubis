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
#include "PrivKey.h"

/**
 * Base-class for communication-tasks.
 *
 * Any communication-related tasks should implement this class.
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
		 * Returns a detailed result-code.
		 *
		 * This result-code highly depends on
		 * - on the concrete ComTask-implementation
		 * - getComTaskResult().
		 *
		 * This result-code can be used to provide more details about
		 * the execution-result of a ComTask. For example, it can
		 * return a concrete error-code, if an operation failed. Please
		 * check the documentation of the ComTask derivated classes for
		 * the concrete meaning of the error-code returned by this
		 * method.
		 *
		 * @return A detailed result-code
		 */
		int getResultDetails(void) const;

		/**
		 * Check if a task is completed. This will set the
		 * result and destroy anoubis_transaction structures
		 * associated with the com task. If the task is not
		 * completed this function must do enough steps to make
		 * sure that the task is once again waiting for a message
		 * from the communication channel. Usually this means
		 * that a transaction with the specified client is created.
		 *
		 * @param None.
		 * @return True if the task is done.
		 */
		virtual bool done(void) = 0;

		/**
		 * Set Client data structure in the ComTask.
		 */
		void setClient(struct anoubis_client *client);

		/**
		 * Set the task's result as appropriate after aborting
		 * the task.
		 */
		void setTaskResultAbort(void);

	protected:
		/**
		 * Default constructor.
		 */
		ComTask(void);

		/**
		 * Updates the execution-result of the task.
		 *
		 * The done()-method should call this method to update the
		 * result according to the task-logic.
		 *
		 * @param result The new result-code
		 */
		void setComTaskResult(ComTaskResult);

		/**
		 * Updates the detailed result-code.
		 *
		 * @param result The result-code
		 * @see getResultDetails()
		 */
		void setResultDetails(int);

		/**
		 * Get the Client data structure of the ComTask.
		 */
		struct anoubis_client	*getClient(void) const;

		/**
		 * Try to load the given private key if it is not yet loaded
		 * and return the raw key or NULL in case of an error.
		 *
		 * @param privkey The private key.
		 * @return The raw ky data or NULL in case of an error.
		 */
		struct anoubis_sig *comLoadPrivateKey(PrivKey *) const;

	private:
		ComTaskResult		 result_;
		int			 resultDetails_;
		struct anoubis_client	*client_;
};

#endif	/* _COMTASK_H_ */

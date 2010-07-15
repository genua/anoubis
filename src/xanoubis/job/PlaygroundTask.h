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

#ifndef _PLAYGROUNDTASK_H_
#define _PLAYGROUNDTASK_H_


#include "ComTask.h"

/**
 * Base-class for playground-related daemon-operations.
 *
 * This class implements exec() and done(). The protected attribute result_
 * contains the answer from the daemon. Derivated classes needs to extract the
 * information from the result-message.
 */
class PlaygroundTask : public ComTask
{
	public:
		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

		/**
		 * Implementation of Task::done().
		 */
		bool done(void);

	protected:
		/**
		 * Constructs the task.
		 *
		 * @param listtype The listtype you want to fetch.
		 *                 One of ANOUBIS_PGREC_*
		 * @param pgid The requested playground-id
		 *             (if used by the operation).
		 */
		PlaygroundTask(uint32_t, uint64_t);

		/**
		 * D'tor.
		 */
		~PlaygroundTask(void);

		/**
		 * The answer from the daemon.
		 * Set to NULL, of no answer is available.
		 */
		struct anoubis_msg *result_;

	private:
		/**
		 * List-type of request-message
		 */
		uint32_t listtype_;

		/**
		 * Playground-id of request-message.
		 */
		uint64_t pgid_;

		/**
		 * The transaction.
		 */
		struct anoubis_transaction *ta_;

		/**
		 * Resets the internal state of the task, so it can be reused.
		 */
		void reset(void);
};

#endif /* _PLAYGROUNDTASK_H_ */

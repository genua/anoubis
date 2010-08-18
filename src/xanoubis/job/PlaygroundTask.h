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

#include <anoubis_msg.h>
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
		virtual void exec(void);

		/**
		 * Implementation of Task::done().
		 */
		virtual bool done(void);

	protected:
		/**
		 * Constructs the task.
		 *
		 * @param listtype The listtype you want to fetch.
		 *                 One of ANOUBIS_PGREC_*
		 * @param pgid The requested playground-id
		 *             (if used by the operation).
		 */
		PlaygroundTask(uint32_t listtype, uint64_t pgid);

		/**
		 * D'tor.
		 */
		~PlaygroundTask(void);

		/**
		 * Playground-id of request-message.
		 * Derivated classes are able to modify the id.
		 */
		uint64_t pgid_;

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
		 * The transaction.
		 */
		struct anoubis_transaction *ta_;

		/**
		 * Resets the internal state of the task, so it can be reused.
		 */
		void reset(void);
};

/**
 * This template class adds a type independent record iterator to the
 * PlaygroudTask. The iterator support two functions: resetRecordIterator
 * and readNextRecord. After a successful call to readNextRecord a derived
 * class can access the raw record with getRecord. The derived class must
 * provide accessor functions for the current record.
 *
 * Use the iterator like this:
 *     task.resetRecordIterator();
 *     while(task.readNextRecord()) {
 *          [ Process current record. ]
 *     }
 */
template<typename T>
class PlaygroundIteratorTask : public PlaygroundTask
{
	private:
		/**
		 * Points to the current record while iterating.
		 */
		T			*record_;
		/**
		 * Points to the message that contains the current record.
		 */
		struct anoubis_msg	*message_;

		/**
		 * The index of next record in the current message.
		 */
		int			 number_;

		/**
		 * Offset of the next record in the current message's
		 * payload.
		 */
		int			 offset_;
	public:
		/**
		 * Constructor. Note that this does not reset the
		 * iterator.
		 */
		PlaygroundIteratorTask(uint32_t listtype, uint64_t pgid)
		    : PlaygroundTask(listtype, pgid) {
			message_ = NULL;
			record_ = NULL;
			number_ = 0;
			offset_ = 0;
		};
		/**
		 * Reset the itertor. This positions the iterator
		 * before the first record. The next call to readNextRecord
		 * will position the iterator at the first record.
		 *
		 * @param None.
		 * @return None.
		 */
		void resetRecordIterator(void) {
			message_ = result_;
			record_ = NULL;
			number_ = 0;
			offset_ = 0;
		};
		/**
		 * Load the next record and make it available via getRecord.
		 * This function returns false if there is no next record.
		 *
		 * @param None.
		 * @return True iff a valid record was loaded.
		 */
		bool readNextRecord(void) {
			if (message_ == NULL) {
				record_ = NULL;
				return false;
			}
			if (number_ >= get_value(message_->u.pgreply->nrec)) {
				message_ = message_->next;
				number_ = 0;
				offset_ = 0;
				return readNextRecord();
			}
			record_ = (T*)(message_->u.pgreply->payload + offset_);
			number_++;
			offset_ += get_value(record_->reclen);
			return true;
		};
	protected:
		/**
		 * Return a pointer to the current record.
		 *
		 * @param None.
		 * @return A pointer to the current record or NULL if
		 *     there is no current record.
		 */
		T *getRecord(void) const {
			return record_;
		};
};

#endif /* _PLAYGROUNDTASK_H_ */

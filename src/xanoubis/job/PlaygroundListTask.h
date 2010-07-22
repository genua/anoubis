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

#ifndef _PLAYGROUNDLISTTASK_H_
#define _PLAYGROUNDLISTTASK_H_

#include <anoubis_msg.h>

#include "PlaygroundTask.h"

/**
 * Task fetches a list with all available playgrounds from the daemon.
 *
 * The task implements an iterator, where you are able to iterate over all
 * playgrounds.
 *
 * setFirstRecords() resets the iterator. setNextRecords() returns true until
 * the end of the list is reached. So you can write something like this:
 *
 * <pre>
 * PlaygroundListTask task;
 *
 * while (task.setNextRecord()) {
 *     // Do something
 * }
 * </pre>
 */
class PlaygroundListTask : public PlaygroundTask
{
	public:
		/**
		 * Std-c'tor.
		 */
		PlaygroundListTask(void);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Initializes the iterator. The iterator is set to the first
		 * element. Use recordIsValid() to check if this it is valid.
		 */
		void setFirstRecord(void);

		/**
		 * Switches the iterator to the next record. setFirstRecord()
		 * must be called before calling this method().
		 *
		 * @return true, if a record is available, false if the end of
		 *         the list is reached.
		 */
		bool setNextRecord(void);

		/**
		 * Returns true if the current record is valid.
		 * @return true if the current record is valid,
		 *         false if the end of the list is reached
		 */
		bool recordIsValid(void) const;

		/**
		 * Returns the playground-id of the current playground-record.
		 * @return PGID of the current playground
		 */
		uint64_t getPGID(void) const;

		/**
		 * Returns the user-id of the current playground-record.
		 * @return Uid of current playground
		 */
		int getUID(void) const;

		/**
		 * Returns true, if the current playground-record is active.
		 * @return true, if the current playground is active.
		 */
		bool isActive(void) const;

		/**
		 * Returns the number of files of the current
		 * playground-record.
		 * @return Number of files of the current playground
		 */
		int getNumFiles(void) const;

		/**
		 * Returns the start-time of the current playground-record.
		 * @return Timestamp of current playground
		 */
		wxDateTime getTime(void) const;

		/**
		 * Returns the command of the current playground-record.
		 * @return Command executed by the current playground
		 */
		wxString getCommand(void) const;

	private:
		/**
		 * Iterator used for iterating through the list of playgrounds
		 * in the reply-message.
		 */
		iterator<Anoubis_PgInfoRecord> it_;
};

#endif /* _PLAYGROUNDLISTTASK_H_ */

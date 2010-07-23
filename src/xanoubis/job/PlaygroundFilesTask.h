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

#ifndef _PLAYGROUNDFILESTASK_H_
#define _PLAYGROUNDFILESTASK_H_

#include "PlaygroundTask.h"

/**
 * Task fetches a list with playground-files from the daemon. The task
 * inherits from PlaygroundIteratorTask, i.e. the iterator of the base
 * class can be used to iterate over all records.
 */
class PlaygroundFilesTask : public PlaygroundIteratorTask<Anoubis_PgFileRecord>
{
	public:
		/**
		 * Creates an task to fetch playground-files from the daemon.
		 * You need to provide the playground-id before sending the
		 * task!
		 * @see setRequestPGID()
		 */
		PlaygroundFilesTask(void);

		/**
		 * Creates a task to fetch a list of playground-files from the
		 * daemon.
		 * @param pgid The id of the requested playground
		 */
		PlaygroundFilesTask(uint64_t);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Returns the requested playground-id.
		 * Depending on the operation, this playground-id is send to
		 * the daemon.
		 * @return The requested PGID send to the daemon
		 */
		uint64_t getRequestedPGID(void) const;

		/**
		 * Updates the requested playground-id.
		 * Calling this method is useful when reusing the task for more
		 * than one request. Then you can update the playground-id send
		 * to the daemon.
		 * @param pgid The new playground-id send to the daemon
		 */
		void setRequestedPGID(uint64_t);

		/**
		 * Returns the playground-id of the current file-record.
		 * @return PGID of the current file
		 */
		uint64_t getPGID(void) const;

		/**
		 * Returns the device of the current file-record.
		 * @return Device-id of the current file
		 */
		uint64_t getDevice(void) const;

		/**
		 * Returns the inode of the current file-record.
		 * @return Inode of the current file
		 */
		uint64_t getInode(void) const;

		/**
		 * Returns the relative path of the current file-record.
		 * The path is relative to the mount-point of the device.
		 * @return Relative path of the current file
		 */
		wxString getPath(void) const;
};

#endif /* _PLAYGROUNDFILESTASK_H_ */

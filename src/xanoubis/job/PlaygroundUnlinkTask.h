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

#ifndef _PLAYGROUNDUNLINKTASK_H_
#define _PLAYGROUNDUNLINKTASK_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <set>

#include "PlaygroundFileEntry.h"
#include "Task.h"
#include "TaskEvent.h"

/**
 * Task to unlink list of (playground) files.
 *
 * You need to setup the file list (setPath()) before the tasks gets scheduled!
 */
class PlaygroundUnlinkTask : public Task
{
	public:
		/**
		 * Constructor.
		 */
		PlaygroundUnlinkTask(void);

		/**
		 * Add file to this task.
		 * @param[in] 1st The PlaygroundFileEntry to add.
		 * @return Nothing.
		 */
		void addFile(PlaygroundFileEntry *);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

		/**
		 * Returns the state of the latest unlink.
		 * @param None.
		 * @return 0 on success, an errno in case of error.
		 */
		int getResult(void) const;

	private:
		/**
		 * Resets the task.
		 * This means cleaning the fileList_ and setting the result
		 * to EINVAL.
		 * @param None.
		 * @return Notinig.
		 */
		void reset(void);

		/**
		 * Store latest unlink result.
		 */
		int result_;

		/**
		 * These files will be deleted.
		 */
		std::set<PlaygroundFileEntry *> fileList_;
};

#endif	/* _PLAYGROUNDUNLINKTASK_H_ */

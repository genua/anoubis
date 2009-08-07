/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#ifndef _COMUPGRADELISTGETTASK_H_
#define _COMUPGRADELISTGETTASK_H_

#include <queue>
#include "ComTask.h"
#include "TaskEvent.h"

/**
 * Task to retrieve the upgrade-list from anoubisd.
 *
 * This is just a dummy implementation. The protocol to transport the
 * information from the daemon to the gui is not settled yet.
 * Thus this class does provide a fake list of files to enable
 * development of the other gui components. In addition the interface
 * of this class is not settled, too.
 */
class ComUpgradeListGetTask : public ComTask
{
	public:
		/**
		 * Constructor
		 * You have to set parameters manually with
		 * setRequestParameter().
		 * @param None
		 * @see setRequestParameter()
		 */
		ComUpgradeListGetTask(void);

		/**
		 * Constructor with parameters.
		 * @param[in] 1st UserId of this request.
		 */
		ComUpgradeListGetTask(uid_t);

		/**
		 * Destructor
		 */
		~ComUpgradeListGetTask(void);

		/**
		 * Get user id.
		 * Returns the user id.
		 * @param None.
		 * @return User id.
		 */
		uid_t getUid(void) const;

		/**
		 * Updates the input-parameter of the task.
		 * This method needs to be called <i>before</i> the task is
		 * executed!
		 * @param[in] 1st The user id for whom we perform
		 *            this request.
		 * @return Nothing.
		 * @see getUid()
		 */
		void setRequestParameter(uid_t);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

		/**
		 * Implementation of Task::done().
		 */
		bool done(void);

		/**
		 * Returns a list of filenames, which have been upgraded.
		 * After successful completition of the task you can receive
		 * the result of the task.
		 * @return List of filenames, which have been upgraded.
		 */
		wxArrayString getFileList(void) const;

	protected:
		/**
		 * @see ComTask::resetComTaskResult()
		 */
		void resetComTaskResult(void);

	private:
		/**
		 * Store the UserId of this request.
		 */
		uid_t uid_;

		/**
		 * Compile the list of files.
		 */
		wxArrayString fileList_;
};

#endif	/* _COMUPGRADELISTGETTASK_H_ */

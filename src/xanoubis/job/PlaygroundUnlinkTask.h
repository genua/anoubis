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

#include <anoubis_msg.h>
#include <set>
#include <map>

#include "PlaygroundFileEntry.h"
#include "Task.h"
#include "ComTask.h"
#include "TaskEvent.h"

typedef std::pair<uint64_t, uint64_t>	devInodePair;

/**
 * Task to unlink list of (playground) files.
 *
 * You need to setup the file list (setPath()) before the tasks gets scheduled!
 */
class PlaygroundUnlinkTask : public ComTask
{
	public:
		/**
		 * Constructor whole playground.
		 * If this constructor is used all files of the given playground
		 * will be removed.
		 * @param[in] 1st Id of playground in question.
		 */
		PlaygroundUnlinkTask(uint64_t);

		/**
		 * Destructor.
		 */
		~PlaygroundUnlinkTask(void);

		/**
		 * Add match list.
		 * Only file in match list will be unlinked.
		 * @param[in] 1st List of file entries.
		 * @return Nothing.
		 * @see addFile()
		 */
		void addMatchList(std::vector<PlaygroundFileEntry *> &);

		/**
		 * Add file to this task.
		 * This will add a single file to the list of files this
		 * task will unlink.
		 * @param[in] 1st The PlaygroundFileEntry to add.
		 * @return True on success.
		 * @see addMatchList()
		 */
		bool addFile(PlaygroundFileEntry *);

		/**
		 * Implementation of Task::getEventType().
		 */
		wxEventType getEventType(void) const;

		/**
		 * Implementation of Task::exec().
		 */
		void exec(void);

		/**
		 * Implementation of ComTask::done().
		 */
		bool done(void);

		/**
		 * Get playground id of this task.
		 * @param None.
		 * @return Playground id.
		 */
		uint64_t getPgId(void) const;

		/**
		 * Has a match list of files to unlink?
		 * @param None.
		 * @return True if list was set.
		 * @see addMatchList()
		 * @see addFile()
		 */
		bool hasMatchList(void) const;

		/**
		 * Has a list of errors?
		 * @param None.
		 * @return True if list of errors is not empty.
		 * @see getErrorList()
		 */
		bool hasErrorList(void) const;

		/**
		 * Get error list.
		 * This method returns a map of device inode pair to error code.
		 * @param None.
		 * @return List of errors.
		 * @see hasErrorList()
		 */
		std::map<devInodePair, int> & getErrorList(void);

	private:
		/**
		 * These states this task traverse.
		 */
		enum State {
			/**
			 * Update playground information.
			 */
			INFO,

			/**
			 * Fetch file list.
			 */
			FILEFETCH,
		};

		/**
		 * Resets the task.
		 * This means cleaning the unlinkList_ and setting the result
		 * to EINVAL.
		 * @param None.
		 * @return Notinig.
		 */
		void reset(void);

		/**
		 * Execute communication part of task.
		 * @param None.
		 * @return Nothing.
		 */
		void execCom(void);

		/**
		 * Execute filesystem part of task.
		 * @param None.
		 * @return Nothing.
		 */
		void execFs(void);

		/**
		 * Extract files from mesage (list of files) and filles
		 * unlinkList_ with wxString.
		 * @param[in] 1st The message of current files.
		 */
		void extractFileList(struct anoubis_msg *);

		/**
		 * Do unlink loop
		 * @param None.
		 * @return The number of unlinked items.
		 */
		int unlinkLoop(void);

		/**
		 * Is playground still active.
		 * @param[in] 1st The current list of playgrounds from daemon.
		 * @return True if playground is active (aka # of procs != 0).
		 */
		bool isPlaygroundActive(struct anoubis_msg *);

		/**
		 * Clean the unlink list. This method iterates over the
		 * given list and deletes every element until the list
		 * is empty.
		 * @param None.
		 * @return Nothing.
		 */
		void cleanUnlinkList(void);

		/**
		 * Store id of playground.
		 */
		uint64_t pgId_;

		/**
		 * The current state.
		 */
		enum State state_;

		/**
		 * The current transaction.
		 */
		struct anoubis_transaction *transaction_;

		/**
		 * Do we have to consider the matchList?
		 */
		bool considerMatchList_;

		/**
		 * These files will be deleted.
		 */
		std::map<char *, devInodePair> unlinkList_;

		/**
		 * Mask list: only unlink these files.
		 */
		std::map<devInodePair, bool> matchList_;

		/**
		 * Counts the delete attempts without any progress.
		 */
		int no_progress_;

		/**
		 * These files couldn't be removed.
		 */
		std::map<devInodePair, int> errorList_;
};

#endif	/* _PLAYGROUNDUNLINKTASK_H_ */

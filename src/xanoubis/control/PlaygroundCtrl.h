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

#ifndef _PLAYGROUNDCTRL_H_
#define _PLAYGROUNDCTRL_H_

#include <set>
#include <wx/arrstr.h>

#include "AnGenericRowProvider.h"
#include "PlaygroundListTask.h"
#include "PlaygroundFilesTask.h"
#include "Singleton.h"
#include "Task.h"
#include "TaskEvent.h"

/**
 * Controller class to do all aspects about playgrounds.
 */
class PlaygroundCtrl : public Singleton<PlaygroundCtrl> , public wxEvtHandler
{
	public:
		/**
		 * Destructor.
		 */
		~PlaygroundCtrl(void);

		/**
		 * Get info provider.
		 * This method will return the RowProvider of the list of
		 * playgrounds. The caller must not delete this object.
		 * @param None.
		 * @return Playground list row provider.
		 */
		AnRowProvider *getInfoProvider(void);

		/**
		 * Command: Update the list of playground infos.
		 * This method will trigger all steps to update the list of
		 * playgrounds. The caller may also want to register to
		 * anEVT_PLAYGROUND_ERROR events to know about errors.
		 * @param None.
		 * @return True on success.
		 */
		bool updatePlaygroundInfo(void);

		/**
		 * Get file provider.
		 * This method will return the RowProvider of the list of
		 * files within one playground. The caller must not delete
		 * this object.
		 * @param None.
		 * @return row provider for playground files
		 */
		AnRowProvider * getFileProvider(void);

		/**
		 * Command: Update the list of playground files.
		 * This method will request an update of the playground
		 * filelist with the files from the specified playground.
		 * Errors are reported through anEVT_PLAYGROUND_ERROR events.
		 * @param 1st  the ID of the playground to read.
		 * @param 2nd True if a non-existing playground should
		 *     be reported as an error.
		 * @return True on success.
		 */
		bool updatePlaygroundFiles(uint64_t pgid, bool err = true);

		/**
		 * Command: Remove a playground.
		 * This method will start a task that deletes all files from
		 * the selected playground. Once the task is finished an event
		 * is sent.
		 * @param 1st  The ID of the playground to remove.
		 * @return True on success.
		 */
		bool removePlayground(uint64_t pgid);

		/**
		 * Returns a list of errors, which occured during the execution
		 * of the last command. The errors are valid until the next
		 * command is started.
		 * @param None
		 * @return Errors collected during execution of the last
		 *         command.
		 */
		const wxArrayString &getErrors(void) const;

		/**
		 * Cleaer list of errors.
		 * @param None.
		 * @return Nothing.
		 */
		void clearErrors(void);

		/**
		 * Command: Commit a set of files.
		 * Try to commit a set of files by creating an appropriate
		 * task. Once the task is finished an event is sent.
		 *
		 * @param[in] 1st The list of file to commit. This
		 *     is a list of indexes into the file list maintained
		 *     by the row provider.
		 * @return True on success.
		 */
		bool commitFiles(const std::vector<int> &files);

	protected:
		/**
		 * Constructor.
		 * You can't call it from anywhere. Use instance().
		 */
		PlaygroundCtrl(void);

	private:
		/**
		 * List of playground infos.
		 */
		AnGenericRowProvider playgroundInfo_;

		/**
		 * List of files in the previously requested playground.
		 */
		AnGenericRowProvider playgroundFiles_;

		/**
		 * List of errors.
		 */
		wxArrayString errorList_;

		/**
		 * Event handler for completed PlaygroundListTasks.
		 * @param[in] 1st The event for PlaygroundListTask in question.
		 * @return Nothing.
		 */
		void OnPlaygroundListArrived(TaskEvent &);

		/**
		 * Event handler for completed PlaygroundFileTasks.
		 * @param[in] 1st The event for PlaygroundFileTask in question.
		 * @return Nothing.
		 */
		void OnPlaygroundFilesArrived(TaskEvent &);

		/**
		 * Event handler for complted PlaygroundCommitTasks.
		 *
		 * @param[in] 1st The event for the playground commit task
		 *     in question.
		 * @return Nothing.
		 */
		void OnPlaygroundFilesCommitted(TaskEvent &);

		/**
		 * Create Playground list task.
		 * This method creates and starts the task to fetch the
		 * playground list.
		 * @param None.
		 * @return True on success.
		 */
		bool createListTask(void);

		/**
		 * Create Playground files task.
		 * This method creates and starts the task ot fetch the
		 * playground filelist.
		 * @param 1st Playground ID for which to get files.
		 * @param 2nd True if a non-existing playground should be
		 *     reported as an error.
		 * @return True on success.
		 */
		bool createFileTask(uint64_t pgid, bool = true);

		/**
		 * Extract list task.
		 * This method clears the playground info list and extracts the
		 * data of a given PlaygroundListTask. Those data are stored in
		 * a corresponding entry object which is stored in the list of
		 * playgrounds.
		 * @param[in] 1st The completed PlaygroundListTask.
		 * @return Nothing.
		 */
		void extractListTask(PlaygroundListTask *);

		/**
		 * Extract file task.
		 * This method clears the playground file list and extracts the
		 * data of the given PlaygroundFileTask. The data is processed
		 * and stored in the list of playground files.
		 * @param[in] 1st The completed PlaygroundFiletask.
		 * @return Nothing.
		 */
		void extractFilesTask(PlaygroundFilesTask *);

		/**
		 * Removes all entries from playgroundInfo_ and destroys the
		 * instances.
		 */
		void clearPlaygroundInfo(void);

		/**
		 * Removes all entries from playgrundFiles_ and destroys the
		 * instances.
		 */
		void clearPlaygroundFiles(void);

		/**
		 * Send error event.
		 * This method will send a anEVT_PLAYGROUND_ERROR event,
		 * to inform all interested parties about an occured error.
		 * @param None.
		 * @return Nothing.
		 */
		void sendErrorEvent(void);

		/**
		 * Send a completed event.
		 * This method will send an anEVT_PLAYGROUND_COMPLETED event,
		 * to inform all interested paties about a playground
		 * operation that completed (successfully or not).
		 * @param None.
		 * @return Nothing.
		 */
		void sendCompletedEvent(void);

		/**
		 * Handle errors of a ComTask by sending an appropriate
		 * error event.
		 *
		 * @param task The task.
		 * @return None.
		 */
		void handleComTaskResult(ComTask *task);

	friend class Singleton<PlaygroundCtrl>;
};

#endif	/* _PLAYGROUNDCTRL_H_ */

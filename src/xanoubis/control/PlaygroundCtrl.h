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
#include "Singleton.h"
#include "Task.h"

/**
 * Controller class to do all aspects about playgrounds.
 */
class PlaygroundCtrl : public Singleton<PlaygroundCtrl> , private wxEvtHandler
{
	public:
		/**
		 * Destructor.
		 */
		~PlaygroundCtrl(void);

		/**
		 * Get info provider.
		 * This method will return the RowProvider of the list of
		 * playgrounds. The caller may not delete this object.
		 * @param None.
		 * @return Playground list row provider.
		 */
		const AnRowProvider * getInfoProvider(void) const;

		/**
		 * Command: updatePlaygroundList
		 * This method will trigger all steps to update the list of
		 * playgrounds. The caller may also want to register to
		 * anEVT_PLAYGROUND_ERROR events to know about errors.
		 * @param None.
		 * @return True on success.
		 */
		bool updatePlaygroundList(void);

		/**
		 * Returns a list of errors, which occured during the execution
		 * of the last command.
		 * Please not, that the error-list is resetted, if you start
		 * another command!
		 * @return Errors collected during execution of the last
		 *         command.
		 */
		const wxArrayString &getErrors(void) const;

	protected:
		/**
		 * Constructor.
		 * You can't call it from anywhere. Use instance().
		 */
		PlaygroundCtrl(void);

	private:
		/**
		 * List of playgrounds.
		 */
		AnGenericRowProvider playgroundList_;

		/**
		 * List of tasks.
		 */
		std::set<Task *> taskList_;

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
		 * Create Playground list task.
		 * This method creates and starts the task to fetch the
		 * playground list.
		 * @param None.
		 * @return True on success.
		 */
		bool createListTask(void);

		/**
		 * Extract list task.
		 * This method clears the playground list and extracts the data
		 * of a given PlaygroudnListTask. Those data are stored in a
		 * corresponding entry object which is stored in the list of
		 * playgrounds.
		 * @param[in] 1st The completed PlaygroundListTask.
		 * @return Nothing.
		 */
		void extractListTask(PlaygroundListTask *);

		/**
		 * Removes all entries from playgrundList_ and destroys the
		 * instances.
		 */
		void clearPlaygroundList(void);

		/**
		 * Send error event.
		 * This method will send a anEVT_PLAYGROUND_ERROR event,
		 * to inform all interested about an occured error.
		 * @param None.
		 * @return Nothing.
		 */
		void sendErrorEvent(void);

	friend class Singleton<PlaygroundCtrl>;
};

#endif	/* _PLAYGROUNDCTRL_H_ */

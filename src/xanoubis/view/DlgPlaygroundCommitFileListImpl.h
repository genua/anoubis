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

#ifndef _DLGPLAYGROUNDCOMMITFILELISTIMPL_H_
#define _DLGPLAYGROUNDCOMMITFILELISTIMPL_H_

#include "AnListCtrl.h"
#include "ModPlaygroundPanelsBase.h"
#include "TaskEvent.h"

class PlaygroundInfoEntry;

class DlgPlaygroundCommitFileListImpl : public DlgPlaygroundCommitFileListBase
{
	public:
		/**
		 * Constructor
		 * @param[in] Playground information for the displayed files
		 */
		DlgPlaygroundCommitFileListImpl(PlaygroundInfoEntry *);

		/**
		 * Destructor.
		 */
		~DlgPlaygroundCommitFileListImpl(void);

	protected:
		/**
		 * Handle update of playground list column header options chosen
		 * by the user.
		 * @param None.
		 * @return
		 * Nothing.
		 */
		virtual void onColumnButtonClick(wxCommandEvent &);

		/**
		 * Handle a click on the commit button in the file commit
		 * dialog.
		 *
		 * @param[in] 1st The click event.
		 * @return Nothing.
		 */
		void onCommitClicked(wxCommandEvent &);

		/**
		 * Handle a click on the delete button in the file commit
		 * dialog.
		 *
		 * @param[in] 1st The click event.
		 * @return Nothing.
		 */
		void onDeleteClicked(wxCommandEvent &);

		/**
		 * Handle a clock on the close button in the file commit
		 * dialog.
		 *
		 * @param[in] 1st The click event.
		 * @return Nothing.
		 */
		void onCloseClicked(wxCommandEvent &);

		/**
		 * Handle error events of playground task's.
		 *
		 * @param event The error event.
		 * @return None.
		 */
		void onPlaygroundError(wxCommandEvent &event);

		/**
		 * Handle completion events of playground task's.
		 *
		 * @param event The completion event.
		 * @return None.
		 */
		void onPlaygroundCompleted(wxCommandEvent &event);

		/**
		 * Called when the user clicks on one of the list column
		 * headers. This is used to sort the list entries based
		 * on this column.
		 *
		 * @param event The list event.
		 */
		void onCommitListColClick(wxListEvent &event);

		/**
		 * Called when the focus leave the search text box
		 * in the playground commit window.
		 *
		 * @param event The focus event.
		 */
		void onCommitSearchKillFocus(wxFocusEvent &event);

		/**
		 * Called when the user presses enter in the search text
		 * box in the playground commit window.
		 *
		 * @param event The event.
		 */
		void onCommitSearchEnter(wxCommandEvent &event);

		/**
		 * Begin activity.
		 * Things we want to do wenn an activity has begun. This is:
		 *	- disable buttons
		 *	- show busy cursor
		 * @param None.
		 * @return Nothing.
		 */
		void beginActivity(void);

		/**
		 * Handle progress reports from a task.
		 *
		 * @param event The task event.
		 * @return None.
		 */
		void onTaskProgress(TaskEvent &event);
};

#endif	/* _DLGPLAYGROUNDCOMMITFILELISTIMPL_H_ */

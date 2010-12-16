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

#ifndef _MODPLAYGROUNDMAINPANELIMPL_H_
#define _MODPLAYGROUNDMAINPANELIMPL_H_

#include "AnEvents.h"

#include "ModPlaygroundPanelsBase.h"

class ModPlaygroundMainPanelImpl : public ModPlaygroundMainPanelBase
{
	public:
		/**
		 * Constructor of ModPlaygroundMainPanelImpl.
		 * @param[in] 1st The parent window and ID.
		 */
		ModPlaygroundMainPanelImpl(wxWindow*, wxWindowID);

		/**
		 * Destructor of ModPlaygroundMainPanelImpl.
		 * @param None.
		 */
		~ModPlaygroundMainPanelImpl(void);

		/**
		 * Updates the main panel.
		 * @param None.
		 * @return Nothing.
		 */
		void update(void);

	private:
		/**
		 * Handle the event, if the status of the daemon-connection has
		 * changed.
		 * Used to know, if you are connected to the daemon. This
		 * information is used to enable/disable buttons the view.
		 * @param[in] 1st The event.
		 */
		virtual void onConnectionStateChange(wxCommandEvent &);
		bool comEnabled_;

		/**
		 * Handles keystrokes-events of AppComboBox.
		 *
		 * This is used to detect whether a path was entered. Then you
		 * are able to enable/disable the start-playground-button.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAppPathEntered(wxCommandEvent &);

		/**
		 * Handle <Return> events of AppComboBox.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAppStartEnter(wxCommandEvent &);

		/**
		 * Handle onClick events to 'Start Playground' button.
		 * @param[in] 1st The click event.
		 * @return Nothing.
		 */
		virtual void onAppStart(wxCommandEvent &);

		/**
		 * Handle error event form PlaygroundCtrl.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onPlaygroundError(wxCommandEvent &);

		/**
		 * Handle onClick events to 'Refresh View' button.
		 * @param[in] 1st The click event.
		 * @return Nothing.
		 */
		void onPgListRefreshClicked(wxCommandEvent &);

		/**
		 * Handle onPgNotebookChanging events to 'Playground Overview'.
		 * @param[in] 1st The focus event.
		 * @return Nothing.
		 */
		void onPgNotebookChanging(wxCommandEvent &);

		/**
		 * Handle onClick events to 'Commit files...' button.
		 * @param[in] 1st The click event.
		 * @return Nothing.
		 */
		virtual void onCommitFiles(wxCommandEvent &);

		/**
		 * Handle onClick events to 'Delete' button.
		 * @param[in] 1st The click event.
		 * @return Nothing.
		 */
		virtual void onDeleteFiles(wxCommandEvent &);

		/**
		 * Handle deselection of a playground.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onPgListItemSelect(wxListEvent &);

		/**
		 * Handle deselection of a playground.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onPgListItemDeselect(wxListEvent &);

		/**
		 * Handle onActivate events (aka double-click or Enter)
		 * on list item.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onPgListItemActivate(wxListEvent &);

		/**
		 * Start application.
		 * This method will take the input from combo box and tries
		 * to start it. On succes the history (combo box) is updated.
		 * On error a dialog is shown.
		 * @param None.
		 * @return Nothing.
		 */
		void startApplication(void);

		/**
		 * Update the enabled state of the delete and commit
		 * buttons.
		 *
		 * @param None.
		 * @return None.
		 */
		void updateButtonState(void);

		/**
		 * Refreshes the grid of playgrounds and handles correspondig
		 * errors.
		 * @param None.
		 * @return Nothing.
		 */
		void refreshPlaygroundList(void);

		/**
		 * Open file commit dialog.
		 * @param None.
		 * @return Nothing.
		 */
		void openCommitDialog(void);

		/**
		 * Convert string to argv array.
		 * This method creates tokens by splitting the given string
		 * at whitespace characters. These tokens are stored in an array
		 * with number of tokens + 1. The last element is NULL for
		 * termination.
		 * @note It is the responibility of the caller to free(3) the
		 *	allocated memory of argv.
		 * @param[in] 1st The string to tokenize.
		 * @return A pointer to argv or NULL in case of error.
		 */
		char **convertStringToArgV(const wxString &) const;

	protected:
		/**
		 * Refresh the list of playgrounds if significant playground
		 * information changed.
		 *
		 * @param event The event.
		 * @return None.
		 */
		void onPgChange(wxCommandEvent &event);
};


#endif	/* _MODPLAYGROUNDMAINPANELIMPL_H_ */

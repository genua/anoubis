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

#ifndef __ModSfsMainPanelImpl__
#define __ModSfsMainPanelImpl__

#include "ModSfsPanelsBase.h"
#include "Observer.h"
#include "SfsCtrl.h"

class Policy;

class ModSfsMainPanelImpl : public ModSfsMainPanelBase, private Observer
{

	public:
		/**
		 * Constructor of the ModSfs main view.
		 * @param[in] 1st The parent window instance.
		 * @param[in] 2nd The window id of the Module.
		 */
		ModSfsMainPanelImpl(wxWindow*, wxWindowID);

		/**
		 * Destructor of ModSfs main view.
		 * @param None.
		 */
		~ModSfsMainPanelImpl(void);

		/**
		 * This is called when an observed policy was modified.
		 * @param[in] 1st The changed policy (aka subject).
		 * @return Nothing.
		 */
		virtual void update(Subject *);

		/**
		 * This is called when an observed policy is about to
		 * be destroyed.
		 * @param[in] 1st The changed policy (aka subject).
		 * @return Nothing.
		 */
		virtual void updateDelete(Subject *);

	private:
		SfsCtrl		*sfsCtrl_;

		bool comEnabled_; /**< Current communicator status. */

		/**
		 * Check if the private key and the certificate match.
		 * Return an appropriate error message.
		 *
		 * @param None.
		 * @return The error message. wxEmptyString if everything
		 *     is ok.
		 */
		wxString compareKeyPair(void);

		/**
		 * Handle the event when the tab of the Sfs notebook is changed.
		 * Checks if the keys are loaded before changing to the keysTab.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onSfsTabChange(wxNotebookEvent&);

		/**
		 * Check the configuration of private and public key for the
		 * following problems:
		 *   - missing private key
		 *   - missing certificate
		 *   - key mismatch
		 * If a problem occurs, display a warning message at the bottom
		 * of the windows.
		 * @param None.
		 * @return Nothing.
		 */
		void checkKeyConfiguration(void);

		/**
		 * Handle the event when main directory of Sfs tree is changed.
		 * Sets show all and updates the view.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsMainDirCtrlSelChanged(wxTreeEvent&);

		/**
		 * If double click (left) on cell occurs, jump to
		 * corresponding rule in RuleEditor
		 * @param[in] 1st The Event
		 * @return Nothing
		 */
		void OnGridCellLeftDClick(wxGridEvent&);

		/**
		 * Handle the event when main directory of Sfs is changed.
		 * Sets show all and updates the tree
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsPathChanged(wxCommandEvent &);

		/**
		 * Handle the event when a row is selected in the SfsList.
		 * Enable/disables buttons of the view accordingly.
		 * @param[in] 1st The event.
		 */
		void OnSfsListSelected(wxListEvent &);

		/**
		 * Handle the event when a row is deselected in the SfsList.
		 * Enable/disable buttons of the view accordingly.
		 * @param[in] 1st The event.
		 */
		void OnSfsListDeselected(wxListEvent &);

		/**
		 * Handle the event, if the status of the daemon-connection has
		 * changed.
		 * Used to know, if you are connected to the daemon. This
		 * information is used to enable/disable buttons the view.
		 * @param[in] 1st The event.
		 */
		void OnConnectionStateChange(wxCommandEvent &);

		/**
		 * Handle the event when Sfs operation finished.
		 * Updates the view.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsOperationFinished(wxCommandEvent&);

		/**
		 * Handle the event that directory is independently changed
		 * from OnSfsMainDirCtrlSelChanged.
		 * Updates the view.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsDirChanged(wxCommandEvent&);

		/**
		 * Handle the event that directory traversal changed state.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsMainDirTraversalChecked(wxCommandEvent&);

		/**
		 * Handle the event when an Sfs error occurs.
		 * Rises message box with the correponding error.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsError(wxCommandEvent&);

		/**
		 * Handle the event when the Sfs main filter is changed.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsMainFilterButtonClicked(wxCommandEvent&);

		/**
		 * Handle the event when the Sfs filter changed (inverse).
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsMainInverseCheckboxClicked(wxCommandEvent&);

		/**
		 * Handle the event when the Sfs validate button is clicked.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsMainValidateButtonClicked(wxCommandEvent&);

		/**
		 * Handle the event when the Sfs apply button is clicked.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsMainApplyButtonClicked(wxCommandEvent&);

		/**
		 * Triggered, if the [Import] button was clicked.
		 *
		 * Imports checksums from a file.
		 */
		void OnSfsMainImportClicked(wxCommandEvent&);

		/**
		 * Triggered, if the [Export] button was clicked.
		 *
		 * Exports checksums of all selected rows.
		 */
		void OnSfsMainExportClicked(wxCommandEvent&);

		void handleSfsCommandResult(SfsCtrl::CommandResult);

		/**
		 * Handle the event when the Sfs sign is enabled.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsMainSigEnabledClicked(wxCommandEvent&);

		/**
		 * Handle the event when a view choice is selected.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsMainDirViewChoiceSelected(wxCommandEvent &);

		/**
		 * Handle the event when Sfs key is loaded.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsMainKeyLoaded(wxCommandEvent&);

		/**
		 * Handle the event when private validity changed.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onPrivKeyValidityChanged(wxCommandEvent&);

		/**
		 * Handle the event when private key validity period changed.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onPrivKeyValidityPeriodChanged(wxSpinEvent&);

		/**
		 * Handle the event when key-pair generation was requested.
		 * Opens the modal Generate-Keypair-Dialogue.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onGenerateKeyPairButton(wxCommandEvent&);

		/**
		 * Initial setup of the Sfs module.
		 * Connects events to the corresponding handlers.
		 * @param None.
		 * @return Nothing.
		 */
		void initSfsMain(void);

		/**
		 * Deletes the Sfs control instance.
		 * @param None.
		 * @return Nothing.
		 */
		void destroySfsMain(void);

		/**
		 * Enables/disables buttons the Sfs browser.
		 *
		 * The decision, if the buttons needs to be enabled or
		 * disabled, is based on various dependencies:
		 *
		 * - If the daemon-connection is not established, no
		 *   daemon-related operation should be possible.
		 * - Operations, which are applied on the current selection
		 *   of the SfsList, should be disabled, if nothing is
		 *   selected.
		 * - The argument specifies, if a Sfs-operation is currently
		 *   running in the background. If set, no other operation
		 *   should be possible in parallel.
		 *
		 * @param sfsOpRunning Flag specifies, whether a sfs-operation
		 *                     is currently running.
		 */
		void enableSfsControls(bool);

		/**
		 * Reads the Sfs options from user configuration file.
		 * @param None.
		 * @return Nothing.
		 */
		void initSfsOptions(void);

		/**
		 * Stores the Sfs options to user configuration file.
		 * @param None.
		 * @return Nothing.
		 */
		void saveSfsOptions(void);

		/**
		 * Update the private key parameters.
		 * @param[in] 1st The path to the private key file.
		 * @param[in] 2rd Time period for validity. Zero means
		 *     till end of session.
		 */
		void privKeyParamsUpdate(const wxString &, int);

		/**
		 * Update the certificate parameters.
		 * @param[in] 1st The path to the certificate file.
		 * @return true if the certificate has changed
		 */
		bool certificateParamsUpdate(const wxString &);

		/**
		 * Handle the event on show SFS browser.
		 * This just receives the event and selects the SFS browser tab
		 * and selects the 'show upgraded' button.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onSfsBrowserShow(wxCommandEvent&);
};

#endif /* __ModSfsMainPanelImpl__ */

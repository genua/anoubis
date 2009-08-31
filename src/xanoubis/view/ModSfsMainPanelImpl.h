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

#include "AnEvents.h"
#include "Observer.h"

#include "DefaultFilterPolicy.h"
#include "ModSfsPanelsBase.h"
#include "SfsAppPolicy.h"
#include "SfsCtrl.h"
#include "SfsDefaultFilterPolicy.h"
#include "SfsFilterPolicy.h"

class IndexArray;
class SfsCtrl;
class TaskEvent;

class ModSfsMainPanelImpl : public Observer, public ModSfsMainPanelBase
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
		 * Adds a Sfs default filter policy.
		 * This should be used by ModSfsAddPolicyVisitor only.
		 * @param[in] 1st Concerning Sfs default filter policy.
		 * @return Nothing.
		 */
		void addSfsDefaultFilterPolicy(SfsDefaultFilterPolicy*);

		/**
		 * Adds a Sfs filter policy.
		 * This should be used by ModSfsAddPolicyVisitor only.
		 * @param[in] 1st Concerning Sfs filter policy.
		 * @return Nothing.
		 */
		void addSfsFilterPolicy(SfsFilterPolicy*);

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
		/**
		 * The columns used within ModSfs
		 */
		enum modSfsListColumns {
			COLUMN_PATH = 0,	/**< path to binary. */
			COLUMN_SUB,		/**< the subject. */
			COLUMN_VA,		/**< valid action. */
			COLUMN_IA,		/**< invalid action. */
			COLUMN_UA,		/**< unknown action. */
			COLUMN_SCOPE,		/**< the scope. */
			COLUMN_USER,		/**< the user. */
			COLUMN_EOL
		};

		wxString columnNames_[COLUMN_EOL]; /** < the header line. */

		long userRuleSetId_;	/**< Id of our ruleSet. */
		long adminRuleSetId_;	/**< Id of our admin ruleSet. */

		SfsCtrl		*sfsCtrl_;

		bool comEnabled_; /**< Current communicator status. */

		/**
		 * Handle the event when main directory of Sfs tree is changed.
		 * Sets show all and updates the view.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsMainDirCtrlSelChanged(wxTreeEvent&);

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
		void OnDaemonRegistration(TaskEvent &);

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
		 * Handle the event that Sfs entry is changed.
		 * Updates the view.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnSfsEntryChanged(wxCommandEvent&);

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
		void OnPrivKeyValidityChanged(wxCommandEvent&);

		/**
		 * Handle the event when private key validity period changed.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void OnPrivKeyValidityPeriodChanged(wxSpinEvent&);

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
		 * @param[in] 2nd Boolean which states validity til session end.
		 * @param[in] 3rd Time period for validity.
		 */
		void privKeyParamsUpdate(const wxString &, bool, int);

		/**
		 * Update the certificate parameters.
		 * @param[in] 1st The path to the certificate file.
		 * return Nothing.
		 */
		void certificateParamsUpdate(const wxString &);

		/**
		 * Find the row of a given policy.
		 * @param[in] 1st The policy to search for.
		 * @return The index of found row or -1.
		 */
		long findListRow(Policy*);

		/**
		 * Remove a given row from the list.
		 * @param[in] 1st The index of the row in question.
		 * @return Nothing.
		 */
		void removeListRow(long);

		/**
		 * Handle the event on loading RuleSet.
		 * This just receives the event and updates the id's of
		 * user and admin ruleset.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onLoadRuleSet(wxCommandEvent&);

		/**
		 * Handle the event on show SFS browser.
		 * This just receives the event and selects the SFS browser tab
		 * and selects the 'show upgraded' button.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onSfsBrowserShow(wxCommandEvent&);

		/**
		 * Appends a new policy to the RuleSet.
		 * Updates Columns regarding the policy and additionally
		 * registers the policy for observation
		 */
		long ruleListAppend(Policy*);

		/**
		 * Update row.
		 * Updates the values of a row showing an Sfs
		 * DefaultFilterPolicy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateSfsDefaultFilterPolicy(long);

		/**
		 * Update row.
		 * Updates the values of a row showing an Sfs FilterPolicy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateSfsFilterPolicy(long);
};

#endif /* __ModSfsMainPanelImpl__ */

/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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

#ifndef __DlgRuleEditor__
#define __DlgRuleEditor__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/config.h>
#include <wx/string.h>

#include "AnEvents.h"
#include "AnTable.h"
#include "DlgRuleEditorBase.h"
#include "Observer.h"

#include "Policy.h"
#include "AppPolicy.h"
#include "AlfFilterPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "DefaultFilterPolicy.h"
#include "SfsFilterPolicy.h"
#include "SfsDefaultFilterPolicy.h"
#include "ContextFilterPolicy.h"
#include "SbAccessFilterPolicy.h"
#include "AnMultiRowProvider.h"

/**
 * This is the anoubis rule editor.
 */
class DlgRuleEditor : public Observer, public DlgRuleEditorBase
{
	public:
		/**
		 * Constructor of RuleEditor.
		 * @param[in] 1st The parent window (aka MainFrame).
		 */
		DlgRuleEditor(wxWindow *);

		/**
		 * Destructor of RuleEditor.
		 * @param None.
		 */
		~DlgRuleEditor(void);

		/**
		 * This is called when an observed policy was modified.
		 * @param[in] 1st The changed policy (aka subject)
		 * @return Nothing.
		 */
		virtual void update(Subject *);

		/**
		 * This is called when an observed policy is about to
		 * be destroyed.
		 * @param[in] 1st The changed policy (aka subject)
		 * @return Nothing.
		 */
		virtual void updateDelete(Subject *);

	private:
		/**
		 * With this multi row provider we join two rule sets
		 * (aka user and admin rule set) to be shown in one table.
		 */
		AnMultiRowProvider multiRowProvider_;

		/**
		 * Table of app policies. This also holds column properties.
		 */
		AnTable *appTable_;

		/**
		 * Table of alf policies. This also holds column properties.
		 */
		AnTable *alfTable_;

		/**
		 * Table of sfs policies. This also holds column properties.
		 */
		AnTable *sfsTable_;

		/**
		 * Table of ctx policies. This also holds column properties.
		 */
		AnTable *ctxTable_;

		/**
		 * Table of sb policies. This also holds column properties.
		 */
		AnTable *sbTable_;

		/**
		 * Id of our ruleSet.
		 */
		long userRuleSetId_;

		/**
		 * Id of our admin ruleSet.
		 */
		long adminRuleSetId_;

		/**
		 * connectiion state of gui
		 */
		bool isConnected_;

		/**
		 * Handle show events.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onShow(wxCommandEvent &);

		/**
		 * Handle close by window decoration.
		 * This will not just toggle the visability of this frame.
		 * Instead an event anEVT_RULEEDITOR_SHOW will been sent to
		 * inform all parts of the GUI (e.g MainFrame menue or the
		 * buttons of the status bar) about the closed RuleEditor.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onClose(wxCloseEvent &);

		/**
		 * Change the current rule set in the rule editor
		 * @param[in] 1st The admin rule set to show.
		 * @param[in] 2nd The user rule set to show.
		 * @return Nothing.
		 */
		void switchRuleSet(long, long);

		/**
		 * Handle connection events.
		 * This will just extract the status from the event and
		 * stores it. The footer is updated.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onConnectionStateChange(wxCommandEvent &);

		/**
		 * Handle new RuleSet events.
		 * This just receives the event and updates the id's of
		 * user and admin ruleset. loadRuleSet() is called to
		 * update the view.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onLoadNewRuleSet(wxCommandEvent &);

		/**
		 * Handle situation, where a ruleset was send to the daemon.
		 *
		 * Used to update the view.
		 *
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onSendRuleSet(wxCommandEvent &);

		/**
		 * Handle Show rule events from the log viewer and the
		 * escalation handling.
		 * @param[in] 1st The command event.
		 * @return Nothing.
		 * The Command member (SetInt) is true if we show a rule from
		 * the admin rule set, otherwise it is false. The extra long
		 * member is the rule id.
		 */
		void onShowRule(wxCommandEvent& event);

		/**
		 * Handle selection of app policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onAppGridCellSelect(wxGridEvent &);

		/**
		 * Handle selection of app policy by row label.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onAppGridLabelClick(wxGridEvent &);

		/**
		 * Handle selection of filter policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onFilterGridCellSelect(wxGridEvent &);

		/**
		 * Handle selection of filter policy by row label.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onFilterGridLabelClick(wxGridEvent &);

		/**
		 * Move selected application policy one row up.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onAppListUpClick(wxCommandEvent &);

		/**
		 * Move selected application policy one row down.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onAppListDownClick(wxCommandEvent &);

		/**
		 * Move selected filter policy one row up.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onFilterListUpClick(wxCommandEvent &);

		/**
		 * Move selected filter policy one row down.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onFilterListDownClick(wxCommandEvent &);

		/**
		 * Delete selected application policy.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onAppListDeleteClick(wxCommandEvent &);

		/**
		 * Delete selected filter policy.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onFilterListDeleteClick(wxCommandEvent &);

		/**
		 * Handle update of App-Rules column header options chosen by
		 * the user.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onAppListColumnsButtonClick(wxCommandEvent &);

		/**
		 * Handle update of Filter-Rules column header options chosen by
		 * the user.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onFilterListColumnsButtonClick(wxCommandEvent &);

		/**
		 * Create application policy.
		 * The type of the new policy is taken from appListTypeChoice.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onAppListCreateButton(wxCommandEvent &);

		/**
		 * Create filter policy.
		 * The type of the new policy is taken from
		 * filterListTypeChoice.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onFilterListCreateButton(wxCommandEvent &);

		/**
		 * Handle event from importButton.
		 * This will load a ruleset form a file.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onFooterImportButton(wxCommandEvent &);

		/**
		 * Retreive a new version of the given policy from the
		 * Daemon. Retruns true if the ruleset is a daemon ruleset
		 * and the ruleset could be reloaded.
		 * @param[in] 1st The ProfileCtrl ruleset id
		 * @return True if a request was stared.
		 */
		bool reloadRuleSet(long);

		/**
		 * Handle event from reloadButton.
		 * This will load the ruleset from daemon.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onFooterReloadButton(wxCommandEvent &);

		/**
		 * Handle event from exportButton.
		 * This will export the ruleset to a file.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onFooterExportButton(wxCommandEvent &);

		/**
		 * Handle event from activateButton.
		 * This will send the ruelset to the daemon.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void onFooterActivateButton(wxCommandEvent &);

		/**
		 * Get the policy of the selected row.
		 * @param[in] 1st The grid to get the selected row from.
		 * @return The policy of the selected row or NULL.
		 */
		Policy *getSelectedPolicy(wxGrid *);

		/**
		 * Select row.
		 * Selects the given row of the given grid by\n
		 *	a) move the cursor to the given row (and column 0)\n
		 *	b) select the whole row
		 * @param[in] 1st The grid in question.
		 * @param[in] 2nd The row to select.
		 * @return Nothing.
		 */
		void selectRow(wxGrid *, int) const;

		/**
		 * Load new ruleSet.
		 * This will clear the appList and load it with new content.
		 * @param None.
		 * @return Nothing.
		 */
		void loadRuleSet(void);

		/**
		 * Create an empty PolicyRuleSet.
		 * @param None.
		 * @return Created PolicyRuleSet.
		 */
		PolicyRuleSet *createEmptyPolicyRuleSet(void);

		/**
		 * Update footer status line and buttons.
		 * Updates the widgets and status information related
		 * to the ruleSet.
		 * @param None.
		 * @return Nothing.
		 */
		void updateFooter(void);

		/**
		 * Display rules of the given user.
		 * @param[in] 1st The user ID.
		 * @return Nothing.
		 */
		void setUser(long);

		/**
		 * Display rules of the given user.
		 * @param[in] 1st The user name.
		 * @return Nothing.
		 */
		void setUser(wxString);

		/**
		 * Event handler for the user selection radio button.
		 * @param None.
		 */
		void onRbUserSelect(wxCommandEvent &);

		/**
		 * Event handler for the "show my rules" radio button.
		 * @param None.
		 */
		void onRbUserMe(wxCommandEvent &);

		/**
		 * Event handler for the "show default rules" radio button.
		 * @param None.
		 */
		void onRbUserDefault(wxCommandEvent &);

		/**
		 * Event handler for text entered into the user select field
		 * @param None.
		 */
		void onUserSelectTextEnter(wxCommandEvent &);

		/**
		 * Event handler for text entered into the user select field
		 * @param None.
		 */
		void onUserSelectKillFocus(wxFocusEvent &);

		/**
		 * The ruleset user-ID that is currently displayed in
		 * the rule editor.
		 */
		int	editorUid_;
};

#endif /* __DlgRuleEditor__ */

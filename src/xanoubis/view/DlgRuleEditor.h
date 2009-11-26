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

#include <wx/progdlg.h>
#include <wx/config.h>

#include "AnEvents.h"
#include "DlgRuleEditorBase.h"
#include "ListCtrlColumn.h"
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

		/**
		 * Add application policy.
		 * A new row is created (by addListRow()) and filled by
		 * updateListAppPolicy(). This should be used by
		 * RuleEditorAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addAppPolicy(AppPolicy *);

		/**
		 * Add alf filter policy.
		 * A new row is created (by addListRow()) and filled by
		 * updateListAlfFilterPolicy(). This should be used by
		 * RuleEditorAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addAlfFilterPolicy(AlfFilterPolicy *);

		/**
		 * Add alf capabiliyt filter policy.
		 * A new row is created (by addListRow()) and filled by
		 * updateListAlfCapabilityFilterPolicy(). This should be used
		 * by RuleEditorAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addAlfCapabilityFilterPolicy(AlfCapabilityFilterPolicy *);

		/**
		 * Add default filter policy.
		 * A new row is created (by addListRow()) and filled by
		 * updateListDefaultFilterPolicy(). This should be used
		 * by RuleEditorAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addDefaultFilterPolicy(DefaultFilterPolicy *);

		/**
		 * Add sfs filter policy.
		 * A new row is created (by addListRow()) and filled by
		 * updateListSfsFilterPolicy(). This should be used
		 * by RuleEditorAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addSfsFilterPolicy(SfsFilterPolicy *);

		/**
		 * Add sfs default filter policy.
		 * A new row is created (by addListRow()) and filled by
		 * updateListSfsDefaultFilterPolicy(). This should be used
		 * by RuleEditorAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addSfsDefaultFilterPolicy(SfsDefaultFilterPolicy *);

		/**
		 * Add context filter policy.
		 * A new row is created (by addListRow()) and filled by
		 * updateListContextFilterPolicy(). This should be used
		 * by RuleEditorAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addContextFilterPolicy(ContextFilterPolicy *);

		/**
		 * Add sandbox filter policy.
		 * A new row is created (by addListRow()) and filled by
		 * updateListSandboxFilterPolicy(). This should be used
		 * by RuleEditorAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addSbAccessFilterPolicy(SbAccessFilterPolicy *);

	private:
		/**
		 * Use these indices to access the related column within the
		 * list of application columns.
		 */
		enum appColumnIndex {
			APP_ID = 0,	/**< apn rule id. */
			APP_TYPE,	/**< type of policy. */
			APP_USER,	/**< user of ruleset. */
			APP_NOSFS,	/**< NOSFS flag. */
			APP_BINARY,	/**< name of concerning binary. */
			APP_EOL		/**< End - Of - List */
		};

		/**
		 * Use these indices to access the related column within the
		 * list of alf filter columns.
		 */
		enum alfColumnIndex {
			ALF_ID = 0,	/**< apn rule id. */
			ALF_TYPE,	/**< type of context */
			ALF_ACTION,	/**< action */
			ALF_LOG,	/**< log */
			ALF_SCOPE,	/**< scope of this policy */
			ALF_CAP,	/**< capability type */
			ALF_DIR,	/**< direction */
			ALF_PROT,	/**< protocol */
			ALF_FHOST,	/**< from host */
			ALF_FPORT,	/**< from port */
			ALF_THOST,	/**< to host */
			ALF_TPORT,	/**< to port */
			ALF_EOL		/**< End - Of - List */
		};

		/**
		 * Use these indices to access the related column within the
		 * list of sfs filter columns.
		 */
		enum sfsColumnIndex {
			SFS_ID = 0,	/**< apn rule id. */
			SFS_TYPE,	/**< type of sfs */
			SFS_PATH,	/**< path (of subject) */
			SFS_SUB,	/**< the subject */
			SFS_SCOPE,	/**< scope of this policy */
			SFS_VA,		/**< valid action */
			SFS_VL,		/**< valid log */
			SFS_IA,		/**< invalid action */
			SFS_IL,		/**< invalid log */
			SFS_UA,		/**< unknown action */
			SFS_UL,		/**< unknown log */
			SFS_EOL		/**< End - Of - List */
		};

		/**
		 * Use these indices to access the related column within the
		 * list of context filter columns.
		 */
		enum ctxColumnIndex {
			CTX_ID = 0,	/**< apn rule id. */
			CTX_TYPE,	/**< type of context */
			CTX_BINARY,	/**< name of binary */
			CTX_EOL		/**< End - Of - List */
		};

		/**
		 * Use these indices to access the related column within the
		 * list of sandbox filter columns.
		 */
		enum sbColumnIndex {
			SB_ID = 0,	/**< apn rule id. */
			SB_TYPE,	/**< type of context */
			SB_ACTION,	/**< action */
			SB_LOG,		/**< log */
			SB_SCOPE,	/**< scope of this policy */
			SB_PATH,	/**< path (of subject) */
			SB_SUB,		/**< the subject */
			SB_MASK,	/**< the access mask */
			SB_EOL		/**< End - Of - List */
		};

		ListCtrlColumn *appColumns_[APP_EOL]; /**< @ appList */
		ListCtrlColumn *alfColumns_[ALF_EOL]; /**< @ filterList */
		ListCtrlColumn *sfsColumns_[SFS_EOL]; /**< @ filterList */
		ListCtrlColumn *ctxColumns_[CTX_EOL]; /**< @ filterList */
		ListCtrlColumn *sbColumns_[SB_EOL];   /**< @ filterList */

		long userRuleSetId_;  /**< Id of our ruleSet. */
		long adminRuleSetId_; /**< Id of our admin ruleSet. */

		bool isConnected_; /**< connectiion state of gui */
		bool isAutoChecksumCheck_; /**< state of Checksums before send
					    * to Daemon
					    */

		int appPolicyLoadProgIdx_; /**< current progress position */
		wxProgressDialog *appPolicyLoadProgDlg_; /**< progress bar */
		int filterPolicyLoadProgIdx_; /**< current progress position */
		wxProgressDialog *filterPolicyLoadProgDlg_; /**< progress bar */

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
		 * Handle AutoChecksumCheck Option.
		 * @param[in] 1st The Show event.
		 * @return Nothing
		 * The Command member (SetInt) is true if the option is set.
		 */
		void onAutoCheck(wxCommandEvent & event);

		/**
		 * Handle selection of app policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAppPolicySelect(wxListEvent &);

		/**
		 * Handle de-selection of app policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onAppPolicyDeSelect(wxListEvent &);

		/**
		 * Handle selection of filter policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onFilterPolicySelect(wxListEvent &);

		/**
		 * Handle de-selection of filter policy.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		virtual void onFilterPolicyDeSelect(wxListEvent &);

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
		 * Add a row to a given list and assign the given policy.
		 * @param[in] 1st The list where the new row is added.
		 * @param[in] 2nd The concerning policy for the new row.
		 * @return The index of the new row.
		 */
		long addListRow(wxListCtrl *, Policy *);

		/**
		 * Remove a given row from a given list.
		 * @param[in] 1st The list the row is removed from.
		 * @param[in] 2nd The index of the row in question.
		 * @return Nothing.
		 */
		void removeListRow(wxListCtrl *, long);

		/**
		 * Find the row of a given policy.
		 * @param[in] 1st The list to search in.
		 * @param[in] 2nd The policy to search for.
		 * @return The index of found row or -1.
		 */
		long findListRow(wxListCtrl *, Policy *);

		/**
		 * Get the index of the selected row.
		 * @param[in] 1st The list to get the selected row from.
		 * @return The index of the selected row or -1
		 * @see getSelectedPolicy()
		 */
		long getSelectedIndex(wxListCtrl *);

		/**
		 * Get the policy of the selected row.
		 * @param[in] 1st The list to get the selected row from.
		 * @return The policy of the selected row or NULL.
		 * @see getSelectedIndex()
		 */
		Policy *getSelectedPolicy(wxListCtrl *);

		/**
		 * Select the a row and make it visible. Try to scroll
		 * away as few as possible from a previously visible
		 * part of the list.
		 * @param[in] 1st The list to select the row.
		 * @param[in] 2nd The index of the first element that
		 *    was previously visible.
		 * @param[in] 3rd The index of the row to select.
		 * @return Nothing.
		 */
		void selectFrame(wxListCtrl *, long, long);

		/**
		 * Deselect row.
		 * This will deselect the selected row of the given list.
		 * This will cause a deselect event.
		 * @param[in] 1st The list to deselect the row.
		 * @return Nothing.
		 */
		void deselect(wxListCtrl *);

		/**
		 * Clean the application list.
		 * @param None.
		 * @return Nothing.
		 */
		void wipeAppList(void);

		/**
		 * Clean the filter list.
		 * @param None.
		 * @return Nothing.
		 */
		void wipeFilterList(void);

		/**
		 * Add / visit ruleset for loading (app policies).
		 * @param[in] 1st RuleSet to load.
		 * @return Nothing.
		 */
		void addPolicyRuleSet(PolicyRuleSet *);

		/**
		 * Add /visit filter of app policy for
		 * @param[in] 1st AppPolicy with filters.
		 * @return Nothind.
		 */
		void addFilterPolicy(AppPolicy *);

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
		 * Update progress bar.
		 * @param[in] 1st The progress bar dialog to be updated.
		 * @param[in] 2nd The index of this progress bar.
		 * @param[in] 3rd The policy caused the update.
		 * @return Nothing.
		 */
		void updateProgDlg(wxProgressDialog *, int *, Policy *);

		/**
		 * Update columns.
		 * Updates the columns of the given list. Only the visible
		 * columns are created and the index of those is updated.
		 * If the list has already columns, no action is performed.
		 * @param[in] 1st The list showing the new columns.
		 * @param[in] 2nd The array of columns.
		 * @param[in] 3rd The size of that array.
		 * @return Nothing.
		 */
		void updateListColumns(wxListCtrl *,ListCtrlColumn **, size_t);

		/* XXX Dokumentation */
		void updateColumnID(wxListCtrl *, long, ListCtrlColumn *,
		    Policy *);

		/* XXX Dokumentation */
		void updateColumnText(wxListCtrl *, long, ListCtrlColumn *,
		    wxString);

		/**
		 * Update row.
		 * Updates the values of a row showing an application policy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateListAppPolicy(long);

		/**
		 * Update row.
		 * Updates the values of a row showing an alf filter policy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateListAlfFilterPolicy(long);

		/**
		 * Update row.
		 * Updates the values of a row showing an alf capability
		 * filter policy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateListAlfCapabilityFilterPolicy(long);

		/**
		 * Update row.
		 * Updates the values of a row showing a sfs filter policy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateListSfsFilterPolicy(long);

		/**
		 * Update row.
		 * Updates the values of a row showing a context filter policy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateListContextFilterPolicy(long);

		/**
		 * Update row.
		 * Updates the values of a row showing a sandbox filter policy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateListSbAccessFilterPolicy(long);

		/**
		 * Update footer status line and buttons.
		 * Updates the widgets and status information related
		 * to the ruleSet.
		 * @param None.
		 * @return Nothing.
		 */
		void updateFooter(void);

		/**
		 * Read settings for the visible column headers
		 * @param None.
		 * @return Nothing.
		 */
		void readOptions(void);

		/**
		 * Write settings for the visible column headers
		 * @param None.
		 * @return Nothing.
		 */
		void writeOptions(void);

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
		 * Save the column width of the columns that are
		 * currently visible in a ListCtrl in the appropriate
		 * column structure.
		 * @param[in] 1st The ListCtrl
		 * @param[in] 2nd The list of column data structures.
		 *     Invisible columns are skipped.
		 * @param[in] 3rd The number of columns in the column list.
		 */
		void saveColumnWidth(wxListCtrl *, ListCtrlColumn *c[], int);

		/**
		 * Save the columns widths of the filter list Ctrl. This
		 * function automatically detects the columns to use from
		 * the selection in the appListCtrl.
		 * @param The line of the current AppPolicy. Minus 1 for
		 *     auto detect.
		 */
		void saveFilterColumnWidth(int selection = -1);

		/**
		 * The ruleset user-ID that is currently displayed in
		 * the rule editor.
		 */
		int	editorUid_;
};

#endif /* __DlgRuleEditor__ */

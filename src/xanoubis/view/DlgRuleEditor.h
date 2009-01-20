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

#if 0
#include <wx/list.h>
#include <wx/string.h>
#include <wx/filename.h>
#include <wx/dynarray.h>
#include <wx/combobox.h>

#include "AnEvents.h"
#include "AnShortcuts.h"
#include "DlgRuleEditorBase.h"
#include "PolicyRuleSet.h"
#include "TaskEvent.h"

enum ruleEditorPanels {
	RULEDITOR_PANEL_COMMON = 0,
	RULEDITOR_PANEL_APP,
	RULEDITOR_PANEL_ALF,
	RULEDITOR_PANEL_SFS,
	RULEDITOR_PANEL_MACRO,
	RULEDITOR_PANEL_EOL
};

enum ruleEditorListColumns {
	RULEDITOR_LIST_COLUMN_PRIO = 0,
	RULEDITOR_LIST_COLUMN_RULE,
	RULEDITOR_LIST_COLUMN_USER,
	RULEDITOR_LIST_COLUMN_BIN,
	RULEDITOR_LIST_COLUMN_HASHT,
	RULEDITOR_LIST_COLUMN_HASH,
	RULEDITOR_LIST_COLUMN_TYPE,
	RULEDITOR_LIST_COLUMN_SCOPE,
	RULEDITOR_LIST_COLUMN_ACTION,
	RULEDITOR_LIST_COLUMN_LOG,
	RULEDITOR_LIST_COLUMN_AF,
	RULEDITOR_LIST_COLUMN_CAP,
	RULEDITOR_LIST_COLUMN_PROTO,
	RULEDITOR_LIST_COLUMN_DIR,
	RULEDITOR_LIST_COLUMN_FHOST,
	RULEDITOR_LIST_COLUMN_FPORT,
	RULEDITOR_LIST_COLUMN_THOST,
	RULEDITOR_LIST_COLUMN_TPORT,
	RULEDITOR_LIST_COLUMN_STATETIMEOUT,
	RULEDITOR_LIST_COLUMN_EOL
};

/*
 * XXX ch: this will be fixed with the next functionality change
 */
//#if 0
class AddrLine
{
	private:
		wxSizer		*sizer_;
		wxWindow	*parent_;
		wxStaticText	*lead_;
		wxComboBox	*addr_;
		wxStaticText	*delimiter_;
		wxSpinCtrl	*net_;
		wxButton	*remove_;
		wxButton	*add_;

	public:
		AddrLine(wxWindow *, wxString, wxString);
		~AddrLine(void);
		void add(wxSizer *, size_t);
		void remove(void);
};
WX_DECLARE_LIST(AddrLine, AddrLineList);
WX_DEFINE_ARRAY_LONG(long, ArrayOfLongs);
//#endif

class DlgRuleEditor : public DlgRuleEditorBase
{

	private:
		unsigned long	selectedId_;
		unsigned long	selectedIndex_;
		bool		autoCheck_;
		wxString	columnNames_[RULEDITOR_LIST_COLUMN_EOL];
		int		columnWidths_[RULEDITOR_LIST_COLUMN_EOL];
		//AddrLineList	extraSrcAddrList;
		//AddrLineList	extraDstAddrList;
		long		userRuleSetId_;
		long		adminRuleSetId_;
		//ArrayOfLongs	foreignAdminRsIds_;

		void OnShow(wxCommandEvent&);
/*
 * XXX ch: this will be fixed with the next functionality change
 */
//#if 0
		void updateBinName(wxString);
		void updateContextName(wxString);
		void updateAction(int);
		void updateType(int);
		void updateProtocol(int);
		void updateAddrFamily(int);
		void updateCapType(int);
		void updateDirection(int);
		void updateTimeout(int);
		void updateAlfSrcAddr(wxString, int, int);
		void updateAlfDstAddr(wxString, int, int);
		void updateAlfSrcPort(wxString);
		void updateAlfDstPort(wxString);
		void updateLog(int);

		ANEVENTS_IDENT_BCAST_METHOD_DECLARATION;

	protected:
		AnShortcuts	*shortcuts_;

		void OnCommonLogNone(wxCommandEvent&);
		void OnCommonLogLog(wxCommandEvent&);
		void OnCommonLogAlert(wxCommandEvent&);

		void OnClose(wxCloseEvent& event);
		void OnTableOptionButtonClick(wxCommandEvent&);
		void OnAppUpdateChkSumButton(wxCommandEvent&);
		void OnAppValidateChkSumButton(wxCommandEvent&);
		void OnLoadRuleSet(wxCommandEvent&);
		void OnLineSelected(wxListEvent&);
		void OnRuleCreateButton(wxCommandEvent&);
		void OnRuleDeleteButton(wxCommandEvent&);
		void OnRuleSetSave(wxCommandEvent&);

		void OnAppBinaryTextCtrl(wxCommandEvent&);
		void OnAppBinaryModifyButton(wxCommandEvent&);
		void onAppContextTextCtrl(wxCommandEvent&);
		void onAppContextModifyButton(wxCommandEvent&);
		void onAppContextDeleteButton(wxCommandEvent&);

		void OnAlfAllowRadioButton(wxCommandEvent&);
		void OnAlfDenyRadioButton(wxCommandEvent&);
		void OnAlfAskRadioButton(wxCommandEvent&);

		void OnAlfFilterRadioButton(wxCommandEvent&);
		void OnAlfCapRadioButton(wxCommandEvent&);
		void OnAlfDefaultRadioButton(wxCommandEvent&);

		void OnAlfTcpRadioButton(wxCommandEvent&);
		void OnAlfUdpRadioButton(wxCommandEvent&);

		void OnAlfInetRadioButton(wxCommandEvent&);
		void OnAlfInet6RadioButton(wxCommandEvent&);
		void OnAlfAnyRadioButton(wxCommandEvent&);

		void OnAlfRawCapRadioButton(wxCommandEvent&);
		void OnAlfOtherCapRadioButton(wxCommandEvent&);
		void OnAlfAllCapRadioButton(wxCommandEvent&);

		void OnAlfAcceptRadioButton(wxCommandEvent&);
		void OnAlfConnectRadioButton(wxCommandEvent&);
		void OnAlfBothRadioButton(wxCommandEvent&);

		void OnSfsBinaryModifyButton(wxCommandEvent&);
		void OnSfsUpdateChkSumButton(wxCommandEvent&);
		void OnSfsValidateChkSumButton(wxCommandEvent&);
		void OnSfsBinaryTextCtrl(wxCommandEvent&);

		void onAlfSrcAddrTextCtrlEnter(wxCommandEvent&);
		void onAlfDstAddrTextCtrlEnter(wxCommandEvent&);
		void OnAlfSrcNetmaskSpinCtrl(wxSpinEvent&);
		void OnAlfDstNetmaskSpinCtrl(wxSpinEvent&);
		void onAlfSrcPortTextCtrlEnter(wxCommandEvent&);
		void onAlfDstPortTextCtrlEnter(wxCommandEvent&);
		void OnAlfStateTimeoutChange(wxCommandEvent&);

		void loadRuleSet(void);
		void OnSrcAddrAddButton(wxCommandEvent&);

		void OnAlfStateTimeoutChange(wxSpinEvent&);
		void OnAutoCheck(wxCommandEvent&);
		void OnShowRule(wxCommandEvent&);

		bool CheckLastSelection(void);

		void OnChecksumCalc(TaskEvent &);
		void OnChecksumAdd(TaskEvent &);
		void OnChecksumGet(TaskEvent &);

		void selectLine(unsigned long);
		void modified(void);
		//#endif
	public:
		DlgRuleEditor(wxWindow *);
		~DlgRuleEditor(void);

		friend class RuleEditorAddPolicyVisitor;
		friend class RuleEditorFillWidgetsVisitor;
		friend class RuleEditorFillTableVisitor;
};
#endif

#include "AnEvents.h"
#include "DlgRuleEditorBase.h"
#include "ListCtrlColumn.h"

#include "Policy.h"
#include "AppPolicy.h"
#include "AlfFilterPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "ContextFilterPolicy.h"
#include "SfsFilterPolicy.h"

/**
 * This is the anoubis rule editor.
 */
class DlgRuleEditor : public DlgRuleEditorBase
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
		 * Add sfs filter policy.
		 * A new row is created (by addListRow()) and filled by
		 * updateListSfsFilterPolicy(). This should be used
		 * by RuleEditorAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addSfsFilterPolicy(SfsFilterPolicy *);

		/**
		 * Add context filter policy.
		 * A new row is created (by addListRow()) and filled by
		 * updateListContextFilterPolicy(). This should be used
		 * by RuleEditorAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addContextFilterPolicy(ContextFilterPolicy *);

	private:
		/**
		 * Use these indices to access the related column within the
		 * list of application columns.
		 */
		enum appColumnIndex {
			APP_ID = 0,	/**< apn rule id. */
			APP_TYPE,	/**< type of policy. */
			APP_USER,	/**< user of ruleset. */
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
			ALF_CAP,	/**< capability type */
			ALF_DIR,	/**< direction */
			ALF_PROT,	/**< protocol */
			ALF_AF,		/**< address family */
			ALF_FHOST,	/**< from host */
			ALF_FPORT,	/**< from port */
			ALF_THOST,	/**< to host */
			ALF_TPORT,	/**< to port */
			ALF_TIME,	/**< state timeout */
			ALF_EOL		/**< End - Of - List */
		};

		/**
		 * Use these indices to access the related column within the
		 * list of sfs filter columns.
		 */
		enum sfsColumnIndex {
			SFS_ID = 0,	/**< apn rule id. */
			SFS_PATH,	/**< path (of subject) */
			SFS_SUB,	/**< the subject */
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

		ListCtrlColumn *appColumns_[APP_EOL]; /**< @ appList */
		ListCtrlColumn *alfColumns_[ALF_EOL]; /**< @ filterList */
		ListCtrlColumn *sfsColumns_[SFS_EOL]; /**< @ filterList */
		ListCtrlColumn *ctxColumns_[CTX_EOL]; /**< @ filterList */

		long userRuleSetId_;  /**< Id of our ruleSet. */
		long adminRuleSetId_; /**< Id of our admin ruleSet. */

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
		 * Handle new RuleSet events.
		 * This just receives the event and updates the id's of
		 * user and admin ruleset. loadRuleSet() is called to
		 * update the view.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onLoadNewRuleSet(wxCommandEvent &);

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
		 * Load new ruleSet.
		 * This will clear the appList and load it with new content.
		 * @param None.
		 * @return Nothing.
		 */
		void loadRuleSet(void);

		/**
		 * Load new ruleSet and refresh the lists.
		 * @param[in] 1st Select this index in the appList 
		 * @param[in] 2nd Select this index in the filterList
		 * @return None.
		 * Both parameters can be -1 if the previous selection
		 * should be reused.
		 */
		void refreshRuleSet(long, long);

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

		/**
		 * Move selected rule up
		 * @param none
		 * @return Nothing.
		 */
		void onAppListUpClick(wxCommandEvent &);

		/**
		 * Move selected rule down
		 * @param none
		 * @return Nothing.
		 */
		void onAppListDownClick(wxCommandEvent &);

		/**
		 * Delete selected rule
		 * @param none
		 * @return Nothing.
		 */
		void onAppListDeleteClick(wxCommandEvent &);

protected:
		/**
		 * Common backend for moving the selected rule up or down
		 * @param Move upwards if true, otherwise move down
		 * @return Nothing.
		 */
		void appListUpDown(bool);

};

#endif /* __DlgRuleEditor__ */

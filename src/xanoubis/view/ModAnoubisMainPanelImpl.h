/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#ifndef __ModAnoubisMainPanelImpl__
#define __ModAnoubisMainPanelImpl__

#include <vector>
#include <map>

#include "ModAnoubisPanelsBase.h"
#include "Notification.h"
#include "NotificationCtrl.h"
#include "Observer.h"

class ModAnoubisMainPanelImpl : public ModAnoubisMainPanelBase,
    private Observer
{
	private:
		NotificationPerspective *listPerspective_;

		/**
		 * This is our hash, to resolve the selected element of
		 * each listPerspective_.
		 */
		std::map<NotificationPerspective *, long> elementNoHash_;

		/**
		 * To notice any removal of an element from
		 * LIST_NOTANSWERED, we remember the old size from
		 * this list.
		 */
		long		 sizeListNotAnswered_;
		Notification	*currentNotify_;
		Notification	*savedNotify_;
		wxString	 selectedProfile;
		wxString	 loadedProfile;

		/* Path string related variables */
		std::vector<wxString>	pathcomp_;
		unsigned int		pathKeep_, minPathKeep_;

		/**
		 * Assemble time answer.
		 *
		 * Create a special type of answer.
		 *
		 * param[in] 1st The answer permit (0=deny, 1=allow).
		 * return The new answer.
		 */
		NotifyAnswer *assembleTimeAnswer(bool);

		/**
		 * Create an answer to the current escalation.
		 *
		 * Depending on the given permit from the buttons 'allow'
		 * and 'deny' an answer is created.
		 *
		 * param[in] 1st The answer permit (0=deny, 1=allow).
		 * return Nothing.
		 */
		void answerEscalation(bool);

		void displayMessage(void);
		void sendNotifierOptionsEvents(WXTYPE, bool, long);
		void sendNotifierOptions(void);
		void readOptions(void);
		void setOptionsWidgetsVisability(void);
		void versionListUpdate(void);
		void versionListUpdateFromSelection(void);
		void profileTabInit(void);
		void profileTabUpdate(void);
		void toolTipParamsUpdate(void);
		void editOptionsEnable(void);
		void initPathLabel(void);
		void setPathLabel(void);
		void setAlfOptions(EscalationNotify *, NotifyAnswer *);
		void setSfsOptions(EscalationNotify *, NotifyAnswer *);
		void setSbOptions(EscalationNotify *, NotifyAnswer *);
		void setPerspective(NotificationCtrl::ListPerspectives);

		void update(Subject *);
		void updateDelete(Subject *);
		void updatePSDetails(void);

		static wxString uidToString(long uid);
		static wxString pgidToString(uint64_t pgid);

	protected:
		void onConnectionStateChange(wxCommandEvent &);
		void onTabChange(wxNotebookEvent &);
		void OnLoadRuleSet(wxCommandEvent&);

		void OnTypeChoosen(wxCommandEvent&);
		void OnFirstBtnClick(wxCommandEvent&);
		void OnPreviousBtnClick(wxCommandEvent&);
		void OnNextBtnClick(wxCommandEvent&);
		void OnLastBtnClick(wxCommandEvent&);
		void OnAllowBtnClick(wxCommandEvent&);
		void OnDenyBtnClick(wxCommandEvent&);
		void OnEscalationOnceButton(wxCommandEvent&);
		void OnEscalationProcessButton(wxCommandEvent&);
		void OnEscalationTimeoutButton(wxCommandEvent&);
		void OnEscalationAlwaysButton(wxCommandEvent&);

		void OnEscalationDisable(wxCommandEvent&);
		void OnAlertDisable(wxCommandEvent&);
		void OnEscalationNoTimeout(wxCommandEvent&);
		void OnAlertNoTimeout(wxCommandEvent&);
		void OnEscalationTimeout(wxSpinEvent&);
		void OnAlertTimeout(wxSpinEvent&);

		void OnEscalationsShow(wxCommandEvent&);
		void OnAnoubisOptionShow(wxCommandEvent&);
		void OnEnableUpgradeMsg(wxCommandEvent&);
		void OnEnableKernelMsg(wxCommandEvent&);
		void OnEnableInformationMsg(wxCommandEvent&);

		void OnDoAutostart(wxCommandEvent&);

		void OnProfileDeleteClicked(wxCommandEvent &);
		void OnProfileLoadClicked(wxCommandEvent &);
		void OnProfileSaveClicked(wxCommandEvent &);
		void OnProfileActivateClicked(wxCommandEvent &);
		void OnProfileSelectionChanged(wxListEvent &);

		void OnNotebookTabChanged(wxNotebookEvent&);
		void OnVersionListCtrlSelected(wxListEvent&);
		void OnVersionRestoreButtonClick(wxCommandEvent&);
		void OnVersionExportButtonClick(wxCommandEvent&);
		void OnVersionDeleteButtonClick(wxCommandEvent&);
		void OnVersionShowButtonClick(wxCommandEvent&);
		void OnVersionActivePolicyClicked(wxCommandEvent &);
		void OnVersionProfilePolicyClicked(wxCommandEvent &);
		void OnVersionProfileChoice(wxCommandEvent &);

		void OnToolTipCheckBox(wxCommandEvent &);
		void OnToolTipSpinCtrl(wxSpinEvent &);

		void OnEscalationSfsPathLeft(wxCommandEvent &);
		void OnEscalationSfsPathRight(wxCommandEvent &);
		void OnEscalationSbPathLeft(wxCommandEvent &);
		void OnEscalationSbPathRight(wxCommandEvent &);

		void OnPSListItemSelected(wxListEvent &);
		void OnPSListItemDeselected(wxListEvent &);

		void OnPSListColumnButtonClick(wxCommandEvent&);

		/**
		 * This is called when the checkbox of the upgrade-dialog
		 * is modified.
		 * The dialog is sending an event (anEVT_ANOUBISOPTIONS_UPDATE)
		 * @param[in] 1st The event which is thrown in the dialog
		 * @return Nothing
		 */
		void OnAnoubisOptionsUpdate(wxCommandEvent &);

		void onPsReloadClicked(wxCommandEvent &);

	public:
		ModAnoubisMainPanelImpl(wxWindow*, wxWindowID);
		~ModAnoubisMainPanelImpl(void);
		void update(void);
		bool Show(bool show = true);
};

#endif /* __ModAnoubisMainPanelImpl__ */

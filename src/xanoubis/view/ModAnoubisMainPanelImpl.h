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

#include "ModAnoubisPanelsBase.h"
#include "Notification.h"
#include "NotificationCtrl.h"
#include "Observer.h"

class ModAnoubisMainPanelImpl : public ModAnoubisMainPanelBase,
    private Observer
{
	private:
		NotificationPerspective *listPerspective_;
		wxArrayLong::const_iterator it_;

		Notification	*currentNotify_;
		Notification	*savedNotify_;
		wxString	 selectedProfile;
		wxString	 loadedProfile;

		/* Path string related variables */
		std::vector<wxString>	pathcomp_;
		unsigned int		pathKeep_, minPathKeep_;

		void answer(bool);
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

	protected:
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
		void OnEscalationRuleError(wxCommandEvent &);

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

		/**
		 * This is called when the checkbox of the upgrade-dialog
		 * is modified.
		 * The dialog is sending an event (anEVT_ANOUBISOPTIONS_UPDATE)
		 * @param[in] 1st The event which is thrown in the dialog
		 * @return Nothing
		 */
		void OnAnoubisOptionsUpdate(wxCommandEvent &);

	public:
		ModAnoubisMainPanelImpl(wxWindow*, wxWindowID);
		~ModAnoubisMainPanelImpl(void);
		void update(void);
};

#endif /* __ModAnoubisMainPanelImpl__ */

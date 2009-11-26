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

#ifndef __MAINFRAME_H__
#define __MAINFRAME_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "AnEvents.h"
#include "AnStatusBar.h"
#include "MainFrameBase.h"
#include "Module.h"
#include "ComSfsListTask.h"
#include "SfsCtrl.h"
#include "DlgUpgradeAsk.h"

class TaskEvent;
class TrayIcon;

class MainFrame : public MainFrameBase
{
	private:
		/*
		 * In wxGTK, frames can't get the focus and thus will not
		 * receive key events; but a (this) panel can.
		 */
		DlgLogViewer	*logViewer_;
		DlgRuleEditor	*ruleEditor_;
		TrayIcon	*trayIcon_;
		unsigned int     messageAlertCount_;
		unsigned int     messageEscalationCount_;
		bool		 exit_;
		wxIcon		*aboutIcon_;
		wxIcon		*okIcon_;
		wxIcon		*errorIcon_;
		wxIcon		*alertIcon_;
		wxIcon		*escalationIcon_;
		ComSfsListTask	 upgradeTask_;

		void setConnectionString(bool, const wxString &);
		void setMessageString(void);

		void onRuleEditorShow(wxCommandEvent&);
		void onLogViewerShow(wxCommandEvent&);
		void onWizardShow(wxCommandEvent&);
		void onSfsBrowserShow(wxCommandEvent&);
		void onBackupPolicy(wxCommandEvent&);
		void doUpgradeNotify(void);
		void onUpgradeNotify(wxCommandEvent &);

		/**
		 * This is called when there is a list of sfs files. The
		 * main frame only cares about the sfs list from its own
		 * upgrade list task and opens the dialog box on connect.
		 * @param[in] 1st The event
		 * @return Nothing
		 */
		void onSfsListArrived(TaskEvent &);

		ANEVENTS_IDENT_BCAST_METHOD_DECLARATION;

	protected:
		AnStatusBar *an_statusbar;

		void OnMbHelpAboutSelect(wxCommandEvent&);
		void OnMbFileConnectSelect(wxCommandEvent&);
		void OnMbFileCloseSelect(wxCommandEvent&);
		void OnMbFileImportSelect(wxCommandEvent&);
		void OnMbFileExportSelect(wxCommandEvent&);
		void OnMbFileQuitSelect(wxCommandEvent&);
		void OnMbEditPreferencesSelect(wxCommandEvent&);
		void OnMbToolsRuleEditorSelect(wxCommandEvent&);
		void OnMbToolsLogViewerSelect(wxCommandEvent&);
		void onMbToolsWizardSelect(wxCommandEvent&);
		void OnMbHelpHelpSelect(wxCommandEvent&);
		void OnTbModuleSelect(wxCommandEvent&);
		void OnConnectionStateChange(wxCommandEvent&);
		void OnOpenAlerts(wxCommandEvent&);
		void OnOpenEscalations(wxCommandEvent&);
		void OnClose(wxCloseEvent& event);
		void OnEscalationsShow(wxCommandEvent&);
		void OnAnoubisOptionShow(wxCommandEvent&);

	public:
		MainFrame(wxWindow*, bool);
		~MainFrame();

		void OnInit(void);
		void addModules(Module* []);

		/**
		 * Call this method to exit the application.
		 *
		 * Invoking Close() will only hide the main-window but the
		 * application is still running!
		 */
		void exitApp(void);
};

#endif /* __MAINFRAME_H__ */

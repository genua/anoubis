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

#include "AnShortcuts.h"
#include "AnStatusBar.h"
#include "MainFrameBase.h"
#include "Module.h"

class MainFrame : public MainFrameBase
{
	private:
		/*
		 * In wxGTK, frames can't get the focus and thus will not
		 * receive key events; but a (this) panel can.
		 */
		AnShortcuts *shortcuts_;
		unsigned int     messageAlertCount_;
		unsigned int     messageEscalationCount_;
		wxIcon		*aboutIcon_;

		void setMessageString(void);

		void onRuleEditorShow(wxCommandEvent&);
		void onLogViewerShow(wxCommandEvent&);
		void onMainFrameShow(wxCommandEvent&);

	protected:
		AnStatusBar *an_statusbar;

		void OnMbHelpAboutSelect(wxCommandEvent&);
		void OnMbFileConnectSelect(wxCommandEvent&);
		void OnMbFileCloseSelect(wxCommandEvent&);
		void OnMbFileImportSelect(wxCommandEvent&);
		void OnMbFileQuitSelect(wxCommandEvent&);
		void OnMbEditPreferencesSelect(wxCommandEvent&);
		void OnMbToolsRuleEditorSelect(wxCommandEvent&);
		void OnMbToolsLogViewerSelect(wxCommandEvent&);
		void OnMbHelpHelpSelect(wxCommandEvent&);
		void OnTbModuleSelect(wxCommandEvent&);
		void OnRemoteStation(wxCommandEvent&);
		void OnOpenAlerts(wxCommandEvent&);
		void OnOpenEscalations(wxCommandEvent&);
		void OnClose(wxCloseEvent& event);

	public:
		MainFrame(wxWindow*);
		~MainFrame();

		void OnInit(void);
		bool OnQuit(void);
		void addModules(Module* []);

		void setDaemonConnection(bool);
		void setConnectionString(wxString);
};

#endif /* __MAINFRAME_H__ */

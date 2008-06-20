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

#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/param.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <wx/app.h>
#include <wx/config.h>
#include <wx/icon.h>
#include <wx/intl.h>
#include <wx/stdpaths.h>
#include <wx/string.h>

#include <wx/wxprec.h>
#include <wx/cmdline.h>

#include <apn.h>

#include "CommunicatorCtrl.h"
#include "ctassert.h"
#include "DlgLogViewer.h"
#include "DlgRuleEditor.h"
#include "MainFrame.h"
#include "Module.h"
#include "TrayIcon.h"
#include "PolicyRuleSet.h"

#define ANOUBISD_SOCKETNAME	"/var/run/anoubisd.sock"

enum moduleIdx {
	OVERVIEW = 0,
	ALF,
	SFS,
	ANOUBIS,
	LAST_MODULE_INDEX
};

compile_time_assert((LAST_MODULE_INDEX == ANOUBIS_MODULESNO), \
    MODULE_INDEX_mismatch_ANOUBIS_MODULESNO);

class AnoubisGuiApp : public wxApp
{
	private:
		wxString		 socketParam_;
		wxString		 profile_;
		wxStandardPaths		 paths_;
		wxLocale		 language_;
		PolicyRuleSet		*ruleSet_;
		MainFrame		*mainFrame;
		DlgLogViewer		*logViewer_;
		DlgRuleEditor		*ruleEditor_;
		CommunicatorCtrl	*comCtrl_;
		TrayIcon		*trayIcon;
		Module			*modules_[ANOUBIS_MODULESNO];
		wxConfig		*userOptions_;

	public:
		AnoubisGuiApp(void);
		~AnoubisGuiApp(void);

		bool	OnInit(void);
		void	quit(void);
		void	sendEvent(wxCommandEvent&);
		void	sendEvent(wxEventType);
		void	status(wxString);
		void	log(wxString);
		void	alert(wxString);

		void	OnInitCmdLine(wxCmdLineParser&);
		bool	OnCmdLineParsed(wxCmdLineParser&);

		void		 toggleLogViewerVisability(void);
		void		 toggleRuleEditorVisability(void);
		void		 connectCommunicator(bool);
		void		 requestPolicy(void);
		void		 usePolicy(wxString);
		wxString	 getCatalogPath(void);
		wxString	 getUtilsPath(void);
		wxString	 getIconPath(wxString);
		wxIcon		*loadIcon(wxString);
		Module		*getModule(enum moduleIdx);
		wxString	 getDataDir(void);
		void		 importPolicyRuleSet(struct apn_ruleset*);
		void		 importPolicyFile(wxString);
		void		 exportPolicyFile(wxString);
		bool		 getCommConnectionState(void);
		wxConfig	*getUserOptions(void);
		bool		 profileFromDiskToDaemon(wxString);
		bool		 profileFromDaemonToDisk(wxString);
		wxString	 getCurrentProfileName(void);
};

DECLARE_APP(AnoubisGuiApp)

#endif /* __MAIN_H__ */

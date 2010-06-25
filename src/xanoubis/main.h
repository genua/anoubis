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

#include <wx/cmdline.h>
#include <wx/config.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/wxprec.h>
#include <wx/wx.h>

#include "ComPolicyRequestTask.h"
#include "ComPolicySendTask.h"
#include "Debug.h"
#include "DlgLogViewer.h"
#include "DlgRuleEditor.h"
#include "KeyCtrl.h"
#include "MainFrame.h"
#include "Module.h"
#include "PolicyCtrl.h"
#include "PolicyRuleSet.h"
#include "TrayIcon.h"


/**
 * Main application class of xanoubis.
 */
class AnoubisGuiApp : public wxApp, private PassphraseReader
{
	public:
		/**
		 * Constructor.
		 */
		AnoubisGuiApp(void);

		/**
		 * Destructor.
		 */
		~AnoubisGuiApp(void);

		/**
		 * On initialisation.
		 * Initialize whole application after wxWidgets
		 * was initializes.
		 * @param None.
		 * @return True on success.
		 */
		bool OnInit(void);

		/**
		 * On exit.
		 * Cleanup on exit.
		 * @param None.
		 * @return Value ignored.
		 * @note From wxWidgets documentation: The return value of
		 * this function is currently ignored, return the same value
		 * as returned by the base class method if you override it.
		 */
		int OnExit(void);

		/**
		 * Initialize the parser with the command line options for this
		 * application.
		 * @param[in] 1st Parser for command lines.
		 * @return Nothing.
		 */
		void OnInitCmdLine(wxCmdLineParser&);

		/**
		 * Called after the command line had been successfully parsed.
		 * @param[in] 1st Parser for command lines.
		 * @return True on success.
		 */
		bool OnCmdLineParsed(wxCmdLineParser&);

	protected:
		/**
		 * Relay notifications.
		 * XXX ch: Can we remove this or do it another way?
		 * XXX ch: I'll take a look at this.
		 */
		void OnAnswerEscalation(wxCommandEvent &);

	private:
		wxLocale	 language_;
		bool		 onInitProfile_;
		bool		 trayVisible_;
		bool		 hide_;
		MainFrame	*mainFrame;
		int		 oldhandle_;

		/**
		 * Open Dialog to read passphrase.
		 * @param[out] 1st When a password was entered this is true.
		 * @return The entered password or an empty string.
		 */
		wxString readPassphrase(bool *);
};

DECLARE_APP(AnoubisGuiApp)

#endif /* __MAIN_H__ */

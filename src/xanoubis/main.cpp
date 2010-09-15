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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <anoubis_apnvm.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>

#include "AlertNotify.h"
#include "AnConfig.h"
#include "AnEvents.h"
#include "AnIconList.h"
#include "AnMessageDialog.h"
#include "DlgLogViewer.h"
#include "DlgRuleEditor.h"
#include "JobCtrl.h"
#include "LogNotify.h"
#include "MainFrame.h"
#include "MainUtils.h"
#include "ModAlf.h"
#include "ModAnoubis.h"
#include "ModOverview.h"
#include "ModPlayground.h"
#include "ModSb.h"
#include "ModSfs.h"
#include "Module.h"
#include "NotificationCtrl.h"
#include "PlaygroundCtrl.h"
#include "PolicyRuleSet.h"
#include "ProcCtrl.h"
#include "TrayIcon.h"
#include "VersionCtrl.h"
#include "main.h"

#ifdef USE_WXGUITESTING
#include <wxGuiTest/WxGuiTestHelper.h>
IMPLEMENT_APP(AnoubisGuiApp)
#else
IMPLEMENT_APP_NO_MAIN(AnoubisGuiApp)
#endif /* USE_WXGUITESTING */

IMPLEMENT_WX_THEME_SUPPORT

AnoubisGuiApp::AnoubisGuiApp(void)
{
	mainFrame = NULL;
	onInitProfile_ = true;
	trayVisible_ = true;
	hide_ = false;
	oldhandle_ = -1;

	SetAppName(wxT("xanoubis"));
	wxInitAllImageHandlers();
	Debug::initialize();
}

AnoubisGuiApp::~AnoubisGuiApp(void)
{
	Debug::uninitialize();
}

bool AnoubisGuiApp::OnInit()
{
	bool	hasLocale;
	int	versionError;

	if (!wxApp::OnInit())
		return false;

	wxTheApp->SetAppName(wxT("xanoubis"));
	/*
	 * Make sure that a ProcCtrl Instance exists before we start any
	 * helper threads.
	 */
	(void)ProcCtrl::instance();

	/* Initialize helper class. */
	(void)MainUtils::instance();

	hasLocale = language_.Init(wxLANGUAGE_DEFAULT, wxLOCALE_CONV_ENCODING);
	if (hasLocale) {
		language_.AddCatalog(wxT("wxstd"));
		language_.AddCatalog(wxT(PACKAGE_NAME));
	}

	/*
	 * Initilize config dirctory and check version of the config
	 * directory.
	 */
	if (anoubis_ui_init() < 0) {
		anMessageBox(_("Error while initialising anoubis_ui"),
		    _("Error"), wxOK | wxICON_ERROR, mainFrame);
	}
	versionError = anoubis_ui_readversion();
	if (versionError > ANOUBIS_UI_VER) {
		wxString msg = wxString::Format(_("Unsupported version (%d) "
		    "found of HOME/.xanoubis\nXanoubis will terminate now.\n"),
		    versionError);
		anMessageBox(msg, _("Warning"), wxOK | wxICON_WARNING,
		    mainFrame);
		return (false);
	}

	/* Initialization of versionmanagement */
	VersionCtrl::instance(); /* Make sure c'tor is invoked */

	/* Initialize policy-controller */
	PolicyCtrl::instance();

	/* Assign the passphrase-callback */
	KeyCtrl::instance()->setPassphraseReader(this);

	/* Initialization of central event management */
	AnEvents::instance();

	/* Initialization of job-controller */
	JobCtrl *jobCtrl = JobCtrl::instance();

	jobCtrl->start();

	AnConfig *config = new AnConfig(GetAppName());
	wxConfig::Set(config);

	mainFrame = new MainFrame((wxWindow*)NULL, trayVisible_);
	SetTopWindow(mainFrame);
	mainFrame->OnInit();

	/* Create and initialize modules. */
	MainUtils::instance()->initModules(mainFrame);

	if (hasLocale) {
		mainFrame->SetStatusText(_("Language setting: ") +
		    language_.GetCanonicalName(), 0);
	}

	/* Show the window only after it is completely constructed. */
	if (!hide_)
		mainFrame->Show();

	return (true);
}

int
AnoubisGuiApp::OnExit(void)
{
	JobCtrl::instance()->stop();
	/* No need to delete singletons. */

	return wxApp::OnExit();
}

void
AnoubisGuiApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	static const wxCmdLineEntryDesc cmdLineDescription[] = {
		{
			wxCMD_LINE_SWITCH,
			wxT("h"),
			wxT("help"),
			_("this help"),
			wxCMD_LINE_VAL_NONE,
			wxCMD_LINE_OPTION_HELP
		}, {
			wxCMD_LINE_OPTION,
			wxT("s"),
			wxT("socket"),
			_("communication socket of anoubis daemon"),
			wxCMD_LINE_VAL_STRING,
			wxCMD_LINE_PARAM_OPTIONAL
		}, {
			wxCMD_LINE_OPTION,
			wxT("d"),
			wxT("debug"),
			_("Debug output with level of \'num\'"),
			wxCMD_LINE_VAL_NUMBER,
			wxCMD_LINE_PARAM_OPTIONAL
		}, {
			wxCMD_LINE_SWITCH,
			wxT("t"),
			wxT("tray"),
			_("Disable TrayIcon"),
			wxCMD_LINE_VAL_NONE,
			wxCMD_LINE_PARAM_OPTIONAL
		}, {
			wxCMD_LINE_SWITCH,
			wxT("H"),
			wxT("hide"),
			_("Do not show the main window"),
			wxCMD_LINE_VAL_NONE,
			wxCMD_LINE_PARAM_OPTIONAL
		}, {
			wxCMD_LINE_NONE,
			NULL,
			NULL,
			NULL,
			wxCMD_LINE_VAL_NONE,
			0
		}
	};

	parser.SetDesc(cmdLineDescription);
	parser.SetSwitchChars(wxT("-"));
}

bool
AnoubisGuiApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	long int	debug_level = 0;
	wxString	socketPath;

	if (parser.Found(wxT("s"), &socketPath))
		JobCtrl::instance()->setSocketPath(socketPath);

	if (parser.Found(wxT("d"), &debug_level)) {
		Debug::setLevel(debug_level);
		Debug::trace(wxT("Debug enabled with level %ld"),
		    Debug::getLevel());
	}

	if (parser.Found(wxT("t")))
		trayVisible_ = false;
	if (parser.Found(wxT("H")))
		hide_ = true;

	return (true);
}

wxString
AnoubisGuiApp::readPassphrase(bool *ok)
{
	wxString dName = wxEmptyString;
	wxString msg = wxEmptyString;
	wxString title = wxEmptyString;

	assert(wxIsMainThread());

	wxWindow *w = wxWindow::FindWindowById(ID_MAINFRAME);

	LocalCertificate &cert = KeyCtrl::instance()->getLocalCertificate();
	PrivKey &pKey = KeyCtrl::instance()->getPrivateKey();

	dName = cert.getDistinguishedName();

	if (pKey.getTries() < 1) {
		msg = wxString::Format(
		    _("Enter the passphrase of your private key:\n%ls"),
		    dName.c_str());
		title = _("Enter passphrase for: ");
	} else {
		msg = wxString::Format(
		    _("The entered passphrase was wrong!\n"
		    "Please enter the passphrase for:\n%ls"), dName.c_str());
		title = _("Enter passphrase for: ");
	}

	wxPasswordEntryDialog *dlg = new wxPasswordEntryDialog(w, msg, title);
	dlg->CentreOnScreen();

	wxString result;
	if (dlg->ShowModal() == wxID_OK) {
		*ok = true;
		result = dlg->GetValue();
	} else {
		*ok = false;
		result = wxEmptyString;
	}

	dlg->Destroy();

	return (result);
}

#ifndef USE_WXGUITESTING
int
main(int argc, char **argv)
{
	if (setresgid(getgid(), getgid(), getgid()) < 0) {
		perror("setresgid");
		/* Try it anyway, but GTK will complain! */
	}
	return wxEntry(argc, argv);
}
#endif /* USE_WXGUITESTING */

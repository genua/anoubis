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
#include <pwd.h>

#include <wx/cmdline.h>
#include <wx/fileconf.h>
#include <wx/icon.h>
#include <wx/msgdlg.h> /* XXX Used by bad AnoubisGuiApp::OnPolicySend */
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/textdlg.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>

#include "AlertNotify.h"
#include "AnEvents.h"
#include "AnIconList.h"
#include "DlgLogViewer.h"
#include "DlgRuleEditor.h"
#include "JobCtrl.h"
#include "LogNotify.h"
#include "MainFrame.h"
#include "main.h"
#include "ModAlf.h"
#include "ModAnoubis.h"
#include "ModOverview.h"
#include "ModSb.h"
#include "ModSfs.h"
#include "Module.h"
#include "PolicyRuleSet.h"
#include "TrayIcon.h"
#include "VersionCtrl.h"

IMPLEMENT_APP(AnoubisGuiApp)

AnoubisGuiApp::AnoubisGuiApp(void)
{
	mainFrame = NULL;
	logViewer_ = NULL;
	ruleEditor_ = NULL;
	trayIcon = NULL;
	userOptions_ = NULL;
	onInitProfile_ = true;
	trayVisible_ = true;

	SetAppName(wxT("xanoubis"));
	wxInitAllImageHandlers();

	paths_.SetInstallPrefix(wxT(PACKAGE_PREFIX));
}

AnoubisGuiApp::~AnoubisGuiApp(void)
{
	/* mainFrame not handled here, because object already destroyed */
	if (trayIcon != NULL)
		delete trayIcon;

	if (userOptions_ != NULL) {
		delete userOptions_;
	}

	/* Destroy versionmanagement */
	delete VersionCtrl::getInstance();

	delete PolicyCtrl::getInstance();
	delete AnIconList::getInstance();
}

void
AnoubisGuiApp::quit(void)
{
	bool result = mainFrame->OnQuit();

	if(result) {
		if (trayIcon != NULL)
			trayIcon->RemoveIcon();
		mainFrame->Destroy();
		delete logViewer_;
		delete ruleEditor_;
	}
}

bool AnoubisGuiApp::OnInit()
{
	bool hasLocale;

	if (!wxApp::OnInit())
		return false;

	hasLocale = language_.Init(wxLANGUAGE_DEFAULT, wxLOCALE_CONV_ENCODING);
	if (hasLocale) {
		language_.AddCatalogLookupPathPrefix(getCatalogPath());
		language_.AddCatalog(GetAppName());
	}

	if (!wxDirExists(paths_.GetUserDataDir())) {
		wxMkdir(paths_.GetUserDataDir());
	}

	/* Initialization of versionmanagement */
	VersionCtrl::getInstance(); /* Make sure c'tor is invoked */

	/* Assign the passphrase-callback */
	KeyCtrl::getInstance()->setPassphraseReader(this);

	/* Initialization of central event management */
	AnEvents *anEvents = AnEvents::getInstance();

	/* Initialization of job-controller */
	JobCtrl *jobCtrl = JobCtrl::getInstance();

	jobCtrl->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(AnoubisGuiApp::OnConnectionStateChange),
	    NULL, this);
	jobCtrl->Connect(anTASKEVT_REGISTER,
	    wxTaskEventHandler(AnoubisGuiApp::OnDaemonRegistration),
	    NULL, this);

	anEvents->Connect(anEVT_ANSWER_ESCALATION,
	    wxCommandEventHandler(AnoubisGuiApp::OnAnswerEscalation),
	    NULL, this);

	jobCtrl->start();

	userOptions_ = new wxFileConfig(GetAppName(), wxEmptyString,
	    wxEmptyString, wxEmptyString,
	    wxCONFIG_USE_SUBDIR | wxCONFIG_USE_LOCAL_FILE);
	mainFrame = new MainFrame((wxWindow*)NULL);
	logViewer_ = new DlgLogViewer(mainFrame);
	ruleEditor_ = new DlgRuleEditor(mainFrame);

	SetTopWindow(mainFrame);
	mainFrame->OnInit();
	mainFrame->Show();

	modules_[OVERVIEW] = new ModOverview(mainFrame);
	modules_[ALF]      = new ModAlf(mainFrame);
	modules_[SFS]      = new ModSfs(mainFrame);
	modules_[SB]      = new ModSb(mainFrame);
	modules_[ANOUBIS]  = new ModAnoubis(mainFrame);

	((ModOverview*)modules_[OVERVIEW])->addModules(modules_);
	mainFrame->addModules(modules_);

	/* XXX [ST]: BUG #424
	 * The following should be considered as a hack to update the state of
	 * Module ALF by calling the update()-method.
	 * Eventually the actual call has to be triggered by an event.
	 */
	((ModAlf*)modules_[ALF])->update();
	((ModSfs*)modules_[SFS])->update();
	((ModSfs*)modules_[SB])->update();
	((ModAnoubis*)modules_[ANOUBIS])->update();

	if (hasLocale) {
		status(_("Language setting: ") + language_.GetCanonicalName());
	}

	/*
	 * Do this last. This increases the probability that the system tray
	 * is already up and running.
	 */
	if (trayVisible_)
		trayIcon = new TrayIcon();

	return (true);
}

int
AnoubisGuiApp::OnExit(void)
{
	int result = wxApp::OnExit();

	/* Destroy job-controller */
	JobCtrl *jobCtrl = JobCtrl::getInstance();
	jobCtrl->stop();
	delete jobCtrl;

	return (result);
}


void
AnoubisGuiApp::status(wxString msg)
{
	mainFrame->SetStatusText(msg, 0);
	Debug::instance()->log(msg, DEBUG_STATUS);
}

void
AnoubisGuiApp::log(wxString msg)
{
	LogNotify	*notify;
	wxCommandEvent	 event(anEVT_ADD_NOTIFICATION);

	notify = new LogNotify(msg);
	Debug::instance()->log(msg, DEBUG_LOG);

	event.SetClientObject((wxClientData*)notify);
	wxPostEvent(AnEvents::getInstance(), event);
	/*
	 * The created AlertNotify object must not been destroyed here, so
	 * it's still alive for the receiver. The notify will be stored by
	 * ModAnoubis and been deleted by it.
	 */

}

void
AnoubisGuiApp::alert(wxString msg)
{
	AlertNotify	*notify;
	wxCommandEvent	 event(anEVT_ADD_NOTIFICATION);

	notify = new AlertNotify(msg);
	Debug::instance()->log(msg, DEBUG_ALERT);

	event.SetClientObject((wxClientData*)notify);
	wxPostEvent(AnEvents::getInstance(), event);
	/*
	 * The created AlertNotify object must not been destroyed here, so
	 * it's still alive for the receiver. The notify will be stored by
	 * ModAnoubis and been deleted by it.
	 */
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
			_("Debug output with \'num\' als level"),
			wxCMD_LINE_VAL_NUMBER,
			wxCMD_LINE_PARAM_OPTIONAL
		}, {
			wxCMD_LINE_OPTION,
			wxT("t"),
			wxT("tray"),
			_("Disable TrayIcon"),
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
	wxString	socketPath, mesg;

	if (parser.Found(wxT("s"), &socketPath))
		JobCtrl::getInstance()->setSocketPath(socketPath);

	if (parser.Found(wxT("d"), &debug_level)) {
		Debug::instance()->setLevel(debug_level);
		mesg = wxT("Debug enabled with level ");
		mesg += wxString::Format(_T("%ld"), debug_level);
		Debug::instance()->log(mesg, DEBUG_ALERT);
	}

	if (parser.Found(wxT("t")))
		trayVisible_ = false;

	return (true);
}

void
AnoubisGuiApp::toggleRuleEditorVisability(void)
{
	wxCommandEvent  showEvent(anEVT_RULEEDITOR_SHOW);
	showEvent.SetInt(!ruleEditor_->IsShown());
	wxPostEvent(AnEvents::getInstance(), showEvent);
}

void
AnoubisGuiApp::toggleLogViewerVisability(void)
{
	wxCommandEvent  showEvent(anEVT_LOGVIEWER_SHOW);
	showEvent.SetInt(!logViewer_->IsShown());
	wxPostEvent(AnEvents::getInstance(), showEvent);
}

bool
AnoubisGuiApp::connectCommunicator(bool doConnect)
{
	if (doConnect == getCommConnectionState()) {
		/* No change of state */
		return (false);
	}

	if (doConnect) {
		/* Start with establishing the connection */
		return (JobCtrl::getInstance()->connect());
	} else {
		/* Start with unregistration */
		regTask_.setAction(ComRegistrationTask::ACTION_UNREGISTER);
		JobCtrl::getInstance()->addTask(&regTask_);

		return (true);
	}
}

void
AnoubisGuiApp::autoStart(bool autostart)
{
	wxString deskFile = wxStandardPaths::Get().GetDataDir() +
	    wxT("/xanoubis.desktop");
	wxString kPath = paths_.GetUserConfigDir() + wxT("/.kde/Autostart");
	wxString gPath = paths_.GetUserConfigDir() + wxT("/.config/autostart");
	wxString kAutoFile = kPath + wxT("/xanoubis.desktop");
	wxString gAutoFile = gPath + wxT("/xanoubis.desktop");

	if (autostart == true) {
		if (wxDirExists(kPath) == false) {
			wxFileName::Mkdir(kPath, 0777, wxPATH_MKDIR_FULL);
		}
		if (wxDirExists(gPath) == false) {
			wxFileName::Mkdir(gPath, 0777, wxPATH_MKDIR_FULL);
		}

		if (wxFileExists(kAutoFile) == false) {
			wxCopyFile(deskFile, kAutoFile);
		}
		if (wxFileExists(gAutoFile) == false) {
			wxCopyFile(deskFile, gAutoFile);
		}
	} else {
		if (wxFileExists(kAutoFile) == true) {
			if (wxRemove(kAutoFile) != 0) {
				wxString msg = wxString::Format(_
				    ("Couldn't remove Autostart file: %ls"),
				    kAutoFile.c_str());
				wxMessageBox(msg, _("Error"), wxICON_ERROR);
			}
		}
		if (wxFileExists(gAutoFile) == true) {
			if (wxRemove(gAutoFile) != 0) {
				wxString msg = wxString::Format(_
				    ("Couldn't remove Autostart file: %ls"),
				    gAutoFile.c_str());
				wxMessageBox(msg, _("Error"), wxICON_ERROR);
			}
		}
	}
}

void
AnoubisGuiApp::sendChecksum(const wxString &File)
{
	csumAddTask_.setPath(File);
	JobCtrl::getInstance()->addTask(&csumAddTask_);
}

void
AnoubisGuiApp::getChecksum(const wxString &File)
{
	csumGetTask_.setPath(File);
	JobCtrl::getInstance()->addTask(&csumGetTask_);
}

void
AnoubisGuiApp::calChecksum(const wxString &File)
{
	csumCalcTask_.setPath(File);
	JobCtrl::getInstance()->addTask(&csumCalcTask_);
}

wxString
AnoubisGuiApp::getCatalogPath(void)
{
	wxString catalogFileName;

	catalogFileName = paths_.GetDataDir() + wxT("/catalogs/");
	if (!::wxFileExists(catalogFileName)) {
		/*
		 * We didn't find our catalog (where --prefix told us)!
		 * Try to take executable path into account. This should
		 * fix a missing --prefix as the matter in our build and test
		 * environment with aegis.
		 */
		catalogFileName  = ::wxPathOnly(paths_.GetExecutablePath()) +
		    wxT("/../../..") + catalogFileName;
	}

	return catalogFileName;
}

wxString
AnoubisGuiApp::getWizardPath(void)
{
	wxString wizardFileName;

	wizardFileName = paths_.GetDataDir() + wxT("/profiles/wizard/");
	if (!::wxFileExists(wizardFileName)) {
		/*
		 * We didn't find our wizard (where --prefix told us)!
		 * Try to take executable path into account. This should
		 * fix a missing --prefix as the matter in our build and test
		 * environment with aegis.
		 */
		wizardFileName  = ::wxPathOnly(paths_.GetExecutablePath()) +
		    wxT("/../../..") + wizardFileName;
	}

	return wizardFileName;
}

wxString
AnoubisGuiApp::getIconPath(wxString iconName)
{
	wxString iconFileName;

	iconFileName = paths_.GetDataDir() + wxT("/icons/") + iconName;
	if (!::wxFileExists(iconFileName)) {
		/*
		 * We didn't find our icon (where --prefix told us)!
		 * Try to take executable path into account. This should
		 * fix a missing --prefix as the matter in our build and test
		 * environment with aegis.
		 */
		iconFileName  = ::wxPathOnly(paths_.GetExecutablePath()) +
		    wxT("/../../..") + iconFileName;
	}
	/*
	 * XXX: by ch: No error check is done, 'cause wxIcon will open a error
	 * dialog, complaining about missing icons itself. But maybe a logging
	 * message should be generated when logging will been implemented.
	 */
	return iconFileName;
}

wxString
AnoubisGuiApp::getRulesetPath(const wxString &profile, bool resolve)
{
	wxString fileName = paths_.GetUserDataDir() + wxT("/") + profile;

	if (!resolve)
		return (fileName);

	if (!wxFileExists(fileName)) {
		fileName = paths_.GetDataDir();
		fileName += wxT("/profiles/") + profile;
	}

	if (!wxFileExists(fileName)) {
		/*
		 * We didn't find our icon (where --prefix told us)!
		 * Try to take executable path into account. This should
		 * fix a missing --prefix as the matter in our build and test
		 * environment with aegis.
		 */
		fileName = wxPathOnly(paths_.GetExecutablePath()) +
		    wxT("/../../..") + fileName;
	}

	return (fileName);
}

wxIcon *
AnoubisGuiApp::loadIcon(wxString iconName)
{
	return (new wxIcon(getIconPath(iconName), wxBITMAP_TYPE_PNG));
}


Module *
AnoubisGuiApp::getModule(enum moduleIdx idx)
{
	return (modules_[idx]);
}

wxString
AnoubisGuiApp::getDataDir(void)
{
	return (paths_.GetUserConfigDir());
}

bool
AnoubisGuiApp::getCommConnectionState(void)
{
	return (JobCtrl::getInstance()->isConnected());
}

wxConfig *
AnoubisGuiApp::getUserOptions(void)
{
	return (userOptions_);
}

bool
AnoubisGuiApp::showingMainFrame(void)
{
	return (mainFrame->isShowing());
}

uid_t
AnoubisGuiApp::getUserIdByName(wxString name) const
{
	struct passwd	*pwd;

	pwd = getpwnam(name.fn_str());
	if (pwd) {
		return pwd->pw_uid;
	} else {
		return (uid_t)-1;
	}
}

/*
 * This function caches the last lookup to speed things up for cases
 * like the rule editor where we call this functions for each application
 * block.
 */
wxString
AnoubisGuiApp::getUserNameById(uid_t uid) const
{
	static int	 lastuid = 1;
	static wxString	 lastname = wxEmptyString;
	struct passwd	*pwd;

	if (lastuid < 0 || uid != (uid_t)lastuid) {
		pwd = getpwuid(uid);
		if (pwd && pwd->pw_name) {
			lastuid = uid;
			lastname = wxString::From8BitData(pwd->pw_name,
			    strlen(pwd->pw_name));
		} else {
			lastuid = -1;
			lastname = wxEmptyString;
		}
	}
	return lastname;
}

void
AnoubisGuiApp::OnConnectionStateChange(wxCommandEvent &event)
{
	JobCtrl::ConnectionState newState =
	    (JobCtrl::ConnectionState)event.GetInt();

	switch (newState) {
	case JobCtrl::CONNECTION_CONNECTED:
		/* Connection established, next make registration */
		regTask_.setAction(ComRegistrationTask::ACTION_REGISTER);
		JobCtrl::getInstance()->addTask(&regTask_);
		break;
	case JobCtrl::CONNECTION_DISCONNECTED:
		/* Nothing to do, already disconnected */
		break;
	case JobCtrl::CONNECTION_FAILED:
	case JobCtrl::CONNECTION_ERROR:
		/* Force a disconnect without deregistration */
		JobCtrl::getInstance()->disconnect();
		break;
	}

	event.Skip();
}

void
AnoubisGuiApp::OnDaemonRegistration(TaskEvent &event)
{
	ComRegistrationTask *task =
	    dynamic_cast<ComRegistrationTask*>(event.getTask());

	if (task == 0)
		return;

	if (task->getAction() == ComRegistrationTask::ACTION_REGISTER) {
		if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
			/* Registration failed, disconnect again */
			JobCtrl::getInstance()->disconnect();
		}
	} else { /* ACTION_UNREGISTER */
		/* Disconnect independent from unregistration-result */
		JobCtrl::getInstance()->disconnect();
	}

	event.Skip();
}

void
AnoubisGuiApp::OnAnswerEscalation(wxCommandEvent &event)
{
	Notification *notify = (Notification*)event.GetClientObject();
	JobCtrl::getInstance()->answerNotification(notify);
	event.Skip();
}

wxString
AnoubisGuiApp::readPassphrase(bool *ok)
{
	wxWindow *w = wxWindow::FindFocus();
	wxString dName = wxEmptyString;

	if (w == 0)
		w = GetTopWindow();

	LocalCertificate &cert = KeyCtrl::getInstance()->getLocalCertificate();
	dName = cert.getDistinguishedName();

	wxPasswordEntryDialog dlg(w,
	    wxString::Format(
	    _("Enter the passphrase of your private key:\n%ls"), dName.c_str()),
	    _("Enter passphrase for: "));
	dlg.CentreOnScreen();

	if (dlg.ShowModal() == wxID_OK) {
		*ok = true;
		return (dlg.GetValue());
	} else {
		*ok = false;
		return (wxEmptyString);
	}
}

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

#include <wx/cmdline.h>
#include <wx/fileconf.h>
#include <wx/icon.h>
#include <wx/stdpaths.h>
#include <wx/string.h>

#include "AnEvents.h"
#include "AlertNotify.h"
#include "CommunicatorCtrl.h"
#include "DlgLogViewer.h"
#include "DlgRuleEditor.h"
#include "LogNotify.h"
#include "main.h"
#include "MainFrame.h"
#include "Module.h"
#include "ModOverview.h"
#include "ModAlf.h"
#include "ModSfs.h"
#include "ModAnoubis.h"
#include "TrayIcon.h"
#include "PolicyRuleSet.h"

IMPLEMENT_APP(AnoubisGuiApp)

AnoubisGuiApp::AnoubisGuiApp(void)
{
	ruleSet_ = NULL;
	mainFrame = NULL;
	logViewer_ = NULL;
	ruleEditor_ = NULL;
	comCtrl_ = NULL;
	trayIcon = NULL;
	userOptions_ = NULL;

	SetAppName(wxT("xanoubis"));
	wxInitAllImageHandlers();

	paths_.SetInstallPrefix(wxT(GENERALPREFIX));
}

AnoubisGuiApp::~AnoubisGuiApp(void)
{
	/* mainFrame not handled here, coz object already destroyed */
	if (trayIcon != NULL)
		delete trayIcon;

	if (ruleSet_ != NULL) {
		delete ruleSet_;
	}

	if (userOptions_ != NULL) {
		delete userOptions_;
	}
}

void
AnoubisGuiApp::quit(void)
{
	bool result = mainFrame->OnQuit();

	userOptions_->Write(wxT("Anoubis/Profile"), profile_);

	if(result) {
		trayIcon->RemoveIcon();
		mainFrame->Destroy();
		delete logViewer_;
		delete ruleEditor_;
		delete comCtrl_;
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
	userOptions_ = new wxFileConfig(GetAppName(), wxEmptyString,
	    wxEmptyString, wxEmptyString,
	    wxCONFIG_USE_SUBDIR | wxCONFIG_USE_LOCAL_FILE);
	mainFrame = new MainFrame((wxWindow*)NULL);
	logViewer_ = new DlgLogViewer(mainFrame);
	ruleEditor_ = new DlgRuleEditor(mainFrame);
	comCtrl_ = new CommunicatorCtrl(socketParam_);
	trayIcon = new TrayIcon();

	modules_[OVERVIEW] = new ModOverview(mainFrame);
	modules_[ALF]      = new ModAlf(mainFrame);
	modules_[SFS]      = new ModSfs(mainFrame);
	modules_[ANOUBIS]  = new ModAnoubis(mainFrame);

	mainFrame->Show();
	SetTopWindow(mainFrame);
	mainFrame->OnInit();

	((ModOverview*)modules_[OVERVIEW])->addModules(modules_);
	mainFrame->addModules(modules_);

	/* XXX [ST]: BUG #424
	 * The following should be considered as a hack to update the state of
	 * Module ALF by calling the update()-method.
	 * Eventually the actual call has to be triggered by an event.
	 */
	((ModAlf*)modules_[ALF])->update();
	((ModSfs*)modules_[SFS])->update();
	((ModAnoubis*)modules_[ANOUBIS])->update();

	if (hasLocale) {
		status(_("Language setting: ") + language_.GetCanonicalName());
	}

	if (!userOptions_->Read(wxT("Anoubis/Profile"), &profile_)) {
		profile_ = wxT("admin");
		profileFromDiskToDaemon(profile_);
	}
	((ModAnoubis*)modules_[ANOUBIS])->setProfile(profile_);

	return (true);
}

void
AnoubisGuiApp::sendEvent(wxCommandEvent& event)
{
	/* XXX [ST]: BUG #424
	 * The following ``wxPostEvent'' is not needed therefore comment it out.
	 * Nethertheless we have to cleanup and normalise the concept of
	 * events within the GUI.
	 *
	 * wxPostEvent(mainFrame, event);
	 *
	 * XXX [KM] : BUG 522
	 * If "logViewer" and "ruleEditor" get a "wxPostEvent" a Event-Handler
	 * from "mainFrame" and the child-Windows gets two event messages per
	 * "sendEvent". Therefore I brought back the wxPostEvent(mainFrame,
	 * event);, commented out the two below and connecte "logViewer" and
	 * "ruleEditor" over their "parent" Window.
	 *
	 * wxPostEvent(logViewer_, event);
	 * wxPostEvent(ruleEditor_, event);
	 */

	wxPostEvent(mainFrame, event);
	wxPostEvent(comCtrl_, event);
	wxPostEvent(trayIcon, event);
}

void
AnoubisGuiApp::sendEvent(wxEventType type)
{
	wxCommandEvent anEvent(type);
	anEvent.SetInt(true);
	sendEvent(anEvent);
}

void
AnoubisGuiApp::status(wxString msg)
{
	mainFrame->SetStatusText(msg, 0);
}

void
AnoubisGuiApp::log(wxString msg)
{
	LogNotify	*notify;
	wxCommandEvent	 event(anEVT_ADD_NOTIFICATION);

	notify = new LogNotify(msg);
	event.SetClientObject((wxClientData*)notify);
	sendEvent(event);
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
	event.SetClientObject((wxClientData*)notify);
	sendEvent(event);
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
			wxCMD_LINE_NONE
		}
	};

	parser.SetDesc(cmdLineDescription);
	parser.SetSwitchChars(wxT("-"));
}

bool
AnoubisGuiApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	if (!parser.Found(wxT("s"), &socketParam_)) {
		socketParam_ = wxT(ANOUBISD_SOCKETNAME);
	}

	return (true);
}

void
AnoubisGuiApp::toggleRuleEditorVisability(void)
{
	wxCommandEvent  showEvent(anEVT_RULEEDITOR_SHOW);
	showEvent.SetInt(!ruleEditor_->IsShown());
	wxGetApp().sendEvent(showEvent);
}

void
AnoubisGuiApp::toggleLogViewerVisability(void)
{
	wxCommandEvent  showEvent(anEVT_LOGVIEWER_SHOW);
	showEvent.SetInt(!logViewer_->IsShown());
	wxGetApp().sendEvent(showEvent);
}

void
AnoubisGuiApp::connectCommunicator(bool doConnect)
{
	if (doConnect) {
		comCtrl_->connect();
	} else {
		comCtrl_->disconnect();
	}
}

void
AnoubisGuiApp::requestPolicy(void)
{
	comCtrl_->requestPolicy();
}

void
AnoubisGuiApp::usePolicy(wxString tmpFile)
{
	comCtrl_->usePolicy(tmpFile);
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
AnoubisGuiApp::getUtilsPath(void)
{
	wxString utilsFileName;

	utilsFileName = paths_.GetDataDir() + wxT("/utils/");
	if (!::wxFileExists(utilsFileName)) {
		/*
		 * We didn't find our utils (where --prefix told us)!
		 * Try to take executable path into account. This should
		 * fix a missing --prefix as the matter in our build and test
		 * environment with aegis.
		 */
		utilsFileName  = ::wxPathOnly(paths_.GetExecutablePath()) +
		    wxT("/../../..") + utilsFileName;
	}

	return utilsFileName;
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
	return (comCtrl_->isConnected());
}

void
AnoubisGuiApp::importPolicyRuleSet(struct apn_ruleset *rule)
{
	wxCommandEvent		 event(anEVT_LOAD_RULESET);

	if (ruleSet_ != NULL) {
		delete ruleSet_;
	}
	ruleSet_ = new PolicyRuleSet(rule);

	event.SetClientData((void*)ruleSet_);
	sendEvent(event);
}

void
AnoubisGuiApp::importPolicyFile(wxString fileName)
{
	wxCommandEvent		 event(anEVT_LOAD_RULESET);

	if (ruleSet_ != NULL) {
		delete ruleSet_;
	}
	ruleSet_ = new PolicyRuleSet(fileName);

	event.SetClientData((void*)ruleSet_);
	sendEvent(event);
}

void
AnoubisGuiApp::exportPolicyFile(wxString fileName)
{
	if (ruleSet_ != NULL) {
		ruleSet_->exportToFile(fileName);
	} else {
		status(_("no loaded RuleSet; export failed!"));
	}
}

wxConfig *
AnoubisGuiApp::getUserOptions(void)
{
	return (userOptions_);
}

bool
AnoubisGuiApp::profileFromDiskToDaemon(wxString profileName)
{
	wxString	fileName;
	wxString	logMsg;

	if ((comCtrl_ == NULL) || !comCtrl_->isConnected()) {
		status(_("Error: xanoubis is not connected to the daemon"));
		return (false);
	}

	logMsg = _("seek for profile ") + profileName + _(" at ");
	fileName = paths_.GetUserDataDir() + wxT("/") + profileName;
	log(logMsg + fileName);
	if (!wxFileExists(fileName)) {
		fileName = paths_.GetDataDir();
		fileName += wxT("/profiles/") + profileName;
		log(logMsg + fileName);
	}
	if (!wxFileExists(fileName)) {
		/*
		 * We didn't find our icon (where --prefix told us)!
		 * Try to take executable path into account. This should
		 * fix a missing --prefix as the matter in our build and test
		 * environment with aegis.
		 */
		fileName  = wxPathOnly(paths_.GetExecutablePath()) +
		    wxT("/../../..") + fileName;
		log(logMsg + fileName);
	}
	if (!wxFileExists(fileName)) {
		status(_("Error: could not find profile: ") + profileName);
		return (false);
	}

	profile_ = profileName;
	comCtrl_->usePolicy(fileName);
	importPolicyFile(fileName);

	return (true);
}

bool
AnoubisGuiApp::profileFromDaemonToDisk(wxString profileName)
{
	wxString	fileName;

	if ((comCtrl_ == NULL) || !comCtrl_->isConnected()) {
		status(_("Error: xanoubis is not connected to the daemon"));
		return (false);
	}

	if (ruleSet_ == NULL) {
		return (false);
	}

	fileName = paths_.GetUserDataDir() + wxT("/") + profileName;
	if (wxFileExists(fileName)) {
		wxRemoveFile(fileName);
	}
	exportPolicyFile(fileName);

	return (true);
}

wxString
AnoubisGuiApp::getCurrentProfileName(void)
{
	return (profile_);
}

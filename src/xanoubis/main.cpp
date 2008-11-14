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
#include <wx/msgdlg.h> /* XXX Used by bad AnoubisGuiApp::OnPolicySend */
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>

#include "AlertNotify.h"
#include "AnEvents.h"
#include "DlgLogViewer.h"
#include "DlgRuleEditor.h"
#include "JobCtrl.h"
#include "LogNotify.h"
#include "MainFrame.h"
#include "main.h"
#include "ModAlf.h"
#include "ModAnoubis.h"
#include "ModOverview.h"
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

	SetAppName(wxT("xanoubis"));
	wxInitAllImageHandlers();

	paths_.SetInstallPrefix(wxT(GENERALPREFIX));
	fillUserList();
}

AnoubisGuiApp::~AnoubisGuiApp(void)
{
	/* mainFrame not handled here, coz object already destroyed */
	if (trayIcon != NULL)
		delete trayIcon;

	if (userOptions_ != NULL) {
		delete userOptions_;
	}

	/* Destroy versionmanagement */
	delete VersionCtrl::getInstance();

	delete ProfileCtrl::getInstance();
	userList_.clear();
}

void
AnoubisGuiApp::quit(void)
{
	bool result = mainFrame->OnQuit();

	userOptions_->Write(wxT("Anoubis/Profile"),
	    ProfileCtrl::getInstance()->getProfile());

	if(result) {
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

	/* Initialization of job-controller */
	JobCtrl *jobCtrl = JobCtrl::getInstance();

	jobCtrl->Connect(anTASKEVT_REGISTER,
	    wxTaskEventHandler(AnoubisGuiApp::OnDaemonRegistration),
	    NULL, this);
	jobCtrl->Connect(anTASKEVT_POLICY_REQUEST,
	    wxTaskEventHandler(AnoubisGuiApp::OnPolicyRequest), NULL, this);
	jobCtrl->Connect(anTASKEVT_POLICY_SEND,
	    wxTaskEventHandler(AnoubisGuiApp::OnPolicySend), NULL, this);

	Connect(anEVT_ANSWER_ESCALATION,
	    wxCommandEventHandler(AnoubisGuiApp::OnAnswerEscalation),
	    NULL, this);

	jobCtrl->start();

	userOptions_ = new wxFileConfig(GetAppName(), wxEmptyString,
	    wxEmptyString, wxEmptyString,
	    wxCONFIG_USE_SUBDIR | wxCONFIG_USE_LOCAL_FILE);
	mainFrame = new MainFrame((wxWindow*)NULL);
	logViewer_ = new DlgLogViewer(mainFrame);
	ruleEditor_ = new DlgRuleEditor(mainFrame);
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

	ProfileCtrl::getInstance()->switchProfile(ProfileMgr::PROFILE_MEDIUM);
	((ModAnoubis*)modules_[ANOUBIS])->setProfile(
	    ProfileCtrl::getInstance()->getProfileName());

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
AnoubisGuiApp::sendEvent(wxCommandEvent& event)
{
	long		 id;
	ProfileCtrl	*profileCtrl;
	PolicyRuleSet	*rs;

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
	wxPostEvent(trayIcon, event);

	profileCtrl = ProfileCtrl::getInstance();
	id = profileCtrl->getUserId();
	if (id != -1) {
		if (profileCtrl->lockToShow(id, this)) {
			rs = profileCtrl->getRuleSetToShow(id, this);
			if (rs != NULL) {
				wxPostEvent(rs, event);
			}
			profileCtrl->unlockFromShow(id, this);
		}
	}
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
	Debug::instance()->log(msg, DEBUG_ALERT);

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
			wxCMD_LINE_OPTION,
			wxT("d"),
			wxT("debug"),
			_("Debug output with \'num\' als level"),
			wxCMD_LINE_VAL_NUMBER,
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

bool
AnoubisGuiApp::connectCommunicator(bool doConnect)
{
	if (doConnect == getCommConnectionState()) {
		/* No change of state */
		return (false);
	}

	if (doConnect) {
		/* Start with establishing the connection */
		JobCtrl *jobCtrl = JobCtrl::getInstance();
		JobCtrl::ConnectionState state = jobCtrl->connect();

		if (state == JobCtrl::CONNECTION_CONNECTED) {
			/* Next make registration */
			regTask_.setAction(
			    ComRegistrationTask::ACTION_REGISTER);
			jobCtrl->addTask(&regTask_);

			return (true);
		} else
			return (false);
	} else {
		/* Start with unregistration */
		regTask_.setAction(ComRegistrationTask::ACTION_UNREGISTER);
		JobCtrl::getInstance()->addTask(&regTask_);

		return (true);
	}
}

void
AnoubisGuiApp::requestPolicy(void)
{
	adminPolicyRequestTask_.setRequestParameter(0, geteuid());
	userPolicyRequestTask_.setRequestParameter(1, geteuid());

	JobCtrl::getInstance()->addTask(&adminPolicyRequestTask_);
	JobCtrl::getInstance()->addTask(&userPolicyRequestTask_);
}

void
AnoubisGuiApp::usePolicy(PolicyRuleSet *ruleset)
{
	policySendTask_.setPolicy(ruleset);
	JobCtrl::getInstance()->addTask(&policySendTask_);
}

void
AnoubisGuiApp::sendChecksum(const wxString &File)
{
	csumAddTask_.setFile(File);
	JobCtrl::getInstance()->addTask(&csumAddTask_);
}

void
AnoubisGuiApp::getChecksum(const wxString &File)
{
	csumGetTask_.setFile(File);
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

void
AnoubisGuiApp::importPolicyRuleSet(int prio, uid_t uid,
    struct apn_ruleset * rule)
{
	wxCommandEvent	 event(anEVT_LOAD_RULESET);
	PolicyRuleSet	*rs;
	ProfileCtrl	*profileCtrl;

	rs = new PolicyRuleSet(prio, uid, rule);
	profileCtrl = ProfileCtrl::getInstance();

	event.SetClientData((void*)rs);

	if (onInitProfile_) {
		ProfileMgr::profile_t	profile;
		long			rProfile;
		onInitProfile_ = false;
		if (!userOptions_->Read(wxT("Anoubis/Profile"), &rProfile)) {
			if (rs == NULL) {
				profile = ProfileMgr::PROFILE_NONE;
			} else if (rs->isEmpty()) {
				profile = ProfileMgr::PROFILE_ADMIN;
				profileCtrl->switchProfile(profile);
				profileFromDiskToDaemon(
				    profileCtrl->getProfileName());
			} else {
				profile = ProfileMgr::PROFILE_MEDIUM;
				profileCtrl->switchProfile(profile);
				profileFromDaemonToDisk(
				    profileCtrl->getProfileName());
			}
		} else {
			profile = (ProfileMgr::profile_t)rProfile;
			profileCtrl->switchProfile(profile);
		}
		((ModAnoubis*)modules_[ANOUBIS])->setProfile(
		    profileCtrl->getProfileName());
	}

	profileCtrl->store(rs);
	sendEvent(event);
}

void
AnoubisGuiApp::importPolicyFile(wxString fileName, bool checkPerm)
{
	wxCommandEvent		 event(anEVT_LOAD_RULESET);
	PolicyRuleSet		*ruleSet;

	ruleSet = new PolicyRuleSet(1, geteuid(), fileName, checkPerm);
	if (!ruleSet->hasErrors()) {
		ProfileCtrl::getInstance()->store(ruleSet);
		sendEvent(event);
	} else {
		wxMessageBox(
		    _("Couldn't import policy file: it contains errors."),
		    _("Error"), wxICON_ERROR);
	}
}

/* XXX ch: this code should be moved to ProfileCtrl. */
void
AnoubisGuiApp::exportPolicyFile(wxString fileName)
{
	long		 rsid;
	ProfileCtrl	*profileCtrl;
	PolicyRuleSet	*ruleSet;

	profileCtrl = ProfileCtrl::getInstance();
	rsid = profileCtrl->getUserId();
	if (rsid == -1) {
		status(_("no loaded RuleSet; export failed!"));
		return;
	}

	if (! profileCtrl->lockToShow(rsid, this)) {
		status(_("Couldn't access RuleSet; export failed!"));
		return;
	}

	ruleSet = profileCtrl->getRuleSetToShow(rsid, this);
	ruleSet->exportToFile(fileName);

	profileCtrl->unlockFromShow(rsid, this);
}

wxConfig *
AnoubisGuiApp::getUserOptions(void)
{
	return (userOptions_);
}

/* XXX ch: this code should be moved to ProfileCtrl. */
bool
AnoubisGuiApp::profileFromDiskToDaemon(const wxString &profileName)
{
	wxString	fileName;
	wxString	tmpFileName;
	wxString	logMsg;

	if (!JobCtrl::getInstance()->isConnected()) {
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
		 * We didn't find our profile (where --prefix told us)!
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

	/*
	 * Create a tmp file as a copy of the local profile,
	 * because usePolicy() will remove the given file!
	 */
	usePolicy(new PolicyRuleSet(1, geteuid(), fileName, false));
	importPolicyFile(fileName, false);

	return (true);
}

bool
AnoubisGuiApp::profileFromDaemonToDisk(const wxString &profileName)
{
	wxString	fileName;

	if (!JobCtrl::getInstance()->isConnected()) {
		status(_("Error: xanoubis is not connected to the daemon"));
		return (false);
	}

	if (ProfileCtrl::getInstance()->getUserId() == -1) {
		return (false);
	}

	fileName = paths_.GetUserDataDir() + wxT("/") + profileName;
	if (wxFileExists(fileName)) {
		wxRemoveFile(fileName);
	}
	exportPolicyFile(fileName);

	return (true);
}

bool
AnoubisGuiApp::showingMainFrame(void)
{
	return (mainFrame->isShowing());
}

void
AnoubisGuiApp::fillUserList(void)
{
	wxTextFile		pwdFile(wxT("/etc/passwd"));
	wxStringTokenizer	tokens;
	wxString		line;
	wxString		user;
	wxString		uid;

	if (geteuid() != 0) {
		uid = wxString::Format(wxT("%d"), geteuid());
		userList_[wxGetUserId()] = uid;
		return;
	}

	if (!pwdFile.Open()) {
		/* We couldn't open /etc/passwd !!! */
		return;
	}

	for (line=pwdFile.GetFirstLine();
	    !pwdFile.Eof();
	    line=pwdFile.GetNextLine()) {
		tokens.SetString(line, wxT(":"));
		user = tokens.GetNextToken();
		tokens.GetNextToken(); /* Just drop the 2nd token. */
		uid = tokens.GetNextToken();
		userList_[user] = uid;
	}
}

wxArrayString
AnoubisGuiApp::getListOfUsersName(void) const
{
	std::map<wxString,wxString>::const_iterator	it;
	wxArrayString					result;

	for (it=userList_.begin(); it!=userList_.end(); it++) {
		result.Add((*it).first);
	}

	return (result);
}

wxArrayString
AnoubisGuiApp::getListOfUsersId(void) const
{
	std::map<wxString,wxString>::const_iterator	it;
	wxArrayString					result;

	for (it=userList_.begin(); it!=userList_.end(); it++) {
		result.Add((*it).second);
	}

	return (result);
}

uid_t
AnoubisGuiApp::getUserIdByName(wxString name) const
{
	unsigned long uid;

	if (userList_.count(name) == 0) {
		uid = 0 - 1;
	} else {
		userList_.find(name)->second.ToULong(&uid);
	}

	return ((uid_t)uid);
}

wxString
AnoubisGuiApp::getUserNameById(uid_t uid) const
{
	std::map<wxString,wxString>::const_iterator	it;
	wxString					result;
	unsigned long					tmpUid;

	result = _("unknown");

	for (it=userList_.begin(); it!=userList_.end(); it++) {
		(*it).second.ToULong(&tmpUid);
		if (tmpUid == (unsigned long)uid) {
			result = (*it).first;
		}
	}

	return (result);
}

void
AnoubisGuiApp::OnDaemonRegistration(TaskEvent &event)
{
	ComRegistrationTask *task =
	    dynamic_cast<ComRegistrationTask*>(event.getTask());

	if (task == 0)
		return;

	if (task->getAction() == ComRegistrationTask::ACTION_REGISTER) {
		if (task->getComTaskResult() == ComTask::RESULT_SUCCESS) {
			/* No success load policies from daemon */
			wxGetApp().requestPolicy();
		} else {
			/* Registration failed, disconnect again */
			connectCommunicator(false);
		}
	} else { /* ACTION_UNREGISTER */
		/* Disconnect independent from unregistration-result */
		JobCtrl::getInstance()->disconnect();
	}

	event.Skip();
}

void
AnoubisGuiApp::OnPolicyRequest(TaskEvent &event)
{
	ComPolicyRequestTask *task =
	    dynamic_cast<ComPolicyRequestTask*>(event.getTask());

	if (task == 0)
		return;

	/* XXX Error-path? */
	importPolicyRuleSet(task->getPriority(), task->getUid(),
	    task->getPolicyApn());

	event.Skip();
}

void
AnoubisGuiApp::OnPolicySend(TaskEvent &event)
{
	ComPolicySendTask *task =
	    dynamic_cast<ComPolicySendTask*>(event.getTask());

	if (task == 0)
		return;

	/* XXX bad error-handling */
	if (task->getComTaskResult() != ComTask::RESULT_SUCCESS) {
		wxString message = _("Error while sending policy to daemon");
		wxString title = _("Error!");

		wxMessageBox(message, title, wxICON_ERROR);
	}
}

void
AnoubisGuiApp::OnAnswerEscalation(wxCommandEvent &event)
{
	Notification *notify = (Notification*)event.GetClientObject();
	JobCtrl::getInstance()->answerNotification(notify);
}

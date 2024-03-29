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
#include "version.h"

#ifdef HAVE_SYS_INOTIFY_H
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#endif

#include <wx/app.h>
#include <wx/filedlg.h>
#include <wx/aboutdlg.h>
#include <wx/gdicmn.h>

#include "AnEvents.h"
#include "AnMessageDialog.h"
#include "AnStatusBar.h"
#include "DlgBackupPolicyImpl.h"
#include "JobCtrl.h"
#include "MainFrame.h"
#include "MainFrameBase.h"
#include "MainUtils.h"
#include "ModAnoubis.h"
#include "ModAnoubisMainPanelImpl.h"
#include "ModPlayground.h"
#include "ModPlaygroundMainPanelImpl.h"
#include "ModSfs.h"
#include "Module.h"
#include "RuleWizard.h"
#include "anoubis_errno.h"
#include "main.h"

MainFrame::MainFrame(wxWindow *parent, bool trayVisible)
    : MainFrameBase(parent)
{
	AnEvents	*anEvents;
	JobCtrl		*jobCtrl;

	exit_ = false;

	messageAlertCount_ = 0;
	messageEscalationCount_ = 0;
	SetIcon(AnIconList::instance()->GetIcon(
	    AnIconList::ICON_ANOUBIS_BLACK_48));

	anEvents = AnEvents::instance();
	jobCtrl = JobCtrl::instance();

	jobCtrl->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(MainFrame::OnConnectionStateChange),
	    NULL, this);

	jobCtrl->Connect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(MainFrame::onSfsListArrived),
	    NULL, this);

	upgradeTask_ = NULL;
	logViewer_ = new DlgLogViewer(this);
	ruleEditor_ = new DlgRuleEditor(this);
	trayIcon_ = NULL;
	if (trayVisible) {
		TrayIcon::waitForSystemTray();
		trayIcon_ = new TrayIcon();
	}

	anEvents->Connect(anEVT_WIZARD_SHOW,
	    wxCommandEventHandler(MainFrame::onWizardShow), NULL, this);
	anEvents->Connect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(MainFrame::onLogViewerShow), NULL, this);
	anEvents->Connect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(MainFrame::onRuleEditorShow), NULL, this);
	anEvents->Connect(anEVT_SFSBROWSER_SHOW,
	    wxCommandEventHandler(MainFrame::onSfsBrowserShow), NULL, this);
	anEvents->Connect(anEVT_OPEN_ALERTS,
	    wxCommandEventHandler(MainFrame::OnOpenAlerts), NULL, this);
	anEvents->Connect(anEVT_OPEN_ESCALATIONS,
	    wxCommandEventHandler(MainFrame::OnOpenEscalations), NULL, this);
	anEvents->Connect(anEVT_ESCALATIONS_SHOW,
	    wxCommandEventHandler(MainFrame::OnEscalationsShow), NULL, this);
	anEvents->Connect(anEVT_ANOUBISOPTIONS_SHOW,
	    wxCommandEventHandler(MainFrame::OnAnoubisOptionShow), NULL, this);
	anEvents->Connect(anEVT_BACKUP_POLICY,
	    wxCommandEventHandler(MainFrame::onBackupPolicy), NULL, this);
	anEvents->Connect(anEVT_UPGRADENOTIFY,
	    wxCommandEventHandler(MainFrame::onUpgradeNotify), NULL, this);

	ANEVENTS_IDENT_BCAST_REGISTRATION(MainFrame);
}

MainFrame::~MainFrame()
{
	AnEvents	*anEvents;
	JobCtrl		*jobCtrl;

	anEvents = AnEvents::instance();
	jobCtrl = JobCtrl::instance();

	anEvents->Disconnect(anEVT_WIZARD_SHOW,
	    wxCommandEventHandler(MainFrame::onWizardShow), NULL, this);
	anEvents->Disconnect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(MainFrame::onLogViewerShow), NULL, this);
	anEvents->Disconnect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(MainFrame::onRuleEditorShow), NULL, this);
	anEvents->Disconnect(anEVT_SFSBROWSER_SHOW,
	    wxCommandEventHandler(MainFrame::onSfsBrowserShow), NULL, this);
	anEvents->Disconnect(anEVT_OPEN_ALERTS,
	    wxCommandEventHandler(MainFrame::OnOpenAlerts), NULL, this);
	anEvents->Disconnect(anEVT_OPEN_ESCALATIONS,
	    wxCommandEventHandler(MainFrame::OnOpenEscalations), NULL, this);
	anEvents->Disconnect(anEVT_ESCALATIONS_SHOW,
	    wxCommandEventHandler(MainFrame::OnEscalationsShow), NULL, this);
	anEvents->Disconnect(anEVT_ANOUBISOPTIONS_SHOW,
	    wxCommandEventHandler(MainFrame::OnAnoubisOptionShow), NULL, this);
	anEvents->Disconnect(anEVT_BACKUP_POLICY,
	    wxCommandEventHandler(MainFrame::onBackupPolicy), NULL, this);

	jobCtrl->Disconnect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(MainFrame::onSfsListArrived), NULL, this);
	ruleEditor_->Hide();
	logViewer_->Hide();

	ANEVENTS_IDENT_BCAST_DEREGISTRATION(MainFrame);

	SetStatusBar(NULL);
	delete an_statusbar;
	if (trayIcon_ != 0)
		delete trayIcon_;
	if (ruleEditor_ != NULL)
		delete ruleEditor_;
	if (logViewer_ != NULL)
		delete logViewer_;
}

void
MainFrame::OnInit(void)
{
	an_statusbar = new AnStatusBar(this);

	setMessageString();
	setConnectionString(false, wxEmptyString);

	SetStatusBar(an_statusbar);
	GetStatusBar()->Show();
	PositionStatusBar();
	Layout();
}

void
MainFrame::addModules(Module* modules[ANOUBIS_MODULESNO])
{
	wxString	 name;
	wxString	 tooltip;
	wxIcon		*icon;
	int		 i;
	int		 id;

	for (i=0; i<ANOUBIS_MODULESNO; i++) {
		if (modules[i] == NULL) {
			continue;
		}

		name = modules[i]->getNick();
		tooltip = modules[i]->getName();
		icon = modules[i]->getIcon();
		id   = modules[i]->getToolbarId();

		/* add module to toolbar and connect selection event */
		tb_LeftToolbarModule->AddRadioTool(id, name, *(icon),
		    wxNullIcon, tooltip);
		tb_LeftToolbarModule->Realize();
		this->Connect(id, wxEVT_COMMAND_MENU_SELECTED,
		    wxCommandEventHandler(MainFrame::OnTbModuleSelect));
	}

	/* add overview module as firt been displayed */
	sz_mainframeMain->Add(modules[OVERVIEW]->getMainPanel(), 1,
	    wxEXPAND, 5);
	sz_mainframeMain->Layout();
}

void
MainFrame::onRuleEditorShow(wxCommandEvent& event)
{
	an_menubar->Check(ID_MITOOLSRULEEDITOR, event.GetInt());
	event.Skip();
}

void
MainFrame::onLogViewerShow(wxCommandEvent& event)
{
	if(!event.GetInt())
		messageAlertCount_ = 0;
	setMessageString();

	an_menubar->Check(ID_MITOOLSLOGVIEWER, event.GetInt());
	event.Skip();
}

void
MainFrame::onWizardShow(wxCommandEvent& event)
{
	RuleWizard	*wizard = new RuleWizard(this);
	wxCommandEvent	showEvent(anEVT_WIZARD_SHOW);
	wxSize		displaySize;
	wxString	displayMsg;

	displaySize = wxGetDisplaySize();
	displayMsg = wxString::Format(_("Display size of %dx%d is too small "
	    "for the wizard.\nPresentation may be broken!"),
	    displaySize.GetWidth(), displaySize.GetHeight());
	an_menubar->Check(ID_MITOOLSWIZARD, event.GetInt());
	event.Skip();

	if (event.GetInt() != 0) {
		if (displaySize.GetWidth() < 1024) {
			anMessageBox(displayMsg, _("Wizard warning"),
			    wxOK|wxICON_EXCLAMATION, this);
		}
		wizard->RunWizard(wizard->getPage(RuleWizard::PAGE_PROGRAM));
		/* After finishing wizard, we uncheck menu and statusbar. */
		showEvent.SetInt(0);
		wxPostEvent(AnEvents::instance(), showEvent);
	}

	wizard->Destroy();
}

void
MainFrame::onSfsBrowserShow(wxCommandEvent& event)
{
	this->Show(event.GetInt());
	Module *module = MainUtils::instance()->getModule(SFS);
	int id = module->getToolbarId();

	/*
	 * Select the corresponding Modul Tab in the Toolbar
	 */
	tb_LeftToolbarModule->ToggleTool(MODSFS_ID_TOOLBAR, true);

	wxCommandEvent selectEvent(wxEVT_COMMAND_MENU_SELECTED, id);
	selectEvent.SetInt(id);
	this->AddPendingEvent(selectEvent);

	event.Skip();
}

void
MainFrame::setConnectionString(bool connected, const wxString &host)
{
	wxString label;
	label = host;

	if (connected) {
		statusBoxComText->SetLabel(_("ok"));
		statusBoxComIcon->SetIcon(
		    AnIconList::instance()->GetIcon(AnIconList::ICON_OK));
	} else {
		statusBoxComText->SetLabel(_("no"));
		statusBoxComIcon->SetIcon(
		    AnIconList::instance()->GetIcon(AnIconList::ICON_ERROR));
		an_menubar->Check(ID_MIFILECONNECT, false);
	}

	Layout();
}

void
MainFrame::setMessageString(void)
{
	/* escalations represent the highest priority */
	if (messageEscalationCount_ > 0) {
		statusBoxMsgIcon->SetIcon(
		    AnIconList::instance()->GetIcon(AnIconList::ICON_QUESTION));
		statusBoxMsgIcon->Show();
		statusBoxMsgText->SetLabel(wxString::Format(wxT("%d"),
		    messageEscalationCount_));
	} else {
		if (messageAlertCount_ > 0) {
			statusBoxMsgIcon->SetIcon(
			    AnIconList::instance()->GetIcon(
			    AnIconList::ICON_ALERT));
			statusBoxMsgIcon->Show();
			statusBoxMsgText->SetLabel(wxString::Format(wxT("%d"),
			    messageAlertCount_));
		} else {
			statusBoxMsgIcon->SetIcon(wxNullIcon);
			statusBoxMsgIcon->Hide();
			statusBoxMsgText->SetLabel(wxT("0"));
		}
	}

	Layout();
}

void
MainFrame::OnTbModuleSelect(wxCommandEvent& event)
{
	int id;
	id = event.GetId() - MODULEID_OFFSET_TOOLBAR + \
	     MODULEID_OFFSET_MAINPANEL;

	wxPanel *modulePanel = (wxPanel *)FindWindow(id);

	/* hide and detach the SECOND item of the mainframe sizer */
	sz_mainframeMain->GetItem(1)->GetWindow()->Hide();
	sz_mainframeMain->Detach(1);

	sz_mainframeMain->Add(modulePanel, 1, wxEXPAND, 5);
	modulePanel->Show();
	sz_mainframeMain->Layout();

	if (id == MODANOUBIS_ID_MAINPANEL) {
		((ModAnoubisMainPanelImpl *)modulePanel)->update();
	}
	if (id == MODPG_ID_MAINPANEL) {
		((ModPlaygroundMainPanelImpl *)modulePanel)->update();
	}
	event.Skip();
}

void
MainFrame::OnConnectionStateChange(wxCommandEvent& event)
{
	JobCtrl *instance = JobCtrl::instance();
	JobCtrl::ConnectionState newState =
	    (JobCtrl::ConnectionState)event.GetInt();
	bool connected = (newState == JobCtrl::CONNECTED);
	wxString hostname = event.GetString();
	wxString logMessage;

	switch (newState) {
	case JobCtrl::CONNECTED:
		logMessage = wxString::Format(
		    _("Connection established with %ls"), hostname.c_str());
		Debug::info(logMessage);
		doUpgradeNotify();
		break;
	case JobCtrl::DISCONNECTED:
	case JobCtrl::ERR_RW:
		logMessage = wxString::Format(
		    _("Disconnected from %ls"), hostname.c_str());
		Debug::info(logMessage);
		break;
	case JobCtrl::ERR_CONNECT:
	case JobCtrl::ERR_REG:
		logMessage = wxString::Format(
		    _("Connection to %ls failed!"), hostname.c_str());
		Debug::err(logMessage);
		break;
	case JobCtrl::ERR_VERSION_PROT:

		if (instance->getDaemonProtocolVersion() <
		    ANOUBIS_PROTO_VERSION) {
			logMessage.Printf(_("Because of an incompatible "
			    "protocol version (xanoubis: %i, Anoubis daemon: "
			    "%i) the connection was rejected. Please update "
			    "the Anoubis daemon package!"),
			    ANOUBIS_PROTO_VERSION,
			    instance->getDaemonProtocolVersion());
		} else {
			logMessage.Printf(_("Because of an incompatible "
			    "protocol version (xanoubis: %i, Anoubis daemon: "
			    "%i) the connection was rejected. Please update "
			    "the xanoubis package!"), ANOUBIS_PROTO_VERSION,
			    instance->getDaemonProtocolVersion());
		}

		anMessageBox(logMessage, _("Protocol version mismatch"),
		    wxOK | wxICON_ERROR, this);
		Debug::info(logMessage);
		break;
	case JobCtrl::ERR_VERSION_APN:

		if (instance->getDaemonApnVersion() < apn_parser_version()) {
			logMessage.Printf(_("The APN parser (v%i.%i) is newer "
			    "than the parser of the Anoubis daemon (v%i.%i). "
			    "Please update the Anoubis daemon package!"),
			    APN_PARSER_MAJOR(apn_parser_version()),
			    APN_PARSER_MINOR(apn_parser_version()),
			    APN_PARSER_MAJOR(instance->getDaemonApnVersion()),
			    APN_PARSER_MINOR(instance->getDaemonApnVersion()));
		} else {
			logMessage.Printf(_("The APN parser (v%i.%i) is older "
			    "than the parser of the Anoubis daemon (v%i.%i). "
			    "Please update the xanoubis package!"),
			    APN_PARSER_MAJOR(apn_parser_version()),
			    APN_PARSER_MINOR(apn_parser_version()),
			    APN_PARSER_MAJOR(instance->getDaemonApnVersion()),
			    APN_PARSER_MINOR(instance->getDaemonApnVersion()));
		}

		anMessageBox(logMessage, _("APN version mismatch"),
		    wxOK | wxICON_ERROR, this);
		Debug::info(logMessage);
		break;
	case JobCtrl::ERR_NO_KEY:
		logMessage = _("Because of missing keys the connection was "
		    "rejected. Please check your key-configuration "
		    "(certificate, private key).");

		anMessageBox(logMessage, _("Missing key"),
		    wxOK | wxICON_ERROR, this);
		Debug::info(logMessage);

		break;
	case JobCtrl::ERR_KEYID:
		logMessage.Printf(_("Your configured certificate does not "
		    "match with the certificate requested by the daemon. "
		    "Please check your key-configuration or ask your "
		    "system-administrator to configure your certificate at "
		    "the daemon."));

		anMessageBox(logMessage, _("Certificate mismatch"),
		    wxOK | wxICON_ERROR, this);
		Debug::info(logMessage);

		break;
	case JobCtrl::ERR_INV_KEY:
	case JobCtrl::ERR_INV_CERT:
		logMessage.Printf(_("Your configured certificate or private "
		    "key is missconfigured or is not supported by Anoubis. "
		    "Please check your certificate and private key."));
		Debug::info(logMessage);
		anMessageBox(logMessage, _("Invalid Certificate / Private Key"),
		    wxOK | wxICON_ERROR, this);
		break;
	case JobCtrl::ERR_AUTH_SYS_FAIL:
		logMessage.Printf(_("An internal error occured while "
		    "processing the authentication."));
		Debug::info(logMessage);
		anMessageBox(logMessage, _("Internal Error"),
		    wxOK | wxICON_ERROR, this);
		break;
	}

	setConnectionString(connected, hostname);
	an_menubar->Check(ID_MIFILECONNECT, connected);

	event.Skip();
}

void
MainFrame::OnOpenAlerts(wxCommandEvent& event)
{
	messageAlertCount_ = event.GetInt();
	setMessageString();
	event.Skip();
}

void
MainFrame::OnOpenEscalations(wxCommandEvent& event)
{
	messageEscalationCount_ = event.GetInt();
	setMessageString();
	event.Skip();
}

void
MainFrame::OnMbHelpAboutSelect(wxCommandEvent&)
{
	wxAboutDialogInfo info;

	info.SetName(wxT("Anoubis GUI"));
	info.SetVersion(wxT(VERSION));
	info.SetCopyright(wxT("(C) 2007-2010 GeNUA mbH"));
	info.SetWebSite(wxT("www.anoubis.org"));
	info.SetIcon(
	    AnIconList::instance()->GetIcon(AnIconList::ICON_ANOUBIS_BLACK_48));
	info.SetDescription(wxT("Build Id: " PACKAGE_BUILD));

	info.AddDeveloper(wxT("Alexander von Gernler"));
	info.AddDeveloper(wxT("Alexander Taute"));
	info.AddDeveloper(wxT("Andreas Fiessler"));
	info.AddDeveloper(wxT("Christian Ehrhardt"));
	info.AddDeveloper(wxT("Christian Hiesl"));
	info.AddDeveloper(wxT("Georg Hoesch"));
	info.AddDeveloper(wxT("Hans-Joerg Hoexer"));
	info.AddDeveloper(wxT("Joachim Ayasse"));
	info.AddDeveloper(wxT("Konrad Merz"));
	info.AddDeveloper(wxT("Martin Weber"));
	info.AddDeveloper(wxT("Michael Gernoth"));
	info.AddDeveloper(wxT("Pedro Martelleto"));
	info.AddDeveloper(wxT("Reinhard Tartler"));
	info.AddDeveloper(wxT("Robin Doer"));
	info.AddDeveloper(wxT("Sebastian Trahm"));
	info.AddDeveloper(wxT("Stefan Fritsch"));
	info.AddDeveloper(wxT("Stefan Rinkes"));
	info.AddDeveloper(wxT("Sten Spans"));
	info.AddDeveloper(wxT("Valentin Dornauer"));

	info.AddTranslator(wxT("Christian Wehrle"));
	info.AddTranslator(wxT("Mathias Pippel"));

	wxAboutBox(info);
}

void
MainFrame::OnMbFileCloseSelect(wxCommandEvent&)
{
	this->Hide();
}

void
MainFrame::OnMbFileImportSelect(wxCommandEvent&)
{
	wxString	caption = _("Choose a policy file:");
	wxString	wildcard = wxT("*");
	wxString	defaultDir = MainUtils::instance()->getDataDir();
	wxString	defaultFilename = wxEmptyString;
	wxFileDialog	fileDlg(this, caption, defaultDir, defaultFilename,
			    wildcard, wxOPEN);
	PolicyCtrl	*policyCtrl = PolicyCtrl::instance();

	if (fileDlg.ShowModal() == wxID_OK) {
		if (!policyCtrl->importFromFile(fileDlg.GetPath())) {
			anMessageBox(
			    _("Couldn't import policy file: it has errors."),
			    _("Error"), wxICON_ERROR);
		}
	}
}

void
MainFrame::OnMbFileExportSelect(wxCommandEvent&)
{
	wxString	caption = _("Choose a file to write the policies to:");
	wxString	wildcard = wxT("*");
	wxString	defaultDir = MainUtils::instance()->getDataDir();
	wxString	defaultFilename = wxEmptyString;
	wxFileDialog	fileDlg(this, caption, defaultDir, defaultFilename,
			    wildcard, wxFD_SAVE);

	if (fileDlg.ShowModal() == wxID_OK) {
		PolicyCtrl *policyCtrl = PolicyCtrl::instance();
		if (!policyCtrl->exportToFile(fileDlg.GetPath()))
			anMessageBox(
			    _("Failed to export the ruleset into a file."),
			    _("Export ruleset"), wxOK|wxICON_ERROR, this);
	}
}

void
MainFrame::OnMbFileQuitSelect(wxCommandEvent&)
{
	exitApp();
}

void
MainFrame::OnMbToolsRuleEditorSelect(wxCommandEvent& event)
{
	wxCommandEvent  showEvent(anEVT_RULEEDITOR_SHOW);

	showEvent.SetInt(event.IsChecked());

	wxPostEvent(AnEvents::instance(), showEvent);
}

void
MainFrame::OnMbToolsLogViewerSelect(wxCommandEvent& event)
{
	wxCommandEvent  showEvent(anEVT_LOGVIEWER_SHOW);

	showEvent.SetInt(event.IsChecked());

	wxPostEvent(AnEvents::instance(), showEvent);
}

void
MainFrame::onMbToolsWizardSelect(wxCommandEvent& event)
{
	wxCommandEvent  showEvent(anEVT_WIZARD_SHOW);

	showEvent.SetInt(event.IsChecked());

	wxPostEvent(AnEvents::instance(), showEvent);
}

void
MainFrame::OnMbHelpHelpSelect(wxCommandEvent&)
{
	printf("Menu Item Help->Help selected\n");
}

void
MainFrame::OnMbFileConnectSelect(wxCommandEvent& event)
{
	MainUtils::instance()->connectCommunicator(event.IsChecked());
}

void
MainFrame::OnMbEditPreferencesSelect(wxCommandEvent&)
{
	wxCommandEvent  showEvent(anEVT_ANOUBISOPTIONS_SHOW);

	showEvent.SetInt(true);

	wxPostEvent(AnEvents::instance(), showEvent);
}

void
MainFrame::OnClose(wxCloseEvent &event)
{
	if (exit_) {
		/* Really close the window */
		uint8_t answer = anMessageBox(
		    _("Do you really want to close xanoubis?"), _("Confirm"),
		    wxYES_NO, this);

		if (answer == wxYES) {
			event.Skip();
		} else {
			if (event.CanVeto())
				event.Veto();
			else
				Destroy();
		}
	} else {
		/* Hide the window */
		if (event.CanVeto())
			event.Veto();
		else
			Destroy();

		this->Hide();
	}
}

void
MainFrame::OnEscalationsShow(wxCommandEvent& event)
{
	Show(event.GetInt());
	if (event.GetInt()) {
		Raise();
		RequestUserAttention(wxUSER_ATTENTION_ERROR);
	}
	Module *module = MainUtils::instance()->getModule(ANOUBIS);
	int id = module->getToolbarId();

	/*
	 * Select the corresponding Modul Tab in the Toolbar
	 */
	tb_LeftToolbarModule->ToggleTool(MODANOUBIS_ID_TOOLBAR, true);

	wxCommandEvent selectEvent(wxEVT_COMMAND_MENU_SELECTED, id);
	selectEvent.SetInt(id);
	this->AddPendingEvent(selectEvent);

	event.Skip();
}

void
MainFrame::OnAnoubisOptionShow(wxCommandEvent& event)
{
	this->Show(event.GetInt());
	Module *module = MainUtils::instance()->getModule(ANOUBIS);
	int id = module->getToolbarId();

	/*
	 * Select the corresponding Modul Tab in the Toolbar
	 */
	tb_LeftToolbarModule->ToggleTool(MODANOUBIS_ID_TOOLBAR, true);

	wxCommandEvent selectEvent(wxEVT_COMMAND_MENU_SELECTED, id);
	selectEvent.SetInt(id);
	this->AddPendingEvent(selectEvent);

	event.Skip();
}

void
MainFrame::onBackupPolicy(wxCommandEvent &event)
{
	PolicyCtrl	*policyCtrl = PolicyCtrl::instance();
	PolicyRuleSet	*rs;

	rs = policyCtrl->getRuleSet(event.GetExtraLong());
	if (rs) {
		DlgBackupPolicy dlg(
		    AnIconList::instance()->getIcon(AnIconList::ICON_ALERT),
		    rs);
		this->Show();
		this->Raise();

		dlg.ShowModal();
		rs->unlock();
	}
}

void
MainFrame::doUpgradeNotify(void)
{
	JobCtrl			*instance = JobCtrl::instance();
	KeyCtrl			*keyCtrl = KeyCtrl::instance();
	LocalCertificate	&cert = keyCtrl->getLocalCertificate();
	struct anoubis_sig	*raw_cert;
	static time_t		 last_message = 0, now;

	if (cert.isLoaded()) {
		bool showUpgradeMessage = true;

		wxConfig::Get()->Read(wxT("/Options/ShowUpgradeMessage"),
		    &showUpgradeMessage);

		/*
		 * You only need to fetch the upgrade-list, if the
		 * related dialog should be displayed.
		 */
		if (showUpgradeMessage == true) {
			now = time(NULL);
			/*
			 * Only do this if we didn't do it within the last
			 * minute.
			 */
			if (now >= last_message + 60) {
				last_message = now;
				raw_cert = cert.getCertificate();
				upgradeTask_ = new ComSfsListTask();
				upgradeTask_->setRequestParameter(0, wxT("/"));
				upgradeTask_->setFetchUpgraded(true);
				upgradeTask_->setOneFile(true);
				upgradeTask_->setRecursive(true);
				upgradeTask_->setKeyId(raw_cert->keyid,
				    raw_cert->idlen);

				instance->addTask(upgradeTask_);
			}
		}
	}
}

void
MainFrame::onUpgradeNotify(wxCommandEvent &)
{
	MainUtils::instance()->checkBootConf();
	doUpgradeNotify();
}

void
MainFrame::exitApp()
{
	exit_ = true;
	Close();
}

void
MainFrame::onSfsListArrived(TaskEvent &event)
{
	ComSfsListTask		*task;
	ComTask::ComTaskResult	comResult;
	wxString		errMsg;
	wxArrayString		result;

	task = dynamic_cast<ComSfsListTask*>(event.getTask());
	comResult = ComTask::RESULT_LOCAL_ERROR;

	if (task == 0) {
		/* No ComUpgradeListGetTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (upgradeTask_ != task) {
		/* Belongs to someone other, ignore it */
		event.Skip();
		return;
	}

	event.Skip(false); /* "My" task -> stop propagating */
	comResult = task->getComTaskResult();

	if (comResult != ComTask::RESULT_SUCCESS) {
		if (comResult == ComTask::RESULT_COM_ERROR) {
			errMsg = wxString::Format(_("Communication "
			    "error while fetching list of upgraded "
			    "files."));
		} else if (comResult == ComTask::RESULT_REMOTE_ERROR) {
			errMsg = wxString::Format(_("%hs.\n\nCould not "
			    "fetch the list of upgraded files."),
			anoubis_strerror(task->getResultDetails()));
		} else {
			errMsg = wxString::Format(_("An unexpected "
			    "error (%i) occured while while fetching list "
			    "of upgraded files."), task->getComTaskResult());
		}
		anMessageBox(errMsg, _("Error"), wxICON_ERROR, this);
	} else {
		result = task->getFileList();
		if (result.Count() > 0) {
			DlgUpgradeAsk *askDlg = new DlgUpgradeAsk(this);
			askDlg->ShowModal();
			askDlg->Destroy();
		}
	}
	delete upgradeTask_;
	upgradeTask_ = NULL;
}

ANEVENTS_IDENT_BCAST_METHOD_DEFINITION(MainFrame)

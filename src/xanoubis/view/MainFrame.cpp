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
#include <wx/msgdlg.h>
#include <wx/aboutdlg.h>

#include "AnEvents.h"
#include "AnMessageDialog.h"
#include "AnShortcuts.h"
#include "AnStatusBar.h"
#include "JobCtrl.h"
#include "MainFrameBase.h"
#include "Module.h"
#include "ModAnoubis.h"
#include "ModSfs.h"
#include "ModAnoubisMainPanelImpl.h"
#include "RuleWizard.h"
#include "DlgBackupPolicyImpl.h"

#include "main.h"

#include "MainFrame.h"

MainFrame::MainFrame(wxWindow *parent) : MainFrameBase(parent)
{
	AnEvents	*anEvents;
	JobCtrl		*jobCtrl;

	shortcuts_ = new AnShortcuts(this);
	show_ = true;
	messageAlertCount_ = 0;
	messageEscalationCount_ = 0;
	aboutIcon_ = wxGetApp().loadIcon(wxT("ModAnoubis_black_48.png"));
	okIcon_ = wxGetApp().loadIcon(wxT("General_ok_16.png"));
	errorIcon_ = wxGetApp().loadIcon(wxT("General_error_16.png"));
	alertIcon_ = wxGetApp().loadIcon(wxT("General_alert_16.png"));
	escalationIcon_ = wxGetApp().loadIcon(wxT("General_question_16.png"));

	timer_.Start(5000);

	anEvents = AnEvents::getInstance();
	jobCtrl = JobCtrl::getInstance();

	jobCtrl->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(MainFrame::OnConnectionStateChange),
	    NULL, this);

	jobCtrl->Connect(anTASKEVT_SFS_LIST,
	    wxTaskEventHandler(MainFrame::onSfsListArrived),
	    NULL, this);

	upgradeTask_.setRequestParameter(0, wxT("/"));
	upgradeTask_.setFetchUpgraded(true);
	upgradeTask_.setOneFile(true);
	upgradeTask_.setRecursive(true);

	anEvents->Connect(anEVT_WIZARD_SHOW,
	    wxCommandEventHandler(MainFrame::onWizardShow), NULL, this);
	anEvents->Connect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(MainFrame::onLogViewerShow), NULL, this);
	anEvents->Connect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(MainFrame::onRuleEditorShow), NULL, this);
	anEvents->Connect(anEVT_MAINFRAME_SHOW,
	    wxCommandEventHandler(MainFrame::onMainFrameShow), NULL, this);
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
	anEvents->Connect(wxEVT_TIMER, wxTimerEventHandler(MainFrame::OnTimer),
	    NULL, this);
	timer_.SetOwner(anEvents);

	ANEVENTS_IDENT_BCAST_REGISTRATION(MainFrame);
}

MainFrame::~MainFrame()
{
	AnEvents	*anEvents;
	JobCtrl		*jobCtrl;

	anEvents = AnEvents::getInstance();
	jobCtrl = JobCtrl::getInstance();

	anEvents->Disconnect(anEVT_WIZARD_SHOW,
	    wxCommandEventHandler(MainFrame::onWizardShow), NULL, this);
	anEvents->Disconnect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(MainFrame::onLogViewerShow), NULL, this);
	anEvents->Disconnect(anEVT_RULEEDITOR_SHOW,
	    wxCommandEventHandler(MainFrame::onRuleEditorShow), NULL, this);
	anEvents->Disconnect(anEVT_MAINFRAME_SHOW,
	    wxCommandEventHandler(MainFrame::onMainFrameShow), NULL, this);
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

	ANEVENTS_IDENT_BCAST_DEREGISTRATION(MainFrame);

	SetStatusBar(NULL);
	delete an_statusbar;
	delete aboutIcon_;
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
	RuleWizard	wizard;
	wxCommandEvent	showEvent(anEVT_WIZARD_SHOW);

	an_menubar->Check(ID_MITOOLSWIZARD, event.GetInt());
	event.Skip();

	if (event.GetInt() != 0) {
		wizard.RunWizard(wizard.getPage(RuleWizard::PAGE_PROGRAM));
		/* After finishing wizard, we uncheck menu and statusbar. */
		showEvent.SetInt(0);
		wxPostEvent(AnEvents::getInstance(), showEvent);
	}
}

void
MainFrame::onMainFrameShow(wxCommandEvent& event)
{
	if (event.GetInt()) {
		this->Show();
		show_ = true;
	} else {
		this->Hide();
		show_ = false;
	}
	event.Skip();
}

void
MainFrame::onSfsBrowserShow(wxCommandEvent& event)
{
	this->Show(event.GetInt());
	Module *module = wxGetApp().getModule(SFS);
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
		statusBoxComIcon->SetIcon(*okIcon_);
	} else {
		statusBoxComText->SetLabel(_("no"));
		statusBoxComIcon->SetIcon(*errorIcon_);
		an_menubar->Check(ID_MIFILECONNECT, false);
	}

	Layout();
}

void
MainFrame::setMessageString(void)
{
	/* escalations represent the highest priority */
	if (messageEscalationCount_ > 0) {
		statusBoxMsgIcon->SetIcon(*escalationIcon_);
		statusBoxMsgIcon->Show();
		statusBoxMsgText->SetLabel(wxString::Format(wxT("%d"),
		    messageEscalationCount_));
	} else {
		if (messageAlertCount_ > 0) {
			statusBoxMsgIcon->SetIcon(*alertIcon_);
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
	event.Skip();
}

void
MainFrame::OnConnectionStateChange(wxCommandEvent& event)
{
	JobCtrl *instance = JobCtrl::getInstance();
	JobCtrl::ConnectionState newState =
	    (JobCtrl::ConnectionState)event.GetInt();
	bool connected = (newState == JobCtrl::CONNECTION_CONNECTED);
	wxString hostname = event.GetString();
	wxString logMessage;

	switch (newState) {
	case JobCtrl::CONNECTION_CONNECTED: {
		KeyCtrl			*keyCtrl = KeyCtrl::getInstance();
		LocalCertificate	&cert = keyCtrl->getLocalCertificate();
		struct anoubis_sig	*raw_cert;

		if (instance->isSfsDisable())
			logMessage = wxString::Format(
			    _("Connection established with %ls"),
			    hostname.c_str());
		else
			logMessage = wxString::Format(
			    _("Connection established with %ls "
			    "without sfsdisable!"),
			    hostname.c_str());

		wxGetApp().log(logMessage);
		if (cert.isLoaded()) {
			bool showUpgradeMessage = true;

			wxGetApp().getUserOptions()->Read(
			    wxT("/Options/ShowUpgradeMessage"),
			    &showUpgradeMessage);

			/*
			 * You only need to fetch the upgrade-list, if the
			 * related dialog should be displayed.
			 */
			if (showUpgradeMessage == true) {
				raw_cert = cert.getCertificate();
				upgradeTask_.setKeyId(
				    raw_cert->keyid, raw_cert->idlen);

				instance->addTask(&upgradeTask_);
			}
		}
		break;
	}
	case JobCtrl::CONNECTION_DISCONNECTED:
	case JobCtrl::CONNECTION_ERROR:
		logMessage = wxString::Format(
		    _("Disconnected from %ls"), hostname.c_str());
		wxGetApp().log(logMessage);
		break;
	case JobCtrl::CONNECTION_FAILED:
		logMessage = wxString::Format(
		    _("Connection to %ls failed!"), hostname.c_str());
		wxGetApp().alert(logMessage);
		break;
	}

	setConnectionString(connected, hostname);
	an_menubar->Check(ID_MIFILECONNECT, connected);
	wxGetApp().status(logMessage);

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
	info.SetCopyright(wxT("(C) 2007-2009 GeNUA mbH"));
	// info.SetWebSite(wxT("www.anoubis.org"));
	info.SetIcon(*aboutIcon_);
	// info.SetDescription(wxT(""));

	info.AddDeveloper(wxT("Alexander von Gernler"));
	info.AddDeveloper(wxT("Andreas Fiessler"));
	info.AddDeveloper(wxT("Christian Ehrhardt"));
	info.AddDeveloper(wxT("Christian Hiesl"));
	info.AddDeveloper(wxT("Hans-Joerg Hoexer"));
	info.AddDeveloper(wxT("Joachim Ayasse"));
	info.AddDeveloper(wxT("Konrad Merz"));
	info.AddDeveloper(wxT("Michael Gernoth"));
	info.AddDeveloper(wxT("Pedro Martelleto"));
	info.AddDeveloper(wxT("Reinhard Tartler"));
	info.AddDeveloper(wxT("Robin Doer"));
	info.AddDeveloper(wxT("Sebastian Trahm"));
	info.AddDeveloper(wxT("Stefan Fritsch"));
	info.AddDeveloper(wxT("Sten Spans"));

	info.AddTranslator(wxT("Christian Wehrle"));
	info.AddTranslator(wxT("Mathias Pippel"));

	wxAboutBox(info);
}

void
MainFrame::OnMbFileCloseSelect(wxCommandEvent&)
{
	this->Hide();
	show_ = false;
}

void
MainFrame::OnMbFileImportSelect(wxCommandEvent&)
{
	wxString	caption = _("Choose a policy file:");
	wxString	wildcard = wxT("*");
	wxString	defaultDir = wxGetApp().getDataDir();
	wxString	defaultFilename = wxEmptyString;
	wxFileDialog	fileDlg(NULL, caption, defaultDir, defaultFilename,
			    wildcard, wxOPEN);
	PolicyCtrl	*policyCtrl = PolicyCtrl::getInstance();

	if (fileDlg.ShowModal() == wxID_OK) {
		if (!policyCtrl->importFromFile(fileDlg.GetPath())) {
			wxMessageBox(
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
	wxString	defaultDir = wxGetApp().getDataDir();
	wxString	defaultFilename = wxEmptyString;
	wxFileDialog	fileDlg(NULL, caption, defaultDir, defaultFilename,
			    wildcard, wxFD_SAVE);

	if (fileDlg.ShowModal() == wxID_OK) {
		PolicyCtrl *policyCtrl = PolicyCtrl::getInstance();
		if (!policyCtrl->exportToFile(fileDlg.GetPath()))
			wxMessageBox(
			    _("Failed to export the ruleset into a file."),
			    _("Export ruleset"), wxOK|wxICON_ERROR, this);
	}
}

void
MainFrame::OnMbFileQuitSelect(wxCommandEvent&)
{
	wxGetApp().quit();
}

bool
MainFrame::OnQuit(void)
{
	uint8_t answer = wxMessageBox(_("xanoubis is going to be closed."),
	    _("Confirm"), wxYES_NO, this);

	if (answer == wxYES) {
		return true;
	} else {
		return false;
	}
}

void
MainFrame::OnMbToolsRuleEditorSelect(wxCommandEvent& event)
{
	wxCommandEvent  showEvent(anEVT_RULEEDITOR_SHOW);

	showEvent.SetInt(event.IsChecked());

	wxPostEvent(AnEvents::getInstance(), showEvent);
}

void
MainFrame::OnMbToolsLogViewerSelect(wxCommandEvent& event)
{
	wxCommandEvent  showEvent(anEVT_LOGVIEWER_SHOW);

	showEvent.SetInt(event.IsChecked());

	wxPostEvent(AnEvents::getInstance(), showEvent);
}

void
MainFrame::onMbToolsWizardSelect(wxCommandEvent& event)
{
	wxCommandEvent  showEvent(anEVT_WIZARD_SHOW);

	showEvent.SetInt(event.IsChecked());

	wxPostEvent(AnEvents::getInstance(), showEvent);
}

void
MainFrame::OnMbHelpHelpSelect(wxCommandEvent&)
{
	printf("Menu Item Help->Help selected\n");
}

void
MainFrame::OnMbFileConnectSelect(wxCommandEvent& event)
{
	wxGetApp().connectCommunicator(event.IsChecked());
}

void
MainFrame::OnMbEditPreferencesSelect(wxCommandEvent&)
{
	wxCommandEvent  showEvent(anEVT_ANOUBISOPTIONS_SHOW);

	showEvent.SetInt(true);

	wxPostEvent(AnEvents::getInstance(), showEvent);
}

void
MainFrame::OnClose(wxCloseEvent&)
{
	this->Hide();
	show_ = false;
}

void
MainFrame::OnEscalationsShow(wxCommandEvent& event)
{
	Show(event.GetInt());
	if (event.GetInt()) {
		Raise();
		RequestUserAttention(wxUSER_ATTENTION_ERROR);
	}
	Module *module = wxGetApp().getModule(ANOUBIS);
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
	Module *module = wxGetApp().getModule(ANOUBIS);
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
	PolicyCtrl	*policyCtrl = PolicyCtrl::getInstance();
	PolicyRuleSet	*rs;

	rs = policyCtrl->getRuleSet(event.GetExtraLong());
	if (rs) {
		DlgBackupPolicy dlg(alertIcon_, rs);
		this->Show();
		this->Raise();

		dlg.ShowModal();
		rs->unlock();
	}
}

bool
MainFrame::isShowing(void)
{
	return (show_);
}

void
MainFrame::OnTimer(wxTimerEvent &event)
{
#ifdef HAVE_SYS_INOTIFY_H
	struct inotify_event	ievent;
	wxString		msg;
	bool			warn = false;
	int			ret, iNotifyFd = -1;
	time_t			modTime = 0, savedTime = 0;
	struct stat		sbuf;

	iNotifyFd = wxGetApp().getINotify();
	if (iNotifyFd == -1) {
		wxGetApp().getUserOptions()->Read(
		     wxT("/Options/GrubModifiedTime"), &savedTime);
		ret = stat(wxGetApp().getGrubPath().fn_str(), &sbuf);
		if (ret != -1)
			modTime = sbuf.st_mtime;
		if (savedTime < modTime)
			warn = true;
	} else {
		if (read(iNotifyFd, &ievent, sizeof(ievent)) > 0)
			warn = true;
	}

	if (warn) {
		wxGetApp().getUserOptions()->Write(
		    wxT("/Options/GrubModifiedTime"), time(NULL));
		msg = _("The Boot Loader configuration has been updated."
			"Please make sure to boot an Anoubis Kernel.");
		AnMessageDialog dlg(this, msg, _("Warning"),
		    wxOK | wxICON_WARNING);
		dlg.onNotifyCheck(wxT("/Options/ShowKernelUpgradeMessage"));
		dlg.ShowModal();
	}
#endif
	event.Skip(false);
}

void
MainFrame::onSfsListArrived(TaskEvent &event)
{
	ComSfsListTask		*task;
	ComTask::ComTaskResult	comResult;
	wxString		errMsg;
	wxArrayString		result;
	DlgUpgradeAsk		askDlg(this);

	task = dynamic_cast<ComSfsListTask*>(event.getTask());
	comResult = ComTask::RESULT_LOCAL_ERROR;

	if (task == 0) {
		/* No ComUpgradeListGetTask -> stop propagating */
		event.Skip(false);
		return;
	}

	if (&upgradeTask_ != task) {
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
			errMsg = wxString::Format(_("Got error "
			    "(%hs) from daemon while fetching list "
			    "of upgraded files."),
			strerror(task->getResultDetails()));
		} else {
			errMsg = wxString::Format(_("An unexpected "
			    "error (%i) occured while while fetching list "
			    "of upgraded files."), task->getComTaskResult());
		}
		wxMessageBox(errMsg, _("Error"), wxICON_ERROR);
	} else {
		result = task->getFileList();
		if (result.Count() > 0) {
			askDlg.ShowModal();
		}
	}
}

ANEVENTS_IDENT_BCAST_METHOD_DEFINITION(MainFrame)

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

IMPLEMENT_APP(AnoubisGuiApp)

AnoubisGuiApp::AnoubisGuiApp(void)
{
	mainFrame = NULL;
	logViewer_ = NULL;
	ruleEditor_ = NULL;
	comCtrl_ = NULL;
	trayIcon = NULL;

	wxInitAllImageHandlers();

	paths_.SetInstallPrefix(wxT(GENERALPREFIX));
}

AnoubisGuiApp::~AnoubisGuiApp(void)
{
	/* mainFrame not handled here, 'cause object already destroyed */
	if (trayIcon != NULL)
		delete trayIcon;
}

void
AnoubisGuiApp::close(void)
{
	trayIcon->RemoveIcon();
}


bool AnoubisGuiApp::OnInit()
{
	mainFrame = new MainFrame((wxWindow*)NULL);
	logViewer_ = new DlgLogViewer(mainFrame);
	ruleEditor_ = new DlgRuleEditor(mainFrame);
	comCtrl_ = new CommunicatorCtrl(wxT("/var/run/anoubisd.sock"));
	trayIcon = new TrayIcon();

	modules_[OVERVIEW] = new ModOverview(mainFrame);
	modules_[ALF]      = new ModAlf(mainFrame);
	modules_[SFS]      = new ModSfs(mainFrame);
	modules_[ANOUBIS]  = new ModAnoubis(mainFrame);

	trayIcon->SetMessageByHand(0);

	mainFrame->Show();
	SetTopWindow(mainFrame);
	mainFrame->OnInit();

	((ModOverview*)modules_[OVERVIEW])->addModules(modules_);
	mainFrame->addModules(modules_);

	// XXX ST: The following should be considered as a hack to update the
	//         state of Module ALF by calling the update()-method.
	//         Eventually the actual call has to be triggered by an event.
	((ModAlf*)modules_[ALF])->update();
	((ModSfs*)modules_[SFS])->update();
	((ModAnoubis*)modules_[ANOUBIS])->update();

	update();

	return (true);
}

void
AnoubisGuiApp::sendEvent(wxCommandEvent& event)
{
	wxPostEvent(mainFrame, event);
	wxPostEvent(logViewer_, event);
	wxPostEvent(ruleEditor_, event);
	//wxPostEvent(com, event);
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
AnoubisGuiApp::connectToDaemon(bool doConnect)
{
	if (doConnect)
		comCtrl_->connect();
	else
		comCtrl_->disconnect();
	update();
}

void
AnoubisGuiApp::update(void)
{
	mainFrame->setDaemonConnection(comCtrl_->isConnected());
	mainFrame->setConnectionString(comCtrl_->getRemoteStation());
	updateTrayIcon();
}

wxIcon *
AnoubisGuiApp::loadIcon(wxString iconName)
{
	wxString iconFileName;

	iconFileName = paths_.GetDataDir() + _T("/icons/") + iconName;
	if (!::wxFileExists(iconFileName)) {
		/*
		 * We didn't find our icon (where --prefix told us)!
		 * Try to take executable path into account. This should
		 * fix a missing --prefix as the matter in our build and test
		 * environment with aegis.
		 */
		iconFileName  = ::wxPathOnly(paths_.GetExecutablePath()) +
		    _T("/../../..") + iconFileName;
	}
	/*
	 * XXX: by ch: No error check is done, 'cause wxIcon will open a error
	 * dialog, complaining about missing icons itself. But maybe a logging
	 * message should be generated when logging will been implemented.
	 */
	return (new wxIcon(iconFileName, wxBITMAP_TYPE_PNG));
}

Module *
AnoubisGuiApp::getModule(enum moduleIdx idx)
{
	return (modules_[idx]);
}

void
AnoubisGuiApp::updateTrayIcon(void)
{
	unsigned int messageNo;
	ModAnoubis *anoubisModule = (ModAnoubis *)modules_[ANOUBIS];

	messageNo = anoubisModule->getListSize(NOTIFY_LIST_NOTANSWERED);
	trayIcon->SetMessageByHand(messageNo);
	trayIcon->SetConnectedDaemon(comCtrl_->getRemoteStation());
}

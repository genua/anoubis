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

#include "Communicator.h"
#include "DlgLogViewer.h"
#include "DlgRuleEditor.h"
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
	com = NULL;
	trayIcon = NULL;

	wxInitAllImageHandlers();

	paths_.SetInstallPrefix(wxT(GENERALPREFIX));
}

AnoubisGuiApp::~AnoubisGuiApp(void)
{
	/* mainFrame not handled here, 'cause object already destroyed */
	if (com != NULL)
		delete com;
	if (trayIcon != NULL)
		delete trayIcon;
}

void
AnoubisGuiApp::close(void)
{
	trayIcon->RemoveIcon();
}

#include "NotifyList.h"

bool AnoubisGuiApp::OnInit()
{
	mainFrame = new MainFrame((wxWindow*)NULL);
	logViewer_ = new DlgLogViewer(mainFrame);
	ruleEditor_ = new DlgRuleEditor(mainFrame);
	com = new Communicator();
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

	return (true);
}

void
AnoubisGuiApp::setRuleEditorVisability(bool visable)
{
	ruleEditor_->Show(visable);
	mainFrame->setRuleEditorVisability(visable);
}

void
AnoubisGuiApp::toggleRuleEditorVisability(void)
{
	setRuleEditorVisability(!ruleEditor_->IsShown());
}

void
AnoubisGuiApp::setLogViewerVisability(bool visable)
{
	logViewer_->Show(visable);
	mainFrame->setLogViewerVisability(visable);
}

void
AnoubisGuiApp::toggleLogViewerVisability(void)
{
	setLogViewerVisability(!logViewer_->IsShown());
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
}

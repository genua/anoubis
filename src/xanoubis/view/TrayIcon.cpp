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

#include <wx/icon.h>
#include <wx/string.h>

#include "AnEvents.h"
#include "main.h"
#include "TrayIcon.h"

TrayIcon::TrayIcon(void)
{
	iconNormal_    = wxGetApp().loadIcon(_T("ModAnoubis_black_48.png"));
	iconMsgByHand_ = wxGetApp().loadIcon(_T("ModAnoubis_question_48.png"));

	messageByHandNo_ = 0;
	daemon_ = wxT("none");

	update();
	Connect(anEVT_COM_REMOTESTATION,
	    wxCommandEventHandler(TrayIcon::OnRemoteStation), NULL, this);
}

TrayIcon::~TrayIcon(void)
{
	delete iconNormal_;
	delete iconMsgByHand_;
}

void
TrayIcon::OnRemoteStation(wxCommandEvent& event)
{
	daemon_ = *(wxString *)(event.GetClientObject());
	update();
	event.Skip();
}

void
TrayIcon::SetMessageByHand(unsigned int number)
{
	messageByHandNo_ = number;
	update();
}

void
TrayIcon::SetConnectedDaemon(wxString daemon)
{
	daemon_ = daemon;
	update();
}

void
TrayIcon::update(void)
{
	wxString tooltip;
	wxIcon	*icon;

	if (messageByHandNo_ > 0) {
		tooltip = wxT("Messages: ");
		tooltip += wxString::Format(wxT("%d\n"), messageByHandNo_);
		icon = iconMsgByHand_;
	} else {
		tooltip = wxT("No messages\n");
		icon = iconNormal_;
	}
	if (!daemon_.Cmp(wxT("none"))) {
		tooltip += wxT("not connected");
	} else {
		tooltip += wxT("connected with ");
		tooltip += daemon_;
	}
	SetIcon(*icon, tooltip);
}

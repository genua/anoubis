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

#include "JobCtrl.h"
#include "ModAnoubisOverviewPanelImpl.h"
#include "main.h"

ModAnoubisOverviewPanelImpl::ModAnoubisOverviewPanelImpl(wxWindow* parent, \
    wxWindowID id) : ModAnoubisOverviewPanelBase(parent, id)
{
	wxIcon *icon = wxGetApp().loadIcon(wxT("ModAnoubis_black_48.png"));
	anoubisStatusIcon->SetIcon(*icon);

	setConnectionState(false);

	JobCtrl *jobCtrl = JobCtrl::getInstance();
	jobCtrl->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(
	       ModAnoubisOverviewPanelImpl::OnConnectionStateChange),
	    NULL, this);
}

void
ModAnoubisOverviewPanelImpl::setRadioButtons(wxString profileName)
{
	if (profileName.Cmp(wxT("admin")) == 0) {
		adminProfileRadioButton->SetValue(true);
	} else if (profileName.Cmp(wxT("medium")) == 0) {
		mediumProfileRadioButton->SetValue(true);
	} else if (profileName.Cmp(wxT("high")) == 0) {
		highProfileRadioButton->SetValue(true);
	} else {
		/* do nothing */
	}
}

void
ModAnoubisOverviewPanelImpl::OnHighProfileRadioButton(wxCommandEvent&)
{
}

void
ModAnoubisOverviewPanelImpl::OnMediumProfileRadioButton(wxCommandEvent&)
{
}

void
ModAnoubisOverviewPanelImpl::OnAdminProfileRadioButton(wxCommandEvent&)
{
}

void
ModAnoubisOverviewPanelImpl::OnConnectClicked(wxCommandEvent&)
{
	wxGetApp().connectCommunicator(true);
}

void
ModAnoubisOverviewPanelImpl::OnDisconnectClicked(wxCommandEvent&)
{
	wxGetApp().connectCommunicator(false);
}

void
ModAnoubisOverviewPanelImpl::OnConnectionStateChange(wxCommandEvent& event)
{
	JobCtrl::ConnectionState newState =
	    (JobCtrl::ConnectionState)event.GetInt();

	setConnectionState(newState == JobCtrl::CONNECTION_CONNECTED);

	event.Skip();
}

void
ModAnoubisOverviewPanelImpl::setConnectionState(bool connected)
{
	connectButton->Enable(!connected);
	disconnectButton->Enable(connected);
}

/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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

#include <anoubis_msg.h>
#include <wx/string.h>

#include "AnEvents.h"
#include "CommunicatorCtrl.h"
#include "main.h"
#include "Notification.h"
#include "EscalationNotify.h"
#include "LogNotify.h"
#include "AlertNotify.h"

CommunicatorCtrl::CommunicatorCtrl(wxString socketPath)
{
	socketPath_	= socketPath;
	connectionState_	= CONNECTION_DISCONNECTED;
	com_		= NULL;

	Connect(anEVT_COM_NOTIFYRECEIVED,
	    wxCommandEventHandler(CommunicatorCtrl::OnNotifyReceived));
	Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(CommunicatorCtrl::OnConnection));
}

CommunicatorCtrl::~CommunicatorCtrl(void)
{
	if (connectionState_ == CONNECTION_CONNECTED) {
		disconnect();
	}
}

void
CommunicatorCtrl::connect(void)
{
	wxThreadError rc;

	if (connectionState_ == CONNECTION_DISCONNECTED) {
		/*
		 * If this is a reconnect, the previous object referenced by
		 * com_, was deleted by 'Delete()' (which also removed the
		 * object). Thus com_ can be filled with a new object.
		 */
		com_ = new Communicator(this, socketPath_);

		rc = com_->Create();
		if (rc != wxTHREAD_NO_ERROR) {
			wxGetApp().alert(
			    wxT("Error: couldn't create communication thread"));
			com_->Delete();
			return;
		}

		rc = com_->Run();
		if (rc != wxTHREAD_NO_ERROR) {
			wxGetApp().alert(
			    wxT("Error: couldn't start communication thread"));
			com_->Delete();
			return;
		}
		wxGetApp().status(wxT("connecting to ") + socketPath_
		    + wxT(" ..."));
	}
}

void
CommunicatorCtrl::disconnect(void)
{
	wxThreadError rc;

	if (connectionState_ == CONNECTION_CONNECTED) {
		rc = com_->Delete(); /* no explicite 'delete com_' needed */
		if  (rc != wxTHREAD_NO_ERROR) {
			wxGetApp().alert(
			    wxT("Error: no proper shutdown of communication"));
		}
		wxGetApp().status(wxT("disconnecting from ") + socketPath_
		    + wxT(" ..."));
	}
}

bool
CommunicatorCtrl::isConnected(void)
{
	if (connectionState_ == CONNECTION_CONNECTED) {
		return true;
	} else {
		return false;
	}
}

wxString
CommunicatorCtrl::getRemoteStation(void)
{
	wxString result;

	if (connectionState_ == CONNECTION_CONNECTED) {
		/*
		 * XXX: currently only connections via unix domain socket
		 * to localhost are supported. -- [CH]
		 */
		result = wxT("localhost");
	} else {
		result = wxT("none");
	}

	return result;
}

void
CommunicatorCtrl::OnNotifyReceived(wxCommandEvent& event)
{
	wxCommandEvent		 addEvent(anEVT_ADD_NOTIFICATION);
	int			 type;
	Notification		*notify;
	struct anoubis_msg	*notifyMsg;

	notify = NULL;
	notifyMsg = (struct anoubis_msg *)(event.GetClientData());

	type = get_value((notifyMsg->u.general)->type);
	if (ANOUBIS_IS_NOTIFY(type)) {
		if (type == ANOUBIS_N_ASK) {
			notify = new EscalationNotify(notifyMsg);
		} else {
			notify = new AlertNotify(notifyMsg);
		}

		addEvent.SetClientObject((wxClientData*)notify);
		wxGetApp().sendEvent(addEvent);
	}
}

void
CommunicatorCtrl::OnConnection(wxCommandEvent& event)
{
	wxCommandEvent	remoteStationEvent(anEVT_COM_REMOTESTATION);
	wxString	logMessage;
	wxString        *host = NULL;

	switch(event.GetInt()) {
	case CONNECTION_CONNECTED:
		host = new wxString(wxT("localhost"));
		logMessage = wxT("Connection established with localhost:")
		    + socketPath_;
		wxGetApp().log(logMessage);
		break;
	case CONNECTION_DISCONNECTED:
		host = new wxString(wxT("none"));
		logMessage = wxT("Disconnected from localhost");
		wxGetApp().log(logMessage);
		break;
	case CONNECTION_FAILED:
		host = new wxString(wxT("none"));
		logMessage = wxT("Connection to localhost failed!");
		wxGetApp().alert(logMessage);
		break;
	}
	remoteStationEvent.SetClientObject((wxClientData*)host);
	wxGetApp().sendEvent(remoteStationEvent);
	wxGetApp().status(logMessage);
}

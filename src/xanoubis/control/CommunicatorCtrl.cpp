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
	com_ = NULL;
	socketPath_ = socketPath;
	isAlive_ = false;

	Connect(anEVT_COM_NOTIFYRECEIVED,
	    wxCommandEventHandler(CommunicatorCtrl::OnNotifyReceived));
}

CommunicatorCtrl::~CommunicatorCtrl(void)
{
	if (isAlive_) {
		disconnect();
	}
}

void
CommunicatorCtrl::connect(void)
{
	wxThreadError	 rc;
	wxCommandEvent	 event(anEVT_COM_REMOTESTATION);
	wxString	*host;

	if (!isAlive_) {
		/*
		 * If this is a reconnect, the previous object referenced by
		 * com_, was deleted by 'Delete()' (which also removed the
		 * object). Thus com_ can be filled with a new object.
		 */
		com_ = new Communicator(this, socketPath_);
		rc = com_->Create();
		if (rc != wxTHREAD_NO_ERROR) {
			wxGetApp().alert(
			    wxT("Error: couldn't crate communication thread"));
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
		isAlive_ = true;
		/*
		 * XXX currently only connections via unix domain socket
		 * to localhost are supported. -- ch
		 */
		host = new wxString(wxT("localhost"));
		event.SetClientObject((wxClientData*)host);
		wxGetApp().sendEvent(event);
		wxGetApp().log(wxT("Connection established"));
	}
}

void
CommunicatorCtrl::disconnect(void)
{
	wxThreadError	 rc;
	wxCommandEvent	 event(anEVT_COM_REMOTESTATION);
	wxString	*host;

	if (isAlive_) {
		rc = com_->Delete(); /* no explicite 'delete com_' needed */
		if  (rc != wxTHREAD_NO_ERROR) {
			wxGetApp().alert(
			    wxT("Error during shutdown of communication"));
		}
		/*
		 * Even if we couldn't Delete() the thread successfully, we
		 * enable the creation of a new thread. Because the old one
		 * will "delete itself", no memory leak is seen here. -- ch
		 */
		isAlive_ = false;
		host = new wxString(wxT("none"));
		event.SetClientObject((wxClientData*)host);
		wxGetApp().sendEvent(event);
		wxGetApp().log(wxT("Connection closed"));
	}
}

bool
CommunicatorCtrl::isConnected(void)
{
	if (!isAlive_) {
		return (false);
	} else {
		return (com_->isConnected());
	}
}

wxString
CommunicatorCtrl::getRemoteStation(void)
{
	if (isAlive_ && com_->isConnected()) {
		/*
		 * XXX currently only connections via unix domain socket
		 * to localhost are supported. -- ch
		 */
		return (wxT("localhost"));
	} else {
		return (wxT("none"));
	}
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

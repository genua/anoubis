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

#include "config.h"
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <wx/app.h>
#include <wx/msgdlg.h>
#include <wx/string.h>
#include <wx/thread.h>

#include <anoubischat.h>
#include <anoubis_protocol.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>

#include "AnEvents.h"
#include "Communicator.h"
#include "main.h"
#include "ModAnoubis.h"
#include "Module.h"
#include "Notification.h"

enum communicatorFlag {
	COMMUNICATOR_FLAG_NONE = 0,
	COMMUNICATOR_FLAG_INIT,
	COMMUNICATOR_FLAG_PROG,
	COMMUNICATOR_FLAG_DONE
};

static long long
getToken(void)
{
	static long long token = 0;
	return (++token);
}

Communicator::Communicator(wxEvtHandler *eventDestination, wxString socketPath)
    : wxThread(wxTHREAD_DETACHED)
{
	socketPath_ = socketPath;
	eventDestination_ = eventDestination;
	isConnected_ = CONNECTION_DISCONNECTED;
	channel_ = NULL;
	client_  = NULL;
}

void
Communicator::setConnectionState(connectionStateType state)
{
	wxCommandEvent event(anEVT_COM_CONNECTION);

	isConnected_ = state;

	event.SetInt(isConnected_);
	eventDestination_->AddPendingEvent(event);
}

achat_rc
Communicator::connect(void)
{
	struct sockaddr_storage	 ss;
	struct sockaddr_un	*ss_sun = (struct sockaddr_un *)&ss;
	achat_rc		 rc = ACHAT_RC_ERROR;

	channel_ = acc_create();
	if (channel_ == NULL)
		return (ACHAT_RC_OOMEM);

	rc = acc_settail(channel_, ACC_TAIL_CLIENT);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel_);
		channel_ = NULL;
		return (rc);
	}

	rc = acc_setsslmode(channel_, ACC_SSLMODE_CLEAR);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel_);
		channel_ = NULL;
		return (rc);
	}

	bzero(&ss, sizeof(ss));
	ss_sun->sun_family = AF_UNIX;
	strlcpy(ss_sun->sun_path, socketPath_.fn_str(),
	    sizeof(ss_sun->sun_path));
	rc = acc_setaddr(channel_, &ss);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel_);
		channel_ = NULL;
		return (rc);
	}

	rc = acc_prepare(channel_);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel_);
		channel_ = NULL;
		return (rc);
	}

	rc = acc_open(channel_);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel_);
		channel_ = NULL;
		return (rc);
	}

	return (ACHAT_RC_OK);
}

void
Communicator::shutdown(connectionStateType state)
{
	setConnectionState(state);

	if (client_ != NULL) {
		anoubis_client_close(client_);
		anoubis_client_destroy(client_);
	}

	if (channel_ != NULL) {
		acc_destroy(channel_);
	}
}

void *
Communicator::Entry(void)
{
	struct anoubis_transaction	*currTa;
	bool				 notDone;
	enum communicatorFlag		 startRegistration;
	enum communicatorFlag		 startDeRegistration;

	currTa  = NULL;
	notDone = true;
	startRegistration   = COMMUNICATOR_FLAG_INIT;
	startDeRegistration = COMMUNICATOR_FLAG_NONE;

	if (connect() != ACHAT_RC_OK) {
		shutdown(CONNECTION_FAILED);
		return (NULL);
	}

	client_ = anoubis_client_create(channel_);
	if (client_ == NULL) {
		shutdown(CONNECTION_FAILED);
		return (NULL);
	}

	currTa = anoubis_client_connect_start(client_, ANOUBIS_PROTO_BOTH);
	if (currTa == NULL) {
		shutdown(CONNECTION_FAILED);
		return (NULL);
	}

	while (notDone) {
		struct anoubis_msg	*msg;
		size_t			 size = 1024;
		achat_rc		 rc = ACHAT_RC_ERROR;

		if (currTa && (currTa->flags & ANOUBIS_T_DONE)) {
			/* the current transaction was done */
			anoubis_transaction_destroy(currTa);
			currTa = NULL;
			if (startRegistration == COMMUNICATOR_FLAG_PROG) {
				startRegistration = COMMUNICATOR_FLAG_DONE;
				setConnectionState(CONNECTION_CONNECTED);
			}
			if (startDeRegistration == COMMUNICATOR_FLAG_PROG) {
				startDeRegistration = COMMUNICATOR_FLAG_DONE;
				notDone = false;
				continue;
			}
		}

		if (currTa == NULL) {
			/* we can start the next transaction */
			if (startRegistration == COMMUNICATOR_FLAG_INIT) {
				currTa = anoubis_client_register_start(client_,
				    getToken(), geteuid(), 0, 0);
				startRegistration = COMMUNICATOR_FLAG_PROG;
			}
			if (startDeRegistration == COMMUNICATOR_FLAG_INIT) {
				currTa = anoubis_client_unregister_start(
				    client_, getToken(), geteuid(), 0, 0);
				startDeRegistration = COMMUNICATOR_FLAG_PROG;
			}
		}

		msg = anoubis_msg_new(1024);
		if (msg == NULL) {
			/* XXX: is this error path ok? -- ch */
			startDeRegistration = COMMUNICATOR_FLAG_INIT;
			continue;
		}

		rc = acc_receivemsg(channel_, (char*)(msg->u.buf), &size);
		if (rc != ACHAT_RC_OK) {
			/* XXX: is this error path ok? -- ch */
			startDeRegistration = COMMUNICATOR_FLAG_INIT;
			continue;
		}

		anoubis_msg_resize(msg, size);

		if (anoubis_client_process(client_, msg) != 1) {
			startDeRegistration = COMMUNICATOR_FLAG_INIT;
		}

		if (anoubis_client_hasnotifies(client_) != 0) {
			wxCommandEvent event(anEVT_COM_NOTIFYRECEIVED);
			struct anoubis_msg *notifyMsg = NULL;

			notifyMsg = anoubis_client_getnotify(client_);
			event.SetClientData(notifyMsg);
			eventDestination_->AddPendingEvent(event);
		}

		if (TestDestroy() &&
		    (startDeRegistration == COMMUNICATOR_FLAG_NONE)) {
			startDeRegistration = COMMUNICATOR_FLAG_INIT;
		}
	}

	shutdown(CONNECTION_DISCONNECTED);

	return (NULL);
}

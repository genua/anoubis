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

#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <wx/app.h>
#include <wx/msgdlg.h>
#include <wx/string.h>
#include <wx/thread.h>

#include "anoubischat.h"
#include "anoubis_protocol.h"
#include "anoubis_client.h"
#include "anoubis_msg.h"
#include "anoubis_transaction.h"
#include "Communicator.h"
#include "main.h"
#include "ModAnoubis.h"
#include "Module.h"
#include "NotifyList.h"

static long long
getToken(void)
{
	static long long token = 0;
	return (++token);
}

Communicator::Communicator(wxString socketPath)
{
	channel_ = acc_create();
	client_  = NULL;
	socketPath_ = socketPath;
	notDone_ = true;
	doRegister_ = true;
	isRegistered_ = false;
	registerInProcess_ = false;
	doShutdown_ = false;
	isDown_ = false;
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
		return (rc);
	}

	rc = acc_setsslmode(channel_, ACC_SSLMODE_CLEAR);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel_);
		return (rc);
	}

	bzero(&ss, sizeof(ss));
	ss_sun->sun_family = AF_UNIX;
#ifdef LINUX
	strncpy(ss_sun->sun_path, socketPath_.fn_str(),
	    sizeof(ss_sun->sun_path) - 1 );
#endif
#ifdef OPENBSD
	strlcpy(ss_sun->sun_path, socketPath_.fn_str(),
	    sizeof(ss_sun->sun_path));
#endif
	rc = acc_setaddr(channel_, &ss);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel_);
		return (rc);
	}

	rc = acc_prepare(channel_);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel_);
		return (rc);
	}

	rc = acc_open(channel_);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel_);
		return (rc);
	}

	return (ACHAT_RC_OK);
}

void *
Communicator::Entry(void)
{
	struct anoubis_transaction	*currTa;

	if (connect() != ACHAT_RC_OK) {
		wxMessageBox(wxT("Can't connect to anoubis daemon!"),
		    wxT("Error"), wxOK | wxICON_ERROR);
		(wxGetApp()).close();
		return (NULL);
	}

	client_ = anoubis_client_create(channel_);
	if (client_ == NULL) {
		acc_destroy(channel_);
		return (NULL);
	}

	currTa = anoubis_client_connect_start(client_, ANOUBIS_PROTO_BOTH);
	if (currTa == NULL) {
		anoubis_client_close(client_);
		anoubis_client_destroy(client_);
		acc_destroy(channel_);
	}

	while (notDone_) {
		struct anoubis_msg * m;
		size_t size = 1024;
		achat_rc rc;

		if (currTa && (currTa->flags & ANOUBIS_T_DONE)) {
			anoubis_transaction_destroy(currTa);
			if (doRegister_ && registerInProcess_ &&
			    (currTa->result == 0)) {
				doRegister_ = false;
				isRegistered_ = true;
				registerInProcess_ = false;
			}
			if (doShutdown_ && (currTa->result == 0)) {
				doShutdown_ = false;
				isDown_ = true;
				notDone_ = false;
			}
			currTa = NULL;
		}

		if (currTa == NULL) {
			if (doRegister_ && !registerInProcess_) {
				currTa = anoubis_client_register_start(client_,
				    getToken(), geteuid(), 0, 0, 0);
				registerInProcess_ = true;
			}
			if (doShutdown_) {
				currTa = anoubis_client_unregister_start(
				    client_, getToken(), geteuid(), 0, 0, 0);
			}
		}

		m = anoubis_msg_new(1024);
		if (m == NULL) {
			notDone_ = false;
		}

		rc = acc_receivemsg(channel_, (char*)(m->u.buf), &size);
		if (rc != ACHAT_RC_OK) {
			notDone_ = false;
		}

		anoubis_msg_resize(m, size);

		int ret;
		ret = anoubis_client_process(client_, m);
		if (ret != 1) {
			notDone_ = false;
		}

		struct anoubis_msg *notify = get_notifies(client_);
		if (notify != NULL) {
			NotifyListEntry *nl;
			ModAnoubis *anoubisModule;

			nl = new NotifyListEntry(notify);
			anoubisModule = (ModAnoubis *)(wxGetApp().getModule(
			    ANOUBIS));
			anoubisModule->insertNotification(nl);
		}

	}

	anoubis_client_close(client_);
	anoubis_client_destroy(client_);
	acc_destroy(channel_);

	return (NULL);
}

void
Communicator::open(void)
{
	doRegister_ = true;
}

void
Communicator::close(void)
{
	doShutdown_ = true;
}

bool
Communicator::isConnected(void)
{
	return (true);
}

wxString
Communicator::getRemoteStation(void)
{
	if(isConnected()) {
		return (wxT("localhost"));
	} else {
		return (wxT("none"));
	}
}

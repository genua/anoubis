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

#ifndef _COMMUNICATOR_H_
#define _COMMUNICATOR_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/string.h>
#include <wx/thread.h>

#include <anoubischat.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>

#include "Notification.h"
#include "EscalationNotify.h"

enum connectionStateType {
	CONNECTION_CONNECTED = 0,
	CONNECTION_DISCONNECTED,
	CONNECTION_FAILED,
	CONNECTION_RXTX_ERROR
};

class Communicator : public wxThread {
	private:
		wxString		 socketPath_;
		wxString		 policyBuf_;
		bool			 policyReq_;
		bool			 policyUse_;
		connectionStateType	 isConnected_;
		NotifyList		 answerList_;
		wxEvtHandler		*eventDestination_;
		struct achat_channel	*channel_;
		struct anoubis_client	*client_;

		void		setConnectionState(connectionStateType);
		achat_rc	connect(void);
		void		shutdown(connectionStateType);

	public:
		Communicator(wxEvtHandler *, wxString);

		virtual void	*Entry(void);

		void sendEscalationAnswer(Notification *);
		void policyRequest(void);
		void policyUse(wxString);
};

#endif	/* _COMMUNICATOR_H_ */

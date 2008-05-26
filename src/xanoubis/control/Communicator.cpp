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
#include <errno.h>
#include <sys/un.h>

#include <unistd.h>

#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis_sfs.h>
#endif
#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#include <dev/anoubis_alf.h>
#include <dev/anoubis_sfs.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

#include <wx/app.h>
#include <wx/msgdlg.h>
#include <wx/string.h>
#include <wx/thread.h>
#include <wx/file.h>
#include <wx/filename.h>

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
#include "EscalationNotify.h"

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
	policyReq_ = false;
	policyUse_ = false;
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
	struct anoubis_transaction	*reqTa;
	bool				 notDone;
	enum communicatorFlag		 startRegistration;
	enum communicatorFlag		 startDeRegistration;
	enum communicatorFlag		 startStatRegistration;
	enum communicatorFlag		 startStatDeRegistration;
	enum connectionStateType	 commRC;
	enum communicatorFlag		 startPolicyRequest;
	enum communicatorFlag		 startPolicyUse;
	wxFile				 tmpFile;
	wxString			 tmpName;
	wxString			 tmpPreFix;
	Policy_GetByUid			req;
	Policy_SetByUid			*ureq;
	char				*buf;
	int				 prio;
	size_t				 length, total, size;

	reqTa	= NULL;
	buf	= NULL;
	ureq	= NULL;
	currTa  = NULL;
	notDone = true;
	commRC			= CONNECTION_FAILED;
	startRegistration	= COMMUNICATOR_FLAG_INIT;
	startDeRegistration	= COMMUNICATOR_FLAG_NONE;
	startStatRegistration	= COMMUNICATOR_FLAG_NONE;
	startStatDeRegistration	= COMMUNICATOR_FLAG_NONE;
	startPolicyRequest	= COMMUNICATOR_FLAG_NONE;
	startPolicyUse		= COMMUNICATOR_FLAG_NONE;
	prio			= 2;
	length			= 0;
	total			= 0;
	tmpPreFix		= wxT("xanoubis");

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
		struct anoubis_msg	*msg, *reqmsg;
		achat_rc		 rc = ACHAT_RC_ERROR;
		NotifyList::iterator	 ali;

		if (currTa && (currTa->flags & ANOUBIS_T_DONE)) {
			/* the current transaction was done */
			anoubis_transaction_destroy(currTa);
			currTa = NULL;
			if (startRegistration == COMMUNICATOR_FLAG_PROG) {
				startRegistration = COMMUNICATOR_FLAG_DONE;
				startStatRegistration = COMMUNICATOR_FLAG_INIT;
				setConnectionState(CONNECTION_CONNECTED);
			}
			if (startDeRegistration == COMMUNICATOR_FLAG_PROG) {
				startDeRegistration = COMMUNICATOR_FLAG_DONE;
				notDone = false;
				commRC = CONNECTION_DISCONNECTED;
				continue;
			}
			if (startStatDeRegistration == COMMUNICATOR_FLAG_PROG) {
				startStatDeRegistration =
				    COMMUNICATOR_FLAG_DONE;
				startDeRegistration = COMMUNICATOR_FLAG_INIT;
			}
		}

		if (currTa == NULL) {
			/* we can start the next transaction */
			if (startRegistration == COMMUNICATOR_FLAG_INIT) {
				currTa = anoubis_client_register_start(client_,
				    getToken(), geteuid(), 0, 0);
				startRegistration = COMMUNICATOR_FLAG_PROG;
			}
			if (startStatRegistration == COMMUNICATOR_FLAG_INIT) {
				currTa = anoubis_client_register_start(client_,
				    getToken(), 0, 0, ANOUBIS_SOURCE_STAT);
				startStatRegistration = COMMUNICATOR_FLAG_PROG;
			}
			if (startDeRegistration == COMMUNICATOR_FLAG_INIT) {
				currTa = anoubis_client_unregister_start(
				    client_, getToken(), geteuid(), 0, 0);
				startDeRegistration = COMMUNICATOR_FLAG_PROG;
			}
			if (startStatDeRegistration == COMMUNICATOR_FLAG_INIT) {
				currTa = anoubis_client_unregister_start(
				    client_, getToken(), 0, 0,
				    ANOUBIS_SOURCE_STAT);
				startStatDeRegistration =
				    COMMUNICATOR_FLAG_PROG;
			}
		}

		/*
		 * Load policy of xanoubis in anoubisd and activate the loaded
		 * policy
		 */
		if (policyUse_ && startPolicyUse == COMMUNICATOR_FLAG_NONE &&
		startPolicyRequest == COMMUNICATOR_FLAG_NONE) {
			startPolicyUse = COMMUNICATOR_FLAG_INIT;
			policyUse_ = false;
			length = policyBuf_.Length();

			buf = (char *)malloc(length);
			if(!buf) {
				/* Here should be a better error handling*/
				notDone = false;
				continue;
			}

			strlcpy( buf,
				(const char *)policyBuf_.mb_str(wxConvUTF8),
				length);
		}

		if (startPolicyUse == COMMUNICATOR_FLAG_INIT) {
			length = strlen(buf);
			total =	sizeof(*ureq) + length;

			ureq = (Policy_SetByUid *)malloc(total);
			if (!ureq) {
			/* XXX: KM Here should be a better error handling*/
				notDone = false;
				continue;
			}

			set_value(ureq->ptype, ANOUBIS_PTYPE_SETBYUID);
			set_value(ureq->uid, geteuid());
			set_value(ureq->prio, 1);

			if (length)
				memcpy(ureq->payload, buf, length+1);

			reqTa = anoubis_client_policyrequest_start(client_,
					ureq, total);
			if (!reqTa) {
				notDone = false;
				commRC =  CONNECTION_RXTX_ERROR;
				continue;
			}

			startPolicyUse = COMMUNICATOR_FLAG_PROG;
		}

		if (startPolicyUse == COMMUNICATOR_FLAG_PROG) {
			if(reqTa->flags & ANOUBIS_T_DONE)
				startPolicyUse = COMMUNICATOR_FLAG_DONE;
		}

		if (startPolicyUse == COMMUNICATOR_FLAG_DONE) {
			if (reqTa->result) {
			/* XXX: KM Here should be a better error handling*/
				notDone = false;
				continue;
			}
			anoubis_transaction_destroy(reqTa);
			free(buf);
			startPolicyUse = COMMUNICATOR_FLAG_NONE;
		}

		/* Request policy from anoubisd				*/
		if (policyReq_ &&
			startPolicyRequest == COMMUNICATOR_FLAG_NONE &&
			startPolicyUse == COMMUNICATOR_FLAG_NONE) {
			startPolicyRequest = COMMUNICATOR_FLAG_INIT;
			policyReq_ = false;
			prio = 0;
		}

		if (startPolicyRequest == COMMUNICATOR_FLAG_INIT && prio < 2) {
			set_value(req.ptype, ANOUBIS_PTYPE_GETBYUID);
			set_value(req.uid, geteuid());
			set_value(req.prio, prio);
			reqTa = anoubis_client_policyrequest_start(client_,
					&req, sizeof(req));
			if(!reqTa) {
			/* XXX: KM Here should be a better error handling*/
				notDone = false;
				commRC =  CONNECTION_RXTX_ERROR;
				continue;
			}
			startPolicyRequest = COMMUNICATOR_FLAG_PROG;
		}

		if (startPolicyRequest == COMMUNICATOR_FLAG_PROG) {
			if(reqTa->flags & ANOUBIS_T_DONE)
				startPolicyRequest = COMMUNICATOR_FLAG_DONE;
		}

		if (startPolicyRequest == COMMUNICATOR_FLAG_DONE) {
			if(reqTa->result) {
			/* XXX: KM Here should be a better error handling*/
				anoubis_transaction_destroy(reqTa);
				startPolicyRequest = COMMUNICATOR_FLAG_NONE;
				prio = 2;
				continue;
			}

			reqmsg = reqTa->msg;
			reqTa->msg = NULL;
			anoubis_transaction_destroy(reqTa);

			if(!reqmsg || !VERIFY_LENGTH(reqmsg,
				sizeof(Anoubis_PolicyReplyMessage)) ||
				get_value(reqmsg->u.policyreply->error) != 0) {
			/* XXX: KM Here should be a better error handling*/
				anoubis_transaction_destroy(reqTa);
				startPolicyRequest = COMMUNICATOR_FLAG_NONE;
				prio = 2;
				continue;
			}

			/* XXX: KM should be done with the apn_parser */
			tmpName = wxFileName::CreateTempFileName(tmpPreFix,
						&tmpFile);
			while(reqmsg) {
				size_t len;
				struct anoubis_msg * tmp;
				len = reqmsg->length - CSUM_LEN
					- sizeof(Anoubis_PolicyReplyMessage);
				if (len != tmpFile.Write(
					reqmsg->u.policyreply->payload, len)) {
			/* XXX: KM Here should be a better error handling*/
					notDone = false;
					continue;
				}
				tmp = reqmsg;
				reqmsg = reqmsg->next;
				anoubis_msg_free(tmp);
			}

			wxCommandEvent event(anEVT_ANOUBISD_RULESET_ARRIVED);
			event.SetString(tmpName);
			eventDestination_->AddPendingEvent(event);
			tmpFile.Close();
			prio++;
			if(prio < 2)
				startPolicyRequest = COMMUNICATOR_FLAG_INIT;
			else
				startPolicyRequest = COMMUNICATOR_FLAG_NONE;
		}

		size = 4096;
		msg = anoubis_msg_new(size);
		if (msg == NULL) {
			/* XXX: is this error path ok? -- ch */
			startStatDeRegistration = COMMUNICATOR_FLAG_INIT;
			continue;
		}

		rc = acc_receivemsg(channel_, (char*)(msg->u.buf), &size);
		if (rc != ACHAT_RC_OK) {
			notDone = false;
			commRC = CONNECTION_RXTX_ERROR;
			continue;
		}

		anoubis_msg_resize(msg, size);

		if (anoubis_client_process(client_, msg) != 1) {
			startStatDeRegistration = COMMUNICATOR_FLAG_INIT;
		}

		if (anoubis_client_hasnotifies(client_) != 0) {
			wxCommandEvent event(anEVT_COM_NOTIFYRECEIVED);
			struct anoubis_msg *notifyMsg = NULL;

			notifyMsg = anoubis_client_getnotify(client_);
			event.SetClientData(notifyMsg);
			eventDestination_->AddPendingEvent(event);
		}

		for (ali=answerList_.begin(); ali!=answerList_.end(); ali++) {
			anoubis_token_t		 token;
			int			 allowed;
			EscalationNotify	*escalation;

			escalation = (EscalationNotify *)(*ali);
			token = escalation->getToken();
			allowed = escalation->getAnswer()->wasAllowed();
			anoubis_client_notifyreply(client_, token,
			    allowed?0:EPERM, 0);
			answerList_.erase(ali);
		}

		if (TestDestroy() &&
		    (startStatDeRegistration == COMMUNICATOR_FLAG_NONE)) {
			startStatDeRegistration = COMMUNICATOR_FLAG_INIT;
		}
	}

	shutdown(commRC);

	return (NULL);
}

void
Communicator::sendEscalationAnswer(Notification *notify)
{
	if ((notify != NULL) && IS_ESCALATIONOBJ(notify)) {
		answerList_.Append(notify);
	}
}

void
Communicator::policyRequest(void)
{
	policyReq_ = true;
}

void
Communicator::policyUse(wxString policyBuf)
{
	policyUse_ = true;
	policyBuf_ = policyBuf;
}

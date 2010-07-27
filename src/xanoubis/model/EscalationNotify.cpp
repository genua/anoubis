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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

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

#include <anoubis_msg.h>
#include <wx/string.h>

#include "AnEvents.h"
#include "main.h"
#include "Notification.h"
#include "NotifyAnswer.h"
#include "EscalationNotify.h"

EscalationNotify::EscalationNotify(struct anoubis_msg *msg) : Notification(msg)
{
	answer_ = NULL;
	allowEdit_ = false;
	rulePath_ = wxEmptyString;
}

EscalationNotify::~EscalationNotify(void)
{
	if (isAnswered()) {
		delete answer_;
	}
}

bool
EscalationNotify::isAnswered(void)
{
	if (answer_ == NULL) {
		return (false);
	} else {
		return (true);
	}
}

void
EscalationNotify::setAnswer(NotifyAnswer *answer)
{
	answer_ = answer;
}

void
EscalationNotify::answer(NotifyAnswer *answer)
{
	wxCommandEvent	event(anEVT_ANSWER_ESCALATION);

	answer_ = answer;

	event.SetClientObject((wxClientData *)this);
	wxPostEvent(AnEvents::instance(), event);
}

NotifyAnswer *
EscalationNotify::getAnswer(void)
{
	return (answer_);
}

anoubis_token_t
EscalationNotify::getToken(void)
{
	return (notify_->u.notify->token);
}

anoubis_cookie_t
EscalationNotify::getTaskCookie(void)
{
	return (get_value(notify_->u.notify->task_cookie));
}

wxString
EscalationNotify::getBinaryName(void)
{
	wxString result;
	int	 offset;
	int	 length;
	char	*buffer;

	offset = get_value(notify_->u.notify->pathoff);
	length = get_value(notify_->u.notify->pathlen);

	if (length <= 0) {
		result = wxEmptyString;
	} else {
		buffer = notify_->u.notify->payload + offset;
		result = wxString::From8BitData(buffer, length);
	}

	return (result);
}

wxString
EscalationNotify::getCtxBinaryName(void)
{
	wxString result;
	int	 offset;
	int	 length;
	char	*buffer;

	offset = get_value(notify_->u.notify->ctxpathoff);
	length = get_value(notify_->u.notify->ctxpathlen);

	if (length <= 0) {
		result = wxEmptyString;
	} else {
		buffer = notify_->u.notify->payload + offset;
		result = wxString::From8BitData(buffer, length);
	}

	return (result);
}

wxString
EscalationNotify::getAction(void)
{
	wxString	action;
	if (answer_ == NULL) {
		action = _("was asked");
	} else {
		if (answer_->wasAllowed()) {
			action = _("was allowed");
		} else {
			action = _("was denied");
		}
	}
	return action;
}

bool
EscalationNotify::getChecksum(unsigned char csum[MAX_APN_HASH_LEN])
{
	int	offset;
	int	length;

	offset = get_value(notify_->u.notify->csumoff);
	length = get_value(notify_->u.notify->csumlen);

	bzero(csum, MAX_APN_HASH_LEN);
	bcopy(notify_->u.notify->payload+offset, csum, length);

	if (length == 0)
		return false;
	else
		return true;
}

bool
EscalationNotify::getCtxChecksum(unsigned char csum[MAX_APN_HASH_LEN])
{
	int	offset;
	int	length;

	offset = get_value(notify_->u.notify->ctxcsumoff);
	length = get_value(notify_->u.notify->ctxcsumlen);

	bzero(csum, MAX_APN_HASH_LEN);
	bcopy(notify_->u.notify->payload+offset, csum, length);

	if (length == 0)
		return false;
	else
		return true;
}

int
EscalationNotify::getProtocolNo(void)
{
	struct alf_event	*alf;
	int			 evoff  =  0;

	evoff = get_value(notify_->u.notify->evoff);
	alf = (struct alf_event *)((notify_->u.notify)->payload + evoff);

	return (alf->protocol);
}

int
EscalationNotify::getDirectionNo(void)
{
	struct alf_event	*alf;
	int			 evoff  =  0;

	evoff = get_value(notify_->u.notify->evoff);
	alf = (struct alf_event *)((notify_->u.notify)->payload + evoff);

	switch (alf->op) {
	case ALF_CONNECT:
		return APN_CONNECT;
	case ALF_ACCEPT:
		return APN_ACCEPT;
	case ALF_SENDMSG:
		return APN_SEND;
	case ALF_RECVMSG:
		return APN_RECEIVE;
	default:
		return APN_BOTH;
	}
}

bool
EscalationNotify::allowEdit(void)
{
	return allowEdit_;
}

void
EscalationNotify::setAllowEdit(bool value)
{
	allowEdit_ = value;
}

wxString
EscalationNotify::rulePath(void)
{
	return rulePath_;
}

void
EscalationNotify::setRulePath(wxString path)
{
	rulePath_ = path;
}

bool
EscalationNotify::allowOptions(void)
{
	if (module_.IsEmpty()) {
		module_ = getModule();
	}
	if (module_ == wxT("SFS") || module_ == wxT("SANDBOX")) {
		return true;
	} else if (module_ == wxT("ALF")) {
		switch(getProtocolNo()) {
		case IPPROTO_TCP:
		case IPPROTO_UDP:
		case IPPROTO_SCTP:
			return true;
		default:
			return false;
		}
	} else {
		return false;
	}
}

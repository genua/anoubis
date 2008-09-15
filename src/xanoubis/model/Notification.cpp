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
#include <wx/intl.h>
#include <wx/utils.h>

#include "Notification.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(NotifyList);

Notification::Notification(struct anoubis_msg *msg)
{
	module_ = wxEmptyString;
	timeStamp_ = wxEmptyString;
	logMessage_ = wxEmptyString;
	notify_ = msg;
}

Notification::~Notification(void)
{
	if (notify_ != NULL) {
		anoubis_msg_free(notify_);
		notify_ = NULL;
	}
}

wxString
Notification::assembleAddress(bool isLocal)
{
	wxString		 localAddr;
	char			 ipAddrBuffer[512];
	unsigned short		 port;
	struct alf_event	*alf;
	const void		*address;
	int			 evoff;

	if (notify_ == NULL) {
		return (_("no notify data available"));
	}

	evoff = get_value(notify_->u.notify->evoff);
	bzero(ipAddrBuffer, sizeof(ipAddrBuffer));
	/* XXX CEH: Should verify that evlen >= sizeof(struct alf_event) */
	alf = (struct alf_event *)((notify_->u.notify)->payload + evoff);

	switch (alf->family) {
	case AF_INET:
		if (isLocal) {
			address = (void *)&(alf->local.in_addr.sin_addr);
			port = ntohs(alf->local.in_addr.sin_port);
		} else {
			address = (void *)&(alf->peer.in_addr.sin_addr);
			port = ntohs(alf->peer.in_addr.sin_port);
		}
		break;
	case AF_INET6:
		if (isLocal) {
			address = (void *)&(alf->local.in6_addr.sin6_addr);
			port = ntohs(alf->local.in6_addr.sin6_port);
		} else {
			address = (void *)&(alf->peer.in6_addr.sin6_addr);
			port = ntohs(alf->peer.in6_addr.sin6_port);
		}
		break;
	default:
		localAddr = _("unknown address type");
		return (localAddr);
		break;
	}

	inet_ntop(alf->family, address, ipAddrBuffer, sizeof(ipAddrBuffer));
	localAddr += wxString::From8BitData(ipAddrBuffer);
	if (alf->protocol == IPPROTO_TCP || alf->protocol == IPPROTO_UDP)
		localAddr += wxString::Format(wxT(" Port %u"), port);

	return (localAddr);
}

wxString
Notification::localAlfAddress(void)
{
	return (assembleAddress(true));
}

wxString
Notification::remoteAlfAddress(void)
{
	return (assembleAddress(false));
}

wxString
Notification::getModule(void)
{
	if (module_.IsEmpty() && (notify_ != NULL)) {
		switch (get_value((notify_->u.notify)->subsystem)) {
		case ANOUBIS_SOURCE_TEST:
			module_ = wxT("TEST");
			break;
		case ANOUBIS_SOURCE_ALF:
			module_ = wxT("ALF");
			break;
		case ANOUBIS_SOURCE_SANDBOX:
			module_ = wxT("SANDBOX");
			break;
		case ANOUBIS_SOURCE_SFS:
			module_ = wxT("SFS");
			break;
		case ANOUBIS_SOURCE_PROCESS:
			module_ = wxT("PROCESS");
			break;
		case ANOUBIS_SOURCE_STAT:
			module_ = wxT("STAT");
			break;
		default:
			module_ = _("(unknown)");
			break;
		}
	}

	return (module_);
}

wxString
Notification::getTime(void)
{
	if (timeStamp_.IsEmpty()) { // && (notify_ != NULL)) {
		/* XXX: use time of notify_  -- ch */
		timeStamp_ = wxNow();
	}

	return (timeStamp_);
}

wxString
Notification::getLogMessage(void){
	int	type;

	if (logMessage_.IsEmpty()) {
		if (getRuleId()) {
			logMessage_ += wxString::Format(_("Rule %d: "),
			    getRuleId());
		}

		logMessage_ += getOperation() + wxT(" ") + getPath();
		type = get_value((notify_->u.notify)->type);

		if (type == ANOUBIS_N_LOGNOTIFY) {
			logMessage_ += wxT(" ") + getAction();
		}
	}

	return (logMessage_);
}

unsigned int
Notification::getRuleId(void)
{
	int ruleId = 0;

	if(notify_ != NULL) {
		ruleId = get_value((notify_->u.notify)->rule_id);
	}

	return (ruleId);
}

wxString
Notification::getAction(void)
{
	wxString action;
	int	 error;

	action = _("was ");
	error = get_value((notify_->u.notify)->error);
	switch (error) {
	case POLICY_ALLOW:
		action += _("allowed");
		break;
	case POLICY_DENY:
		action += _("denied");
		break;
	case POLICY_ASK:
		action += _("asked");
		break;
	default:
		action = _("caused unknown action");
		break;
	}

	return (action);
}

wxString
Notification::getOperation(void)
{
	wxString		 operation;
	struct sfs_open_message	*sfs;
	struct alf_event	*alf;
	int			 evoff;

	if (notify_ == NULL) {
		return (_("no notify data available"));
	}

	evoff = get_value(notify_->u.notify->evoff);
	if (module_.IsEmpty()) {
		module_ = getModule();
	}

	if (module_.Cmp(wxT("ALF")) == 0) {
		/* XXX CEH: Should verify that evlen >= sizeof(*alf) */
		alf = (struct alf_event *)
		    ((notify_->u.notify)->payload + evoff);
		switch (alf->op) {
		case ALF_CONNECT:
			operation = wxT("connect");
			break;
		case ALF_ACCEPT:
			operation = wxT("accept");
			break;
		case ALF_SENDMSG:
			operation = wxT("send message");
			break;
		case ALF_RECVMSG:
			operation = wxT("receive message");
			break;
		default:
			operation = _("unknown");
			break;
		}
		switch (alf->protocol) {
		case IPPROTO_TCP:
			operation += wxT(" (TCP)");
			break;
		case IPPROTO_UDP:
			operation += wxT(" (UDP)");
			break;
		case IPPROTO_ICMP:
			operation += wxT(" (ICMP)");
			break;
		default:
			operation += wxT(" (unknown protocol)");
		}
	} else if (module_.Cmp(wxT("SFS")) == 0) {
		/* XXX CEH: Should verify that evlen >= sizeof(*sfs) */
		sfs = (struct sfs_open_message *)
		    ((notify_->u.notify)->payload + evoff);
		if (sfs->flags & ANOUBIS_OPEN_FLAG_READ) {
			operation += wxT("read ");
		}
		if (sfs->flags & ANOUBIS_OPEN_FLAG_WRITE) {
			operation += wxT("write");
		}
	} else {
		operation = _("unknown module");
	}

	return (operation);
}

wxString
Notification::getPath(void)
{
	wxString		 path;
	struct sfs_open_message	*sfs;
	struct alf_event	*alf;
	int			 evoff;

	if (notify_ == NULL) {
		return (_("no notify data available"));
	}

	evoff = get_value(notify_->u.notify->evoff);
	if (module_.IsEmpty()) {
		module_ = getModule();
	}

	if (module_.Cmp(wxT("ALF")) == 0) {
		/* XXX CEH: Should verify that evlen >= sizeof(*alf) */
		alf = (struct alf_event *)
		    ((notify_->u.notify)->payload + evoff);
		path = wxT("from ");
		if (alf->op == ALF_RECVMSG || alf->op == ALF_ACCEPT) {
			path += remoteAlfAddress() + wxT("  to  ");
			path += localAlfAddress();
		} else {
			path += localAlfAddress() + wxT("  to  ");
			path += remoteAlfAddress();
		}
	} else if (module_.Cmp(wxT("SFS")) == 0) {
		/* XXX CEH: Should verify that evlen >= sizeof(*sfs) */
		sfs = (struct sfs_open_message *)
		    ((notify_->u.notify)->payload + evoff);
		if (sfs->flags & ANOUBIS_OPEN_FLAG_PATHHINT) {
			path = wxString::From8BitData(sfs->pathhint);
		} else {
			path = _("no path information available");
		}
	} else {
		path = _("unable to extract path information");
	}

	return (path);
}

wxString
Notification::getOrigin(void)
{
	wxString origin;
	int	 offset;
	int	 length;
	char	*buffer;

	if (notify_ == NULL) {
		return (_("no notify data available"));
	}

	origin = wxString::Format(wxT("Pid: %u / Uid: %u"),
	    get_value((notify_->u.notify)->pid),
	    get_value((notify_->u.notify)->uid));

	offset = get_value((notify_->u.notify)->pathoff);
	length = get_value((notify_->u.notify)->pathlen);
	if (length > 0) {
		buffer = notify_->u.notify->payload + offset;
		origin += _(" / Program: ");
		origin += wxString::From8BitData(buffer, length);
	}

	return (origin);
}

wxString
Notification::getCheckSum(void)
{
	wxString		 checksum;
	struct sfs_open_message	*sfs;
	int			 evoff;
	int			 csumoff;
	int			 csumlen;

	if (notify_ == NULL) {
		return (_("no notify data available"));
	}

	evoff = get_value(notify_->u.notify->evoff);
	if (module_.IsEmpty()) {
		module_ = getModule();
	}

	if (module_.Cmp(wxT("ALF")) == 0) {
		csumoff = get_value((notify_->u.notify)->csumoff);
		csumlen = get_value((notify_->u.notify)->csumlen);
		if (csumlen > 0) {
			unsigned char *csum;
			csum = (unsigned char *)notify_->u.notify->payload +
			    csumoff;
			checksum = wxT("0x");
			for (int i=0; i<csumlen; i++) {
				checksum += wxString::Format(wxT("%02x"),
				    csum[i]);
			}
		} else {
			checksum = _("no checksum information available");
		}
	} else if (module_.Cmp(wxT("SFS")) == 0) {
		/* XXX CEH: Should verify that evlen >= sizeof(*sfs) */
		sfs = (struct sfs_open_message *)
		    ((notify_->u.notify)->payload + evoff);
		if (sfs->flags & ANOUBIS_OPEN_FLAG_CSUM) {
			checksum = wxT("0x");
			for (int i=0; i<ANOUBIS_SFS_CS_LEN; i++) {
				checksum += wxString::Format(wxT("%02x"),
				    sfs->csum[i]);
			}
		} else {
			checksum = _("no checksum information available");
		}
	} else {
		checksum = _("unable to extract checksum information");
	}

	return (checksum);
}

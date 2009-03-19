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

#include "PolicyUtils.h"
#include "Notification.h"
#include "main.h"

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
	int		type;
	wxString	action;

	if (logMessage_.IsEmpty()) {
		if (isAdmin()) {
			logMessage_ += wxString::Format(_("(A): "));
		}

		if (getRuleId()) {
			logMessage_ += wxString::Format(_("Rule %d: "),
			    getRuleId());
		}

		logMessage_ += getOperation() + wxT(" ") + getPath();
		type = get_value((notify_->u.notify)->type);
	}
	action = getAction();
	if (action.IsEmpty()) {
		return logMessage_;
	} else {
		return (logMessage_ + wxT(" ") + this->getAction());
	}
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

bool
Notification::isAdmin(void)
{
	bool result = false;

	if (notify_ != NULL) {
		result = get_value((notify_->u.notify)->prio) ? false : true;
	}

	return (result);
}

wxString
Notification::getAction(void)
{
	wxString action = wxEmptyString;
	int	 error;

	if (notify_) {
		error = get_value((notify_->u.notify)->error);
		if (error == 0) {
			action = _("was allowed");
		} else {
			action = _("was denied");
		}
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
	} else if (module_ == wxT("SFS") || module_ == wxT("SANDBOX")) {
		/* XXX CEH: Should verify that evlen >= sizeof(*sfs) */
		sfs = (struct sfs_open_message *)
		    ((notify_->u.notify)->payload + evoff);
		if (sfs->flags & ANOUBIS_OPEN_FLAG_READ) {
			operation += wxT("read ");
		}
		if (sfs->flags & ANOUBIS_OPEN_FLAG_WRITE) {
			operation += wxT("write ");
		}
		if (sfs->flags & ANOUBIS_OPEN_FLAG_EXEC) {
			operation += wxT("exec");
		}
	} else {
		operation = _("unknown module");
	}

	return (operation);
}

bool
Notification::isRwx(unsigned long mask)
{
	int				 evoff;
	struct sfs_open_message		*sfs;

	if (module_.IsEmpty()) {
		module_ = getModule();
	}
	if (module_ != wxT("SFS") && module_ != wxT("SANDBOX"))
		return false;
	evoff = get_value(notify_->u.notify->evoff);
	sfs = (struct sfs_open_message *)((notify_->u.notify)->payload + evoff);
	if ((sfs->flags & mask) == mask)
		return true;
	return false;
}

bool
Notification::isRead(void)
{
	return isRwx(ANOUBIS_OPEN_FLAG_READ);
}

bool
Notification::isWrite(void)
{
	return isRwx(ANOUBIS_OPEN_FLAG_WRITE);
}

bool
Notification::isExec(void)
{
	return isRwx(ANOUBIS_OPEN_FLAG_EXEC);
}

wxString
Notification::getPath(void)
{
	wxString		 path, csum;
	struct alf_event	*alf;
	int			 evoff;

	if (notify_ == NULL) {
		return (_("no notify data available"));
	}

	evoff = get_value(notify_->u.notify->evoff);
	if (module_.IsEmpty()) {
		module_ = getModule();
	}

	if (module_ == wxT("ALF")) {
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
	} else if (module_ == wxT("SFS") || module_ == wxT("SANDBOX")) {
		path = filePath();
		if (path.IsEmpty()) {
			path = _("no path information available");
		} else {
			csum = getFileChecksum();
			if (!csum.IsEmpty()) {
				path += wxT(" (0x") + csum + wxT(")");
			}
		}
	} else {
		path = _("unable to extract path information");
	}

	return (path);
}

wxString
Notification::filePath(void)
{
	wxString	path = wxEmptyString;

	if (module_.IsEmpty()) {
		module_ = getModule();
	}
	if (module_ == wxT("SFS") || module_ == wxT("SANDBOX")) {
		struct sfs_open_message		*sfs;
		int				 evoff;

		/* XXX CEH: Should verify that evlen >= sizeof(*sfs) */
		evoff = get_value(notify_->u.notify->evoff);
		sfs = (struct sfs_open_message *)
		    ((notify_->u.notify)->payload + evoff);
		if (sfs->flags & ANOUBIS_OPEN_FLAG_PATHHINT) {
			path = wxString::From8BitData(sfs->pathhint);
		}
	}
	return path;
}

wxString
Notification::getOrigin(void)
{
	wxString	 origin, user;
	int		 off, len;
	char		*buffer;

	if (notify_ == NULL) {
		return (_("no notify data available"));
	}

	off = get_value((notify_->u.notify)->pathoff);
	len = get_value((notify_->u.notify)->pathlen);
	buffer = notify_->u.notify->payload;
	user = wxGetApp().getUserNameById(get_value(notify_->u.notify->uid));
	if (len > 0) {
		wxString path = wxString::From8BitData(buffer+off, len);
		origin = wxString::Format(_("%ls (Pid: %d, Uid: %ls)"),
		    path.c_str(), get_value(notify_->u.notify->pid),
		    user.c_str());
	} else {
		origin = wxString::Format(_("Pid: %d, Uid: %ls"),
		    get_value(notify_->u.notify->pid), user.c_str());
	}
	return (origin);
}

wxString
Notification::getCtxOrigin(void)
{
	wxString	 origin, path, csum;
	int		 off, len, csoff, cslen;
	char		*buffer;

	if (notify_ == NULL) {
		return (_("no notify data available"));
	}

	off = get_value((notify_->u.notify)->ctxpathoff);
	len = get_value((notify_->u.notify)->ctxpathlen);
	csoff = get_value((notify_->u.notify)->ctxcsumoff);
	cslen = get_value((notify_->u.notify)->ctxcsumlen);
	buffer = notify_->u.notify->payload;
	path = wxString::From8BitData(buffer+off, len);
	if (!PolicyUtils::csumToString(
	    (unsigned char*)buffer+csoff, cslen, csum)) {
		cslen = 0;
	}
	if (len > 0 && cslen > 0) {
		origin = wxString::Format(_("%ls (%ls)"), path.c_str(),
		    csum.c_str());
	} else if (len > 0) {
		origin = path;
	} else if (cslen > 0) {
		origin = csum;
	} else {
		origin = _("unknown");
	}
	return (origin);
}

wxString
Notification::getFileChecksum(void)
{
	wxString		 checksum = wxEmptyString;
	struct sfs_open_message	*sfs;
	int			 evoff;

	if (notify_ == NULL) {
		return (_("no notify data available"));
	}

	evoff = get_value(notify_->u.notify->evoff);
	if (module_.IsEmpty()) {
		module_ = getModule();
	}

	if (module_ == wxT("SFS") || module_ == wxT("SANDBOX")) {
		sfs = (struct sfs_open_message *)
		    ((notify_->u.notify)->payload + evoff);
		if (sfs->flags & ANOUBIS_OPEN_FLAG_CSUM) {
			if (!PolicyUtils::csumToString(sfs->csum,
			    ANOUBIS_SFS_CS_LEN, checksum))
				checksum = wxEmptyString;
		}
	}
	return (checksum);
}

uid_t
Notification::getUid(void)
{
	return get_value(notify_->u.notify->uid);
}

int
Notification::getSfsmatch(void)
{
	return get_value(notify_->u.notify->sfsmatch);
}

int
Notification::getType(void)
{
	return get_value(notify_->u.general->type);
}

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

#include <netinet/in.h>

#include "anoubis_protocol.h"
#include "anoubis_msg.h"
#include "main.h"
#include "NotifyList.h"
#include "NotifyAnswer.h"

#include <wx/string.h>
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(NotifyList);

static unsigned int ids = 0;

NotifyListEntry::NotifyListEntry(struct anoubis_msg *notify)
{
	struct sfs_open_message *sfs;
	struct alf_event *alf;
	char ipAddr[512];

	id_ = ids++;
	isAnswered_ = false;
	isAnswerAble_ = false;

	type_ = get_value((notify->u.notify)->subsystem);
	switch (type_) {
	case ANOUBIS_SOURCE_TEST:
		module_ = wxT("TEST");
		break;
	case ANOUBIS_SOURCE_ALF:
		module_ = wxT("ALF");
		alf = (struct alf_event *)(notify->u.notify)->payload;

		switch (alf->op) {
		case ALF_CONNECT:
			alf_op_ = wxT("connect");
			break;
		case ALF_ACCEPT:
			alf_op_ = wxT("accept");
			break;
		case ALF_SENDMSG:
			alf_op_ = wxT("send message");
			break;
		case ALF_RECVMSG:
			alf_op_ = wxT("receive message");
			break;
		default:
			alf_op_ = wxT("unknown");
			break;
		}

		alf_transport_ = wxT("from ");

		bzero(ipAddr, sizeof(ipAddr));
		inet_ntop (alf->family, (const void *)&(alf->local),
		    ipAddr, sizeof(ipAddr));
		alf_transport_ += wxString::From8BitData(ipAddr);
		alf_transport_ += wxString::Format(wxT(" / %d   to   "),
		    ntohs(alf->local.in_addr.sin_port));
		bzero(ipAddr, sizeof(ipAddr));
		inet_ntop (alf->family, (const void *)&(alf->peer),
		    ipAddr, sizeof(ipAddr));
		alf_transport_ += wxString::From8BitData(ipAddr);
		alf_transport_ += wxString::Format(wxT(" / %d"),
		    ntohs(alf->peer.in_addr.sin_port));

		alf_who_ = wxString::Format(wxT("%d / %d"),
		    alf->pid, alf->uid);
		break;
	case ANOUBIS_SOURCE_SANDBOX:
		module_ = wxT("SANDBOX");
		break;
	case ANOUBIS_SOURCE_SFS:
		module_ = wxT("SFS");
		sfs = (struct sfs_open_message *)(notify->u.notify)->payload;
		sfs_op_ = wxT("open for: ");
		if (sfs->flags & ANOUBIS_OPEN_FLAG_READ) {
			sfs_op_ += wxT("read ");
		}
		if (sfs->flags & ANOUBIS_OPEN_FLAG_WRITE) {
			sfs_op_ += wxT("write");
		}
		if (sfs->flags & ANOUBIS_OPEN_FLAG_STRICT) {
			if (sfs->flags & ANOUBIS_OPEN_FLAG_CSUM) {
				sfs_chksum_ = wxT("");
				for (int i=0; i<ANOUBIS_SFS_CS_LEN; i++) {
					sfs_chksum_ += wxString::Format(
					    wxT("%02x"), sfs->csum[i]);
				}
			} else {
				sfs_chksum_ = wxT("unknown");
			}
			if (sfs->flags & ANOUBIS_OPEN_FLAG_PATHHINT) {
				sfs_path_ = wxString::From8BitData(
				    sfs->pathhint);
			} else {
				sfs_path_ = wxT("unknown");
			}
		} else {
			sfs_path_ = wxT("unknown");
			sfs_chksum_ = wxT("unknown");
		}
		break;
	default:
		module_ = wxT("unknown");
		break;
	}
}

bool
NotifyListEntry::isAnswered(void)
{
	return (isAnswered_);
}

bool
NotifyListEntry::isAnswerAble(void)
{
	return (isAnswerAble_);
}

void
NotifyListEntry::answer(NotifyAnswer *answer)
{
	isAnswered_ = true;
	answer_ = answer;
}

unsigned int
NotifyListEntry::getId(void)
{
	return (id_);
}

int
NotifyListEntry::getType(void)
{
	return (type_);
}

wxString
NotifyListEntry::getAlfOp(void)
{
	return (alf_op_);
}

wxString
NotifyListEntry::getAlfTransport(void)
{
	return (alf_transport_);
}

wxString
NotifyListEntry::getAlfWho(void)
{
	return (alf_who_);
}

wxString
NotifyListEntry::getSfsOp(void)
{
	return (sfs_op_);
}

wxString
NotifyListEntry::getSfsPath(void)
{
	return (sfs_path_);
}

wxString
NotifyListEntry::getSfsChkSum(void)
{
	return (sfs_chksum_);
}

wxString
NotifyListEntry::getModule(void)
{
	return (module_);
}

NotifyAnswer *
NotifyListEntry::getAnswer(void)
{
	return (answer_);
}

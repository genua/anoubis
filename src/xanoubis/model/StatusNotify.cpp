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
#include <anoubis_protocol.h>
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/utils.h>

#include "Notification.h"
#include "StatusNotify.h"

StatusNotify::StatusNotify(wxString msg) : Notification(NULL)
{
	module_ = wxT("xanoubis");
	timeStamp_ = wxNow();
	logMessage_ = msg;
}

StatusNotify::StatusNotify(struct anoubis_msg *msg) : Notification(msg)
{
	valueNo_ = msg->length;
	valueNo_ -= CSUM_LEN;
	valueNo_ -= sizeof(Anoubis_NotifyMessage);
	valueNo_ -= sizeof(struct anoubis_stat_message);
	valueNo_ /= sizeof(struct anoubis_stat_value);

	statMsg_ = (struct anoubis_stat_message *)(msg->u.notify)->payload;
}

StatusNotify::~StatusNotify(void)
{
	/* Nothing special needs to be done here. */
}

bool
StatusNotify::extractValue(unsigned int subsystem, unsigned int key,
    wxString *result)
{
	u_int64_t	value;
	bool		wasFound;

	value = 0;
	wasFound = false;

	for (int i=0; i<valueNo_; i++) {
		if ((statMsg_->vals[i].subsystem == subsystem) &&
		    (statMsg_->vals[i].key == key)) {
			value = statMsg_->vals[i].value;
			wasFound = true;
		}
	}
	if (wasFound) {
		(*result) = wxString::Format(wxT("%lld"), value);
	} else {
		(*result) = _("no value");
	}

	return (wasFound);
}

bool
StatusNotify::hasAlfLoadtime(void)
{
	wxString result;
	return (extractValue(ANOUBIS_SOURCE_ALF, ALF_STAT_LOADTIME, &result));
}

wxString
StatusNotify::getAlfLoadtime(void)
{
	wxString result;

	extractValue(ANOUBIS_SOURCE_ALF, ALF_STAT_LOADTIME, &result);

	return (result);
}

bool
StatusNotify::hasSfsLoadtime(void)
{
	wxString result;
	return (extractValue(ANOUBIS_SOURCE_SFS, SFS_STAT_LOADTIME, &result));
}

wxString
StatusNotify::getSfsLoadtime(void)
{
	wxString result;

	extractValue(ANOUBIS_SOURCE_SFS, SFS_STAT_LOADTIME, &result);

	return (result);
}

wxString
StatusNotify::getSfsCsumRecalc(void)
{
	wxString result;

	extractValue(ANOUBIS_SOURCE_SFS, SFS_STAT_CSUM_RECALC, &result);

	return (result);
}

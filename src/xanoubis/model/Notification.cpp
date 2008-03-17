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
		default:
			module_ = wxT("(unknown)");
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

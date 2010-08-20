/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <wx/intl.h>

#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#endif

#include <anoubis_msg.h>
#include <anoubis_protocol.h>
#include <anoubis_playground.h>
#include <linux/anoubis_playground.h>

#include "Notification.h"
#include "PlaygroundFileNotify.h"


PlaygroundFileNotify::PlaygroundFileNotify(struct anoubis_msg *msg) :
    Notification(msg)
{
	const char* prefix = NULL;
	char* prefixfail = NULL;
	char* path = NULL;
	int   rc;

	/* get the containing kernel message */
	int offset = get_value(msg->u.notify->evoff);
	filemsg_ = (struct pg_file_message*)
	    (msg->u.notify->payload+offset);

	/* expand the path value (user friendly version of path) */
	prefix = pgfile_resolve_dev(filemsg_->dev);
	if (!prefix) {
		rc = asprintf(&prefixfail, "<dev %"PRIx64">", filemsg_->dev);
		(void)rc;
		prefix = prefixfail;
	}

	path = strdup(filemsg_->path);

	if (prefix && path) {
		pgfile_normalize_file(path);

		path_ = wxString::FromAscii(prefix) +
		    wxString::FromAscii(path);
	}

	if (prefixfail) {
		free(prefixfail);
	}
	if (path) {
		free(path);
	}
}

PlaygroundFileNotify::~PlaygroundFileNotify(void)
{
	/* Nothing to be done here. */
}

wxString
PlaygroundFileNotify::getLogMessage(void)
{
	return wxString::Format(_("Commit request for file %ls"),
	    path_.c_str());
}

wxString
PlaygroundFileNotify::getPath(void)
{
	return path_;
}

uint64_t
PlaygroundFileNotify::getPgId(void) {
	return filemsg_->pgid;
}

uint64_t
PlaygroundFileNotify::getDevice(void) {
	return filemsg_->dev;
}

uint64_t
PlaygroundFileNotify::getInode(void) {
	return filemsg_->ino;
}

const char*
PlaygroundFileNotify::getRawPath(void) {
	return filemsg_->path;
}

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

#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#include <linux/anoubis_playground.h>
#endif
#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#endif

#include <anoubis_msg.h>
#include <anoubis_protocol.h>

#include "Notification.h"
#include "PlaygroundFileNotify.h"


PlaygroundFileNotify::PlaygroundFileNotify(struct anoubis_msg *msg) :
    Notification(msg)
{
	/* get the containing kernel message */
	int offset = get_value(msg->u.notify->evoff);
	filemsg_ = (struct pg_file_message*)
	    (msg->u.notify->payload+offset);
}

PlaygroundFileNotify::~PlaygroundFileNotify(void)
{
	/* Nothing to be done here. */
}

uint64_t
PlaygroundFileNotify::getPgId(void) {
#ifdef LINUX
	return filemsg_->pgid;
#else
	return 0;
#endif
}

uint64_t
PlaygroundFileNotify::getDevice(void) {
#ifdef LINUX
	return filemsg_->dev;
#else
	return 0;
#endif
}

uint64_t
PlaygroundFileNotify::getInode(void) {
#ifdef LINUX
	return filemsg_->ino;
#else
	return 0;
#endif
}

const char*
PlaygroundFileNotify::getFilePath(void) {
#ifdef LINUX
	return filemsg_->path;
#else
	return NULL;
#endif
}

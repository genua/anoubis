/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <anoubisd.h>
#include <amsg.h>

#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis_sfs.h>
#else
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#include <sys/anoubis_alf.h>
#include <sys/anoubis_sfs.h>
#endif

/**
 * Layout of anoubis_event_common before kernel protocol version
 * 10004.
 */
struct anoubis_event_common_10004 {
	anoubis_cookie_t task_cookie;
};

/**
 * This function converts older kernel events to the structure layout
 * expected by the current ANOUBISCORE_VERSION. If the message must be
 * converted, a new message must be created and the memory allocated for
 * the old message must be freed.
 * NOTE: The old message has not yet been passed through amsg_verify!
 *
 * @param old The old message.
 * @param version The ANOUBISCORE_VERSION reported by the kernel.
 * @return The converted message or NULL if an error occurs.
 */
struct anoubisd_msg *
compat_get_event(struct anoubisd_msg *old, unsigned long version)
{
	int					 pre, post, total;
	struct anoubisd_msg			*n;
	struct anoubis_event_common		*common;
	struct anoubis_event_common_10004	*oldcommon;
	struct eventdev_hdr			*hdr;

	if (version < 0x10001UL) {
		log_warnx("compat_get_event: Version %lx is too old", version);
		free(old);
		return  NULL;
	}
	/*
	 * If we convert to 0x00010004UL there is nothing todo.
	 */
	if (ANOUBISCORE_VERSION == 0x00010004UL)
		return old;
	/*
	 * The rest of this conversion function assumes that the current
	 * ANOUBISCORE_VERSION is 0x00010005UL or 0x00010006UL.
	 * Abort if this is not the case.
	 */
	if (ANOUBISCORE_VERSION != 0x10005UL &&
	    ANOUBISCORE_VERSION != 0x10006UL) {
		log_warnx("compat_get_event: Current verion is %lx but we "
		    "only support 0x00010005 and 0x00010006",
		    ANOUBISCORE_VERSION);
		free(old);
		return NULL;
	}
	total = old->size - sizeof(struct anoubisd_msg);
	pre = sizeof(struct eventdev_hdr);
	if (total < pre + (int)sizeof(struct anoubis_event_common_10004)) {
		log_warnx("compat_get_event: Dropping short message");
		free(old);
		return NULL;
	}
	post = total - pre - sizeof(struct anoubis_event_common_10004);
	n = msg_factory(ANOUBISD_MSG_EVENTDEV,
	    pre + sizeof(struct anoubis_event_common) + post);
	if (n == NULL) {
		free(old);
		log_warnx("compat_get_event: Out of memory");
		return NULL;
	}
	memcpy(n->msg, old->msg, pre);
	oldcommon = (struct anoubis_event_common_10004 *)(old->msg + pre);
	common = (struct anoubis_event_common *)(n->msg + pre);
	common->task_cookie = oldcommon->task_cookie;
#if ANOUBISCORE_VERSION >= ANOUBISCORE_PG_VERSION
	common->pgid = 0;
#endif
	hdr = (struct eventdev_hdr *)n->msg;
	hdr->msg_size += n->size - old->size;
	memcpy(common+1, oldcommon+1, post);
	free(old);
	return n;
}

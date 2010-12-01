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
#include <linux/anoubis_playground.h>
#else
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#include <sys/anoubis_alf.h>
#include <sys/anoubis_sfs.h>
#include <bsdcompat/dev/anoubis_playground.h>
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

#if ANOUBISCORE_VERSION > 0x10006UL
#error "compat_get_event function needs to be extended for compatibility"
#endif

	if (version == 0x10005UL) {
		/*
		 * We 'just' need to add another trailing 0-byte to
		 * pg_file_message->path
		 */
		int msg_size;
		struct pg_file_message *pg;
		char *end;
		hdr = (struct eventdev_hdr *)old->msg;

		if (old->size < (int)sizeof(struct eventdev_hdr)) {
			log_warnx("compat_get_event: Dropping short message");
			free(old);
			return NULL;
		}
		else if (hdr->msg_source != ANOUBIS_SOURCE_PLAYGROUNDFILE) {
			return old;
		} else if (old->size < (int)sizeof(struct eventdev_hdr) +
		    (int)sizeof(struct pg_file_message)) {
			log_warnx("compat_get_event: "
			    "Dropping short pg_file_message");
			free(old);
			return NULL;
		}

		n = msg_factory(ANOUBISD_MSG_EVENTDEV, old->size + 1);
		if (n == NULL) {
			free(old);
			log_warnx("compat_get_event: Out of memory");
			return NULL;
		}
		memcpy(n->msg, old->msg, old->size);
		msg_size = hdr->msg_size + 1;
		hdr = (struct eventdev_hdr *)n->msg;
		hdr->msg_size = msg_size;
		pg = (struct pg_file_message*)(n->msg +
		    sizeof(struct eventdev_hdr));
		free(old);

		/*
		 * Now actually add the 0-byte.
		 * We can't simply use msg_size for the position because of
		 * potential padding.
		 */
		end = memchr(pg->path, 0, msg_size - sizeof(struct pg_file_message));
		if (!end) {
			/* pg->path was not 0-terminated */
			free(n);
			return NULL;
		}
		end++;
		*end = 0;
		return n;
	} else if (ANOUBISCORE_VERSION == 0x00010004UL) {
		/*
		 * If we convert to 0x00010004UL there is nothing todo.
		 */
		return old;
	} else if (version >= 0x10001UL && version <= 0x10003UL) {
		total = old->size - sizeof(struct anoubisd_msg);
		pre = sizeof(struct eventdev_hdr);
		if (total <
		    pre + (int)sizeof(struct anoubis_event_common_10004)) {
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
		oldcommon = (struct anoubis_event_common_10004 *)(old->msg +
		    pre);
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
	} else if (version < 0x10001UL) {
		log_warnx("compat_get_event: Version %lx is too old", version);
		free(old);
		return  NULL;
	} else {
		log_warnx("compat_get_event: don't know how to convert "
		    "version %ld to %ld", version, ANOUBISCORE_VERSION);
		free(old);
		return NULL;
	}
}

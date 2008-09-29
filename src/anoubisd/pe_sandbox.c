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

#include "config.h"

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>

#include <apn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef LINUX
#include <queue.h>
#include <bsdcompat.h>
#include <linux/anoubis_sfs.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/queue.h>
#include <dev/anoubis_sfs.h>
#include <dev/anoubis.h>
#endif

#include <anoubis_protocol.h>

#include "anoubisd.h"
#include "sfs.h"
#include "pe.h"

anoubisd_reply_t *
pe_decide_sandbox(uid_t uid, anoubisd_msg_sfsopen_t *sfsmsg, time_t now)
{
	struct eventdev_hdr	*hdr;
	struct sfs_open_message	*msg;
	anoubisd_reply_t	*reply = NULL;
	char			*path = "";
	char			 cs[2*ANOUBIS_CS_LEN+1];

	uid = uid; now = now; /* XXX */
	hdr = &sfsmsg->hdr;
	msg = (struct sfs_open_message *)(hdr+1);

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_handle_sandbox: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}
	reply->ask = 0;
	reply->reply = 0;

	/* XXX CEH: Missing input validation! */
	if ((msg->flags & ANOUBIS_OPEN_FLAG_FOLLOW) == 0)
		return reply;
	cs[0] = 0;
	if (msg->flags & ANOUBIS_OPEN_FLAG_CSUM) {
		int i;
		for (i=0; i<ANOUBIS_CS_LEN; ++i)
			sprintf(cs+2*i, "%02x", msg->csum[i]);
	}
	if (msg->flags & ANOUBIS_OPEN_FLAG_PATHHINT)
		path = msg->pathhint;
	log_warnx("Follow link: path=%s csum=%s", path, cs);
	return reply;
}

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
#include <dev/anoubis_alf.h>
#include <dev/anoubis_sfs.h>
#endif

/*
 * This function converts older kernel events to the structure layout
 * expected by the current ANOUBISCORE_VERSION. If the message must be
 * converted, a new message must be created and the memory allocated for
 * the old message must be freed.
 * NOTE: The old message has not yet been passed through amsg_verify!
 */
anoubisd_msg_t *
compat_get_event(anoubisd_msg_t *old, unsigned long version)
{
	/*
	 * This conversion function assumes that the current
	 * ANOUBISCORE_VERSION is 0x00010004UL. Abort if this is not the case.
	 */
	if (ANOUBISCORE_VERSION != 0x10005UL) {
		log_warnx("compat_get_event: Current verion is %lx but we "
		    "only support 0x00010005", ANOUBISCORE_VERSION);
		free(old);
		return NULL;
	}
	if (version < 0x10001UL) {
		log_warnx("compat_get_event: Version %lx is too old", version);
		free(old);
		return  NULL;
	}
	/*
	 * There have been no structure layout changes between versions
	 * 0x00010001UL and 0x00010005.
	 */
	return  old;
}

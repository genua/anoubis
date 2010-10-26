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

#ifndef _COMPAT_ANOUBIS_PLAYGROUND_H_
#define _COMPAT_ANOUBIS_PLAYGROUND_H_

#include "config.h"

#ifdef OPENBSD

#include <dev/anoubis.h>

#define ANOUBIS_PLAYGROUND_OP_OPEN      1
#define ANOUBIS_PLAYGROUND_OP_RENAME    2
#define ANOUBIS_PLAYGROUND_OP_SCAN      3

struct pg_open_message {
	struct anoubis_event_common	common;
	u_int32_t			op;
	u_int32_t			mode;
	char				pathbuf[0];
};

struct pg_proc_message {
	struct anoubis_event_common	common;
};

#define ANOUBIS_PGFILE_INSTANTIATE	1
#define ANOUBIS_PGFILE_DELETE		2
#define ANOUBIS_PGFILE_SCAN		3

struct pg_file_message {
	struct anoubis_event_common	common;
	anoubis_cookie_t		pgid;
	u_int64_t			dev;
	u_int64_t			ino;
	int				op;
	char				path[0];
};

#endif

#endif	/* _COMPAT_ANOUBIS_PLAYGROUND_H_ */

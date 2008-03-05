/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#endif
#include "queue.h"

#define ANOUBISD_OPT_VERBOSE		0x0001
#define ANOUBISD_OPT_VERBOSE2		0x0002
#define ANOUBISD_OPT_NOACTION		0x0004

#define ANOUBISD_USER			"_anoubisd"

#define ANOUBISD_SOCKETNAME		"/var/run/anoubisd.sock"

#ifdef LINUX
#define __dead
#endif

struct anoubisd_config {
	int	opts;
	const char * unixsocket;	/* defaults to ANOUBISD_SOCKETNAME */
};

struct anoubisd_event_in {
	TAILQ_ENTRY(anoubisd_event_in) events;

	int event_size;
	struct eventdev_hdr hdr;
	char msg[0];
};

struct anoubisd_event_out {
	TAILQ_ENTRY(anoubisd_event_out) events;

	struct eventdev_reply reply;
};

enum {
	PROC_MAIN,
	PROC_POLICY,
	PROC_SESSION
} anoubisd_process;

pid_t	session_main(struct anoubisd_config *, int[], int[], int[]);
pid_t	policy_main(struct anoubisd_config *, int[], int[], int[]);
void	log_init(int);
void	log_warn(const char *, ...);
void	log_warnx(const char *, ...);
void	log_info(const char *, ...);
void	log_debug(const char *, ...);
void	fatalx(const char *);
void	fatal(const char *);

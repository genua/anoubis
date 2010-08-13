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

#ifndef _APNCMD_H_
#define _APNCMD_H_

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include "anoubis_apnvm.h"

#define APNCMD_MAXARGS		20

struct apncmd {
	pid_t	 pid;		/* PID of child process. */
	char	*workdir;	/* NULL if not chdir required. */
	char	*fullpath;	/* Full path to binary. */
	int	 argc;		/* Number of arguments. */
	char	*argv[APNCMD_MAXARGS];	/* Argument vector. */
	apnvm_pidcallback_t	pidcallback;
	struct sigaction	oldact;
	char			restoresigact;
};

extern struct apncmd	*apncmd_create(const char *workdir,
			     const char *fullpath, const char *cmdarg, ...)
			     __attribute__((sentinel));
extern void		 apncmd_set_pidcallback(struct apncmd *,
			     apnvm_pidcallback_t);
extern void		 apncmd_free(struct apncmd *);

extern int		 apncmd_start(struct apncmd *);
extern int		 apncmd_start_pipe(struct apncmd *);
extern int		 apncmd_wait(struct apncmd *);

#endif	/* _APNCMD_H_ */

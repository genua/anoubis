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

#include "config.h"

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/ioctl.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef LINUX
#include <linux/anoubis.h>
#include <bsdcompat.h>

int
playground_setMarker(void)
{
	int fd;
	int rc;

	fd = open(_PATH_DEV "anoubis", O_RDONLY);
	if (fd < 0) {
		return (-errno);
	}

	rc = ioctl(fd, ANOUBIS_CREATE_PLAYGROUND, 0);
	close(fd);
	if (rc != 0) {
		rc = -errno;
	}

	return (rc);
}

int
playground_start_exec(char **argv)
{
	int rc;

	rc = playground_setMarker();
	if (rc != 0) {
		return (rc);
	}

	execvp(argv[0], argv);
	/* Only reached if execvp returns an error. */
	return (-errno);
}

int
playground_start_fork(char **argv)
{
	int	rc  = 0;
	pid_t	pid = 0;

	if ((pid = fork()) < 0) {
		return (-errno);
	}

	if (pid == 0) {
		/* Child */
		rc = playground_start_exec(argv);
		/* Only reached if playground_start_exec returns an error. */
		exit(1);
	} else {
		/* Parent */
		rc = pid;
	}

	return (rc);
}

#endif /* LINUX */

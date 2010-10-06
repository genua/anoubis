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

#include "anoubis_playground.h"

#ifdef LINUX
#include <linux/anoubis.h>
#include <bsdcompat.h>

/**
 * Size of message between child and parend in case of error.
 * Specifies the number of bytes.
 */
#define MSG_SIZE 16

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
	int	i   = 0;
	int	rc  = 0;
	pid_t	pid = 0;
	int	pipes[2];
	char	buffer[MSG_SIZE];

	if (pipe(pipes) == -1) {
		return (-errno);
	}

	if ((pid = vfork()) < 0) {
		return (-errno);
	}

	bzero(buffer, MSG_SIZE);
	if (pid == 0) {
		/* Child */
		close(pipes[0]);
		fcntl(pipes[1], F_SETFD, FD_CLOEXEC);

		/* Mark ourself as playground process. */
		rc = playground_setMarker();
		if (rc != 0) {
			/* Error: inform parent and exit! */
			snprintf(buffer, MSG_SIZE, "%d\n", rc);
			rc = write(pipes[1], buffer, MSG_SIZE);
			close(pipes[1]);
			_exit(EXIT_FAILURE);
		}

		/*
		 * Close open filehandles except stdin, stdout, stderr.
		 * Also do not close pipe to parend. It's been closed on
		 * exec(SUCCESS) (see fcntl(FD_CLOEXEC)).
		 */
		i = 3;
		do {
			if (i != pipes[1]) {
				close(i);
			}
			i++;
		} while (i < sysconf(_SC_OPEN_MAX));

		execvp(argv[0], argv);
		/*
		 * Only reached if execvp() returns an error.
		 * Inform parent and exit!
		 */
		snprintf(buffer, MSG_SIZE, "%d\n", -errno);
		rc = write(pipes[1], buffer, MSG_SIZE);
		close(pipes[1]);
		_exit(EXIT_FAILURE);
	} else {
		/* Parent */
		close(pipes[1]);

		/* Was child successfull? */
		rc = read(pipes[0], buffer, MSG_SIZE);
		if (rc == 0) {
			/* Child just closed pipe, no error sent. */
			rc = pid;
		} else {
			/* Received error. */
			sscanf(buffer, "%d", &rc);
		}
		close(pipes[0]);
		/*
		 * Give the child time to complete its startup. This
		 * is mainly useful in the GUI which wants to see the
		 * program name after the exec in the playground list.
		 */
		usleep(100000);
	}

	return (rc);
}

#endif /* LINUX */

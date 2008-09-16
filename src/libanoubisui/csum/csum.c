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

#include <config.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#ifdef LINUX
#include <linux/anoubis.h>
#endif

#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include "csum.h"

int
anoubis_csum_calc(const char *file, u_int8_t * csbuf, int *cslen)
{
	int ret, fd, afd;
	struct anoubis_ioctl_csum cs;

	if (*cslen < ANOUBIS_CS_LEN)
		return -EINVAL;
	afd = open("/dev/anoubis", O_RDONLY);
	if (afd < 0)
		return -errno;
	fd = open(file, O_RDONLY);
	if (fd < 0) {
		ret = -errno;
		close(afd);
		return ret;
	}
	cs.fd = fd;
	if (ioctl(afd, ANOUBIS_GETCSUM, &cs) < 0) {
		ret = -errno;
		close(fd);
		close(afd);
		return ret;
	}
	close(fd);
	close(afd);
	memcpy(csbuf, cs.csum, ANOUBIS_CS_LEN);
	*cslen = ANOUBIS_CS_LEN;
	return 0;
}

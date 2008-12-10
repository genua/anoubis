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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <anoubis_msg.h>

#ifdef LINUX
#include <linux/anoubis.h>
#include <bsdcompat.h>
#endif

#ifdef OPENBSD
#include <sha2.h>
#include <dev/anoubis.h>
#endif

#include <openssl/sha.h>

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

int
anoubis_csum_link_calc(const char *link, u_int8_t * csbuf, int *cslen)
{
	SHA256_CTX	shaCtx;
	char		buf[PATH_MAX];
	struct stat	sb;
	ssize_t		ret;

	if (*cslen < ANOUBIS_CS_LEN)
		return -EINVAL;

	if(lstat(link, &sb) < 0)
		return -errno;
	if (!S_ISLNK(sb.st_mode))
		return anoubis_csum_calc(link, csbuf, cslen);

	if ((ret = readlink(link, buf, PATH_MAX)) != -1)
		buf[ret] = '\0';
	SHA256_Init(&shaCtx);
	SHA256_Update(&shaCtx, buf, ret);
	SHA256_Final(csbuf, &shaCtx);

	*cslen = ANOUBIS_CS_LEN;
	return 0;
}

char**
anoubis_csum_list(struct anoubis_msg *m, int *listcnt)
{
	char	**result = NULL;
	char	**tmp = NULL;
	int	  cnt,
		  end,
		  msg_end,
		  name_size;

	if (m == NULL || listcnt == NULL)
		goto err;

	/* Return Message from Daemon cannot return partial names */
	end = cnt = 0;
	while (m) {
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_ChecksumPayloadMessage))) {
			goto err;
		}

		msg_end = m->length - sizeof(Anoubis_ChecksumPayloadMessage)
		    - CSUM_LEN - 1;
		if (msg_end < 1 ||
		    m->u.checksumpayload->payload[msg_end] != '\0') {
			goto err;
		}
		end = 0;
		while (end <= msg_end) {
			name_size = strlen(
			    (char *)&m->u.checksumpayload->payload[end]) + 1;
			cnt++;
			tmp = (char **)realloc(result, cnt * sizeof(char *));
			if (!tmp) {
				cnt--;
				goto err;
			}
			result = tmp;
			result[cnt-1] = calloc(name_size, sizeof(char));
			if (!result)
				goto err;
			strlcpy(result[cnt-1],
			    (char *)&m->u.checksumpayload->payload[end],
			    name_size);
			end += name_size;
		}
		m = m->next;
	}

	*listcnt = cnt;
	return result;

err:
	if (result) {
		for(end = 0; end < cnt; end++) {
			if (result[end])
				free(result[end]);
		}
		free(result);
	}

	*listcnt = 0;
	return NULL;
}

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

#ifndef _COMPAT_OPENAT_H_
#define _COMPAT_OPENAT_H_

#include <config.h>

#ifndef OPENBSD
#include <bsdcompat.h>
#endif

struct atfd {
#ifdef LINUX
	int	 atfd;
#endif
#ifdef OPENBSD
	char	*prefix;
#endif
};

static inline int
atfd_open(struct atfd *atfd, const char *path)
{
#ifdef LINUX
	atfd->atfd = open(path, O_RDONLY | O_DIRECTORY);
	if (atfd->atfd < 0)
		return -errno;
	return 0;
#endif
#ifdef OPENBSD
	atfd->prefix = strdup(path);
	if (atfd->prefix == NULL)
		return -ENOMEM;
	return 0;
#endif
}

static inline int
atfd_isopen(struct atfd *atfd)
{
#ifdef LINUX
	return (atfd->atfd >= 0);
#endif
#ifdef OPENBSD
	return (atfd->prefix != NULL);
#endif
}

static inline int
atfd_close(struct atfd *atfd)
{
#ifdef LINUX
	int fd = atfd->atfd;
	atfd->atfd = -1;
	return close(fd);
#endif
#ifdef OPENBSD
	free(atfd->prefix);
	atfd->prefix = NULL;
	return 0;
#endif
}

#ifdef OPENBSD
static char *
atfd_openat_path(const char *prefix, const char *path)
{
	char	*result;
	int	 len;

	len = strlen(prefix) + strlen(path) + 2;
	result = malloc(len);
	if (result == NULL)
		return result;
	strlcpy(result, prefix, len);
	strncat(result, "/", len);
	strncat(result, path, len);
	return result;
}

#endif

static inline int
atfd_openat(struct atfd *atfd, const char *path, int flags, int mode)
{
#ifdef LINUX
	return openat(atfd->atfd, path, flags, mode);
#endif
#ifdef OPENBSD
	char *fullpath = atfd_openat_path(atfd->prefix, path);
	int ret;

	if (fullpath == NULL) {
		errno = ENOMEM;
		return -1;
	}
	ret = open(fullpath, flags, mode);
	free(fullpath);
	return ret;
#endif
}

static inline int
atfd_unlinkat(struct atfd *atfd, const char *path)
{
#ifdef LINUX
	return unlinkat(atfd->atfd, path, 0);
#endif
#ifdef OPENBSD
	char *fullpath = atfd_openat_path(atfd->prefix, path);
	int ret;

	if (fullpath == NULL) {
		errno = ENOMEM;
		return -1;
	}
	ret = unlink(fullpath);
	free(fullpath);
	return ret;
#endif
}

static inline DIR *
atfd_fdopendir(struct atfd *atfd)
{
#ifdef LINUX
	int	dirfd = openat(atfd->atfd, ".", O_RDONLY|O_DIRECTORY);
	if (dirfd < 0)
		return NULL;
	return fdopendir(dirfd);
#endif
#ifdef OPENBSD
	return opendir(atfd->prefix);
#endif
}

#endif	/* _COMPAT_OPENAT_H_ */

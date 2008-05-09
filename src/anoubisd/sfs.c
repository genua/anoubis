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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <strings.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#ifdef LINUX
#include <linux/anoubis_sfs.h>
#include <bsdcompat.h>
#else
#include <dev/anoubis_sfs.h>
#endif

#ifdef OPENBSD
#include <sha2.h>
#else
#include <openssl/sha.h>
#endif

#include <anoubis_protocol.h>

#include "anoubisd.h"

int
sfs_sha256(const char * filename, unsigned char md[SHA256_DIGEST_LENGTH])
{
	SHA256_CTX c;
	int fd;
	static char buf[40960];

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return fd;
	SHA256_Init(&c);
	while(1) {
		int ret = read(fd, buf, sizeof(buf));
		if (ret == 0)
			break;
		if (ret < 0) {
			close(fd);
			return -1;
		}
		SHA256_Update(&c, buf, ret);
	}
	SHA256_Final(md, &c);
	close(fd);
	return 0;
}

int
mkpath(const char *path)
{
	char *tmppath;
	int i;
	int len;

	len = strlen(path) + 1;

	tmppath = malloc(len);
	if (tmppath == NULL)
		return -ENOMEM;

	bzero(tmppath, len);

	for (i = 1; i < len; i++) {
		if (path[i] == '/' || path[i] == '\0') {
			strlcpy(tmppath, path, i+1);
			if (mkdir(tmppath, 0700) == -1) {
				if (errno != EEXIST) {
					free(tmppath);
					return -errno;
				}
			}
		}
		if (path[i] == '\0')
			break;
	}

	free(tmppath);
	return 0;
}

int
sfs_checksumop(const char *path, unsigned int operation, uid_t uid)
{
	char *csum_path;
	int ret = -1;
	int len;

	if (!path || path[0] != '/')
		return -EINVAL;

	if(strstr(path, "/../") != NULL)
		return -EINVAL;

	if (strlen(path) >= strlen("/..")) {
		if (!strncmp(path + strlen(path) - strlen("/.."), "/..",
		    strlen("/.."))) {
			return -EINVAL;
		}
	}

	/* XXX: 100 is length of uid as string */
	len = strlen(path) + strlen(SFS_CHECKSUMROOT) + 1 + 100;

	csum_path = malloc(len);
	if (csum_path == NULL)
		return -ENOMEM;

	bzero(csum_path, len);

	if (operation == ANOUBIS_CHECKSUM_OP_ADD) {
		unsigned char md[SHA256_DIGEST_LENGTH];
		int fd;
		int written = 0;

		ret = sfs_sha256(path, md);
		if (ret < 0) {
			free(csum_path);
			return ret;
		}

		bzero(csum_path, len);
		snprintf(csum_path, len - 1, "%s%s", SFS_CHECKSUMROOT, path);

		ret = mkpath(csum_path);
		if (ret < 0) {
			free(csum_path);
			return ret;
		}

		snprintf(csum_path, len - 1, "%s%s/%u", SFS_CHECKSUMROOT, path,
		    uid);

		fd = open(csum_path, O_WRONLY|O_CREAT, 0600);
		if (fd < 0) {
			free(csum_path);
			return -errno;
		}

		while (written < sizeof(md)) {
			ret = write(fd, md + written, sizeof(md) - written);
			if (ret < 0) {
				unlink(csum_path);
				ret = -errno;
				break;
			}
			written += ret;
		}
		ret = 0;
		close(fd);
	} else if (operation == ANOUBIS_CHECKSUM_OP_DEL) {
		snprintf(csum_path, len - 1, "%s%s/%u", SFS_CHECKSUMROOT, path,
		    uid);
		ret = unlink(csum_path);
	}

	free(csum_path);
	return ret;
}

int
sfs_getchecksum(const char *path, uid_t uid, unsigned char *md)
{
	char *csum_path;
	int len;
	int ret = 0;
	int fd;
	int bytes_read = 0;

	bzero(md, ANOUBIS_SFS_CS_LEN);

	/* XXX: 100 is length of uid as string */
	len = strlen(path) + strlen(SFS_CHECKSUMROOT) + 1 + 100;
	csum_path = malloc(len);
	if (csum_path == NULL)
		return -ENOMEM;

	bzero(csum_path, len);
	snprintf(csum_path, len - 1, "%s%s/%u", SFS_CHECKSUMROOT, path, uid);

	fd = open(csum_path, O_RDONLY);
	if (fd < 0) {
		free(csum_path);
		return -errno;
	}

	while (bytes_read < SHA256_DIGEST_LENGTH) {
		ret = read(fd, md + bytes_read,
		    SHA256_DIGEST_LENGTH - bytes_read);
		if (ret < 0) {
			ret = -errno;
			break;
		} else if (ret == 0) {
			ret = -EIO;
			break;
		}
		bytes_read += ret;
	}

	close(fd);

	free(csum_path);

	return ret;
}

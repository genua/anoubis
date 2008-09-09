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
#include "sfs.h"

static int	sfs_readchecksum(const char *csum_file, unsigned char *md);
static int	sfs_sha256(const char * filename,
		    unsigned char md[SHA256_DIGEST_LENGTH], uid_t auth_uid);
static int	convert_user_path(const char * path, char **dir);

/*
 * NOTE: This function is not reentrant for several reasons:
 * - Both credentials and buf can be quite large (65k supplementary group
 *   IDs on Linux) and are thus declared static.
 * - The switch_uid/restore_uid pair of functions is not reentrant either.
 */
static int
sfs_sha256(const char * filename, unsigned char md[SHA256_DIGEST_LENGTH],
    uid_t auth_uid)
{
	SHA256_CTX			c;
	int				fd;
	static struct credentials	savedcred;
	static char			buf[40960];

	if (switch_uid(auth_uid, &savedcred) < 0)
		return -1;
	fd = open(filename, O_RDONLY);
	if (restore_uid(&savedcred) < 0) {
		log_warnx("FATAL: Cannot restore credentials");
		master_terminate(EPERM);
		return -1;
	}
	if (fd < 0)
		return -1;
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

char *
insert_escape_seq(char *path)
{
	char *newpath = NULL;
	int k, i;
	int j = 1;

	i = strlen(path);
	k = 0;
	while (k <= i) {
		if (path[k] == '*')
			j++;
		k++;
	}

	newpath = (char *)malloc(strlen(path) + j + 1);
	if (newpath == NULL)
		return NULL;

	for (k = j = 0; j <= i; j++, k++) {
		if (path[j] == '*') {
			newpath[k] = '*';
			k++;
		}
		newpath[k] = path[j];
	}
	k = strlen(newpath);
	while (k > 0) {
		if (newpath[k] == '/')
			break;
		k--;
	}
	k++;
	memmove(&newpath[k+1], &newpath[k], ((strlen(newpath) + 1) - k));
	newpath[k] = '*';

	return newpath;
}

static int
convert_user_path(const char * path, char **dir)
{
	int	i;
	char	*newpath = NULL;

	*dir = NULL;
	if (!path || path[0] != '/')
		return -EINVAL;
	if (strstr(path, "/../") != NULL)
		return -EINVAL;
	if (strstr(path, "/./") != NULL)
		return -EINVAL;
	if (strstr(path, "//") != NULL)
		return -EINVAL;
	if (path[0] == '/' && path[1] == '.') {
		if (path[2] == 0)
			return -EINVAL;
		if (path[2] == '.' && path[3] == 0)
			return -EINVAL;
	}
	i = strlen(path) - 1;
	if (path[i] == '.') {
		if (path[i-1] == '/')
			return -EINVAL;
		if (path[i-1] == '.' && path[i-2] == '/')
			return -EINVAL;
	}
	if (path[i] == '/')
		return -EINVAL;

#ifdef OPENBSD
{
	newpath = insert_escape_seq((char *)path);
	if (newpath == NULL)
		return -ENOMEM;

	if (asprintf(dir, "%s%s", SFS_CHECKSUMROOT, newpath) == -1) {
		free(newpath);
		return -ENOMEM;
	}

	free(newpath);
	return 0;
}
#endif
#ifdef LINUX
{
	struct stat	 statbuf;
	dev_t		 dev;
	char		*tmppath = strdup(path);
	char		*tmp = NULL;
	int		 error = -EINVAL;
	int		 samei = -1;
	unsigned long	 major, minor;

	if (!tmppath)
		return -ENOMEM;
	if (lstat(tmppath, &statbuf) < 0) {
		error = -errno;
		goto err;
	}
	if (!S_ISREG(statbuf.st_mode))
		goto err;
	dev = statbuf.st_dev;
	if (!dev)
		goto err;

	while(i > 0) {
		while(tmppath[i] != '/')
			i--;
		/* Keep the slash */
		tmppath[i+1] = 0;
		if (lstat(tmppath, &statbuf) < 0) {
			error = -errno;
			goto err;
		}
		if (!S_ISDIR(statbuf.st_mode))
			goto err;
		if (statbuf.st_dev != dev)
			break;
		samei = i;
		i--;	/* Skip slah */
	}
	if (samei < 0)
		goto err;
	major = (dev >> 8);
	minor = (dev & 0xff);

	tmp = (char *)(path+samei);
	newpath = insert_escape_seq(tmp);
	if (newpath == NULL)
		return -ENOMEM;

	if (asprintf(dir, "%s/%lu:%lu%s",
	    SFS_CHECKSUMROOT, major, minor, newpath) == -1) {
		error = -ENOMEM;
		goto err;
	}
	free(tmppath);
	free(newpath);
	return 0;
err:
	free(tmppath);
	if (newpath)
		free(newpath);
	return error;
}
#endif
	return -EOPNOTSUPP;
}

int
sfs_checksumop(const char *path, unsigned int operation, uid_t uid,
    u_int8_t md[SHA256_DIGEST_LENGTH])
{
	char *csum_path = NULL, *csum_file = NULL;
	int ret;

	ret = convert_user_path(path, &csum_path);
	if (ret < 0)
		return ret;
	if (asprintf(&csum_file, "%s/%d", csum_path, uid) == -1) {
		ret = -ENOMEM;
		goto out;
	}
	if (operation == ANOUBIS_CHECKSUM_OP_ADD
	    || operation == ANOUBIS_CHECKSUM_OP_CALC) {
		int fd;
		int written = 0;

		ret = sfs_sha256(path, md, uid);
		if (ret < 0)
			goto out;
		ret = mkpath(csum_path);
		if (ret < 0)
			goto out;
		if (operation == ANOUBIS_CHECKSUM_OP_ADD) {
			fd = open(csum_file, O_WRONLY|O_CREAT|O_TRUNC, 0600);
			if (fd < 0) {
				ret = -errno;
				goto out;
			}

			while (written < SHA256_DIGEST_LENGTH) {
				ret = write(fd, md + written,
				    SHA256_DIGEST_LENGTH - written);
				if (ret < 0) {
					ret = -errno;
					unlink(csum_file);
					close(fd);
					goto out;
				}
				written += ret;
			}
			close(fd);
		}
		ret = 0;
	} else if (operation == ANOUBIS_CHECKSUM_OP_DEL) {
		ret = unlink(csum_file);
		if (ret < 0)
			ret = -errno;
	} else if (operation == ANOUBIS_CHECKSUM_OP_GET) {
		ret = sfs_readchecksum(csum_file, md);
	}

out:
	if (csum_path)
		free(csum_path);
	if (csum_file)
		free(csum_file);
	return ret;
}

static int
sfs_readchecksum(const char *csum_file, unsigned char *md)
{
	int fd;
	int ret = 0;
	int bytes_read = 0;

	fd = open(csum_file, O_RDONLY);
	if (fd < 0)
		return -errno;

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
	if (ret < 0)
		return ret;
	return 0;
}

int
sfs_getchecksum(u_int64_t kdev __used, const char *kpath, uid_t uid,
    unsigned char *md)
{
#ifdef LINUX
{
	char		*path;
	char		*newpath;
	int		 ret;
	unsigned long	 major, minor;

	/*
	 * XXX CEH: Ultimately the kernel should report major and minor
	 * XXX CEH: numbers. Currently this duplicates kernel source code.
	 */
	major = (kdev >> 20);
	minor = (kdev & ((1UL << 20) - 1));
	newpath = insert_escape_seq((char *)kpath);
	if (newpath == NULL)
		return -ENOMEM;

	if (asprintf(&path, "%s/%lu:%lu%s/%d",
	    SFS_CHECKSUMROOT, major, minor, newpath, uid) == -1) {
		free(newpath);
		return -ENOMEM;
	}
	ret = sfs_readchecksum(path, md);
	free(path);
	free(newpath);
	return ret;
}
#endif
#ifdef OPENBSD
{
	char	*path;
	char	*newpath;
	int	 ret;

	newpath = insert_escape_seq((char *)kpath);
	if (newpath == NULL)
		return -ENOMEM;

	if (asprintf(&path, "%s%s/%d", SFS_CHECKSUMROOT, newpath, uid) == -1) {
		free(newpath);
		return -ENOMEM;
	}

	ret = sfs_readchecksum(path, md);
	free(path);
	free(newpath);
	return ret;
}
#endif
	return -EOPNOTSUPP;
}

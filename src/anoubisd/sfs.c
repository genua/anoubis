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

#include <dirent.h>

#ifdef LINUX
#include <linux/anoubis_sfs.h>
#include <bsdcompat.h>
#else
#include <dev/anoubis_sfs.h>
#endif

#include <anoubis_protocol.h>

#include "anoubisd.h"
#include "sfs.h"

static int	sfs_readchecksum(const char *csum_file, unsigned char *md);
static int	sfs_deletechecksum(const char *csum_file);
static int	check_empty_dir(const char *path);
char *		insert_escape_seq(const char *path, int dir);

#if 0

/*
 * NOTE CEH: We keep this function ifdef'ed out for the moment because we
 * NOTE CEH: might need something like this again once we support file change
 * NOTE CEH: notifications based on checksums. However, PLEASE think twice
 * NOTE CEH: before reusing this function because its use is most likely a
 * NOTE CEH: hint that there is a design flaw somewhere.
 * NOTE CEH: Additionally this function might cause a DOS on the anoubisd
 * NOTE CEH: because calling open on a large file might trigger checksum
 * NOTE CEH: calculation in the kernel.
 */

/*
 * NOTE: This function is not reentrant for several reasons:
 * - Both credentials and buf can be quite large (65k supplementary group
 *   IDs on Linux) and are thus declared static.
 * - The switch_uid/restore_uid pair of functions is not reentrant either.
 */
static int
sfs_open(const char *filename, uid_t auth_uid)
{
	static struct credentials	savedcred;
	int fd;
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
	return fd;
}
#endif

int
check_for_uid(const char *path)
{
	struct stat sb;

	if (stat(path, &sb) == 0)
		return 1;
	else
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
			if (mkdir(tmppath, 0750) == -1) {
				if (errno != EEXIST) {
					int ret = -errno;
					free(tmppath);
					return ret;
				}
			} else {
				if (chown(tmppath, -1, anoubisd_gid) < 0) {
					int ret = -errno;
					free(tmppath);
					return ret;
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
insert_escape_seq(const char *path, int dir)
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

	if (!dir) {
		k = strlen(newpath);
		while (k > 0) {
			if (newpath[k] == '/')
				break;
			k--;
		}
		k++;
		memmove(&newpath[k+1], &newpath[k],
		    ((strlen(newpath) + 1) - k));
		newpath[k] = '*';
	}

	return newpath;
}

char *
remove_escape_seq(const char *name, int is_uid)
{
	char *newpath = NULL;
	unsigned int size =  strlen(name);
	unsigned int k = 0;
	unsigned int i, mod;
	int cnt = 0;
	int stars = 0;

	for (k = 0; k <= size; k++) {
		if (name[k] == '*')
			stars++;
	}

	cnt = stars / 2;
	mod = stars % 2;

	size = (strlen(name) - cnt) + 1;
	if (!mod)
		size++;
	newpath = (char *)malloc(size);
	if (!newpath)
		return NULL;

	for (k = 0, i = mod; i <= strlen(name) && k <= size; i++, k++ ) {
		if (name[i] == '*') {
			if (name[i+1] != '*') {
				continue;
			}
			i++;
		}
		newpath[k] = name[i];
	}
	k--;
	if (!mod && !is_uid) {
		newpath[k] = '/';
		k++;
	}
	newpath[k] = '\0';

	return newpath;
}

static int
__convert_user_path(const char * path, char **dir, int is_dir, int is_chroot)
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

	newpath = insert_escape_seq(path, is_dir);
	if (newpath == NULL)
		return -ENOMEM;

	if (asprintf(dir, "%s%s",
	    is_chroot ? SFS_CHECKSUMCHROOT : SFS_CHECKSUMROOT, newpath) == -1) {
		free(newpath);
		return -ENOMEM;
	}

	free(newpath);
	return 0;
}

int
convert_user_path(const char * path, char **dir, int is_dir)
{
	return __convert_user_path(path, dir, is_dir, 0);
}

#if 0	/* Not used yet. ifdef'ed out to avoid a warning */
static int
convert_user_path_chroot(const char * path, char **dir, int is_dir)
{
	return __convert_user_path(path, dir, is_dir, 1);
}
#endif

static int
__sfs_checksumop(const char *path, unsigned int operation, uid_t uid,
    u_int8_t md[ANOUBIS_CS_LEN], int is_chroot)
{
	char *csum_path = NULL, *csum_file = NULL;
	int ret;

	if (is_chroot && (operation != ANOUBIS_CHECKSUM_OP_GET))
		return -EPERM;
	ret = __convert_user_path(path, &csum_path, 0, is_chroot);
	if (ret < 0)
		return ret;

	if (asprintf(&csum_file, "%s/%d", csum_path, uid) == -1) {
		ret = -ENOMEM;
		goto out;
	}
	if (operation == ANOUBIS_CHECKSUM_OP_ADDSUM) {
		int fd;
		int written = 0;

		ret = mkpath(csum_path);
		if (ret < 0)
			goto out;
		fd = open(csum_file, O_WRONLY|O_CREAT|O_TRUNC, 0640);
		if (fd < 0) {
			ret = -errno;
			goto out;
		}
		if (fchown(fd, -1, anoubisd_gid) < 0) {
			ret = -errno;
			close(fd);
			unlink(csum_file);
			goto out;
		}

		while (written < ANOUBIS_CS_LEN) {
			ret = write(fd, md + written, ANOUBIS_CS_LEN - written);
			if (ret < 0) {
				ret = -errno;
				unlink(csum_file);
				close(fd);
				goto out;
			}
			written += ret;
		}
		close(fd);
		ret = 0;
	} else if (operation == ANOUBIS_CHECKSUM_OP_DEL) {
		ret = sfs_deletechecksum(csum_file);
		if (ret < 0)
			ret = -errno;
	} else if (operation == ANOUBIS_CHECKSUM_OP_GET) {
		ret = sfs_readchecksum(csum_file, md);
	} else {
		ret = -ENOSYS;
	}

out:
	if (csum_path)
		free(csum_path);
	if (csum_file)
		free(csum_file);
	return ret;
}

int
sfs_checksumop(const char *path, unsigned int operation, uid_t uid,
    u_int8_t md[ANOUBIS_CS_LEN])
{
	return __sfs_checksumop(path, operation, uid, md, 0);
}

int
sfs_checksumop_chroot(const char *path, unsigned int operation, uid_t uid,
    u_int8_t md[ANOUBIS_CS_LEN])
{
	return __sfs_checksumop(path, operation, uid, md, 1);
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

	while (bytes_read < ANOUBIS_CS_LEN) {
		ret = read(fd, md + bytes_read, ANOUBIS_CS_LEN - bytes_read);
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
__sfs_getchecksum(const char *path, uid_t uid, unsigned char *md, int is_chroot)
{
	char	*abspath;
	char	*newpath;
	int	 ret;

	newpath = insert_escape_seq(path, 0);
	if (newpath == NULL)
		return -ENOMEM;

	if (asprintf(&abspath, "%s%s/%d",
	    is_chroot ? SFS_CHECKSUMCHROOT : SFS_CHECKSUMROOT,
	    newpath, uid) == -1) {
		free(newpath);
		return -ENOMEM;
	}

	ret = sfs_readchecksum(abspath, md);
	free(abspath);
	free(newpath);
	return ret;
}

int
sfs_getchecksum(const char *path, uid_t uid, unsigned char *md)
{
	return __sfs_getchecksum(path, uid, md, 0);
}

int
sfs_getchecksum_chroot(const char *path, uid_t uid, unsigned char *md)
{
	return __sfs_getchecksum(path, uid, md, 1);
}

static int
sfs_deletechecksum(const char *csum_file)
{
	int	 ret = 0;
	int	 root_len = strlen(SFS_CHECKSUMROOT);
	int	 k, path_len;
	char	*tmppath = strdup(csum_file);

	ret = unlink(csum_file);
	if (ret < 0) {
		free(tmppath);
		return ret;
	}

	path_len = strlen(tmppath);
	k = path_len;
	while (k > root_len) {
		if (tmppath[k] == '/') {
			tmppath[k] = '\0';
			ret = check_empty_dir(tmppath);
			if (ret < 0)
				return ret;

			if (ret == 1)
				break;
			ret = rmdir(tmppath);
			if (ret < 0) {
				free(tmppath);
				return ret;
			}
		}
		k--;
	}

	free(tmppath);
	return 0;
}

static int
check_empty_dir(const char *path)
{
	DIR	*dir = NULL;
	int	 cnt = 0;

	dir = opendir(path);
	if (dir == NULL)
		return -1;

	while (readdir(dir) != NULL) {
		cnt++;
		if (cnt > 2) {
			closedir(dir);
			return 1;
		}
	}
	closedir(dir);
	return 0;
}

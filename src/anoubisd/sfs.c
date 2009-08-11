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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <strings.h>

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/objects.h>

#ifdef LINUX
#include <queue.h>
#include <linux/anoubis_sfs.h>
#include <bsdcompat.h>
#else
#include <sys/queue.h>
#include <dev/anoubis_sfs.h>
#endif

#include <anoubis_sig.h>
#include <anoubis_protocol.h>

#include "anoubisd.h"
#include "sfs.h"
#include "cert.h"

static int	 sfs_writechecksum(const char *csum_file, const char *csum_path,
		     u_int8_t *md, int len, int idlen);
static int	 sfs_readsigdata(const char *csum_file, void *buffer, int);
static int	 sfs_deletechecksum(const char *csum_file);
static int	 check_empty_dir(const char *path);

static int
mkpath(const char *path)
{
	char *tmppath;
	int i;
	int len;

	if (!path)
		return -EINVAL;

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

static char *
insert_escape_seq(const char *path, int dir)
{
	char *newpath = NULL;
	int k, j, size;
	int stars = 1;

	if (!path)
		return NULL;

	size = strlen(path);
	for (k = 0; k <= size; k++) {
		if (path[k] == '*')
			stars++;
	}

	newpath = (char *)malloc(size + stars + 1);
	if (newpath == NULL)
		return NULL;

	for (k = j = 0; j <= size; j++, k++) {
		if (path[j] == '*') {
			newpath[k] = '*';
			k++;
		}
		newpath[k] = path[j];
	}

	/* If the given path is not a file, then it is a new entry
	 * which has a odd number of stars to mark it as a entry
	 * to the shadowtree. So we add a star to the beginning
	 * of the name of the entry.
	 */
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
remove_escape_seq(const char *name)
{
	char *newpath = NULL;
	unsigned int size = 0;
	unsigned int k = 0;
	unsigned int i, mod;
	int cnt = 0;
	int stars = 0;

	if (!name)
		return NULL;

	size = strlen(name);
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
	if (!mod) {
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
	if (path[i] == '/') {
		/*
		* Special case: root-directory
		* This is the only directory where a trailing slash is allowed
		*/
		if (!is_dir || i)
			return -EINVAL;
	}

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

static int
__sfs_checksumop(const struct sfs_checksumop *csop, void **bufptr,
    int * buflen, int is_chroot)
{
	char		*csum_path = NULL, *csum_file, *keystring;
	int		 realsiglen = 0;
	int		 error = -EINVAL;
	struct cert	*cert = NULL;
	void		*sigdata = NULL;
	int		 siglen = 0;

	if (is_chroot && csop->op != ANOUBIS_CHECKSUM_OP_GET
	    && csop->op != ANOUBIS_CHECKSUM_OP_GETSIG)
		return -EPERM;

	error = __convert_user_path(csop->path, &csum_path, 0, is_chroot);
	if (error < 0)
		return error;

	switch(csop->op) {
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case ANOUBIS_CHECKSUM_OP_GET:
	case ANOUBIS_CHECKSUM_OP_DEL:
		error = -EPERM;
		if (csop->uid != csop->auth_uid && csop->auth_uid != 0)
			goto err1;
		error = -EINVAL;
		if (csop->op == ANOUBIS_CHECKSUM_OP_ADDSUM) {
			if (csop->sigdata == NULL
			    || csop->siglen != ANOUBIS_CS_LEN)
				goto err1;
		}
		error = -ENOMEM;
		if (asprintf(&csum_file, "%s/%d", csum_path, csop->uid) < 0)
			goto err1;
		break;
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
	case ANOUBIS_CHECKSUM_OP_GETSIG:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
		error = -EINVAL;
		if (csop->keyid == NULL)
			goto err1;
		error = -EPERM;
		cert = cert_get_by_keyid(csop->keyid, csop->idlen);
		if (cert == NULL)
			goto err1;
		if (cert->uid != csop->auth_uid && csop->auth_uid != 0)
			goto err1;
		realsiglen = EVP_PKEY_size(cert->pkey);
		error = -EINVAL;
		if (csop->op == ANOUBIS_CHECKSUM_OP_ADDSIG) {
			int ret;
			if (realsiglen + ANOUBIS_CS_LEN != csop->siglen)
				goto err1;
			ret = anoubisd_verify_csum(cert->pkey, csop->sigdata,
			    csop->sigdata + ANOUBIS_CS_LEN, realsiglen);
			if (ret != 1) {
				error = -EPERM;
				goto err1;
			}
		}
		error = -ENOMEM;
		keystring = anoubis_sig_key2char(csop->idlen, csop->keyid);
		if (keystring == NULL)
			goto err1;
		if (asprintf(&csum_file, "%s/k%s", csum_path, keystring) < 0) {
			free(keystring);
			goto err1;
		}
		free(keystring);
		break;
	default:
		error = -EINVAL;
		goto err1;
	}
	switch(csop->op) {
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
		error = sfs_writechecksum(csum_file, csum_path, csop->sigdata,
		    csop->siglen, 0);
		break;
	case ANOUBIS_CHECKSUM_OP_GET:
	case ANOUBIS_CHECKSUM_OP_GETSIG:
		error = -ENOMEM;
		siglen = ANOUBIS_CS_LEN + realsiglen;
		sigdata = malloc(siglen);
		if (sigdata == NULL)
			goto err2;
		error = sfs_readsigdata(csum_file, sigdata, siglen);
		if (error == 0 && csop->op == ANOUBIS_CHECKSUM_OP_GETSIG) {
			int ret;
			ret = anoubisd_verify_csum(cert->pkey, sigdata,
			    sigdata + ANOUBIS_CS_LEN, realsiglen);
			if (ret != 1)
				error = -EPERM;
		}
		break;
	case ANOUBIS_CHECKSUM_OP_DEL:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
		error = sfs_deletechecksum(csum_file);
		break;
	}
	if (error < 0)
		goto err3;
	(*bufptr) = sigdata;
	(*buflen) = siglen;
	free(csum_file);
	free(csum_path);
	return 0;
err3:
	if (sigdata)
		free(sigdata);
err2:
	free(csum_file);
err1:
	free(csum_path);
	return error;
}

int
__sfs_checksumop_wrapper(const char *path, unsigned int operation, uid_t uid,
    u_int8_t *md, u_int8_t **sign, int *siglen, int len, int idlen, int chroot)
{
	struct sfs_checksumop		 csop;
	int				 ret;
	void				*buffer = NULL;
	int				 buflen = 0;

	csop.op = operation;
	csop.uid = uid;
	csop.auth_uid = uid;
	csop.path = (void*)path;
	csop.idlen = idlen;
	if (idlen) {
		csop.keyid = md;
	} else {
		csop.keyid = NULL;
	}
	csop.siglen = len;
	csop.sigdata = md + idlen;
	ret = __sfs_checksumop(&csop, &buffer, &buflen, chroot);
	if (ret != 0)
		return ret;
	switch(operation) {
	case ANOUBIS_CHECKSUM_OP_GET:
		if (buflen != len || buflen != ANOUBIS_CS_LEN) {
			free(buffer);
			return -EFAULT;
		}
		memcpy(md, buffer, ANOUBIS_CS_LEN);
		free(buffer);
		break;
	default:
		if (sign && siglen) {
			(*sign) = buffer;
			(*siglen) = buflen;
		}
		break;
	}
	return ret;
}

int
sfs_checksumop(const char *path, unsigned int operation, uid_t uid,
    u_int8_t *md, u_int8_t **sign, int *siglen, int len, int idlen)
{
	return __sfs_checksumop_wrapper(path, operation, uid, md, sign,
	    siglen, len, idlen, 0);
}

int
sfs_checksumop_chroot(const char *path, unsigned int operation, uid_t uid,
    u_int8_t *md, u_int8_t **sign, int *siglen, int len, int idlen)
{
	return __sfs_checksumop_wrapper(path, operation, uid, md, sign,
	    siglen, len, idlen, 1);
}

/* We going through the dirctory looking for registered uids and
 * update them
 */
int
sfs_update_all(const char *path, u_int8_t *md, int len)
{
	DIR		*sfs_dir = NULL;
	struct dirent	*sfs_ent = NULL;
	char		 testarg;
	char		*csum_file = NULL;
	char		*sfs_path;
	unsigned int	 uid;
	int		 ret;

	if (path == NULL || md == NULL || len != ANOUBIS_CS_LEN)
		return -EINVAL;
	ret = convert_user_path(path, &sfs_path, 0);
	if (ret < 0)
		return ret;
	sfs_dir = opendir(sfs_path);
	if (sfs_dir == NULL) {
		/* since we just update we don't create new one */
		ret = errno;
		if (errno == ENOENT)
			ret = 0;
		free(sfs_path);
		return -ret;
	}
	while ((sfs_ent = readdir(sfs_dir)) != NULL) {
		/* since we updating checksums we skip keyid entries */
		if (strcmp(sfs_ent->d_name, ".") == 0 ||
		    strcmp(sfs_ent->d_name, "..") == 0 ||
		    (sfs_ent->d_name[0] == 'k'))
			continue;
		if (sscanf(sfs_ent->d_name, "%u%c", &uid, &testarg) != 1)
			continue;
		if (asprintf(&csum_file, "%s/%d", sfs_path, uid) == -1) {
			closedir(sfs_dir);
			free(sfs_path);
			return -ENOMEM;
		}
		ret = sfs_writechecksum(csum_file, sfs_path, md, len, 0);
		if (ret)
			log_warnx("Cannot update checksum for %s (uid %d): "
			    "error = %d\n", path, uid, ret);
		free(csum_file);
		csum_file = NULL;
	}
	free(sfs_path);
	closedir(sfs_dir);
	return 0;
}


static int
sfs_writechecksum(const char *csum_file, const char *csum_path, u_int8_t *md,
    int len, int idlen)
{
	int ret = 0, written = 0;
	int fd;
	char *csum_tmp = NULL;

	if (md == NULL || csum_file == NULL || csum_path == NULL) {
		ret = -EINVAL;
		return ret;
	}
	if (len < ANOUBIS_CS_LEN) {
		ret = -EINVAL;
		return ret;
	}
	ret = mkpath(csum_path);
	if (ret < 0) {
		return ret;
	}

	if (asprintf(&csum_tmp, "%s%s", csum_file, ".tmp") == -1) {
		ret = -ENOMEM;
		return ret;
	}

	fd = open(csum_tmp, O_WRONLY|O_CREAT|O_TRUNC, 0640);
	if (fd < 0) {
		ret = -errno;
		free(csum_tmp);
		return ret;
	}
	if (fchown(fd, -1, anoubisd_gid) < 0) {
		ret = -errno;
		close(fd);
		unlink(csum_tmp);
		free(csum_tmp);
		return ret;
	}
	while (written < len) {
		ret = write(fd, md + idlen + written, len - written);
		if (ret < 0) {
			ret = -errno;
			unlink(csum_tmp);
			free(csum_tmp);
			close(fd);
			return ret;
		}
		written += ret;
	}
	if ((fsync(fd) < 0) || (rename(csum_tmp, csum_file) < 0)) {
		ret = -errno;
		close(fd);
		unlink(csum_tmp);
		free(csum_tmp);
		return ret;
	}

	free(csum_tmp);
	close(fd);
	return 0;
}

static int
sfs_readsigdata(const char *csum_file, void *sigdata, int siglen)
{
	int fd;
	int ret = 0;
	int bytes_read = 0;

	if (csum_file == NULL || sigdata == NULL || siglen < ANOUBIS_CS_LEN)
		return -EINVAL;

	fd = open(csum_file, O_RDONLY);
	if (fd < 0)
		return -errno;

	while (bytes_read < siglen) {
		ret = read(fd, sigdata + bytes_read, siglen - bytes_read);
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
sfs_haschecksum_chroot(const char *path)
{
	DIR *dir = NULL;
	struct dirent *sfs_ent = NULL;
	unsigned int uid;
	char testarg;
	char *csum_path = NULL;
	int ret;

	ret = __convert_user_path(path, &csum_path, 0, 1);
	if (ret < 0)
		return ret;

	dir = opendir(csum_path);
	if (dir == NULL) {
		free(csum_path);
		return -1;
	}
	while ((sfs_ent = readdir(dir)) != NULL) {
		if (strcmp(sfs_ent->d_name, ".") == 0 ||
		    strcmp(sfs_ent->d_name, "..") == 0)
			continue;
		if (sscanf(sfs_ent->d_name, "%u%c", &uid, &testarg) == 1) {
			closedir(dir);
			free(csum_path);
			return 1;
		}
	}
	closedir(dir);
	free(csum_path);
	return 0;
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
		ret = -errno;
		free(tmppath);
		return ret;
	}

	path_len = strlen(tmppath);
	k = path_len;
	while (k > root_len) {
		if (tmppath[k] == '/') {
			tmppath[k] = '\0';
			ret = check_empty_dir(tmppath);
			if (ret < 0) {
				ret = -errno;
				free(tmppath);
				return ret;
			}

			if (ret == 1)
				break;
			ret = rmdir(tmppath);
			if (ret < 0) {
				ret = -errno;
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
		return -ENOENT;

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

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
#include <ctype.h>

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/objects.h>

#ifdef LINUX
#include <linux/anoubis_sfs.h>
#include <bsdcompat.h>
#else
#include <dev/anoubis_sfs.h>
#endif

#include <sys/queue.h>

#include <anoubis_sig.h>
#include <anoubis_protocol.h>
#include <anoubis_msg.h>

#include "anoubisd.h"
#include "sfs.h"
#include "cert.h"

/*
 * sfsdata file format in sfs checksumroot:
 *
 * First the file version (SFSDATA_FORMAT_VERSION):
 *	u_int32_t	version
 *
 * Then the table of contents which consists of entries with:
 *	u_int32_t	type	(one of the SFSDATA_TYPE_*)
 *	u_int32_t	offset  (data offset from the start of the file)
 *	u_int32_t	length  (data length)
 *
 * The table of contents ends with an entry with type = SFSDATA_TYPE_EOT.
 *
 * After that, the data parts are appended. One data part cannot be longer
 * than SFSDATA_MAX_FIELD_LENGTH bytes. Currently, only one entry per
 * type may appear.
 */

#define	SFSDATA_TYPE_EOT	0
#define	SFSDATA_TYPE_CS		1
#define	SFSDATA_TYPE_SIG	2
#define	SFSDATA_TYPE_UPGRADE_CS 3
#define	NUM_SFSDATA_TYPES	4

static int	 sfs_writesfsdata(const char *csum_file, const char *csum_path,
		     const struct sfs_data *sfsdata);
static int	 sfs_readsfsdata(const char *csum_file,
		     struct sfs_data *sfsdata);
static int	 sfs_deletechecksum(const char *csum_file);
static int	 check_empty_dir(const char *path);

void
sfs_freesfsdata(struct sfs_data *sfsdata) {
	if (sfsdata == NULL)
		return;
	if (sfsdata->csdata)
		free(sfsdata->csdata);
	sfsdata->csdata = NULL;
	if (sfsdata->sigdata)
		free(sfsdata->sigdata);
	sfsdata->sigdata = NULL;
	if (sfsdata->upgradecsdata)
		free(sfsdata->upgradecsdata);
	sfsdata->upgradecsdata = NULL;
}

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
__sfs_checksumop(const struct sfs_checksumop *csop, struct sfs_data *data,
    int is_chroot)
{
	char		*csum_path = NULL, *csum_file, *keystring;
	int		 realsiglen = 0;
	int		 error = -EINVAL;
	struct cert	*cert = NULL;
	struct sfs_data	 sfsdata;

	switch(csop->op) {
	case ANOUBIS_CHECKSUM_OP_GET2:
	case _ANOUBIS_CHECKSUM_OP_GET_OLD:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
	case _ANOUBIS_CHECKSUM_OP_GETSIG_OLD:
		if (!data)
			return -EINVAL;
		break;
	default:
		if (is_chroot)
			return -EPERM;
		if (data)
			return -EINVAL;
		break;
	}

	error = __convert_user_path(csop->path, &csum_path, 0, is_chroot);
	if (error < 0)
		return error;

	memset(&sfsdata, 0, sizeof(sfsdata));

	switch(csop->op) {
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case _ANOUBIS_CHECKSUM_OP_GET_OLD:
	case ANOUBIS_CHECKSUM_OP_GET2:
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
	case _ANOUBIS_CHECKSUM_OP_GETSIG_OLD:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
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
		realsiglen = EVP_PKEY_size(cert->pubkey);
		error = -EINVAL;
		if (csop->op == ANOUBIS_CHECKSUM_OP_ADDSIG) {
			int ret;
			if (realsiglen + ANOUBIS_CS_LEN != csop->siglen)
				goto err1;
			ret = anoubisd_verify_csum(cert->pubkey, csop->sigdata,
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
		sfsdata.csdata = (u_int8_t *)csop->sigdata;
		sfsdata.cslen  = csop->siglen;
		error = sfs_writesfsdata(csum_file, csum_path, &sfsdata);
		break;
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
		sfsdata.sigdata = (u_int8_t *)csop->sigdata;
		sfsdata.siglen  = csop->siglen;
		error = sfs_writesfsdata(csum_file, csum_path, &sfsdata);
		break;
	case _ANOUBIS_CHECKSUM_OP_GET_OLD:
	case ANOUBIS_CHECKSUM_OP_GET2:
	case _ANOUBIS_CHECKSUM_OP_GETSIG_OLD:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
		error = sfs_readsfsdata(csum_file, &sfsdata);
		if (error < 0)
			goto err2;
		if (sfsdata.cslen && sfsdata.cslen != ANOUBIS_CS_LEN) {
			error = -EINVAL;
			goto err3;
		}
		if (sfsdata.siglen && realsiglen && cert) {
			int	ret;

			if (sfsdata.siglen != ANOUBIS_CS_LEN + realsiglen) {
				error = -ENOENT;
				goto err3;
			}
			ret = anoubisd_verify_csum(cert->pubkey,
			    sfsdata.sigdata, sfsdata.sigdata + ANOUBIS_CS_LEN,
			    realsiglen);
			if (ret != 1) {
				error = -ENOENT;
				goto err3;
			}
		} else {
			if (sfsdata.sigdata) {
				free(sfsdata.sigdata);
				sfsdata.sigdata = NULL;
				sfsdata.siglen = 0;
			}
		}
		if (data) {
			(*data) = sfsdata;
		} else {
			sfs_freesfsdata(&sfsdata);
		}
		error = 0;
		break;
	case ANOUBIS_CHECKSUM_OP_DEL:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
		error = sfs_deletechecksum(csum_file);
		break;
	}
	if (error < 0)
		goto err2;
	free(csum_file);
	free(csum_path);
	return 0;
err3:
	sfs_freesfsdata(&sfsdata);
err2:
	free(csum_file);
err1:
	free(csum_path);
	return error;
}

static int
sfs_copy_one_sig_type(void *dst, const void *src, int len, int type)
{
	int	off = 0;

	if (!len)
		return 0;
	set_value(*(u32n*)(dst + off), type);
	off += sizeof(u32n);
	set_value(*(u32n*)(dst + off), len);
	off += sizeof(u32n);
	memcpy(dst + off, src, len);
	off += len;
	return off;
}

int
sfs_checksumop(const struct sfs_checksumop *csop, void **buf, int *buflen)
{
	int		 ret, off;
	struct sfs_data	 tmpdata;
	int		 siglen;
	void		*sigbuf;

	switch(csop->op) {
	case _ANOUBIS_CHECKSUM_OP_GET_OLD:
	case ANOUBIS_CHECKSUM_OP_GET2:
	case _ANOUBIS_CHECKSUM_OP_GETSIG_OLD:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
		ret = __sfs_checksumop(csop, &tmpdata, 0);
		if (ret < 0)
			return ret;
		break;
	default:
		return __sfs_checksumop(csop, NULL, 0);
	}
	switch(csop->op) {
	case _ANOUBIS_CHECKSUM_OP_GET_OLD:
		if (tmpdata.cslen == 0) {
			sfs_freesfsdata(&tmpdata);
			return -ENOENT;
		}
		(*buflen) = tmpdata.cslen;
		(*buf) = tmpdata.csdata;
		tmpdata.csdata = NULL;
		break;
	case _ANOUBIS_CHECKSUM_OP_GETSIG_OLD:
		if (tmpdata.siglen == 0) {
			sfs_freesfsdata(&tmpdata);
			return -ENOENT;
		}
		(*buflen) = tmpdata.siglen;
		(*buf) = tmpdata.sigdata;
		tmpdata.sigdata = NULL;
		break;
	case ANOUBIS_CHECKSUM_OP_GET2:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
		if ((csop->op == ANOUBIS_CHECKSUM_OP_GET2 && tmpdata.cslen == 0)
		    || (csop->op == ANOUBIS_CHECKSUM_OP_GETSIG2
		    && tmpdata.siglen == 0)) {
			sfs_freesfsdata(&tmpdata);
			return -ENOENT;
		}
		siglen = 2 * sizeof(u32n);
		if (tmpdata.cslen)
			siglen += tmpdata.cslen + 2 * sizeof(u32n);
		if (tmpdata.siglen)
			siglen += tmpdata.siglen + 2 * sizeof(u32n);
		if (tmpdata.upgradecslen)
			siglen += tmpdata.upgradecslen + 2 * sizeof(u32n);
		sigbuf = malloc(siglen);
		if (!sigbuf) {
			sfs_freesfsdata(&tmpdata);
			return -ENOMEM;
		}
		off = 0;
		off += sfs_copy_one_sig_type(sigbuf+off,
		    tmpdata.csdata, tmpdata.cslen, ANOUBIS_SIG_TYPE_CS);
		off += sfs_copy_one_sig_type(sigbuf+off,
		    tmpdata.sigdata, tmpdata.siglen, ANOUBIS_SIG_TYPE_SIG);
		off += sfs_copy_one_sig_type(sigbuf+off,
		    tmpdata.upgradecsdata, tmpdata.upgradecslen,
		    ANOUBIS_SIG_TYPE_UPGRADECS);
		set_value(*(u32n*)(sigbuf + off), ANOUBIS_SIG_TYPE_EOT);
		off += sizeof(u32n);
		set_value(*(u32n*)(sigbuf + off), 0);
		if (buf) {
			(*buf) = sigbuf;
		} else {
			free(sigbuf);
		}
		if (buflen)
			(*buflen) = siglen;
	}
	sfs_freesfsdata(&tmpdata);
	return 0;
}

int
sfs_checksumop_chroot(const struct sfs_checksumop *csop, struct sfs_data *data)
{
	return __sfs_checksumop(csop, data, 1);
}

/*
 * We go through the dirctory looking for registered uids and update them.
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
	struct sfs_data	 sfsdata;

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
		int		need_sfsfree;
		/* since we updating checksums we skip keyid entries */
		if (strcmp(sfs_ent->d_name, ".") == 0 ||
		    strcmp(sfs_ent->d_name, "..") == 0)
			continue;
		if (asprintf(&csum_file, "%s/%s", sfs_path,
		    sfs_ent->d_name) < 0) {
			closedir(sfs_dir);
			free(sfs_path);
			return -ENOMEM;
		}
		need_sfsfree = 0;
		if (sscanf(sfs_ent->d_name, "%u%c", &uid, &testarg) == 1) {
			memset(&sfsdata, 0, sizeof(sfsdata));
			sfsdata.cslen = len;
			sfsdata.csdata = md;
		} else if (sfs_ent->d_name[0] == 'k') {
			memset(&sfsdata, 0, sizeof(sfsdata));
			ret = sfs_readsfsdata(csum_file, &sfsdata);
			if (ret < 0) {
				log_warnx("Cannot update checksum for %s "
				    "(key %s)", csum_file, sfs_ent->d_name+1);
				free(csum_file);
				continue;
			}
			if (sfsdata.siglen == 0) {
				free(csum_file);
				sfs_freesfsdata(&sfsdata);
				continue;
			}
			need_sfsfree = 1;
			if (sfsdata.upgradecsdata) {
				free(sfsdata.upgradecsdata);
			}
			sfsdata.upgradecslen = len;
			sfsdata.upgradecsdata = md;
		} else {
			free(csum_file);
			continue;
		}
		ret = sfs_writesfsdata(csum_file,  sfs_path, &sfsdata);
		if (need_sfsfree) {
			/* The upgrade signature is not malloced */
			sfsdata.upgradecslen = 0;
			sfsdata.upgradecsdata = NULL;
			sfs_freesfsdata(&sfsdata);
		}
		if (ret < 0)
			log_warnx("Cannot update checksum for %s "
			    "(file=%s, error=%d)", path, sfs_ent->d_name, ret);
		free(csum_file);
	}
	free(sfs_path);
	closedir(sfs_dir);
	return 0;
}


/*
 * Update the signature assocated with @cert for the file @path.
 * The signature is updated if the corresponding file in the sfs tree
 * exists. The return value is zero in case of success (either the
 * signature file did not exist or it could be updated successfully).
 * If an error occurs a negative errno value is returned.
 */
int
sfs_update_signature(const char *path, struct cert *cert,
    u_int8_t *md, int len)
{
	int		 ret;
	char		*sfs_path = NULL, *sfs_file = NULL, *keystr = NULL;
	struct sfs_data	 sfs_data;
	unsigned int	 keylen, siglen;
	struct stat	 statbuf;
	EVP_MD_CTX	 ctx;

	if (path == NULL || md == NULL || len != ANOUBIS_CS_LEN)
		return -EINVAL;
	if (!cert->privkey || !cert->kidlen)
		return -EINVAL;
	ret = convert_user_path(path, &sfs_path, 0);
	if (ret < 0)
		return ret;
	ret = -ENOMEM;
	keystr = anoubis_sig_key2char(cert->kidlen, cert->keyid);
	if (keystr == NULL)
		goto out;
	if (asprintf(&sfs_file, "%s/k%s", sfs_path, keystr) < 0) {
		sfs_file = NULL;
		goto out;
	}
	if (stat(sfs_file, &statbuf) < 0) {
		ret = -errno;
		if (ret == -ENOTDIR || ret == -ENOENT)
			ret = 0;
		goto out;
	}
	memset(&sfs_data, 0, sizeof(sfs_data));
	siglen = keylen = EVP_PKEY_size(cert->privkey);
	sfs_data.siglen = len + keylen;
	sfs_data.sigdata = malloc(sfs_data.siglen);
	if (sfs_data.sigdata == NULL)
		goto out;
	ret = -EINVAL;
	memcpy(sfs_data.sigdata, md, len);
	EVP_MD_CTX_init(&ctx);
	if (EVP_SignInit(&ctx, ANOUBIS_SIG_HASH_DEFAULT) == 0) {
		EVP_MD_CTX_cleanup(&ctx);
		sfs_freesfsdata(&sfs_data);
		goto out;
	}
	EVP_SignUpdate(&ctx, md, len);
	if (EVP_SignFinal(&ctx, sfs_data.sigdata + len, &siglen,
	    cert->privkey) == 0 || siglen != keylen) {
		EVP_MD_CTX_cleanup(&ctx);
		sfs_freesfsdata(&sfs_data);
		goto out;
	}
	EVP_MD_CTX_cleanup(&ctx);
	ret = sfs_writesfsdata(sfs_file, sfs_path, &sfs_data);
	sfs_freesfsdata(&sfs_data);
out:
	if (sfs_path)
		free(sfs_path);
	if (keystr)
		free(keystr);
	if (sfs_file)
		free(sfs_file);
	return ret;
}

static int
write_bytes(int fd, void *buf, size_t bytes)
{
	int ret = 0;
	size_t bytes_written = 0;
	while (bytes_written < bytes) {
		ret = write(fd, buf + bytes_written, bytes - bytes_written);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			ret = -errno;
			break;
		} else if (ret == 0) {
			ret = -EIO;
			break;
		}
		bytes_written += ret;
	}
	if (ret < 0)
		return ret;
	return bytes_written;
}

static int
sfs_create_upgradeindex(const char *csum_path, const char *csum_file)
{
	char		*thisfile;
	const char	*suffix;
	int		 min = strlen(SFS_CHECKSUMROOT);
	int		 i;

	/*
	 * First determine the suffix of the csum file, i.e. uid or
	 * 'k' followed by the key ID.
	 */
	for (i=strlen(csum_file)-1; i >= 0; i--)
		if (csum_file[i] == '/')
			break;
	if (i < 0 || csum_file[i] != '/')
		return -EFAULT;
	suffix = csum_file + i + 1;
	if ((*suffix) == 0)
		return -EINVAL;
	/* Make sure that csum_path is below the checksum root. */
	if (strncmp(csum_path, SFS_CHECKSUMROOT, min) != 0)
		return -EINVAL;
	if (csum_path[min] != '/')
		return -EINVAL;
	/* Allocate enough memory for all path names that we'll need. */
	if (asprintf(&thisfile, "%s/.*u_%s", csum_path, suffix) < 0)
		return -ENOMEM;
	i = strlen(csum_path) - 1;
	while(i > min) {
		int	fd;
		/* Strip trailing slashes. */
		if (thisfile[i] == '/') {
			i--;
			continue;
		}
		/* Remove the trailing path component. */
		while (thisfile[i] != '/')
			i--;
		/* Add the suffix prefixed by ".*u_" */
		sprintf(thisfile+i, "/.*u_%s", suffix);
		/* Create the index file if possible. */
		fd = open(thisfile, O_RDWR | O_CREAT | O_EXCL, 0640);
		if (fd < 0) {
			if (errno == EEXIST)
				return 0;
			return -errno;
		}
		if (fchown(fd, -1, anoubisd_gid) < 0) {
			int err = errno;
			close(fd);
			return -err;
		}
		close(fd);
	}
	return 0;
}

static int
sfs_need_upgrade_data(const void *updata, int uplen, const void *cmpdata,
    int cmplen)
{
	if (!updata || !cmpdata)
		return 0;
	if (uplen > cmplen)
		return 1;
	if (memcmp(updata, cmpdata, uplen) != 0)
		return 1;
	return 0;
}

static int
sfs_writesfsdata(const char *csum_file, const char *csum_path,
    const struct sfs_data *sfsdata)
{
	int ret = 0;
	int fd;
	char *csum_tmp = NULL;
	u_int32_t version = SFSDATA_FORMAT_VERSION;
	u_int32_t toc_entry[3] = { 0, 0, 0 };
	int offset = 0;
	int num_entries = 0;
	int write_upgrade_data = 0;

	if (sfsdata == NULL || csum_file == NULL || csum_path == NULL)
		return -EINVAL;

	if (sfsdata->upgradecsdata) {
		if (sfs_need_upgrade_data(sfsdata->upgradecsdata,
		    sfsdata->upgradecslen, sfsdata->csdata, sfsdata->cslen)
		    || sfs_need_upgrade_data(sfsdata->upgradecsdata,
		    sfsdata->upgradecslen, sfsdata->sigdata, sfsdata->siglen))
			write_upgrade_data = 1;
	}
	if (sfsdata->csdata != NULL) {
		num_entries++;
		if (sfsdata->cslen == 0 ||
		    sfsdata->cslen > SFSDATA_MAX_FIELD_LENGTH)
			return -EINVAL;
	}
	if (sfsdata->sigdata != NULL) {
		num_entries++;
		if (sfsdata->siglen == 0 ||
		    sfsdata->siglen > SFSDATA_MAX_FIELD_LENGTH)
			return -EINVAL;
	}
	if (write_upgrade_data && sfsdata->upgradecsdata != NULL) {
		num_entries++;
		if (sfsdata->upgradecslen == 0 ||
		    sfsdata->upgradecslen > SFSDATA_MAX_FIELD_LENGTH)
			return -EINVAL;
	}

	if (num_entries == 0)
		return -EINVAL;


	ret = mkpath(csum_path);
	if (ret < 0)
		return ret;

	if (asprintf(&csum_tmp, "%s%s", csum_file, ".tmp") == -1)
		return -ENOMEM;

	fd = open(csum_tmp, O_WRONLY|O_CREAT|O_TRUNC, 0640);
	if (fd < 0) {
		ret = -errno;
		log_warn("sfsdata: Could not write %s:", csum_tmp);
		free(csum_tmp);
		return ret;
	}
	if (fchown(fd, -1, anoubisd_gid) < 0) {
		log_warn("sfsdata: Could not chown %s:", csum_tmp);
		ret = -errno;
		goto err;
	}


	ret = write_bytes(fd, &version, sizeof(version));
	if (ret < 0)
		goto err;
	offset = ret + (num_entries + 1) * sizeof(toc_entry);

	if (sfsdata->csdata) {
		if (sfsdata->cslen == 0) {
			ret = -EINVAL;
			goto err;
		}
		toc_entry[0] = SFSDATA_TYPE_CS;
		toc_entry[1] = offset;
		toc_entry[2] = sfsdata->cslen;
		offset += sfsdata->cslen;
		ret = write_bytes(fd, toc_entry, sizeof(toc_entry));
		if (ret < 0)
			goto err;
	}

	if (sfsdata->sigdata) {
		if (sfsdata->siglen == 0) {
			ret = -EINVAL;
			goto err;
		}
		toc_entry[0] = SFSDATA_TYPE_SIG;
		toc_entry[1] = offset;
		toc_entry[2] = sfsdata->siglen;
		offset += sfsdata->siglen;
		ret = write_bytes(fd, toc_entry, sizeof(toc_entry));
		if (ret < 0)
			goto err;
	}

	if (write_upgrade_data && sfsdata->upgradecsdata) {
		if (sfsdata->upgradecslen == 0) {
			ret = -EINVAL;
			goto err;
		}
		toc_entry[0] = SFSDATA_TYPE_UPGRADE_CS;
		toc_entry[1] = offset;
		toc_entry[2] = sfsdata->upgradecslen;
		offset += sfsdata->upgradecslen;
		ret = write_bytes(fd, toc_entry, sizeof(toc_entry));
		if (ret < 0)
			goto err;
	}

	toc_entry[0] = SFSDATA_TYPE_EOT;
	toc_entry[1] = toc_entry[2] = 0;
	ret = write_bytes(fd, toc_entry, sizeof(toc_entry));
	if (ret < 0)
		goto err;

	if (sfsdata->csdata) {
		ret = write_bytes(fd, sfsdata->csdata, sfsdata->cslen);
		if (ret < 0)
			goto err;
	}

	if (sfsdata->sigdata) {
		ret = write_bytes(fd, sfsdata->sigdata, sfsdata->siglen);
		if (ret < 0)
			goto err;
	}

	if (sfsdata->upgradecsdata) {
		ret = write_bytes(fd, sfsdata->upgradecsdata,
		    sfsdata->upgradecslen);
		if (ret < 0)
			goto err;
	}

	if (fsync(fd) < 0) {
		ret = -errno;
		goto err;
	}
	if (sfsdata->upgradecsdata) {
		ret = sfs_create_upgradeindex(csum_path, csum_file);
		if (ret < 0)
			goto err;
	}
	if (rename(csum_tmp, csum_file) < 0) {
		ret = -errno;
		goto err;
	}

	goto out;

err:
	log_warn("sfsdata %s: Write error", csum_tmp);
	unlink(csum_tmp);
out:
	free(csum_tmp);
	close(fd);
	if (ret < 0)
		return ret;
	return 0;
}

static int
read_bytes(int fd, void *buf, size_t bytes) {
	int ret = 0;
	size_t bytes_read = 0;
	while (bytes_read < bytes) {
		ret = read(fd, buf + bytes_read, bytes - bytes_read);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			ret = -errno;
			break;
		} else if (ret == 0) {
			ret = -EIO;
			break;
		}
		bytes_read += ret;
	}
	if (ret < 0)
		return ret;
	return bytes_read;
}

static void *
memdup(const void *src, size_t size)
{
	void *buf = malloc(size);
	if (buf == NULL)
		return NULL;
	return memcpy(buf, src, size);
}

static int
sfs_readsfsdata(const char *csum_file, struct sfs_data *sfsdata)
{
	int fd;
	/* every toc entry consists of three values: type, offset, length */
	u_int32_t toc[NUM_SFSDATA_TYPES][3];
	int version, i, ret = 0;
	unsigned int total_length = 0, offset = 0, data_size;
	int last_entry;
	u_int8_t *buf = NULL;


	if (csum_file == NULL || sfsdata == NULL)
		return -EINVAL;

	memset(sfsdata, 0, sizeof(*sfsdata));

	fd = open(csum_file, O_RDONLY);
	if (fd < 0)
		return -errno;

	ret = read_bytes(fd, &version, sizeof(version));
	if (ret < 0)
		goto err;
	offset += ret;
	if (version != SFSDATA_FORMAT_VERSION) {
		log_warnx("sfsdata %s: Format version %i not supported",
		    csum_file, version);
		ret = -EOPNOTSUPP;
	}

	for (i = 0; i < NUM_SFSDATA_TYPES ; i++) {
		unsigned int end;
		ret = read_bytes(fd, &toc[i], sizeof(toc[0]));
		if (ret < 0 )
			goto err;
		offset += ret;

		if (toc[i][0] == SFSDATA_TYPE_EOT)
			break;

		if (toc[i][0] >= NUM_SFSDATA_TYPES || toc[i][1] == 0 ||
		    toc[i][2] == 0) {
			ret = -EINVAL;
			log_warnx("sfsdata %s: TOC error", csum_file);
			goto err;
		}
		if (toc[i][2] > SFSDATA_MAX_FIELD_LENGTH) {
			log_warnx("sfsdata %s: field too long", csum_file);
			ret = -EMSGSIZE;
			goto err;
		}

		end = toc[i][1] + toc[i][2];
		if (end > total_length)
			total_length = end;
	}
	if (i == NUM_SFSDATA_TYPES) {
		log_warnx("sfsdata %s: TOC too long", csum_file);
		ret = -E2BIG;
		goto err;
	}
	if (i == 0) {
		log_warnx("sfsdata %s: TOC empty", csum_file);
		ret = -EINVAL;
		goto err;
	}
	last_entry = i - 1;
	data_size = total_length - offset;

	buf = malloc(data_size);
	if (buf == NULL) {
		ret = -ENOMEM;
		goto err;
	}
	ret = read_bytes(fd, buf, data_size);
	if (ret < 0 )
		goto err;

	for (i = 0; i <= last_entry; i++) {
		switch (toc[i][0]) {
		case SFSDATA_TYPE_CS:
			if (sfsdata->cslen != 0) {
				log_warnx("sfsdata %s: invalid cslen",
				    csum_file);
				ret = -EINVAL;
				goto err;
			}
			sfsdata->cslen  = toc[i][2];
			sfsdata->csdata = memdup(buf + toc[i][1] - offset,
			    toc[i][2]);
			if (sfsdata->csdata == NULL) {
				ret = -ENOMEM;
				goto err;
			}
			break;
		case SFSDATA_TYPE_SIG:
			if (sfsdata->siglen != 0) {
				log_warnx("sfsdata %s: invalid siglen",
				    csum_file);
				ret = -EINVAL;
				goto err;
			}
			sfsdata->siglen  = toc[i][2];
			sfsdata->sigdata = memdup(buf + toc[i][1] - offset,
			    toc[i][2]);
			if (sfsdata->sigdata == NULL) {
				ret = -ENOMEM;
				goto err;
			}
			break;
		case SFSDATA_TYPE_UPGRADE_CS:
			if (sfsdata->upgradecslen != 0) {
				log_warnx("sfsdata %s: invalid upgradecdlen",
				    csum_file);
				ret = -EINVAL;
				goto err;
			}
			sfsdata->upgradecslen  = toc[i][2];
			sfsdata->upgradecsdata =
			    memdup(buf + toc[i][1] - offset, toc[i][2]);
			if (sfsdata->upgradecsdata == NULL) {
				ret = -ENOMEM;
				goto err;
			}
			break;
		default:
			ret = -EINVAL;
			log_warnx("sfsdata %s: invalid TOC entry %i",
			    csum_file, toc[i][0]);
			goto err;
		}
	}


	goto out;

err:
	sfs_freesfsdata(sfsdata);
out:
	free(buf);
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
		if (sscanf(sfs_ent->d_name, "%u%c", &uid, &testarg) == 1
		    || sfs_ent->d_name[0] == 'k') {
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

	path_len = strlen(tmppath);
	if (path_len < root_len || tmppath[root_len] != '/') {
		free(tmppath);
		return -EINVAL;
	}
	ret = unlink(csum_file);
	if (ret < 0) {
		ret = -errno;
		free(tmppath);
		return ret;
	}

	k = path_len;
	while (k > root_len) {
		if (tmppath[k] == '/') {
			tmppath[k] = '\0';
			if (!check_empty_dir(tmppath))
				break;
			if (rmdir(tmppath) < 0)
				break;
		}
		k--;
	}
	free(tmppath);
	return 0;
}

/*
 * Currently this function returns true even if the directory
 * containis only index files but no real entry. This is because
 * otherwise we would have to remove the index files manually before
 * the rmdir for the directory. The directory will be removed only after the
 * user asks for a checksum list that deletes the index.
 */
static int
check_empty_dir(const char *path)
{
	DIR		*dir = NULL;
	struct dirent	*entry;
	int		 empty = 1;

	dir = opendir(path);
	if (dir == NULL)
		return 0;

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0
		    || strcmp(entry->d_name, "..") == 0)
			continue;
		empty = 0;
		break;
	}
	closedir(dir);
	return empty;
}

static int
sfs_upgraded(const char *csum_file)
{
	struct sfs_data		sfsdata;
	int			len;

	if (sfs_readsfsdata(csum_file, &sfsdata) < 0)
		return 0;
	len = sfsdata.upgradecslen;
	sfs_freesfsdata(&sfsdata);
	return len != 0;
}

/* If a message arrives regarding a signature the payload of the message
 * should be including at least the keyid and the regarding path.
 *	---------------------------------
 *	|	keyid	|	path	|
 *	---------------------------------
 * futhermore should the message include the signature and the checksum
 * if the requested operation is ADDSIG
 *	-----------------------------------------------------------------
 *	|	keyid	|	csum	|	sigbuf	|	path	|
 *	-----------------------------------------------------------------
 * idlen is in this the length of the keyid. rawmsg.length is the total
 * length of the message excluding the path length.
 */

static int
sfs_validate_checksumop(struct sfs_checksumop *csop)
{
	struct cert		*cert = NULL;

	if (!csop->path)
		return -EINVAL;
	/* Requests for another user's uid are only allowed for root. */
	if (csop->uid != csop->auth_uid && csop->auth_uid != 0)
		return -EPERM;
	if (csop->keyid) {
		cert = cert_get_by_keyid(csop->keyid, csop->idlen);
		if (cert == NULL)
			return -EPERM;
		/*
		 * If the requesting user is not root, the certificate
		 * must belong to the authenticated user.
		 */
		if (cert->uid != csop->auth_uid && csop->auth_uid != 0)
			return -EPERM;
		/*
		 * If the requested uid is different from the authenticated
		 * user it must be the same as the uid in the cert.
		 */
		if (csop->auth_uid != csop->uid && csop->uid != cert->uid)
			return -EPERM;
	}

	switch(csop->op) {
	case ANOUBIS_CHECKSUM_OP_DEL:
	case _ANOUBIS_CHECKSUM_OP_GET_OLD:
	case ANOUBIS_CHECKSUM_OP_GET2:
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
		if (csop->listflags)
			return  -EINVAL;
		if (cert != NULL)
			return -EINVAL;
		if (csop->op == ANOUBIS_CHECKSUM_OP_ADDSUM) {
			if (!csop->sigdata || csop->siglen != ANOUBIS_CS_LEN)
				return -EINVAL;
		} else {
			if (csop->sigdata || csop->siglen)
				return -EINVAL;
		}
		break;
	case _ANOUBIS_CHECKSUM_OP_GETSIG_OLD:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
		if (csop->listflags)
			return  -EINVAL;
		if (cert == NULL)
			return -EPERM;
		if (csop->op == ANOUBIS_CHECKSUM_OP_ADDSIG) {
			if (!csop->sigdata || csop->siglen < ANOUBIS_CS_LEN)
				return -EINVAL;
		} else {
			if (csop->sigdata)
				return -EINVAL;
		}
		break;
	case ANOUBIS_CHECKSUM_OP_GENERIC_LIST:
		if (csop->listflags & ~(ANOUBIS_CSUM_FLAG_MASK))
			return -EINVAL;
		if (csop->sigdata || csop->siglen)
			return -EINVAL;
		if (csop->listflags & ANOUBIS_CSUM_UPGRADED) {
			if (csop->listflags
			    != (ANOUBIS_CSUM_UPGRADED | ANOUBIS_CSUM_KEY))
				return -EINVAL;
			if (csop->idlen == 0 || csop->keyid == NULL)
				return -EINVAL;
			if  (csop->sigdata || csop->siglen)
				return -EINVAL;
		} else if (csop->listflags & ANOUBIS_CSUM_WANTIDS) {
			/* WANTIDS implies ALL */
			if ((csop->listflags & ANOUBIS_CSUM_ALL) == 0)
				return -EINVAL;
			/* WANTIDS requires root privileges */
			if (csop->auth_uid != 0)
				return -EPERM;
			if (csop->keyid || csop->idlen)
				return -EINVAL;
			if (csop->uid)
				return -EINVAL;
		} else if (csop->listflags & ANOUBIS_CSUM_ALL) {
			if ((csop->listflags & ANOUBIS_CSUM_UID) == 0)
				return -EINVAL;
			if ((csop->listflags & ANOUBIS_CSUM_KEY) == 0)
				return -EINVAL;
			if (csop->auth_uid != 0)
				return -EPERM;
			if (csop->keyid)
				return -EINVAL;
		} else {
			if (csop->listflags & ANOUBIS_CSUM_KEY) {
				if (csop->idlen == 0 || csop->keyid == NULL)
					return -EINVAL;
			} else {
				if (csop->idlen || csop->keyid)
					return -EINVAL;
			}
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int
sfs_op_is_add(int op)
{
	return (op == ANOUBIS_CHECKSUM_OP_ADDSUM
	    || op == ANOUBIS_CHECKSUM_OP_ADDSIG);
}

int
sfs_parse_checksumop(struct sfs_checksumop *dst, struct anoubis_msg *m,
    uid_t auth_uid)
{
	void		*payload = NULL;
	int		 curlen, plen, error;
	unsigned int	 flags;

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_ChecksumRequestMessage)))
		return -EINVAL;
	memset(dst, 0, sizeof(struct sfs_checksumop));
	dst->op = get_value(m->u.checksumrequest->operation);
	flags = get_value(m->u.checksumrequest->flags);
	dst->auth_uid = auth_uid;
	if ((flags & ANOUBIS_CSUM_UID) && !(flags & ANOUBIS_CSUM_ALL))
		dst->uid = get_value(m->u.checksumrequest->uid);
	else
		dst->uid = auth_uid;
	dst->idlen = get_value(m->u.checksumrequest->idlen);
	if (dst->idlen < 0)
		return -EFAULT;
	if (sfs_op_is_add(dst->op)) {
		if (!VERIFY_LENGTH(m, sizeof(Anoubis_ChecksumAddMessage)))
			return -EINVAL;
		payload = m->u.checksumadd->payload;
		dst->siglen = get_value(m->u.checksumadd->cslen);
		if (dst->siglen < 0)
			return -EFAULT;
		dst->sigdata = (u_int8_t*)payload + dst->idlen;
	} else {
		payload = m->u.checksumrequest->payload;
	}
	if (dst->idlen)
		dst->keyid = payload;
	dst->path = payload + dst->idlen + dst->siglen;
	curlen = (char*)payload - (char*)m->u.buf;
	if (curlen < 0 || curlen > (int)m->length - CSUM_LEN)
		return -EFAULT;
	plen = m->length - curlen - CSUM_LEN;
	switch (dst->op) {
	case ANOUBIS_CHECKSUM_OP_GENERIC_LIST:
		dst->listflags = flags;
		break;
	case _ANOUBIS_CHECKSUM_OP_LIST:			/* Deprecated */
		dst->op = ANOUBIS_CHECKSUM_OP_GENERIC_LIST;
		if (dst->keyid) {
			dst->listflags = ANOUBIS_CSUM_KEY;
		} else {
			dst->listflags = ANOUBIS_CSUM_UID;
		}
		break;
	case _ANOUBIS_CHECKSUM_OP_LIST_ALL:		/* Deprecated */
		dst->op = ANOUBIS_CHECKSUM_OP_GENERIC_LIST;
		if (flags & ANOUBIS_CSUM_ALL) {
			dst->listflags = ( ANOUBIS_CSUM_ALL
			    | ANOUBIS_CSUM_UID | ANOUBIS_CSUM_KEY );
		} else {
			dst->listflags = ANOUBIS_CSUM_UID;
			if (dst->idlen)
				dst->listflags |= ANOUBIS_CSUM_KEY;
		}
		break;
	case _ANOUBIS_CHECKSUM_OP_UID_LIST:		/* Deprecated */
	case _ANOUBIS_CHECKSUM_OP_KEYID_LIST:		/* Deprecated */
		dst->op = ANOUBIS_CHECKSUM_OP_GENERIC_LIST;
		dst->listflags = (ANOUBIS_CSUM_WANTIDS | ANOUBIS_CSUM_ALL );
		if (dst->op == _ANOUBIS_CHECKSUM_OP_UID_LIST)
			dst->listflags |= ANOUBIS_CSUM_UID;
		else
			dst->listflags |= ANOUBIS_CSUM_KEY;
		/* XXX CEH: Fixup for deprecated messages from sfssig */
		if (dst->uid != dst->auth_uid && dst->auth_uid == 0)
			dst->uid = 0;
		break;
	default:
		if (dst->idlen == 0 && (flags & ANOUBIS_CSUM_UID))
			dst->uid = get_value(m->u.checksumrequest->uid);
		dst->listflags = 0;
	}
	error = -EINVAL;
	for (curlen = 0; curlen < plen; ++curlen) {
		if (dst->path[curlen] == 0) {
			error = 0;
			break;
		}
	}
	return sfs_validate_checksumop(dst);
}

static int
sfs_check_for_uid(const char *sfs_path, const char *entryname, uid_t uid,
    unsigned int listflags, int check_for_index)
{
	char		*file;
	int		 ret;
	struct stat	 statbuf;
	const char	*idxstr = "";
	int		 upgrade_only = !!(listflags & ANOUBIS_CSUM_UPGRADED);

	/* We only support indices for upgrade only for now. */
	if (check_for_index) {
		if (!upgrade_only)
			return 1;
		idxstr = ".*u_";
	}
	ret = asprintf(&file, "%s/%s/%s%d", sfs_path, entryname, idxstr, uid);
	if (ret < 0) {
		master_terminate(ENOMEM);
		return 0;
	}
	ret = lstat(file, &statbuf);
	if (ret < 0 || !S_ISREG(statbuf.st_mode)) {
		free(file);
		return 0;
	}
	if (check_for_index) {
		free(file);
		return 1;
	}
	if (upgrade_only && !sfs_upgraded(file)) {
		free(file);
		return 0;
	}
	free(file);
	return 1;
}

static int
sfs_check_for_key(const char *sfs_path, const char *entryname,
    const unsigned char *key, int keylen, unsigned int listflags,
    int check_for_index)
{
	char		*keystr, *file;
	int		 ret;
	struct stat	 statbuf;
	const char	*idxstr = "";
	int		 upgrade_only = !!(listflags & ANOUBIS_CSUM_UPGRADED);

	if (check_for_index) {
		/* We only support indices for upgrade_only for now. */
		if (!upgrade_only)
			return 1;
		idxstr = ".*u_";
	}
	if ((keystr = anoubis_sig_key2char(keylen, key)) == NULL) {
		master_terminate(ENOMEM);
		return 0;
	}
	if (asprintf(&file, "%s/%s/%sk%s", sfs_path, entryname, idxstr,
	    keystr) < 0) {
		master_terminate(ENOMEM);
		return 0;
	}
	free(keystr);
	ret = lstat(file, &statbuf);
	if (ret < 0 || !S_ISREG(statbuf.st_mode)) {
		free(file);
		return 0;
	}
	if (check_for_index) {
		free(file);
		return 1;
	}
	if (upgrade_only && !sfs_upgraded(file)) {
		free(file);
		return 0;
	}
	free(file);
	return 1;
}

int
sfs_wantentry(const struct sfs_checksumop *csop, const char *sfs_path,
    const char *entryname)
{
	int		stars;
	int		check_for_index = 0;

	/* Never report "." and ".." */
	if (entryname[0] == '.') {
		switch (entryname[1]) {
		case '*':
		case 0:
			return 0;
		case '.':
			if (entryname[2] == 0)
				return 0;
		}
	}

	/* ANOUBIS_CSUM_WANTIDS is not allowed without ANOUBIS_CSUM_ALL. */
	if (csop->listflags & ANOUBIS_CSUM_WANTIDS) {
		if (isdigit(entryname[0])
		    && (csop->listflags & ANOUBIS_CSUM_UID))
			return 1;
		if (entryname[0] == 'k' && (csop->listflags & ANOUBIS_CSUM_KEY))
			return 1;
		return 0;
	}

	/*
	 * We do not support ANOUBIS_CSUM_ALL for uids and signatures
	 * separately.
	 */
	if (csop->listflags & ANOUBIS_CSUM_ALL)
		return 1;

	/*
	 * At this point we know that we are listing directory contents for
	 * a specific user ID and/or Key.
	 */
	stars = 0;
	while (entryname[stars] == '*')
		stars++;

	/* Non-leaf directory entries are always interesting. */
	if (stars % 2 == 0)
		check_for_index = 1;

	if ((csop->listflags & ANOUBIS_CSUM_UID)
	    && sfs_check_for_uid(sfs_path, entryname, csop->uid,
	    csop->listflags, check_for_index))
		return 1;
	if ((csop->listflags & ANOUBIS_CSUM_KEY) && sfs_check_for_key(
	    sfs_path, entryname, csop->keyid, csop->idlen, csop->listflags,
	    check_for_index))
		return 1;
	return 0;
}

void
sfs_remove_index(const char *path, struct sfs_checksumop *csop)
{
	char	*idxfile;
	if ((csop->listflags & ANOUBIS_CSUM_UPGRADED) == 0)
		return;
	if (csop->listflags & (ANOUBIS_CSUM_ALL | ANOUBIS_CSUM_WANTIDS))
		return;
	if (csop->listflags & ANOUBIS_CSUM_UID) {
		if (asprintf(&idxfile, "%s/.*u_%d", path, csop->uid) < 0) {
			master_terminate(ENOMEM);
			return;
		}
		sfs_deletechecksum(idxfile);
		free(idxfile);
	}
	if (csop->listflags & ANOUBIS_CSUM_KEY) {
		char	*keystr;
		keystr = anoubis_sig_key2char(csop->idlen, csop->keyid);
		if (keystr == NULL) {
			master_terminate(ENOMEM);
			return;
		}
		if (asprintf(&idxfile, "%s/.*u_k%s", path, keystr) < 0) {
			master_terminate(ENOMEM);
			return;
		}
		sfs_deletechecksum(idxfile);
		free(keystr);
		free(idxfile);
	}
}

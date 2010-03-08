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
#include <anoubis_alloc.h>

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
		     struct sfs_data *sfsdata);
static int	 sfs_readsfsdata(const char *csum_file,
		     struct sfs_data *sfsdata);
static int	 sfs_deletechecksum(const char *csum_file);
static int	 check_empty_dir(const char *path);

void
sfs_initsfsdata(struct sfs_data *sfsdata)
{
	sfsdata->csdata = ABUF_EMPTY;
	sfsdata->sigdata = ABUF_EMPTY;
	sfsdata->upgradecsdata = ABUF_EMPTY;
}

void
sfs_freesfsdata(struct sfs_data *sfsdata) {
	if (sfsdata == NULL)
		return;
	abuf_free(sfsdata->csdata);
	sfsdata->csdata = ABUF_EMPTY;
	abuf_free(sfsdata->sigdata);
	sfsdata->sigdata = ABUF_EMPTY;
	abuf_free(sfsdata->upgradecsdata);
	sfsdata->upgradecsdata = ABUF_EMPTY;
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
	unsigned int	 realsiglen = 0;
	int		 error = -EINVAL;
	struct cert	*cert = NULL;
	struct sfs_data	 sfsdata;

	DEBUG(DBG_CSUM, " >sfs_checksumop: path=%s, op=%d", csop->path,
	    csop->op);
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

	sfs_initsfsdata(&sfsdata);

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
			if (abuf_length(csop->sigbuf) != ANOUBIS_CS_LEN)
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
			if (realsiglen + ANOUBIS_CS_LEN !=
			    abuf_length(csop->sigbuf))
				goto err1;
			ret = anoubisd_verify_csum(cert->pubkey,
			    abuf_toptr(csop->sigbuf, 0, ANOUBIS_CS_LEN),
			    abuf_toptr(csop->sigbuf, ANOUBIS_CS_LEN,
				realsiglen),
			    realsiglen);
			if (ret != 1) {
				DEBUG(DBG_CSUM, "Signature verifcation failed");
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
		sfsdata.csdata = csop->sigbuf;
		if (!abuf_empty(sfsdata.csdata)) {
			error = sfs_writesfsdata(csum_file, csum_path,
			    &sfsdata);
		} else {
			error = -ENOMEM;
		}
		break;
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
		sfsdata.sigdata = csop->sigbuf;
		if (!abuf_empty(sfsdata.sigdata)) {
			error = sfs_writesfsdata(csum_file, csum_path,
			    &sfsdata);
		} else {
			error = -ENOMEM;
		}
		break;
	case _ANOUBIS_CHECKSUM_OP_GET_OLD:
	case ANOUBIS_CHECKSUM_OP_GET2:
	case _ANOUBIS_CHECKSUM_OP_GETSIG_OLD:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
		error = sfs_readsfsdata(csum_file, &sfsdata);
		if (error < 0)
			goto err2;
		if (abuf_length(sfsdata.csdata)
		    && abuf_length(sfsdata.csdata) != ANOUBIS_CS_LEN) {
			error = -EINVAL;
			goto err3;
		}
		if (abuf_length(sfsdata.sigdata) && realsiglen && cert) {
			int		ret;

			if (abuf_length(sfsdata.sigdata) !=
			    ANOUBIS_CS_LEN + realsiglen) {
				error = -ENOENT;
				goto err3;
			}
			ret = anoubisd_verify_csum(cert->pubkey,
			    abuf_toptr(sfsdata.sigdata, 0,
			    ANOUBIS_CS_LEN),
			    abuf_toptr(sfsdata.sigdata,
			    ANOUBIS_CS_LEN, realsiglen), realsiglen);
			if (ret != 1) {
				error = -ENOENT;
				goto err3;
			}
		} else {
			abuf_free(sfsdata.sigdata);
			sfsdata.sigdata = ABUF_EMPTY;
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
	DEBUG(DBG_CSUM, " <sfs_checksumop: sucess path=%s op=%d", csop->path,
	    csop->op);

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
sfs_copy_one_sig_type(struct abuf_buffer dst, unsigned int off,
    const struct abuf_buffer src, int type)
{
	u32n			*intptr;
	struct abuf_buffer	 tmpbuf;

	if (type != ANOUBIS_SIG_TYPE_EOT && abuf_empty(src))
		return 0;
	tmpbuf = abuf_open(dst, off);
	off = 0;
	intptr = abuf_cast_off(tmpbuf, off, u32n);
	set_value(*intptr, type);
	off += sizeof(u32n);
	intptr = abuf_cast_off(tmpbuf, off, u32n);
	set_value(*intptr, abuf_length(src));
	off += sizeof(u32n);
	if (!abuf_empty(src)) {
		tmpbuf = abuf_open(tmpbuf, off);
		off += abuf_copy(tmpbuf, src);
	}
	return off;
}

int
sfs_checksumop(const struct sfs_checksumop *csop, struct abuf_buffer *buf)
{
	int			ret, off;
	struct sfs_data		tmpdata;
	int			siglen;
	struct abuf_buffer	sigbuf;

	sfs_initsfsdata(&tmpdata);
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
		if (abuf_empty(tmpdata.csdata)) {
			sfs_freesfsdata(&tmpdata);
			return -ENOENT;
		}
		(*buf) = tmpdata.csdata;
		tmpdata.csdata = ABUF_EMPTY;
		break;
	case _ANOUBIS_CHECKSUM_OP_GETSIG_OLD:
		if (abuf_empty(tmpdata.sigdata)) {
			sfs_freesfsdata(&tmpdata);
			return -ENOENT;
		}
		(*buf) = tmpdata.sigdata;
		tmpdata.sigdata = ABUF_EMPTY;
		break;
	case ANOUBIS_CHECKSUM_OP_GET2:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
		if ((csop->op == ANOUBIS_CHECKSUM_OP_GET2
		    && abuf_empty(tmpdata.csdata))
		    || (csop->op == ANOUBIS_CHECKSUM_OP_GETSIG2
		    && abuf_empty(tmpdata.sigdata))) {
			sfs_freesfsdata(&tmpdata);
			return -ENOENT;
		}
		siglen = 2 * sizeof(u32n);
		if (!abuf_empty(tmpdata.csdata))
			siglen += abuf_length(tmpdata.csdata)
			    + 2 * sizeof(u32n);
		if (!abuf_empty(tmpdata.sigdata))
			siglen += abuf_length(tmpdata.sigdata)
			    + 2 * sizeof(u32n);
		if (!abuf_empty(tmpdata.upgradecsdata))
			siglen += abuf_length(tmpdata.upgradecsdata)
			    + 2 * sizeof(u32n);
		sigbuf = abuf_alloc(siglen);
		if (abuf_empty(sigbuf)) {
			sfs_freesfsdata(&tmpdata);
			return -ENOMEM;
		}
		off = 0;
		off += sfs_copy_one_sig_type(sigbuf, off,
		    tmpdata.csdata, ANOUBIS_SIG_TYPE_CS);
		off += sfs_copy_one_sig_type(sigbuf, off,
		    tmpdata.sigdata, ANOUBIS_SIG_TYPE_SIG);
		off += sfs_copy_one_sig_type(sigbuf, off,
		    tmpdata.upgradecsdata, ANOUBIS_SIG_TYPE_UPGRADECS);
		off += sfs_copy_one_sig_type(sigbuf, off, ABUF_EMPTY,
		    ANOUBIS_SIG_TYPE_EOT);
		if (buf) {
			(*buf) = sigbuf;
		} else {
			abuf_free(sigbuf);
		}
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
 * We go through the directory looking for registered uids and update them.
 */
int
sfs_update_all(const char *path, u_int8_t *md, unsigned int len)
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
		if (sscanf(sfs_ent->d_name, "%u%c", &uid, &testarg) == 1) {
			sfs_initsfsdata(&sfsdata);
			sfsdata.csdata = abuf_alloc(len);
			abuf_copy_tobuf(sfsdata.csdata, md, len);
		} else if (sfs_ent->d_name[0] == 'k') {
			ret = sfs_readsfsdata(csum_file, &sfsdata);
			if (ret < 0) {
				log_warnx("Cannot update checksum for %s "
				    "(key %s)", csum_file, sfs_ent->d_name+1);
				free(csum_file);
				continue;
			}
			if (abuf_empty(sfsdata.sigdata)) {
				free(csum_file);
				sfs_freesfsdata(&sfsdata);
				continue;
			}
			if (abuf_length(sfsdata.upgradecsdata) != len) {
				abuf_free(sfsdata.upgradecsdata);
				sfsdata.upgradecsdata = abuf_alloc(len);
				if (abuf_empty(sfsdata.upgradecsdata)) {
					free(csum_file);
					sfs_freesfsdata(&sfsdata);
					closedir(sfs_dir);
					free(sfs_path);
					return -ENOMEM;
				}
			}
			abuf_copy_tobuf(sfsdata.upgradecsdata, md, len);
		} else {
			free(csum_file);
			continue;
		}
		ret = sfs_writesfsdata(csum_file, sfs_path, &sfsdata);
		sfs_freesfsdata(&sfsdata);
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
	sfs_initsfsdata(&sfs_data);
	siglen = keylen = EVP_PKEY_size(cert->privkey);
	sfs_data.sigdata = abuf_alloc(len + keylen);
	if (abuf_empty(sfs_data.sigdata))
		goto out;
	ret = -EINVAL;
	abuf_copy_tobuf(sfs_data.sigdata, md, len);
	EVP_MD_CTX_init(&ctx);
	if (EVP_SignInit(&ctx, ANOUBIS_SIG_HASH_DEFAULT) == 0) {
		EVP_MD_CTX_cleanup(&ctx);
		sfs_freesfsdata(&sfs_data);
		goto out;
	}
	EVP_SignUpdate(&ctx, md, len);
	if (EVP_SignFinal(&ctx, abuf_toptr(sfs_data.sigdata, len,
	    siglen), &siglen, cert->privkey) == 0 || siglen != keylen) {
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
write_buffer(int fd, struct abuf_buffer buf)
{
	return write_bytes(fd, abuf_toptr(buf, 0, abuf_length(buf)),
	    abuf_length(buf));
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
sfs_need_upgrade_data(const struct abuf_buffer updata,
    const struct abuf_buffer cmpdata)
{
	if (abuf_empty(updata) || abuf_empty(cmpdata))
		return 0;
	if (abuf_ncmp(updata, cmpdata, abuf_length(updata)) != 0)
		return 1;
	return 0;
}

/*
 * NOTE: Might free the contents of sfsdata->upgradecsdata.
 * NOTE: sfsdata->csdata and sfsdata->sigdata is not touched.
 */
static int
sfs_writesfsdata(const char *csum_file, const char *csum_path,
    struct sfs_data *sfsdata)
{
	int		 ret = 0;
	int		 fd;
	char		*csum_tmp = NULL;
	u_int32_t	 version = SFSDATA_FORMAT_VERSION;
	u_int32_t	 toc_entry[3] = { 0, 0, 0 };
	int		 offset = 0;
	int		 num_entries = 0;
	unsigned int	 len;

	if (sfsdata == NULL || csum_file == NULL || csum_path == NULL)
		return -EINVAL;

	if (!sfs_need_upgrade_data(sfsdata->upgradecsdata, sfsdata->csdata)
	    && !sfs_need_upgrade_data(sfsdata->upgradecsdata,
	    sfsdata->sigdata)) {
		abuf_free(sfsdata->upgradecsdata);
		sfsdata->upgradecsdata = ABUF_EMPTY;
	}
	if (!abuf_empty(sfsdata->csdata)) {
		num_entries++;
		len = abuf_length(sfsdata->csdata);
		if (len == 0 || len > SFSDATA_MAX_FIELD_LENGTH)
			return -EINVAL;
	}
	if (!abuf_empty(sfsdata->sigdata)) {
		num_entries++;
		len = abuf_length(sfsdata->sigdata);
		if (len == 0 || len > SFSDATA_MAX_FIELD_LENGTH)
			return -EINVAL;
	}
	if (!abuf_empty(sfsdata->upgradecsdata)) {
		num_entries++;
		len = abuf_length(sfsdata->upgradecsdata);
		if (len == 0 || len > SFSDATA_MAX_FIELD_LENGTH)
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

	if (!abuf_empty(sfsdata->csdata)) {
		toc_entry[0] = SFSDATA_TYPE_CS;
		toc_entry[1] = offset;
		toc_entry[2] = abuf_length(sfsdata->csdata);
		offset += abuf_length(sfsdata->csdata);
		ret = write_bytes(fd, toc_entry, sizeof(toc_entry));
		if (ret < 0)
			goto err;
	}

	if (!abuf_empty(sfsdata->sigdata)) {
		toc_entry[0] = SFSDATA_TYPE_SIG;
		toc_entry[1] = offset;
		toc_entry[2] = abuf_length(sfsdata->sigdata);
		offset += abuf_length(sfsdata->sigdata);
		ret = write_bytes(fd, toc_entry, sizeof(toc_entry));
		if (ret < 0)
			goto err;
	}

	if (!abuf_empty(sfsdata->upgradecsdata)) {
		toc_entry[0] = SFSDATA_TYPE_UPGRADE_CS;
		toc_entry[1] = offset;
		toc_entry[2] = abuf_length(sfsdata->upgradecsdata);
		offset += abuf_length(sfsdata->upgradecsdata);
		ret = write_bytes(fd, toc_entry, sizeof(toc_entry));
		if (ret < 0)
			goto err;
	}

	toc_entry[0] = SFSDATA_TYPE_EOT;
	toc_entry[1] = toc_entry[2] = 0;
	ret = write_bytes(fd, toc_entry, sizeof(toc_entry));
	if (ret < 0)
		goto err;

	if (!abuf_empty(sfsdata->csdata)) {
		ret = write_buffer(fd, sfsdata->csdata);
		if (ret < 0)
			goto err;
	}

	if (!abuf_empty(sfsdata->sigdata)) {
		ret = write_buffer(fd, sfsdata->sigdata);
		if (ret < 0)
			goto err;
	}

	if (!abuf_empty(sfsdata->upgradecsdata)) {
		ret = write_buffer(fd, sfsdata->upgradecsdata);
		if (ret < 0)
			goto err;
	}

	if (fsync(fd) < 0) {
		ret = -errno;
		goto err;
	}
	if (!abuf_empty(sfsdata->upgradecsdata)) {
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

static int
read_buffer(int fd, struct abuf_buffer buf)
{
	return read_bytes(fd, abuf_toptr(buf, 0, abuf_length(buf)),
	    abuf_length(buf));
}

static int
sfs_readsfsdata(const char *csum_file, struct sfs_data *sfsdata)
{
	int fd;
	/* every toc entry consists of three values: type, offset, length */
	u_int32_t		toc[NUM_SFSDATA_TYPES][3];
	int			version, i, ret = 0;
	unsigned int		total_length = 0, offset = 0, data_size;
	int			last_entry;
	struct abuf_buffer	databuf;

	if (csum_file == NULL || sfsdata == NULL)
		return -EINVAL;

	sfs_initsfsdata(sfsdata);

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

	if (data_size) {
		databuf = abuf_alloc(data_size);
		if (abuf_empty(databuf)) {
			ret = -ENOMEM;
			goto err;
		}
	} else {
		databuf = ABUF_EMPTY;
	}
	ret = read_buffer(fd, databuf);
	if (ret < 0 )
		goto err;

	for (i = 0; i <= last_entry; i++) {
		switch (toc[i][0]) {
		case SFSDATA_TYPE_CS:
			if (toc[i][2] == 0 || !abuf_empty(sfsdata->csdata)) {
				log_warnx("sfsdata %s: invalid cslen",
				    csum_file);
				ret = -EINVAL;
				goto err;
			}
			sfsdata->csdata = abuf_alloc(toc[i][2]);
			if (abuf_empty(sfsdata->csdata)) {
				ret = -ENOMEM;
				goto err;
			}
			abuf_copy_part(sfsdata->csdata, 0, databuf,
			    toc[i][1] - offset, toc[i][2]);
			break;
		case SFSDATA_TYPE_SIG:
			if (toc[i][2] == 0 || !abuf_empty(sfsdata->sigdata)) {
				log_warnx("sfsdata %s: invalid siglen",
				    csum_file);
				ret = -EINVAL;
				goto err;
			}
			sfsdata->sigdata = abuf_alloc(toc[i][2]);
			if (abuf_empty(sfsdata->sigdata)) {
				ret = -ENOMEM;
				goto err;
			}
			abuf_copy_part(sfsdata->sigdata, 0, databuf,
			    toc[i][1] - offset, toc[i][2]);
			break;
		case SFSDATA_TYPE_UPGRADE_CS:
			if (toc[i][2] == 0
			    || !abuf_empty(sfsdata->upgradecsdata)) {
				log_warnx("sfsdata %s: invalid upgradecdlen",
				    csum_file);
				ret = -EINVAL;
				goto err;
			}
			sfsdata->upgradecsdata = abuf_alloc(toc[i][2]);
			if (abuf_empty(sfsdata->upgradecsdata)) {
				ret = -ENOMEM;
				goto err;
			}
			abuf_copy_part(sfsdata->upgradecsdata, 0, databuf,
			    toc[i][1] - offset, toc[i][2]);
			break;
		default:
			ret = -EINVAL;
			log_warnx("sfsdata %s: invalid TOC entry %i",
			    csum_file, toc[i][0]);
			goto err;
		}
	}
out:
	abuf_free(databuf);
	close(fd);
	if (ret < 0)
		return ret;
	return 0;
err:
	sfs_freesfsdata(sfsdata);
	goto out;
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
	int			ret;

	if (sfs_readsfsdata(csum_file, &sfsdata) < 0)
		return 0;
	ret = !abuf_empty(sfsdata.upgradecsdata);
	sfs_freesfsdata(&sfsdata);
	return ret;
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
			if (abuf_length(csop->sigbuf) != ANOUBIS_CS_LEN)
				return -EINVAL;
		} else {
			if (abuf_length(csop->sigbuf))
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
			if (abuf_length(csop->sigbuf) < ANOUBIS_CS_LEN)
				return -EINVAL;
		} else {
			if (!abuf_empty(csop->sigbuf))
				return -EINVAL;
		}
		break;
	case ANOUBIS_CHECKSUM_OP_GENERIC_LIST:
		if (csop->listflags & ~(ANOUBIS_CSUM_FLAG_MASK))
			return -EINVAL;
		if (!abuf_empty(csop->sigbuf))
			return -EINVAL;
		if (csop->listflags & ANOUBIS_CSUM_UPGRADED) {
			if (csop->listflags
			    != (ANOUBIS_CSUM_UPGRADED | ANOUBIS_CSUM_KEY))
				return -EINVAL;
			if (csop->idlen == 0 || csop->keyid == NULL)
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
sfs_parse_csmulti(struct sfs_checksumop *dst, struct abuf_buffer mbuf,
    uid_t auth_uid)
{
	Anoubis_CSMultiRequestMessage		*req;
	unsigned int				 off, off2;
	unsigned int				 i, nrec = 0;
	int					 add = 0;

	DEBUG(DBG_CSUM, ">sfs_parse_csmulti");
	if ((req = abuf_cast(mbuf, Anoubis_CSMultiRequestMessage)) == NULL)
		return -EFAULT;
	memset(dst, 0, sizeof(struct sfs_checksumop));
	dst->auth_uid = auth_uid;
	off = sizeof(Anoubis_CSMultiRequestMessage);
	if (get_value(req->type) != ANOUBIS_P_CSMULTIREQUEST)
		return -EINVAL;
	dst->op = get_value(req->operation);
	switch(dst->op) {
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
		add = 1;
		break;
	case ANOUBIS_CHECKSUM_OP_GET2:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
	case ANOUBIS_CHECKSUM_OP_DEL:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
		break;
	default:
		return -EINVAL;
	}
	dst->listflags = 0;
	dst->uid = get_value(req->uid);
	dst->idlen = get_value(req->idlen);
	if (dst->idlen && dst->uid)
		return -EINVAL;
	if (dst->idlen) {
		dst->keyid = abuf_toptr(mbuf, off, dst->idlen);
		if (dst->keyid == NULL)
			return -EFAULT;
		off += dst->idlen;
	}
	dst->sigbuf = ABUF_EMPTY;
	dst->path = NULL;

	/* Validation checks. */
	if (dst->idlen) {
		struct cert	*cert = NULL;

		cert = cert_get_by_keyid(dst->keyid, dst->idlen);
		if (cert == NULL) {
			DEBUG(DBG_CSUM, "<sfs_parse_csmulti: no cert");
			return -EPERM;
		}
		/*
		 * If the requesting user is not root, the certificate
		 * must belong to the authenticated user.
		 */
		if (cert->uid != dst->auth_uid && dst->auth_uid != 0) {
			DEBUG(DBG_CSUM, "<sfs_parse_csmulti: bad cert");
			return -EPERM;
		}
	} else {
		if (dst->uid != dst->auth_uid && dst->auth_uid != 0) {
			DEBUG(DBG_CSUM, "<sfs_parse_csmulti: bad uid");
			return -EPERM;
		}
	}
	/* Count the records */
	off2 = off;
	while (1) {
		struct abuf_buffer		 rbuf = abuf_open(mbuf, off2);
		Anoubis_CSMultiRequestRecord	*r;
		unsigned int			 len;

		r = abuf_cast(rbuf, Anoubis_CSMultiRequestRecord);
		if (!r)
			return -EFAULT;
		len = get_value(r->length);
		if (len == 0)
			break;
		if (len > abuf_length(rbuf))
			return -EFAULT;
		off2 += len;
		nrec++;
	}
	/* Allocate memory for the records */
	dst->nrec = nrec;
	dst->csmulti = malloc(nrec * sizeof(struct sfs_csmulti_record));
	if (dst->csmulti == NULL)
		return -ENOMEM;
	/* Fill the records one at a time. */
	for (i=0; i<nrec; ++i) {
		struct abuf_buffer		 rbuf = abuf_open(mbuf, off);
		struct abuf_buffer		 pbuf;
		Anoubis_CSMultiRequestRecord	*r;
		unsigned int			 len, cslen;

		r = abuf_cast(rbuf, Anoubis_CSMultiRequestRecord);
		if (!r)
			goto fault;

		len = get_value(r->length);
		if (len < sizeof(Anoubis_CSMultiRequestRecord))
			goto fault;
		if (abuf_limit(&rbuf, len) < len)
			goto fault;
		off += len;
		off2 = offsetof(Anoubis_CSMultiRequestRecord, payload);

		/*
		 * If this is an add request, we expect cslen bytes of
		 * checksum data.
		 */
		dst->csmulti[i].index = get_value(r->index);
		cslen = get_value(r->cslen);
		if (cslen) {
			switch(dst->op) {
			case ANOUBIS_CHECKSUM_OP_ADDSUM:
				if (cslen != ANOUBIS_CS_LEN)
					goto invalid;
				break;
			case ANOUBIS_CHECKSUM_OP_ADDSIG:
				if (cslen < ANOUBIS_CS_LEN)
					goto invalid;
				break;
			default:
				goto invalid;
			}
			pbuf = abuf_open(rbuf, off2);
			if (abuf_limit(&pbuf, cslen) != cslen)
				goto fault;
			dst->csmulti[i].csdata = pbuf;
			off2 += cslen;
		} else {
			if (add)
				goto invalid;
			dst->csmulti[i].csdata = ABUF_EMPTY;
		}
		/* The rest of the payload is the path name. */
		dst->csmulti[i].path = abuf_tostr(rbuf, off2);
		if (dst->csmulti[i].path == NULL)
			goto fault;
	}
	DEBUG(DBG_CSUM, "<sfs_parse_csmulti (success)");
	return 0;
fault:
	free(dst->csmulti);
	dst->csmulti = NULL;
	DEBUG(DBG_CSUM, "<sfs_parse_csmulti (fault)");
	return -EFAULT;
invalid:
	free(dst->csmulti);
	dst->csmulti = NULL;
	DEBUG(DBG_CSUM, "<sfs_parse_csmulti (invalid)");
	return -EINVAL;
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
		int	siglen;

		if (!VERIFY_LENGTH(m, sizeof(Anoubis_ChecksumAddMessage)))
			return -EINVAL;
		payload = m->u.checksumadd->payload;
		siglen = get_value(m->u.checksumadd->cslen);
		if (siglen < 0)
			return -EFAULT;
		dst->sigbuf =
		    abuf_open_frommem((u_int8_t*)payload + dst->idlen, siglen);
	} else {
		payload = m->u.checksumrequest->payload;
	}
	if (dst->idlen)
		dst->keyid = payload;
	dst->path = payload + dst->idlen + abuf_length(dst->sigbuf);
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

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

struct sfs_data {
	int		 siglen;
	u_int8_t	*sigdata;
	int		 cslen;
	u_int8_t        *csdata;
	int		 upgradecslen;
	u_int8_t        *upgradecsdata;
};

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

static void
sfs_freesfsdata(struct sfs_data *sfsdata) {
	if (sfsdata == NULL)
		return;
	free(sfsdata->csdata);
	sfsdata->csdata = NULL;
	free(sfsdata->sigdata);
	sfsdata->sigdata = NULL;
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
__sfs_checksumop(const struct sfs_checksumop *csop, void **bufptr,
    int * buflen, int is_chroot)
{
	char		*csum_path = NULL, *csum_file, *keystring;
	int		 realsiglen = 0;
	int		 error = -EINVAL;
	struct cert	*cert = NULL;
	u_int8_t	*sigdata = NULL;
	int		 siglen = 0;
	struct sfs_data	 sfsdata;

	if (is_chroot && csop->op != ANOUBIS_CHECKSUM_OP_GET
	    && csop->op != ANOUBIS_CHECKSUM_OP_GETSIG)
		return -EPERM;

	error = __convert_user_path(csop->path, &csum_path, 0, is_chroot);
	if (error < 0)
		return error;

	memset(&sfsdata, 0, sizeof(sfsdata));

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
		sfsdata.csdata = (u_int8_t *)csop->sigdata;
		sfsdata.cslen  = csop->siglen;
		error = sfs_writesfsdata(csum_file, csum_path, &sfsdata);
		sfsdata.csdata = NULL;
		break;
	case ANOUBIS_CHECKSUM_OP_ADDSIG:
		sfsdata.sigdata = (u_int8_t *)csop->sigdata;
		sfsdata.siglen  = csop->siglen;
		error = sfs_writesfsdata(csum_file, csum_path, &sfsdata);
		sfsdata.sigdata = NULL;
		break;
	case ANOUBIS_CHECKSUM_OP_GET:
	case ANOUBIS_CHECKSUM_OP_GETSIG:
		error = sfs_readsfsdata(csum_file, &sfsdata);
		if (error < 0)
			goto err2;
		if (csop->op == ANOUBIS_CHECKSUM_OP_GETSIG) {
			sigdata = sfsdata.sigdata;
			siglen  = sfsdata.siglen;
		} else {
			sigdata = sfsdata.csdata;
			siglen  = sfsdata.cslen;
		}
		if (siglen != ANOUBIS_CS_LEN + realsiglen || sigdata == NULL) {
			error = -EINVAL;
			goto err3;
		}
		if (csop->op == ANOUBIS_CHECKSUM_OP_GETSIG) {
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
	if (bufptr) {
		(*bufptr) = sigdata;
		if (sfsdata.csdata && sfsdata.csdata != sigdata)
			free(sfsdata.csdata);
		if (sfsdata.sigdata && sfsdata.sigdata != sigdata)
			free(sfsdata.sigdata);
		if (sfsdata.upgradecsdata && sfsdata.upgradecsdata != sigdata)
			free(sfsdata.upgradecsdata);
	} else {
		sfs_freesfsdata(&sfsdata);
	}
	if (buflen)
		(*buflen) = siglen;
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

int
sfs_checksumop(const struct sfs_checksumop *csop, void **buf, int *buflen)
{
	return __sfs_checksumop(csop, buf, buflen, 0);
}

int
sfs_checksumop_chroot(const struct sfs_checksumop *csop, void **buf,
    int *buflen)
{
	return __sfs_checksumop(csop, buf, buflen, 1);
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

	memset(&sfsdata, 0, sizeof(sfsdata));
	sfsdata.cslen = len;
	sfsdata.csdata = md;

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
		ret = sfs_writesfsdata(csum_file, sfs_path, &sfsdata);
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
write_bytes(int fd, void *buf, size_t bytes) {
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


	if (sfsdata == NULL || csum_file == NULL || csum_path == NULL)
		return -EINVAL;

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
	if (sfsdata->upgradecsdata != NULL) {
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

	if (sfsdata->upgradecsdata) {
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

	if ((fsync(fd) < 0) || (rename(csum_tmp, csum_file) < 0)) {
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

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
#include <sys/statvfs.h>

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
#include <anoubis_errno.h>

#include "anoubisd.h"
#include "sfs.h"
#include "cert.h"
#include <anoubis_alloc.h>

/**
 * \file
 * Management functions for the SFS on disk tree.
 *
 * By default the SFS tree is stored on disk in SFS_CHECKSUMROOT. Processes
 * that are chroot-ed can access the SFS tree via SFS_CHECKSUMROOT_CHROOT.
 * The current version of the tree is stored in the version file
 * ANOUBISD_SFS_TREE_VERSIONFILE or ANOUBISD_SFS_TREE_VERSIONFILE_CHROOT.
 * Individual files in the tree must not have a version that is smaller
 * than than the version number in this file.
 *
 * Checksum and signature for files can be stored in the SFS-tree. For each
 * file in the system there is corresponding directory in the SFS-tree. The
 * directory name is constructed as follows:
 * - Each existing star ('*') in the path name is duplicated
 * - The final component of the path is prefixed with an additional star.
 * - SFS_CHECKSUMROOT or SFS_CHECKSUMROOT_CHROOT is prepended.
 *
 * This results in a (potentially non-existing) directory in the SFS-tree.
 * Checksums and signatures for the file are stored in file in this directory.
 * There are two types of files:
 * - A file that store unsigned checksums (used-ID file). The name of the
 *   file is the numeric user-ID of the user that stored the checksum.
 * - A file that stores signed checksums (signature file). The name of the
 *   file is the letter 'k' followed by the printable representation of
 *   the keyid (lower case) that signed this checksum.
 *
 * The format of a checksums file is a follows:
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
 * Other possible types are:
 * - SFSDATA_TYPE_CS: An unsigned signature (can only appear in user-ID files).
 * - SFSDATA_TYPE_SIG: A signed checksum (can only appear in signature files).
 * - SFSDATA_TYPE_UPGRADE_CS: This can only appear in signaturer files, too.
 *     It is automatically added to a signature file if the file contents
 *     change during an upgrade and the daemon is configured to adjust
 *     checksums in such a case. It is the (unsigned) checksum of the
 *     file contents immediately after the upgrade and cannot be modified
 *     manually.
 *
 * After that, the data parts are appended. One data part cannot be longer
 * than SFSDATA_MAX_FIELD_LENGTH bytes. Currently, only one entry per
 * type may appear.
 *
 * Currently, an individual file contain either
 * - a checksum and no signature or upgrade data     or
 * - no checksum, a signature and an optional upgrade checksum.
 */

#define	SFSDATA_TYPE_EOT	0
#define	SFSDATA_TYPE_CS		1
#define	SFSDATA_TYPE_SIG	2
#define	SFSDATA_TYPE_UPGRADE_CS 3
#define	NUM_SFSDATA_TYPES	4

/* Prototypes. See the implementation for inline documentation. */
static int	 sfs_writesfsdata(const char *csum_file, const char *csum_path,
		     const struct sfs_data *sfsdata);
static int	 sfs_readsfsdata(const char *csum_file,
		     struct sfs_data *sfsdata);
static int	 sfs_deletechecksum(const char *csum_file);
static int	 check_empty_dir(const char *path);

/**
 * Initialize a struct sfs_data structure.
 * @param The pre-allocated sfsdata structure.
 * @return None.
 */
void
sfs_initsfsdata(struct sfs_data *sfsdata)
{
	sfsdata->csdata = ABUF_EMPTY;
	sfsdata->sigdata = ABUF_EMPTY;
	sfsdata->upgradecsdata = ABUF_EMPTY;
}

/**
 * Free the buffers in an sfsdata structure. The sfsdata structure itself
 * is not freed by this function. In fact it is usually allocated on the
 * stack.
 *
 * @param sfsdata A pointer to the sfsdata structure.
 * @return None.
 */
void
sfs_freesfsdata(struct sfs_data *sfsdata)
{
	if (sfsdata == NULL)
		return;
	abuf_free(sfsdata->csdata);
	sfsdata->csdata = ABUF_EMPTY;
	abuf_free(sfsdata->sigdata);
	sfsdata->sigdata = ABUF_EMPTY;
	abuf_free(sfsdata->upgradecsdata);
	sfsdata->upgradecsdata = ABUF_EMPTY;
}

/**
 * Create all components of a path if  they do not exist. It is not an error
 * if the path already exists. If the path already exists but is not a
 * directory this function will return success anyway. The caller must
 * check for this case.
 *
 * @param path The path name to create.
 * @return Zero in case of success. A negative error code if something
 *     went wrong.
 */
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

/**
 * Insert escape characters into a path name for the SFS-tree.
 * Each star ('*') in the path name is replaced by two stars. Is @dir is
 * false the last component of the path is prefixed with an additional star.
 * Memory for the string is allocated using malloc(3c) and must be freed
 * by the caller.
 *
 * @param The original path.
 * @dir True if the path refers to a directory. In this case the final
 *     path component must not be prepended with a star. False if the
 *     path refers to a file.
 * @return The escaped path name or NULL in case of an error.
 *
 * NOTE: All paths returned from this function should be interpreted as
 *     direcotries in the SFS tree. However, there is a difference between
 *     SFS tree directories that mirror directories in the system (e.g.
 *     /usr/bin) and SFS tree directories that correspond to regular files
 *     in the system (e.g. /usr/bin/vi).
 *     - "/usr/bin" should be converted with @dir = true. The result is
 *       is "/usr/bin", too.
 *     - "/usr/bin/vi" should be converted with @dir = false. The result is
 *       "/usr/bin/ *vi" (the  space is only there to silence a gcc warning),
 *       a directory in the SFS-tree that can contain files that store
 *       checksums.
 *     - in theory it is possible to convert "/usr/bin" with @dir = false.
 *       The result would be "/usr/ *bin". Checksums and signatures for the
 *       directory "/usr/bin" would be stored in this SFS tree directory.
 *       However, at this time, checksums/signatures for directories are
 *       not supported.
 */
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

	/*
	 * If the given path is not a file, then it is a new entry
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

/**
 * Remove any additional stars from an escaped path name component.
 * The name should not contain any slashes. If the number of stars is
 * even (i.e. the component refers to a directory name) a slash is appended
 * to the converted result.
 * The result is allocated using malloc(3c) and must be freed by the caller.
 *
 * @param name The path name.
 * @return The path name component with the stars removed. NULL in case of
 *     an error.
 */
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

/**
 * Convert a user path name to the corresponding directory name in the
 * SFS tree. The given path name must be absolute and must not contain
 * any additional components (.e.g /../, /./, duplicate or trailing slashes).
 * This prevents path name based attacks via the SFS tree.
 * This function inserts escape characters as required and prepends the
 * checksum tree root.
 *
 * @param path The original user provided path name.
 * @param dir A pointer to the result will be stored in dir. The memory
 *     for the string is allocated using malloc(3c) and must be freed
 *     by the caller.
 * @param is_dir True if the user path names a directory (e.g. for list
 *     operations).
 * @param is_chroot True if the name conversion should assume that the
 *     process is chroot-ed and needs a shorted shadow tree preifx.
 * @return Zero in case of success, a negative error code in case of
 *     an error.
 */
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

/**
 * This function simply calls @see __convert_user_path with the is_chroot
 * parameter set to false.
 *
 * @param path The orignal path name.
 * @param dir The converted path name is returned here. The memory is
 *     allocated using malloc(3c) and must be freed by the caller.
 * @param is_dir True if the path name corresponds to a directory.
 * @return Zero in case of success. A negative error code in case of errors.
 */
int
convert_user_path(const char * path, char **dir, int is_dir)
{
	return __convert_user_path(path, dir, is_dir, 0);
}

/**
 * Execute the checksum operation csop. This function always applies the
 * operation to an individual file even in case of a csmulti operation.
 * The caller must make sure the the sigbuf and path fields of csop refer
 * to the proper data.
 *
 * In case of a GET or GETSIG operation the result is returned in an
 * sfsdata structure. The structure itself must be provided by the caller,
 * the contents are allocated dynamically and must be freed by the caller.
 *
 * @param csop The checksum operation
 * @param data A pointer to the sfsdata structure the hold the results
 *     for GET/GETSIG requests. Must be NULL for all other request types.
 * @param is_chroot True if the caller is chroot-ed. This changes the
 *     path name handling. Only GET/GETSIG requests are allowed if this
 *     value is true.
 * @return  True in case of success. A negative error code in case of errors.
 *     No data must be freed in case of errors.
 */
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
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
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
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
	case ANOUBIS_CHECKSUM_OP_DELSIG:
		error = -EINVAL;
		if (abuf_empty(csop->keyid))
			goto err1;
		cert = cert_get_by_keyid(csop->keyid);
		if (cert == NULL) {
			error = -A_EPERM_NO_CERTIFICATE;
			goto err1;
		}
		if (cert->uid != csop->auth_uid && csop->auth_uid != 0) {
			error = -A_EPERM_UID_MISMATCH;
			goto err1;
		}
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
		keystring = abuf_convert_tohexstr(csop->keyid);
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
	case ANOUBIS_CHECKSUM_OP_GET2:
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

/**
 * Append a checksum record to the buffer.
 * This is a helper function for @see sfs_checksumop. It adds a
 * checksum/signature record for GET/GETSIG requests to the buffer at
 * offset @off. This is the format that is expected by the userland
 * utilities.
 *
 * @param dst The destination buffer.
 * @param off The offset in the destination buffer where the checksum
 *     record should be stored.
 * @param src The checksum/signature data. If this is empty and the record
 *     type is not the end of list marker (ANOUBIS_SIG_TYPE_EOT) nothing
 *     is added to the destination buffer.
 * @param type The record type (ANOBUIS_SIG_TYPE_*).
 * @return The total number of bytes copied to the destination buffer.
 */
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

/**
 * Perform a the checksum operation @csop. In case of a GET/GETSIG
 * request the result is returned in @buf. The contents of the
 * result buffer are formatted can can be used directly in csmulti reply
 * messages.
 * Even in case of csmulti message this function will only perform
 * a single checksum operation. The caller must set the sigbuf and path
 * fields of @csop to the desired values.
 *
 * @param csop The checksum operation.
 * @param buf The result buffer. Memory for the buffer is allocated
 *     dynamically and must be freed by the caller.
 * @return Zero in case of success, a negative error code in case of errors.
 */
int
sfs_checksumop(const struct sfs_checksumop *csop, struct abuf_buffer *buf)
{
	int			ret, off;
	struct sfs_data		tmpdata;
	int			siglen;
	struct abuf_buffer	sigbuf;

	sfs_initsfsdata(&tmpdata);
	switch(csop->op) {
	case ANOUBIS_CHECKSUM_OP_GET2:
	case ANOUBIS_CHECKSUM_OP_GETSIG2:
		ret = __sfs_checksumop(csop, &tmpdata, 0);
		if (ret < 0)
			return ret;
		break;
	default:
		return __sfs_checksumop(csop, NULL, 0);
	}
	switch(csop->op) {
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

/**
 * The same a @see __sfs_checksumop except that the chroot parameter is
 * always true.
 *
 * @param csop The checksum operation.
 * @param data The result data of the checksum operation.
 * @return Zero in case of success, a negative error code in case of an error.
 */
int
sfs_checksumop_chroot(const struct sfs_checksumop *csop, struct sfs_data *data)
{
	return __sfs_checksumop(csop, data, 1);
}

/**
 * Go through the directory looking for registered uids and keyids.
 * Update all unsigned checksum for all users to the value given by @md
 * and add (or replace) an upgrade checksum for all keyids. This function
 * is used during upgrade if checksum/signature updates are configured.
 *
 * @param path The file name to be updated.
 * @param md The new checksum.
 * @return Zero in case of sucess, a negative error code in case of an
 *     error. Some errors are only indicated by log messages. In particular,
 *     a transient error in the update of one file does not prevent updates
 *     of other files in the directory.
 */
int
sfs_update_all(const char *path, struct abuf_buffer md)
{
	DIR		*sfs_dir = NULL;
	struct dirent	*sfs_ent = NULL;
	char		 testarg;
	char		*csum_file = NULL;
	char		*sfs_path;
	unsigned int	 uid;
	int		 ret;
	struct sfs_data	 sfsdata;

	if (path == NULL || abuf_length(md) != ANOUBIS_CS_LEN)
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
			sfsdata.csdata = md;
			ret = sfs_writesfsdata(csum_file, sfs_path, &sfsdata);
			sfsdata.csdata = ABUF_EMPTY;
			sfs_freesfsdata(&sfsdata);
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
			if (abuf_length(sfsdata.upgradecsdata))
				abuf_free(sfsdata.upgradecsdata);
			sfsdata.upgradecsdata = md;
			ret = sfs_writesfsdata(csum_file, sfs_path, &sfsdata);
			sfsdata.upgradecsdata = ABUF_EMPTY;
			sfs_freesfsdata(&sfsdata);
		} else {
			free(csum_file);
			continue;
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

/**
 * Update the signature assocated with @cert for the file @path.
 * The signature is updated if the corresponding file in the sfs tree
 * exists.
 *
 * @param path The path name of the file.
 * @param cert The certificate structure. A private key must be loaded
 *     for a successful update!
 * @param md The checksum to sign.
 * @return The return value is zero in case of success (either the
 *     signature file did not exist or it could be updated successfully).
 *     If an error occurs a negative errno value is returned.
 *
 * NOTE: This function is used during a system upgrade if the passphrase
 * NOTE: for the root certificate is configured. This is a supported but
 * NOTE: potentially dangerous configuration! This function must not be
 * NOTE: used for anything else.
 */
int
sfs_update_signature(const char *path, struct cert *cert,
    struct abuf_buffer md)
{
	int		 ret;
	char		*sfs_path = NULL, *sfs_file = NULL, *keystr = NULL;
	struct sfs_data	 sfs_data;
	unsigned int	 keylen, siglen;
	struct stat	 statbuf;
	EVP_MD_CTX	 ctx;

	if (path == NULL || abuf_length(md) != ANOUBIS_CS_LEN)
		return -EINVAL;
	if (cert->privkey == NULL || abuf_empty(cert->keyid))
		return -EINVAL;
	ret = convert_user_path(path, &sfs_path, 0);
	if (ret < 0)
		return ret;
	ret = -ENOMEM;
	keystr = abuf_convert_tohexstr(cert->keyid);
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
	sfs_data.sigdata = abuf_alloc(abuf_length(md) + keylen);
	if (abuf_empty(sfs_data.sigdata))
		goto out;
	ret = -EINVAL;
	abuf_copy_part(sfs_data.sigdata, 0, md, 0, abuf_length(md));
	EVP_MD_CTX_init(&ctx);
	if (EVP_SignInit(&ctx, ANOUBIS_SIG_HASH_DEFAULT) == 0) {
		EVP_MD_CTX_cleanup(&ctx);
		sfs_freesfsdata(&sfs_data);
		goto out;
	}
	EVP_SignUpdate(&ctx, abuf_toptr(md, 0, abuf_length(md)),
	    abuf_length(md));
	if (EVP_SignFinal(&ctx, abuf_toptr(sfs_data.sigdata, abuf_length(md),
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

/**
 * Write the bytes to the file given by @fd. An interrupted or
 * partial write is resumed. The function only returns after all bytes
 * have been written or an uncorrectable error occured.
 *
 * @param fd The filedescriptor.
 * @param buf The buffer to write.
 * @param bytes The number of bytes to write.
 * @return The number of bytes actually written or a negative error code
 *     if an error occured.
 */
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

/**
 * Write the entire contents of the buffer to the given  file using
 * @see write_bytes.
 *
 * @param fd The file descriptor.
 * @param buf The data to write.
 * @return The number of bytes actually written or a negative error code
 *     if an error occured.
 */
static int
write_buffer(int fd, struct abuf_buffer buf)
{
	return write_bytes(fd, abuf_toptr(buf, 0, abuf_length(buf)),
	    abuf_length(buf));
}

/**
 * This function creates special index files that are used to speed
 * up lookups for upgraded files.
 * If a directory or one of its sub-directories contains a file with an
 * upgrade checksum the directory contains an empty file named ".*u_kxxx"
 * where xxx is the key-ID of the key that has upgraded files. (If more
 * than one key has upgraded files there is more than one index file)
 * The file name contains an unescaped star in the middle of the name, i.e.
 * it cannot be the result of an SFS tree lookup.
 *
 * This function creates index files starting at the directory that contains
 * the given file and continues up to the root of the SFS tree.
 * It is possible that an index file exists but no upgraded files for the
 * key exist. The index file will only be deleted during the next list
 * operation.
 *
 * @param csum_path The path name component of @csum_file. The first
 *     index file is created in the directory that is obtained by stripping
 *     the final path component.
 * @param csum_file The full file name that an upgrade checksum was just
 *     added to. The key-ID is extracted from this path name.
 * @return Zero in case of success, a negative error code in case of an
 *     error.
 */
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

/**
 * Compare the checksum in @updata with the checksum at the
 * start of @cmpdata (usually a signature) and return true if
 * the data differs. The length of @updata determines the length
 * of the comparison. If either buffer is empty the return value is false.
 * This function is used to determine if an upgrade checksum must be
 * written or not. It is not needed if the upgrade checksum does not
 * differ from the existing signature.
 *
 * @param updata The checksum.
 * @param cmpdata The signature data.
 * @return True or false depending on the result of the comparison.
 */
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

/**
 * Estimate the number of sfs writes that can be performed without
 * reaching the file sytem limit.
 *
 * Rules for the estimation:
 * - We estimate the number of writes possible. These writes are performed
 *   without looking a the file system again. Thus lots of writes from a
 *   different source can still lead to a full file system.
 * - The estimate is limited to SFS_FREE_WRITES writes, i.e. we consult the
 *   file system at least once per SFS_FREE_WRITES write operations.
 * - We never touch the last percent of blocks and/or inodes.
 * - We assume that a single write consumes at most 3 blocks and 3 inodes.
 * - If statvfs(2) is not supported, we assume that the file system has
 *   enough space.
 * - If the checksum changeroot directory does not yet exist, we allow
 *   a few writes (SFS_INITIAL_FREE_WRITES).
 *
 * @param None.
 * @return Zero if writing is ok, a negative error code (ENOSPC) otherwise.
 */
static int
sfs_space_available(void)
{
	/*
	 * This static variable stores a previous estimate of the total
	 * number of writes that are possible. As long as this value is
	 * not zero, it is decremented and the file system is not checked.
	 */
	static int		sfs_free_writes = 0;

	/* Parameters for the free space calculation */
	static const int	SFS_FREE_WRITES = 1000;
	static const int	SFS_INITIAL_FREE_WRITES = 10;

	struct statvfs		buf;
	int			ret;
	long long		tmp;

	if (sfs_free_writes > 0) {
		--sfs_free_writes;
		return 0;
	}
	ret = statvfs(SFS_CHECKSUMROOT, &buf);
	if (ret < 0) {
		switch (errno) {
		case ENOSYS:
			/* Not supported: Allow many writes. */
			sfs_free_writes = SFS_FREE_WRITES;
			return 0;
		case ENOENT:
		case ENOTDIR:
			/* No SFS-Tree yet: Allow a few writes */
			sfs_free_writes = SFS_INITIAL_FREE_WRITES;
			return 0;
		default:
			return -errno;
		}
	}
	/* Less than one percent free blocks/inodes? */
	if (buf.f_blocks/100 > buf.f_bavail || buf.f_files/100 > buf.f_favail)
		return -ENOSPC;

	sfs_free_writes = SFS_FREE_WRITES;
	/* Allow at most one sfs write for every 3 blocks above one precent. */
	tmp = (buf.f_bavail - buf.f_blocks / 100) / 3;
	if (tmp < sfs_free_writes)
		sfs_free_writes = tmp;
	/* Allow at most one sfs write for every 3 inodes above one precent. */
	tmp = (buf.f_favail - buf.f_files / 100) / 3;
	if (tmp < sfs_free_writes)
		sfs_free_writes = tmp;
	/* Should not happen... */
	if (sfs_free_writes < 0) {
		sfs_free_writes = 0;
		return -ENOSPC;
	}
	return 0;
}

/**
 * Write the data in @sfsdata to the disk. The complete file name
 * is in @csum_file. The directory name of that file is in @csum_path.
 * This function does not modify @sfsdata in any way.
 *
 * @param csum_file The file name that the data should be written to.
 * @param csum_path The function ensures that this directory exists before
 *     trying to open @csum_file.
 * @param sfsdata The sfs data to write. The upgrade checksum (if present)
 *     is only written if a signature is present and the checksum in the
 *     signature differs from the upgrade checksum.
 * @return Zero in case of success, a negative error code in case of errors.
 */
static int
sfs_writesfsdata(const char *csum_file, const char *csum_path,
    const struct sfs_data *sfsdata)
{
	int		 ret = 0;
	int		 fd;
	char		*csum_tmp = NULL;
	u_int32_t	 version = SFSDATA_FORMAT_VERSION;
	u_int32_t	 toc_entry[3] = { 0, 0, 0 };
	int		 offset = 0;
	int		 num_entries = 0;
	unsigned int	 len;
	int		 need_upgrade_data;

	if (sfsdata == NULL || csum_file == NULL || csum_path == NULL)
		return -EINVAL;

	ret = sfs_space_available();
	if (ret < 0)
		return ret;
	need_upgrade_data = sfs_need_upgrade_data(sfsdata->upgradecsdata,
	    sfsdata->sigdata);
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
	if (need_upgrade_data && !abuf_empty(sfsdata->upgradecsdata)) {
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

	if (need_upgrade_data && !abuf_empty(sfsdata->upgradecsdata)) {
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

	if (need_upgrade_data && !abuf_empty(sfsdata->upgradecsdata)) {
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

/**
 * Read bytes from a file descriptor. Interrupted or partial reads are
 * resumed.
 *
 * @param fd The filedescriptor to read from.
 * @param buf The data is stored in the memory area pointed to by @buf.
 * @param bytes The number of bytes to read.
 * @return The total number of bytes that were read or a negative error
 *     code in case of an error.
 */
static int
read_bytes(int fd, void *buf, size_t bytes)
{
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

/**
 * Read data from the file descriptor into the buffer @buf. The number
 * of bytes to read is determined by the size of the buffer. Interrupted
 * or partial reads are resumed.
 *
 * @param fd The file descriptor to read from.
 * @param buf The data is stored in this buffer. Memory for the buffer
 *     must be allocated by the caller.
 * @return The total number of bytes read or a negative error code in case
 *     or errors.
 */
static int
read_buffer(int fd, struct abuf_buffer buf)
{
	return read_bytes(fd, abuf_toptr(buf, 0, abuf_length(buf)),
	    abuf_length(buf));
}

/**
 * Read SFS data from an SFS file on disk.
 * The caller must allocate the sfsdata structure itself. The buffers for
 * the checksum/signature data are allocated by this function and the
 * caller must free them.
 *
 * @param csum_file The file that the data should be read from.
 * @param sfsdata A pre-allocated sfsdata structure where the data
 *     will be stored. The buffer with the pre-allocated structure must
 *     be initialized and empty.
 * @return Zero in case of success, a negative error code in case of an
 *     error. No memory is allocated in case of an error.
 */
static int
sfs_readsfsdata(const char *csum_file, struct sfs_data *sfsdata)
{
	int fd;
	/* every toc entry consists of three values: type, offset, length */
	u_int32_t		toc[NUM_SFSDATA_TYPES][3];
	int			version, i, ret = 0;
	unsigned int		total_length = 0, offset = 0, data_size;
	int			last_entry;
	struct abuf_buffer	databuf = ABUF_EMPTY;

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
		goto err;
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

/**
 * Check if the file in @path has at least one checksum or signature
 * assigned to it.
 *
 * @param path The path name.
 * @return A negative error code if an error occured, zero if the file
 *     has no checksum and one if the file has a checksum.
 */
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

/**
 * Delete the file @csum_file in the SFS tree. If this creates an
 * empty directory, the parent directory is removed. This happens
 * recursively.
 *
 * @param csum_file The checksum file.
 * @return Zero in case of success, a negative error code in case of an
 *     error. Errors while deleting empty parent directories are ignored,
 *     if the file itself could be removed.
 */
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

/**
 * Return true if the directory @path is empty. The directory must be
 * completely empty. A stale index file is sufficient to prevent the
 * deletion of the directory. If will only be deleted after the user
 * asks for a checksum list that deletes the stale index.
 *
 * @param path The path name of the directory.
 * @return True if the directory is empty.
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

/**
 * Return true if the SFS tree file @csum_file contains an upgrade checksum.
 *
 * @param The path of the file in the SFS tree.
 * @return True if the file can be read and contains an upgrade checksum.
 */
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

/**
 * Do consistency checks on the checksum operation. The checks are only
 * done for list operations and old style checksum operations.
 *
 * @param csop The checksum operation to check.
 * @return Zero in case of a successful validation, a negative error code
 *     if the validate fails.
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
	if (!abuf_empty(csop->keyid)) {
		cert = cert_get_by_keyid(csop->keyid);
		if (cert == NULL)
			return -A_EPERM_NO_CERTIFICATE;
		/*
		 * If the requesting user is not root, the certificate
		 * must belong to the authenticated user.
		 */
		if (cert->uid != csop->auth_uid && csop->auth_uid != 0)
			return -A_EPERM_UID_MISMATCH;
		/*
		 * If the requested uid is different from the authenticated
		 * user it must be the same as the uid in the cert.
		 */
		if (csop->auth_uid != csop->uid && csop->uid != cert->uid)
			return -A_EPERM_UID_MISMATCH;
	}

	switch(csop->op) {
	case ANOUBIS_CHECKSUM_OP_DEL:
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
			if (abuf_empty(csop->keyid))
				return -EINVAL;
		} else if (csop->listflags & ANOUBIS_CSUM_WANTIDS) {
			/* WANTIDS implies ALL */
			if ((csop->listflags & ANOUBIS_CSUM_ALL) == 0)
				return -EINVAL;
			/* WANTIDS requires root privileges */
			if (csop->auth_uid != 0)
				return -EPERM;
			if (!abuf_empty(csop->keyid))
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
			if (!abuf_empty(csop->keyid))
				return -EINVAL;
		} else {
			if (csop->listflags & ANOUBIS_CSUM_KEY) {
				if (abuf_empty(csop->keyid))
					return -EINVAL;
			} else {
				if (!abuf_empty(csop->keyid))
					return -EINVAL;
			}
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/**
 * Return true if the checksum operation is ADD or ADDSIG, i.e. the
 * request data must contain checksum or signature data.
 *
 * @param op The operation.
 * @return True if the operation is an add operation.
 */
static int
sfs_op_is_add(int op)
{
	return (op == ANOUBIS_CHECKSUM_OP_ADDSUM
	    || op == ANOUBIS_CHECKSUM_OP_ADDSIG);
}

/**
 * Parse a csmulti request message and store the result in the checksum
 * operation @dst. The sfs_checksumop structure must be preallocated by
 * the caller. This function will allocate memory for the csmulti array
 * in @dst. All other fields will reference the request message in @mbuf
 * directly.
 *
 * @param dst The checksum operation.
 * @param mbuf A buffer that contains the entire request message, i.e. the
 *     buffer starts with an Anoubis_CSMultiRequestMessage.
 *     See anoubis_protocol.h for a detailed description of the message
 *     format.
 * @param auth_uid The user-ID of the authorized user that issued the
 *     request.
 * @return Zero if the message could be parsed successfully. A negative
 *     error code if something went wrong. No memory is allocated if an
 *     error occurs. The csmulti field is always initialized.
 */
int
sfs_parse_csmulti(struct sfs_checksumop *dst, struct abuf_buffer mbuf,
    uid_t auth_uid)
{
	Anoubis_CSMultiRequestMessage		*req;
	unsigned int				 off, off2;
	unsigned int				 i, nrec = 0;
	int					 add = 0;
	unsigned int				 idlen;

	DEBUG(DBG_CSUM, ">sfs_parse_csmulti");
	dst->csmulti = csmultiarr_EMPTY;
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
	idlen = get_value(req->idlen);
	if (idlen && dst->uid)
		return -EINVAL;
	if (idlen) {
		dst->keyid = abuf_open(mbuf, off);
		if (abuf_length(dst->keyid) < idlen)
			return -EFAULT;
		abuf_limit(&dst->keyid, idlen);
		off += idlen;
	} else {
		dst->keyid = ABUF_EMPTY;
	}
	dst->sigbuf = ABUF_EMPTY;
	dst->path = NULL;

	/* Validation checks. */
	if (abuf_length(dst->keyid)) {
		struct cert	*cert = NULL;

		cert = cert_get_by_keyid(dst->keyid);
		if (cert == NULL) {
			DEBUG(DBG_CSUM, "<sfs_parse_csmulti: no cert");
			return -A_EPERM_NO_CERTIFICATE;
		}
		/*
		 * If the requesting user is not root, the certificate
		 * must belong to the authenticated user.
		 */
		if (cert->uid != dst->auth_uid && dst->auth_uid != 0) {
			DEBUG(DBG_CSUM, "<sfs_parse_csmulti: bad cert");
			return -A_EPERM_UID_MISMATCH;
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
	dst->csmulti = csmultiarr_alloc(nrec);
	if (csmultiarr_size(dst->csmulti) != nrec)
		return -ENOMEM;
	/* Fill the records one at a time. */
	for (i=0; i<nrec; ++i) {
		struct abuf_buffer		 rbuf = abuf_open(mbuf, off);
		struct abuf_buffer		 pbuf;
		Anoubis_CSMultiRequestRecord	*r;
		unsigned int			 len, cslen;
		struct sfs_csmulti_record	*rec;

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
		rec = &csmultiarr_access(dst->csmulti, i);
		rec->index = get_value(r->index);
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
			rec->csdata = pbuf;
			off2 += cslen;
		} else {
			if (add)
				goto invalid;
			rec->csdata = ABUF_EMPTY;
		}
		/* The rest of the payload is the path name. */
		rec->path = abuf_tostr(rbuf, off2);
		if (rec->path == NULL)
			goto fault;
	}
	DEBUG(DBG_CSUM, "<sfs_parse_csmulti (success)");
	return 0;
fault:
	csmultiarr_free(dst->csmulti);
	dst->csmulti = csmultiarr_EMPTY;
	DEBUG(DBG_CSUM, "<sfs_parse_csmulti (fault)");
	return -EFAULT;
invalid:
	csmultiarr_free(dst->csmulti);
	dst->csmulti = csmultiarr_EMPTY;
	DEBUG(DBG_CSUM, "<sfs_parse_csmulti (invalid)");
	return -EINVAL;
}

/**
 * Parse a checksum list request or old style checksum add/get/del
 * requests. The message is an Anoubis_ChecksumRequestMessage.
 * However, if the operation is ADD or ADDSIG, the message is an
 * Anoubis_ChecksumAddMessage.
 *
 * The payload of the message consists of the following components
 * in this order (some componente may not be present depending on
 * the request type):
 *  - A keyid if the idlen field of the request message is non-zero.
 *  - Add requests contain signature data. The length of the data is
 *    given by the siglen field of the request message.
 *  - All messages contain a path name. The path name must be a string
 *    that is terminated by a NUL byte.
 *
 * @param dst A pre-allocated sfs_checksumop structure that is filled
 *     by this function. The memory for the sfs_checksumop structure
 *     itself must be provided by the caller. In case of success all fields
 *     reference the memory of the request message directly. No memory
 *     is allocated by this function.
 * @param m The request message.
 * @param auth_uid The user ID of the authorized user that initiated the
 *     request.
 * @return Zero in case of success, a negative error code in case of an
 *     error.
 *
 * This function calls sfs_validate_checksumop before returning. Any
 * errors during validation are propagated.
 */
int
sfs_parse_checksumop(struct sfs_checksumop *dst, struct anoubis_msg *m,
    uid_t auth_uid)
{
	void		*payload = NULL;
	int		 error;
	unsigned int	 flags, idlen, msgsz, i, plen;

	msgsz = sizeof(Anoubis_ChecksumRequestMessage);
	if (!VERIFY_LENGTH(m, msgsz))
		return -EINVAL;
	memset(dst, 0, sizeof(struct sfs_checksumop));
	dst->op = get_value(m->u.checksumrequest->operation);
	flags = get_value(m->u.checksumrequest->flags);
	dst->auth_uid = auth_uid;
	if ((flags & ANOUBIS_CSUM_UID) && !(flags & ANOUBIS_CSUM_ALL))
		dst->uid = get_value(m->u.checksumrequest->uid);
	else
		dst->uid = auth_uid;
	idlen = get_value(m->u.checksumrequest->idlen);
	if (sfs_op_is_add(dst->op)) {
		unsigned int	siglen;

		msgsz = sizeof(Anoubis_ChecksumAddMessage);
		if (!VERIFY_LENGTH(m, msgsz))
			return -EINVAL;
		plen = PAYLOAD_LEN(m, checksumadd, payload);
		payload = m->u.checksumadd->payload;
		siglen = get_value(m->u.checksumadd->cslen);
		if (idlen > plen || siglen > plen)
			return -EINVAL;
		if (!VERIFY_LENGTH(m, msgsz + idlen + siglen))
			return -EINVAL;
		dst->sigbuf = abuf_open_frommem(payload + idlen, siglen);
	} else {
		plen = PAYLOAD_LEN(m, checksumrequest, payload);
		if (idlen > plen)
			return -EINVAL;
		if (!VERIFY_LENGTH(m, msgsz + idlen))
			return -EINVAL;
		payload = m->u.checksumrequest->payload;
	}
	if (idlen)
		dst->keyid = abuf_open_frommem(payload, idlen);
	dst->path = payload + idlen + abuf_length(dst->sigbuf);
	plen -= (idlen + abuf_length(dst->sigbuf));
	switch (dst->op) {
	case ANOUBIS_CHECKSUM_OP_GENERIC_LIST:
		dst->listflags = flags;
		break;
	default:
		if (abuf_empty(dst->keyid) && (flags & ANOUBIS_CSUM_UID))
			dst->uid = get_value(m->u.checksumrequest->uid);
		dst->listflags = 0;
	}
	error = -EINVAL;
	for (i = 0; i < plen; ++i) {
		if (dst->path[i] == 0) {
			error = 0;
			break;
		}
	}
	if (error)
		return error;
	return sfs_validate_checksumop(dst);
}

/**
 * Return true if the entry @entryname in the directory @sfs_path should
 * be listed as part of a list request with the user ID @uid and the
 * list flags @listflags.
 *
 * @param sfs_path The SFS directory that is being listed.
 * @param entryname The entry that was found in the directory.
 * @param uid Checksums should be listed for this user-ID.
 * @param listflags Only used to check if this request asks for
 *     files with an upgrade checksum. However, this function assumes
 *     that ANOUBIS_CSUM_UID is set in listflags and both
 *     ANOUBIS_CSUM_WANTIDS and ANOUBIS_CSUM_ALL are clear.
 * @param check_for_index True if the object given by @entryname refers to
 *     a directory (i.e. it can contain index files). Directories are always
 *     part of a listing except for those cases where a missing index file
 *     tells us that the directory is not interesting (only works for
 *     upgrade checksum at this time).
 * @return True if the entry @entryname should be part of the listing.
 */
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
	if (ret < 0)
		master_terminate();
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

/**
 * Return true if the entry @entryname in the directory @sfs_path should
 * be listed as part of a list request with the key ID @keyid and the
 * list flags @listflags.
 *
 * @param sfs_path The SFS directory that is being listed.
 * @param entryname The entry that was found in the directory.
 * @param keyid Checksums should be listed for this key ID.
 * @param listflags Only used to check if this request asks for
 *     files with an upgrade checksum. However, this function assumes
 *     that ANOUBIS_CSUM_KEY is set in listflags and both
 *     ANOUBIS_CSUM_WANTIDS and ANOUBIS_CSUM_ALL are clear.
 * @param check_for_index True if the object given by @entryname refers to
 *     a directory (i.e. it can contain index files). Directories are always
 *     part of a listing except for those cases where a missing index file
 *     tells us that the directory is not interesting (only works for
 *     upgrade checksum at this time).
 * @return True if the entry @entryname should be part of the listing.
 */
static int
sfs_check_for_key(const char *sfs_path, const char *entryname,
    struct abuf_buffer keyid, unsigned int listflags, int check_for_index)
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
	if ((keystr = abuf_convert_tohexstr(keyid)) == NULL)
		master_terminate();
	if (asprintf(&file, "%s/%s/%sk%s", sfs_path, entryname, idxstr,
	    keystr) < 0)
		master_terminate();
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

/**
 * Return true if the directory entry @entryname should be part of the
 * list request @csop.
 *
 * @param csop The checksum list operation.
 * @param sfs_path The name of the directory where @entryname was found.
 * @param entryname The name of the entry.
 * @return True if the entry should be part of the listing.
 */
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

	/*
	 * Non-leaf directory entries are always interesting except for the
	 * case where a missing index file tells us otherwise.
	 */
	if (stars % 2 == 0)
		check_for_index = 1;

	if ((csop->listflags & ANOUBIS_CSUM_UID)
	    && sfs_check_for_uid(sfs_path, entryname, csop->uid,
	    csop->listflags, check_for_index))
		return 1;
	if ((csop->listflags & ANOUBIS_CSUM_KEY) && sfs_check_for_key(sfs_path,
	    entryname, csop->keyid, csop->listflags, check_for_index))
		return 1;
	return 0;
}

/**
 * Remove an index file after a list request that did not return any
 * matching entries.
 *
 * @param path The path name that was listed.
 * @param csop The checksum list request that did not return entries.
 * @return None.
 */
void
sfs_remove_index(const char *path, struct sfs_checksumop *csop)
{
	char	*idxfile;
	if ((csop->listflags & ANOUBIS_CSUM_UPGRADED) == 0)
		return;
	if (csop->listflags & (ANOUBIS_CSUM_ALL | ANOUBIS_CSUM_WANTIDS))
		return;
	if (csop->listflags & ANOUBIS_CSUM_UID) {
		if (asprintf(&idxfile, "%s/.*u_%d", path, csop->uid) < 0)
			master_terminate();
		sfs_deletechecksum(idxfile);
		free(idxfile);
	}
	if (csop->listflags & ANOUBIS_CSUM_KEY) {
		char	*keystr;
		keystr = abuf_convert_tohexstr(csop->keyid);
		if (keystr == NULL)
			master_terminate();
		if (asprintf(&idxfile, "%s/.*u_k%s", path, keystr) < 0)
			master_terminate();
		sfs_deletechecksum(idxfile);
		free(keystr);
		free(idxfile);
	}
}

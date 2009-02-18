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

#define ANOUBISD_CERT_DIR ANOUBISD_POLICYDIR ANOUBISD_POLICYCHROOT "/pubkeys"
#define ANOUBISD_CERT_DIR_CHROOT ANOUBISD_POLICYCHROOT "/pubkeys"

struct sfs_cert {
	TAILQ_ENTRY(sfs_cert)	 entry;
	uid_t			 uid;
	EVP_PKEY		*pkey;
	unsigned char		*keyid;
	int			 kidlen;
	X509			*req;
};
TAILQ_HEAD(sfs_cert_db, sfs_cert) *sfs_certs;

static int	 sfs_readchecksum(const char *csum_file, unsigned char *md);
static int	 sfs_readsignature(const char *csum_file, u_int8_t **sign,
		     int *siglen);
static int	 sfs_deletechecksum(const char *csum_file);
static int	 check_empty_dir(const char *path);
char		*insert_escape_seq(const char *path, int dir);
static int	 sfs_cert_keyid(unsigned char **keyid, X509 *cert);
struct sfs_cert	*sfs_pubkey_get_by_uid(uid_t u);
struct sfs_cert	*sfs_pubkey_get_by_keyid(unsigned char *keyid, int klen);
static void	 sfs_cert_flush_db(struct sfs_cert_db *sc);
static int	 sfs_cert_load_db(const char *, struct sfs_cert_db *);

int
check_if_exist(const char *path)
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

char *
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
__sfs_checksumop(const char *path, unsigned int operation, uid_t uid,
    u_int8_t *md, u_int8_t **sign, int *siglen, int len, int idlen,
    int is_chroot)
{
	char *csum_path = NULL, *csum_file = NULL;
	char *sigfile = NULL;
	struct sfs_cert *cert = NULL;
	int fd;
	int vlen = 0;
	int written = 0;
	int ret;

	if (is_chroot && operation != ANOUBIS_CHECKSUM_OP_GET
	    && operation != ANOUBIS_CHECKSUM_OP_GETSIG)
		return -EPERM;

	ret = __convert_user_path(path, &csum_path, 0, is_chroot);
	if (ret < 0)
		return ret;

	if (operation == ANOUBIS_CHECKSUM_OP_ADDSIG ||
	    operation == ANOUBIS_CHECKSUM_OP_GETSIG ||
	    operation == ANOUBIS_CHECKSUM_OP_DELSIG) {
		if (idlen <= 0) {
			ret = -EINVAL;
			goto out;
		}
		if ((cert = sfs_pubkey_get_by_keyid(md, idlen))
		    == NULL) {
			ret = -EINVAL;
			goto out;
		}
		if ((sigfile = anoubis_sig_key2char(idlen, md)) == NULL) {
			ret = -ENOMEM;
			goto out;
		}
		if (asprintf(&csum_file, "%s/k%s", csum_path, sigfile) == -1) {
			ret = -ENOMEM;
			goto out;
		}
	} else {
		if (asprintf(&csum_file, "%s/%d", csum_path, uid) == -1) {
			ret = -ENOMEM;
			goto out;
		}
	}

	if (operation == ANOUBIS_CHECKSUM_OP_ADDSIG) {
		if (cert->uid != uid)
			return -EPERM;
		/* Before we add the signature we should verify it */
		vlen = len - ANOUBIS_CS_LEN;
		if ((ret = anoubisd_verify_csum(cert->pkey, md + idlen, md +
		    ANOUBIS_CS_LEN + idlen, vlen)) != 1) {
			log_warn("could not verify %d", ret);
			ret =-EINVAL;
			goto out;
		}
	}
	/* And now we finally adding the signature */
	if ((operation == ANOUBIS_CHECKSUM_OP_ADDSUM) ||
	    (operation == ANOUBIS_CHECKSUM_OP_ADDSIG)) {
		if (md == NULL) {
			ret = -EINVAL;
			goto out;
		}

		if (len < ANOUBIS_CS_LEN) {
			ret = -EINVAL;
			goto out;
		}
		ret = mkpath(csum_path);
		if (ret < 0) {
			goto out;
		}
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

		while (written < len) {
			ret = write(fd, md + idlen + written, len - written);
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
	} else if (operation == ANOUBIS_CHECKSUM_OP_DEL ||
	    operation == ANOUBIS_CHECKSUM_OP_DELSIG) {
		if (operation == ANOUBIS_CHECKSUM_OP_DELSIG)
			if (cert->uid != uid)
				return -EPERM;
		ret = sfs_deletechecksum(csum_file);
		if (ret < 0)
			ret = -errno;
	} else if (operation == ANOUBIS_CHECKSUM_OP_GET) {
		ret = sfs_readchecksum(csum_file, md);
	} else if (operation == ANOUBIS_CHECKSUM_OP_GETSIG) {
		if (idlen <= 0) {
			ret = -EINVAL;
			goto out;
		}
		*siglen = EVP_PKEY_size(cert->pkey) + ANOUBIS_CS_LEN;
		if ((*sign = calloc(*siglen, sizeof(u_int8_t))) == NULL) {
			ret = -ENOMEM;
			goto out;
		}
		ret = sfs_readsignature(csum_file, sign, siglen);
		if (ret < 0) {
			goto out;
		}
		/* Before we send the signature we should verify it */
		vlen = *siglen - ANOUBIS_CS_LEN;
		if ((ret = anoubisd_verify_csum(cert->pkey, *sign, *sign
		    + ANOUBIS_CS_LEN, vlen)) != 1) {
			log_warn("could not verify %d", ret);
			ret =-EPERM;
			free(*sign);
			*sign = NULL;
			*siglen = 0;
			goto out;
		} else {
			ret = 0;
		}
	} else {
		ret = -ENOSYS;
	}
out:
	if (sigfile)
		free(sigfile);
	if (csum_path)
		free(csum_path);
	if (csum_file)
		free(csum_file);
	return ret;
}

int
sfs_checksumop(const char *path, unsigned int operation, uid_t uid,
    u_int8_t *md, u_int8_t **sign, int *siglen, int len, int idlen)
{
	return __sfs_checksumop(path, operation, uid, md, sign, siglen, len,
	    idlen, 0);
}

int
sfs_checksumop_chroot(const char *path, unsigned int operation, uid_t uid,
    u_int8_t *md, u_int8_t **sign, int *siglen, int len, int idlen)
{
	return __sfs_checksumop(path, operation, uid, md, sign, siglen, len,
	    idlen, 1);
}

static int
sfs_readsignature(const char *csum_file, u_int8_t **sign, int *siglen)
{
	int fd;
	int ret = 0;
	int bytes_read = 0;
	int size;

	if (csum_file == NULL || sign == NULL || siglen == NULL) {
		return -EINVAL;
	}

	size = *siglen;
	if (size <= ANOUBIS_CS_LEN)
		return -EINVAL;

	fd = open(csum_file, O_RDONLY);
	if (fd < 0)
		return -errno;


	while (bytes_read < size) {
		ret = read(fd, *sign + bytes_read, size - bytes_read);
		if (ret < 0) {
			ret = -errno;
			*siglen = 0;
			break;
		} else if (ret == 0) {
			ret = -EIO;
			*siglen = 0;
			break;
		}
		bytes_read += ret;
	}
	close(fd);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

static int
sfs_readchecksum(const char *csum_file, unsigned char *md)
{
	int fd;
	int ret = 0;
	int bytes_read = 0;

	if (!csum_file || !md)
		return -EINVAL;

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

void
sfs_cert_init(int chroot)
{
	struct sfs_cert_db	*cdb;
	int			 count;
	const char		*certdir = ANOUBISD_CERT_DIR;

	if (chroot)
		certdir = ANOUBISD_CERT_DIR_CHROOT;
	if ((cdb = calloc(1, sizeof(struct sfs_cert_db))) == NULL) {
		log_warn("%s: calloc", certdir);
		master_terminate(ENOMEM);
	}
	TAILQ_INIT(cdb);

	if ((count = sfs_cert_load_db(certdir, cdb)) == -1)
		fatal("sfs_cert_init: failed to load sfs certificates");

	log_info("%d certificates loaded from %s", count, certdir);
	sfs_certs = cdb;
}

void
sfs_cert_reconfigure(int chroot)
{
	struct sfs_cert_db	*new, *old;
	int			 count;
	const char		*certdir = ANOUBISD_CERT_DIR;

	if (chroot)
		certdir = ANOUBISD_CERT_DIR_CHROOT;
	if ((new = calloc(1, sizeof(struct sfs_cert_db))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);
	}
	TAILQ_INIT(new);

	if ((count = sfs_cert_load_db(certdir, new)) == -1) {
		log_warnx("sfs_cert_reconfigure: could not load public keys");
		free(new);
		return;
	}
	log_info("%d certificates loaded from %s", count, certdir);

	old = sfs_certs;
	sfs_certs = new;
	sfs_cert_flush_db(old);
}

static int
sfs_cert_load_db(const char *dirname, struct sfs_cert_db *certs)
{
	DIR		*dir;
	FILE		*fp;
	struct dirent	*dp;
	struct sfs_cert	*sc;
	uid_t		 uid;
	int		 count;
	const char	*errstr;
	char		*filename;

	if (certs == NULL) {
		log_warnx("sfs_cert_load_db: illegal sfscert queue");
		return 0;
	}

	if ((dir = opendir(dirname)) == NULL) {
		log_warn("opendir %s", dirname);
		return 0;
	}

	count = 0;

	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_type != DT_REG)
			continue;
		uid = strtonum(dp->d_name, 0, UID_MAX, &errstr);
		if (errstr) {
			log_warnx("sfs_cert_load_db: filename \"%s/%s\" %s",
			    dirname, dp->d_name, errstr);
			continue;
		}
		if (asprintf(&filename, "%s/%s", dirname, dp->d_name) == -1) {
			log_warnx("asprintf: Out of memory");
			continue;
		}
		if ((sc = calloc(1, sizeof(struct sfs_cert))) == NULL) {
			log_warnx("calloc: Out of memory");
			free(filename);
			continue;
		}
		if ((fp = fopen(filename, "r")) == NULL) {
			log_warn("fopen %s", filename);
			free(sc);
			free(filename);
			continue;
		}
		if((sc->req = PEM_read_X509(fp, NULL, NULL, NULL)) == NULL) {
			log_warn("PEM_read_X509: %s", filename);
			free(sc);
			fclose(fp);
			continue;
		}
		fclose(fp);
		if ((sc->pkey = X509_get_pubkey(sc->req)) == NULL) {
			log_warn("X509_get_pubkey: %s", filename);
			X509_free(sc->req);
			free(sc);
			continue;
		}
		sc->uid = uid;
		sc->kidlen = sfs_cert_keyid(&sc->keyid, sc->req);
		if (sc->kidlen == -1) {
			log_warn("Error while loading key from uid %d", uid);
			X509_free(sc->req);
			EVP_PKEY_free(sc->pkey);
			free(sc);
			free(filename);
			continue;
		}
		TAILQ_INSERT_TAIL(certs, sc, entry);
		count++;
		free(filename);
	}

	if (closedir(dir) == -1)
		log_warn("closedir");

	return count;
}

static int
sfs_cert_keyid(unsigned char **keyid, X509 *cert)
{
	X509_EXTENSION *ext = NULL;
	ASN1_OCTET_STRING *o_asn = NULL;
	int rv = -1, len = -1;

	if (keyid == NULL || cert == NULL)
		return -1;

	for (rv = X509_get_ext_by_NID(cert, NID_subject_key_identifier, rv);
	    rv >= 0;
	    rv = X509_get_ext_by_NID(cert, NID_subject_key_identifier, rv))
		ext = X509_get_ext(cert, rv);

	o_asn = X509_EXTENSION_get_data(ext);
	if (!o_asn) {
		log_warn("X509_EXTENSION_get_data\n");
		return -1;
	}

	/* The result of X509_EXTENSION_get_data beginns with
	 * a number which discripes algorithm (see RFC 3280)
	 * We just need the KeyID, for that we cut a way these number
	 */
	if (!memcmp(o_asn->data, "\004\024", 2)) {
		len = ASN1_STRING_length(o_asn) - 2;
		if ((*keyid = calloc(len, sizeof(unsigned char))) == NULL)
			return -1;
		memcpy(*keyid, &o_asn->data[2], len);
	} else {
		len = ASN1_STRING_length(o_asn);
		if ((*keyid = calloc(len, sizeof(unsigned char))) == NULL)
			return -1;
		memcpy(*keyid, &o_asn->data, len);
	}

	return (len);
}

static void
sfs_cert_flush_db(struct sfs_cert_db *sc)
{
	struct sfs_cert	*p, *next;

	if (sc == NULL)
		sc = sfs_certs;
	for (p = TAILQ_FIRST(sc); p != TAILQ_END(sc); p = next) {
		next = TAILQ_NEXT(p, entry);
		TAILQ_REMOVE(sc, p, entry);

		EVP_PKEY_free(p->pkey);
		X509_free(p->req);
		free(p->keyid);
		free(p);
	}
}

struct sfs_cert *
sfs_pubkey_get_by_uid(uid_t uid)
{
	struct sfs_cert *p;

	TAILQ_FOREACH(p, sfs_certs, entry) {
		if (p->uid == uid)
			return p;
	}

	return NULL;
}

struct sfs_cert *
sfs_pubkey_get_by_keyid(unsigned char *keyid, int klen)
{
	struct sfs_cert *p;
	int ret = 0;

	if (keyid == 0)
		return NULL;
	TAILQ_FOREACH(p, sfs_certs, entry) {
		if (klen != p->kidlen)
			continue;
		if ((ret = memcmp(keyid, p->keyid, p->kidlen)) == 0)
			return p;
	}

	return NULL;
}

char *
sfs_cert_keyid_for_uid(uid_t uid)
{
	struct sfs_cert		*p;
	char			*ret;
	int			 i, j;

	TAILQ_FOREACH(p, sfs_certs, entry) {
		if (p->uid == uid)
			break;
	}
	if (!p)
		return NULL;
	ret = malloc(2*p->kidlen+1);
	if (!ret)
		return NULL;
	for (i=j=0; i<p->kidlen; ++i) {
		sprintf(ret+j, "%02x", p->keyid[i]);
		j += 2;
	}
	ret[j] = 0;
	return ret;
}

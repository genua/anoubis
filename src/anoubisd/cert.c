/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#include "cert.h"
#include "pe.h"

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

#include <anoubis_sig.h>

#include "anoubis_alloc.h"


TAILQ_HEAD(cert_db, cert) *certs;

static int		 cert_load_db(const char *, struct cert_db *);
static void		 cert_flush_db(struct cert_db *sc);
static int		 cert_keyid(unsigned char **keyid, X509 *cert);
static struct cert	*_cert_get_by_keyid(const unsigned char *keyid,
			    int klen, int check_ignore, struct cert_db *certdb);
static struct cert	*_cert_get_by_uid(uid_t, int);

/**
 * Initialise the database of certificates. Allocate memory and load the
 * certificates into the database.
 *
 * @param chroot True if the process which calls this function is chrooted.
 * @return None;
 */
void
cert_init(int chroot)
{
	struct cert_db	*cdb;
	int			 count;
	const char		*certdir = CERT_DIR;

	if (chroot)
		certdir = CERT_DIR_CHROOT;
	if ((cdb = abuf_alloc_type(struct cert_db)) == NULL) {
		log_warn("%s: abuf_alloc_type", certdir);
		master_terminate(ENOMEM);
	}
	TAILQ_INIT(cdb);

	if ((count = cert_load_db(certdir, cdb)) == -1)
		fatal("cert_init: failed to load sfs certificates");

	log_info("%d certificates loaded from %s", count, certdir);
	certs = cdb;
}

/**
 * Delete all certificates from the database and reread the database
 * from disk.
 * @param chroot True if the process which calls this function is chrooted.
 * @return None.
 */
void
cert_reconfigure(int chroot)
{
	struct cert_db	*newdb, *old;
	int			 count;
	const char		*certdir = CERT_DIR;

	if (chroot)
		certdir = CERT_DIR_CHROOT;
	if ((newdb = abuf_alloc_type(struct cert_db)) == NULL) {
		log_warn("abuf_alloc_type");
		master_terminate(ENOMEM);
	}
	TAILQ_INIT(newdb);

	if ((count = cert_load_db(certdir, newdb)) == -1) {
		log_warnx("cert_reconfigure: could not load public keys");
		abuf_free_type(newdb, struct cert_db);
		return;
	}
	log_info("%d certificates loaded from %s", count, certdir);

	old = certs;
	certs = newdb;
	cert_flush_db(old);
}

/**
 * Read certificates from a specified directory and load them into the
 * database given as argument. The certificates must be named according to
 * the numerical user IDs of their respective owner.
 *
 * @param dirname Directory containing the certificates.
 * @param certs	  Database where the certificates shall be stored.
 *
 * @return Number of certificates loaded.
 */
static int
cert_load_db(const char *dirname, struct cert_db *certs)
{
	DIR		*dir;
	FILE		*fp;
	struct dirent	*dp;
	struct cert	*sc, *check;
	uid_t		 uid;
	int		 count;
	const char	*errstr;
	char		*filename;

	if (certs == NULL) {
		log_warnx("cert_load_db: illegal sfscert queue");
		return 0;
	}

	if ((dir = opendir(dirname)) == NULL) {
		log_warn("opendir %s", dirname);
		return 0;
	}

	count = 0;

	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_type != DT_REG && dp->d_type != DT_UNKNOWN)
			continue;
		uid = strtonum(dp->d_name, 0, UID_MAX, &errstr);
		if (errstr) {
			log_warnx("cert_load_db: filename \"%s/%s\" %s",
			    dirname, dp->d_name, errstr);
			continue;
		}
		if (asprintf(&filename, "%s/%s", dirname, dp->d_name) == -1) {
			log_warnx("asprintf: Out of memory");
			continue;
		}
		if (dp->d_type == DT_UNKNOWN) {
			struct stat	statbuf;
			if (lstat(filename, &statbuf) < 0) {
				log_warn("lstat: Cannot stat %s", filename);
			} else if (!S_ISREG(statbuf.st_mode)) {
				free(filename);
				continue;
			}
		}
		if ((sc = abuf_zalloc_type(struct cert)) == NULL) {
			log_warnx("zalloc_type: Out of memory");
			free(filename);
			continue;
		}
		sc->privkey = NULL;
		if ((fp = fopen(filename, "r")) == NULL) {
			log_warn("fopen %s", filename);
			abuf_free_type(sc, struct cert);
			free(filename);
			continue;
		}
		if((sc->req = PEM_read_X509(fp, NULL, NULL, NULL)) == NULL) {
			log_warnx("PEM_read_X509: %s", filename);
			abuf_free_type(sc, struct cert);
			fclose(fp);
			continue;
		}
		fclose(fp);
		if ((sc->pubkey = X509_get_pubkey(sc->req)) == NULL) {
			log_warnx("X509_get_pubkey: %s", filename);
			X509_free(sc->req);
			abuf_free_type(sc, struct cert);
			continue;
		}
		sc->uid = uid;
		sc->kidlen = cert_keyid(&sc->keyid, sc->req);
		if (sc->kidlen == -1) {
			log_warnx("Error while loading key from uid %d", uid);
			X509_free(sc->req);
			EVP_PKEY_free(sc->pubkey);
			abuf_free_type(sc, struct cert);
			free(filename);
			continue;
		}

		/*
		 * Check if the key was already loaded. If the key is found
		 * we will ignore it.
		 */
		if ((check = _cert_get_by_keyid(sc->keyid, sc->kidlen, 0,
		    certs))) {
			log_warnx("Warning: Certificate already found in "
			    "database. Will ignore for KeyID from uid %d "
			    "and %d", check->uid, uid);
			sc->ignore = 1;
			check->ignore = 1;
		} else
			sc->ignore = 0;
		TAILQ_INSERT_TAIL(certs, sc, entry);
		count++;
		free(filename);
	}

	if (closedir(dir) == -1)
		log_warn("closedir");

	return count;
}

/**
 * Read a KeyID from a certificate and store the result in a buffer.
 * The buffer is allocated using malloc and must be freed by the caller.
 * The KeyID is returned in binary form, it is not converted to a printable
 * string and is not terminated by a 0-Byte.
 *
 * @param keyid Pointer to a char pointer where the result gets stored.
 * @param cert  The X509 certificate which contains the KeyID.
 *
 * @return Size of allocated memory for the result buffer in keyid.
 */
static int
cert_keyid(unsigned char **keyid, X509 *cert)
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

	/*
	 * The result of X509_EXTENSION_get_data begins with
	 * a number which discribes algorithm (see RFC 3280)
	 * We just need the KeyID, for that we cut a way these number
	 */
	if (memcmp(o_asn->data, "\004\024", 2) == 0) {
		len = ASN1_STRING_length(o_asn) - 2;
		if ((*keyid = calloc(len, sizeof(unsigned char))) == NULL)
			return -1;
		memcpy(*keyid, &o_asn->data[2], len);
	} else {
		/*
		 * XXX KM: In case of changing this strange behavior
		 * XXX KM: also put in a variant where the 2 bytes
		 * XXX KM: doesn't exist.
		 */
		len = ASN1_STRING_length(o_asn);
		if ((*keyid = calloc(len, sizeof(unsigned char))) == NULL)
			return -1;
		memcpy(*keyid, &o_asn->data, len);
	}

	return (len);
}

/**
 * Delete all certificates from the database and free the memory.
 *
 * @param sc Database of certificates
 * @return None.
 */
static void
cert_flush_db(struct cert_db *sc)
{
	struct cert	*p, *next;

	if (sc == NULL)
		sc = certs;
	for (p = TAILQ_FIRST(sc); p != TAILQ_END(sc); p = next) {
		next = TAILQ_NEXT(p, entry);
		TAILQ_REMOVE(sc, p, entry);

		EVP_PKEY_free(p->pubkey);
		if (p->privkey)
			EVP_PKEY_free(p->privkey);
		X509_free(p->req);
		free(p->keyid);
		abuf_free_type(p, struct cert);
	}
	abuf_free_type(sc, struct cert_db);
}

/**
 * Get a pointer to the certificate of a specified user from the database.
 * Include certificates in this search which should be ignored.
 *
 * @param uid User whose certificate is searched in the database.
 * @return The certificate of the user or NULL if no certificate was found.
 *
 * NOTE: This function will return certificates with the ignore flag
 *       set. This is usually not what you want. Use cert_get_by_uid instead!
 */
struct cert *
cert_get_by_uid_ignored(uid_t uid)
{
	return _cert_get_by_uid(uid, 0);
}

/**
 * Get a pointer to the certificate of a specified user from the database.
 * This function will not return certificates which should be ignored.
 *
 * @param uid User which certificate is searched in the database.
 * @return The certificate of the user or NULL if no certificate was found.
 */
struct cert *
cert_get_by_uid(uid_t uid)
{
	return _cert_get_by_uid(uid, 1);
}

/**
 * Get a pointer to the certificate of a specified user from the database.
 * If specified include certificates in this search which should be ignored.
 *
 * @param uid User whose certificate is searched in the database.
 * @param check_ignore True is the ignore flag of a certificate should be
 *     honored.
 * @return The certificate of the user or NULL if no certificate was found.
 */
static struct cert *
_cert_get_by_uid(uid_t uid, int check_ignore)
{
	struct cert *p;

	TAILQ_FOREACH(p, certs, entry) {
		if (p->uid == uid) {
			/*
			 * We will return NULL since the
			 * uid is unique in the database
			 */
			if (check_ignore && p->ignore == 1)
				return NULL;
			return p;
		}
	}
	return NULL;
}

/**
 * Get the certificate that must be used to check the signature of a policy
 * for the given user and priority. As admin policies must be signed by root
 * this function will return root's certificate if the priority is ADMIN.
 * Otherwise, the certificate of the user is returned.
 *
 * @param uid The specified user
 * @param prio The priority of the policy from the specified user.
 * @return The certificate or NULL if the policy needs no signature.
 */
struct cert *
cert_get_by_uid_prio(uid_t uid, int prio)
{
	if (prio == PE_PRIO_ADMIN || uid == (uid_t)-1)
		return cert_get_by_uid(0);
	else
		return cert_get_by_uid(uid);
}

/**
 * Return the certificate with the given keyid from the database.
 * The keyid must be given in binary format not as a printable string.
 *
 * @param keyid The id which specifies the certificate.
 * @param klen The length of the keyid.
 * @return The certificate or NULL if non certificate with the given keyid
 *     was found.
 */
struct cert *
cert_get_by_keyid(const unsigned char *keyid, int klen)
{
	return _cert_get_by_keyid(keyid, klen, 1, certs);
}

/**
 * Return the certificate for the given keyid from a specified database.
 * If specified also include ignored certificates in the search.
 *
 * @param keyid The id which specifies the certificate.
 * @param klen The length of the keyid.
 * @param check_ignore If a certificate should be ignored, ignore it really.
 *     You should not set this to false unless you really know what you are
 *     doing.
 * @param certdb Look up certificate in this database.
 * @return Certificate which matches the the keyid or NULL if no suitable
 *     certificate was found.
 *
 * NOTE: The database can contain multiple keys with the same keyid.
 *     Normally, all of them are ignored. However, if multiple certificates
 *     exist and check_ignored is false, it is unspecified which of these
 *     certificates is returned. This feature is intended for internal use
 *     only.
 */
static struct cert *
_cert_get_by_keyid(const unsigned char *keyid, int klen, int check_ignore,
    struct cert_db *certdb)
{
	struct cert *p;

	if (keyid == 0)
		return NULL;
	TAILQ_FOREACH(p, certdb, entry) {
		if (klen != p->kidlen)
			continue;
		if (memcmp(keyid, p->keyid, p->kidlen) == 0) {
			if (check_ignore  && p->ignore)
				return NULL;
			return p;
		}
	}

	return NULL;
}

/**
 * Return keyid for the specified user-ID as a printable string. The
 * memory for the string is allocated using malloc(3) and must be freed
 * by the caller.
 *
 * @param uid The user ID.
 * @return The corresponding user-ID converted to a printable string.
 *     NULL if no certificate was found for the user-ID.
 */
char *
cert_keyid_for_uid(uid_t uid)
{
	struct cert		*p;
	char			*ret;
	int			 i, j;

	TAILQ_FOREACH(p, certs, entry) {
		if (p->uid == uid && p->ignore == 0)
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

/**
 * Load a private key from path. Use the given passphrase if necessary.
 * The private key is stored in the certificate for later use.
 *
 * @param cert Certificate where the key should be stored.
 * @param path Path to the private key.
 * @param passphrase Passphrase of the private key.
 * @return None.
 */
void
cert_load_priv_key(struct cert *cert, const char *path, char *passphrase)
{
	EVP_PKEY	*priv;
	FILE		*file;

	if (cert && cert->privkey) {
		EVP_PKEY_free(cert->privkey);
		cert->privkey = NULL;
	}

	if (!cert || !path)
		return;
	if (passphrase == NULL)
		passphrase = "";
	if ((file = fopen(path, "r")) == NULL) {
		log_warnx("Cannot open rootkey file %s", path);
		return;
	}
	OpenSSL_add_all_algorithms();
	priv = PEM_read_PrivateKey(file, NULL, NULL, passphrase);
	fclose(file);
	if (!priv) {
		if (passphrase)
			log_warnx("Failed to read private rootkey %s", path);
		return;
	}
	/* XXX CEH: Make sure that private and public keys match! */
	cert->privkey = priv;
}

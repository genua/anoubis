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

#define ANOUBISD_CERT_DIR ANOUBISD_POLICYDIR ANOUBISD_POLICYCHROOT "/pubkeys"
#define ANOUBISD_CERT_DIR_CHROOT ANOUBISD_POLICYCHROOT "/pubkeys"


TAILQ_HEAD(cert_db, cert) *certs;

static int	 cert_load_db(const char *, struct cert_db *);
static void	 cert_flush_db(struct cert_db *sc);
static int	 cert_keyid(unsigned char **keyid, X509 *cert);

void
cert_init(int chroot)
{
	struct cert_db	*cdb;
	int			 count;
	const char		*certdir = ANOUBISD_CERT_DIR;

	if (chroot)
		certdir = ANOUBISD_CERT_DIR_CHROOT;
	if ((cdb = calloc(1, sizeof(struct cert_db))) == NULL) {
		log_warn("%s: calloc", certdir);
		master_terminate(ENOMEM);
	}
	TAILQ_INIT(cdb);

	if ((count = cert_load_db(certdir, cdb)) == -1)
		fatal("cert_init: failed to load sfs certificates");

	log_info("%d certificates loaded from %s", count, certdir);
	certs = cdb;
}

void
cert_reconfigure(int chroot)
{
	struct cert_db	*new, *old;
	int			 count;
	const char		*certdir = ANOUBISD_CERT_DIR;

	if (chroot)
		certdir = ANOUBISD_CERT_DIR_CHROOT;
	if ((new = calloc(1, sizeof(struct cert_db))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);
	}
	TAILQ_INIT(new);

	if ((count = cert_load_db(certdir, new)) == -1) {
		log_warnx("cert_reconfigure: could not load public keys");
		free(new);
		return;
	}
	log_info("%d certificates loaded from %s", count, certdir);

	old = certs;
	certs = new;
	cert_flush_db(old);
}

static int
cert_load_db(const char *dirname, struct cert_db *certs)
{
	DIR		*dir;
	FILE		*fp;
	struct dirent	*dp;
	struct cert	*sc;
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
		if (dp->d_type != DT_REG)
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
		if ((sc = calloc(1, sizeof(struct cert))) == NULL) {
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
		sc->kidlen = cert_keyid(&sc->keyid, sc->req);
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
cert_flush_db(struct cert_db *sc)
{
	struct cert	*p, *next;

	if (sc == NULL)
		sc = certs;
	for (p = TAILQ_FIRST(sc); p != TAILQ_END(sc); p = next) {
		next = TAILQ_NEXT(p, entry);
		TAILQ_REMOVE(sc, p, entry);

		EVP_PKEY_free(p->pkey);
		X509_free(p->req);
		free(p->keyid);
		free(p);
	}
}

struct cert *
cert_get_by_uid(uid_t uid)
{
	struct cert *p;

	TAILQ_FOREACH(p, certs, entry) {
		if (p->uid == uid)
			return p;
	}

	return NULL;
}

struct cert *
cert_get_by_keyid(unsigned char *keyid, int klen)
{
	struct cert *p;

	if (keyid == 0)
		return NULL;
	TAILQ_FOREACH(p, certs, entry) {
		if (klen != p->kidlen)
			continue;
		if (memcmp(keyid, p->keyid, p->kidlen) == 0)
			return p;
	}

	return NULL;
}

char *
cert_keyid_for_uid(uid_t uid)
{
	struct cert		*p;
	char			*ret;
	int			 i, j;

	TAILQ_FOREACH(p, certs, entry) {
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

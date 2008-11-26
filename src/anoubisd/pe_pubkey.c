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
#ifdef OPENBSD
#include <sys/limits.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <openssl/pem.h>

#ifdef LINUX
#include <queue.h>
#include <bsdcompat.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/queue.h>
#include <dev/anoubis.h>
#endif

#include "anoubisd.h"
#include "pe.h"

struct pe_pubkey {
	TAILQ_ENTRY(pe_pubkey)	 entry;
	uid_t			 uid;
	EVP_PKEY		*pubkey;
	X509			*cert;
};
TAILQ_HEAD(pe_pubkey_db, pe_pubkey) *pubkeys;

static int		 pe_pubkey_load_db(const char *, struct pe_pubkey_db *);
static EVP_PKEY		*pe_pubkey_get(uid_t);

void
pe_pubkey_init(void)
{
	struct pe_pubkey_db	*pk;
	if ((pk = calloc(1, sizeof(struct pe_pubkey_db))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);	/* XXX HSH */
	}
	TAILQ_INIT(pk);

	/* We die gracefully if loading of public keys fails. */
	if (pe_pubkey_load_db(ANOUBISD_PUBKEYDIR, pk) == -1)
		fatal("pe_init: failed to load policy keys");
	pubkeys = pk;
}

void
pe_pubkey_reconfigure(void)
{
	struct pe_pubkey_db	*newpk, *oldpk;

	if ((newpk = calloc(1, sizeof(struct pe_pubkey_db))) == NULL) {
		log_warn("calloc");
		master_terminate(ENOMEM);       /* XXX HSH */
	}
	TAILQ_INIT(newpk);

	if (pe_pubkey_load_db(ANOUBISD_PUBKEYDIR, newpk) == -1) {
		log_warnx("pe_reconfigure: could not load public keys");
		free(newpk);
		return;
	}

	/*
	 * We switch pubkeys right now, so newly loaded policies can
	 * be verfied using the most recent keys.
	 */
	oldpk = pubkeys;
	pubkeys = newpk;
	pe_pubkey_flush_db(oldpk);
}

static int
pe_pubkey_load_db(const char *dirname, struct pe_pubkey_db *pubkeys)
{
	DIR			*dir;
	FILE			*fp;
	struct dirent		*dp;
	struct pe_pubkey	*pk;
	uid_t			 uid;
	int			 count;
	const char		*errstr;
	char			*filename;

	DEBUG(DBG_PE_POLICY, "pe_pubkey_load_db: %s %p", dirname, pubkeys);

	if (pubkeys == NULL) {
		log_warnx("pe_pubkey_load_db: illegal pubkey queue");
		return (0);
	}

	if ((dir = opendir(dirname)) == NULL) {
		log_warn("opendir");
		return (0);
	}

	count = 0;

	/* iterate over directory entries */
	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_type != DT_REG)
			continue;
		/*
		 * Validate the file name: It has to be a numeric uid.
		 */
		uid = strtonum(dp->d_name, 0, UID_MAX, &errstr);
		if (errstr) {
			log_warnx("pe_pubkey_load_db: filename \"%s/%s\" %s",
			    dirname, dp->d_name, errstr);
			continue;
		}
		if (asprintf(&filename, "%s/%s", dirname, dp->d_name) == -1) {
			log_warnx("asprintf: Out of memory");
			continue;
		}
		if ((pk = calloc(1, sizeof(struct pe_pubkey))) == NULL) {
			log_warnx("calloc: Out or memory");
			free(filename);
			continue;
		}
		if ((fp = fopen(filename, "r")) == NULL) {
			log_warn("fopen");
			free(pk);
			free(filename);
			continue;
		}
		free(filename);

		if ((pk->cert = PEM_read_X509(fp, NULL, NULL, NULL))
		    == NULL) {
			log_warn("PEM_read_X509");
			free(pk);
			fclose(fp);
			continue;
		}
		if ((pk->pubkey = X509_get_pubkey(pk->cert)) == NULL) {
			log_warn("X509_get_pubkey");
			X509_free(pk->cert);
			free(pk);
			fclose(fp);
			continue;
		}
		pk->uid = uid;
		fclose(fp);
		TAILQ_INSERT_TAIL(pubkeys, pk, entry);
		count++;
	}

	if (closedir(dir) == -1)
		log_warn("closedir");

	DEBUG(DBG_PE_POLICY, "pe_pubkey_load_db: %d public keys loaded", count);

	return (count);
}

void
pe_pubkey_flush_db(struct pe_pubkey_db *pk)
{
	struct pe_pubkey	*p, *next;

	if (pk == NULL)
		pk = pubkeys;
	for (p = TAILQ_FIRST(pk); p != TAILQ_END(pk); p = next) {
		next = TAILQ_NEXT(p, entry);
		TAILQ_REMOVE(pk, p, entry);

		X509_free(p->cert);
		EVP_PKEY_free(p->pubkey);
		free(p);
	}
}

int
pe_pubkey_verifysig(const char *filename, uid_t uid)
{
	char		 buffer[1024];
	EVP_PKEY	*sigkey;
	EVP_MD_CTX	 ctx;
	unsigned char	*sigbuf;
	char		*sigfile;
	int		 fd, n, siglen, result;

	if (asprintf(&sigfile, "%s.sig", filename) == -1) {
		log_warnx("asprintf: Out of memory");
		return (0);	/* XXX policy will not be loaded... */
	}

	/* If no signature file is available, return successfully */
	if ((fd = open(sigfile, O_RDONLY)) == -1) {
		free(sigfile);
		return (1);
	}
	free(sigfile);

	/* Fail when we have no public key for that uid */
	if ((sigkey = pe_pubkey_get(uid)) == NULL) {
		log_warnx("pe_pubkey_verifysig: no key for uid %lu available",
		    (unsigned long)uid);
		close(fd);
		return (0);
	}

	/* Read in signature */
	siglen = EVP_PKEY_size(sigkey);
	if ((sigbuf = calloc(siglen, sizeof(unsigned char))) == NULL) {
		close(fd);
		return (0);
	}
	if (read(fd, sigbuf, siglen) != siglen) {
		log_warnx("pe_pubkey_verifysig: error reading "
		    "signature file %s", sigfile);
		close(fd);
		free(sigbuf);
		return (0);
	}
	close(fd);

	EVP_MD_CTX_init(&ctx);
	if (EVP_VerifyInit(&ctx, EVP_sha1()) == 0) {
		log_warnx("pe_pubkey_verifysig: could not verify signature");
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return (0);
	}

	/* Open policy file */
	if ((fd = open(filename, O_RDONLY)) == -1) {
		log_warnx("pe_pubkey_verifysig: could not read policy %s",
		    filename);
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return (0);
	}

	while ((n = read(fd, buffer, sizeof(buffer))) > 0)
		EVP_VerifyUpdate(&ctx, buffer, n);
	if (n == -1) {
		log_warn("read");
		close(fd);
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return (0);
	}
	close(fd);

	/* Verify */
	if ((result = EVP_VerifyFinal(&ctx, sigbuf, siglen, sigkey)) == -1)
		log_warnx("pe_pubkey_verifysig: could not verify signature");

	EVP_MD_CTX_cleanup(&ctx);
	free(sigbuf);

	return (result);
}

static EVP_PKEY *
pe_pubkey_get(uid_t uid)
{
	struct pe_pubkey	*p;

	TAILQ_FOREACH(p, pubkeys, entry) {
		if (p->uid == uid)
			return (p->pubkey);
	}

	return (NULL);
}

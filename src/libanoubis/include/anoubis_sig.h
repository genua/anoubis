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

#ifndef _ANOUBIS_SIG_H_
#define _ANOUBIS_SIG_H_

#include <config.h>

#include <sys/cdefs.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/objects.h>
#include <openssl/err.h>

#ifdef LINUX
#include <openssl/sha.h>
#endif
#ifdef OPENBSD
#include <sha2.h>
#endif

#define __used __attribute__((unused))

#define ANOUBIS_SIG_HASH_SHA256_LEN 32
#define	ANOUBIS_SIG_NAME_LEN 256

#define ANOUBIS_SIG_PUB		0
#define ANOUBIS_SIG_PRIV	1

#define ANOUBIS_SIG_HASH_MD_NULL	EVP_md_null()
#define ANOUBIS_SIG_HASH_MD2		EVP_md2()
#define ANOUBIS_SIG_HASH_MD5		EVP_md5()
#define ANOUBIS_SIG_HASH_SHA		EVP_sha()
#define ANOUBIS_SIG_HASH_SHA1		EVP_sha1()
#define ANOUBIS_SIG_HASH_DSS		EVP_dss()
#define ANOUBIS_SIG_HASH_DSS1		EVP_dss1()
#define ANOUBIS_SIG_HASH_MDC2		EVP_mdc2()
#define ANOUBIS_SIG_HASH_RIPEMD160	EVP_ripemd160()

#define ANOUBIS_DEF_CRT	".xanoubis/default.crt"
#define ANOUBIS_DEF_KEY ".xanoubis/default.key"

/*
 * XXX KM for now this is the standard HASH until i figured out
 * how to syncronize it with the daemon
 */
#define ANOUBIS_SIG_HASH_DEFAULT	EVP_sha1()
/* XXX KM */

/*
 * If you add another entry into the struct
 * don't forget to free it in anoubis_sig_free
 */
struct anoubis_sig {
	/** The signature type that is used with these keys. */
	const EVP_MD	*type;
	/** The certificate that contains the public key. */
	X509		*cert;
	/** The keyid extracted from the certificate. */
	unsigned char	*keyid;
	/** The length of the keyid. */
	int		 idlen;
	/** The private key extracted from the private key file. */
	EVP_PKEY	*pkey;
};

__BEGIN_DECLS

unsigned char *anoubis_sign_csum(struct anoubis_sig *as,
    unsigned char csum[ANOUBIS_SIG_HASH_SHA256_LEN], unsigned int *len);
unsigned char *anoubis_sign_policy(struct anoubis_sig *as, char *file,
    unsigned int *len);
unsigned char *anoubis_sign_policy_buf(struct anoubis_sig *as, char *buf,
    unsigned int *len);
int anoubis_sig_verify_policy_file(const char *filename, EVP_PKEY *sigkey);
int anoubis_sig_verify_policy(const char *filename,
   unsigned char *sigbuf, int siglen, EVP_PKEY *sigkey);
int anoubis_sig_sign_buffer(const void *, int len, void **sigp, int *siglenp,
    struct anoubis_sig *as);
int anoubis_sig_verify_buffer(const void *data, int datalen,
    const void *sig, int siglen, EVP_PKEY *key);
int anoubisd_verify_csum(EVP_PKEY *pkey,
     const unsigned char csum[ANOUBIS_SIG_HASH_SHA256_LEN],
     const unsigned char *sfs_sign, int sfs_len);
int anoubis_sig_create(struct anoubis_sig **, const char *keyfile,
    const char *certfile, pem_password_cb *passcb);
void anoubis_sig_free(struct anoubis_sig *as);
int pass_cb(char *buf, int size, int rwflag, void *u);
char *anoubis_sig_key2char(int idlen, const unsigned char *keyid);
char *anoubis_sig_cert_name(X509 *cert);
int anoubis_sig_check_consistency(const struct anoubis_sig *);


/**
 * Get certificate validity.
 *
 * This method returns a newly allocated time structure tm filled with
 * the certificates 'notAfter' time values.
 *
 * @param cert The certificate in question.
 * @param time The time structure to be filled.
 * @return 0 on success.
 *	-EINVAL in case of invalid argument.
 *	-ERANGE in case of corrupt notAfter time.
 */
int anoubis_sig_cert_validity(X509 *cert, struct tm *time);

/**
 * Compares two keys.
 *
 * This method takes the pkey elements of two keys given as anoubis_sig
 * structures and compares those with EVP_PKEY_cmp(). It works with two
 * private keys, a key pais split in two anoubis_sig structures or with
 * two public keys.
 *
 * @param keyA one key
 * @param keyB another key
 * @return 0 if they are equal.
 *	-EINVAL in case of invalid argument.
 *	-EPERM in case of not equal.
 */
int anoubis_sig_keycmp(struct anoubis_sig *, struct anoubis_sig *);

__END_DECLS

#endif	/* _ANOUBIS_SIG_H_ */

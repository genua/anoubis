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

#include "anoubis_sig.h"

static int cert_keyid(unsigned char **keyid, X509 *cert);

/* This function returns a memory area filled with the checksum (csum)
 * and the signature of the checksum (sigbuf)
 *	-------------------------------------------------
 *	|	csum	|	sigbuf			|
 *	-------------------------------------------------
 * The returned len is the complete len of the memory area
 * since the len of checksum is known.
 */

unsigned char *
anoubis_sign_csum(struct anoubis_sig *as,
    unsigned char csum[ANOUBIS_SIG_HASH_SHA256_LEN], unsigned int *len)
{
	unsigned char *sigbuf = NULL;
	unsigned char *result = NULL;
	unsigned int siglen = 0;
	EVP_MD_CTX ctx;
	EVP_PKEY *pkey;

	if (as == NULL || len == NULL || csum == NULL)
		return NULL;

	pkey = as->pkey;
	siglen = EVP_PKEY_size(pkey);

	if ((sigbuf = calloc(siglen, sizeof(unsigned char))) == NULL)
		return NULL;
	EVP_MD_CTX_init(&ctx);
	if (EVP_SignInit(&ctx, as->type) == 0) {
		EVP_MD_CTX_cleanup(&ctx);
		free(sigbuf);
		return NULL;
	}
	EVP_SignUpdate(&ctx, csum, ANOUBIS_SIG_HASH_SHA256_LEN);
	if (EVP_SignFinal(&ctx, sigbuf, &siglen, pkey) == 0) {
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return NULL;
	}
	EVP_MD_CTX_cleanup(&ctx);
	result = calloc((ANOUBIS_SIG_HASH_SHA256_LEN + siglen),
	    sizeof(unsigned char));
	if (!result) {
		*len = 0;
		return NULL;
	}

	memcpy(result, csum, ANOUBIS_SIG_HASH_SHA256_LEN);
	memcpy(result + ANOUBIS_SIG_HASH_SHA256_LEN, sigbuf, siglen);
	free(sigbuf);
	*len = siglen + ANOUBIS_SIG_HASH_SHA256_LEN;
	return result;
}

int
anoubis_sig_verify_policy_file(const char *filename, EVP_PKEY *sigkey)
{
	char		*sigfile = NULL;
	unsigned char	*sigbuf = NULL;
	int		 siglen, fd, ret;

	if (!filename || !sigkey)
		return (-1);

	if (asprintf(&sigfile, "%s.sig", filename) == -1)
		return (-1);

	if ((fd = open(sigfile, O_RDONLY)) == -1) {
		free(sigfile);
		return (-2);
	}
	free(sigfile);

	/* Read in signature */
	siglen = EVP_PKEY_size(sigkey);
	if ((sigbuf = calloc(siglen, sizeof(unsigned char))) == NULL) {
		close(fd);
		return (-3);
	}
	while ((ret = read(fd, sigbuf, siglen)) > 0) {
		if (ret == -1) {
			close(fd);
			free(sigbuf);
			return (-4);
		}
	}
	close(fd);

	ret = anoubis_sig_verify_policy(filename, sigbuf, siglen, sigkey);

	free(sigbuf);

	return ret;
}

int
anoubis_sig_verify_buffer(const void *buf, int buflen, const void *sig,
    int siglen, EVP_PKEY *key)
{
	EVP_MD_CTX	ctx;
	int		result;

	if (!key || !buf || !sig)
		return -EINVAL;
	if (siglen != EVP_PKEY_size(key))
		return -ERANGE;
	EVP_MD_CTX_init(&ctx);
	if (EVP_VerifyInit(&ctx, ANOUBIS_SIG_HASH_DEFAULT) == 0) {
		EVP_MD_CTX_cleanup(&ctx);
		return -ENOMEM;
	}
	EVP_VerifyUpdate(&ctx, buf, buflen);
	result = EVP_VerifyFinal(&ctx, sig, siglen, key);
	EVP_MD_CTX_cleanup(&ctx);
	if (result <= 0)
		return -EPERM;
	return 0;
}

int
anoubis_sig_sign_buffer(const void *buf, int buflen, void **sigp, int *siglenp,
    struct anoubis_sig *as)
{
	unsigned int	 len;
	int		 result;
	void		*sig;
	EVP_MD_CTX	 ctx;

	if (!as || !as->pkey || !buf || !sigp || !siglenp)
		return -EINVAL;
	(*sigp) = NULL;
	(*siglenp) = 0;
	len = EVP_PKEY_size(as->pkey);
	sig = malloc(len);
	if (!sig)
		return -ENOMEM;
	EVP_MD_CTX_init(&ctx);
	if (!EVP_SignInit(&ctx, as->type)) {
		EVP_MD_CTX_cleanup(&ctx);
		free(sig);
		return -ENOMEM;
	}
	EVP_SignUpdate(&ctx, buf, buflen);
	result = EVP_SignFinal(&ctx, sig, &len, as->pkey);
	EVP_MD_CTX_cleanup(&ctx);
	if (result != 1) {
		free(sig);
		return -EPERM;
	}
	(*sigp) = sig;
	(*siglenp) = len;
	return 0;
}

int
anoubis_sig_verify_policy(const char *filename, unsigned char *sigbuf,
    int siglen, EVP_PKEY *sigkey)
{
	char		 buffer[1024];
	EVP_MD_CTX	 ctx;
	int		 fd, n, result = 0;

	EVP_MD_CTX_init(&ctx);
	if (EVP_VerifyInit(&ctx, ANOUBIS_SIG_HASH_DEFAULT) == 0) {
		EVP_MD_CTX_cleanup(&ctx);
		return (-5);
	}

	/* Open policy file */
	if ((fd = open(filename, O_RDONLY)) == -1) {
		EVP_MD_CTX_cleanup(&ctx);
		return (-6);
	}

	while ((n = read(fd, buffer, sizeof(buffer))) > 0)
		EVP_VerifyUpdate(&ctx, buffer, n);
	if (n == -1) {
		close(fd);
		EVP_MD_CTX_cleanup(&ctx);
		return (-7);
	}
	close(fd);

	/* Verify */
	if ((result = EVP_VerifyFinal(&ctx, sigbuf, siglen, sigkey)) == -1)
		result = -8;

	EVP_MD_CTX_cleanup(&ctx);

	return (result);
}

unsigned char *
anoubis_sign_policy_buf(struct anoubis_sig *as, char *buf, unsigned int *len)
{
	unsigned char	*sigbuf = NULL;
	unsigned int	 buf_len;
	unsigned int	 siglen = 0;
	EVP_MD_CTX	 ctx;
	EVP_PKEY	*pkey;

	if (as == NULL || len == NULL || buf == NULL) {
		*len = -EINVAL;
		return NULL;
	}

	buf_len = strlen(buf);
	if (buf_len != *len) {
		*len = -ERANGE;
		return NULL;
	}
	pkey = as->pkey;
	siglen = EVP_PKEY_size(pkey);

	if ((sigbuf = calloc(siglen, sizeof(unsigned char))) == NULL)
		return NULL;


	EVP_MD_CTX_init(&ctx);
	if (EVP_SignInit(&ctx, as->type) == 0) {
		EVP_MD_CTX_cleanup(&ctx);
		free(sigbuf);
		*len = -ERR_get_error();
		return NULL;
	}
	EVP_SignUpdate(&ctx, buf, buf_len);
	if (EVP_SignFinal(&ctx, sigbuf, &siglen, pkey) == 0) {
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		*len = -ERR_get_error();
		return NULL;
	}
	EVP_MD_CTX_cleanup(&ctx);

	*len = siglen;
	return sigbuf;
}

unsigned char *
anoubis_sign_policy(struct anoubis_sig *as, char *file, unsigned int *len)
{
	unsigned char	*sigbuf = NULL;
	char		*buf[1024];
	int		 fd, n;
	unsigned int	 siglen = 0;
	EVP_MD_CTX	 ctx;
	EVP_PKEY	*pkey;

	if (as == NULL || len == NULL || file == NULL)
		return NULL;

	pkey = as->pkey;
	siglen = EVP_PKEY_size(pkey);

	/* Open Policy file */
	if ((fd = open(file, O_RDONLY)) == -1)
		return NULL;

	if ((sigbuf = calloc(siglen, sizeof(unsigned char))) == NULL) {
		close(fd);
		return NULL;
	}

	EVP_MD_CTX_init(&ctx);
	if (EVP_SignInit(&ctx, as->type) == 0) {
		EVP_MD_CTX_cleanup(&ctx);
		close(fd);
		free(sigbuf);
		return NULL;
	}
	while ((n = read(fd, buf, sizeof(buf))) > 0)
		EVP_SignUpdate(&ctx, buf, n);
	if (n == -1) {
		close(fd);
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return NULL;
	}
	close(fd);
	if (EVP_SignFinal(&ctx, sigbuf, &siglen, pkey) == 0) {
		free(sigbuf);
		EVP_MD_CTX_cleanup(&ctx);
		return NULL;
	}
	EVP_MD_CTX_cleanup(&ctx);

	*len = siglen;
	return sigbuf;
}

/* This function will be used inside of the daemon */
int
anoubisd_verify_csum(EVP_PKEY *pkey,
     const unsigned char csum[ANOUBIS_SIG_HASH_SHA256_LEN],
     const unsigned char *sfs_sign, int sfs_len)
{
	int result = 0;
	EVP_MD_CTX ctx;

	if (pkey == NULL || csum == NULL || sfs_sign == NULL)
		return -1;

	EVP_MD_CTX_init(&ctx);
	if (EVP_VerifyInit(&ctx, ANOUBIS_SIG_HASH_DEFAULT) == 0) {
		EVP_MD_CTX_cleanup(&ctx);
		return -1;
	}
	EVP_VerifyUpdate(&ctx, csum, ANOUBIS_SIG_HASH_SHA256_LEN);
	result = EVP_VerifyFinal(&ctx, sfs_sign, sfs_len, pkey);

	EVP_MD_CTX_cleanup(&ctx);

	return result;
}

/**
 * Compare two EVP_PKEYs assuming that they are both RSA keys.
 * The keys are considered to be indentical if public modulus and exponent
 * are the same, i.e. if their public parts are the same.
 * @param key1 The first key.
 * @param key2 The second key.
 * @return Zero if the two keys are equal. -EINVAL if they are different.
 */
static int
anoubis_sig_dorsacmp(EVP_PKEY *key1, EVP_PKEY *key2)
{
	/* XXX CEH: Only works for RSA keys right now. */
	if (key1->type != key2->type || key1->type != EVP_PKEY_RSA)
		return 0;
	if (key1->pkey.rsa->n == NULL || key2->pkey.rsa->n == NULL)
		return -EINVAL;
	if (BN_cmp(key1->pkey.rsa->n, key2->pkey.rsa->n) != 0)
		return -EINVAL;
	if (key1->pkey.rsa->e == NULL || key2->pkey.rsa->e == NULL)
		return -EINVAL;
	if (BN_cmp(key1->pkey.rsa->e, key2->pkey.rsa->e) != 0)
		return -EINVAL;

	return 0;
}

/**
 * Verify that the anoubis signature structure contains consistent
 * public and private keys.
 * @param as The anoubis_sig structure.
 * @return Zero in case of success, a negative error code in case of an error.
 *
 * If either no cert or no private key has been loaded, the comparison will
 * be successful.
 */
int
anoubis_sig_check_consistency(const struct anoubis_sig *as)
{
	EVP_PKEY	*cert_pubkey = NULL;
	int		 ret;

	/* No private key or no certificate. This is ok. */
	if (as->pkey == NULL || as->cert == NULL)
		return 0;
	cert_pubkey = X509_get_pubkey(as->cert);
	if (!cert_pubkey)
		return -EIO;
	ret = anoubis_sig_dorsacmp(cert_pubkey, as->pkey);
	EVP_PKEY_free(cert_pubkey);
	return ret;
}

/**
 * Allocate a new anoubis_sig structure and initialize it with the givenb
 * key and certificate.
 *
 * @param asp The pointer to the new structure is stored *asp.
 * @param keyfile Read the private key from this file.
 * @param certfile Read the certificate from this file.
 * @param passcb Callback function to prompt the user for the password
 *     of the private key.
 * @return A negative error code in case of an error, zero in case of success.
 *     If an error occured *asp will be NULL otherwise *asp will point to
 *     the new anoubis_sig structure.
 *
 * NOTE: This funtion will verify that the private key (if any) matches
 *     the public key extracted from the certificate (if any).
 */
int
anoubis_sig_create(struct anoubis_sig **asp, const char *keyfile,
    const char *certfile, pem_password_cb *passcb)
{

	struct anoubis_sig *as = NULL;
	EVP_PKEY *pkey = NULL;
	FILE *f = NULL;
	X509	*cert = NULL;
	int err = 0;

	OpenSSL_add_all_algorithms();

	if (asp == NULL)
		return -EINVAL;
	*asp = NULL;
	if (!certfile && !keyfile)
		return -EINVAL;
	if (keyfile) {
		f = fopen(keyfile, "r");
		if (f == NULL)
			return -errno;
		pkey = PEM_read_PrivateKey(f, NULL, passcb, &err);
		fclose(f);

		if (!pkey) {
			/* key == NULL and err == 0 implies a bad password */
			if (err == 0)
				return -EPERM;
			/* Handle interesting errors or return -EIO */
			err = ERR_GET_REASON(ERR_peek_last_error());
			ERR_clear_error();

			/* PEM_R_BAD_DECRYPT indicates a bad password */
			if (err == PEM_R_BAD_DECRYPT)
				return -EPERM;
			if (err == PEM_R_BAD_PASSWORD_READ)
				return -ECANCELED;
			return -EIO;
		}
	}
	if (certfile) {
		if ((f = fopen(certfile, "r")) == NULL) {
			err = -errno;
			if (pkey)
				EVP_PKEY_free(pkey);
			return err;
		}
		cert = PEM_read_X509(f, NULL, NULL, NULL);
		fclose(f);
		if (cert == NULL) {
			if (pkey)
				EVP_PKEY_free(pkey);
			return -EINVAL;
		}
	}

	as = calloc(1, sizeof(struct anoubis_sig));
	if (as == NULL) {
		if (pkey)
			EVP_PKEY_free(pkey);
		if (cert)
			X509_free(as->cert);
		return -ENOMEM;
	}
	as->type = ANOUBIS_SIG_HASH_DEFAULT;
	as->pkey = pkey;
	as->cert = cert;
	as->keyid = NULL;
	if (cert) {
		err = cert_keyid(&as->keyid, cert);
		if (err < 0) {
			anoubis_sig_free(as);
			return err;
		}
		as->idlen = err;
	} else {
		as->idlen = 0;
	}
	err = anoubis_sig_check_consistency(as);
	if (err < 0) {
		anoubis_sig_free(as);
		return err;
	}
	(*asp) = as;
	return 0;
}

void
anoubis_sig_free(struct anoubis_sig *as)
{
	if (as == NULL)
		return;

	if (as->pkey)
		EVP_PKEY_free(as->pkey);
	if (as->keyid)
		free(as->keyid);
	if (as->cert)
		X509_free(as->cert);

	EVP_cleanup();
	free(as);
}

char *
anoubis_sig_key2char(int idlen, const unsigned char *keyid)
{
	int i;
	char *res = NULL;

	if ((res = calloc((2*idlen) + 1 , sizeof(char))) == NULL)
		return NULL;

	for (i = 0; i < idlen; i++)
		sprintf(&res[2*i], "%2.2x", keyid[i]);
	res[2*i] = '\0';
	return res;
}

/*
 * NOTE: NEVER write to stdout in this function. Try to use the tty instead.
 */
int
pass_cb(char *buf, int size, int rwflag __used, void *err __used)
{
	int len;
	char *tmp;

	tmp = getpass("Enter passphrase for Private Key: ");
	len = strlen(tmp);
	if (len <= 0)
		return 0;

	if (len > size) len = size;

	memcpy(buf, tmp, len);
	bzero(tmp, len);

	return len;
}

static int
cert_keyid(unsigned char **keyid, X509 *cert)
{
	X509_EXTENSION *ext = NULL;
	ASN1_OCTET_STRING *o_asn = NULL;
	int rc = 0;
	int rv = -1, len = -1;

	if (keyid == NULL || cert == NULL)
		return -EINVAL;

	for (rv = X509_get_ext_by_NID(cert, NID_subject_key_identifier, rv);
	    rv >= 0;
	    rv = X509_get_ext_by_NID(cert, NID_subject_key_identifier, rv))
		ext = X509_get_ext(cert, rv);

	o_asn = X509_EXTENSION_get_data(ext);
	if (!o_asn) {
		rc = ERR_get_error();
		return -rc;
	}
	/* The result of X509_EXTENSION_get_data beginns with
	 * a number which describes algorithm (see RFC 3280)
	 * We just need the KeyID, for that we cut away this number
	 */
	if (!memcmp(o_asn->data, "\004\024", 2)) {
		len = ASN1_STRING_length(o_asn) - 2;
		if ((*keyid = calloc(len, sizeof(unsigned char))) == NULL)
			return -ENOMEM;
		memcpy(*keyid, &o_asn->data[2], len);
	} else {
		len = ASN1_STRING_length(o_asn);
		if ((*keyid = calloc(len, sizeof(unsigned char))) == NULL)
			return -ENOMEM;
		memcpy(*keyid, &o_asn->data, len);
	}

	return (len);
}

char *
anoubis_sig_cert_name(X509 *cert)
{
	BIO	*bio;
	char	*buf = NULL;
	int	num;

	if ((bio = BIO_new(BIO_s_mem())) == NULL)
		return (NULL);

	X509_NAME_print_ex(bio, X509_get_subject_name(cert), 0,
	    XN_FLAG_ONELINE & ~XN_FLAG_SPC_EQ & ~ASN1_STRFLGS_ESC_MSB);

	if ((buf = calloc(ANOUBIS_SIG_NAME_LEN + 1, sizeof(char))) == NULL) {
		BIO_free(bio);
		return (NULL);
	}

	if ((num = BIO_read(bio, buf, ANOUBIS_SIG_NAME_LEN)) < 0) {
		BIO_free(bio);
		free(buf);

		return (NULL);
	}

	buf[num] = '\0';

	BIO_free(bio);

	return (buf);
}

/* The idea of this code is taken from openssl ASN1_UTCTIME_print(). */
#define GETNUM(x) (((x)[0] - '0') * 10 + (x)[1] - '0')
int
anoubis_sig_cert_validity(X509 *cert, struct tm *time)
{
	ASN1_TIME	*notAfter = NULL;

	if ((cert == NULL) || (time == NULL)) {
		return (-EINVAL);
	}

	notAfter = X509_get_notAfter(cert);
	if (notAfter == NULL) {
		return (-EINVAL);
	} else if (notAfter->length < 10 || !ASN1_UTCTIME_check(notAfter)) {
		return (-ERANGE);
	}

	time->tm_year = GETNUM(notAfter->data);
	if (time->tm_year <= 50) {
		time->tm_year += 100;
	}

	/* Type and range checks already done by ASN1_UTCTIME_check(). */
	time->tm_mon  = GETNUM(notAfter->data + 2);
	time->tm_mday = GETNUM(notAfter->data + 4);
	time->tm_hour = GETNUM(notAfter->data + 6);
	time->tm_min  = GETNUM(notAfter->data + 8);
	time->tm_sec  = GETNUM(notAfter->data + 10);

	return (0);
}
#undef GETNUM

/**
 * Check the internal consistency of both keyA and keyB. Additionally,
 * if both signature structures contain are non NULL make sure that they
 * use the same private and public keys.
 *
 * @param keyA The first anoubis_sig structure.
 * @param keyB The second anoubis_sig structure.
 * @return Zero if the comparison succeeds, a negative error code if the
 *     comparison fails.
 */
int
anoubis_sig_keycmp(struct anoubis_sig *keyA, struct anoubis_sig *keyB)
{
	int		 rc;
	EVP_PKEY	*certpubA = NULL, *certpubB = NULL;

	if (keyA && (rc = anoubis_sig_check_consistency(keyA)) < 0)
		return rc;
	if (keyB && (rc = anoubis_sig_check_consistency(keyB)) < 0)
		return rc;
	if (!keyA || !keyB)
		return 0;

	rc = -EIO;
	if (keyA->cert) {
		certpubA = X509_get_pubkey(keyA->cert);
		if (!certpubA)
			goto out;
	}
	if (keyB->cert) {
		certpubB = X509_get_pubkey(keyB->cert);
		if (!certpubB)
			goto out;
	}
	rc = -EPERM;

#define DO_KEY_CMP(X,Y) do {					\
	if ((X) != NULL && (Y) != NULL)				\
		if (anoubis_sig_dorsacmp((X), (Y)) != 0)	\
			goto out;				\
	} while (0)

	DO_KEY_CMP(certpubA, certpubB);
	DO_KEY_CMP(certpubA, keyB->pkey);
	DO_KEY_CMP(keyA->pkey, certpubB);
	DO_KEY_CMP(keyA->pkey, keyB->pkey);
#undef DO_KEY_CMP

	rc = 0;
out:
	if (certpubA)
		EVP_PKEY_free(certpubA);
	if (certpubB)
		EVP_PKEY_free(certpubB);
	return rc;
}

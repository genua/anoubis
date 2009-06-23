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
			return (-4);
		}
	}
	close(fd);

	ret = anoubis_sig_verify_policy(filename, sigbuf, siglen, sigkey);

	free(sigbuf);

	return ret;
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
		*len = -EINVAL;
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

int
anoubis_verify_csum(struct anoubis_sig *as,
     unsigned char csum[ANOUBIS_SIG_HASH_SHA256_LEN], unsigned char *sfs_sign,
     int sfs_len)
{
	int result = 0;
	EVP_MD_CTX ctx;
	EVP_PKEY *pkey;

	if (as == NULL || csum == NULL || sfs_sign == NULL)
		return 0;

	pkey = as->pkey;

	EVP_MD_CTX_init(&ctx);
	if (EVP_VerifyInit(&ctx, as->type) == 0) {
		EVP_MD_CTX_cleanup(&ctx);
		return 0;
	}
	EVP_VerifyUpdate(&ctx, csum, ANOUBIS_SIG_HASH_SHA256_LEN);
	if((result = EVP_VerifyFinal(&ctx, sfs_sign, sfs_len, pkey)) == -1)
		result = 0;

	EVP_MD_CTX_cleanup(&ctx);

	return result;
}

/* This function will be used inside of the daemon */
int
anoubisd_verify_csum(EVP_PKEY *pkey,
     unsigned char csum[ANOUBIS_SIG_HASH_SHA256_LEN], unsigned char *sfs_sign,
     int sfs_len)
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

struct anoubis_sig *
anoubis_sig_pub_init(const char *keyfile, const char *certfile,
     pem_password_cb *passcb, int *error)
{
	return anoubis_sig_init(keyfile, certfile, passcb,
	    ANOUBIS_SIG_HASH_DEFAULT, ANOUBIS_SIG_PUB, error);
}

struct anoubis_sig *
anoubis_sig_priv_init(const char *keyfile, const char *certfile,
    pem_password_cb *passcb, int *error)
{
	return anoubis_sig_init(keyfile, certfile, passcb,
	    ANOUBIS_SIG_HASH_DEFAULT, ANOUBIS_SIG_PRIV, error);
}

struct anoubis_sig *
anoubis_sig_init(const char *keyfile, const char *certfile,
    pem_password_cb *passcb, const EVP_MD *type, int pub_priv, int *error)
{

	struct anoubis_sig *as = NULL;
	EVP_PKEY *pkey = NULL;
	FILE *f = NULL;
	X509	*cert = NULL;

	OpenSSL_add_all_algorithms();

	if (!error)
		return NULL;
	if (!certfile && !keyfile) {
		*error = EINVAL;
		return NULL;
	}
	if (keyfile) {
		f = fopen(keyfile, "r");
		if (f == NULL) {
			*error = errno;
			return NULL;
		}

		switch (pub_priv) {
		case ANOUBIS_SIG_PUB:
			pkey = PEM_read_PUBKEY(f, NULL, passcb, "Public Key");
			break;
		case ANOUBIS_SIG_PRIV:
			pkey = PEM_read_PrivateKey(f, NULL, passcb,
			    "Private Key");
			break;
		default:
			fclose(f);
			*error = EINVAL;
			return NULL;
		}
		fclose(f);

		if (!pkey) {
			/* ERR_get_error gives us a certian amount of
			 * errors that occurs but we want just to know
			 * if the password ist wrong. We take the last
			 * error code. */
			*error = ERR_GET_REASON(ERR_peek_last_error());

			/* _BAD_DECRYPT just means that the password
			 * didn't encrypt the key -> wrong password.
			 */
			if (*error == PEM_R_BAD_DECRYPT)
				*error = EPERM;
			/*
			 * _BAD_PASSWORD_READ for a empty passphrase
			 * entered
			 */
			else if (*error == PEM_R_BAD_PASSWORD_READ)
				*error = EPERM;

			ERR_clear_error();
			return NULL;
		}
	}

	if (certfile) {
		if ((f = fopen(certfile, "r")) == NULL) {
			if (pkey) {
				EVP_PKEY_free(pkey);
			}
			*error = errno;
			return NULL;
		}
		if ((cert = PEM_read_X509(f, NULL, NULL, NULL)) == NULL) {
			if (pkey)
				EVP_PKEY_free(pkey);
			fclose(f);
			*error = EINVAL;
			return NULL;
		}
		fclose(f);
	}

	if ((as = calloc(1, sizeof(struct anoubis_sig))) == NULL) {
		if (pkey)
			EVP_PKEY_free(pkey);
		if (cert)
			X509_free(as->cert);

		*error = ENOMEM;
		return NULL;
	}
	as->type = type;
	as->pkey = pkey;
	as->cert = cert;
	if (cert)
		as->idlen = cert_keyid(&as->keyid, cert);
	else
		as->idlen = 0;
	if (as->idlen < 0) {
		anoubis_sig_free(as);
		*error = -as->idlen;
		return NULL;
	}
	return as;
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
anoubis_sig_key2char(int idlen, unsigned char *keyid)
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

int
pass_cb(char *buf, int size, int rwflag __used, void *text)
{
	int len;
	char *tmp;

	printf("Enter pass phrase for \"%s\"\n", (char *)text);

	tmp = getpass("Password\n");
	len = strlen(tmp);

	if (len <= 0) return 0;

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
	char *buf = NULL;

	if ((buf = calloc(ANOUBIS_SIG_NAME_LEN, sizeof(char))) == NULL) {
		return NULL;
	}

	X509_NAME_oneline(X509_get_subject_name(cert), buf,
	    ANOUBIS_SIG_NAME_LEN);

	return (buf);
}

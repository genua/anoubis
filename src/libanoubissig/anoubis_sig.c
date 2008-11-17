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

int pass_cb(char *buf, int size, int rwflag, void *u);

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
	memcpy(&result[ANOUBIS_SIG_HASH_SHA256_LEN], sigbuf, siglen);
	free(sigbuf);
	*len = siglen + ANOUBIS_SIG_HASH_SHA256_LEN;
	return result;
}

int
anoubis_verify_csum(struct anoubis_sig *as,
     unsigned char csum[ANOUBIS_SIG_HASH_SHA256_LEN], unsigned char *sfs_sign,
     int sfs_len)
{
	int result = 0;
	EVP_MD_CTX ctx;
	EVP_PKEY *pkey;

	if (as == NULL || csum == NULL)
		return -1;

	pkey = as->pkey;

	EVP_MD_CTX_init(&ctx);
	if (EVP_VerifyInit(&ctx, as->type) == 0) {
		EVP_MD_CTX_cleanup(&ctx);
		return -1;
	}
	EVP_VerifyUpdate(&ctx, csum, ANOUBIS_SIG_HASH_SHA256_LEN);
	result = EVP_VerifyFinal(&ctx, sfs_sign, sfs_len, pkey);

	EVP_MD_CTX_cleanup(&ctx);

	return result;
}

struct anoubis_sig *
anoubis_sig_pub_init(const char *file, const EVP_MD *type, char *pass,
    int need_pass)
{
	return anoubis_sig_init(file, pass, type, ANOUBIS_SIG_PUB, need_pass);
}

struct anoubis_sig *
anoubis_sig_priv_init(const char *file, const EVP_MD *type, char *pass,
    int need_pass)
{
	return anoubis_sig_init(file, pass, type, ANOUBIS_SIG_PRIV, need_pass);
}

struct anoubis_sig *
anoubis_sig_init(const char *file, char *pass, const EVP_MD *type, int pub_priv,
    int need_pass)
{

	struct anoubis_sig *as = NULL;
	EVP_PKEY *pkey = NULL;
	FILE *f = NULL;

	if (!file)
		return NULL;

	f = fopen(file, "rb");
	if (f == NULL) {
		perror(file);
		return NULL;
	}

	switch (pub_priv) {
	case ANOUBIS_SIG_PUB:
		if(need_pass) {
			if (pass)
				pkey = PEM_read_PUBKEY(f, NULL, NULL, pass);
			else
				pkey = PEM_read_PUBKEY(f, NULL, pass_cb,
				    "Public Key");
		} else
			pkey = PEM_read_PUBKEY(f, NULL, NULL, NULL);

		break;
	case ANOUBIS_SIG_PRIV:
		if (need_pass) {
			if (pass)
				pkey = PEM_read_PrivateKey(f, NULL, NULL, pass);
			else
				pkey = PEM_read_PrivateKey(f, NULL, pass_cb,
				    "Private Key");
		} else
			pkey = PEM_read_PrivateKey(f, NULL, NULL, NULL);
		break;
	default:
		fclose(f);
		return NULL;
	}
	fclose(f);

	if (!pkey)
		return NULL;

	if ((as = malloc(sizeof(struct anoubis_sig))) == NULL) {
		EVP_PKEY_free(pkey);
		return NULL;
	}
	as->type = type;
	as->pkey = pkey;

	return as;
}

void
anoubis_sig_free(struct anoubis_sig *as)
{
	if (as == NULL)
		return;

	if (as->pkey)
		EVP_PKEY_free(as->pkey);

	free(as);
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

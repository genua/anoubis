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

#include <config.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <openssl/rand.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_sig.h>
#include <stdlib.h>

#include "anoubis_auth.h"

#define ANOUBIS_IV_RANDOM_BYTES	128
#define ANOUBIS_IV_PREFIX		"Anoubis UI Auth package"

#define AUTH_CALLBACK_CLEANUP \
	do { \
		if (ivBuf != NULL) { \
			free(ivBuf); \
		} \
		if (sigBuf != NULL) { \
			free(sigBuf); \
		} \
	} while (0)

int anoubis_auth_callback(struct anoubis_sig *privKey,
    struct anoubis_sig *pubKey, struct anoubis_msg *in,
    struct anoubis_msg **outp, int flags)
{
	int	 rc		= 0;	/* different return codes */
	void	*sigBuf		= NULL;	/* signature buffer */
	int	 sigLen		= 0;	/* signature buffer length */
	void	*kidBuf		= NULL;	/* key id buffer */
	int	 kidLen		= 0;	/* key id buffer length */
	void	*chlBuf		= NULL;	/* server challange buffer */
	int	 chlLen		= 0;	/* server challange buffer length */
	char	*ivBuf		= NULL;	/* initialize vector buffer */
	int	 ivLen		= 0;	/* initialize vector buffer length */

	struct anoubis_msg	*out = NULL; /* Assembled output msg */

	/* Check arguments. */
	if (privKey == NULL || pubKey == NULL || in == NULL || outp == NULL)
		return -ANOUBIS_AUTHERR_INVAL;
	(*outp) = NULL;

	if (!VERIFY_LENGTH(in, sizeof(Anoubis_AuthTransportMessage)))
		return -ANOUBIS_AUTHERR_PKG;

	rc = get_value(in->u.authtransport->auth_type);
	if (rc != ANOUBIS_AUTH_CHALLENGE)
		return -ANOUBIS_AUTHERR_PKG;

	if (!VERIFY_FIELD(in, authchallenge, payload))
		return -ANOUBIS_AUTHERR_PKG;

	/* Find server challange and keyid within input msg. */
	chlLen = get_value(in->u.authchallenge->challengelen);
	kidLen = get_value(in->u.authchallenge->idlen);
	if (!VERIFY_BUFFER(in, authchallenge, payload, 0, chlLen)
	    || !VERIFY_BUFFER(in, authchallenge, payload, chlLen, kidLen)) {
		return -ANOUBIS_AUTHERR_PKG;
	}
	chlBuf =  in->u.authchallenge->payload;
	kidBuf = &in->u.authchallenge->payload[chlLen];

	/* Check key from server with local key. */
	if (anoubis_sig_keycmp(privKey, pubKey) != 0)
		return -ANOUBIS_AUTHERR_KEY;

	if ((flags & ANOUBIS_AUTHFLAG_IGN_KEY_MISMATCH) == 0) {
		if ((pubKey->idlen != kidLen) || (kidLen == 0))
			return -ANOUBIS_AUTHERR_CERT;

		if ((rc = memcmp(pubKey->keyid, kidBuf, kidLen)) != 0)
			return -ANOUBIS_AUTHERR_KEY_MISMATCH;
	}

	/*
	 * To avoid attacs from server, we'll not just sign the server
	 * challenge. We create a initialize vector with a prefix and
	 * random data. Then we sign server challange and initialize vector.
	 * The signature and initialize vector are send back to the server.
	 */
	ivLen = strlen(ANOUBIS_IV_PREFIX);
	ivBuf = calloc(ivLen + ANOUBIS_IV_RANDOM_BYTES + chlLen, 1);
	if (ivBuf == NULL) {
		AUTH_CALLBACK_CLEANUP;
		return -ANOUBIS_AUTHERR_NOMEM;
	}
	if (!RAND_status()) {
		AUTH_CALLBACK_CLEANUP;
		return -ANOUBIS_AUTHERR_RAND;
	}
	/* Fill initialize vector with prefix and random data. */
	strcpy(ivBuf, ANOUBIS_IV_PREFIX);
	RAND_pseudo_bytes((unsigned char *)ivBuf + ivLen,
	    ANOUBIS_IV_RANDOM_BYTES);
	ivLen += ANOUBIS_IV_RANDOM_BYTES;
	/*
	 * Add server challange to end of initialize vector.
	 * But don't update ivLen, 'cause only iv is send back
	 * _without_ server challenge.
	 */
	memcpy(ivBuf + ivLen, chlBuf, chlLen);

	/* Sign iv _and_ server challange. */
	rc = anoubis_sig_sign_buffer(ivBuf, ivLen + chlLen, &sigBuf, &sigLen,
	    privKey);
	if (rc < 0) {
		AUTH_CALLBACK_CLEANUP;
		return -ANOUBIS_AUTHERR_SIGN;
	}

	/* Assemble output message. */
	out = anoubis_msg_new(sizeof(Anoubis_AuthChallengeReplyMessage)
	    + ivLen + sigLen);
	if (out == NULL) {
		AUTH_CALLBACK_CLEANUP;
		return -ANOUBIS_AUTHERR_NOMEM;
	}
	set_value(out->u.authchallengereply->type, ANOUBIS_C_AUTHDATA);
	set_value(out->u.authchallengereply->auth_type,
	    ANOUBIS_AUTH_CHALLENGEREPLY);
	set_value(out->u.authchallengereply->uid, geteuid());
	set_value(out->u.authchallengereply->ivlen, ivLen);
	set_value(out->u.authchallengereply->siglen, sigLen);
	memcpy(out->u.authchallengereply->payload, ivBuf, ivLen);
	memcpy(out->u.authchallengereply->payload + ivLen, sigBuf, sigLen);
	(*outp) = out;

	AUTH_CALLBACK_CLEANUP;
	return 0;
}
#undef AUTH_CALLBACK_CLEANUP

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef GCOV
#include <anoubis_chat.h>
#include <anoubis_msg.h>
#endif

#include <anoubischeck.h>
#include <anoubis_auth.h>
#include <anoubis_errno.h>


#define TEST_CHALLENGE	"bvcdertyuilkmnjbvf"
#define TEST_KEY_FILE	"test.key"
#define TEST_CRT_FILE	"test.crt"

static struct anoubis_sig *asPrivKey	= NULL;
static struct anoubis_msg *inputMsg	= NULL;
static struct anoubis_msg *outputMsg	= NULL;

static void
setup(void)
{
	int	 error		= 0;
	int	 challengeLen	= 0;
	void	*challengeBuf	= NULL;
	char *keyfile = TEST_KEY_FILE;
	char *crtfile = TEST_CRT_FILE;

	if (getenv("KEYDIR")) {
		fail_unless(asprintf(&keyfile, "%s/%s", getenv("KEYDIR"), keyfile) > 0);
		fail_unless(asprintf(&crtfile, "%s/%s", getenv("KEYDIR"), crtfile) > 0);
	}

	/* Create / read local key. */
	error = anoubis_sig_create(&asPrivKey, keyfile, crtfile, NULL);
	fail_if(asPrivKey == NULL || error != 0, "Setup error: can't load "
	    "private key %s / %s: %s", keyfile, crtfile,
	    anoubis_strerror(-error));

	/* Create AuthChallange message as received from server. */
	challengeLen = strlen(TEST_CHALLENGE);
	challengeBuf = malloc(challengeLen + asPrivKey->idlen);
	fail_if(challengeBuf == NULL, "Setup error: NOMEM for challengeBuf");
	memcpy(challengeBuf, TEST_CHALLENGE, challengeLen);
	memcpy(challengeBuf + challengeLen, asPrivKey->keyid,
	    asPrivKey->idlen);
	challengeLen += asPrivKey->idlen;

	inputMsg = anoubis_msg_new(sizeof(Anoubis_AuthChallengeMessage)
	    + challengeLen);
	fail_if(inputMsg == NULL, "Setup error: can't create inputMsg");

	set_value(inputMsg->u.authchallenge->type, ANOUBIS_C_AUTHDATA);
	set_value(inputMsg->u.authchallenge->auth_type,
	    ANOUBIS_AUTH_CHALLENGE);
	set_value(inputMsg->u.authchallenge->challengelen,
	    strlen(TEST_CHALLENGE));
	set_value(inputMsg->u.authchallenge->idlen, asPrivKey->idlen);
	memcpy(inputMsg->u.authchallenge->payload, challengeBuf, challengeLen);
	free(challengeBuf);

	fail_if(outputMsg != NULL, "Setup error: outputMsg not clean");
}

static void
teardown(void)
{
	if (asPrivKey != NULL) {
		anoubis_sig_free(asPrivKey);
		asPrivKey = NULL;
	}

	if (inputMsg != NULL) {
		anoubis_msg_free(inputMsg);
		inputMsg = NULL;
	}

	if (outputMsg != NULL) {
		anoubis_msg_free(outputMsg);
		outputMsg = NULL;
	}
}


/*
 * Unit tests
 */
START_TEST(auth_inval_priv)
{
	int	rc	= -1;
	int	expect	= -ANOUBIS_AUTHERR_INVAL;

	rc = anoubis_auth_callback(NULL, asPrivKey, inputMsg, &outputMsg, 0);
	fail_if(rc != expect, "Failed to detect invalid anoubis_sig /w "
	    "rc 0x%x expect 0x%x", rc, expect);
}
END_TEST

START_TEST(auth_inval_pub)
{
	int	rc	= -1;
	int	expect	= -ANOUBIS_AUTHERR_INVAL;

	rc = anoubis_auth_callback(asPrivKey, NULL, inputMsg, &outputMsg, 0);
	fail_if(rc != expect, "Failed to detect invalid anoubis_sig /w "
	    "rc 0x%x expect 0x%x", rc, expect);
}
END_TEST

START_TEST(auth_inval_in)
{
	int	rc	= -1;
	int	expect	= -ANOUBIS_AUTHERR_INVAL;

	rc = anoubis_auth_callback(asPrivKey, asPrivKey, NULL, &outputMsg, 0);
	fail_if(rc != expect, "Failed to detect invalid input message /w "
	    "rc 0x%x expect 0x%x", rc, expect);
}
END_TEST

START_TEST(auth_inval_out)
{
	int	rc	= -1;
	int	expect	= -ANOUBIS_AUTHERR_INVAL;

	rc = anoubis_auth_callback(asPrivKey, asPrivKey, inputMsg, NULL, 0);
	fail_if(rc != expect, "Failed to detect invalid output message /w "
	    "rc 0x%x expect 0x%x", rc, expect);
}
END_TEST

START_TEST(auth_pkg_type)
{
	int	rc	= -1;
	int	expect	= -ANOUBIS_AUTHERR_PKG;

	set_value(inputMsg->u.authtransport->auth_type,
	    ANOUBIS_AUTH_CHALLENGE + 3);
	rc = anoubis_auth_callback(asPrivKey, asPrivKey, inputMsg,
	    &outputMsg, 0);
	fail_if(rc != expect, "Failed to detect inval pkg type /w "
	    "rc 0x%x expect 0x%x", rc, expect);
}
END_TEST

START_TEST(auth_pkg_len_message)
{
	int	rc	= -1;
	int	expect	= -ANOUBIS_AUTHERR_PKG;

	inputMsg->length -= 3;
	rc = anoubis_auth_callback(asPrivKey, asPrivKey, inputMsg,
	    &outputMsg, 0);
	fail_if(rc != expect, "Failed to detect inval message length /w "
	    "rc 0x%x expect 0x%x", rc, expect);
}
END_TEST

START_TEST(auth_pkg_len_challenge)
{
	int	rc	= -1;
	int	expect	= -ANOUBIS_AUTHERR_PKG;

	set_value(inputMsg->u.authchallenge->challengelen,
	    get_value(inputMsg->u.authchallenge->challengelen) + 3);
	rc = anoubis_auth_callback(asPrivKey, asPrivKey, inputMsg,
	    &outputMsg, 0);
	fail_if(rc != expect, "Failed to detect inval challange length /w "
	    "rc 0x%x expect 0x%x", rc, expect);
}
END_TEST

START_TEST(auth_pkg_len_keyid)
{
	int	rc	= -1;
	int	expect	= -ANOUBIS_AUTHERR_PKG;

	set_value(inputMsg->u.authchallenge->idlen,
	    get_value(inputMsg->u.authchallenge->idlen) + 3);
	rc = anoubis_auth_callback(asPrivKey, asPrivKey, inputMsg,
	    &outputMsg, 0);
	fail_if(rc != expect, "Failed to detect inval keyid length /w "
	    "rc 0x%x expect 0x%x", rc, expect);
}
END_TEST

START_TEST(auth_key_len)
{
	int	rc	= -1;
	int	expect	= -ANOUBIS_AUTHERR_CERT;

	asPrivKey->idlen -= 3;
	rc = anoubis_auth_callback(asPrivKey, asPrivKey, inputMsg,
	    &outputMsg, 0);
	fail_if(rc != expect, "Failed to detect inval key length /w "
	    "rc 0x%x expect 0x%x", rc, expect);
}
END_TEST

START_TEST(auth_key_local)
{
	int		 rc	= -1;
	int		 expect	= -ANOUBIS_AUTHERR_KEY;
	EVP_PKEY	*priv = NULL;

	priv = asPrivKey->pkey;
	asPrivKey->pkey = NULL;
	rc = anoubis_auth_callback(asPrivKey, asPrivKey, inputMsg,
	    &outputMsg, 0);
	asPrivKey->pkey = priv;
	fail_if(rc != expect, "Failed to detect corrupted key /w "
	    "rc 0x%x expect 0x%x", rc, expect);
}
END_TEST

START_TEST(auth_key_mismatch)
{
	int	rc	= -1;
	int	expect	= -ANOUBIS_AUTHERR_KEY_MISMATCH;

	asPrivKey->keyid[3] = ~asPrivKey->keyid[3];
	rc = anoubis_auth_callback(asPrivKey, asPrivKey, inputMsg,
	    &outputMsg, 0);
	fail_if(rc != expect, "Failed to detect mismatch key /w "
	    "rc 0x%x expect 0x%x", rc, expect);

	rc = anoubis_auth_callback(asPrivKey, asPrivKey, inputMsg,
	    &outputMsg, ANOUBIS_AUTHFLAG_IGN_KEY_MISMATCH);
	fail_if(rc == expect, "Failed to ignore mismatch key /w "
	    "rc 0x%x expect 0x%x", -rc, -expect);
}
END_TEST

START_TEST(auth_ok)
{
	int		 rc		= -1;
	int		 expect		=  0;
	void		*dataBuf	= NULL;
	int		 dataLen	= 0;
	void		*sigBuf		= NULL;
	int		 sigLen		= 0;
	void		*chlBuf		= NULL;
	int		 chlLen		= 0;
	EVP_PKEY	*pubKey		= NULL;

	rc = anoubis_auth_callback(asPrivKey, asPrivKey, inputMsg,
	    &outputMsg, 0);
	fail_if(rc != expect, "Unsuccessfull anoubis_auth_callback /w "
	    "rc 0x%x expect 0x%x", rc, expect);

	/* Verify challenge reply. */
	pubKey = X509_get_pubkey(asPrivKey->cert);
	fail_if(pubKey == NULL, "Can't load public key from cert.");

	dataLen = get_value(outputMsg->u.authchallengereply->ivlen);
	rc = VERIFY_BUFFER(outputMsg, authchallengereply, payload, 0, dataLen);
	fail_if(!rc, "Error in challenge reply: data length");

	sigLen  = get_value(outputMsg->u.authchallengereply->siglen);
	rc = VERIFY_BUFFER(outputMsg, authchallengereply, payload, dataLen,
	    sigLen);
	fail_if(!rc, "Error in challenge reply: signature length");

	dataBuf =  outputMsg->u.authchallengereply->payload;
	sigBuf  = &outputMsg->u.authchallengereply->payload[dataLen];

	/* Assemble challenge */
	chlLen = strlen(TEST_CHALLENGE);
	chlBuf = malloc(chlLen + dataLen);
	memcpy(chlBuf, dataBuf, dataLen);
	memcpy(chlBuf + dataLen, TEST_CHALLENGE, chlLen);
	chlLen += dataLen;
	rc = anoubis_sig_verify_buffer(chlBuf, chlLen, sigBuf, sigLen,
	    pubKey);
	fail_if(rc != 0, "Error in signature verification /w rc %d expect 0",
	    rc);

	EVP_PKEY_free(pubKey);
	free(chlBuf);
}
END_TEST


TCase *
auth_tcase(void)
{
	TCase *tc = tcase_create("AuthTcase");

	tcase_add_checked_fixture(tc, setup, teardown);

	tcase_add_test(tc, auth_inval_priv);
	tcase_add_test(tc, auth_inval_pub);
	tcase_add_test(tc, auth_inval_in);
	tcase_add_test(tc, auth_inval_out);
	tcase_add_test(tc, auth_pkg_type);
	tcase_add_test(tc, auth_pkg_len_message);
	tcase_add_test(tc, auth_pkg_len_challenge);
	tcase_add_test(tc, auth_pkg_len_keyid);
	tcase_add_test(tc, auth_key_len);
	tcase_add_test(tc, auth_key_local);
	tcase_add_test(tc, auth_key_mismatch);

	tcase_add_test(tc, auth_ok);

	return (tc);
}

static Suite*
auth_testsuite(void)
{
	Suite *s = suite_create("AuthSuite");

	suite_add_tcase(s, auth_tcase());

	return (s);
}

int
main (void)
{
	int result;

	SRunner *suiterunner = srunner_create(auth_testsuite());

	srunner_run_all(suiterunner, CK_NORMAL);
	result = check_eval_srunner(suiterunner);
	srunner_free(suiterunner);

	return (result);
}

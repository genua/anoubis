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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <check.h>
#include <errno.h>
#include <string.h>

#include "anoubischat.h"
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

START_TEST(tc_set_tail)
{
	struct achat_channel	*c  = NULL;
	achat_rc		rc;

	c = acc_create();
	fail_if(c == NULL, "Couldn't create channel");
	mark_point();

	rc = acc_settail(c, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "Couldn't set tail");
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc == ACHAT_RC_OK,
		"Preparation successful but configuration not completed");
	mark_point();

	acc_destroy(c);
}
END_TEST

START_TEST(tc_set_sslmode)
{
	struct achat_channel	*c  = NULL;
	achat_rc		rc;

	c = acc_create();
	fail_if(c == NULL, "Couldn't create channel");
	mark_point();

	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "Couldn't set sslmode");
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc == ACHAT_RC_OK,
		"Preparation successful but configuration not completed");
	mark_point();

	acc_destroy(c);
}
END_TEST

START_TEST(tc_set_addr)
{
	struct achat_channel	*c  = NULL;
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_in	in;
	} sa;
	achat_rc		rc;

	c = acc_create();
	fail_if(c == NULL, "Couldn't create channel");
	mark_point();

	memset(&sa.ss, 0, sizeof(sa.ss));
	sa.in.sin_family = AF_INET;
	sa.in.sin_port = htons(ACHAT_SERVER_PORT);
	inet_aton("127.0.0.1", &(sa.in.sin_addr));
#ifdef OPENBSD
	sa.in.sin_len = sizeof(struct sockaddr_in);
#endif

	rc = acc_setaddr(c, &sa.ss, sizeof(struct sockaddr_in));
	fail_if(rc != ACHAT_RC_OK, "Couldn't set addr");
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc == ACHAT_RC_OK,
		"Preparation successful but configuration not completed");
	mark_point();

	acc_destroy(c);
}
END_TEST

START_TEST(tc_set_tail_sslmode)
{
	struct achat_channel	*c  = NULL;
	achat_rc		rc;

	c = acc_create();
	fail_if(c == NULL, "Couldn't create channel");
	mark_point();

	rc = acc_settail(c, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "Couldn't set tail");
	mark_point();

	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "Couldn't set sslmode");
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc == ACHAT_RC_OK,
		"Preparation successful but configuration not completed");
	mark_point();

	acc_destroy(c);
}
END_TEST

START_TEST(tc_set_tail_addr)
{
	struct achat_channel	*c  = NULL;
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_in	in;
	} sa;
	achat_rc		rc;

	c = acc_create();
	fail_if(c == NULL, "Couldn't create channel");
	mark_point();

	rc = acc_settail(c, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "Couldn't set tail");
	mark_point();

	memset(&sa.ss, 0, sizeof(sa.ss));
	sa.in.sin_family = AF_INET;
	sa.in.sin_port = htons(ACHAT_SERVER_PORT);
	inet_aton("127.0.0.1", &(sa.in.sin_addr));
#ifdef OPENBSD
	sa.in.sin_len = sizeof(struct sockaddr_in);
#endif

	rc = acc_setaddr(c, &sa.ss, sizeof(struct sockaddr_in));
	fail_if(rc != ACHAT_RC_OK, "Couldn't set addr");
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc == ACHAT_RC_OK,
		"Preparation successful but configuration not completed");
	mark_point();

	acc_destroy(c);
}
END_TEST

START_TEST(tc_set_sslmode_addr)
{
	struct achat_channel	*c  = NULL;
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_in	in;
	} sa;
	achat_rc		rc;

	c = acc_create();
	fail_if(c == NULL, "Couldn't create channel");
	mark_point();

	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "Couldn't set sslmode");
	mark_point();

	memset(&sa.ss, 0, sizeof(sa.ss));
	sa.in.sin_family = AF_INET;
	sa.in.sin_port = htons(ACHAT_SERVER_PORT);
	inet_aton("127.0.0.1", &(sa.in.sin_addr));
#ifdef OPENBSD
	sa.in.sin_len = sizeof(struct sockaddr_in);
#endif

	rc = acc_setaddr(c, &sa.ss, sizeof(struct sockaddr_in));
	fail_if(rc != ACHAT_RC_OK, "Couldn't set addr");
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc == ACHAT_RC_OK,
		"Preparation successful but configuration not completed");
	mark_point();

	acc_destroy(c);
}
END_TEST

START_TEST(tc_set_tail_sslmode_addr)
{
	struct achat_channel	*c  = NULL;
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_in	in;
	} sa;
	achat_rc		rc;

	c = acc_create();
	fail_if(c == NULL, "Couldn't create channel");
	mark_point();

	rc = acc_settail(c, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "Couldn't set tail");
	mark_point();

	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "Couldn't set sslmode");
	mark_point();

	memset(&sa.ss, 0, sizeof(sa.ss));
	sa.in.sin_family = AF_INET;
	sa.in.sin_port = htons(ACHAT_SERVER_PORT);
	inet_aton("127.0.0.1", &(sa.in.sin_addr));
#ifdef OPENBSD
	sa.in.sin_len = sizeof(struct sockaddr_in);
#endif

	rc = acc_setaddr(c, &sa.ss, sizeof(struct sockaddr_in));
	fail_if(rc != ACHAT_RC_OK, "Couldn't set addr");
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc != ACHAT_RC_OK, "client prepare failed with rc=%d [%s]",
	    rc, strerror(errno));
	mark_point();

	acc_destroy(c);
}
END_TEST

TCase *
libanoubischat_testcase_prepare(void)
{
	TCase *tc_prepare = tcase_create("prepare");

	tcase_add_test(tc_prepare, tc_set_tail);
	tcase_add_test(tc_prepare, tc_set_sslmode);
	tcase_add_test(tc_prepare, tc_set_addr);
	tcase_add_test(tc_prepare, tc_set_tail_sslmode);
	tcase_add_test(tc_prepare, tc_set_tail_addr);
	tcase_add_test(tc_prepare, tc_set_sslmode_addr);
	tcase_add_test(tc_prepare, tc_set_tail_sslmode_addr);

	return (tc_prepare);
}

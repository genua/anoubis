/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <check.h>
#include <string.h>

#include "anoubischat.h"
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

START_TEST(tc_core_creation)
{
	struct achat_channel	*c  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;

	c = acc_create();
	fail_if(c == NULL, "pointer empty");
	fail_if(c->fd != -1, "not correctly initialized @sockfd");
	fail_if(c->addrsize != 0, "not correctly initialized @addrsize");
	fail_if(c->euid != -1, "not correctly initialized @euid");
	fail_if(c->egid != -1, "not correctly initialized @egid");
	fail_if(c->tail != ACC_TAIL_NONE, "not correctly initialized @tail");
	fail_if(c->sslmode != ACC_SSLMODE_NONE,
		"not correctly initialized @sslmode");
	fail_if(c->blocking != ACC_BLOCKING,
		"not correctly initialized @blocking");
	mark_point();

	rc = acc_destroy(NULL);
	fail_if(rc != ACHAT_RC_INVALPARAM,
	    "destroy expected to fail with rc=%d but returned rc=%d",
	    ACHAT_RC_INVALPARAM, rc);
	mark_point();

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "destroy failed with rc=%d", rc);
}
END_TEST

START_TEST(tc_core_tail)
{
	struct achat_channel    *c  = NULL;
	achat_rc                 rc = ACHAT_RC_ERROR;

	c = acc_create();
	fail_if(c == NULL, "couldn't create channel");
	mark_point();

	rc = acc_settail(NULL, ACC_TAIL_NONE);
	fail_if(rc != ACHAT_RC_INVALPARAM,
	    "settail expected to fail with rc=%d but returned rc=%d",
	    ACHAT_RC_INVALPARAM, rc);
	mark_point();

	rc = acc_settail(c, 1024);
	fail_if(rc != ACHAT_RC_INVALPARAM,
	    "settail expected to fail with rc=%d but returned rc=%d",
	    ACHAT_RC_INVALPARAM, rc);
	mark_point();

	rc = acc_settail(c, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "settail failed with rc=%d", rc);
	fail_if(c->tail != ACC_TAIL_SERVER, "tail not set correctly");
	mark_point();

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "destroy failed with rc=%d", rc);
}
END_TEST

START_TEST(tc_core_sslmode)
{
	struct achat_channel    *c  = NULL;
	achat_rc                 rc = ACHAT_RC_ERROR;

	c = acc_create();
	fail_if(c == NULL, "couldn't create channel");
	mark_point();

	rc = acc_setsslmode(NULL, ACC_SSLMODE_NONE);
	fail_if(rc != ACHAT_RC_INVALPARAM,
	    "setsslmode expected to fail with rc=%d but returned rc=%d",
	    ACHAT_RC_INVALPARAM, rc);
	mark_point();

	rc = acc_setsslmode(c, 1024);
	fail_if(rc != ACHAT_RC_INVALPARAM,
	    "setsslmode expected to fail with rc=%d but returned rc=%d",
	    ACHAT_RC_INVALPARAM, rc);
	mark_point();

	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "setsslmode failed with rc=%d", rc);
	fail_if(c->sslmode != ACC_SSLMODE_CLEAR, "sslmode not set correctly");
	mark_point();

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "destroy failed with rc=%d", rc);
}
END_TEST

START_TEST(tc_core_blocking)
{
	struct achat_channel    *c  = NULL;
	achat_rc                 rc = ACHAT_RC_ERROR;

	c = acc_create();
	fail_if(c == NULL, "couldn't create channel");
	mark_point();

	rc = acc_setblockingmode(c, ACC_BLOCKING);
	fail_if(rc != ACHAT_RC_OK, "acc_setblockingmode failed with rc=%d", rc);
	fail_if(c->blocking != ACC_BLOCKING, "blocking not set correctly");
	mark_point();

	rc = acc_setblockingmode(c, ACC_NON_BLOCKING);
	fail_if(rc != ACHAT_RC_OK, "acc_setblockingmode failed with rc=%d", rc);
	fail_if(c->blocking != ACC_NON_BLOCKING, "blocking not set correctly");
	mark_point();

	rc = acc_setblockingmode(c, -1);
	fail_if(rc != ACHAT_RC_INVALPARAM,
	    "blockingmode expected to fail with rc=%d but returned rc=%d",
	    ACHAT_RC_INVALPARAM, rc);
	mark_point();

	rc = acc_setblockingmode(c, 2);
	fail_if(rc != ACHAT_RC_INVALPARAM,
	    "blockingmode expected to fail with rc=%d but returned rc=%d",
	    ACHAT_RC_INVALPARAM, rc);
	mark_point();

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "destroy failed with rc=%d", rc);
}
END_TEST

START_TEST(tc_core_addr)
{
	struct achat_channel    *c = NULL;
	struct sockaddr_storage	 sa;
	struct sockaddr_un	*sa_un = (struct sockaddr_un *)&sa;
	struct sockaddr_in	*sa_in = (struct sockaddr_in *)&sa;
	char			 testpath[] = "The road goes ever on and on";
	achat_rc                 rc = ACHAT_RC_ERROR;

	c = acc_create();
	fail_if(c == NULL, "couldn't create channel");
	mark_point();

	rc = acc_setaddr(NULL, NULL, 0);
	fail_if(rc != ACHAT_RC_INVALPARAM,
	    "setaddr expected to fail with rc=%d but returned rc=%d",
	    ACHAT_RC_INVALPARAM, rc);
	mark_point();

	rc = acc_setaddr(c, NULL, 0);
	fail_if(rc != ACHAT_RC_INVALPARAM,
	    "setaddr expected to fail with rc=%d but returned rc=%d",
	    ACHAT_RC_INVALPARAM, rc);
	mark_point();

	bzero(&sa, sizeof(sa));
	sa_un->sun_family = AF_UNIX;
	strlcpy(sa_un->sun_path, testpath, sizeof(sa_un->sun_path));
#ifdef OPENBSD
	sa_un->sun_len = SUN_LEN(sa_un);
#endif
	rc = acc_setaddr(c, &sa, sizeof(struct sockaddr_un));
	fail_if(rc != ACHAT_RC_OK, "setaddr failed with rc=%d", rc);
	if (memcmp(&c->addr, &sa, sizeof(sa)) != 0)
		fail("socket address not set correctly");
	fail_if(c->addrsize != sizeof(struct sockaddr_un),
		"socket address not set correctly");
	mark_point();

	bzero(&sa, sizeof(sa));
	sa_in->sin_family = AF_INET;
	sa_in->sin_port = htons(ACHAT_SERVER_PORT);
	inet_aton("127.0.0.1", &(sa_in->sin_addr));
#ifdef OPENBSD
	sa_in->sin_len = sizeof(struct sockaddr_in);
#endif
	rc = acc_setaddr(c, &sa, sizeof(struct sockaddr_in));
	fail_if(rc != ACHAT_RC_OK, "setaddr failed with rc=%d", rc);
	if (memcmp(&c->addr, &sa, sizeof(sa)) != 0)
		fail("socket address not set correctly");
	fail_if(c->addrsize != sizeof(struct sockaddr_in),
		"socket address not set correctly");
	mark_point();

	/* XXX by ch: test of AF_INET6 is missing here */

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "destroy failed with rc=%d", rc);
}
END_TEST

START_TEST(tc_core_state_initialised)
{
	struct achat_channel    *c = NULL;
	struct sockaddr_storage	 sa;
	struct sockaddr_in	*sa_in = (struct sockaddr_in *)&sa;
	achat_rc                 rc = ACHAT_RC_ERROR;

	c = acc_create();
	fail_if(c == NULL, "couldn't create channel");
	mark_point();

	bzero(&sa, sizeof(sa));
	sa_in->sin_family = AF_INET;
	sa_in->sin_port = htons(ACHAT_SERVER_PORT);
	inet_aton("127.0.0.1", &(sa_in->sin_addr));
#ifdef OPENBSD
	sa_in->sin_len = sizeof(struct sockaddr_in);
#endif

	rc = acc_setaddr(c, &sa, sizeof(struct sockaddr_in));
	fail_if(rc != ACHAT_RC_OK, "setaddr failed with rc=%d", rc);
	if (memcmp(&c->addr, &sa, sizeof(sa)) != 0)
		fail("socket address not set correctly");
	fail_if(c->addrsize != sizeof(struct sockaddr_in),
		"socket address not set correctly");
	mark_point();

	rc = acc_settail(c, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "settail failed with rc=%d", rc);
	fail_if(c->tail != ACC_TAIL_SERVER, "tail not set correctly");
	mark_point();

	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "setsslmode failed with rc=%d", rc);
	fail_if(c->sslmode != ACC_SSLMODE_CLEAR, "sslmode not set correctly");

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "destroy failed with rc=%d", rc);
}
END_TEST

TCase *
libanoubischat_testcase_core(void)
{
	/* Core test case */
	TCase *tc_core = tcase_create("Core");

	tcase_add_test(tc_core, tc_core_creation);
	tcase_add_test(tc_core, tc_core_tail);
	tcase_add_test(tc_core, tc_core_sslmode);
	tcase_add_test(tc_core, tc_core_blocking);
	tcase_add_test(tc_core, tc_core_addr);
	tcase_add_test(tc_core, tc_core_state_initialised);

	return (tc_core);
}

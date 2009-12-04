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

#include <config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <check.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "anoubischat.h"
#include <anoubis_protocol.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>
#include <anoubis_dump.h>

#ifdef LINUX
#include <linux/anoubis.h>
#include <bsdcompat.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

/* It's a bit tricky to include anoubisd.h here to access the path and
 * name of the listening socket, because we would inherit the need of
 * all include paths (kernel headers) as the anoubisd itself (see
 * struct eventdev_hdr). Thus we set it manually for this test.
 */
#define ANOUBISD_SOCKETNAME	"/var/run/anoubisd.sock"
const char * sockname = ANOUBISD_SOCKETNAME;

START_TEST(tc_Sessions_one)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
	struct achat_channel	*c  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;
	int			 ret;
	struct anoubis_client   *client;

	c = acc_create();
	fail_if(c == NULL, "couldn't create channel");
	rc = acc_settail(c, ACC_TAIL_CLIENT);
	fail_if(rc != ACHAT_RC_OK, "settail failed with rc=%d", rc);
	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "setsslmode failed with rc=%d", rc);
	mark_point();

	bzero(&sa.ss, sizeof(sa.ss));
	sa.un.sun_family = AF_UNIX;
	strlcpy(sa.un.sun_path, sockname, sizeof(sa.un.sun_path));
	rc = acc_setaddr(c, &sa.ss, sizeof(struct sockaddr_un));
	fail_if(rc != ACHAT_RC_OK, "setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc != ACHAT_RC_OK, "prepare failed with rc=%d [%s]", rc,
	    strerror(errno));
	rc = acc_open(c);
	fail_if(rc != ACHAT_RC_OK, "open failed with rc=%d [%s]", rc,
	    strerror(errno));
	mark_point();

	client = anoubis_client_create(c, ANOUBIS_AUTH_TRANSPORT, NULL);
	fail_if(!client, "Failed to create client");
	mark_point();

	ret = anoubis_client_connect(client, ANOUBIS_PROTO_BOTH);
	fail_if(ret < 0, "client connect failed with code %d", ret);
	mark_point();

	anoubis_client_close(client);
	mark_point();

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "destroy failed with rc=%d", rc);
}
END_TEST

START_TEST(tc_Sessions_two)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
	struct achat_channel	*c1  = NULL;
	struct achat_channel	*c2  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;
	struct anoubis_client   *client1, *client2;
	int			 ret;

	c1 = acc_create();
	fail_if(c1 == NULL, "couldn't create first channel");
	rc = acc_settail(c1, ACC_TAIL_CLIENT);
	fail_if(rc != ACHAT_RC_OK, "1st channel settail failed with rc=%d", rc);
	rc = acc_setsslmode(c1, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "1st channel setsslmode failed with rc=%d",
	    rc);
	mark_point();

	c2 = acc_create();
	fail_if(c2 == NULL, "couldn't create second channel");
	rc = acc_settail(c2, ACC_TAIL_CLIENT);
	fail_if(rc != ACHAT_RC_OK, "2nd channel settail failed with rc=%d", rc);
	rc = acc_setsslmode(c2, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "2nd channel setsslmode failed with rc=%d",
	    rc);
	mark_point();

	bzero(&sa.ss, sizeof(sa.ss));
	sa.un.sun_family = AF_UNIX;
	strlcpy(sa.un.sun_path, sockname, sizeof(sa.un.sun_path));
	rc = acc_setaddr(c1, &sa.ss, sizeof(struct sockaddr_un));
	fail_if(rc != ACHAT_RC_OK, "1st channel setaddr failed with rc=%d", rc);
	rc = acc_setaddr(c2, &sa.ss, sizeof(struct sockaddr_un));
	fail_if(rc != ACHAT_RC_OK, "2nd channel setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(c1);
	fail_if(rc != ACHAT_RC_OK, "1st channel prepare failed with rc=%d [%s]",
	    rc, strerror(errno));
	rc = acc_open(c1);
	fail_if(rc != ACHAT_RC_OK, "1st channel open failed with rc=%d [%s]",
	    rc, strerror(errno));
	mark_point();

	client1 = anoubis_client_create(c1, ANOUBIS_AUTH_TRANSPORT, NULL);
	fail_if(!client1, "1st client create failed");
	mark_point();

	rc = acc_prepare(c2);
	fail_if(rc != ACHAT_RC_OK, "2nd channel prepare failed with rc=%d [%s]",
	    rc, strerror(errno));
	rc = acc_open(c2);
	fail_if(rc != ACHAT_RC_OK, "2nd channel open failed with rc=%d [%s]",
	    rc, strerror(errno));
	mark_point();
	client2 = anoubis_client_create(c2, ANOUBIS_AUTH_TRANSPORT, NULL);
	fail_if(!client1, "2nd client create failed");
	mark_point();

	ret = anoubis_client_connect(client1, ANOUBIS_PROTO_BOTH);
	fail_if(ret < 0, "1st client connect failed with %d", ret);
	mark_point();

	ret = anoubis_client_connect(client2, ANOUBIS_PROTO_BOTH);
	fail_if(ret < 0, "2nd client connect failed with %d", ret);
	mark_point();

	rc = acc_destroy(c1);
	fail_if(rc != ACHAT_RC_OK, "1st channel destroy failed with rc=%d", rc);

	rc = acc_destroy(c2);
	fail_if(rc != ACHAT_RC_OK, "2nd channel destroy failed with rc=%d", rc);
}
END_TEST

START_TEST(tc_Sessions_three)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
	struct achat_channel	*c  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;
	struct anoubis_client   *client;
	struct anoubis_transaction * t;
	int count = 0;
	int ret;

	c = acc_create();
	fail_if(c == NULL, "couldn't create channel");
	rc = acc_settail(c, ACC_TAIL_CLIENT);
	fail_if(rc != ACHAT_RC_OK, "settail failed with rc=%d", rc);
	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "setsslmode failed with rc=%d", rc);
	mark_point();

	bzero(&sa.ss, sizeof(sa.ss));
	sa.un.sun_family = AF_UNIX;
	strlcpy(sa.un.sun_path, sockname, sizeof(sa.un.sun_path));
	rc = acc_setaddr(c, &sa.ss, sizeof(struct sockaddr_un));
	fail_if(rc != ACHAT_RC_OK, "setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc != ACHAT_RC_OK, "prepare failed with rc=%d [%s]", rc,
	    strerror(errno));
	rc = acc_open(c);
	fail_if(rc != ACHAT_RC_OK, "open failed with rc=%d [%s]", rc,
	    strerror(errno));
	mark_point();

	client = anoubis_client_create(c, ANOUBIS_AUTH_TRANSPORT, NULL);
	fail_if(!client, "Failed to create client");
	mark_point();

	ret = anoubis_client_connect(client, ANOUBIS_PROTO_BOTH);
	fail_if(ret < 0, "client connect failed with code %d", ret);
	mark_point();

	t = anoubis_client_register_start(client, 0x123, 0, 0,
	    ANOUBIS_SOURCE_STAT);
	fail_if(!t, "Failed to register for stat events");
	while(count < 3) {
		struct anoubis_msg * m = anoubis_msg_new(4000);
		size_t length = 4000;
		fail_if(!m, "Cannot allocate message");
		rc = acc_receivemsg(c, m->u.buf, &length);
		fail_if(rc != ACHAT_RC_OK, "receive message returned %d", rc);
		anoubis_msg_resize(m, length);
		rc = anoubis_client_process(client, m);
		fail_if(rc != 1, "client process failed");
		m = anoubis_client_getnotify(client);
		if (m) {
			anoubis_dump(m, "NOTIFY");
			anoubis_msg_free(m);
			count++;
		}
	}
	anoubis_client_close(client);
	mark_point();

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "destroy failed with rc=%d", rc);
}
END_TEST

TCase *
anoubisd_testcase_sessions(void)
{
	/* sessions test case */
	TCase *tc_sessions = tcase_create("Sessions");

	/* Timeout set to 20 seconds for tc_Sessions_three! */
	tcase_set_timeout(tc_sessions, 20);
	tcase_add_test(tc_sessions, tc_Sessions_one);
	tcase_add_test(tc_sessions, tc_Sessions_two);
	tcase_add_test(tc_sessions, tc_Sessions_three);

	return (tc_sessions);
}

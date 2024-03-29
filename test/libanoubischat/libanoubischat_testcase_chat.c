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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "anoubis_chat.h"
#include <anoubis_errno.h>
#include "config.h"
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

const char *msgs[] = {
	"Hello World",
	"Hi there!"
};

void
tc_chat_lud_client(const char *sockname, int num_msgs)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
	struct achat_channel    *c  = NULL;
	char			buffer[16];
	size_t			size;
	achat_rc		rc = ACHAT_RC_ERROR;
	int			i;

	c = acc_create();
	fail_if(c == NULL, "couldn't create client channel");
	rc = acc_settail(c, ACC_TAIL_CLIENT);
	fail_if(rc != ACHAT_RC_OK, "client settail failed with rc=%d", rc);
	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "client setsslmode failed with rc=%d", rc);
	mark_point();

	bzero(&sa.ss, sizeof(sa.ss));
	sa.un.sun_family = AF_UNIX;
	strlcpy(sa.un.sun_path, sockname, sizeof(sa.un.sun_path));
	rc = acc_setaddr(c, &sa.ss, sizeof(struct sockaddr_un));
	if (rc != ACHAT_RC_OK)
		fail("client setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc != ACHAT_RC_OK, "client prepare failed with rc=%d [%s]",
	    rc, anoubis_strerror(errno));

	sleep(4); /* give the server time to open his socket */
	rc = acc_open(c);
	fail_if(rc != ACHAT_RC_OK, "client open failed with rc=%d [%s]",
	    rc, anoubis_strerror(errno));
	mark_point();

	for (i = 0; i < num_msgs; i++) {
		bzero(buffer, sizeof(buffer));
		size = sizeof(msgs[i]);
		rc = acc_receivemsg(c, buffer, &size);
		fail_if(rc != ACHAT_RC_OK,
			"client receive msg failed with rc=%d [%s]", rc,
			anoubis_strerror(errno));
		if (strncmp(buffer, msgs[i], sizeof(msgs[i])) != 0)
			fail("client received msg mismatch [%s] != [%s]",
			msgs[i], buffer);
		if (size != sizeof(msgs[i]))
			fail("client recieved msg size mismatch %d != %d",
				size, sizeof(msgs[i]));
		mark_point();
	}

	for (i = 0; i < num_msgs; i++) {
		rc = acc_sendmsg(c, msgs[i], sizeof(msgs[i]));
		fail_if(rc != ACHAT_RC_OK,
			"client send msg failed with rc=%d [%s]", rc,
			anoubis_strerror(errno));
		mark_point();
	}

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "client destroy failed with rc=%d", rc);

	exit(0);
}

void
tc_chat_lud_server(const char *sockname, int num_msgs)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
	struct achat_channel    *s  = NULL;
	char			buffer[16];
	size_t			size;
	achat_rc		rc = ACHAT_RC_ERROR;
	int			i;

	s = acc_create();
	fail_if(s == NULL, "couldn't create server channel");
	rc = acc_settail(s, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "server settail failed with rc=%d", rc);
	rc = acc_setsslmode(s, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "server setsslmode failed with rc=%d", rc);
	mark_point();

	bzero(&sa.ss, sizeof(sa.ss));
	sa.un.sun_family = AF_UNIX;
	strlcpy(sa.un.sun_path, sockname, sizeof(sa.un.sun_path));
	rc = acc_setaddr(s, &sa.ss, sizeof(struct sockaddr_un));
	if (rc != ACHAT_RC_OK)
		fail("server setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(s);
	fail_if(rc != ACHAT_RC_OK, "server prepare failed with rc=%d [%s]",
	    rc, anoubis_strerror(errno));

	rc = acc_open(s);
	fail_if(rc != ACHAT_RC_OK, "server open failed with rc=%d [%s]",
	    rc, anoubis_strerror(errno));
	mark_point();

	for (i = 0; i < num_msgs; i++) {
		rc = acc_sendmsg(s, msgs[i], sizeof(msgs[i]));
		fail_if(rc != ACHAT_RC_OK,
			"server send msg failed with rc=%d [%s]", rc,
			anoubis_strerror(errno));
		mark_point();
	}

	for (i = 0; i < num_msgs; i++) {
		bzero(buffer, sizeof(buffer));
		size = sizeof(msgs[i]);
		rc = acc_receivemsg(s, buffer, &size);
		fail_if(rc != ACHAT_RC_OK,
			"server receive msg failed with rc=%d [%s]", rc,
			anoubis_strerror(errno));
		if (strncmp(buffer, msgs[i], sizeof(msgs[i])) != 0)
			fail("server received msg mismatch [%s] != [%s]",
				msgs[i], buffer);
		if (size != sizeof(msgs[i]))
			fail("server recieved msg size mismatch %d != %d",
				size, sizeof(msgs[i]));
		mark_point();
	}

	/* the client will close the channel now;
	 * we'll give him another couple of seconds.
	 */
	sleep(2);
	rc = acc_receivemsg(s, buffer, &size);
	fail_if(rc != ACHAT_RC_EOF, "server EOF not detected rc=%d", rc);

	rc = acc_destroy(s);
	fail_if(rc != ACHAT_RC_OK, "server destroy failed with rc=%d", rc);
}

void
tc_chat_lip_client(short port)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_in	in;
	} sa;
	struct achat_channel    *c  = NULL;
	size_t			 size;
	char			 buffer[16];
	achat_rc		 rc = ACHAT_RC_ERROR;

	c = acc_create();
	fail_if(c == NULL, "couldn't create client channel");
	rc = acc_settail(c, ACC_TAIL_CLIENT);
	fail_if(rc != ACHAT_RC_OK, "client settail failed with rc=%d", rc);
	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "client setsslmode failed with rc=%d", rc);
	mark_point();

	bzero(&sa.ss, sizeof(sa.ss));
	sa.in.sin_family = AF_INET;
	sa.in.sin_port = htons(port);
	inet_aton("127.0.0.1", &(sa.in.sin_addr));
	rc = acc_setaddr(c, &sa.ss, sizeof(struct sockaddr_in));
	if (rc != ACHAT_RC_OK)
		fail("client setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc != ACHAT_RC_OK, "client prepare failed with rc=%d [%s]",
	    rc, anoubis_strerror(errno));

	sleep(4); /* give the server time to open his socket */
	rc = acc_open(c);
	fail_if(rc != ACHAT_RC_OK, "client open failed with rc=%d [%s]",
	    rc, anoubis_strerror(errno));
	mark_point();

	rc = acc_sendmsg(c, msgs[0], sizeof(msgs[0]));
	fail_if(rc != ACHAT_RC_OK, "client send msg failed with rc=%d [%s]",
	    rc, anoubis_strerror(errno));
	mark_point();

	bzero(buffer, sizeof(buffer));
	size = sizeof(msgs[0]);
	rc = acc_receivemsg(c, buffer, &size);
	fail_if(rc != ACHAT_RC_OK, "client receive msg failed with rc=%d [%s]",
	    rc, anoubis_strerror(errno));
	if (strncmp(buffer, msgs[0], sizeof(msgs[0])) != 0)
		fail("client received msg mismatch [%s] != [%s]",
			msgs[0], buffer);
	if (size != sizeof(msgs[0]))
		fail("server recieved msg size mismatch %d != %d", size,
		    sizeof(msgs[0]));
	mark_point();

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "client destroy failed with rc=%d", rc);

	exit(0);
}


START_TEST(tc_chat_localunixdomain)
{
	pid_t childpid;
	char *sockname;

	sockname = tempnam(NULL, "ac");

	switch (childpid = check_fork()) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		/* child / clinet */
		tc_chat_lud_client(sockname, 1);
		check_waitpid_and_exit(0);
		break;
	default:
		/* parent / server */
		tc_chat_lud_server(sockname, 1);
		check_waitpid_and_exit(childpid);
		break;
	}
	if (access(sockname, F_OK) == 0)
		fail("unix domain test: socket not removed!");
}
END_TEST


START_TEST(tc_chat_localip)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_in	in;
	} sa;
	struct achat_channel    *s  = NULL;
	char			 buffer[16];
	size_t			 size;
	achat_rc		 rc = ACHAT_RC_ERROR;
	pid_t			 childpid;
	socklen_t                sslen;

	s = acc_create();
	fail_if(s == NULL, "couldn't create server channel");
	rc = acc_settail(s, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "server settail failed with rc=%d", rc);
	rc = acc_setsslmode(s, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "server setsslmode failed with rc=%d", rc);
	mark_point();

	bzero(&sa.ss, sizeof(sa.ss));
	sa.in.sin_family = AF_INET;
	sa.in.sin_port = 0;
	inet_aton("127.0.0.1", &(sa.in.sin_addr));
	rc = acc_setaddr(s, &sa.ss, sizeof(struct sockaddr_in));
	if (rc != ACHAT_RC_OK)
		fail("server setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(s);
	fail_if(rc != ACHAT_RC_OK, "server prepare failed with rc=%d [%s]",
	    rc, anoubis_strerror(errno));
	mark_point();

	bzero(&sa.ss, sizeof(sa.ss));
	sslen = sizeof(sa.ss);
	if (getsockname(s->fd, (struct sockaddr *)&sa.ss, &sslen) == -1)
		fail("error while asking about server socket name [%s]",
		    anoubis_strerror(errno));
	fail_if(sa.in.sin_port == 0,
	    "couldn't determine port of server socket");
	mark_point();

	switch (childpid = check_fork()) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		/* child / clinet */
		tc_chat_lip_client(ntohs(sa.in.sin_port));
		check_waitpid_and_exit(0);
		break;
	default:
		/* parent / server */
		rc = acc_open(s);
		fail_if(rc != ACHAT_RC_OK, "server open failed with rc=%d [%s]",
		    rc, anoubis_strerror(errno));
		mark_point();

		bzero(buffer, sizeof(buffer));
		size = sizeof(msgs[0]);
		rc = acc_receivemsg(s, buffer, &size);
		fail_if(rc != ACHAT_RC_OK,
		    "server receive msg failed with rc=%d [%s]", rc,
		    anoubis_strerror(errno));
		if (strncmp(buffer, msgs[0], sizeof(msgs[0])) != 0)
			fail("server received msg mismatch [%s] != [%s]",
			    msgs[0], buffer);
		if (size != sizeof(msgs[0]))
			fail("server recieved msg size mismatch %d != %d", size,
			    sizeof(msgs[0]));
		mark_point();

		rc = acc_sendmsg(s, msgs[0], sizeof(msgs[0]));
		fail_if(rc != ACHAT_RC_OK,
		    "server send msg failed with rc=%d [%s]", rc,
		    anoubis_strerror(errno));
		mark_point();

		/* the client will close the channel now;
		 * we'll give him another couple of seconds.
		 */
		sleep(2);
		rc = acc_receivemsg(s, buffer, &size);
		fail_if(rc != ACHAT_RC_EOF, "server EOF not detected rc=%d",
		    rc);

		rc = acc_destroy(s);
		fail_if(rc != ACHAT_RC_OK, "server destroy failed with rc=%d",
		    rc);

		check_waitpid_and_exit(childpid);
		break;
	}
}
END_TEST

START_TEST(tc_chat_two_messages)
{
	pid_t childpid;
	char *sockname;

	sockname = tempnam(NULL, "ac");

	switch (childpid = check_fork()) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		/* child / clinet */
		tc_chat_lud_client(sockname, 2);
		check_waitpid_and_exit(0);
		break;
	default:
		/* parent / server */
		tc_chat_lud_server(sockname, 2);
		check_waitpid_and_exit(childpid);
		break;
	}
	if (access(sockname, F_OK) == 0)
		fail("unix domain test: socket not removed!");
}
END_TEST

TCase *
libanoubischat_testcase_chat(void)
{
	/* Chat test case */
	TCase *tc_chat = tcase_create("Chat");

	tcase_set_timeout(tc_chat, 10);
	tcase_add_test(tc_chat, tc_chat_localunixdomain);
	tcase_add_test(tc_chat, tc_chat_localip);
	tcase_add_test(tc_chat, tc_chat_two_messages);

	return (tc_chat);
}

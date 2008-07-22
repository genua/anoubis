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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "anoubischat.h"
#include "config.h"
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#define TCCHAT_SOCKETDIR	"/tmp/"
#define TCCHAT_SOCKETPREFIX	"achat_socket."

const char msg[] = "Hello World";

void
tc_chat_lud_client(const char *sockname)
{
	struct sockaddr_storage	 ss;
	struct sockaddr_un	*ss_sun = (struct sockaddr_un *)&ss;
	struct achat_channel    *c  = NULL;
	char			 buffer[16];
	size_t			 size;
	achat_rc		 rc = ACHAT_RC_ERROR;

	c = acc_create();
	fail_if(c == NULL, "couldn't create client channel");
	rc = acc_settail(c, ACC_TAIL_CLIENT);
	fail_if(rc != ACHAT_RC_OK, "client settail failed with rc=%d", rc);
	rc = acc_setsslmode(c, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "client setsslmode failed with rc=%d", rc);
	mark_point();

	bzero(&ss, sizeof(ss));
	ss_sun->sun_family = AF_UNIX;
	strlcpy(ss_sun->sun_path, sockname, sizeof(ss_sun->sun_path));
	rc = acc_setaddr(c, &ss);
	if (rc != ACHAT_RC_OK)
		fail("client setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc != ACHAT_RC_OK, "client prepare failed with rc=%d [%s]",
	    rc, strerror(errno));

	sleep(4); /* give the server time to open his socket */
	rc = acc_open(c);
	fail_if(rc != ACHAT_RC_OK, "client open failed with rc=%d [%s]",
	    rc, strerror(errno));
	mark_point();

	bzero(buffer, sizeof(buffer));
	size = sizeof(msg);
	rc = acc_receivemsg(c, buffer, &size);
	fail_if(rc != ACHAT_RC_OK, "client receive msg failed with rc=%d [%s]",
	    rc, strerror(errno));
	if (strncmp(buffer, msg, sizeof(msg)) != 0)
		fail("client received msg mismatch [%s] != [%s]", msg, buffer);
	if (size != sizeof(msg))
		fail("client recieved msg size mismatch %d != %d", size,
		    sizeof(msg));
	mark_point();

	rc = acc_sendmsg(c, msg, sizeof(msg));
	fail_if(rc != ACHAT_RC_OK, "client send msg failed with rc=%d [%s]",
	    rc, strerror(errno));
	mark_point();

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "client destroy failed with rc=%d", rc);

	exit(0);
}

void
tc_chat_lud_server(const char *sockname)
{
	struct sockaddr_storage	 ss;
	struct sockaddr_un	*ss_sun = (struct sockaddr_un *)&ss;
	struct achat_channel    *s  = NULL;
	char			 buffer[16];
	size_t			 size;
	achat_rc		 rc = ACHAT_RC_ERROR;

	s = acc_create();
	fail_if(s == NULL, "couldn't create server channel");
	rc = acc_settail(s, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "server settail failed with rc=%d", rc);
	rc = acc_setsslmode(s, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "server setsslmode failed with rc=%d", rc);
	mark_point();

	bzero(&ss, sizeof(ss));
	ss_sun->sun_family = AF_UNIX;
	strlcpy(ss_sun->sun_path, sockname, sizeof(ss_sun->sun_path));
	rc = acc_setaddr(s, &ss);
	if (rc != ACHAT_RC_OK)
		fail("server setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(s);
	fail_if(rc != ACHAT_RC_OK, "server prepare failed with rc=%d [%s]",
	    rc, strerror(errno));

	rc = acc_open(s);
	fail_if(rc != ACHAT_RC_OK, "server open failed with rc=%d [%s]",
	    rc, strerror(errno));
	mark_point();

	rc = acc_sendmsg(s, msg, sizeof(msg));
	fail_if(rc != ACHAT_RC_OK, "server send msg failed with rc=%d [%s]",
	    rc, strerror(errno));
	mark_point();

	bzero(buffer, sizeof(buffer));
	size = sizeof(msg);
	rc = acc_receivemsg(s, buffer, &size);
	fail_if(rc != ACHAT_RC_OK, "server receive msg failed with rc=%d [%s]",
	    rc, strerror(errno));
	if (strncmp(buffer, msg, sizeof(msg)) != 0)
		fail("server received msg mismatch [%s] != [%s]", msg, buffer);
	if (size != sizeof(msg))
		fail("server recieved msg size mismatch %d != %d", size,
		    sizeof(msg));
	mark_point();

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
	struct sockaddr_storage  ss;
	struct sockaddr_in	*ss_sin = (struct sockaddr_in *)&ss;
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

	bzero(&ss, sizeof(ss));
	ss_sin->sin_family = AF_INET;
	ss_sin->sin_port = htons(port);
	inet_aton("127.0.0.1", &(ss_sin->sin_addr));
	rc = acc_setaddr(c, &ss);
	if (rc != ACHAT_RC_OK)
		fail("client setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(c);
	fail_if(rc != ACHAT_RC_OK, "client prepare failed with rc=%d [%s]",
	    rc, strerror(errno));

	sleep(4); /* give the server time to open his socket */
	rc = acc_open(c);
	fail_if(rc != ACHAT_RC_OK, "client open failed with rc=%d [%s]",
	    rc, strerror(errno));
	mark_point();

	rc = acc_sendmsg(c, msg, sizeof(msg));
	fail_if(rc != ACHAT_RC_OK, "client send msg failed with rc=%d [%s]",
	    rc, strerror(errno));
	mark_point();

	bzero(buffer, sizeof(buffer));
	size = sizeof(msg);
	rc = acc_receivemsg(c, buffer, &size);
	fail_if(rc != ACHAT_RC_OK, "client receive msg failed with rc=%d [%s]",
	    rc, strerror(errno));
	if (strncmp(buffer, msg, sizeof(msg)) != 0)
		fail("client received msg mismatch [%s] != [%s]", msg, buffer);
	if (size != sizeof(msg))
		fail("server recieved msg size mismatch %d != %d", size,
		    sizeof(msg));
	mark_point();

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "client destroy failed with rc=%d", rc);

	exit(0);
}


START_TEST(tc_chat_localunixdomain)
{
	pid_t childpid;
	char sockname[FILENAME_MAX];

	bzero(sockname, sizeof(sockname));
	snprintf(sockname, sizeof(sockname) - 1, "%s%s%s%08d",
	    TCCHAT_SOCKETDIR, TCCHAT_SOCKETPREFIX, "server", getpid());

	switch (childpid = check_fork()) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		/* child / clinet */
		tc_chat_lud_client(sockname);
		check_waitpid_and_exit(0);
		break;
	default:
		/* parent / server */
		tc_chat_lud_server(sockname);
		check_waitpid_and_exit(childpid);
		break;
	}
	if (access(sockname, F_OK) == 0)
		fail("unix domain test: socket not removed!");
}
END_TEST


START_TEST(tc_chat_localip)
{
	struct sockaddr_storage  ss;
	struct sockaddr_in	*ss_sin = (struct sockaddr_in *)&ss;
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

	bzero(&ss, sizeof(ss));
	ss_sin->sin_family = AF_INET;
	ss_sin->sin_port = 0;
	inet_aton("127.0.0.1", &(ss_sin->sin_addr));
	rc = acc_setaddr(s, &ss);
	if (rc != ACHAT_RC_OK)
		fail("server setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(s);
	fail_if(rc != ACHAT_RC_OK, "server prepare failed with rc=%d [%s]",
	    rc, strerror(errno));
	mark_point();

	bzero(&ss, sizeof(ss));
	sslen = sizeof(ss);
	if (getsockname(s->fd, (struct sockaddr *)&ss, &sslen) == -1)
		fail("error while asking about server socket name [%s]",
		    strerror(errno));
	fail_if(ss_sin->sin_port == 0,
	    "couldn't determine port of server socket");
	mark_point();

	switch (childpid = check_fork()) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		/* child / clinet */
		tc_chat_lip_client(ntohs(ss_sin->sin_port));
		check_waitpid_and_exit(0);
		break;
	default:
		/* parent / server */
		rc = acc_open(s);
		fail_if(rc != ACHAT_RC_OK, "server open failed with rc=%d [%s]",
		    rc, strerror(errno));
		mark_point();

		bzero(buffer, sizeof(buffer));
		size = sizeof(msg);
		rc = acc_receivemsg(s, buffer, &size);
		fail_if(rc != ACHAT_RC_OK,
		    "server receive msg failed with rc=%d [%s]", rc,
		    strerror(errno));
		if (strncmp(buffer, msg, sizeof(msg)) != 0)
			fail("server received msg mismatch [%s] != [%s]",
			    msg, buffer);
		if (size != sizeof(msg))
			fail("server recieved msg size mismatch %d != %d", size,
			    sizeof(msg));
		mark_point();

		rc = acc_sendmsg(s, msg, sizeof(msg));
		fail_if(rc != ACHAT_RC_OK,
		    "server send msg failed with rc=%d [%s]", rc,
		    strerror(errno));
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


TCase *
libanoubischat_testcase_chat(void)
{
	/* Chat test case */
	TCase *tc_chat = tcase_create("Chat");

	tcase_set_timeout(tc_chat, 10);
	tcase_add_test(tc_chat, tc_chat_localunixdomain);
	tcase_add_test(tc_chat, tc_chat_localip);

	return (tc_chat);
}

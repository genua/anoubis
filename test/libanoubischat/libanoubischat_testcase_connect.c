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
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "anoubis_chat.h"
#include <anoubis_errno.h>
#include "config.h"
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

void
tc_connect_lud_client(const char *sockname)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
	struct achat_channel    *c  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;

	c = acc_create();
	fail_if(c == NULL, "couldn't create client channel");
	fail_if(c->euid != (uid_t)-1, "euid expected -1 but is %i", c->euid);
	fail_if(c->egid != (gid_t)-1, "egid expected -1 but is %i", c->egid);
	mark_point();

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

	fail_if(c->euid == (uid_t)-1, "euid != -1 expected, is %i", c->euid);
	fail_if(c->egid == (gid_t)-1, "egid != -1 expected, is %i", c->egid);

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "client destroy failed with rc=%d", rc);

	exit(0);
}

void
tc_connect_lud_server(const char *sockname)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
	struct achat_channel    *s  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;

	s = acc_create();
	fail_if(s == NULL, "couldn't create server channel");
	fail_if(s->euid != (uid_t)-1, "euid expected -1 but is %i", s->euid);
	fail_if(s->egid != (gid_t)-1, "egid expected -1 but is %i", s->egid);

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

	fail_if(s->euid != geteuid(), "server retrieved bogus uid %d, "
	    "expected %d", s->euid, geteuid());
	fail_if(s->egid != getegid(), "server retrieved bogus gid %d, "
	    "expected %d", s->egid, getegid());

	rc = acc_destroy(s);
	fail_if(rc != ACHAT_RC_OK, "server destroy failed with rc=%d", rc);
}

void
tc_connect_lud_serverdup(const char *sockname)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
	struct achat_channel    *s  = NULL;
	struct achat_channel    *s2 = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;

	s = acc_create();
	fail_if(s == NULL, "couldn't create server channel");
	fail_if(s->euid != (uid_t)-1, "euid expected -1 but is %i", s->euid);
	fail_if(s->egid != (gid_t)-1, "egid expected -1 but is %i", s->egid);

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

	s2 = acc_opendup(s);
	fail_if(s2 == NULL, "server opendup failed [%s]",
		anoubis_strerror(errno));
	fail_if(s2->euid != geteuid(), "server retrieved bogus uid %d, "
	    "expected %d", s2->euid, geteuid());
	fail_if(s2->egid != getegid(), "server retrieved bogus gid %d, "
	    "expected %d", s2->egid, getegid());

	rc = acc_destroy(s);
	fail_if(rc != ACHAT_RC_OK, "server org destroy failed with rc=%d",
	    rc);

	rc = acc_destroy(s2);
	fail_if(rc != ACHAT_RC_OK, "server dup destroy failed with rc=%d",
	    rc);
}

void
tc_connect_lip_client(short port)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_in	in;
	} sa;
	struct achat_channel    *c  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;

	c = acc_create();
	fail_if(c == NULL, "couldn't create client channel");
	fail_if(c->euid != (uid_t)-1, "euid expected -1 but is %i", c->euid);
	fail_if(c->egid != (gid_t)-1, "egid expected -1 but is %i", c->egid);

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
	fail_unless(c->euid == (uid_t)-1, "euid: -1 expected but is %i",
	    c->euid);
	fail_unless(c->egid == (gid_t)-1, "egid: -1 expected but is %i",
	    c->egid);

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "client destroy failed with rc=%d", rc);

	exit(0);
}


START_TEST(tc_connect_localunixdomain)
{
	pid_t childpid;
	char *sockname;

	sockname = tempnam(NULL, "ac");

	switch (childpid = check_fork()) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		/* child / client */
		tc_connect_lud_client(sockname);
		check_waitpid_and_exit(0);
		break;
	default:
		/* parent / server */
		tc_connect_lud_server(sockname);
		check_waitpid_and_exit(childpid);
		break;
	}

	if (access(sockname, F_OK) == 0)
		fail("unix domain test: socket not removed!");
}
END_TEST


/*
 * This is the same test as tc_connect_localunixdomain, but "swap"
 * order of execution of server and client. In the past this happened
 * sporadically and triggered bug #279.
 */
START_TEST(tc_connect_localunixdomain_swapped)
{
	pid_t childpid;
	char *sockname;

	sockname = tempnam(NULL, "ac");

	switch (childpid = check_fork()) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		/* child / client */
		sleep(2);
		tc_connect_lud_client(sockname);
		check_waitpid_and_exit(0);
		break;
	default:
		/* parent / server */
		tc_connect_lud_server(sockname);
		check_waitpid_and_exit(childpid);
		break;
	}

	if (access(sockname, F_OK) == 0)
		fail("unix domain test: socket not removed!");
}
END_TEST


/*
 * This test prepares the client and server unix domain socket and run
 * a destroy on the client first. This should check/trigger bug #334 or
 * verify it's solution: the client may not unlink the server socket.
 */
START_TEST(tc_connect_localunixdomain_clientclose)
{
	char *sockname;
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_un	un;
	} sa;
	struct achat_channel    *s  = NULL;
	struct achat_channel    *c  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;

	sockname = tempnam(NULL, "ac");

	c = acc_create();
	fail_if(c == NULL, "couldn't create client channel");
	fail_if(c->euid != (uid_t)-1, "euid expected -1 but is %i", c->euid);
	fail_if(c->egid != (gid_t)-1, "egid expected -1 but is %i", c->egid);

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
	mark_point();

	s = acc_create();
	fail_if(s == NULL, "couldn't create server channel");
	fail_if(c->euid != (uid_t)-1, "euid expected -1 but is %i", c->euid);
	fail_if(c->egid != (gid_t)-1, "egid expected -1 but is %i", c->egid);

	rc = acc_settail(s, ACC_TAIL_SERVER);
	fail_if(rc != ACHAT_RC_OK, "server settail failed with rc=%d", rc);
	rc = acc_setsslmode(s, ACC_SSLMODE_CLEAR);
	fail_if(rc != ACHAT_RC_OK, "server setsslmode failed with rc=%d", rc);
	mark_point();

	rc = acc_setaddr(s, &sa.ss, sizeof(struct sockaddr_un));
	if (rc != ACHAT_RC_OK)
		fail("server setaddr failed with rc=%d", rc);
	mark_point();

	rc = acc_prepare(s);
	fail_if(rc != ACHAT_RC_OK, "server prepare failed with rc=%d [%s]",
	    rc, anoubis_strerror(errno));
	mark_point();

	if (access(sockname, F_OK) != 0)
		fail("unix domain socket not created!");

	rc = acc_destroy(c);
	fail_if(rc != ACHAT_RC_OK, "client destroy failed with rc=%d", rc);

	if (access(sockname, F_OK) != 0)
		fail("unix domain socket (of server) removed by client!");

	rc = acc_destroy(s);
	fail_if(rc != ACHAT_RC_OK, "server destroy failed with rc=%d", rc);

	if (access(sockname, F_OK) == 0)
		fail("unix domain test: socket not removed!");
}
END_TEST


START_TEST(tc_connect_localunixdomain_dup)
{
	pid_t childpid;
	char *sockname;

	sockname = tempnam(NULL, "ac");

	switch (childpid = check_fork()) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		/* child / client */
		tc_connect_lud_client(sockname);
		check_waitpid_and_exit(0);
		break;
	default:
		/* parent / server */
		tc_connect_lud_serverdup(sockname);
		check_waitpid_and_exit(childpid);
		break;
	}

	if (access(sockname, F_OK) == 0)
		fail("unix domain test: socket not removed!");
}
END_TEST


START_TEST(tc_connect_localip)
{
	union {
		struct sockaddr_storage	ss;
		struct sockaddr_in	in;
	} sa;
	struct achat_channel    *s  = NULL;
	achat_rc		 rc = ACHAT_RC_ERROR;
	pid_t			 childpid;
	socklen_t		 sslen;

	s = acc_create();
	fail_if(s == NULL, "couldn't create server channel");
	fail_if(s->euid != (uid_t)-1, "euid expected -1 but is %i", s->euid);
	fail_if(s->egid != (gid_t)-1, "egid expected -1 but is %i", s->egid);

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
		/* child / client */
		tc_connect_lip_client(ntohs(sa.in.sin_port));
		check_waitpid_and_exit(0);
		break;
	default:
		/* parent / server */
		rc = acc_open(s);
		fail_if(rc != ACHAT_RC_OK, "server open failed with rc=%d [%s]",
		    rc, anoubis_strerror(errno));
		fail_unless(s->euid == (uid_t)-1, "euid: -1 expected but is %i",
			s->euid);
		fail_unless(s->egid == (gid_t)-1, "egid: -1 expected but is %i",
			s->egid);

		rc = acc_destroy(s);
		fail_if(rc != ACHAT_RC_OK, "server destroy failed with rc=%d",
		    rc);

		check_waitpid_and_exit(childpid);
		break;
	}
}
END_TEST


TCase *
libanoubischat_testcase_connect(void)
{
	/* Connect test case */
	TCase *tc_connect = tcase_create("Connect");

	tcase_set_timeout(tc_connect, 10);
	tcase_add_test(tc_connect, tc_connect_localunixdomain);
	tcase_add_test(tc_connect, tc_connect_localunixdomain_swapped);
	tcase_add_test(tc_connect, tc_connect_localunixdomain_clientclose);
	tcase_add_test(tc_connect, tc_connect_localunixdomain_dup);
	tcase_add_test(tc_connect, tc_connect_localip);

	return (tc_connect);
}

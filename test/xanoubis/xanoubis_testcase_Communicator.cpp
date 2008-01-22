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
#include "Communicator.h"
#include "CommunicatorException.h"

#define TCCHAT_SOCKETDIR        "/tmp/"
#define TCCHAT_SOCKETPREFIX     "achat_socket."

const char msg[] = "Hello World";

void
tc_Communicator_lud_client(const char *sockname)
{
	struct sockaddr_storage  ss;
	struct sockaddr_un      *ss_sun = (struct sockaddr_un *)&ss;
	Communicator		*com;
	char                     buffer[16];

	com = new Communicator();
	fail_if(com == NULL, "couldn't create client communicator");
	try {
		com->setTail(ACC_TAIL_CLIENT);
	} catch(CommunicatorException *ce) {
		fail("client setTail failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	try {
		com->setSslMode(ACC_SSLMODE_CLEAR);
	} catch(CommunicatorException *ce) {
		fail("client setSslMode failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	bzero(&ss, sizeof(ss));
	ss_sun->sun_family = AF_UNIX;
	strncpy(ss_sun->sun_path, sockname, sizeof(ss_sun->sun_path) - 1 );
	try {
		com->setAddr(&ss);
	} catch(CommunicatorException *ce) {
		fail("client setAddr failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	try {
		com->prepare();
	} catch(CommunicatorException *ce) {
		fail("client prepare failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	sleep(4); /* give the server time to open his socket */
	try {
		com->open();
	} catch(CommunicatorException *ce) {
		fail("client open failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	bzero(buffer, sizeof(buffer));
	try {
		com->receiveMessage(buffer, sizeof(msg));
	} catch(CommunicatorException *ce) {
		fail("client receiveMessage failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	if (strncmp(buffer, msg, sizeof(msg)) != 0)
		fail("client received msg mismatch [%s] != [%s]", msg, buffer);
	mark_point();

	try {
		com->sendMessage(msg, sizeof(msg));
	} catch(CommunicatorException *ce) {
		fail("client sendMessage failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}

	delete com;
	exit(0);
}

void
tc_Communicator_lud_server(const char *sockname)
{
	struct sockaddr_storage  ss;
	struct sockaddr_un      *ss_sun = (struct sockaddr_un *)&ss;
	Communicator		*com;
	char                     buffer[16];

	com = new Communicator();
	fail_if(com == NULL, "couldn't create server communicator");
	try {
		com->setTail(ACC_TAIL_SERVER);
	} catch(CommunicatorException *ce) {
		fail("server setTail failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	try {
		com->setSslMode(ACC_SSLMODE_CLEAR);
	} catch(CommunicatorException *ce) {
		fail("server setSslMode failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	bzero(&ss, sizeof(ss));
	ss_sun->sun_family = AF_UNIX;
	strncpy(ss_sun->sun_path, sockname, sizeof(ss_sun->sun_path) - 1 );
	try {
		com->setAddr(&ss);
	} catch(CommunicatorException *ce) {
		fail("server setAddr failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	try {
		com->prepare();
	} catch(CommunicatorException *ce) {
		fail("server prepare failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	try {
		com->open();
	} catch(CommunicatorException *ce) {
		fail("server open failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	try {
		com->sendMessage(msg, sizeof(msg));
	} catch(CommunicatorException *ce) {
		fail("server sendMessage failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	bzero(buffer, sizeof(buffer));
	try {
		com->receiveMessage(buffer, sizeof(msg));
	} catch(CommunicatorException *ce) {
		fail("server receiveMessage failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	if (strncmp(buffer, msg, sizeof(msg)) != 0)
		fail("server received msg mismatch [%s] != [%s]", msg, buffer);

	delete com;
}

void
tc_Communicator_lip_client(short port)
{
	struct sockaddr_storage  ss;
	struct sockaddr_in      *ss_sin = (struct sockaddr_in *)&ss;
	Communicator		*com;
	char                     buffer[16];

	com = new Communicator();
	fail_if(com == NULL, "couldn't create client communicator");
	try {
		com->setTail(ACC_TAIL_CLIENT);
	} catch(CommunicatorException *ce) {
		fail("client setTail failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	try {
		com->setSslMode(ACC_SSLMODE_CLEAR);
	} catch(CommunicatorException *ce) {
		fail("client setSslMode failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	bzero(&ss, sizeof(ss));
	ss_sin->sin_family = AF_INET;
	ss_sin->sin_port = htons(port);
	inet_aton("127.0.0.1", &(ss_sin->sin_addr));
	try {
		com->setAddr(&ss);
	} catch(CommunicatorException *ce) {
		fail("client setAddr failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	try {
		com->prepare();
	} catch(CommunicatorException *ce) {
		fail("client prepare failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	sleep(4); /* give the server time to open his socket */
	try {
		com->open();
	} catch(CommunicatorException *ce) {
		fail("client open failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	bzero(buffer, sizeof(buffer));
	try {
		com->receiveMessage(buffer, sizeof(msg));
	} catch(CommunicatorException *ce) {
		fail("client receiveMessage failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	if (strncmp(buffer, msg, sizeof(msg)) != 0)
		fail("client received msg mismatch [%s] != [%s]", msg, buffer);
	mark_point();

	try {
		com->sendMessage(msg, sizeof(msg));
	} catch(CommunicatorException *ce) {
		fail("client sendMessage failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}

	delete com;
	exit(0);
}


START_TEST(tc_Communicator_creation)
{
	Communicator *comm = new Communicator();
	fail_if(comm == NULL, "couldn't create communicator");
	delete comm;
}
END_TEST

START_TEST(tc_Communicator_localunixdomain)
{
	pid_t pid, childpid;
	char sockname[FILENAME_MAX];

	bzero(sockname, sizeof(sockname));
	snprintf(sockname, sizeof(sockname) - 1, "%s%s%s%08d",
	    TCCHAT_SOCKETDIR, TCCHAT_SOCKETPREFIX, "server", getpid());

	switch (childpid = fork()) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		/* child / clinet */
		tc_Communicator_lud_client(sockname);
		break;
	default:
		/* parent / server */
		tc_Communicator_lud_server(sockname);
		break;
	}
	/* cleanup childs */
	do {
		if ((pid = wait(NULL)) == -1 &&
		    errno != EINTR && errno != ECHILD)
			fail("errors while cleanup childs (wait)");
	} while (pid != -1 || (pid == -1 && errno == EINTR));
	if (access(sockname, F_OK) == 0)
		fail("unix domain test: socket not removed!");
}
END_TEST

START_TEST(tc_Communicator_localip)
{
	struct sockaddr_storage  ss;
	struct sockaddr_in      *ss_sin = (struct sockaddr_in *)&ss;
	Communicator            *com;
	char                     buffer[16];
	pid_t                    pid, childpid;

	com = new Communicator();
	fail_if(com == NULL, "couldn't create server communicator");
	try {
		com->setTail(ACC_TAIL_SERVER);
	} catch(CommunicatorException *ce) {
		fail("server setTail failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	try {
		com->setSslMode(ACC_SSLMODE_CLEAR);
	} catch(CommunicatorException *ce) {
		fail("server setSslMode failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	bzero(&ss, sizeof(ss));
	ss_sin->sin_family = AF_INET;
	ss_sin->sin_port = 0;
	inet_aton("127.0.0.1", &(ss_sin->sin_addr));
	try {
		com->setAddr(&ss);
	} catch(CommunicatorException *ce) {
		fail("server setAddr failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	try {
		com->prepare();
	} catch(CommunicatorException *ce) {
		fail("server prepare failed with r=%d [%ls]",
		    ce->getReason(), ce->explain()->c_str());
	}
	mark_point();

	switch (childpid = fork()) {
	case -1:
		fail("couldn't fork");
		break;
	case 0:
		/* child / clinet */
		tc_Communicator_lip_client(ntohs(com->getPort()));
		break;
	default:
		/* parent / server */
		try {
			com->open();
		} catch(CommunicatorException *ce) {
			fail("server open failed with r=%d [%ls]",
			    ce->getReason(), ce->explain()->c_str());
		}
		mark_point();

		try {
			com->sendMessage(msg, sizeof(msg));
		} catch(CommunicatorException *ce) {
			fail("server sendMessage failed with r=%d [%ls]",
			    ce->getReason(), ce->explain()->c_str());
		}
		mark_point();

		bzero(buffer, sizeof(buffer));
		try {
			com->receiveMessage(buffer, sizeof(msg));
		} catch(CommunicatorException *ce) {
			fail("server receiveMessage failed with r=%d [%ls]",
			    ce->getReason(), ce->explain()->c_str());
		}
		if (strncmp(buffer, msg, sizeof(msg)) != 0)
			fail("server received msg mismatch [%s] != [%s]", msg,
			    buffer);
		break;
	}
	delete com;
	/* cleanup childs */
	do {
		if ((pid = wait(NULL)) == -1 &&
		    errno != EINTR && errno != ECHILD)
			fail("errors while cleanup childs (wait)");
	} while (pid != -1 || (pid == -1 && errno == EINTR));
}
END_TEST

TCase *
xanoubis_testcase_Communicator(void)
{
	/* Communicator test case */
	TCase *tc_comm = tcase_create("Communicator");

	tcase_set_timeout(tc_comm, 10);
	tcase_add_test(tc_comm, tc_Communicator_creation);
	tcase_add_test(tc_comm, tc_Communicator_localunixdomain);
	tcase_add_test(tc_comm, tc_Communicator_localip);

	return (tc_comm);
}

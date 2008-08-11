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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <check.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <anoubischat.h>

char raw_msg[] = {
	0x0, 0x0, 0x0, 0xE,
	'H', 'a', 'l', 'l', 'o', ' ', 'W', 'e', 'l', 't'
};

static struct achat_channel*
tc_chatnb_channel_init(int *port)
{
	struct achat_channel	*acc;
	struct sockaddr_storage  ss;
	struct sockaddr_in	*ss_sin = (struct sockaddr_in *)&ss;
	socklen_t		sslen;

	if ((acc = acc_create()) == NULL)
		return (NULL);

	if (acc_settail(acc, ACC_TAIL_SERVER) != ACHAT_RC_OK) {
		acc_destroy(acc);
		return (NULL);
	}

	if (acc_setsslmode(acc, ACC_SSLMODE_CLEAR) != ACHAT_RC_OK) {
		acc_destroy(acc);
		return (NULL);
	}

	if (acc_setblockingmode(acc, ACC_NON_BLOCKING) != ACHAT_RC_OK) {
		acc_destroy(acc);
		return (NULL);
	}

	bzero(&ss, sizeof(ss));
	ss_sin->sin_family = AF_INET;
	ss_sin->sin_port = 0;
	inet_aton("127.0.0.1", &(ss_sin->sin_addr));

	if (acc_setaddr(acc, &ss, sizeof(struct sockaddr_in)) != ACHAT_RC_OK) {
		acc_destroy(acc);
		return (NULL);
	}

	if (acc_prepare(acc) != ACHAT_RC_OK) {
		acc_destroy(acc);
		return (NULL);
	}

	bzero(&ss, sizeof(ss));
	sslen = sizeof(ss);
	if (getsockname(acc->fd, (struct sockaddr *)&ss, &sslen) == -1) {
		acc_destroy(acc);
		return (NULL);
	}

	*port = ntohs(ss_sin->sin_port);
	return (acc);
}

static void
tc_chatnb_channel_destroy(struct achat_channel *acc)
{
	if (acc != NULL)
		acc_destroy(acc);
}

static int
tc_chatnb_socket_init(int port)
{
	struct sockaddr_storage  ss;
	struct sockaddr_in	*ss_sin = (struct sockaddr_in *)&ss;
	int			so;

	bzero(&ss, sizeof(ss));
	ss_sin->sin_family = AF_INET;
	ss_sin->sin_port = htons(port);
	inet_aton("127.0.0.1", &(ss_sin->sin_addr));

	so = socket(ss.ss_family, SOCK_STREAM, 0);
	if (so == -1)
		return (-1);

	if (connect(so, (struct sockaddr *)&ss,
		sizeof(struct sockaddr_in)) == -1)
		return (-1);

	return so;
}

static void
tc_chatnb_socket_destroy(int so)
{
	if (so != -1)
		close(so);
}

static int
tc_chatnb_read_cmd(int fd)
{
	int cmd = -1;

	if (read(fd, &cmd, sizeof(cmd)) == -1)
		cmd = -1;

	return cmd;
}

static int
tc_chatnb_write_cmd(int fd, int cmd)
{
	return (write(fd, &cmd, sizeof(cmd)) == sizeof(cmd));
}

int
tc_chatnb_read_arg(int fd, void *arg, size_t *sarg)
{
	if (read(fd, sarg, sizeof(size_t)) != sizeof(size_t))
		return 0;

	if (read(fd, arg, *sarg) != *sarg)
		return 0;

	return 1;
}

int
tc_chatnb_write_arg(int fd, void* arg, size_t sarg)
{
	if (write(fd, &sarg, sizeof(size_t)) != sizeof(sarg))
		return 0;

	if (write(fd, arg, sarg) != sarg)
		return 0;

	return 1;
}

#define TC_CHATNB_CMD_START (int)1
#define TC_CHATNB_CMD_QUIT (int)2
#define TC_CHATNB_CMD_READ (int)3
#define TC_CHATNB_CMD_WRITE (int)4

int
tc_chatnb_child(int pfd)
{
	int so = -1;

	while (1) {
		int cmd = tc_chatnb_read_cmd(pfd);

		if (cmd == TC_CHATNB_CMD_START) {
			size_t sport = 0;
			int port = 0;

			tc_chatnb_read_arg(pfd, &port, &sport);
			so = tc_chatnb_socket_init(port);

			if (so == -1)
				return (1);
		}
		else if (cmd == TC_CHATNB_CMD_QUIT) {
			tc_chatnb_socket_destroy(so);
			return (0);
		}
		else if (cmd == TC_CHATNB_CMD_READ) {
			char	expmsg[256];
			size_t	sexpmsg		= sizeof(expmsg);
			char	msg[256];
			size_t	nread		= 0;

			/* The expected message */
			tc_chatnb_read_arg(pfd, expmsg, &sexpmsg);

			while (nread < sexpmsg) {
				int result = read(so, msg + nread,
						sexpmsg - nread);
				if (result == -1)
					return (1);

				nread += result;
			}

			return memcmp(expmsg, msg, sexpmsg);
		}
		else if (cmd == TC_CHATNB_CMD_WRITE) {
			char	msg[256];
			size_t	smsg		= sizeof(msg);
			size_t	nwritten	= 0;

			tc_chatnb_read_arg(pfd, msg, &smsg);

			while (nwritten < smsg) {
				int result = write(so, msg + nwritten,
						smsg - nwritten);
				if (result == -1)
					return (1);

				nwritten += result;
			}
		}
		else {
			/* Unsupported command */
			return (2);
		}
	}
}

START_TEST(tc_chatnb_nomessage)
{
	int	pfd[2];
	pid_t	cpid;

	if (pipe(pfd) == -1)
		fail("Failed to create pipe: %s", strerror(errno));

	cpid = check_fork();
	fail_if(cpid == -1, "Failed to fork sub-process: %s", strerror(errno));

	if (cpid == 0) { /* Child process */
		int result = tc_chatnb_child(pfd[0]);
		close(pfd[0]);
		close(pfd[1]);

		exit(result);
	}
	else { /* Father process */
		struct achat_channel	*sc, *cc;
		int			port = 0;
		char			msg[256];
		size_t			smsg = sizeof(msg);
		achat_rc		rc;

		sc = tc_chatnb_channel_init(&port);
		fail_if(sc == NULL, "Failed to initialize channel: %s",
			strerror(errno));

		/* Ask child to establish connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_START);
		tc_chatnb_write_arg(pfd[1], &port, sizeof(port));

		/* Wait for client-connection */
		cc = acc_opendup(sc);
		fail_if(cc == NULL, "Failed to connect to client: %s",
			strerror(errno));

		/* Receive a message */
		rc = acc_receivemsg(cc, msg, &smsg);
		fail_if(rc != ACHAT_RC_PENDING,
			"Unexpected receive-result %i", rc);

		/* Ask one more time for a message */
		rc = acc_receivemsg(cc, msg, &smsg);
		fail_if(rc != ACHAT_RC_PENDING,
			"Unexpected receive-result %i", rc);

		/* Ask child to close connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_QUIT);
		tc_chatnb_channel_destroy(sc);
		tc_chatnb_channel_destroy(cc);

		close(pfd[0]); close(pfd[1]);

		/* Wait for child */
		check_waitpid_and_exit(cpid);
		exit(0);
	}
}
END_TEST

START_TEST(tc_chatnb_complete_message)
{
	int	pfd[2];
	pid_t	cpid;

	if (pipe(pfd) == -1)
		fail("Failed to create pipe: %s", strerror(errno));

	cpid = check_fork();
	fail_if(cpid == -1, "Failed to fork sub-process: %s", strerror(errno));

	if (cpid == 0) { /* Child process */
		int result = tc_chatnb_child(pfd[0]);
		close(pfd[0]);
		close(pfd[1]);

		exit(result);
	}
	else { /* Father process */
		struct achat_channel	*sc, *cc;
		int			port = 0;
		char			msg[256];
		size_t			smsg = sizeof(msg);
		achat_rc		rc;
		fd_set			rfds;
		struct timeval		tv;

		sc = tc_chatnb_channel_init(&port);
		fail_if(sc == NULL, "Failed to initialize channel: %s",
			strerror(errno));

		/* Ask child to establish connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_START);
		tc_chatnb_write_arg(pfd[1], &port, sizeof(port));

		/* Wait for client-connection */
		cc = acc_opendup(sc);
		fail_if(cc == NULL, "Failed to connect to client: %s",
			strerror(errno));

		/* Watch client-channel to see when it has input */
		FD_ZERO(&rfds);
		FD_SET(cc->fd, &rfds);

		/* Wait up to one second */
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		/* Receive a message */
		rc = acc_receivemsg(cc, msg, &smsg);
		fail_if(rc != ACHAT_RC_PENDING,
			"Unexpected receive-result %i", rc);

		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_WRITE);
		tc_chatnb_write_arg(pfd[1], raw_msg, sizeof(raw_msg));

		while (1) {
			int retval = select(cc->fd + 1, &rfds, NULL, NULL, &tv);
			fail_if(retval < 0, "Select failed: %s",
				strerror(errno));

			rc = acc_receivemsg(cc, msg, &smsg);

			if (rc == ACHAT_RC_OK) {
				msg[smsg] = '\0';
				fail_if (strcmp(msg, "Hallo Welt") != 0,
					"Unexpected message received: %s", msg);
				break;
			}
			else if (rc != ACHAT_RC_PENDING) {
				fail("Unexpected recv-result: %i [%s]",
					rc, strerror(errno));
			}
		}

		/* Receive a message */
		rc = acc_receivemsg(cc, msg, &smsg);
		fail_if(rc != ACHAT_RC_PENDING,
			"Unexpected receive-result %i", rc);

		/* Ask child to close connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_QUIT);
		tc_chatnb_channel_destroy(sc);
		tc_chatnb_channel_destroy(cc);

		close(pfd[0]); close(pfd[1]);

		/* Wait for child */
		check_waitpid_and_exit(cpid);
		exit(0);
	}
}
END_TEST

START_TEST(tc_chatnb_part_message)
{
	int	pfd[2];
	pid_t	cpid;

	if (pipe(pfd) == -1)
		fail("Failed to create pipe: %s", strerror(errno));

	cpid = check_fork();
	fail_if(cpid == -1, "Failed to fork sub-process: %s", strerror(errno));

	if (cpid == 0) { /* Child process */
		int result = tc_chatnb_child(pfd[0]);
		close(pfd[0]);
		close(pfd[1]);

		exit(result);
	}
	else { /* Father process */
		struct achat_channel	*sc, *cc;
		int			port = 0;
		char			msg[256];
		size_t			smsg = sizeof(msg);
		achat_rc		rc;
		fd_set			rfds;
		struct timeval		tv;
		int			num_pendings = 0;

		sc = tc_chatnb_channel_init(&port);
		fail_if(sc == NULL, "Failed to initialize channel: %s",
			strerror(errno));

		/* Ask child to establish connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_START);
		tc_chatnb_write_arg(pfd[1], &port, sizeof(port));

		/* Wait for client-connection */
		cc = acc_opendup(sc);
		fail_if(cc == NULL, "Failed to connect to client: %s",
			strerror(errno));

		/* Watch client-channel to see when it has input */
		FD_ZERO(&rfds);
		FD_SET(cc->fd, &rfds);

		/* Wait up to one second */
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		/* Receive a message */
		rc = acc_receivemsg(cc, msg, &smsg);
		fail_if(rc != ACHAT_RC_PENDING,
			"Unexpected receive-result %i", rc);

		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_WRITE);
		tc_chatnb_write_arg(pfd[1], raw_msg, 7);

		while (1) {
			int retval = select(cc->fd + 1, &rfds, NULL, NULL, &tv);
			fail_if(retval == -1, "Select failed: %s",
				strerror(errno));

			if (retval == 0 && num_pendings == 1) { /* Timeout */
				tc_chatnb_write_cmd(pfd[1],
					TC_CHATNB_CMD_WRITE);
				tc_chatnb_write_arg(pfd[1],
					raw_msg + 7, sizeof(raw_msg) - 7);
			}

			rc = acc_receivemsg(cc, msg, &smsg);

			if (rc == ACHAT_RC_OK) {
				msg[smsg] = '\0';
				fail_if (strcmp(msg, "Hallo Welt") != 0,
					"Unexpected message received: %s", msg);
				break;
			}
			else if (rc == ACHAT_RC_PENDING) {
				num_pendings++;
			}
			else {
				fail("Unexpected receive-result: %i [%s]",
					rc, strerror(errno));
			}
		}

		/* Receive a message */
		rc = acc_receivemsg(cc, msg, &smsg);
		fail_if(rc != ACHAT_RC_PENDING,
			"Unexpected receive-result %i", rc);

		/* Ask child to close connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_QUIT);
		tc_chatnb_channel_destroy(sc);
		tc_chatnb_channel_destroy(cc);

		close(pfd[0]); close(pfd[1]);

		/* Wait for child */
		check_waitpid_and_exit(cpid);
		exit(0);
	}
}
END_TEST

START_TEST(tc_chatnb_cutheader_message)
{
	int	pfd[2];
	pid_t	cpid;

	if (pipe(pfd) == -1)
		fail("Failed to create pipe: %s", strerror(errno));

	cpid = check_fork();
	fail_if(cpid == -1, "Failed to fork sub-process: %s", strerror(errno));

	if (cpid == 0) { /* Child process */
		int result = tc_chatnb_child(pfd[0]);
		close(pfd[0]);
		close(pfd[1]);

		exit(result);
	}
	else { /* Father process */
		struct achat_channel	*sc, *cc;
		int			port = 0;
		char			msg[256];
		size_t			smsg = sizeof(msg);
		achat_rc		rc;
		fd_set			rfds;
		struct timeval		tv;
		int			num_pendings = 0;

		sc = tc_chatnb_channel_init(&port);
		fail_if(sc == NULL, "Failed to initialize channel: %s",
			strerror(errno));

		/* Ask child to establish connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_START);
		tc_chatnb_write_arg(pfd[1], &port, sizeof(port));

		/* Wait for client-connection */
		cc = acc_opendup(sc);
		fail_if(cc == NULL, "Failed to connect to client: %s",
			strerror(errno));

		/* Watch client-channel to see when it has input */
		FD_ZERO(&rfds);
		FD_SET(cc->fd, &rfds);

		/* Wait up to one second */
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		/* Receive a message */
		rc = acc_receivemsg(cc, msg, &smsg);
		fail_if(rc != ACHAT_RC_PENDING,
			"Unexpected receive-result %i", rc);

		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_WRITE);
		tc_chatnb_write_arg(pfd[1], raw_msg, 2);

		while (1) {
			int retval = select(cc->fd + 1, &rfds, NULL, NULL, &tv);
			fail_if(retval == -1, "Select failed: %s",
				strerror(errno));

			if (retval == 0 && num_pendings == 1) { /* Timeout */
				tc_chatnb_write_cmd(pfd[1],
					TC_CHATNB_CMD_WRITE);
				tc_chatnb_write_arg(pfd[1],
					raw_msg + 2, sizeof(raw_msg) - 2);
			}

			rc = acc_receivemsg(cc, msg, &smsg);

			if (rc == ACHAT_RC_OK) {
				msg[smsg] = '\0';
				fail_if (strcmp(msg, "Hallo Welt") != 0,
					"Unexpected message received: %s", msg);
				break;
			}
			else if (rc == ACHAT_RC_PENDING) {
				num_pendings++;
			}
			else {
				fail("Unexpected receive-result: %i [%s]",
					rc, strerror(errno));
			}
		}

		/* Receive a message */
		rc = acc_receivemsg(cc, msg, &smsg);
		fail_if(rc != ACHAT_RC_PENDING,
			"Unexpected receive-result %i", rc);

		/* Ask child to close connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_QUIT);
		tc_chatnb_channel_destroy(sc);
		tc_chatnb_channel_destroy(cc);

		close(pfd[0]); close(pfd[1]);

		/* Wait for child */
		check_waitpid_and_exit(cpid);
		exit(0);
	}
}
END_TEST

START_TEST(tc_chatnb_write)
{
	int	pfd[2];
	pid_t	cpid;

	if (pipe(pfd) == -1)
		fail("Failed to create pipe: %s", strerror(errno));

	cpid = check_fork();
	fail_if(cpid == -1, "Failed to fork sub-process: %s", strerror(errno));

	if (cpid == 0) { /* Child process */
		int result = tc_chatnb_child(pfd[0]);
		close(pfd[0]);
		close(pfd[1]);

		exit(result);
	}
	else { /* Father process */
		struct achat_channel	*sc, *cc;
		int			port = 0;
		achat_rc		rc;

		sc = tc_chatnb_channel_init(&port);
		fail_if(sc == NULL, "Failed to initialize channel: %s",
			strerror(errno));

		/* Ask child to establish connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_START);
		tc_chatnb_write_arg(pfd[1], &port, sizeof(port));

		/* Wait for client-connection */
		cc = acc_opendup(sc);
		fail_if(cc == NULL, "Failed to connect to client: %s",
			strerror(errno));

		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_READ);
		tc_chatnb_write_arg(pfd[1], raw_msg, sizeof(raw_msg));

		rc = acc_sendmsg(cc, raw_msg + 4, sizeof(raw_msg) - 4);
		fail_if(rc != ACHAT_RC_OK && rc != ACHAT_RC_PENDING,
			"Failed to send message");

		while (rc != ACHAT_RC_OK) {
			rc = acc_flush(cc);
			fail_if(rc != ACHAT_RC_OK && rc != ACHAT_RC_PENDING,
				"Failed to flush channel");
		}

		/* Ask child to close connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_QUIT);
		tc_chatnb_channel_destroy(sc);
		tc_chatnb_channel_destroy(cc);

		close(pfd[0]); close(pfd[1]);

		/* Wait for child */
		check_waitpid_and_exit(cpid);
		exit(0);
	}
}
END_TEST

START_TEST(tc_chatnb_write_only)
{
	int	pfd[2];
	pid_t	cpid;

	if (pipe(pfd) == -1)
		fail("Failed to create pipe: %s", strerror(errno));

	cpid = check_fork();
	fail_if(cpid == -1, "Failed to fork sub-process: %s", strerror(errno));

	if (cpid == 0) { /* Child process */
		int result = tc_chatnb_child(pfd[0]);
		close(pfd[0]);
		close(pfd[1]);

		exit(result);
	}
	else { /* Father process */
		struct achat_channel	*sc, *cc;
		int			port = 0;
		achat_rc		rc;
		int			num_msg = 0;
		int			do_send_msg = 1;

		sc = tc_chatnb_channel_init(&port);
		fail_if(sc == NULL, "Failed to initialize channel: %s",
			strerror(errno));

		/* Ask child to establish connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_START);
		tc_chatnb_write_arg(pfd[1], &port, sizeof(port));

		/* Wait for client-connection */
		cc = acc_opendup(sc);
		fail_if(cc == NULL, "Failed to connect to client: %s",
			strerror(errno));

		while (do_send_msg > 0) {
			rc = acc_sendmsg(cc, raw_msg + 4, sizeof(raw_msg) - 4);
			num_msg++;

			if (rc == ACHAT_RC_ERROR) {
				/* Buffer is full */
				fail_if(num_msg < 2,
					"Could not flush a message: %i (%s)",
					rc, strerror(errno));
				do_send_msg = 0;
				break;
			}

			fail_if(rc != ACHAT_RC_OK && rc != ACHAT_RC_PENDING,
				"Failed to send message #%i (%i): %s",
				num_msg, rc, strerror(errno));

			while (rc != ACHAT_RC_OK) {
				rc = acc_flush(cc);

				if (rc == ACHAT_RC_ERROR) {
					/* Buffer is full */
					fail_if(num_msg < 2,
					"Could not flush a message: %i (%s)",
					rc, strerror(errno));

					do_send_msg = 0;
					break;
				}
				else if (rc != ACHAT_RC_OK)
					fail("Unexpected flush-result: %i",
						rc);
			}

			mark_point();
		}

		/* Ask child to close connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_QUIT);
		tc_chatnb_channel_destroy(sc);
		tc_chatnb_channel_destroy(cc);

		close(pfd[0]); close(pfd[1]);

		/* Wait for child */
		check_waitpid_and_exit(cpid);
		exit(0);
	}
}
END_TEST

START_TEST(tc_chatnb_eof)
{
	int	pfd[2];
	pid_t	cpid;

	if (pipe(pfd) == -1)
		fail("Failed to create pipe: %s", strerror(errno));

	cpid = check_fork();
	fail_if(cpid == -1, "Failed to fork sub-process: %s", strerror(errno));

	if (cpid == 0) { /* Child process */
		int result = tc_chatnb_child(pfd[0]);
		close(pfd[0]);
		close(pfd[1]);

		exit(result);
	}
	else { /* Father process */
		struct achat_channel	*sc, *cc;
		int			port = 0;
		char			msg[256];
		size_t			smsg = sizeof(msg);
		achat_rc		rc;
		fd_set			rfds;

		sc = tc_chatnb_channel_init(&port);
		fail_if(sc == NULL, "Failed to initialize channel: %s",
			strerror(errno));

		/* Ask child to establish connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_START);
		tc_chatnb_write_arg(pfd[1], &port, sizeof(port));

		/* Wait for client-connection */
		cc = acc_opendup(sc);
		fail_if(cc == NULL, "Failed to connect to client: %s",
			strerror(errno));

		/* Ask child to close connection */
		tc_chatnb_write_cmd(pfd[1], TC_CHATNB_CMD_QUIT);

		/* Receive a message after client socket is closed */
		FD_ZERO(&rfds);
		FD_SET(cc->fd, &rfds);
		select(cc->fd + 1, &rfds, NULL, NULL, NULL);

		rc = acc_receivemsg(cc, msg, &smsg);
		fail_if(rc != ACHAT_RC_EOF,
			"Unexpected receive-result %i", rc);

		tc_chatnb_channel_destroy(sc);
		tc_chatnb_channel_destroy(cc);

		close(pfd[0]); close(pfd[1]);

		/* Wait for child */
		check_waitpid_and_exit(cpid);
		exit(0);
	}
}
END_TEST

TCase *
libanoubischat_testcase_chatnonblock(void)
{
	/* Non-blocking chat test case */
	TCase *tc_chat = tcase_create("NonBlockingChat");

	tcase_add_test(tc_chat, tc_chatnb_nomessage);
	tcase_add_test(tc_chat, tc_chatnb_complete_message);
	tcase_add_test(tc_chat, tc_chatnb_part_message);
	tcase_add_test(tc_chat, tc_chatnb_cutheader_message);
	tcase_add_test(tc_chat, tc_chatnb_write);
	tcase_add_test(tc_chat, tc_chatnb_write_only);
	tcase_add_test(tc_chat, tc_chatnb_eof);

	return (tc_chat);
}

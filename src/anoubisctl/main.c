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

#include "config.h"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef LINUX
#include <bsdcompat.h>
#endif

#include <anoubis_protocol.h>
#include <anoubis_errno.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>

#include <anoubischat.h>

#include "anoubisctl.h"
#include "apn.h"

void		usage(void) __dead;
static int	daemon_start(void);
static int	daemon_stop(void);
static int	daemon_status(void);
static int	daemon_reload(void);
static int	load(char *);
static int	dump(char *);
static int	sfs_addsum(char *file);
static int	sfs_delsum(char *file);
static int	create_channel(void);
static void	destroy_channel(void);

typedef int (*func_int_t)(void);
typedef int (*func_char_t)(char *);

struct cmd {
	char	       *command;
	func_int_t	func;
	int		file;
} commands[] = {
	{ "start",   daemon_start,   0 },
	{ "stop",    daemon_stop,    0 },
	{ "status",  daemon_status,  0 },
	{ "reload",  daemon_reload,  0 },
	{ "load",    (func_int_t)load, 1 },
	{ "dump",    (func_int_t)dump, 1 },
	{ "addsum",  (func_int_t)sfs_addsum, 1},
	{ "delsum",  (func_int_t)sfs_delsum, 1},
};

static char    *anoubis_socket = "/var/run/anoubisd.sock";

static int	opts = 0;


static struct achat_channel	*channel;
struct anoubis_client		*client;


__dead void
usage(void)
{
	int	i;
	extern char	*__progname;

	fprintf(stderr, "usage: %s [-fnv] <command> [<file>]\n", __progname);
	fprintf(stderr, "    <command>:\n");
	for (i=0; i < sizeof(commands)/sizeof(struct cmd); i++) {
		if (commands[i].file)
			fprintf(stderr, "	%s <file>\n",
				commands[i].command);
		else
			fprintf(stderr, "	%s\n", commands[i].command);

	}
	exit(1);
}

/*
 * Return values:
 *  0 - success
 *  1 - rules parse error
 *  2 - start/stop error
 *  3 - error in getting rules
 *  4 - options parse error
 *  5 - communication error
 *  6 - checksum request error
 */

int
main(int argc, char *argv[])
{
	int	ch, i, done;
	int	error = 0;
	char	*command = NULL;
	char	*rulesopt = NULL;

	if (argc < 2)
		usage();

	while ((ch = getopt(argc, argv, "fnv")) != -1) {
		switch (ch) {

		case 'n':
			opts |= ANOUBISCTL_OPT_NOACTION;
			break;

		case 'v':
			if (opts & ANOUBISCTL_OPT_VERBOSE)
				opts |= ANOUBISCTL_OPT_VERBOSE2;
			opts |= ANOUBISCTL_OPT_VERBOSE;
			break;
		case 'f':
			opts |= ANOUBISCTL_OPT_FORCE;
			break;

		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (argc <= 0)
		usage();
		/* NOTREACHED */

	command = *argv++;
	argc--;

	if (argc > 0) {
		rulesopt = *argv++;
		argc--;
	}

	done = 0;
	for (i=0; i < sizeof(commands)/sizeof(struct cmd); i++) {

		if (strcmp(command, commands[i].command) == 0) {

			if (commands[i].file)  {

				if (rulesopt == NULL) {
					fprintf(stderr, "no rules file\n");
					error = 4;
				} else {
					error = ((func_char_t)commands[i].func)(
					    rulesopt);
					done = 1;
				}

			} else {

				if (rulesopt != NULL) {
					fprintf(stderr, "too many arguments\n");
					error = 4;
				} else {
					error = commands[i].func();
					done = 1;
				}
			}
		}
	}
	if (!done)
		usage();

	exit(error);
}

static int
daemon_start(void)
{
	int	err;
	int	error = 0;

	if ((err = system("/sbin/anoubisd"))) {
		perror("system(\"/sbin/anoubisd\")");
		error = 2;
	}

	return error;
}

static int
daemon_stop(void)
{
	FILE * fp = fopen("/var/run/anoubisd.pid", "r");
	int i, pid;

	if (!fp)
		return 2;
	if (fscanf(fp, "%d", &pid) != 1)
		return 2;
	if (kill(pid, SIGTERM) != 0)
		return 2;
	/* Wait at most 10 seconds for termination. */
	for(i=0; i<10; ++i) {
		if (kill(pid, 0) < 0 && errno == ESRCH)
			return 0;
		sleep(1);
	}
	if (kill(pid, 0) < 0 && errno == ESRCH)
		return 0;
	return 2;
}

static int
daemon_status(void)
{
	int	error = 0;

	error = create_channel();
	if (error == 0)
		printf("anoubisd is running\n");
	else
		printf("anoubisd is not running\n");
	destroy_channel();
	return error;
}

static int
daemon_reload(void)
{
	int	error = 0;

	error = create_channel();

/* XXX RD anoubis_profile ?? reload */

	destroy_channel();
	return error;
}


static void
free_msg_list(struct anoubis_msg * m)
{
	struct anoubis_msg * tmp;
	while (m) {
		tmp = m;
		m = m->next;
		anoubis_msg_free(tmp);
	}
}

static int
dump(char *file)
{
	int	error = 0;
	FILE * fp = NULL;
	struct anoubis_transaction * t;
	struct anoubis_msg * m;
	Policy_GetByUid req;
	int prio, len;

	error = create_channel();
	if (error) {
		fprintf(stderr, "Cannot connect to anoubisd\n");
		return error;
	}
	set_value(req.ptype, ANOUBIS_PTYPE_GETBYUID);
	set_value(req.uid, geteuid());

	for (prio = 0; prio < 2 /*XXX CEH: Should be PE_PRIO_MAX */; ++prio) {
		set_value(req.prio, prio);
		t = anoubis_client_policyrequest_start(client, &req,
		    sizeof(req));
		if (!t) {
			destroy_channel();
			return 3;
		}
		while(1) {
			int ret = anoubis_client_wait(client);
			if (ret <= 0) {
				anoubis_transaction_destroy(t);
				destroy_channel();
				return 3;
			}
			if (t->flags & ANOUBIS_T_DONE)
				break;
		}
		if (t->result) {
			fprintf(stderr, "Policy Request failed: %d\n",
			    t->result);
			anoubis_transaction_destroy(t);
			return 3;
		}
		m = t->msg;
		t->msg = NULL;
		anoubis_transaction_destroy(t);
		if (!m || !VERIFY_LENGTH(m, sizeof(Anoubis_PolicyReplyMessage))
		    || get_value(m->u.policyreply->error) != 0) {
			fprintf(stderr, "Error retrieving policy\n");
			free_msg_list(m);
			return 3;
		}
		if (!fp) {
			if (!file || strcmp(file, "-") == 0) {
				fp = stdout;
			} else {
				fp = fopen(file, "w");
			}
		}
		if (!fp) {
			free_msg_list(m);
			destroy_channel();
			fprintf(stderr, "Cannot open %s for writing\n", file);
			return 3;
		}
		fprintf(fp, "\nPolicies for UID %d PRIO %d\n",
		    geteuid(), prio);
		while (m) {
			struct anoubis_msg * tmp;
			len = m->length - CSUM_LEN
			    - sizeof(Anoubis_PolicyReplyMessage);
			if (fwrite(m->u.policyreply->payload, 1, len, fp)
			    != len) {
				fprintf(stderr, "Error writing to %s\n", file);
				free_msg_list(m);
				fclose(fp);
				destroy_channel();
				return 3;
			}
			tmp = m;
			m = m->next;
			anoubis_msg_free(tmp);
		}
	}
	if (fp)
		fclose(fp);
	destroy_channel();
	return 0;
}

static int
sfs_sumop(char *file, int operation)
{
	int				 error = 0;
	struct anoubis_transaction	*t;

	error = create_channel();
	if (error) {
		fprintf(stderr, "Cannot connect to anoubis daemon\n");
		return error;
	}

	t = anoubis_client_csumrequest_start(client, operation, file);
	if (!t) {
		destroy_channel();
		fprintf(stderr, "Cannot send checksum request\n");
		return 6;
	}
	while(1) {
		int ret = anoubis_client_wait(client);
		if (ret <= 0) {
			anoubis_transaction_destroy(t);
			destroy_channel();
			fprintf(stderr, "Checksum request interrupted\n");
			return 6;
		}
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}
	if (t->result) {
		fprintf(stderr, "Checksum Request failed: %d\n", t->result);
		anoubis_transaction_destroy(t);
		destroy_channel();
		return 6;
	}
	destroy_channel();
	return error;
}

static int
sfs_addsum(char *file)
{
	return sfs_sumop(file, ANOUBIS_CHECKSUM_OP_ADD);
}

static int
sfs_delsum(char *file)
{
	return sfs_sumop(file, ANOUBIS_CHECKSUM_OP_DEL);
}

static int
load(char *rulesopt)
{
	int fd;
	struct stat statbuf;
	size_t length, total;
	char * buf = NULL;
	Policy_SetByUid * req;
	struct anoubis_transaction *t;
	struct apn_ruleset *ruleset = NULL;
	int error, flags = 0;

	if (rulesopt == NULL) {
		fprintf(stderr, "Need a rules file");
		return 3;
	}
	if (strcmp(rulesopt, "-") == 0
	    && (opts & ANOUBISCTL_OPT_NOACTION) == 0)  {
		fprintf(stderr, "Rules must be loaded from a regular file\n");
		return 3;
	}
	if (opts & ANOUBISCTL_OPT_VERBOSE)
		flags |= APN_FLAG_VERBOSE;
	if (opts & ANOUBISCTL_OPT_VERBOSE2)
		flags |= APN_FLAG_VERBOSE2;
	if ((opts & ANOUBISCTL_OPT_FORCE) == 0) {
		if (apn_parse(rulesopt, &ruleset, flags)) {
			error = 1;
			if (ruleset) {
				apn_print_errors(ruleset, stderr);
				free(ruleset);
			} else {
				fprintf(stderr, "FATAL: Out of memory\n");
			}
			return error;
		}
		apn_free_ruleset(ruleset);
	}
	if (opts & ANOUBISCTL_OPT_NOACTION)
		return 0;
	error = create_channel();
	if (error) {
		fprintf(stderr, "Cannot connect to anoubisd\n");
		return error;
	}
	fd = open(rulesopt, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return 3;
	}
	if (fstat(fd, &statbuf) < 0) {
		perror("fstat");
		close(fd);
		return 3;
	}
	length = statbuf.st_size;
	if (length) {
		buf = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
		if (buf == MAP_FAILED) {
			perror("mmap");
			close(fd);
			return 3;
		}
	}
	close(fd);
	total = sizeof(*req) + length;
	req = malloc(total);
	if (!req) {
		fprintf(stderr, "Out of memory\n");
		if (length)
			munmap(buf, length);
		return 3;
	}
	set_value(req->ptype, ANOUBIS_PTYPE_SETBYUID);
	/*
	 * XXX CEH: Add posibility to set admin rules for a user and
	 * XXX CEH: default rules
	 */
	set_value(req->uid, geteuid());
	set_value(req->prio, 1);
	if (length) {
		memcpy(req->payload, buf, length);
		munmap(buf, length);
	}
	t = anoubis_client_policyrequest_start(client, req, total);
	free(req);
	if (!t) {
		fprintf(stderr, "Failed to issue policy request\n");
		destroy_channel();
		return 3;
	}
	while(1) {
		int ret = anoubis_client_wait(client);
		if (ret <= 0) {
			anoubis_transaction_destroy(t);
			destroy_channel();
			return 3;
		}
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}
	if (t->result) {
		fprintf(stderr, "Policy Request failed: %d\n", t->result);
		anoubis_transaction_destroy(t);
		return 3;
	}
	destroy_channel();
	return 0;
}

static int
create_channel(void)
{
	achat_rc		rc;
	struct sockaddr_storage ss;
	struct sockaddr_un     *ss_sun = (struct sockaddr_un *)&ss;
	int			error = 0;

	if (opts & ANOUBISCTL_OPT_VERBOSE)
		fprintf(stderr, ">create_channel\n");

	if (opts & ANOUBISCTL_OPT_VERBOSE2)
		fprintf(stderr, "acc_create\n");
	if ((channel = acc_create()) == NULL) {
		fprintf(stderr, "cannot create client channel\n");
		error = 5;
		goto err;
	}
	if (opts & ANOUBISCTL_OPT_VERBOSE2)
		fprintf(stderr, "acc_settail\n");
	if ((rc = acc_settail(channel, ACC_TAIL_CLIENT)) != ACHAT_RC_OK) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "client settail failed\n");
		error = 5;
		goto err;
	}
	if (opts & ANOUBISCTL_OPT_VERBOSE2)
		fprintf(stderr, "acc_setsslmode\n");
	if ((rc = acc_setsslmode(channel, ACC_SSLMODE_CLEAR)) != ACHAT_RC_OK) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "client setsslmode failed\n");
		error = 5;
		goto err;
	}

	bzero(&ss, sizeof(ss));
	ss_sun->sun_family = AF_UNIX;

	strlcpy(ss_sun->sun_path, anoubis_socket,
	    sizeof(ss_sun->sun_path));

	if (opts & ANOUBISCTL_OPT_VERBOSE2)
		fprintf(stderr, "acc_setaddr\n");
	rc = acc_setaddr(channel, &ss);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "client setaddr failed\n");
		error = 5;
		goto err;
	}

	if (opts & ANOUBISCTL_OPT_VERBOSE2)
		fprintf(stderr, "acc_prepare\n");
	rc = acc_prepare(channel);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "client prepare failed\n");
		error = 5;
		goto err;
	}

	if (opts & ANOUBISCTL_OPT_VERBOSE2)
		fprintf(stderr, "acc_open\n");
	rc = acc_open(channel);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "client open failed\n");
		error = 5;
		goto err;
	}

	if (opts & ANOUBISCTL_OPT_VERBOSE2)
		fprintf(stderr, "anoubis_client_create\n");
	client = anoubis_client_create(channel);
	if (client == NULL) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "anoubis_client_create failed\n");
		error = 5;
		goto err;
	}

	if (opts & ANOUBISCTL_OPT_VERBOSE2)
		fprintf(stderr, "anoubis_client_connect\n");
	if ((error = anoubis_client_connect(client, ANOUBIS_PROTO_BOTH))) {
		anoubis_client_destroy(client);
		client = NULL;
		acc_destroy(channel);
		channel = NULL;
		perror("anoubis_client_connect");
		error = 5;
		goto err;
	}

err:
	if (opts & ANOUBISCTL_OPT_VERBOSE)
		fprintf(stderr, "<create_channel\n");

	return error;
}

static void
destroy_channel(void)
{
	if (client) {
		anoubis_client_close(client);
		anoubis_client_destroy(client);
	}
	if (channel)
		acc_destroy(channel);
}

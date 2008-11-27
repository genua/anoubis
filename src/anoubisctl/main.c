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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/param.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>

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
#include <openssl/sha.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sha2.h>
#include <dev/anoubis.h>
#endif

#include <anoubis_protocol.h>
#include <anoubis_errno.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>
#include <anoubis_dump.h>

#include <anoubischat.h>
#include <apnvm/apnvm.h>

#include "anoubisctl.h"
#include "apn.h"
#include "csum/csum.h"


void		usage(void) __dead;
static int	daemon_start(void);
static int	daemon_stop(void);
static int	daemon_status(void);
static int	daemon_reload(void);
static int	monitor(int argc, char **argv);
static int	load(char *);
static int	dump(char *);
static int	sfs_addsum(char *file);
static int	sfs_delsum(char *file);
static int	sfs_getsum(char *file);
static int	sfs_calcsum(char *file);
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
	{ "getsum",  (func_int_t)sfs_getsum, 1},
	{ "calcsum",  (func_int_t)sfs_calcsum, 1},
};

static char    *anoubis_socket = "/var/run/anoubisd.sock";

static int	opts = 0;
static char	*profile = "none";


static struct achat_channel	*channel;
struct anoubis_client		*client = NULL;


__dead void
usage(void)
{
	unsigned int	i;
	extern char	*__progname;

	fprintf(stderr, "usage: %s [-fnv] [-p profile] <command> [<file>]\n",
	    __progname);
	fprintf(stderr, "    <command>:\n");
	for (i=0; i < sizeof(commands)/sizeof(struct cmd); i++) {
		if (commands[i].file)
			fprintf(stderr, "	%s <file>\n",
				commands[i].command);
		else
			fprintf(stderr, "	%s\n", commands[i].command);

	}
	fprintf(stderr, "	monitor [ delegate ] [ error=<num> ] "
	    "[ count=<num> ]\n");
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
 *  7 - Versioning error
 */

int
main(int argc, char *argv[])
{
	int		ch, done;
	unsigned int	i;
	int		error = 0;
	char		*command = NULL;
	char		*rulesopt = NULL;
	char		real_path[PATH_MAX];

	if (argc < 2)
		usage();

	while ((ch = getopt(argc, argv, "fnvp:")) != -1) {
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
		case 'p':
			profile = optarg;
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

	if (argc > 0)
		rulesopt = *argv;

	done = 0;
	for (i=0; i < sizeof(commands)/sizeof(struct cmd); i++) {

		if (strcmp(command, commands[i].command) == 0) {

			if (commands[i].file)  {

				if (rulesopt == NULL || argc != 1) {
					fprintf(stderr, "no rules file\n");
					error = 4;
				} else {
					if ((rulesopt = realpath(rulesopt, real_path))
					    == NULL) {
						perror(rulesopt);
						error = 4;
						break;
					}
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
	if (!done && strcmp(command, "monitor") == 0) {
		error = monitor(argc, argv);
		done = 1;
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
	int	prio;
	size_t	len;

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
			fprintf(stderr, "Policy Request failed: %d (%s)\n",
			    t->result, strerror(t->result));
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
sfs_sumop(char *file, int operation, u_int8_t *cs)
{
	int				 error = 0;
	int				 len = 0;
	struct anoubis_transaction	*t;

	if (!client) {
		error = create_channel();
		if (error) {
			fprintf(stderr, "Cannot connect to anoubis daemon\n");
			return error;
		}
	}

	if (cs)
		len = ANOUBIS_CS_LEN;
	t = anoubis_client_csumrequest_start(client, operation, file, cs, len,
	    0, 0, ANOUBIS_CSUM_NONE);
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
		fprintf(stderr, "Checksum Request failed: %d (%s)\n", t->result,
		    strerror(t->result));
		anoubis_transaction_destroy(t);
		destroy_channel();
		return 6;
	}
	destroy_channel();
	if (operation == ANOUBIS_CHECKSUM_OP_GET) {
		int i;
		if (!VERIFY_LENGTH(t->msg, sizeof(Anoubis_AckPayloadMessage)
		    + SHA256_DIGEST_LENGTH)) {
			fprintf(stderr, "Short checksum in reply (len=%d)\n",
			    t->msg->length);
			return 6;
		}
		for (i=0; i<SHA256_DIGEST_LENGTH; ++i)
			printf("%02x", t->msg->u.ackpayload->payload[i]);
		printf("\n");
	} else if (operation == ANOUBIS_CHECKSUM_OP_ADDSUM) {
		/* Print the checksum that has been added. */
		int i;
		for (i=0; i<SHA256_DIGEST_LENGTH; ++i)
			printf("%02x", cs[i]);
		printf("\n");
	}
	anoubis_transaction_destroy(t);
	return error;
}

static int
sfs_addsum(char *file)
{
	u_int8_t cs[ANOUBIS_CS_LEN];
	int len = ANOUBIS_CS_LEN;
	int ret;

	create_channel();
	ret = anoubis_csum_calc(file, cs, &len);
	if (ret < 0) {
		errno = -ret;
		perror("anoubis_csum_calc");
		return 6;
	}
	if (len != ANOUBIS_CS_LEN) {
		fprintf(stderr, "Bad csum length from anoubis_csum_calc\n");
		return 6;
	}
	return sfs_sumop(file, ANOUBIS_CHECKSUM_OP_ADDSUM, cs);
}

static int
sfs_delsum(char *file)
{
	return sfs_sumop(file, ANOUBIS_CHECKSUM_OP_DEL, NULL);
}

static int
sfs_getsum(char *file)
{
	return sfs_sumop(file, ANOUBIS_CHECKSUM_OP_GET, NULL);
}

static int
sfs_calcsum(char *file)
{
	int i, ret;
	u_int8_t cs[ANOUBIS_CS_LEN];
	int len = ANOUBIS_CS_LEN;

	create_channel();
	ret = anoubis_csum_calc(file, cs, &len);
	destroy_channel();
	if (ret < 0) {
		errno = -ret;
		perror("anoubis_csum_calc");
		return 6;
	}
	for (i=0; i<len; ++i) {
		printf("%02x", cs[i]);
	}
	printf("\n");
	return 0;
}

static int
make_version(struct apn_ruleset *ruleset, const char *profile)
{
	const char		*home = getenv("HOME");
	const char		*user = getenv("USER");
	char			cvsroot[PATH_MAX];
	int			nwritten;
	struct apnvm_md		md;
	apnvm			*vm;
	apnvm_result		vmrc;

	if (home == NULL) {
		fprintf(stderr, "Could not detect home-directory\n");
		return (0);
	}
	if (user == NULL) {
		fprintf(stderr, "Could not detect username\n");
		return (0);
	}

	nwritten = snprintf(cvsroot, sizeof(cvsroot),
	    "%s/.xanoubis/apnvmroot", home);
	if ((nwritten >= PATH_MAX) || (nwritten < 0)) {
		fprintf(stderr, "Could not build CVSROOT-path\n");
		return (0);
	}

	vm = apnvm_init(cvsroot, user);
	if (vm == NULL) {
		fprintf(stderr, "Failed to initialize version management\n");
		return (0);
	}

	vmrc = apnvm_prepare(vm);
	if (vmrc != APNVM_OK) {
		fprintf(stderr, "Preparation of version management failed");
		apnvm_destroy(vm);
		return (0);
	}

	md.comment = "Automatically created by anoubisctl";
	md.auto_store = 1;

	vmrc = apnvm_insert(vm, user, profile, ruleset, &md);
	if (vmrc != APNVM_OK) {
		fprintf(stderr, "Failed to create a new version\n");
		apnvm_destroy(vm);
		return (0);
	}

	apnvm_destroy(vm);
	return (1);
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
	if (profile == NULL ||
	    (strcmp(profile, "high") && strcmp(profile, "medium") &&
	    strcmp(profile, "admin") && strcmp(profile, "none"))) {
		fprintf(stderr, "Need a profile, one of high, medium, admin\n");
		return 4;
	}
	if (opts & ANOUBISCTL_OPT_VERBOSE)
		flags |= APN_FLAG_VERBOSE;
	if (opts & ANOUBISCTL_OPT_VERBOSE2)
		flags |= APN_FLAG_VERBOSE2;
	if (apn_parse(rulesopt, &ruleset, flags)) {
		if (ruleset)
			apn_print_errors(ruleset, stderr);
		else
			fprintf(stderr, "FATAL: Out of memory\n");

		if ((opts & ANOUBISCTL_OPT_FORCE) == 0) {
			apn_free_ruleset(ruleset);
			return 1;
		}
	}
	if (opts & ANOUBISCTL_OPT_NOACTION) {
		apn_free_ruleset(ruleset);
		return 0;
	}

	error = create_channel();
	if (error) {
		fprintf(stderr, "Cannot connect to anoubisd\n");
		apn_free_ruleset(ruleset);
		return error;
	}

	if (!make_version(ruleset, profile)) {
		apn_free_ruleset(ruleset);
		return 7;
	}

	apn_free_ruleset(ruleset);

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
		fprintf(stderr, "Policy Request failed: %d (%s)\n", t->result,
		    strerror(t->result));
		anoubis_transaction_destroy(t);
		return 3;
	}
	destroy_channel();

	return 0;
}

#define ALLOCLEN	4000

static int monitor(int argc, char **argv)
{
	int				 i, count = 0, error;
	static anoubis_token_t		 nexttok = 1;
	struct anoubis_transaction	*t;

	int maxcount = -1;
	int defaulterror = -1;
	int delegate = 0;
	char ch;

	for(i=0; i<argc; ++i) {
		if (strcasecmp(argv[i], "delegate") == 0) {
			if (delegate)
				usage();
			delegate = 1;
			continue;
		}
		if (strncasecmp(argv[i], "error=", 6) == 0) {
			if (defaulterror >= 0)
				usage();
			if (sscanf(argv[i]+6, "%d%c", &defaulterror, &ch) != 1)
				usage();
			if (defaulterror < 0)
				usage();
			continue;
		}
		if (strncasecmp(argv[i], "count=", 6) == 0) {
			if (maxcount >= 0)
				usage();
			if (sscanf(argv[i]+6, "%d%c", &maxcount, &ch) != 1)
				usage();
			if (maxcount < 0)
				usage();
			continue;
		}
		usage();
	}
	if (maxcount < 0)
		maxcount = 0;
	if (defaulterror < 0)
		defaulterror = 0;

	error = create_channel();
	if (error) {
		fprintf(stderr, "Cannot create channel\n");
		return error;
	}
	t = anoubis_client_register_start(client, ++nexttok, geteuid(), 0, 0);
	if (!t) {
		fprintf(stderr, "Cannot register for notificiations\n");
		destroy_channel();
		return 5;
	}
	while(1) {
		error = anoubis_client_wait(client);
		if (error <= 0) {
			anoubis_transaction_destroy(t);
			destroy_channel();
			return 5;
		}
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}
	if (t->result) {
		fprintf(stderr, "Notify registration failed with %d (%s)\n",
		    t->result, strerror(t->result));
		anoubis_transaction_destroy(t);
		destroy_channel();
		return 5;
	}
	anoubis_transaction_destroy(t);
	while (1) {
		struct anoubis_msg	*m;
		if (anoubis_client_wait(client) <= 0) {
			fprintf(stderr, "message receive failed\n");
			destroy_channel();
			return 5;
		}
		while((m = anoubis_client_getnotify(client))) {
			count++;
			/* __anoubis_dump will verify the message length */
			__anoubis_dump(m, "");
			if (get_value(m->u.notify->type) != ANOUBIS_N_ASK) {
				anoubis_msg_free(m);
				continue;
			}
			error = anoubis_client_notifyreply(client,
			    m->u.notify->token, defaulterror, delegate);
			anoubis_msg_free(m);
			if (error < 0) {
				fprintf(stderr, "notify reply failed "
				    "with code %d", -error);
				destroy_channel();
				return 5;
			}
		}
		if (maxcount > 0 && count >= maxcount)
			break;
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
	struct anoubis_transaction *t;

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
	rc = acc_setaddr(channel, &ss, sizeof(struct sockaddr_un));
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
	t = anoubis_client_sfsdisable_start(client, getpid());
	while(1) {
		int ret = anoubis_client_wait(client);
		if (ret <= 0) {
			anoubis_transaction_destroy(t);
			destroy_channel();
			return 5;
		}
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}
	error = t->result;
	anoubis_transaction_destroy(t);
	if (error) {
		destroy_channel();
		errno = error;
		perror("Cannot disable SFS checks");
		return 5;
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

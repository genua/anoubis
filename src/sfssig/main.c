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

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/un.h>
#include <sys/mman.h>
#include <sys/socket.h>

#ifndef NEEDBSDCOMPAT
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <anoubis_protocol.h>
#include <anoubis_errno.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>
#include <anoubis_dump.h>

#include <anoubischat.h>

#include "csum/csum.h"

#ifdef LINUX
#include <bsdcompat.h>
#include <linux/anoubis.h>
#include <openssl/sha.h>
#include <attr/xattr.h>
#endif
#ifdef OPENBSD
#include <sha2.h>
#include <dev/anoubis.h>
#endif

#ifndef __used
#define __used __attribute__((unused))
#endif

#ifndef __dead
#define __dead __attribute__((noreturn))
#endif



#define SFSSIG_OPT_NOACTION		0x0001
#define SFSSIG_OPT_VERBOSE		0x0002
#define SFSSIG_OPT_VERBOSE2		0x0004

typedef int (*func_int_t)(void);
typedef int (*func_char_t)(char *);

void		 usage(void) __dead;
static int	 sfs_add(char *file);
static int	 sfs_del(char *file);
static int	 sfs_list(char *file);
static int	 sfs_sumop(char *file, int operation, u_int8_t *cs);
static int	 create_channel(void);
static void	 destroy_channel(void);

#define SYSSIGNAME "security.anoubis_syssig"

struct cmd {
	char		*command;
	func_int_t	 func;
	int		 file;
} commands[] = {
	{ "add",   (func_int_t)sfs_add, 1},
	{ "del",   (func_int_t)sfs_del, 1},
	{ "list",  (func_int_t)sfs_list, 1},
};

static struct achat_channel	*channel;
struct anoubis_client		*client;
static int			 opts = 0;
static char			*anoubis_socket = "/var/run/anoubisd.sock";

__dead void
usage(void)
{
	extern char	*__progname;
	unsigned int	i;

	/*
	 * NOTE: Capitalized options are supposed to affect system signatures
	 * NOTE: other options letters will be used for cert signed checksums.
	 */
	fprintf(stderr, "usage: %s [-nv] command [<file>]\n", __progname);
	/* Add System checksum */
	fprintf(stderr, "       %s -A checksum file\n", __progname);
	/* Update or add system checksum according to current file contents */
	fprintf(stderr, "       %s -U file...\n", __progname);
	/* Remove system checksum from files */
	fprintf(stderr, "       %s -R file...\n", __progname);
	/* List system checksum */
	fprintf(stderr, "       %s -L file...\n", __progname);

	for (i = 0; i < sizeof(commands)/sizeof(struct cmd); i++) {
		if (commands[i].file) {
			fprintf(stderr,
			    "       %s %s file...\n", __progname,
			    commands[i].command);
		} else {
			fprintf(stderr,
			    "       %s %s\n", __progname,
			    commands[i].command);
		}
	}
	exit(1);
}

#ifndef OPENBSD
int openregular(char * filename, int * error)
{
	int fd;
	struct stat statbuf;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		(*error) = errno;
		return -1;
	}
	if (fstat(fd, &statbuf) < 0) {
		(*error) = errno;
		close(fd);
		return -1;
	}
	if (!S_ISREG(statbuf.st_mode)) {
		(*error) = EINVAL;
		close(fd);
		return -1;
	}
	return fd;
}


/* Return  file descriptor for use with fstat later on. */
int
opensha256(char * filename, unsigned char md[SHA256_DIGEST_LENGTH],
    int * error)
{
	SHA256_CTX c;
	int fd;
	static char buf[40960];

	fd = openregular(filename, error);
	if (fd < 0)
		return -1;
	SHA256_Init(&c);
	while(1) {
		int ret = read(fd, buf, sizeof(buf));
		if (ret == 0)
			break;
		if (ret < 0) {
			(*error) = errno;
			close(fd);
			return -1;
		}
		SHA256_Update(&c, buf, ret);
	}
	SHA256_Final(md, &c);
	return fd;
}

static int syssig_add(char * file, unsigned char cksum[SHA256_DIGEST_LENGTH])
{
	unsigned char filesum[SHA256_DIGEST_LENGTH];
	int err;

	int fd = opensha256(file, filesum, &err);
	if (fd < 0) {
		errno = err;
		perror(file);
		close(fd);
		return 1;
	}
	if (memcmp(filesum, cksum, SHA256_DIGEST_LENGTH) != 0) {
		fprintf(stderr, "Warning: Signature provided with -A does "
		    "not match file\n");
	}
	if (fsetxattr(fd, SYSSIGNAME, cksum, SHA256_DIGEST_LENGTH, 0) < 0) {
		perror(file);
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}

static int syssig_list(int argc, char * argv[])
{
	int i, ret = 0;
	const char * ok;
	unsigned char csum[SHA256_DIGEST_LENGTH];
	unsigned char fcsum[SHA256_DIGEST_LENGTH];

	for (i=0; i<argc; ++i) {
		int fd, err, j;
		ok = "BAD";
		err = getxattr(argv[i], SYSSIGNAME, csum, SHA256_DIGEST_LENGTH);
		if (err < 0 && errno == ENOATTR)
			continue;
		if (err < 0) {
			perror(argv[i]);
			ret = 1;
			continue;
		}
		if (err != SHA256_DIGEST_LENGTH) {
			fprintf(stderr, "%s: Invalid system checksum\n",
			    argv[i]);
			ret = 1;
			continue;
		}
		fd = opensha256(argv[i], fcsum, &err);
		if (fd < 0) {
			ok = "??";
		} else {
			close(fd);
			if (memcmp(csum, fcsum, SHA256_DIGEST_LENGTH) == 0)
				ok = "ok";
		}
		for (j=0; j<SHA256_DIGEST_LENGTH; ++j)
			printf("%02x", csum[j]);
		printf(" %3s %s\n", ok, argv[i]);
	}
	return ret;
}

static int syssig_update(int argc, char * argv[])
{
	int i, err, ret = 0;
	unsigned char csum[SHA256_DIGEST_LENGTH];

	for (i=0; i<argc; ++i) {
		int fd = opensha256(argv[i], csum, &err);
		if (fd < 0) {
			errno = err;
			perror(argv[i]);
			ret = 1;
			continue;
		}
		if (fsetxattr(fd, SYSSIGNAME, csum,
		    SHA256_DIGEST_LENGTH, 0) < 0) {
			perror(argv[i]);
			ret = 1;
		}
		close(fd);
	}
	return ret;
}

static int syssig_remove(int argc, char * argv[])
{
	int i, ret = 0;
	for (i=0; i<argc; ++i) {
		int err = removexattr(argv[i], SYSSIGNAME);
		if (err < 0 && errno != ENOATTR) {
			perror(argv[i]);
			ret = 1;
		}
	}
	return ret;
}
#else

static int
syssig_remove(int argc __used, char * argv[] __used)
{
	fprintf(stderr, "Not implemented in OpenBSD yet\n");
	return 1;
}

static int
syssig_update(int argc __used, char * argv[] __used)
{
	fprintf(stderr, "Not implemented in OpenBSD yet\n");
	return 1;
}

static int
syssig_list(int argc __used, char * argv[] __used)
{
	fprintf(stderr, "Not implemented in OpenBSD yet\n");
	return 1;
}

static int
syssig_add(char * file __used, unsigned char cksum[SHA256_DIGEST_LENGTH] __used)
{
	fprintf(stderr, "Not implemented in OpenBSD yet\n");
	return 1;
}

#endif /* ifndef OPENBSD */

int str2hash(const char * s, unsigned char dest[SHA256_DIGEST_LENGTH])
{
	unsigned int i;
	char t[3];

	if (strlen(s) != 2*SHA256_DIGEST_LENGTH)
		return -1;
	t[2] = 0;
	for (i=0; i<SHA256_DIGEST_LENGTH; ++i) {
		t[0] = s[2*i];
		t[1] = s[2*i+1];
		if (!isxdigit(t[0]) || !isxdigit(t[1]))
			return -1;
		dest[i] = strtoul(t, NULL, 16);
	}
	return 0;
}

int
main(int argc, char *argv[])
{
	char		 ch;
	char		*argcsumstr = NULL;
	char		*command = NULL;
	char		*rulesopt = NULL;
	unsigned char	 argcsum[SHA256_DIGEST_LENGTH];
	int		 syssigmode = 0;
	int		 done = 0;
	int		 error = 0;
	unsigned int	 i;

	while ((ch = getopt(argc, argv, "A:URLnv")) != -1) {
		switch (ch) {
		case 'n':
			opts |= SFSSIG_OPT_NOACTION;
			break;
		case 'v':
			if (opts & SFSSIG_OPT_VERBOSE)
				opts |= SFSSIG_OPT_VERBOSE2;
			opts |= SFSSIG_OPT_VERBOSE;
			break;
		case 'A':
			argcsumstr = optarg;
		case 'U':
		case 'R':
		case 'L':
			if (syssigmode)
				usage();
			syssigmode = ch;
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

	if (syssigmode != 0) {
		switch(syssigmode) {
		case 'A':
			if (argc > 1)
				usage();
			if (strlen(argcsumstr) != 2 * SHA256_DIGEST_LENGTH)
				usage();
			if (str2hash(argcsumstr, argcsum) < 0)
				usage();
			return syssig_add(argv[0], argcsum);
		case 'R':
			return syssig_remove(argc, argv);
		case 'L':
			return syssig_list(argc, argv);
		case 'U':
			return syssig_update(argc, argv);
		default:
			/* Internal error */
			assert(0);
		}
		return 1;
	}
	command = *argv++;
	argc--;

	if (argc > 0)
		rulesopt = *argv;

	for (i = 0; i < sizeof(commands)/sizeof(struct cmd); i++) {

		if(strcmp(command, commands[i].command) == 0) {

			if (commands[i].file) {

				if (rulesopt == NULL || argc != 1) {
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

	return error;
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
	t = anoubis_client_csumrequest_start(client, operation, file, cs, len);
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
		for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
			printf("%02x", cs[i]);
		printf("\n");
	} else if (operation == ANOUBIS_CHECKSUM_OP_LIST) {
		struct anoubis_msg *m = t->msg;
		int i = 0;
		int cnt = 0;

		/* Print the list of path */
		if (!VERIFY_LENGTH(t->msg,
		    sizeof(Anoubis_ChecksumPayloadMessage))) {
			fprintf(stderr, "Short reply (len=%d)\n",
			    t->msg->length);
			return 6;
		}
		while (m) {
			cnt = m->length - sizeof(Anoubis_ChecksumPayloadMessage)
			    - CSUM_LEN - 1;
			printf("%s/", file);
			for (i = 0; i < cnt; i++) {
				if (m->u.checksumpayload->payload[i] == '\0') {
					printf("\n%s/", file);
					continue;
				}
				printf("%c", m->u.checksumpayload->payload[i]);
			}
			m = m->next;
			printf("\n");
		}
	}
	anoubis_transaction_destroy(t);
	return error;
}

static int
sfs_add(char *file)
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
		fprintf(stderr, " Bad csum length from anoubis_csum_calc\n");
		return 6;
	}
	return sfs_sumop(file, ANOUBIS_CHECKSUM_OP_ADDSUM, cs);
}

static int
sfs_del(char *file)
{
	return sfs_sumop(file, ANOUBIS_CHECKSUM_OP_DEL, NULL);
}

static int
sfs_list(char *file)
{
	int len = strlen(file) - 1;
	if (file[len] == '/')
		file[len] = '\0';

	return sfs_sumop(file, ANOUBIS_CHECKSUM_OP_LIST, NULL);
}

static int
create_channel(void)
{
	achat_rc		rc;
	struct sockaddr_storage ss;
	struct sockaddr_un     *ss_sun = (struct sockaddr_un *)&ss;
	int			error = 0;
	struct anoubis_transaction *t;

	if (opts & SFSSIG_OPT_VERBOSE)
		fprintf(stderr, ">create_channel\n");

	if (opts & SFSSIG_OPT_VERBOSE2)
		fprintf(stderr, "acc_create\n");
	if ((channel = acc_create()) == NULL) {
		fprintf(stderr, "cannot create client channel\n");
		error = 5;
		goto err;
	}
	if (opts & SFSSIG_OPT_VERBOSE2)
		fprintf(stderr, "acc_settail\n");
	if ((rc = acc_settail(channel, ACC_TAIL_CLIENT)) != ACHAT_RC_OK) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "client settail failed\n");
		error = 5;
		goto err;
	}
	if (opts & SFSSIG_OPT_VERBOSE2)
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

	if (opts & SFSSIG_OPT_VERBOSE2)
		fprintf(stderr, "acc_setaddr\n");
	rc = acc_setaddr(channel, &ss, sizeof(struct sockaddr_un));
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel);
	channel = NULL;
		fprintf(stderr, "client setaddr failed\n");
		error = 5;
		goto err;
	}

	if (opts & SFSSIG_OPT_VERBOSE2)
		fprintf(stderr, "acc_prepare\n");
	rc = acc_prepare(channel);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "client prepare failed\n");
		error = 5;
		goto err;
	}

	if (opts & SFSSIG_OPT_VERBOSE2)
		fprintf(stderr, "acc_open\n");
	rc = acc_open(channel);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "client open failed\n");
		error = 5;
		goto err;
	}

	if (opts & SFSSIG_OPT_VERBOSE2)
		fprintf(stderr, "anoubis_client_create\n");
	client = anoubis_client_create(channel);
	if (client == NULL) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "anoubis_client_create failed\n");
		error = 5;
		goto err;
	}

	if (opts & SFSSIG_OPT_VERBOSE2)
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

	if (opts & SFSSIG_OPT_VERBOSE2)
		fprintf(stderr, "anoubis_client_sfsdisable\n");
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
	if (opts & SFSSIG_OPT_VERBOSE)
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

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

#include <limits.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <getopt.h>

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
#include <anoubis_sig.h>

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
#define SFSSIG_OPT_TREE			0x0008
#define SFSSIG_OPT_FILE			0x0010
#define SFSSIG_OPT_DEBUG		0x0020
#define SFSSIG_OPT_DEBUG2		0x0040
#define SFSSIG_OPT_SIG			0x0080
#define SFSSIG_OPT_SUM			0x0100

typedef int (*func_int_t)(void);
typedef int (*func_char_t)(char *);

void		 usage(void) __dead;
static int	 sfs_add(char *file);
static int	 sfs_del(char *file);
static int	 sfs_get(char *file);
static int	 sfs_list(char *file);
static int	 sfs_validate(char *file);
static struct anoubis_transaction *sfs_sumop(char *file, int operation,
    u_int8_t *cs, int cslen, int idlen);
static int	 create_channel(void);
static void	 destroy_channel(void);
static int	 sfs_tree(char *path, int op);
static int	 sfs_add_tree(char *path, int op);
static uid_t	*request_uids(char *file, int *count);
static int	__sfs_get(char *file, int flag);

#define SYSSIGNAME "security.anoubis_syssig"

struct cmd {
	char		*command;
	func_int_t	 func;
	int		 file;
	int		 opt;
} commands[] = {
	{ "add",   (func_int_t)sfs_add, 1, ANOUBIS_CHECKSUM_OP_ADDSUM},
	{ "del",   (func_int_t)sfs_del, 1, ANOUBIS_CHECKSUM_OP_DEL},
	{ "list",  (func_int_t)sfs_list, 1, ANOUBIS_CHECKSUM_OP_LIST},
	{ "get",  (func_int_t)sfs_get, 1, ANOUBIS_CHECKSUM_OP_GET},
	{ "validate",  (func_int_t)sfs_validate, 1, ANOUBIS_CHECKSUM_OP_GET},
};



static struct anoubis_sig	*as = NULL;
static struct achat_channel	*channel;
struct anoubis_client		*client;
static int			 opts = 0;
char				*cert = NULL;
uid_t				 uid = 0;
int				 checksum_flag = ANOUBIS_CSUM_NONE;
static char			*anoubis_socket = "/var/run/anoubisd.sock";
static const char		 def_cert[] = ".xanoubis/default.crt";
static const char		 def_pri[] =  ".xanoubis/private.pem";

__dead void
usage(void)
{
	extern char	*__progname;
	unsigned int	i;

	/*
	 * NOTE: Capitalized options are supposed to affect system signatures
	 * NOTE: other options letters will be used for cert signed checksums.
	 */
	fprintf(stderr, "usage: %s [-nrv] [-f <fileset> ]\n", __progname);
	fprintf(stderr, "       [--sig | --sum] [--cert <certificate>]");
	fprintf(stderr, "       [-k <keyfile>] command [<file>]\n");
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
	unsigned char	 argcsum[SHA256_DIGEST_LENGTH];
	unsigned int	 file_cnt = 0;
	unsigned int	 i = 0, j;
	struct stat	 sb;
	FILE		*fp = NULL;
	char		*argcsumstr = NULL;
	char		*file = NULL;
	char		*keyfile = NULL;
	char		*command = NULL;
	char		*homepath = NULL;
	char		*arg = NULL;
	char		 realarg[PATH_MAX];
	char		**args = NULL;
	char		**tmp = NULL;
	char		 ch;
	char		 fileinput[PATH_MAX];
	int		 syssigmode = 0;
	int		 options_index = 0;
	int		 done = 0;
	int		 error = 0;
	int		 end;

	struct option options [] = {
		{ "sig", 0, 0, 0 },
		{ "sum", 0, 0, 0 },
		{ "cert", 1, 0, 0 },
		{ 0, 0, 0, 0 }
	};

	while ((ch = getopt_long(argc, argv, "A:URLf:k:u:ndvr",
	    options, &options_index)) != -1) {
		switch (ch) {
		case 0:
			if (!strcmp(options[options_index].name, "sig"))
				opts |= SFSSIG_OPT_SIG;
			else if (!strcmp(options[options_index].name, "sum"))
				opts |= SFSSIG_OPT_SUM;
			else if (!strcmp(options[options_index].name, "cert")) {
				cert = optarg;
			} else {
				break;
			}
			break;
		case 'n':
			opts |= SFSSIG_OPT_NOACTION;
			break;
		case 'r':
			opts |= SFSSIG_OPT_TREE;
			break;
		case 'v':
			if (opts & SFSSIG_OPT_VERBOSE)
				opts |= SFSSIG_OPT_VERBOSE2;
			opts |= SFSSIG_OPT_VERBOSE;
			break;
		case 'd':
			if (opts & SFSSIG_OPT_DEBUG)
				opts |= SFSSIG_OPT_DEBUG2;
			opts |= SFSSIG_OPT_DEBUG;
			break;
		case 'k':
			keyfile = optarg;
			break;
		case 'f':
			file = optarg;
			break;
		case 'u':
			if (getuid() != 0) {
				fprintf(stderr,
				    "You need root privilegs to do this.\n");
				return 1;
			}
			if (strcmp(optarg, "all") == 0) {
				checksum_flag = ANOUBIS_CSUM_UID_ALL;
				uid = 0;
			} else {
				checksum_flag = ANOUBIS_CSUM_UID;
				uid = (uid_t)atoi(optarg);
			}
			break;
		case 'A':
			argcsumstr = optarg;
			/* FALLTROUGH */
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

	if (argc <= 0 && !file) {
		usage();
	}

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

	if (file) {
		if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "Input from file handler\n");

		file_cnt = 0;
		i = 0;

		if (strcmp(file, "-") == 0) {
			fp = stdin;
		} else {
			fp = fopen(file, "r");
			if (!fp) {
				perror(file);
				return 1;
			}
		}

		while (fgets(fileinput, PATH_MAX, fp) != NULL) {
			file_cnt++;
			tmp = realloc(args, file_cnt * sizeof(char *));
			if (!tmp) {
				file_cnt--;
				for (i = 0; i < file_cnt; i++)
					free(args[i]);
				free(args);
				perror(file);
				fclose(fp);
				return 1;
			}
			args = tmp;
			args[file_cnt-1] = strdup(fileinput);
			if (!args[file_cnt-1]) {
				for (i = 0; i < file_cnt; i++)
					free(args[i]);
				free(args);
				fclose(fp);
				return 1;
			}
			end = strlen(args[file_cnt-1]) - 1;
			if (args[file_cnt-1][end] == '\n')
				args[file_cnt-1][end] = '\0';

		}
		fclose(fp);
	} else {
		args = argv;
		file_cnt = argc;
	}

	if (opts & SFSSIG_OPT_SIG) {
		if (cert == NULL) {
			homepath = getenv("HOME");
			if (asprintf(&cert, "%s/%s", homepath, def_cert) < 0){
				fprintf(stderr, "Error while allocating"
				    "memory\n");
			}
			if (stat(cert, &sb) != 0) {
				fprintf(stderr, "To apply command to signature,"
				    " you have to specify a certifcate\n");
				return 1;
			}
		}
		/* You also need a keyfile if you want to add a signature */
		if (!strcmp(command, "add") && keyfile == NULL) {
			homepath = getenv("HOME");
			if (asprintf(&keyfile, "%s/%s", homepath, def_pri) < 0){
				fprintf(stderr, "Error while allocating"
				    "memory\n");
			}
			if (stat(keyfile, &sb) != 0) {
				fprintf(stderr, "You have to specify a"
				    " keyfile\n");
				return 1;
			}
		}

		as = anoubis_sig_priv_init(keyfile, cert, NULL, 1);
		if (as == NULL) {
			fprintf(stderr, "Error while loading keyfile: %s\n",
			    keyfile);
			return 1;
		}
	}

	for (i = 0; i < sizeof(commands)/sizeof(struct cmd); i++) {
		if(strcmp(command, commands[i].command) == 0) {
			for (j = 0; j < file_cnt; j++) {
				arg = args[j];
				arg = realpath(arg, realarg);
				if (!arg) {
					perror(args[j]);
					continue;
				}
				if (commands[i].file) {
					if (arg == NULL) {
						fprintf(stderr, "no file\n");
						error = 4;
					} else if (opts & SFSSIG_OPT_TREE) {
						error = sfs_tree(arg,
						    commands[i].opt);
						done = 1;
					} else {
						error = ((func_char_t)
						    commands[i].func)(arg);
						done = 1;
					}
				} else {
					if (arg != NULL) {
						fprintf(stderr,
						    "too many arguments\n");
						error = 4;
					} else {
						error = commands[i].func();
						done = 1;
					}
				}
			}
		}
	}

	if (as)
		anoubis_sig_free(as);

	if (client)
		destroy_channel();

	if (!done)
		usage();

	return error;
}

/* If a signature is delivered to the daemon the signature, the keyid and the
 * checksum will be stored in cs.
 *	---------------------------------------------------------
 *	|	keyid	|	csum	|	sigbuf		|
 *	---------------------------------------------------------
 * idlen is the length of the keyid and cslen is the length of
 * csum + sigbuf.
 *
 * If a signature is requested from daemon, the keyid only is stored in cs,
 * cslen will be 0 and the len of the keyid will be stroed in idlen.
 *	------------------
 *	|	keyid	 |
 *	------------------
 */
static struct anoubis_transaction *
sfs_sumop(char *file, int operation, u_int8_t *cs, int cslen, int idlen)
{
	struct anoubis_transaction	*t;
	int				 error = 0;
	int				 len = 0;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_sumop\n");
	if (!client) {
		error = create_channel();
		if (error) {
			fprintf(stderr, "Cannot connect to anoubis daemon\n");
			return NULL;
		}
	}

	if (cs) {
		if (operation == ANOUBIS_CHECKSUM_OP_ADDSUM)
			len = ANOUBIS_CS_LEN;
		else
			len = cslen;
	}
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "start checksum request\n");
	t = anoubis_client_csumrequest_start(client, operation, file, cs, len,
	    idlen, uid, checksum_flag);
	if (!t) {
		fprintf(stderr, "%s: Cannot send checksum request\n", file);
		return NULL;
	}
	while(1) {
		int ret = anoubis_client_wait(client);
		if (ret <= 0) {
			anoubis_transaction_destroy(t);
			fprintf(stderr, "Checksum request interrupted\n");
			return NULL;
		}
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_sumop\n");
	return t;
}

static uid_t*
request_uids(char *file, int *count)
{
	int ret;
	uid_t *uids;
	int cnt = 0;
	struct stat sb;
	struct anoubis_transaction	*t;
	char				**result;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">request_uids\n");

	if (!file || !count || checksum_flag != ANOUBIS_CSUM_UID_ALL) {
		if (opts & SFSSIG_OPT_DEBUG2)
			fprintf(stderr, "error in requesting uids\n");
		return NULL;
	}

	ret = stat(file, &sb);
	if (ret != 0) {
		perror(file);
		return NULL;
	}
	if (!S_ISREG(sb.st_mode)) {
		return NULL;
	}

	t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_UID_LIST, NULL, 0, 0);
	if (!t)
		return NULL;
	if (t->result) {
		fprintf(stderr, "%s Checksum Request failed: %d (%s)\n", file,
		    t->result, strerror(t->result));
		anoubis_transaction_destroy(t);
		return NULL;
	}

	result = anoubis_csum_list(t->msg, &cnt);
	if (!result || cnt <= 0) {
		if (opts & SFSSIG_OPT_DEBUG2)
			fprintf(stderr, "no result.\n");
		anoubis_transaction_destroy(t);
		return NULL;
	}

	uids = calloc(cnt, sizeof(uid_t));
	if (!uids) {
		perror(file);
		anoubis_transaction_destroy(t);
		return NULL;
	}
	for (ret = 0; ret < cnt; ret++) {
		if (opts & SFSSIG_OPT_DEBUG2)
			fprintf(stderr, "request_uids %d: %s\n", ret,
			    result[ret]);
		uids[ret] = (uid_t)atoi(result[ret]);
	}

	*count = cnt;
	anoubis_transaction_destroy(t);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<request_uids\n");

	return uids;
}

static int
sfs_add(char *file)
{
	struct anoubis_transaction	*t = NULL;
	unsigned int			 siglen = 0, k;
	u_int8_t			 cs[ANOUBIS_CS_LEN];
	u_int8_t			 *sig = NULL;
	u_int8_t			 *msg = NULL;
	uid_t				*result = NULL;
	int				 len = ANOUBIS_CS_LEN;
	int				 ret;
	int				 cnt = 0;
	int				 i;
	int				 error = 0;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_add\n");

	if ((opts & SFSSIG_OPT_SIG)) {
		if ((as == NULL) || (as->pkey == NULL) ||
		    (as->cert == NULL)) {
			fprintf(stderr, "You must specify a keyfile to add a"
			    " signature\n");
			return 1;
		}
	}

	if (!client) {
		error = create_channel();
		if (error) {
			perror("sfs_add");
			return 1;
		}
	}
	ret = anoubis_csum_calc(file, cs, &len);
	if (ret < 0) {
		errno = -ret;
		perror("anoubis_csum_calc");
		return 1;
	}

	if (len != ANOUBIS_CS_LEN) {
		fprintf(stderr, "Bad csum length from anoubis_csum_calc\n");
		return 1;
	}

	if ((opts & SFSSIG_OPT_SIG) && as) {
		sig = anoubis_sign_csum(as, cs, &siglen);
		if (!sig) {
			fprintf(stderr, "Error while anoubis_sign_csum\n");
			return 1;
		}
		if ((msg = calloc((siglen + as->idlen), sizeof(u_int8_t)))
		    == NULL) {
			free(sig);
			return 1;
		}
		memcpy(msg, as->keyid, as->idlen);
		memcpy(msg + as->idlen, sig, siglen);
	}

	if (checksum_flag == ANOUBIS_CSUM_UID_ALL) {
		result = request_uids(file, &cnt);
		if (!result) {
			return 1;
		}
		checksum_flag = ANOUBIS_CSUM_UID;
	} else {
		result = calloc(1, sizeof(uid_t));
		if (!result) {
			perror("sfs_add");
			return 1;
		}
		result[0] = uid;
		cnt = 1;
	}
	for (i = 0; i < cnt; i++) {
		uid = result[i];
		if (opts & SFSSIG_OPT_SIG) {
			t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_ADDSIG, msg,
			    siglen, as->idlen);
		} else {
			t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_ADDSUM, cs,
			    len, 0);
		}
		if (!t)
			return 1;
		if (t->result) {
			fprintf(stderr, "Checksum Request failed: %d (%s)\n",
			t->result, strerror(t->result));
			anoubis_transaction_destroy(t);
			return 1;
		}
	}
	/* Print the checksum that has been added, if verbose output activated*/
	if (opts & SFSSIG_OPT_VERBOSE2)
		printf("add: ");
	if (opts & SFSSIG_OPT_VERBOSE) {
		printf("%s csum:\t", file);
		for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
			printf("%02x", cs[i]);
		printf("\n");
		if (opts & SFSSIG_OPT_SIG) {
			printf("%s signature:\t", file);
			for (k = 0; k < siglen; k++)
				printf("%2.2x", sig[k]);
			printf("\n");
		}
	}

	if (msg)
		free(msg);
	if (result)
		free(result);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_add\n");
	anoubis_transaction_destroy(t);
	return 0;
}

static int
sfs_del(char *file)
{
	struct anoubis_transaction	*t;
	uid_t				*result = NULL;
	int				 cnt = 0;
	int				 i;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_del\n");
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "%s\n", file);

	if (opts & SFSSIG_OPT_SIG) {
		if ((as == NULL) || (as->keyid == NULL)) {
			fprintf(stderr, "You need to specify a\
			    certifcate\n");
			return 1;
		}
	}

	if (checksum_flag == ANOUBIS_CSUM_UID_ALL) {
		result = request_uids(file, &cnt);
		if (!result) {
			return 1;
		}
		checksum_flag = ANOUBIS_CSUM_UID;
	} else {
		result = calloc(1, sizeof(uid_t));
		if (!result) {
			perror("sfs_del");
			return 1;
		}
		result[0] = uid;
		cnt = 1;
	}
	for ( i = 0; i < cnt; i++) {
		uid = result[i];
		if (opts & SFSSIG_OPT_SIG)
			t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_DELSIG,
			    as->keyid, 0, as->idlen);
		else
			t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_DEL, NULL,
			    0, 0);

		if (!t)
			return 1;
		if (t->result) {
			if (t->result == ENOENT) {
				fprintf(stderr, "No entry found for %s\n",
				    file);
				anoubis_transaction_destroy(t);
				return 1;
			}
			fprintf(stderr, "Checksum Request failed: %d (%s)\n",
			t->result, strerror(t->result));
			anoubis_transaction_destroy(t);
			return 1;
		}
	}

	if (result)
		free(result);

	if (opts & SFSSIG_OPT_VERBOSE2)
		printf("del: ");
	if (opts & SFSSIG_OPT_VERBOSE)
		printf("%s\n", file);
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_del\n");

	return 0;
}

static int
sfs_get(char *file)
{
	return __sfs_get(file, 0);
}

static int
sfs_validate(char *file)
{
	return __sfs_get(file, 1);
}

static int
__sfs_get(char *file, int vflag)
{
	struct anoubis_transaction	*t;
	int				 i, j;
	int				 cnt = 0;
	int				 siglen = 0;
	int				 error = 0;
	int				 len = ANOUBIS_CS_LEN;
	u_int8_t			 val_check[ANOUBIS_CS_LEN];
	uid_t				*result = NULL;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_get\n");

	if (opts & SFSSIG_OPT_SIG) {
		if ((as == NULL) || (as->keyid == NULL)) {
			fprintf(stderr, "You need to specify a "
			    "certifcate\n");
			return 1;
		}
	}

	if (vflag) {
		if (!client) {
			error = create_channel();
			if (error) {
				perror("sfs_validate");
				return 1;
			}
		}
		error = anoubis_csum_calc(file, val_check, &len);
		if (error < 0) {
			errno = -error;
			perror("anoubis_csum_calc");
			return 1;
		}

		if (len != ANOUBIS_CS_LEN) {
			fprintf(stderr, "Bad csum length from"
			    "anoubis_csum_calc\n");
			return 1;
		}
	}
	if (checksum_flag == ANOUBIS_CSUM_UID_ALL) {
		result = request_uids(file, &cnt);
		if (!result) {
			return 1;
		}
		checksum_flag = ANOUBIS_CSUM_UID;
	} else {
		result = calloc(1, sizeof(uid_t));
		if (!result) {
			perror("sfs_get");
			return 1;
		}
		result[0] = uid;
		cnt = 1;
	}
	for ( j = 0; j < cnt; j++) {
		if (opts & SFSSIG_OPT_DEBUG2)
			fprintf(stderr, "sfs_get for uid %d\n", result[j]);
		uid = result[j];

		if (opts & SFSSIG_OPT_SIG)
			t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_GETSIG,
			    as->keyid, 0, as->idlen);
		else
			t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_GET, NULL, 0,
			    0);
		if (!t) {
			if (opts & SFSSIG_OPT_DEBUG2)
				fprintf(stderr, "sfs_sumop returned NULL");
			return 1;
		}
		if (t->result) {
			if (t->result == ENOENT) {
				fprintf(stderr, "No entry found for %s\n",
				    file);
				anoubis_transaction_destroy(t);
				return 1;
			}
			fprintf(stderr, "Checksum Request failed: %d (%s)\n",
			    t->result, strerror(t->result));
			anoubis_transaction_destroy(t);
			return 1;
		}
		if (opts & SFSSIG_OPT_SIG) {
			if (!VERIFY_LENGTH(t->msg,
			    sizeof(Anoubis_AckPayloadMessage)
			    + SHA256_DIGEST_LENGTH)) {
				fprintf(stderr,
				    "Short checksum in reply (len=%d)\n",
				    t->msg->length);
				anoubis_transaction_destroy(t);
				return 1;
			}
			if (vflag) {
				if (memcmp(t->msg->u.ackpayload->payload,
				    val_check, ANOUBIS_CS_LEN))
					printf("Signature Missmatch\n");
				else
					printf("Signature Match\n");
				continue;
			}
			siglen = t->msg->length - CSUM_LEN
			    - SHA256_DIGEST_LENGTH
			    - sizeof(Anoubis_AckPayloadMessage);
			printf("%d: %s\t",result[j], file);
			for (i=0; i<SHA256_DIGEST_LENGTH; ++i)
				printf("%02x",
				    t->msg->u.ackpayload->payload[i]);
				printf("\n");

			for (i = SHA256_DIGEST_LENGTH; i
			    <(SHA256_DIGEST_LENGTH + siglen); i++)
				printf("%02x",
				    t->msg->u.ackpayload->payload[i]);
			printf("\n");
		} else {
			if (!VERIFY_LENGTH(t->msg,
			    sizeof(Anoubis_AckPayloadMessage)
			    + SHA256_DIGEST_LENGTH)) {
				fprintf(stderr,
				    "Short checksum in reply (len=%d)\n",
				    t->msg->length);
				anoubis_transaction_destroy(t);
				return 1;
			}
			if (vflag) {
				if (memcmp(t->msg->u.ackpayload->payload,
				    val_check, ANOUBIS_CS_LEN))
					printf("Checksum Missmatch\n");
				else
					printf("Checksum Match\n");
				continue;
			}
			printf("%d: %s\t",result[j], file);
			for (i=0; i<SHA256_DIGEST_LENGTH; ++i)
				printf("%02x",
				    t->msg->u.ackpayload->payload[i]);
			printf("\n");
		}
		anoubis_transaction_destroy(t);
	}

	if (result)
		free(result);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_get\n");

	return 0;
}

static int
sfs_list(char *file)
{
	struct anoubis_transaction	 *t;
	char				**result;
	int				  sfs_cnt = 0;
	int				  len;
	int				  i;
	int				  ret;
	struct stat			  sb;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_list\n");

	if (!file)
		return 1;

	ret = stat(file, &sb);
	if (ret != 0) {
		perror(file);
		return 1;
	}
	if (S_ISREG(sb.st_mode)) {
		return sfs_get(file);
	}

	len = strlen(file) - 1;
	if (len < 0)
		return 1;

	/*
	 * Remove trailing slash but leave the slash if file is the
	 * root-directory.
	 */
	if ((len > 1) && file[len] == '/')
		file[len] = '\0';

	if (opts & SFSSIG_OPT_SIG) {
		if ((as == NULL) || (as->keyid == NULL)) {
			fprintf(stderr, "You need to specify a\
			    certifcate\n");
			return 1;
		}
		t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_SIG_LIST, as->keyid, 0,
		    as->idlen);

	} else {
		t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_LIST, NULL, 0, 0);
	}
	if (!t)
		return 1;

	if (t->result) {
		if (t->result == ENOENT && (opts & SFSSIG_OPT_TREE))
			return 1;

		fprintf(stderr, "%s: Checksum Request failed: %d (%s)\n", file,
		    t->result, strerror(t->result));
		anoubis_transaction_destroy(t);
		return 1;
	}
	result = anoubis_csum_list(t->msg, &sfs_cnt);
	if (!result) {
		fprintf(stderr, "anoubis_csum_list: no result\n");
		anoubis_transaction_destroy(t);
		return 1;
	}

	anoubis_transaction_destroy(t);

	for (i = 0; i < sfs_cnt; i++) {
		len = strlen(result[i]) - 1;
		if (result[i][len] == '/')
			continue;

		/* Avoid double slash at the beginning */
		if (strcmp(file, "/") != 0)
			printf("%s/%s\n", file, result[i]);
		else
			printf("/%s\n", result[i]);
	}

	for (i = 0; i < sfs_cnt; i++)
		free(result[i]);
	free(result);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_list\n");

	return 0;
}

static int
create_channel(void)
{
	struct anoubis_transaction	*t;
	struct sockaddr_storage		 ss;
	struct sockaddr_un		*ss_sun = (struct sockaddr_un *)&ss;
	achat_rc			 rc;
	int				 error = 0;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">create_channel\n");

	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "acc_create\n");
	if ((channel = acc_create()) == NULL) {
		fprintf(stderr, "cannot create client channel\n");
		error = 6;
		goto err;
	}
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "acc_settail\n");
	if ((rc = acc_settail(channel, ACC_TAIL_CLIENT)) != ACHAT_RC_OK) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "client settail failed\n");
		error = 5;
		goto err;
	}
	if (opts & SFSSIG_OPT_DEBUG2)
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

	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "acc_setaddr\n");
	rc = acc_setaddr(channel, &ss, sizeof(struct sockaddr_un));
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel);
	channel = NULL;
		fprintf(stderr, "client setaddr failed\n");
		error = 5;
		goto err;
	}

	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "acc_prepare\n");
	rc = acc_prepare(channel);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "client prepare failed\n");
		error = 5;
		goto err;
	}

	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "acc_open\n");
	rc = acc_open(channel);
	if (rc != ACHAT_RC_OK) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "client open failed\n");
		error = 5;
		goto err;
	}

	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "anoubis_client_create\n");
	client = anoubis_client_create(channel);
	if (client == NULL) {
		acc_destroy(channel);
		channel = NULL;
		fprintf(stderr, "anoubis_client_create failed\n");
		error = 5;
		goto err;
	}

	if (opts & SFSSIG_OPT_DEBUG2)
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

	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "anoubis_client_sfsdisable\n");
	t = anoubis_client_sfsdisable_start(client, getpid());
	while(1) {
		int ret = anoubis_client_wait(client);
		if (ret <= 0) {
			perror("sfsdisable");
			anoubis_transaction_destroy(t);
			return 5;
		}
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}
	error = t->result;
	anoubis_transaction_destroy(t);
	if (error) {
		perror("Cannot disable SFS checks");
		errno = error;
		return 5;
	}
err:
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<create_channel\n");

	return error;
}

static void
destroy_channel(void)
{
	if (client) {
		anoubis_client_close(client);
		anoubis_client_destroy(client);
		client = NULL;
	}
	if (channel)
		acc_destroy(channel);
}

static int
sfs_add_tree(char *path, int op)
{
	struct dirent	*d_ent;
	struct stat	 sb;
	char		*tmp = NULL;
	DIR		*dirp;
	int		 ret = 0;
	int		 len;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_add_tree\n");

	if (!path)
		return 1;

	len = strlen(path) -1;
	if (len < 0)
		return 1;

	/*
	 * Remove trailing slash but leave the slash if file is the
	 * root-directory.
	 */
	if ((len > 1) && path[len] == '/')
		path[len] = '\0';

	dirp = opendir(path);
	if (dirp == NULL) {
		switch(errno) {
		case EACCES:
			return 0;
		case EMFILE:
		case ENOENT:
		case ENOMEM:
		case ENOTDIR:
			return sfs_add(path);
		default:
			return 0;
		}
	}
	while ((d_ent = readdir(dirp)) != NULL) {
		if (!strcmp(d_ent->d_name, ".") ||
		    !strcmp(d_ent->d_name, ".."))
			continue;

		if (asprintf(&tmp, "%s/%s", path,
			    d_ent->d_name) < 0) {
				perror("asprintf");
				ret = 1;
				goto out;
		}

		switch (d_ent->d_type) {
		case DT_DIR:
			ret = sfs_add_tree(tmp, op);
			if (ret != 0) {
				goto out;
			}
			break;
		case DT_REG:
			ret = sfs_add(tmp);
			break;
		case DT_UNKNOWN:
			ret = stat(tmp, &sb);
			if (ret < 0) {
				free(tmp);
				tmp = NULL;
				ret = 0;
				break;
			}
			if (S_ISDIR(sb.st_mode)) {
				ret = sfs_add_tree(tmp, op);
				if (ret != 0) {
					goto out;
				}
			}
			else if (S_ISREG(sb.st_mode)) {
				ret = sfs_add(tmp);
			}
			break;
		default:
			break;
		}
		free(tmp);
		tmp = NULL;
	}
out:
	closedir(dirp);
	if (tmp)
		free(tmp);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_add_tree\n");

	return ret;
}

static int
sfs_tree(char *path, int op)
{
	char		*tmp = NULL;
	char		**result = NULL;
	int		 ret = 0;
	int		 len;
	int		 k = 0;
	int		 j, cnt;
	struct anoubis_transaction *t = NULL;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_tree\n");
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "path: %s\nop: %d\n", path, op);

	if (op == ANOUBIS_CHECKSUM_OP_ADDSUM)
		return sfs_add_tree(path, op);

	if (!path)
		return 1;

	len = strlen(path) -1;
	if (len < 0)
		return 1;

	/*
	 * Remove trailing slash but leave the slash if file is the
	 * root-directory.
	 */
	if ((len > 1) && path[len] == '/')
		path[len] = '\0';

	tmp = strdup(path);
	if (opts & SFSSIG_OPT_SIG) {
		if ((as == NULL) || (as->keyid == NULL)) {
			fprintf(stderr, "You need to specify a\
			    certifcate\n");
			return 1;
		}
		t = sfs_sumop(tmp, ANOUBIS_CHECKSUM_OP_SIG_LIST, as->keyid, 0,
		    as->idlen);
	} else {
		t = sfs_sumop(tmp, ANOUBIS_CHECKSUM_OP_LIST, NULL, 0, 0);
	}
	if (!t)
		return 1;

	if (t->result) {
		if (t->result == ENOENT && (opts & SFSSIG_OPT_TREE))
			return 1;

		fprintf(stderr, "%s: Checksum Request failed: %d (%s)\n", tmp,
		    t->result, strerror(t->result));
		anoubis_transaction_destroy(t);
		return 1;
	}

	result = anoubis_csum_list(t->msg, &k);
	if (!result || k <= 0) {
		ret = 1;
		goto out;
	}

	anoubis_transaction_destroy(t);

	free(tmp);
	tmp = NULL;

	for (j = 0; j < k; j++)
	{
		if (asprintf(&tmp, "%s/%s", path, result[j]) < 0) {
			ret = 1;
			goto out;
		}

		cnt = strlen(result[j]) - 1;
		if (result[j][cnt] == '/') {
			sfs_tree(tmp, op);
			free(tmp);
			tmp = NULL;
			continue;
		}

		switch (op) {
		case ANOUBIS_CHECKSUM_OP_DEL:
			ret = sfs_del(tmp);
			if (ret)
				goto out;
			break;
		case ANOUBIS_CHECKSUM_OP_GET:
			ret = sfs_get(tmp);
			if (ret)
				goto out;
			break;
		case ANOUBIS_CHECKSUM_OP_LIST:
			printf("%s/%s\n", path, result[j]);
			break;
		default:
			return 1;
		}
		free(tmp);
		tmp = NULL;
	}
out:
	if (tmp)
		free(tmp);
	if (result) {
		for (j = 0; j < k; j++)
			free(result[j]);
		free(result);
	}

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_tree\n");

	return ret;
}

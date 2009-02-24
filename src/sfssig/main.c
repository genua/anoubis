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
#include <pwd.h>

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

#define SFSSIG_OPT_NOACTION		0x00001
#define SFSSIG_OPT_VERBOSE		0x00002
#define SFSSIG_OPT_VERBOSE2		0x00004
#define SFSSIG_OPT_TREE			0x00008
#define SFSSIG_OPT_FILE			0x00010
#define SFSSIG_OPT_DEBUG		0x00020
#define SFSSIG_OPT_DEBUG2		0x00040
#define SFSSIG_OPT_SIG			0x00080
#define SFSSIG_OPT_SUM			0x00100
#define SFSSIG_OPT_HASSUM		0x00200
#define SFSSIG_OPT_NOSUM		0x00400
#define SFSSIG_OPT_HASSIG		0x00800
#define SFSSIG_OPT_NOSIG		0x01000
#define SFSSIG_OPT_ORPH			0x02000
#define SFSSIG_OPT_NOTFILE		0x04000
#define SFSSIG_OPT_FILTER		0x08000
#define SFSSIG_OPT_LN			0x10000

#define SYSSIGNAME "security.anoubis_syssig"

void					 usage(void) __dead;
static int				 sfs_add(char *file);
static int				 sfs_del(char *file);
static int				 sfs_get(char *file);
static int				 sfs_list(char *file);
static int				 sfs_validate(char *file);
static int				 sfs_export(char *directory);
static int				 _export(char *directory, int rec);
static int				 create_channel(void);
static void				 destroy_channel(void);
static int				 sfs_tree(char *path, int op);
static int				 sfs_add_tree(char *path, int op);
static uid_t				*request_uids(char *file, int *count);
static int				 __sfs_get(char *file, int flag);
static int				 filter_hassum(char *arg);
static int				 filter_hasnosum(char *arg);
static int				 filter_hassig(char *arg);
static int				 filter_hasnosig(char *arg);
static int				 filter_orphaned(char *arg);
static int				 filter_notfile(char *arg);
static char				**file_input(int *file_cnt, char *file);
static int				 load_keys(int priv_key);
static struct sfs_entry			*get_entry(char *file,
    unsigned char *keyid, int len);
static unsigned char			**request_keyids(char *file, int **ids,
    int *cont);
static char				*build_path(const char *path,
    const char *file);
static char				*sfs_realpath(const char *path,
    char resolved[PATH_MAX]);
static int				 filter_sumsig(char *arg, int op,
    unsigned int idlen, unsigned char *keyid);
static char				**filter_args(int *argc, char **argv,
    char *path);
static struct anoubis_transaction	*sfs_sumop(char *file, int operation,
    u_int8_t *cs, int cslen, int idlen);

typedef int (*func_int_t)(void);
typedef int (*func_char_t)(char *);

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
	{ "export",  (func_int_t)sfs_export, 1, 0},
	{ "validate",  (func_int_t)sfs_validate, 1, ANOUBIS_CHECKSUM_OP_GET},
};

static FILE			*exim_fd = NULL;
static struct anoubis_sig	*as = NULL;
static struct achat_channel	*channel;
struct anoubis_client		*client;
static const char		*def_cert = ".xanoubis/default.crt";
static const char		*def_pri =  ".xanoubis/default.key";
static const char		*anoubis_socket = "/var/run/anoubisd.sock";
static char			*exim_file = NULL;
static char			*cert = NULL;
static char			*keyfile = NULL;
static uid_t			 uid = 0;
static unsigned int		 opts = 0;
static int			 checksum_flag = ANOUBIS_CSUM_NONE;

__dead void
usage(void)
{
	extern char	*__progname;
	unsigned int	i;

	/*
	 * NOTE: Capitalized options are supposed to affect system signatures
	 * NOTE: other options letters will be used for cert signed checksums.
	 */
	fprintf(stderr, "usage: %s [-nlrv] [-f <fileset> ]\n", __progname);
	fprintf(stderr, "	[-o exporttofile] [-i importfromfile]\n");
	fprintf(stderr, "       [--sig | --sum] [--cert <certificate>] \n");
	fprintf(stderr, "       [--hassig | --hasnossig] [--hassum |"
	    " --hasnosum] \n");
	fprintf(stderr, "	[--orphaned | --notfile]\n");
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
	struct passwd	*pw = NULL;
	unsigned int	 i = 0, k;
	char		*argcsumstr = NULL;
	char		*file = NULL;
	char		*command = NULL;
	char		*arg = NULL;
	char		 realarg[PATH_MAX];
	char		 linkpath[PATH_MAX];
	char		*linkfile = NULL;
	char		**args = NULL;
	char		 testarg;
	char		 ch;
	int		 j;
	int		 file_cnt = 0;
	int		 syssigmode = 0;
	int		 ret = 0;
	int		 options_index = 0;
	int		 done = 0;
	int		 error = 0;

	struct option options [] = {
		{ "sig", 0, 0, 0 },
		{ "sum", 0, 0, 0 },
		{ "orphaned", 0, 0, 0 },
		{ "notfile", 0, 0, 0 },
		{ "hassum", 0, 0, 0 },
		{ "hasnosum", 0, 0, 0 },
		{ "hassig", 0, 0, 0 },
		{ "hasnosig", 0, 0, 0 },
		{ "cert", 1, 0, 0 },
		{ 0, 0, 0, 0 }
	};

	while ((ch = getopt_long(argc, argv, "A:URLf:k:u:i:o:nldvr",
	    options, &options_index)) != -1) {
		switch (ch) {
		case 0:
			if (!strcmp(options[options_index].name, "sig"))
				opts |= SFSSIG_OPT_SIG;
			else if (!strcmp(options[options_index].name, "sum"))
				opts |= SFSSIG_OPT_SUM;
			else if (!strcmp(options[options_index].name, "cert")) {
				if (!strcmp(optarg, "all") && (getuid() == 0)) {
					checksum_flag |= ANOUBIS_CSUM_KEY_ALL;
					checksum_flag |= ANOUBIS_CSUM_KEY;
					if (checksum_flag &
					    ANOUBIS_CSUM_UID_ALL)
						checksum_flag |=
						    ANOUBIS_CSUM_ALL;
				}
				else
					cert = optarg;
			} else if (!strcmp(options[options_index].name,
			    "orphaned")) {
				if (opts & SFSSIG_OPT_NOTFILE) {
					fprintf(stderr, "You can use orphaned "
					    "or notfile not both\n");
					usage();
					/* NOTREACHED */
				}
				opts |= SFSSIG_OPT_FILTER;
				opts |= SFSSIG_OPT_ORPH;
			} else if (!strcmp(options[options_index].name,
			    "notfile")) {
				if (opts & SFSSIG_OPT_ORPH) {
					fprintf(stderr, "You can use orphaned "
					    "or notfile not both\n");
					usage();
					/* NOTREACHED */
				}
				opts |= SFSSIG_OPT_FILTER;
				opts |= SFSSIG_OPT_NOTFILE;
			} else if (!strcmp(options[options_index].name,
			    "hassum")) {
				if (opts & SFSSIG_OPT_NOSUM) {
					fprintf(stderr, "You can use hassum "
					    "or hasnosum not both\n");
					usage();
					/* NOTREACHED */
				}
				opts |= SFSSIG_OPT_FILTER;
				opts |= SFSSIG_OPT_HASSUM;
			} else if (!strcmp(options[options_index].name,
			    "hasnosum")) {
				if (opts & SFSSIG_OPT_HASSUM) {
					fprintf(stderr, "You can use hassum "
					    "or hasnosum not both\n");
					usage();
					/* NOTREACHED */
				}
				opts |= SFSSIG_OPT_FILTER;
				opts |= SFSSIG_OPT_NOSUM;
			} else if (!strcmp(options[options_index].name,
			    "hassig")) {
				if (opts & SFSSIG_OPT_NOSIG) {
					fprintf(stderr, "You can use hassig "
					    "or hasnosig not both\n");
					usage();
					/* NOTREACHED */
				}
				opts |= SFSSIG_OPT_FILTER;
				opts |= SFSSIG_OPT_HASSIG;
			} else if (!strcmp(options[options_index].name,
			    "hasnosig")) {
				if (opts & SFSSIG_OPT_HASSIG) {
					fprintf(stderr, "You can use hassig or "
					    "hasnosig not both\n");
					usage();
					/* NOTREACHED */
				}
				opts |= SFSSIG_OPT_FILTER;
				opts |= SFSSIG_OPT_NOSIG;

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
		case 'l':
			opts |= SFSSIG_OPT_LN;
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
		case 'o':
		case 'i':
			exim_file = optarg;
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
				checksum_flag |= ANOUBIS_CSUM_UID_ALL;
				checksum_flag |= ANOUBIS_CSUM_UID;
				if (checksum_flag & ANOUBIS_CSUM_KEY_ALL)
					checksum_flag |= ANOUBIS_CSUM_ALL;
				uid = 0;
			} else {
				checksum_flag |= ANOUBIS_CSUM_UID;
				if (sscanf(optarg, "%u%c", &uid, &testarg)
				    != 1) {
					if ((pw = getpwnam(optarg)) != NULL) {
						uid = pw->pw_uid;
					} else {
						fprintf(stderr, "Unknown "
						    "Username: %s\n", optarg);
						return 1;
					}
				}
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

	if (command == NULL || argc < 0) {
		fprintf(stderr, "No command specified\n");
		usage();
		/* NOTREACHED */
	}

	if (file) {
		args = file_input(&file_cnt, file);
	} else {
		args = argv;
		file_cnt = argc;
	}

	if (opts & SFSSIG_OPT_SIG || opts & SFSSIG_OPT_HASSIG
	|| opts & SFSSIG_OPT_NOSIG) {
		if (!strcmp(command, "add"))
			ret = load_keys(1);
		else
			ret = load_keys(0);
		if (ret)
			return 1;
	}

	if (opts & SFSSIG_OPT_FILTER && !(opts & SFSSIG_OPT_TREE)
	    && strcmp(command, "list")) {
		ret = file_cnt;
		args = filter_args(&ret, args, NULL);
		if (args == NULL && ret != 0) {
			fprintf(stderr, "Error while procceeding filter\n");
			return -ret;
		} else if (args == NULL && ret == 0) {
			if (opts & SFSSIG_OPT_DEBUG2)
				fprintf(stderr, "No result after filter \n");
			return 0;
		}
		file_cnt = ret;
	}
	if ((!strcmp(command, "export")) && (opts & SFSSIG_OPT_TREE))
		opts &= ~SFSSIG_OPT_TREE;

	for (i = 0; i < sizeof(commands)/sizeof(struct cmd); i++) {
		if(strcmp(command, commands[i].command) == 0) {
			for (j = 0; j < file_cnt; j++) {
				arg = args[j];
				if (opts & SFSSIG_OPT_LN) {
					for (k = strlen(arg); k > 0; k--) {
						if (arg[k] == '/')
							break;
					}
					if (k == 0 && arg[0] != '/') {
						linkpath[0] = '.';
						linkpath[1] = '/';
						linkpath[2] = '\0';
						linkfile = arg;
					} else {
						k++;
						strlcpy(linkpath, arg,
						    PATH_MAX);
						linkpath[k] = '\0';
						linkfile = arg + k;
					}
					if (sfs_realpath(linkpath, realarg)
					    == NULL) {
						perror(linkpath);
						return 1;
					}
					if ((arg = build_path(realarg,
					    linkfile)) == NULL)
						return 1;
				} else
					arg = sfs_realpath(arg, realarg);
				if (!arg) {
					perror(args[j]);
					continue;
				}
				if (commands[i].file) {
					if (arg == NULL) {
						fprintf(stderr, "No file "
						    "specified\n");
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
	struct anoubis_transaction	*t;
	char				**result;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">request_uids\n");

	if (!file || !count || !(checksum_flag & ANOUBIS_CSUM_UID_ALL)) {
		if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "internal error in requesting uids\n");
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
	for (ret = 0; ret < cnt; ret++)
		uids[ret] = (uid_t)atoi(result[ret]);

	*count = cnt;
	anoubis_transaction_destroy(t);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<request_uids\n");

	return uids;
}

static unsigned char **
request_keyids(char *file, int **ids, int *count)
{
	int cnt = 0;
	int *idlens = NULL;
	struct anoubis_transaction	*t;
	unsigned char			**result = NULL;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">request_keyids\n");

	if (!file || !count || !(checksum_flag & ANOUBIS_CSUM_KEY_ALL)) {
		if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "internal error in requesting "
			    "keyids\n");
		return NULL;
	}

	t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_KEYID_LIST, NULL, 0, 0);
	if (!t)
		return NULL;
	if (t->result) {
		fprintf(stderr, "%s Checksum Request failed: %d (%s)\n", file,
		    t->result, strerror(t->result));
		anoubis_transaction_destroy(t);
		return NULL;
	}

	result = anoubis_keyid_list(t->msg, &idlens, &cnt);
	if (!result || cnt <= 0) {
		if (opts & SFSSIG_OPT_DEBUG2)
			fprintf(stderr, "no result.\n");
		anoubis_transaction_destroy(t);
		return NULL;
	}
	*count = cnt;
	*ids = idlens;
	anoubis_transaction_destroy(t);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<request_keyids\n");

	return result;
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
	if (opts & SFSSIG_OPT_LN) {
		ret = anoubis_csum_link_calc(file, cs, &len);
	} else {
		ret = anoubis_csum_calc(file, cs, &len);
	}
	if (ret < 0) {
		errno = -ret;
		fprintf(stderr, "%s: ", file);
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
	if (checksum_flag & ANOUBIS_CSUM_UID_ALL) {
		result = request_uids(file, &cnt);
		if (!result) {
			return 1;
		}
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
	if (opts & SFSSIG_OPT_VERBOSE2) {
		printf("add: ");
		printf("%s csum:\t", file);
	}
	if (opts & SFSSIG_OPT_VERBOSE) {
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

	if (checksum_flag & ANOUBIS_CSUM_UID_ALL) {
		result = request_uids(file, &cnt);
		if (!result) {
			return 1;
		}
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
				if (opts & SFSSIG_OPT_VERBOSE)
					fprintf(stderr,
					    "No entry found for %s\n", file);
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
	if (checksum_flag & ANOUBIS_CSUM_UID_ALL) {
		result = request_uids(file, &cnt);
		if (!result) {
			return 1;
		}
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
				if (opts & SFSSIG_OPT_VERBOSE)
					fprintf(stderr, "No entry found for "
					    "%s\n", file);
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

static struct sfs_entry *
get_entry(char *file, unsigned char *keyid_p, int idlen_p)
{
	struct anoubis_transaction	*t_sig = NULL,
					*t_sum = NULL;
	struct sfs_entry		*se = NULL;
	int				siglen = 0,
					idlen = 0;
	unsigned char			*sig = NULL,
					*keyid = NULL,
					*csum = NULL;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">get_entry\n");
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "file: %s\n", file);

	if (keyid_p != NULL) {
		keyid = keyid_p;
		idlen = idlen_p;
	} else if (as) {
		keyid = as->keyid;
		idlen = as->idlen;
	} else {
		keyid = NULL;
		idlen = 0;
	}
	if (keyid) {
		t_sig = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_GETSIG, keyid,
		    0, idlen);
		if (!t_sig) {
			if (opts & SFSSIG_OPT_DEBUG2)
				fprintf(stderr, "sfs_sumop returned NULL");
		} else if (t_sig->result) {
			if (t_sig->result == ENOENT) {
				if (opts & SFSSIG_OPT_VERBOSE)
					fprintf(stderr, "No entry found for "
					     "%s\n", file);
			}
			if (opts & SFSSIG_OPT_DEBUG)
				fprintf(stderr, "Checksum Request failed: %s\n",
				    strerror(t_sig->result));
		} else if (!VERIFY_LENGTH(t_sig->msg,
		    sizeof(Anoubis_AckPayloadMessage)
		    + SHA256_DIGEST_LENGTH)) {
			fprintf(stderr,
			    "Short checksum in reply (len=%d)\n",
			    t_sig->msg->length);
		} else {
			siglen = t_sig->msg->length - CSUM_LEN
			    - SHA256_DIGEST_LENGTH
			    - sizeof(Anoubis_AckPayloadMessage);
			sig = t_sig->msg->u.ackpayload->payload +
			    SHA256_DIGEST_LENGTH;
		}
	}
	t_sum = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_GET, NULL, 0, 0);
	if (!t_sum) {
		if (opts & SFSSIG_OPT_DEBUG2)
			fprintf(stderr, "sfs_sumop returned NULL");
		if (t_sig)
			anoubis_transaction_destroy(t_sig);
		return NULL;
	} else if (t_sum->result) {
		if (t_sum->result == ENOENT) {
			if (opts & SFSSIG_OPT_VERBOSE)
				fprintf(stderr, "No entry found for %s\n",
				    file);
			goto out;
		}
		fprintf(stderr, "Checksum Request failed: %s\n",
		    strerror(t_sum->result));
		goto out;
	} else if (!VERIFY_LENGTH(t_sum->msg,
	    sizeof(Anoubis_AckPayloadMessage)
	    + SHA256_DIGEST_LENGTH)) {
		fprintf(stderr,
		    "Short checksum in reply (len=%d)\n",
		    t_sum->msg->length);
		anoubis_transaction_destroy(t_sum);
		if (t_sig)
			anoubis_transaction_destroy(t_sig);
		return NULL;
	} else {
		csum = t_sum->msg->u.ackpayload->payload;
	}
out:
	if (uid == 0)
		uid = getuid();
	se = anoubis_build_entry(file, csum, SHA256_DIGEST_LENGTH,
	    sig, siglen, uid, keyid, idlen);

	if (t_sum)
		anoubis_transaction_destroy(t_sum);
	if (t_sig)
		anoubis_transaction_destroy(t_sig);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<get_entry\n");

	return (se);
}

static int
sfs_list(char *file)
{
	struct anoubis_transaction	 *t;
	struct dirent			 *d_ent;
	struct stat			  sb;
	char				**result = NULL;
	char				**filter = NULL;
	char				 *tmp = NULL;
	DIR				  *dirp;
	int				  sfs_cnt = 0;
	int				  fil_cnt = 0;
	int				  len;
	int				  i;
	int				  ret;

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

	if (!(opts & SFSSIG_OPT_FILTER) ||
	    (opts & SFSSIG_OPT_NOTFILE) ||
	    (opts & SFSSIG_OPT_ORPH) ||
	    (opts & SFSSIG_OPT_HASSIG) ||
	    (opts & SFSSIG_OPT_HASSUM)) {

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
				fprintf(stderr, "You need to specify a"
				    " certifcate\n");
				return 1;
			}
			t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_SIG_LIST,
			    as->keyid, 0, as->idlen);

		} else {
			t = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_LIST, NULL, 0,
			    0);
		}
		if (!t)
			return 1;

		if (t->result) {
			if (t->result == ENOENT && (opts & SFSSIG_OPT_TREE))
				return 1;

			fprintf(stderr, "%s: Checksum Request failed: %d "
			    "(%s)\n", file, t->result, strerror(t->result));
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
	} else {
		dirp = opendir(file);
		if (!dirp) {
			perror(file);
			return 1;
		}
		while ((d_ent = readdir(dirp)) != NULL) {
			if (!strcmp(d_ent->d_name, ".") ||
			    !strcmp(d_ent->d_name, ".."))
				continue;
			if ((tmp = build_path(file, d_ent->d_name)) == NULL)
				goto err;
			if (opts & SFSSIG_OPT_SIG) {
				if (!filter_hassig(tmp)) {
					free(tmp);
					continue;
				}
			} else {
				if (!filter_hassum(tmp)) {
					free(tmp);
					continue;
				}
			}
			free(tmp);
			sfs_cnt++;
			filter = realloc(result, sfs_cnt * sizeof(char *));
			if (!filter)
				goto err;
			result = filter;
			result[sfs_cnt-1] = strdup(d_ent->d_name);
			if (!result[sfs_cnt-1])
				goto err;

		}
		closedir(dirp);
	}

	if (opts & SFSSIG_OPT_FILTER) {
		fil_cnt = sfs_cnt;
		filter = filter_args(&fil_cnt, result, file);
		if (!filter && fil_cnt < 0) {
			fprintf(stderr, "Error while filter\n");
			goto err;
		}
		for (i = 0; i < sfs_cnt; i++)
			free(result[i]);
		free(result);
		result = filter;
		sfs_cnt = fil_cnt;
	}

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
err:
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
	int		 fil = 1;
	char		 *filter[1];
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

		if ((tmp = build_path(path, d_ent->d_name)) == NULL) {
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
			if (opts & SFSSIG_OPT_FILTER) {
				fil = 1;
				filter[0] = strdup(tmp);
				if (!filter[0])
					return -1;
				filter_args(&fil, filter, NULL);
				if (fil == 0) {
					if (tmp)
						free(tmp);
					tmp = NULL;
					ret = 0;
					break;
				}
			}
			ret = sfs_add(tmp);
			break;
		case DT_UNKNOWN:
			ret = stat(tmp, &sb);
			if (ret < 0) {
				if (tmp)
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
				if (opts & SFSSIG_OPT_FILTER) {
					fil = 1;
					filter[0] = strdup(tmp);
					if (!filter[0])
						return -1;
					filter_args (&fil, filter, NULL);
					if (fil == 0) {
						if (tmp)
							free(tmp);
						tmp = NULL;
						ret = 0;
						break;
					}
				}
				ret = sfs_add(tmp);
			}
			break;
		default:
			break;
		}
		if (tmp)
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
	struct anoubis_transaction *t = NULL;
	struct dirent	*d_ent = NULL;
	struct stat	sb;
	DIR		*dirp = NULL;
	char		*tmp = NULL;
	char		**result = NULL;
	char		**tmpalloc = NULL;
	char		 *filter[1];
	int		 ret = 0;
	int		 len;
	int		 k = 0;
	int		 j, cnt;

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

	if ((op == ANOUBIS_CHECKSUM_OP_DEL) ||
	    (!(opts & SFSSIG_OPT_FILTER)) ||
	    (opts & SFSSIG_OPT_NOTFILE) ||
	    (opts & SFSSIG_OPT_ORPH) ||
	    (opts & SFSSIG_OPT_HASSIG) ||
	    (opts & SFSSIG_OPT_HASSUM)) {

		tmp = strdup(path);

		if (opts & SFSSIG_OPT_SIG) {
			if ((as == NULL) || (as->keyid == NULL)) {
				fprintf(stderr, "You need to specify a\
				    certifcate\n");
				return 1;
			}
			t = sfs_sumop(tmp, ANOUBIS_CHECKSUM_OP_SIG_LIST,
			    as->keyid, 0, as->idlen);
		} else {
			t = sfs_sumop(tmp, ANOUBIS_CHECKSUM_OP_LIST, NULL,
			    0, 0);
		}
		if (!t)
			return 1;

		if (t->result) {
			if (t->result == ENOENT && (opts & SFSSIG_OPT_TREE))
				return 1;

			fprintf(stderr, "%s: Checksum Request failed: %d "
			    "(%s)\n", tmp, t->result, strerror(t->result));
			anoubis_transaction_destroy(t);
			return 1;
		}

		result = anoubis_csum_list(t->msg, &k);
		if (!result || k <= 0) {
			ret = 1;
			goto out;
		}

		anoubis_transaction_destroy(t);
	} else {
		dirp = opendir(path);
		if (!dirp) {
			perror(path);
			return 1;
		}
		while ((d_ent = readdir(dirp)) != NULL) {
			if (!strcmp(d_ent->d_name, ".") ||
			    !strcmp(d_ent->d_name, ".."))
				continue;
			if ((tmp = build_path(path, d_ent->d_name)) == NULL)
				goto out;
			if (opts & SFSSIG_OPT_SIG) {
				if (!filter_hassig(tmp)) {
					free(tmp);
					tmp = NULL;
					continue;
				}
			} else {
				if (!filter_hassum(tmp)) {
					free(tmp);
					tmp = NULL;
					continue;
				}
			}
			ret = 0;
			free(tmp);
			tmp = NULL;

			k++;
			tmpalloc = realloc(result, k * sizeof(char *));
			if (!tmpalloc)
				goto out;
			result = tmpalloc;
			result[k-1] = strdup(d_ent->d_name);
			if (!result[k-1])
				goto out;

		}
		closedir(dirp);
	}
	if (tmp)
		free(tmp);
	tmp = NULL;

	for (j = 0; j < k; j++)
	{
		ret = 0;
		if ((tmp = build_path(path, result[j])) == NULL) {
			ret = 1;
			goto out;
		}

		if (lstat(tmp, &sb) == 0)
			ret = S_ISDIR(sb.st_mode);

		cnt = strlen(result[j]) - 1;
		if (ret || (result[j][cnt] == '/')) {
			sfs_tree(tmp, op);
			free(tmp);
			tmp = NULL;
			continue;
		}
		ret = 0;

		if (opts & SFSSIG_OPT_FILTER) {
			cnt = 1;
			filter[0] = strdup(result[j]);
			filter_args(&cnt, filter, path);
			if (cnt == 0) {
				continue;
			}
			free(filter[0]);
			filter[0] = NULL;
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

static char **
filter_args(int *argc, char **argv, char *path)
{
	int ret, i;
	int cnt = 0;
	char				**result = NULL;
	char				*altpath;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">filter_args\n");

	if (argc == NULL || *argc <= 0)
		return NULL;
	if (argv == NULL) {
		*argc = -1;
		return NULL;
	}

	for (i = 0; i < *argc; i++) {
		ret = 1;

		if (path) {
			if ((altpath = build_path(path, argv[i])) == NULL) {
				*argc = -1;
				return NULL;
			}
		} else {
			 altpath = strdup(argv[i]);
			 if (!altpath) {
				 *argc = -1;
				 return NULL;
			 }
		}
		if (opts & SFSSIG_OPT_ORPH && ret)
			ret = filter_orphaned(altpath);
		if (opts & SFSSIG_OPT_NOTFILE && ret)
			ret = filter_notfile(altpath);
		if (opts & SFSSIG_OPT_HASSUM && ret)
			ret = filter_hassum(altpath);
		if (opts & SFSSIG_OPT_NOSUM && ret)
			ret = filter_hasnosum(altpath);
		if (opts & SFSSIG_OPT_HASSIG && ret)
			ret = filter_hassig(altpath);
		if (opts & SFSSIG_OPT_NOSIG && ret)
			ret = filter_hasnosig(altpath);

		if (ret) {
			cnt++;
			if ((result = realloc(result, cnt * sizeof(char *)))
			    == NULL) {
				*argc = -1;
				return NULL;
			}
			result[cnt-1] = strdup(argv[i]);
			if (!result[cnt-1]) {
				*argc = -1;
				return NULL;
			}
		}

		if (altpath) {
			free(altpath);
			altpath = NULL;
		}
	}

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<filter_args\n");

	*argc = cnt;
	return result;
}

static int
filter_orphaned(char *arg)
{
	struct stat sb;

	if (lstat(arg, &sb) < 0) {
		if (errno == ENOENT)
			return 1;
		else
			return 0;
	}
	if (S_ISREG(sb.st_mode))
		return 0;
	else
		return 1;
}

static int
filter_notfile(char *arg)
{
	struct stat sb;

	if (lstat(arg, &sb) < 0) {
		return 0;
	}
	if (S_ISREG(sb.st_mode))
		return 0;
	else
		return 1;
}

static int
filter_hassum(char *arg)
{
	return filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GET, 0, NULL);
}

static int
filter_hassig(char *arg)
{
	if ((as == NULL) || (as->keyid == NULL)) {
			fprintf(stderr, "You need to specify a "
			    "certifcate\n");
			return 0;
	}
	return filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GETSIG, as->idlen,
	    as->keyid);
}

static int
filter_hasnosum(char *arg)
{
	int rc = 0;
	rc = filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GET, 0, NULL);
	if (rc)
		return 0;
	else
		return 1;
}

static int
filter_hasnosig(char *arg)
{
	int rc = 0;
	if ((as == NULL) || (as->keyid == NULL)) {
			fprintf(stderr, "You need to specify a "
			    "certifcate\n");
			return 0;
	}
	rc =  filter_sumsig(arg, ANOUBIS_CHECKSUM_OP_GETSIG, as->idlen,
	    as->keyid);
	if (rc)
		return 0;
	else
		return 1;
}

static int
filter_sumsig(char *arg, int op, unsigned int idlen, unsigned char *keyid)
{
	struct anoubis_transaction	*t = NULL;
	char				 tmp[PATH_MAX];

	if (sfs_realpath(arg, tmp) == NULL)
		return 0;

	t = sfs_sumop(tmp, op, keyid, 0, idlen);
	if (!t) {
		if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "filter_sumsig: failed transaction\n");
		return 0;
	}
	if (t->result) {
		if (opts & SFSSIG_OPT_VERBOSE2)
			fprintf(stderr, "%s: Has no entry\n", arg);
		anoubis_transaction_destroy(t);
		return 0;
	} else {
		return 1;
	}
}

static char**
file_input(int *file_cnt, char *file)
{
	FILE	 *fp = NULL;
	char	**args = NULL;
	char	**tmp = NULL;
	char	  fileinput[PATH_MAX];
	int	  cnt, i, end;

	if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "Input from file handler\n");
	if (!file_cnt || !file)
		return NULL;

	cnt = 0;
	i = 0;

	if (strcmp(file, "-") == 0) {
		fp = stdin;
	} else {
		fp = fopen(file, "r");
		if (!fp) {
			perror(file);
			return NULL;
		}
	}

	while (fgets(fileinput, PATH_MAX, fp) != NULL) {
		cnt++;
		tmp = realloc(args, cnt * sizeof(char *));
		if (!tmp) {
			cnt--;
			for (i = 0; i < cnt; i++)
				free(args[i]);
			free(args);
			perror(file);
			fclose(fp);
			return NULL;
		}
		args = tmp;
		args[cnt-1] = strdup(fileinput);
		if (!args[cnt-1]) {
			for (i = 0; i < cnt; i++)
				free(args[i]);
			free(args);
			fclose(fp);
			return NULL;
		}
		end = strlen(args[cnt-1]) - 1;
		if (args[cnt-1][end] == '\n')
			args[cnt-1][end] = '\0';
	}
	fclose(fp);

	*file_cnt = cnt;
	return args;
}

/* We need our own Version of realpath since we need to
 * distinguish between filesystem and shadowtree.
 * The usual realpath would not resolv a file
 * which is not in the filesystem anymore.
 */
static char *
sfs_realpath(const char *path, char resolved[PATH_MAX])
{
	struct stat sb;
	char *p, *q, *s;
	size_t left_len, resolved_len;
	unsigned symlinks;
	int serrno, slen, lstat_fail = 0;
	char left[PATH_MAX], next_token[PATH_MAX], symlink[PATH_MAX];

	serrno = errno;
	symlinks = 0;
	if (path[0] == '/') {
		resolved[0] = '/';
		resolved[1] = '\0';
		if (path[1] == '\0')
			return (resolved);
		resolved_len = 1;
		left_len = strlcpy(left, path + 1, sizeof(left));
	} else {
		if (getcwd(resolved, PATH_MAX) == NULL) {
			strlcpy(resolved, ".", PATH_MAX);
			return (NULL);
		}
		resolved_len = strlen(resolved);
		left_len = strlcpy(left, path, sizeof(left));
	}
	if (left_len >= sizeof(left) || resolved_len >= PATH_MAX) {
		errno = ENAMETOOLONG;
		return (NULL);
	}

	/*
	 * Iterate over path components in `left'.
	 */
	while (left_len != 0) {
		/*
		 * Extract the next path component and adjust `left'
		 * and its length.
		 */
		p = strchr(left, '/');
		s = p ? p : left + left_len;
		if ((unsigned)(s - left) >= sizeof(next_token)) {
			errno = ENAMETOOLONG;
			return (NULL);
		}
		memcpy(next_token, left, s - left);
		next_token[s - left] = '\0';
		left_len -= s - left;
		if (p != NULL)
			memmove(left, s + 1, left_len + 1);
		if (resolved[resolved_len - 1] != '/') {
			if (resolved_len + 1 >= PATH_MAX) {
				errno = ENAMETOOLONG;
				return (NULL);
			}
			resolved[resolved_len++] = '/';
			resolved[resolved_len] = '\0';
		}
		if (next_token[0] == '\0')
			continue;
		else if (strcmp(next_token, ".") == 0)
			continue;
		else if (strcmp(next_token, "..") == 0) {
			/*
			 * Strip the last path component except when we have
			 * single "/"
			 */
			if (resolved_len > 1) {
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}
			continue;
		}

		/*
		 * Append the next path component and lstat() it. If
		 * lstat() fails we still can return successfully if
		 * there are no more path components left.
		 */
		resolved_len = strlcat(resolved, next_token, PATH_MAX);
		if (resolved_len >= PATH_MAX) {
			errno = ENAMETOOLONG;
			return (NULL);
		}

		/* If lstat failed once then the resolving path doesn't
		 * exisit in the filesystem, but probably in the
		 * shadowtree. So we don't lstat anymore.
		 */
		if (lstat_fail)
			continue;

		if (lstat(resolved, &sb) != 0) {
			lstat_fail++;
			continue;
		}
		/* If lstat is succsessful and its a link we should
		 * resolving it.
		 */
		if (S_ISLNK(sb.st_mode)) {
			if (symlinks++ > MAXSYMLINKS) {
				errno = ELOOP;
				return (NULL);
			}
			slen = readlink(resolved, symlink, sizeof(symlink) - 1);
			if (slen < 0)
				return (NULL);
			symlink[slen] = '\0';
			if (symlink[0] == '/') {
				resolved[1] = 0;
				resolved_len = 1;
			} else if (resolved_len > 1) {
				/* Strip the last path component. */
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}

			/*
			 * If there are any path components left, then
			 * append them to symlink. The result is placed
			 * in `left'.
			 */
			if (p != NULL) {
				if (symlink[slen - 1] != '/') {
					if ((unsigned)(slen + 1) >=
					    sizeof(symlink)) {
						errno = ENAMETOOLONG;
						return (NULL);
					}
					symlink[slen] = '/';
					symlink[slen + 1] = 0;
				}
				left_len = strlcat(symlink, left, sizeof(left));
				if (left_len >= sizeof(left)) {
					errno = ENAMETOOLONG;
					return (NULL);
				}
			}
			left_len = strlcpy(left, symlink, sizeof(left));
		}
	}

	/*
	 * Remove trailing slash except when the resolved pathname
	 * is a single "/".
	 */
	if (resolved_len > 1 && resolved[resolved_len - 1] == '/')
		resolved[resolved_len - 1] = '\0';
	return (resolved);
}

int
sfs_export(char *arg)
{
	return _export(arg, 0);
}

int
_export(char *arg, int rec)
{
	struct anoubis_transaction	*t = NULL;
	struct sfs_entry		**export = NULL;
	struct sfs_entry		**alloc = NULL;
	struct sfs_entry		*tmp = NULL;
	uid_t				*uid_result = NULL;
	char				*path = NULL;
	char				**result = NULL;
	unsigned char			**keyid_result = NULL;
	unsigned char			*keyid = NULL;
	int				 i, cnt = 0, sfs_cnt = 0;
	int				 j, len, uid_cnt = 0;
	int				 keyid_cnt = 0, idlen = 0;
	int				 *ids = NULL, ret = 0;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_export\n");
	if (arg == NULL)
		return 1;
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "arg: %s\n", arg);
	if (exim_fd == NULL) {
		if (!exim_file) {
			exim_fd = stdout;
		} else {
			exim_fd = fopen(exim_file, "w+");
			if (!exim_fd) {
				perror(exim_file);
				return 1;
			}
		}
	}
	if (!as)
		load_keys(0);
	if (!rec) {
		cnt = 1;
		if ((export = calloc(1, sizeof(struct sfssig_entry *))) == NULL)
			return 1;
		export[0] = get_entry(arg, NULL, 0);
		if (!export[0]) {
			cnt = 0;
			free(export);
			export = NULL;
		}
	}
	if (as != NULL)
		t = sfs_sumop(arg, ANOUBIS_CHECKSUM_OP_LIST_ALL, as->keyid, 0,
		    as->idlen);
	else
		t = sfs_sumop(arg, ANOUBIS_CHECKSUM_OP_LIST_ALL, NULL, 0, 0);
	if (!t)
		return 1;
	if (t->result) {
		if (t->result != ENOENT) {
			if (opts & SFSSIG_OPT_DEBUG)
				fprintf(stderr, "Error while transaction: %s\n",
				    strerror(t->result));
			anoubis_transaction_destroy(t);
			return 1;
		}
	}
	if ((result = anoubis_csum_list(t->msg, &sfs_cnt)) == NULL) {
		if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "anoubis_csum_list: no result\n");
	}
	anoubis_transaction_destroy(t);
	for (i = 0; i < sfs_cnt; i++) {
		if ((path = build_path(arg, result[i])) == NULL)
			goto err;
		len = strlen(result[i]);
		if (result[i][len-1] == '/') {
			ret = _export(path, 1);
			free(path);
			continue;
		}
		if (checksum_flag & ANOUBIS_CSUM_UID_ALL) {
			uid_result = request_uids(path, &uid_cnt);
			if (uid_result) {
				for (j = 0; j < uid_cnt; j++) {
					uid = uid_result[j];
					tmp = get_entry(path, NULL, 0);
					if (!tmp)
						continue;
					cnt++;
					alloc = realloc(export, sizeof(
					    struct sfssig_entry *) * cnt);
					if (alloc == NULL)
						goto err;
					export = alloc;
					export[cnt-1] = tmp;
					tmp = NULL;
				}
			}
		}
		if (checksum_flag & ANOUBIS_CSUM_KEY_ALL) {
			keyid_result = request_keyids(path, &ids, &keyid_cnt);
			if (keyid_result) {
				for (j = 0; j < keyid_cnt; j++) {
					keyid = keyid_result[j];
					idlen = ids[j];
					tmp = get_entry(path, keyid, idlen);
					if (!tmp)
						continue;
					cnt++;
					alloc = realloc(export, sizeof(
					    struct sfssig_entry *) * cnt);
					if (alloc == NULL)
						goto err;
					export = alloc;
					export[cnt-1] = tmp;
					tmp = NULL;
				}
				for(j = 0; j < keyid_cnt; j++)
					free(keyid_result[j]);
				free(keyid_result);
				keyid_result = NULL;
			}
		}
		if (checksum_flag == ANOUBIS_CSUM_NONE) {
			tmp = get_entry(path, NULL, 0);
			if (!tmp)
				continue;
			cnt++;
			alloc = realloc(export, sizeof(struct sfssig_entry *) *
			    cnt);
			if (alloc == NULL)
				goto err;
			export = alloc;
			export[cnt-1] = tmp;
			tmp = NULL;
		}
		free(path);
	}
	if (export) {
		if ((ret = anoubis_print_entries(exim_fd, export, cnt)) != 0) {
			fprintf(stderr, "Error in export entries: %s\n",
			    strerror(ret));
			goto err;
		}
		for (i = 0; i < cnt; i++)
			anoubis_entry_free(export[i]);
		free(export);
	}

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_export\n");

	return ret;
err:
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_export\n");
	if (uid_result)
		free(uid_result);
	if (keyid_result) {
		for(i = 0; i < keyid_cnt; i++)
			free(keyid_result[i]);
		free(keyid_result);
	}
	if (export) {
		for (i = 0; i < cnt; i++)
			anoubis_entry_free(export[i]);
		free(export);
	}
	return 1;
}

static char *
build_path(const char *path, const char *file)
{
	char	*res = NULL;
	int	 ret = 0,
		 len = 0;

	if (!path || !file)
		return NULL;
	len = strlen(path);

	if (path[len-1] == '/')
		ret = asprintf(&res, "%s%s", path, file);
	else
		ret = asprintf(&res, "%s/%s", path, file);
	if (ret < 0)
		perror(path);

	len = strlen(res);
	if ((len > 1) && res[len - 1] == '/')
		res[len - 1] = '\0';
	return res;
}

static int
load_keys(int priv_key)
{
	char		*homepath = NULL;
	int		 error = 0;
	struct stat	 sb;

	if (cert == NULL) {
		homepath = getenv("HOME");
		if ((cert = build_path(homepath, def_cert)) == NULL) {
			fprintf(stderr, "Error while allocating"
			    "memory\n");
		}
		if (stat(cert, &sb) != 0) {
			return 1;
		}
	}
	/* You also need a keyfile if you want to add a signature */
	if (priv_key == 1 && keyfile == NULL) {
		homepath = getenv("HOME");
		if ((keyfile = build_path(homepath, def_pri)) == NULL) {
			fprintf(stderr, "Error while allocating"
			    "memory\n");
		}
		if (stat(keyfile, &sb) != 0) {
			fprintf(stderr, "You have to specify a"
			    " keyfile\n");
			return 1;
		}
	}

	as = anoubis_sig_priv_init(keyfile, cert, NULL, &error);
	if (as == NULL) {
		if (opts &SFSSIG_OPT_VERBOSE)
			fprintf(stderr, "Error while loading keyfile/certfile"
			    "\n");
		return 1;
	}
	return 0;
}

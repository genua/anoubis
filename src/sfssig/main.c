/*
 * Copyright (c) 2008 - 2009 GeNUA mbH <info@genua.de>
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

#include <openssl/rand.h>

#include "sfssig.h"
#include <anoubis_msg.h>
#include <apnvm/apnvm.h>
#include <auth/auth.h>

/* These are the functions to the sfs commandos */
static int	 sfs_add(char *, uid_t);
static int	 sfs_del(char *, uid_t);
static int	 sfs_get(char *, uid_t);
static int	 sfs_list(char *, uid_t);
static int	 sfs_validate(char *, uid_t);
static int	 sfs_export(char *, uid_t);
static int	 sfs_import(char *, uid_t);
static int	 sfs_tree(char *, int op, uid_t);
static int	 sfs_add_tree(char *, int op, uid_t);

/* These are helper functions for the sfs commandos */
static int			 _export(char *, FILE *, int, uid_t);
static int			 __sfs_get(char *, int, uid_t);
static int			 create_channel(void);
static void			 destroy_channel(void);
static int			 add_entry(struct sfs_entry *);
static int			 list(char *arg, uid_t sfs_uid);
static struct sfs_entry		*get_entry(char *, unsigned char *, int, uid_t);
static uid_t			*request_uids(const char *, int *);
static unsigned char		**request_keyids(const char *, int **, int *);
void				 usage(void) __dead;

typedef int (*func_int_t)(void);
typedef int (*func_char_t)(char *, uid_t);

struct cmd {
	char		*command;
	func_int_t	 func;
	int		 opt;
} commands[] = {
	{ "add", (func_int_t)sfs_add, ANOUBIS_CHECKSUM_OP_ADDSUM},
	{ "del", (func_int_t)sfs_del, ANOUBIS_CHECKSUM_OP_DEL},
	{ "list", (func_int_t)sfs_list, ANOUBIS_CHECKSUM_OP_GENERIC_LIST},
	{ "get", (func_int_t)sfs_get, ANOUBIS_CHECKSUM_OP_GET2},
	{ "export", (func_int_t)sfs_export, 0},
	{ "import", (func_int_t)sfs_import, 0},
	{ "validate", (func_int_t)sfs_validate, _ANOUBIS_CHECKSUM_OP_VALIDATE},
};

static struct achat_channel	*channel;
struct anoubis_client		*client;
static const char		*anoubis_socket = PACKAGE_SOCKET;
static char			*out_file = NULL;
static struct anoubis_sig	*as = NULL;
static char			*sfs_cert = NULL;
static char			*sfs_key = NULL;

unsigned int		 opts = 0;
extern char		*__progname;

__dead void
usage(void)
{
	unsigned int	i;

	/*
	 * NOTE: Capitalized options are used for extended attributes
	 * NOTE: other options will be used for signed checksums.
	 */
	fprintf(stderr, "usage: %s [-dvi]\n", __progname);
	fprintf(stderr, "   [-r | --recursive]\n");
	fprintf(stderr, "   [-l | --link]\n");
	fprintf(stderr, "   [-f <listfile> ]\n");
	fprintf(stderr, "   [-o <exportfile>]\n");
	fprintf(stderr, "   [-u | --uid <uid>] \n");
	fprintf(stderr, "   [-k | --key <keyfile>]\n");
	fprintf(stderr, "   [-c | --cert <certificate>] \n");
	fprintf(stderr, "   [--hassum]\n");
	fprintf(stderr, "   [--hasnosum]\n");
	fprintf(stderr, "   [--hassig]\n");
	fprintf(stderr, "   [--hasnossig]\n");
	fprintf(stderr, "   [--orphaned]\n");
	fprintf(stderr, "   [--upgraded]\n");
	fprintf(stderr, "   [--notfile]\n");
	fprintf(stderr, "   [--sum]\n");
	fprintf(stderr, "   [--sig]\n");
	fprintf(stderr, "   command [file...]\n");

	for (i = 0; i < sizeof(commands)/sizeof(struct cmd); i++) {
		fprintf(stderr, "   %s %s file\n", __progname,
		    commands[i].command);
	}

	fprintf(stderr, "commands for extended Attributes:\n");
	/* Add checksum xattr*/
	fprintf(stderr, "   %s -A checksum file\n", __progname);
	/* Update or add checksum xattr matching current file contents */
	fprintf(stderr, "   %s -U file\n", __progname);
	/* Remove checksum xattr from files */
	fprintf(stderr, "   %s -R file\n", __progname);
	/* Show checksum xattr */
	fprintf(stderr, "   %s -L file\n", __progname);
	/* Add skipsum xattr */
	fprintf(stderr, "   %s -S file\n", __progname);
	/* Clear skipsum xattr */
	fprintf(stderr, "   %s -C file\n", __progname);
	exit(1);
}

#define		OPTSIG		256
#define		OPTSUM		257
#define		OPTORPHANED	258
#define		OPTNOTFILE	259
#define		OPTHASSUM	260
#define		OPTNOSUM	261
#define		OPTHASSIG	262
#define		OPTNOSIG	263
#define		OPTUPGRADED	264

static void
set_flag_opt(int opt)
{
	unsigned int		flag;

	switch(opt) {
	case OPTHASSIG:		flag = SFSSIG_OPT_HASSIG; break;
	case OPTNOSIG:		flag = SFSSIG_OPT_NOSIG; break;
	case OPTHASSUM:		flag = SFSSIG_OPT_HASSUM; break;
	case OPTNOSUM:		flag = SFSSIG_OPT_NOSUM; break;
	case OPTNOTFILE:	flag = SFSSIG_OPT_NOTFILE; break;
	case OPTORPHANED:	flag = SFSSIG_OPT_ORPH; break;
	case OPTUPGRADED:	flag = SFSSIG_OPT_UPGRADED; break;
	case OPTSUM:		flag = SFSSIG_OPT_SUM; break;
	case OPTSIG:		flag = SFSSIG_OPT_SIG; break;
	case 'n':		flag = SFSSIG_OPT_NOACTION; break;
	case 'l':		flag = SFSSIG_OPT_LN; break;
	case 'r':		flag = SFSSIG_OPT_TREE; break;
	case 'i':		flag = SFSSIG_OPT_FORCE; break;
	default:
		usage();
	}
	if (opts & flag)
		usage();
	opts |= flag;
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
	uid_t			 sfs_uid = 0;
	struct passwd	*sfs_pw = NULL;
	unsigned char	 sys_argcsum[SHA256_DIGEST_LENGTH];
	unsigned int	 i = 0, k;
	char		*sys_argcsumstr = NULL;
	char		*sfs_infile = NULL;
	char		*sfs_command = NULL;
	char		*sfs_cmdarg = NULL;
	char		*linkfile = NULL;
	char		 linkpath[PATH_MAX];
	char		 realarg[PATH_MAX];
	char		**sfs_argv = NULL;
	int		 ch;
	int		 j;
	int		 syssigmode = 0;
	int		 sfs_argc = 0;
	int		 error = 0;
	int		 done = 0;
	int		 ret = 0;
	int		 got_uid = 0;
	int		 got_cert = 0;
	int		 dofilter = 0;

	struct option options [] = {
		{ "sig", no_argument, NULL, OPTSIG },
		{ "sum", no_argument, NULL, OPTSUM },
		{ "orphaned", no_argument, NULL, OPTORPHANED },
		{ "notfile", no_argument, NULL, OPTNOTFILE },
		{ "hassum", no_argument, NULL, OPTHASSUM },
		{ "hasnosum", no_argument, NULL, OPTNOSUM },
		{ "hassig", no_argument, NULL, OPTHASSIG },
		{ "hasnosig", no_argument, NULL, OPTNOSIG },
		{ "upgraded", no_argument, NULL, OPTUPGRADED },
		{ "link", no_argument, NULL, 'l' },
		{ "recursive", no_argument, NULL, 'r' },
		{ "cert", required_argument, NULL, 'c' },
		{ "key", required_argument, NULL, 'k' },
		{ "uid", required_argument, NULL, 'u' },
		{ 0, 0, 0, 0 }
	};

	while ((ch = getopt_long(argc, argv, "A:URLCSf:c:k:u:o:nlidvr",
	    options, NULL)) != -1) {
		/* No more options are allowed after a syssig option. */
		if (syssigmode > 0)
			usage();
		switch (ch) {
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
		case OPTORPHANED:
		case OPTNOTFILE:
		case OPTHASSUM:
		case OPTNOSUM:
		case OPTHASSIG:
		case OPTNOSIG:
		case OPTUPGRADED:
			set_flag_opt(ch);
			break;
		case OPTSUM:
		case OPTSIG:
		case 'n':
		case 'r':
		case 'i':
		case 'l':
			set_flag_opt(ch);
			break;
		case 'o':
			if (out_file)
				usage();
			out_file = optarg;
			break;
		case 'k':
			if (sfs_key)
				usage();
			sfs_key = optarg;
			break;
		case 'f':
			if (sfs_infile)
				usage();
			sfs_infile = optarg;
			break;
		case 'u':
			if (got_uid)
				usage();
			got_uid = 1;
			if (strcmp(optarg, "all") == 0) {
				opts |= SFSSIG_OPT_ALLUID;
			} else {
				char		ch;
				int		ret;

				ret = sscanf(optarg, "%u%c", &sfs_uid, &ch);
				if (ret != 1) {
					sfs_pw = getpwnam(optarg);
					if (sfs_pw) {
						sfs_uid = sfs_pw->pw_uid;
					} else {
						fprintf(stderr, "Unknown "
						    "Username: %s\n", optarg);
						return 1;
					}
				}
			}
			break;
		case 'c':
			if (got_cert)
				usage();
			got_cert = 1;
			if (strcmp(optarg, "all") == 0) {
				opts |= SFSSIG_OPT_ALLCERT;
			} else {
				sfs_cert = optarg;
			}
			break;
		case 'A':
			sys_argcsumstr = optarg;
			/* FALLTROUGH */
		case 'U':
		case 'R':
		case 'L':
		case 'S':
		case 'C':
			/* Only one syssig options is allowed. */
			if (syssigmode)
				usage();
			syssigmode = ch;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
		/*
		 * We managed to parse an option that is not a syssig
		 * option. This means that syssig options are not allowed
		 * for now on.
		 */
		if (syssigmode == 0)
			syssigmode = -1;
	}

	if ((opts & SFSSIG_OPT_HASSIG) && (opts & SFSSIG_OPT_NOSIG))
		usage();
	if ((opts & SFSSIG_OPT_HASSUM) && (opts & SFSSIG_OPT_NOSUM))
		usage();
	if ((opts & SFSSIG_OPT_NOTFILE) && (opts & SFSSIG_OPT_ORPH))
		usage();
	if ((opts & SFSSIG_OPT_SIG) && (opts & SFSSIG_OPT_SUM))
		usage();
	if (!(opts & SFSSIG_OPT_SIG) && (opts & SFSSIG_OPT_UPGRADED))
		usage();
	if ((opts & SFSSIG_OPT_UPGRADED) && (opts & SFSSIG_OPT_ALLCERT))
		usage();

	argc -= optind;
	argv += optind;

	if (argc <= 0 && sfs_infile == NULL)
		usage();

	openlog(__progname, LOG_ODELAY|LOG_PERROR, LOG_USER);

	if (syssigmode > 0) {
		switch(syssigmode) {
		case 'A':
			if (argc > 1)
				usage();
			if (strlen(sys_argcsumstr) != 2 * SHA256_DIGEST_LENGTH)
				usage();
			if (str2hash(sys_argcsumstr, sys_argcsum) < 0)
				usage();
			return syssig_add(argv[0], sys_argcsum);
		case 'R':
			return syssig_remove(argc, argv);
		case 'L':
			return syssig_list(argc, argv);
		case 'U':
			return syssig_update(argc, argv);
		case 'S':
			return skipsum_update(argv[0], 1);
		case 'C':
			return skipsum_update(argv[0], 0);
		default:
			/* Internal error */
			assert(0);
		}
		return 1;
	}
	sfs_command = *argv++;
	argc--;

	if (anoubis_ui_init() < 0) {
		fprintf(stderr, "Error while initialising anoubis_ui\n");
		return 1;
	}
	error = anoubis_ui_readversion();
	if (error > ANOUBIS_UI_VER) {
		char *home = getenv("HOME");
		syslog(LOG_WARNING,
		    "Unsupported version (%d) of %s/" ANOUBIS_UI_DIR " found.",
		    error, (home)? home:"HOME");
		return 1;
	}
	if (error < 0) {
		fprintf(stderr, "Error occurred while reading version: %s\n",
		    strerror(-error));
		return 1;
	}

	if (sfs_command == NULL || argc < 0) {
		fprintf(stderr, "No command specified\n");
		usage();
		/* NOTREACHED */
	}

	if (!got_uid) {
		sfs_uid = geteuid();
	}
	if (sfs_infile) {
		/* Cannot have command line arguments _and_ an infile via -f */
		if (argc)
			usage();
		sfs_argv = file_input(&sfs_argc, sfs_infile);
		if (sfs_argv == NULL) {
			fprintf(stderr, "Cannot open %s\n", sfs_infile);
		}
	} else {
		sfs_argv = argv;
		sfs_argc = argc;
	}

	/* In case of export we try to load a key quietly */
	if (strcmp(sfs_command, "export") == 0) {
		if (opts & SFSSIG_OPT_TREE)
			opts &= ~SFSSIG_OPT_TREE;
		as = load_keys(0, 0, sfs_cert, sfs_key);
	}

	/* XXX CEH: This is incorrect at least for the --cert all case */
	if (opts & (SFSSIG_OPT_SIG|SFSSIG_OPT_HASSIG|SFSSIG_OPT_NOSIG|
	    SFSSIG_OPT_UPGRADED)) {
		/* We need the private key for 'add' operations only */
		if (strcmp(sfs_command, "add") == 0) {
			as = load_keys(1, 1, sfs_cert, sfs_key);
			if (as == NULL) {
				fprintf(stderr, "Couldn't load private key"
				    " or certificate\n");
				return 1;
			}
		} else {
			as = load_keys(0, 1, sfs_cert, NULL);
			if (as == NULL) {
				fprintf(stderr, "Couldn't load certificate\n");
				return 1;
			}
		}
	}

	/*
	 * If we have to filter for a general command (e.g add)
	 * we do it here.
	 */
	if ((opts & SFSSIG_OPT_FILTER) && !(opts & SFSSIG_OPT_TREE)
	    && strcmp(sfs_command, "list") != 0) {
		dofilter = 1;
	}

	ret = create_channel();
	if (ret != 0) {
		syslog(LOG_WARNING, "Cannot connect to the Anoubis daemon");
		done = 1;
		goto finish;
	}

	for (i = 0; i < sizeof(commands)/sizeof(struct cmd); i++) {
		if(strcmp(sfs_command, commands[i].command) != 0)
			continue;
		for (j = 0; j < sfs_argc; j++) {
			sfs_cmdarg = sfs_argv[j];
			if (dofilter && !filter_one_file(sfs_cmdarg,
			    NULL, sfs_uid, as)) {
				done = 1;
				continue;
			}
			if (opts & SFSSIG_OPT_LN) {
				for (k = strlen(sfs_cmdarg); k > 0; k--) {
					if (sfs_cmdarg[k] == '/')
						break;
				}
				if (k == 0 && sfs_cmdarg[0] != '/') {
					linkpath[0] = '.';
					linkpath[1] = '/';
					linkpath[2] = '\0';
					linkfile = sfs_cmdarg;
				} else {
					k++;
					strlcpy(linkpath, sfs_cmdarg, PATH_MAX);
					linkpath[k] = '\0';
					linkfile = sfs_cmdarg + k;
				}
				if (sfs_realpath(linkpath, realarg) == NULL) {
					perror(linkpath);
					return 1;
				}
				sfs_cmdarg = build_path(realarg, linkfile);
				errno = ENOMEM;
			} else {
				sfs_cmdarg = sfs_realpath(sfs_cmdarg, realarg);
			}
			if (!sfs_cmdarg) {
				perror(sfs_argv[j]);
				continue;
			}
			if (opts & SFSSIG_OPT_TREE &&
			    (strcmp(sfs_command, "list") != 0)) {
				error = sfs_tree(sfs_cmdarg, commands[i].opt,
				    sfs_uid);
				done = 1;
			} else {
				error = ((func_char_t) commands[i].func)(
				    sfs_cmdarg, sfs_uid);
				done = 1;
			}
			if (error > 0)
				ret = error;
		}
	}

finish:

	closelog();

	if (as)
		anoubis_sig_free(as);

	if (client)
		destroy_channel();

	if (done == 0)
		usage();

	return ret;
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
struct anoubis_transaction *
sfs_sumop(const char *file, int operation, u_int8_t *cs, int cslen, int idlen,
    uid_t uid, unsigned int flags)
{
	struct anoubis_transaction	*t;
	int				 len = 0;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_sumop\n");

	if (cs) {
		if (operation == ANOUBIS_CHECKSUM_OP_ADDSUM)
			len = ANOUBIS_CS_LEN;
		else
			len = cslen;
	}
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "start checksum request\n");
	t = anoubis_client_csumrequest_start(client, operation, file, cs, len,
	    idlen, uid, flags);
	if (t == NULL) {
		fprintf(stderr, "%s: Cannot send checksum request\n", file);
		return NULL;
	}
	while(1) {
		int ret = anoubis_client_wait(client);
		if (ret <= 0) {
			anoubis_transaction_destroy(t);
			syslog(LOG_WARNING, "Checksum request interrupted");
			return NULL;
		}
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_sumop\n");
	return t;
}

struct anoubis_transaction *
sfs_listop(const char *file, uid_t uid, struct anoubis_sig *as,
    unsigned int flags)
{
	struct anoubis_transaction	*t;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_listop\n");
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "starting checksum list request: "
		    "file=%s uid=%d as=%p flags=%u\n", file, uid, as, flags);
	t = anoubis_client_csumrequest_start(client,
	    ANOUBIS_CHECKSUM_OP_GENERIC_LIST, file, as?as->keyid:NULL,
	    0, as?as->idlen:0, uid, flags);
	while(1) {
		int ret= anoubis_client_wait(client);
		if (ret <= 0) {
			anoubis_transaction_destroy(t);
			syslog(LOG_WARNING, "Checksum request interrupted");
			return NULL;
		}
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_listop: t=%p\n", t);
	return t;
}

/*
 * NOTE: It is possible that this function returns an emtpy list.
 * NOTE: In this case the return value will still be non-NULL but *count
 * NOTE: will be zero.
 * XXX CEH: The calling conventions for request_uids and request_keys
 * XXX CEH: need unification. The return value should be count or a
 * XXX CEH: negative error in both cases.
 */
static uid_t*
request_uids(const char *file, int *count)
{
	int ret;
	uid_t *uids;
	int cnt = 0;
	struct anoubis_transaction	*t;
	char				**result;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">request_uids\n");

	if (!count) {
		if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "internal error in requesting uids\n");
		return NULL;
	}

	t = sfs_listop(file, 0, NULL, ANOUBIS_CSUM_WANTIDS
	    | ANOUBIS_CSUM_ALL | ANOUBIS_CSUM_UID);
	if (t == NULL)
		return NULL;
	if (t->result) {
		fprintf(stderr, "%s Checksum Request failed: %d (%s)\n", file,
		    t->result, strerror(t->result));
		anoubis_transaction_destroy(t);
		return NULL;
	}

	result = anoubis_csum_list(t->msg, &cnt);
	if (result == NULL && cnt < 0) {
		if (opts & SFSSIG_OPT_DEBUG2)
			fprintf(stderr, "no result.\n");
		anoubis_transaction_destroy(t);
		return NULL;
	}
	/* Make sure that we do not return NULL event if cnt == 0. */
	uids = calloc(cnt ? cnt : 1, sizeof(uid_t));
	if (uids == NULL) {
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

/*
 * NOTE: It is possible that this function returns an empty list.
 * NOTE: In this case result and *ids might be zero.
 * NOTE: *cnt will be less than zero in case of an error and zero in case
 * NOTE: of an empty list.
 * XXX CEH: The calling conventions for request_uids and request_keys
 * XXX CEH: need unification. The return value should be count or a
 * XXX CEH: negative error in both cases.
 */
static unsigned char **
request_keyids(const char *file, int **ids, int *count)
{
	int cnt = 0;
	int *idlens = NULL;
	struct anoubis_transaction	*t;
	unsigned char			**result = NULL;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">request_keyids\n");

	if (*count)
		(*count) = -EINVAL;
	if (ids)
		(*ids) = NULL;
	if (!file || !count || !ids) {
		if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "internal error in requesting "
			    "keyids\n");
		return NULL;
	}

	(*count) = -ENOMEM;
	t = sfs_listop(file, 0, NULL, ANOUBIS_CSUM_WANTIDS | ANOUBIS_CSUM_ALL
	    | ANOUBIS_CSUM_KEY);
	if (t == NULL)
		return NULL;
	(*count) = -t->result;
	if (t->result) {
		fprintf(stderr, "%s Checksum Request failed: %d (%s)\n", file,
		    t->result, strerror(t->result));
		anoubis_transaction_destroy(t);
		return NULL;
	}

	result = anoubis_keyid_list(t->msg, &idlens, &cnt);
	(*count) = cnt;
	if (!result || cnt < 0) {
		if (opts & SFSSIG_OPT_DEBUG2)
			fprintf(stderr, "no result.\n");
		anoubis_transaction_destroy(t);
		return NULL;
	}
	*ids = idlens;
	anoubis_transaction_destroy(t);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<request_keyids\n");

	return result;
}

typedef int (*sumop_callback_t)(const char *, struct anoubis_msg *, uid_t);

static int
sfs_get_callback(const char *file, struct anoubis_msg *m,
    uid_t sfs_uid)
{
	const void		*sigdata, *upcsdata;
	int			 siglen = 0, upcslen = 0, i;

	if (opts & SFSSIG_OPT_SIG) {
		siglen = anoubis_extract_sig_type(m,
		    ANOUBIS_SIG_TYPE_SIG, &sigdata);
		upcslen = anoubis_extract_sig_type(m,
		    ANOUBIS_SIG_TYPE_UPGRADECS, &upcsdata);
	} else {
		siglen = anoubis_extract_sig_type(m,
		    ANOUBIS_SIG_TYPE_CS, &sigdata);
	}
	if (siglen < ANOUBIS_CS_LEN) {
		if (opts & SFSSIG_OPT_VERBOSE)
			fprintf(stderr, "No entry found for %s\n", file);
		return -ENOENT;
	}
	printf("%d: %s\t", sfs_uid, file);
	for (i=0; i<SHA256_DIGEST_LENGTH; ++i)
		printf("%02x", ((u_int8_t *)sigdata)[i]);
	printf("\n");
	if (opts & SFSSIG_OPT_SIG) {
		for (i = SHA256_DIGEST_LENGTH; i < siglen; i++)
			printf("%02x", ((u_int8_t *)sigdata)[i]);
		printf("\n");
		if (upcslen == SHA256_DIGEST_LENGTH) {
			printf("* Upgraded * New Checksum: ");
			for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
				printf("%02x", ((u_int8_t *)upcsdata)[i]);
			printf("\n");
		}
	}
	return 0;
}

static int
sfs_validate_callback(const char *file, struct anoubis_msg *m,
    uid_t sfs_uid __attribute__((unused)))
{
	const void		*sigdata;
	int			 siglen;
	char			*sigtype;
	u_int8_t		 csum[ANOUBIS_CS_LEN];
	int			 len = ANOUBIS_CS_LEN, error = 0;

	if (opts & SFSSIG_OPT_SIG) {
		siglen = anoubis_extract_sig_type(m,
		    ANOUBIS_SIG_TYPE_SIG, &sigdata);
		sigtype = "Signature";
	} else {
		siglen = anoubis_extract_sig_type(m,
		    ANOUBIS_SIG_TYPE_CS, &sigdata);
		sigtype = "Checksum";
	}
	if (siglen < ANOUBIS_CS_LEN)
		return -ENOENT;
	if (opts & SFSSIG_OPT_LN) {
		error = anoubis_csum_link_calc(file, csum, &len);
	} else {
		error = anoubis_csum_calc(file, csum, &len);
	}
	if (error < 0) {
		if (opts & SFSSIG_OPT_DEBUG)
			perror("anoubis_csum_calc");
		return error;
	}
	if (len != ANOUBIS_CS_LEN) {
		fprintf(stderr, "Bad csum length from anoubis_csum_calc\n");
		return -EIO;
	}
	if (memcmp(sigdata, csum, ANOUBIS_CS_LEN) == 0)
		printf("%s: %s Match\n", file, sigtype);
	else
		printf("%s: %s Mismatch\n", file, sigtype);
	return 0;
}

static int
sfs_getnosum_callback(const char *file, struct anoubis_msg *m,
    uid_t sfs_uid __attribute__((unused)))
{
	const void		*sigdata;
	int			 siglen;

	if (opts & SFSSIG_OPT_SIG) {
		siglen = anoubis_extract_sig_type(m,
		    ANOUBIS_SIG_TYPE_SIG, &sigdata);
	} else {
		siglen = anoubis_extract_sig_type(m,
		    ANOUBIS_SIG_TYPE_CS, &sigdata);
	}
	if (siglen < ANOUBIS_CS_LEN)
		return -ENOENT;
	printf("%s\n", file);
	return 0;
}

/**
 * Perform a checksum operation for a single user ID.
 *
 * @param file The target file.
 * @param operation The checksum operation (must not be a signature operation)
 * @param cs The payload for the checksum operation.
 * @param cslen Then length of the checksum operation.
 * @param uid The uid for which to perform the operation.
 * @callback Callback function that will be call with the result message of
 *     the checksum request. Not called in case of an error.
 *
 * @return Zero in case of success or a negative error code in case of an
 *     error.
 */
static int
sfs_csumop_oneuid(const char *file, int operation, u_int8_t *cs,
    int cslen, uid_t uid, sumop_callback_t callback)
{
	struct anoubis_transaction	*t;
	int				 error;

	t =  sfs_sumop(file, operation, cs, cslen, 0 /* idlen */,
	    uid, ANOUBIS_CSUM_UID);
	if (t == NULL)
		return -ENOMEM;
	error = -t->result;
	if (error == 0 && callback) {
		error = (*callback)(file, t->msg, uid);
	}
	anoubis_transaction_destroy(t);
	return error;
}

/**
 * Perform a checksum operation for all uids that have a registered
 * checksum for the given file.
 *
 * @param file The path to the file.
 * @param operation A checksum operation (must not be a signature operation!)
 * @param cs The checksum operation payload or NULL.
 * @param cslen The length of the payload.
 * @callback Callback function that will be call with the result message of
 *     the checksum request. Not called in case of an error.
 *
 * @return Zero on success, a negative error code in case of an error.
 *
 * NOTE: Currently, this function will abort after the first error which
 * NOTE: might not be what you want but this is what happens in the callers
 * NOTE: as of the writing of this function.
 */
static int
sfs_csumop_alluid(const char *file, int operation, u_int8_t *cs, int cslen,
    sumop_callback_t callback)
{
	int				 count, i, error = 0;
	uid_t				*uids;

	uids = request_uids(file, &count);
	if (!uids) {
		fprintf(stderr, "Failed to list uids for %s\n", file);
		return -EIO;
	}
	for (i=0; i<count; ++i) {
		error = sfs_csumop_oneuid(file, operation, cs, cslen, uids[i],
		    callback);
		if (error)
			break;
	}
	free(uids);
	return error;
}

/**
 * Perform a signature operation on a file for a single keyid.
 *
 * @param file The target file.
 * @param operation The signature operation (must not be a checksum operation).
 * @param keyid The keyid for the signature operation.
 * @param idlen The length of the keyid.
 * @param sigdata The signature for ADDSIG requests.
 * @param siglen The length of the signature.
 * @callback Callback function that will be call with the result message of
 *     the checksum request. Not called in case of an error.
 *
 * @return Zero in case of success. A negative error code in case of an
 *     error.
 */
static int
sfs_sigop_onekey(const char *file, int operation, void *keyid, int idlen,
    void *sigdata, int siglen, sumop_callback_t callback)
{
	struct anoubis_transaction	*t;
	int				 error;
	void				*payload;

	payload = malloc(idlen + siglen);
	if (!payload)
		return -ENOMEM;
	if (idlen)
		memcpy(payload, keyid, idlen);
	if (siglen)
		memcpy(payload + idlen, sigdata, siglen);
	t = sfs_sumop(file, operation, payload, siglen, idlen,
	    0 /* uid */, 0 /* flags */);
	free(payload);
	if (t == NULL)
		return -ENOMEM;
	error = -t->result;
	/*
	 * XXX CEH: Using euid here is in fact bogus but this is how the
	 * XXX CEH: code used to work.
	 */
	if (error == 0 && callback)
		error = (*callback)(file, t->msg, geteuid());
	anoubis_transaction_destroy(t);
	return error;
}

/**
 * Perform a signature operation on a file for all keyids that have a
 * registered checksum for the file.
 *
 * @param file The target file.
 * @param operation The signature operation. This must not be a checksum
 *     operation and ADDSIG cannot be supported, either!
 * @callback Callback function that will be call with the result message of
 *     the checksum request. Not called in case of an error.
 *
 * @return Zero in case of succes. A negative error code in case of an
 *     error.
 *
 * NOTE: This function aborts after the first error encountered and does
 * NOTE: not try to continue for rest of the keyids. This might not be
 * NOTE: what we really want but it is at the time of this writing consistent
 * NOTE: with the previously existing callers.
 */
static int
sfs_sigop_allkeys(const char *file, int operation, sumop_callback_t callback)
{
	unsigned char		**keyids = NULL;
	int			 *idlens = NULL;
	int			  count, i, error = 0;

	keyids = request_keyids(file, &idlens, &count);
	if (count < 0) {
		fprintf(stderr, "Failed to list keyids for %s\n", file);
		return -EIO;
	}
	for (i=0; i<count; ++i) {
		error = sfs_sigop_onekey(file, operation, as->keyid,
		    as->idlen, NULL /* sigdata */, 0 /* siglen */, callback);
		if (error)
			break;
	}
	if (keyids) {
		for (i=0; i<count; ++i)
			free(keyids[i]);
		free(keyids);
	}
	if (idlens)
		free(idlens);
	return error;
}

static int
sfs_add(char *file, uid_t sfs_uid)
{
	unsigned int			 siglen = 0, k;
	u_int8_t			 cs[ANOUBIS_CS_LEN];
	u_int8_t			 *sig = NULL;
	int				 len = ANOUBIS_CS_LEN;
	int				 ret;
	int				 i;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_add\n");

	if (opts & SFSSIG_OPT_LN) {
		ret = anoubis_csum_link_calc(file, cs, &len);
	} else {
		ret = anoubis_csum_calc(file, cs, &len);
	}
	if (ret < 0) {
		errno = -ret;
		perror(file);
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
	}
	if ((opts & SFSSIG_OPT_SIG) == 0) {
		if (opts & SFSSIG_OPT_ALLUID) {
			ret = sfs_csumop_alluid(file,
			    ANOUBIS_CHECKSUM_OP_ADDSUM, cs, len, NULL);
		} else {
			ret = sfs_csumop_oneuid(file,
			    ANOUBIS_CHECKSUM_OP_ADDSUM, cs, len, sfs_uid, NULL);
		}
	} else {
		if (opts & SFSSIG_OPT_ALLCERT)
			fprintf(stderr, "WARNING: Will ignore '--cert all "
			    "for add operation");
		ret = sfs_sigop_onekey(file, ANOUBIS_CHECKSUM_OP_ADDSIG,
		    as->keyid, as->idlen, sig, siglen, NULL);
	}
	if (ret < 0) {
		fprintf(stderr, "Checksum request failed: %s\n",
		    strerror(-ret));
		return 1;
	}
	/* Print the checksum that has been added, if verbose output activated*/
	if (opts & SFSSIG_OPT_VERBOSE2)
		printf("add: %s csum:\t", file);
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
	if (sig)
		free(sig);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_add\n");
	return 0;
}

static int
sfs_del(char *file, uid_t sfs_uid)
{
	int	ret;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_del\n");
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "%s\n", file);

	if ((opts & SFSSIG_OPT_SIG) == 0) {
		if (opts & SFSSIG_OPT_ALLUID) {
			ret = sfs_csumop_alluid(file,
			    ANOUBIS_CHECKSUM_OP_DEL, NULL, 0, NULL);
		} else {
			ret = sfs_csumop_oneuid(file,
			    ANOUBIS_CHECKSUM_OP_DEL, NULL, 0, sfs_uid, NULL);
		}
	} else {
		if (opts & SFSSIG_OPT_ALLCERT) {
			ret = sfs_sigop_allkeys(file,
			    ANOUBIS_CHECKSUM_OP_DELSIG, NULL);
		} else {
			ret = sfs_sigop_onekey(file,
			    ANOUBIS_CHECKSUM_OP_DELSIG, as->keyid, as->idlen,
			    NULL /* sigdata */, 0 /* siglen */, NULL);
		}
	}
	if (ret < 0) {
		fprintf(stderr, "Checksum request failed: %s\n",
		    strerror(-ret));
		return 1;
	}
	if (opts & SFSSIG_OPT_VERBOSE2)
		printf("del: ");
	if (opts & SFSSIG_OPT_VERBOSE)
		printf("%s\n", file);
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_del\n");

	return 0;
}

/*
 * We use __sfs_get in different variarions:
 */
#define GET_DEFAULT	0x00	/* A normal get operation */
#define GET_VALIDATE	0x01	/* Validate returned checksums */
#define GET_NOSUM	0x02	/* Print name if in tree */

static int
sfs_get(char *file, uid_t uid)
{
	return __sfs_get(file, GET_DEFAULT, uid);
}

static int
sfs_validate(char *file, uid_t uid)
{
	return __sfs_get(file, GET_VALIDATE, uid);
}

static int
__sfs_get(char *file, int getmode, uid_t sfs_uid)
{
	int				 error = 0;
	sumop_callback_t		 callback = NULL;

	if (opts & SFSSIG_OPT_DEBUG) {
		fprintf(stderr, ">sfs_get\n");
		fprintf(stderr, "path %s\n", file);
	}

	switch (getmode) {
	case GET_DEFAULT:
		callback = sfs_get_callback;
		break;
	case GET_VALIDATE:
		callback = sfs_validate_callback;
		break;
	case GET_NOSUM:
		callback = sfs_getnosum_callback;
		break;
	}
	if ((opts & SFSSIG_OPT_SIG) == 0) {
		if (opts & SFSSIG_OPT_ALLUID) {
			error = sfs_csumop_alluid(file,
			    ANOUBIS_CHECKSUM_OP_GET2,
			    NULL /* csdata */, 0 /* cslen */, callback);
		} else {
			error = sfs_csumop_oneuid(file,
			    ANOUBIS_CHECKSUM_OP_GET2, NULL /* csdata */,
			    0 /* cslen */, sfs_uid, callback);
		}
	} else {
		if (opts & SFSSIG_OPT_ALLCERT) {
			error = sfs_sigop_allkeys(file,
			    ANOUBIS_CHECKSUM_OP_GETSIG2, callback);
		} else {
			error = sfs_sigop_onekey(file,
			    ANOUBIS_CHECKSUM_OP_GETSIG2, as->keyid, as->idlen,
			    NULL /* sigdata */, 0 /* siglen */, callback);
		}
	}
	if (error < 0) {
		if (error == -ENOENT) {
			if (opts & SFSSIG_OPT_VERBOSE) {
				fprintf(stderr, "No entry found for %s\n",
				    file);
			}
		} else {
			fprintf(stderr, "Checksum request failed: %s\n",
			    strerror(-error));
		}
		return 1;
	}
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_get\n");
	return 0;
}

static int
sfs_list(char *arg, uid_t sfs_uid)
{
	int ret = 1;

	/* sfs_get with / produces Invalid Argument */
	if (strcmp(arg, "/") != 0)
		ret = __sfs_get(arg, GET_NOSUM, sfs_uid);

	if (list(arg, sfs_uid) == 0)
		return 0;
	else
		return ret;
}

static int
list(char *arg, uid_t sfs_uid)
{
	struct anoubis_transaction	 *t;
	char				**result = NULL;
	char				 *new_dir = NULL;
	int				  sfs_cnt = 0;
	int				  len;
	uid_t				  thisuid;
	struct anoubis_sig		 *thisas;
	unsigned int			  flags;
	int				  i;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_list\n");

	/* Remove trailing slashes but never the initial slash at idx 0 */
	len = strlen(arg);
	while(len >= 2 && arg[len-1] == '/')
		len--;
	arg[len] = 0;

	if (opts & (SFSSIG_OPT_ALLUID | SFSSIG_OPT_ALLCERT)) {
		/* XXX CEH: We do not support ALL for either uid or key */
		flags = ANOUBIS_CSUM_ALL | ANOUBIS_CSUM_UID | ANOUBIS_CSUM_KEY;
		thisas = NULL;
		thisuid = 0;
	} else if (opts & SFSSIG_OPT_UPGRADED) {
		flags = ANOUBIS_CSUM_KEY | ANOUBIS_CSUM_UPGRADED;
		thisas = as;
		thisuid = 0;
	} else if (opts & SFSSIG_OPT_SIG) {
		flags = ANOUBIS_CSUM_KEY;
		thisas = as;
		thisuid = 0;
	} else {
		flags = ANOUBIS_CSUM_UID;
		thisas = NULL;
		thisuid = sfs_uid;
	}
	t = sfs_listop(arg, thisuid, thisas, flags);
	if (t == NULL)
		return 1;

	if (t->result) {
		if (t->result == ENOENT) {
			anoubis_transaction_destroy(t);
			return 0;
		}
		fprintf(stderr, "%s: Checksum Request failed: %d "
			    "(%s)\n", arg, t->result, strerror(t->result));
		anoubis_transaction_destroy(t);
		return 1;
	}
	result = anoubis_csum_list(t->msg, &sfs_cnt);
	if (!result) {
		if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "anoubis_csum_list: no result\n");
		anoubis_transaction_destroy(t);
		return 0;
	}
	anoubis_transaction_destroy(t);

	for (i = 0; i < sfs_cnt; i++) {
		len = strlen(result[i]) - 1;
		if (opts & SFSSIG_OPT_DEBUG2)
			fprintf(stderr, "sfs_list: got file %s\n", result[i]);
		if (result[i][len] == '/') {
			if (opts & SFSSIG_OPT_TREE) {
				new_dir = build_path(arg,result[i]);
				if (new_dir == NULL)
					return 1;
				list(new_dir, sfs_uid);
				free(new_dir);
			}
			continue;
		}
		if (!filter_one_file(result[i], arg, sfs_uid, as))
			continue;
		/* Avoid double slash at the beginning of root-dir */
		if (strcmp(arg, "/") != 0)
			printf("%s/%s\n", arg, result[i]);
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
sfs_add_tree(char *path, int op, uid_t sfs_uid)
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
			return sfs_add(path, sfs_uid);
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

		if (d_ent->d_type == DT_UNKNOWN) {
			if (stat(tmp, &sb) == 0) {
				if (S_ISREG(sb.st_mode)) {
					d_ent->d_type = DT_REG;
				} else if (S_ISDIR(sb.st_mode)) {
					d_ent->d_type = DT_DIR;
				}
			}
		}
		switch (d_ent->d_type) {
		case DT_DIR:
			ret = sfs_add_tree(tmp, op, sfs_uid);
			if (ret != 0)
				goto out;
			break;
		case DT_REG:
			if (opts & SFSSIG_OPT_FILTER) {
				if (!filter_one_file(tmp, NULL, sfs_uid, as))
					break;
			}
			ret = sfs_add(tmp, sfs_uid);
			break;
		default:
			ret = 0;
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
sfs_tree(char *path, int op, uid_t sfs_uid)
{
	struct anoubis_transaction *t = NULL;
	struct dirent	*d_ent = NULL;
	struct stat	sb;
	DIR		*dirp = NULL;
	char		*tmp = NULL;
	char		**result = NULL;
	char		**tmpalloc = NULL;
	int		 ret = 0;
	int		 len;
	int		 k = 0;
	int		 j, cnt;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_tree\n");
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "path: %s\nop: %d\n", path, op);

	if (op == ANOUBIS_CHECKSUM_OP_ADDSUM)
		return sfs_add_tree(path, op, sfs_uid);

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
			if (opts & SFSSIG_OPT_ALLCERT) {
				t = sfs_listop(tmp, 0, NULL, ANOUBIS_CSUM_ALL
				    | ANOUBIS_CSUM_UID | ANOUBIS_CSUM_KEY);
			} else {
				t = sfs_listop(tmp, 0, as, ANOUBIS_CSUM_KEY);
			}
		} else {
			if (opts & SFSSIG_OPT_ALLUID) {
				t = sfs_listop(tmp, 0, NULL, ANOUBIS_CSUM_ALL
				    | ANOUBIS_CSUM_UID | ANOUBIS_CSUM_KEY);
			} else {
				t = sfs_listop(tmp, sfs_uid, NULL,
				    ANOUBIS_CSUM_UID);
			}
		}
		if (t == NULL)
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
		if (!result && k < 0) {
			ret = 1;
			goto out;
		} else if (!result) {
			ret = 0;
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
				if (!filter_hassig(tmp, as)) {
					free(tmp);
					tmp = NULL;
					continue;
				}
			} else {
				if (!filter_hassum(tmp, sfs_uid)) {
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

	for (j = 0; j < k; j++) {
		ret = 0;
		if ((tmp = build_path(path, result[j])) == NULL) {
			ret = 1;
			goto out;
		}

		if (lstat(tmp, &sb) == 0)
			ret = S_ISDIR(sb.st_mode);

		cnt = strlen(result[j]) - 1;
		if (ret || (result[j][cnt] == '/')) {
			sfs_tree(tmp, op, sfs_uid);
			ret = 0;
			free(tmp);
			tmp = NULL;
			continue;
		}
		ret = 0;

		if (opts & SFSSIG_OPT_FILTER) {
			if (!filter_one_file(result[j], path, sfs_uid, as))
				continue;
		}

		switch (op) {
		case ANOUBIS_CHECKSUM_OP_DEL:
			ret = sfs_del(tmp, sfs_uid);
			if (ret)
				goto out;
			break;
		case ANOUBIS_CHECKSUM_OP_GET2:
			ret = sfs_get(tmp, sfs_uid);
			if (ret)
				goto out;
			break;
		case _ANOUBIS_CHECKSUM_OP_VALIDATE:
			ret = sfs_validate(tmp, sfs_uid);
			if (ret)
				goto out;
			break;
		default:
			if (opts & SFSSIG_OPT_DEBUG)
				fprintf(stderr, "Unknown Operation!\n");
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

int
sfs_export(char *arg, uid_t uid)
{
	static FILE	*out_fd = NULL;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_export\n");
	if (!out_file) {
		out_fd = stdout;
	} else {
		out_fd = fopen(out_file, "w+");
		if (!out_fd) {
			perror(out_file);
			return 1;
		}
	}
	return _export(arg, out_fd, 0, uid);
}

int
_export(char *arg, FILE *out_fd, int rec, uid_t sfs_uid)
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

	if (arg == NULL)
		return 1;
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "arg: %s\n", arg);
	if (!rec && (strcmp(arg, "/") != 0)) {
		cnt = 1;
		if ((export = calloc(1, sizeof(struct sfssig_entry *))) == NULL)
			return 1;
		export[0] = get_entry(arg, NULL, 0, sfs_uid);
		if (!export[0]) {
			cnt = 0;
			free(export);
			export = NULL;
		}
	}
	if (opts & (SFSSIG_OPT_ALLUID | SFSSIG_OPT_ALLCERT)) {
		t = sfs_listop(arg, 0, NULL, ANOUBIS_CSUM_ALL
		    | ANOUBIS_CSUM_UID | ANOUBIS_CSUM_KEY);
	} else if (as != NULL) {
		t = sfs_listop(arg, sfs_uid, as,
		    ANOUBIS_CSUM_UID | ANOUBIS_CSUM_KEY);
	} else {
		t = sfs_listop(arg, sfs_uid, NULL, ANOUBIS_CSUM_UID);
	}
	if (t == NULL)
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
			ret = _export(path, out_fd, 1, sfs_uid);
			free(path);
			continue;
		}
		if (opts & SFSSIG_OPT_ALLUID) {
			uid_result = request_uids(path, &uid_cnt);
			if (uid_result) {
				for (j = 0; j < uid_cnt; j++) {
					sfs_uid = uid_result[j];
					tmp = get_entry(path, NULL, 0, sfs_uid);
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
		if (opts & SFSSIG_OPT_ALLCERT) {
			/*
			 * XXX CEH: keyid_cnt < 0 indicates an error.
			 * XXX CEH: we should probably give some kind of
			 * XXX CEH: message in this case.
			 */
			keyid_result = request_keyids(path, &ids, &keyid_cnt);
			if (keyid_result) {
				for (j = 0; j < keyid_cnt; j++) {
					keyid = keyid_result[j];
					idlen = ids[j];
					tmp = get_entry(path, keyid, idlen,
					    sfs_uid);
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
		if ((opts & (SFSSIG_OPT_ALLUID | SFSSIG_OPT_ALLCERT)) == 0) {
			tmp = get_entry(path, NULL, 0, sfs_uid);
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
		if ((ret = anoubis_print_entries(out_fd, export, cnt)) != 0) {
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

int
sfs_import(char *filename, uid_t sfs_uid __used)
{
	struct sfs_entry *head = NULL;
	struct sfs_entry *next = NULL;
	int rc, res = 0;
	FILE *file;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_import\n");
	if (!filename)
		return 1;

	file = fopen(filename, "r");
	if (!file) {
		perror("filename");
		return 1;
	}
	head = import_csum(file);
	fclose(file);
	if (!head) {
		return 1;
	}
	for (next = head; next; next = next->next) {
		rc = add_entry(next);
		if (rc)
			res = -rc;
	}
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_import\n");
	return res;
}

/**
 * Apply an sfs_entry to the checksum tree.
 * 
 * Signature entries are always applied as is. This will usually fail for
 * foreign signatures though.
 * Checksum entries are applied for the uid given in the entry if
 * the current user is root. Otherwise they are applied to the current uid
 * if either the uid in the entry matches the current uid or if the force flag
 * is given.
 *
 * @param entry The sfs entry.
 * @return Zero if checksum and signature (if present) could be applied,
 *     a negative error code in case of an error.
 *
 * NOTE: As opposed to the original code this function will always try to
 * NOTE: apply the signature even if the checksum could not be applied. A
 * NOTE: combined error value is returned.
 */
static int
add_entry(struct sfs_entry *entry)
{
	uid_t			self, sfs_uid;
	int			cserr = 0, sigerr = 0;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">add_entry\n");
	if (!entry)
		return EINVAL;
	if (entry->checksum) {
		self = geteuid();
		if (self == 0) {
			sfs_uid = entry->uid;
		} else if (entry->uid == self || (opts & SFSSIG_OPT_FORCE)) {
			/*
			 * If the force flag is given and we are not root
			 * ignore the uid and import the checksum as if it
			 * were our own.
			 */
			sfs_uid = self;
		} else {
			fprintf(stderr, "%s: Incorrect uid\n", entry->name);
			cserr = EINVAL;
		}
		if (cserr == 0) {
			cserr = sfs_csumop_oneuid(entry->name,
			    ANOUBIS_CHECKSUM_OP_ADDSUM, entry->checksum,
			    ANOUBIS_CS_LEN, sfs_uid, NULL);
			if (cserr) {
				fprintf(stderr, "Checksum request for "
				    "file=%s uid=%d failed: %s\n",
				    entry->name, entry->uid, strerror(-cserr));
			}
		}
	}
	if (entry->signature) {
		if (!entry->keyid) {
			fprintf(stderr, "Parse Error\n");
			sigerr = EINVAL;
		} else {
			sigerr = sfs_sigop_onekey(entry->name,
			    ANOUBIS_CHECKSUM_OP_ADDSIG, entry->keyid,
			    entry->keylen, entry->signature, entry->siglen,
			    NULL);
			if (sigerr) {
				fprintf(stderr, "Signature request for "
				    "file=%s failed: %s\n", entry->name,
				    strerror(-sigerr));
			}
		}
	}
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<add_entry\n");
	if (cserr)
		return cserr;
	return sigerr;
}

static struct sfs_entry *
get_entry(char *file, unsigned char *keyid_p, int idlen_p, uid_t sfs_uid)
{
	struct anoubis_transaction	*t_sig = NULL,
					*t_sum = NULL;
	struct sfs_entry		*se = NULL;
	int				siglen = 0,
					idlen = 0;
	unsigned char			*keyid = NULL;
	const unsigned char		*sig = NULL, *csum = NULL;

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
		t_sig = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_GETSIG2,
		    keyid, 0 /* siglen */, idlen,
		    0 /* sfs_uid */, 0 /* flags */);
		if (t_sig == NULL) {
			return NULL;
		} else if (t_sig->result) {
			if (t_sig->result == ENOENT) {
				if (opts & SFSSIG_OPT_VERBOSE)
					fprintf(stderr, "No entry found for "
					     "%s\n", file);
			}
			if (opts & SFSSIG_OPT_DEBUG)
				fprintf(stderr, "Checksum Request failed: %s\n",
				    strerror(t_sig->result));
		} else {
			const void	*sigdata = NULL;

			siglen = anoubis_extract_sig_type(t_sig->msg,
			    ANOUBIS_SIG_TYPE_SIG, &sigdata);
			if (siglen == 0) {
				if (opts & SFSSIG_OPT_VERBOSE)
					fprintf(stderr, "No entry found for "
					    "%s\n", file);
			} else if (siglen <= ANOUBIS_CS_LEN) {
				fprintf(stderr, "Bad checksum in reply "
				    "(len=%d)\n", siglen);
				anoubis_transaction_destroy(t_sig);
				return NULL;
			} else {
				sig = sigdata;
			}
		}
	}
	t_sum = sfs_sumop(file, ANOUBIS_CHECKSUM_OP_GET2,
	    NULL /* payload */, 0 /* siglen */, 0 /* idlen */,
	    sfs_uid, ANOUBIS_CSUM_UID);
	if (t_sum == NULL) {
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
	} else {
		int		 cslen;
		const void	*csdata = NULL;

		cslen = anoubis_extract_sig_type(t_sum->msg,
		    ANOUBIS_SIG_TYPE_CS, &csdata);
		if (cslen == 0) {
			if (opts & SFSSIG_OPT_VERBOSE)
				fprintf(stderr, "No entry found for %s\n",
				    file);
			goto out;
		}
		if (cslen != ANOUBIS_CS_LEN) {
			fprintf(stderr, "Bad checksum in reply (len=%d)\n",
			    cslen);
			anoubis_transaction_destroy(t_sum);
			if (t_sig)
				anoubis_transaction_destroy(t_sig);
			return NULL;
		}
		csum = csdata;
	}
out:
	if (sfs_uid == 0)
		sfs_uid = geteuid();
	se = anoubis_build_entry(file, csum, SHA256_DIGEST_LENGTH,
	    sig, siglen, sfs_uid, keyid, idlen);

	if (t_sum)
		anoubis_transaction_destroy(t_sum);
	if (t_sig)
		anoubis_transaction_destroy(t_sig);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<get_entry\n");

	return (se);
}

static int
auth_callback(struct anoubis_client *client __used, struct anoubis_msg *in,
    struct anoubis_msg **outp)
{
	struct anoubis_sig	*as_auth;
	int			need_priv;
	int			rc	= -1;
	int			flags	=  0;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">auth_callback\n");

	/*
	 * Decide if you need to load the private key. If the key is already
	 * loaded, don't do it again because then the user is asked twice for
	 * the passphrase.
	 */
	need_priv = (as == NULL) || (as->pkey == NULL);
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "need_priv = %i\n", need_priv);

	/*
	 * Do not touch global anoubis_sig-structure! The variable as is used
	 * to decide if the keyid should be passed to the daemon. If you
	 * alter the structure here, the keyid can be passed to the daemon
	 * and the message might be rejected.
	 */
	as_auth = load_keys(need_priv, 1, sfs_cert, sfs_key);
	if (as_auth == NULL || as_auth->cert == NULL ||
	    (need_priv && as_auth->pkey == NULL)) {
		fprintf(stderr, "Error while loading cert/key required"
		    " for key based authentication\n");
		return -EPERM;
	}

	if ((opts & SFSSIG_OPT_FORCE) != 0) {
		flags = ANOUBIS_AUTHFLAG_IGN_KEY_MISMATCH;
	}

	rc = anoubis_auth_callback(need_priv ? as_auth : as, as_auth,
	    in, outp, flags);

	switch (-rc) {
	case 0:
		/* Success. Anything is fine. */
		break;
	case ANOUBIS_AUTHERR_INVAL:
		fprintf(stderr,
		    "Invalid argument passed to authentication function\n");
		rc = -EPERM;
		break;
	case ANOUBIS_AUTHERR_PKG:
		fprintf(stderr, "Invalid authentication message received\n");
		rc = -EFAULT;
		break;
	case ANOUBIS_AUTHERR_KEY:
		fprintf(stderr,
		    "No private key available for authentication.\n");
		rc = -EPERM;
		break;
	case ANOUBIS_AUTHERR_CERT:
		fprintf(stderr,
		    "No certificate available for authentication.\n");
		rc = -EPERM;
		break;
	case ANOUBIS_AUTHERR_KEY_MISMATCH:
		fprintf(stderr, "The daemon key and the key used by sfssig"
		    " do not match\n");
		rc = -EPERM;
		break;
	case ANOUBIS_AUTHERR_RAND:
		fprintf(stderr, "No entropy available.\n");
		rc = -EPERM;
		break;
	case ANOUBIS_AUTHERR_NOMEM:
		fprintf(stderr, "Out of memory during authentication\n");
		rc = -ENOMEM;
		break;
	case ANOUBIS_AUTHERR_SIGN:
		fprintf(stderr, "Signing failed during authentication\n");
		rc = -EPERM;
		break;
	default:
		fprintf(stderr,
		    "An unknown error (%i) occurred during authentication\n",
		    -rc);
		rc = -EINVAL;
		break;
	}

	if (need_priv && as != NULL) {
		/* You can reuse the private key */
		as->pkey = as_auth->pkey;
		as_auth->pkey = NULL;
	}

	anoubis_sig_free(as_auth);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<auth_callback\n");

	return (rc);
}

static int
create_channel(void)
{
	struct sockaddr_un		 ss;
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
	ss.sun_family = AF_UNIX;

	strlcpy((&ss)->sun_path, anoubis_socket,
	    sizeof((&ss)->sun_path));

	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "acc_setaddr\n");
	rc = acc_setaddr(channel,(struct sockaddr_storage *)&ss,
	    sizeof(struct sockaddr_un));
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
	client = anoubis_client_create(channel, ANOUBIS_AUTH_TRANSPORTANDKEY,
	    &auth_callback);
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
		if (error == - EPROTONOSUPPORT)
			syslog(LOG_WARNING, "Anoubis protocol mismatch: "
			    "local: %d (min %d) -- daemon: %d (min %d)",
			    ANOUBIS_PROTO_VERSION, ANOUBIS_PROTO_MINVERSION,
			    anoubis_client_serverversion(client),
			    anoubis_client_serverminversion(client));
		anoubis_client_destroy(client);
		client = NULL;
		acc_destroy(channel);
		channel = NULL;
		errno = -error;
		perror("anoubis_client_connect");
		error = 5;
		goto err;
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

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
#include <anoubis_apnvm.h>
#include <anoubis_auth.h>
#include <anoubis_errno.h>

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
static int	 _export(char *, FILE *, int, uid_t);
static int	 __sfs_get(char *, uid_t, sumop_callback_t);
static int	 create_channel(void);
static void	 destroy_channel(void);
static int	 add_entry(struct sfs_entry *);
static int	 list(char *arg, uid_t sfs_uid);
static int	 get_entry(char *, unsigned char *, int, uid_t);
void		 usage(void) __dead;
static int	 process_a_request(struct sfs_request_node *n);
static int	 filter_file_list(int, char **, uid_t, char *);

static int filter_result(struct sfs_request_node *, int, char **, char *);

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
static FILE			*out_fd = NULL;
static struct anoubis_sig	*as = NULL;
static char			*sfs_cert = NULL;
static char			*sfs_key = NULL;
static struct sfs_request_tree	*req_tree = NULL;
static int			 request_error = 0;

struct sfs_request_tree	*filter_tree = NULL;
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
	struct passwd		*sfs_pw = NULL;
	struct sfs_request_node	*node	= NULL;

	unsigned char	 sys_argcsum[SHA256_DIGEST_LENGTH];
	unsigned int	 i = 0, k;
	char		*sys_argcsumstr = NULL;
	char		*sfs_infile = NULL;
	char		*sfs_command = NULL;
	char		*sfs_cmdarg = NULL;
	char		*linkfile = NULL;
	char		 linkpath[PATH_MAX];
	char		 realarg[PATH_MAX];
	char		**tmp_argv = NULL;
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

	while ((ch = getopt_long(argc, argv, "+A:URLCSf:c:k:u:o:nlidvr",
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
			/* Only one syssig option is allowed. */
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
		    anoubis_strerror(-error));
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
		tmp_argv = file_input(&sfs_argc, sfs_infile);
		if (tmp_argv == NULL) {
			fprintf(stderr, "Cannot open %s\n", sfs_infile);
		}
	} else {
		tmp_argv = argv;
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

	req_tree = sfs_init_request_tree();
	if (req_tree == NULL) {
		syslog(LOG_WARNING, "Cannot initilize request tree\n");
		done = 1;
		goto finish;
	}

	filter_tree = sfs_init_request_tree();
	if (filter_tree == NULL) {
		syslog(LOG_WARNING, "Cannot initilize filter tree\n");
		done = 1;
		goto finish;
	}

	/*
	 * To avoid the cost of calling sfs_realpath multiple times
	 * we do it once... NOW.
	 */
	sfs_argv = calloc(sfs_argc, sizeof(char *));
	if (sfs_argv == NULL) {
		fprintf(stderr, "%s", anoubis_strerror(ENOMEM));
		return 1;
	}
	for(j = 0; j < sfs_argc; j++) {
		sfs_cmdarg = tmp_argv[j];
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
				goto finish;
			}
			sfs_cmdarg = build_path(realarg, linkfile);
			errno = ENOMEM;
		} else {
			sfs_cmdarg = sfs_realpath(sfs_cmdarg, realarg);
		}
		if (!sfs_cmdarg) {
			perror(tmp_argv[i]);
			continue;
		}
		sfs_argv[j] = strdup(sfs_cmdarg);
		if (sfs_argv[j] == NULL) {
			fprintf(stderr, "%s", anoubis_strerror(ENOMEM));
			goto finish;
		}
	}

	if (dofilter) {
		filter_file_list(sfs_argc, sfs_argv, sfs_uid, NULL);
		done = 1;
	}

	for (i = 0; i < sizeof(commands)/sizeof(struct cmd); i++) {
		if(strcmp(sfs_command, commands[i].command) != 0)
			continue;
		for (j = 0; j < sfs_argc; j++) {
			if (sfs_argv[j] == NULL)
				continue;
			sfs_cmdarg = sfs_argv[j];
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

	while((node = sfs_first_request(req_tree)) != NULL) {
		if (process_a_request(node))
			request_error = 1;
	}

finish:
	closelog();

	if (req_tree)
		sfs_destroy_request_tree(req_tree);

	if (as)
		anoubis_sig_free(as);

	if (client)
		destroy_channel();

	if (sfs_argv) {
		for (j = 0; j < sfs_argc; j++) {
			if (sfs_argv[j])
				free(sfs_argv[j]);
		}
		free(sfs_argv);
	}

	if (out_fd)
		fclose(out_fd);

	if (done == 0)
		usage();

	return (ret||request_error);
}

int
sfs_csumop(const char *file, int op, uid_t uid, u_int8_t *keyid, int idlen,
    u_int8_t *cs, int cslen, sumop_callback_t callback)
{
	struct sfs_request_node *node = NULL;
	int error = -1;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">sfs_csumop\n");

	node = sfs_find_request(req_tree, uid, keyid, idlen, op);
	if (node == NULL) {
		node = sfs_insert_request_node(req_tree, op, uid, keyid, idlen,
		    callback, NULL);
		if (node == NULL) {
			fprintf(stderr, "sfs_insert_request_node > ERROR\n");
			return -ENOMEM;
		}
	}

	error = anoubis_csmulti_add(node->req, file, cs, cslen);
	if (error < 0) {
		fprintf(stderr, "%s", anoubis_strerror(-error));
		return (-error);
	}

	if (node->req->nreqs >= REQUESTS_MAX)
		request_error = process_a_request(node);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_csumop\n");
	return 0;
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

/**
 * Return a list with the numerical user-IDs of all users that have a
 * registered checksum for a file.
 *
 * @param file The target file name.
 * @param uidp An array of uid_t values. The memory for the array will
 *     be allocated using malloc(3) and must be freed by the caller.
 * @return The number of user-IDs found or a negative error code in case
 *     of an error.
 *
 * It is possible that this function returns an emtpy list. In this case
 * the return value will be zero and no memory will be allocated.
 */
static int
request_uids(const char *file, uid_t **uidp)
{
	uid_t				 *uids;
	int				  cnt = 0;
	struct anoubis_transaction	 *t;
	char				**result;
	int				  i;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">request_uids\n");

	(*uidp) = NULL;
	t = sfs_listop(file, 0, NULL, ANOUBIS_CSUM_WANTIDS
	    | ANOUBIS_CSUM_ALL | ANOUBIS_CSUM_UID);
	if (t == NULL)
		return -ENOMEM;
	if (t->result) {
		fprintf(stderr, "%s Checksum Request failed: %d (%s)\n", file,
		    t->result, anoubis_strerror(t->result));
		anoubis_transaction_destroy(t);
		return -t->result;
	}

	result = anoubis_csum_list(t->msg, &cnt);
	anoubis_transaction_destroy(t);
	if (cnt == 0) {
		return 0;
	} else if (cnt < 0) {
		if (opts & SFSSIG_OPT_DEBUG2)
			fprintf(stderr, "anoubis_csum_list error: %s\n",
			    anoubis_strerror(-cnt));
		return cnt;
	}
	uids = calloc(cnt,  sizeof(uid_t));
	if (uids == NULL) {
		perror(file);
		for (i=0; i<cnt; ++i)
			free(result[i]);
		free(result);
		return -ENOMEM;
	}
	for (i = 0; i < cnt; i++) {
		char		ch1, ch2;
		unsigned int	tmp;
		int		ret;

		/*
		 * NOTE CEH: Old versions of the daemon used to append
		 * NOTE CEH: a slash to the IDs in the list. Accept this
		 * NOTE CEH: for compatibility reasons.
		 */
		ret = sscanf(result[i], "%u%c%c", &tmp, &ch1, &ch2);
		if (ret != 1 && (ret != 2 || ch1 != '/')) {
			fprintf(stderr, "Invalid data in user-ID list. %s\n",
			    result[i]);
			free(uids);
			/* Free remaining elements in result list. */
			for (; i<cnt; ++i)
				free(result[i]);
			free(result);
			return -EIO;
		}
		uids[i] = tmp;
		/* Free the current element of the result list. */
		free(result[i]);
	}

	free(result);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<request_uids\n");

	(*uidp) = uids;
	return cnt;
}

/**
 * Return a list with all key-IDs that have a signature stored in the
 * sfs tree for a given file.
 *
 * @param file The file name.
 * @param keyidp The list of the key-IDs will be returned in this array.
 *     Both the memory pointed to by (*ids) and each individual key-ID will
 *     be allocated using malloc(3) and must be freed by the caller.
 *     The key-IDs are returned as a sequence of hexadecimal digits and
 *     are not printable.
 * @param lenp The list of key-ID lengths will be returned in this arry.
 *     The memory pointed to by (*lenp) will be allocated by malloc(3) and
 *     must be freed by the caller.
 * @return The number of entries in the key-ID list or a negative error code
 *     in case of an error.
 *
 * It is possible that this function returns an empty list. In this case
 * the return value will be zero and no memory will be allocated.
 */
static int
request_keyids(const char *file, unsigned char *** keyidp, int **lenp)
{
	int cnt = 0;
	int *idlens = NULL;
	struct anoubis_transaction	*t;
	unsigned char			**result = NULL;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">request_keyids\n");

	(*keyidp) = NULL;
	(*lenp) = NULL;
	t = sfs_listop(file, 0, NULL, ANOUBIS_CSUM_WANTIDS | ANOUBIS_CSUM_ALL
	    | ANOUBIS_CSUM_KEY);
	if (t == NULL)
		return -ENOMEM;
	if (t->result) {
		int ret = -t->result;
		fprintf(stderr, "%s Checksum Request failed: %d (%s)\n", file,
		    t->result, anoubis_strerror(t->result));
		anoubis_transaction_destroy(t);
		return ret;
	}

	result = anoubis_keyid_list(t->msg, &idlens, &cnt);
	anoubis_transaction_destroy(t);
	if (cnt <= 0)
		return cnt;

	(*lenp) = idlens;
	(*keyidp) = result;
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<request_keyids\n");
	return cnt;
}

/**
 * Callback function that handles the result of GET and GETSIG request
 * when doing an actual get operation.
 * This function expects a csmulit_record and the matching request
 *
 * @param req The request which was send to the daemon
 * @param rec The record which contains the result from the daemon
 * @return Zero in case of success. A negative error code in case of an error.
 */
static int
sfs_get_callback(struct anoubis_csmulti_request *req,
    struct anoubis_csmulti_record *rec)
{
	struct anoubis_csentry *ent = NULL;
	uid_t uid = 0;
	unsigned int i, siglen = 0;

	if (req == NULL || rec == NULL || rec->path == NULL)
		return -EINVAL;

	if (rec->error != 0)
		return (-rec->error);

	if (req->uid)
		uid = req->uid;

	printf("%u: %s\t", uid, rec->path);
	if (rec->u.get.csum) {
		ent = rec->u.get.csum;
		for (i=0; i < get_value(ent->cslen); ++i)
			printf("%02x", ((u_int8_t *)ent->csdata)[i]);
		printf("\n");
	}
	if (rec->u.get.sig) {
		ent = rec->u.get.sig;
		siglen = get_value(ent->cslen) - ANOUBIS_CS_LEN;
		if (siglen <= 0)
			return -EINVAL;

		for (i = 0; i < ANOUBIS_CS_LEN; i++)
			printf("%02x", ((u_int8_t *)ent->csdata)[i]);
		printf("\n");
		for (; i < (siglen+ANOUBIS_CS_LEN); i++)
			printf("%02x", ((u_int8_t *)ent->csdata)[i]);
		printf("\n");
		if (rec->u.get.upgrade) {
			ent = rec->u.get.upgrade;
			printf("* Upgraded * New Checksum: ");
			for (i = 0; i < get_value(ent->cslen); i++)
				printf("%02x", ((u_int8_t *)ent->csdata)[i]);
			printf("\n");
		}
	}
	return 0;
}

/**
 * Callback function that handles the result of GET and GETSIG request
 * when doing an export request.
 * This function expects a csmulit_record and the matching request
 *
 * @param req The request which was send to the daemon
 * @param rec The record which contains the result from the daemon
 * @return Zero in case of success. A negative error code in case of an error.
 */
static int
sfs_export_callback(struct anoubis_csmulti_request *req,
    struct anoubis_csmulti_record *rec)
{
	struct sfs_entry	*export = NULL;
	unsigned char		*cs = NULL;
	unsigned char		*sig = NULL;
	unsigned char		*keyid = NULL;
	int			 ret, cslen = 0, idlen = 0, siglen = 0;
	uid_t			 uid = 0;

	if (req == NULL || rec == NULL || rec->path == NULL)
		return -EINVAL;

	if (rec->error != 0) {
		if (rec->error == ENOENT) {
			if (opts & SFSSIG_OPT_VERBOSE)
				fprintf(stderr, "No entry found for %s\n",
				    rec->path);
			return 0;
		}
		if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "Checksum Request failed: %s\n",
			    anoubis_strerror(rec->error));
		return (-rec->error);
	}

	if (req->uid)
		uid = req->uid;
	if (req->keyid) {
		keyid = req->keyid;
		idlen = req->idlen;
	}
	if (rec->u.get.csum) {
		cs = (u_int8_t *)rec->u.get.csum->csdata;
		cslen = get_value(rec->u.get.csum->cslen);
	}
	if (rec->u.get.sig) {
		sig = (u_int8_t *)rec->u.get.sig->csdata;
		siglen = get_value(rec->u.get.sig->cslen);
	}

	export = anoubis_build_entry(rec->path, cs, cslen, sig, siglen, uid,
	    keyid, idlen);
	if (export == NULL) {
		fprintf(stderr, "%s: %s\n", rec->path,
		    anoubis_strerror(ENOMEM));
		return 1;
	}

	if ((ret = anoubis_print_entries(out_fd, &export, 1)) != 0) {
		fprintf(stderr, "Error in export entries: %s\n",
		    anoubis_strerror(ret));
	}

	return 0;
}

/**
 * Callback function that handles the result of GET and GETSIG operations
 * when validating files.
 * This function compares the checksum or signature in the result with
 * the real value of the file and prints an appropriate message.
 *
 * @param req The request which was send to the daemon
 * @param rec The record which contains the result from the daemon
 * @return Zero in case of success. A negative error code in case of an error.
 */
static int
sfs_validate_callback(struct anoubis_csmulti_request *req
    __attribute__((unused)), struct anoubis_csmulti_record *rec)
{
	u_int8_t		 csum[ANOUBIS_CS_LEN];
	char			*recv = NULL;
	int			 len = ANOUBIS_CS_LEN, error = 0;
	char			*sigtype;

	if (rec == NULL || rec->path == NULL)
		return -EINVAL;

	if (rec->error != 0)
		return (-rec->error);

	if (opts & SFSSIG_OPT_SIG) {
		if (rec->u.get.sig == NULL)
			return -ENOENT;
		sigtype = "Signature";
		recv = rec->u.get.sig->csdata;
	} else {
		if (rec->u.get.csum == NULL)
			return -ENOENT;
		sigtype = "Checksum";
		recv = rec->u.get.csum->csdata;
	}

	if (opts & SFSSIG_OPT_LN) {
		error = anoubis_csum_link_calc(rec->path, csum, &len);
	} else {
		error = anoubis_csum_calc(rec->path, csum, &len);
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

	if (memcmp(recv, csum, ANOUBIS_CS_LEN) == 0)
		printf("%s: %s Match\n", rec->path, sigtype);
	else
		printf("%s: %s Mismatch\n", rec->path, sigtype);
	return 0;
}

/**
 * Callback function to handle GET and GETSIG results when listing files.
 * This function simply prints the file name if the result contains are
 * checksum or signature (depending on the --sum and --sig flag).
 *
 * @param req The request which was send to the daemon
 * @param rec The record which contains the result from the daemon
 * @return Zero in case of success. A negative error code in case of an error.
 */
static int
sfs_getnosum_callback(struct anoubis_csmulti_request *req
    __attribute__((unused)), struct anoubis_csmulti_record *rec)
{
	if (rec == NULL || rec->path == NULL)
		return -EINVAL;
	if (rec->error)
		return (-rec->error);

	printf("%s\n", rec->path);
	return 0;
}

/**
 * Callback function to handle GET and GETSIG results when listing files.
 * This function simply prints the file name if the result contains are
 * checksum or signature (depending on the --sum and --sig flag).
 *
 * @param req The request which was send to the daemon
 * @param rec The record which contains the result from the daemon
 * @return Zero in case of success. A negative error code in case of an error.
 */
static int
sfs_add_del_callback(struct anoubis_csmulti_request *req
    __attribute__((unused)), struct anoubis_csmulti_record *rec)
{
	int i;

	if (rec == NULL || rec->path == NULL)
		return -EINVAL;
	if (rec->error) {
		fprintf(stderr, "%s: %s\n", rec->path,
		    anoubis_strerror(rec->error));
		return (-rec->error);
	}

	if (opts & SFSSIG_OPT_VERBOSE) {
		printf("%s", rec->path);

		if (rec->u.add.csdata) {
			printf(": ");
			for (i = 0; i < ANOUBIS_CS_LEN; i++)
				printf("%02x",
				    ((u_int8_t *)rec->u.add.csdata)[i]);
			printf("\n");
		} else {
			printf("\n");
		}
	}

	return 0;
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

	count = request_uids(file, &uids);
	if (count < 0) {
		fprintf(stderr, "Failed to list uids for %s: %s\n",
		    file, anoubis_strerror(-count));
		return -EIO;
	} else if (count == 0) {
		/* No uids found at all. Return ENOENT. */
		return -ENOENT;
	}
	for (i=0; i<count; ++i) {
		error = sfs_csumop(file, operation, uids[i], NULL, /* keyid */
		    0, /* idlen */ cs, cslen, callback);
		if (error)
			break;
	}
	free(uids);
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

	count = request_keyids(file, &keyids, &idlens);
	if (count == 0) {
		return -ENOENT;
	} else if (count < 0) {
		fprintf(stderr, "Failed to list keyids for %s: %s\n",
		    file, anoubis_strerror(-count));
		return -EIO;
	}
	for (i=0; i<count; ++i) {
		error = sfs_csumop(file, operation, 0, keyids[i], idlens[i],
		    NULL, 0, callback);
		if (error)
			break;
	}
	for (i=0; i<count; ++i)
		free(keyids[i]);
	free(keyids);
	free(idlens);

	return error;
}

/**
 * Common function to perform add, del, get and validate operations.
 *
 * @param file The target file.
 * @param operation The operation that should be performed.
 * @param sfs_uid The target user-ID for a single uid checksum operation.
 * @param callback The callback function that handles the result of a
 *     successful get operation.
 * @return Zero in case of success, one in case of an error. ENOENT errors
 *     for GET and GETSIG operations are suppressed.
 *     XXX CEH: Returning 1 for an error and zero for success is bogus.
 *     XXX CEH: Calling conventions should be fixed.
 */
static int
sfs_generic_op(const char *file, int operation, uid_t sfs_uid,
    sumop_callback_t callback)
{
	u_int8_t	 csum[ANOUBIS_CS_LEN];
	unsigned int	 siglen = 0;
	int		 len = 0, ret;
	u_int8_t	*sig = NULL;
	void		*csp = NULL;

	if (operation == ANOUBIS_CHECKSUM_OP_ADDSIG
	    && (opts & SFSSIG_OPT_ALLCERT)) {
		fprintf(stderr, "WARNING: Will ignore '--cert all "
		    "for signature add operation");
		opts &= ~(SFSSIG_OPT_ALLCERT);
	}
	if (operation == ANOUBIS_CHECKSUM_OP_ADDSUM
	    || operation == ANOUBIS_CHECKSUM_OP_ADDSIG) {
		len = ANOUBIS_CS_LEN;
		csp = csum;
		if (opts & SFSSIG_OPT_LN) {
			ret = anoubis_csum_link_calc(file, csum, &len);
		} else {
			ret = anoubis_csum_calc(file, csum, &len);
		}
		if (ret < 0) {
			fprintf(stderr, "Checksum calculation for "
			    "%s failed: %s\n", file, anoubis_strerror(-ret));
			return 1;
		}
		if (opts & SFSSIG_OPT_SIG) {
			sig = anoubis_sign_csum(as, csum, &siglen);
			if (!sig) {
				fprintf(stderr,
				    "Error in anoubis_sign_csum\n");
				return 1;
			}
		}
	}
	if ((opts & SFSSIG_OPT_SIG) == 0) {
		if (opts & SFSSIG_OPT_ALLUID) {
			ret = sfs_csumop_alluid(file, operation, csp, len,
			    callback);
		} else {
			ret = sfs_csumop(file, operation, sfs_uid,
			    NULL, 0, csp, len, callback);
		}
	} else {
		if (opts & SFSSIG_OPT_ALLCERT) {
			ret = sfs_sigop_allkeys(file, operation, callback);
		} else {
			ret = sfs_csumop(file, operation, 0,
			    as->keyid, as->idlen, sig, siglen, callback);
		}
	}
	if (sig)
		free(sig);

	if (ret != 0) {
		if (ret == -ENOENT && (operation == ANOUBIS_CHECKSUM_OP_GET2
		    || operation == ANOUBIS_CHECKSUM_OP_GETSIG2)) {
			if (opts & SFSSIG_OPT_VERBOSE)
				fprintf(stderr, "No entry found for %s\n",
				    file);
			return 1;
		}
		fprintf(stderr, "Checksum request failed: %s\n",
		    anoubis_strerror(-ret));
		return 1;
	}

	return 0;
}

/**
 * Perfrom an sfs add operation on a file.
 * @param file The target file.
 * @param sfs_uid The sfs User-ID that should be used.
 * @return See sfs_generic_op
 */
static int
sfs_add(char *file, uid_t sfs_uid) {
	int	op = ANOUBIS_CHECKSUM_OP_ADDSUM;

	if (opts & SFSSIG_OPT_SIG)
		op = ANOUBIS_CHECKSUM_OP_ADDSIG;
	return sfs_generic_op(file, op, sfs_uid, sfs_add_del_callback);
}

/**
 * Perform an sfs delete operation on a file.
 *
 * @param file The target file.
 * @param sfs_uid The sfs User-ID that should be used for checksum operations.
 * @return @see sfs_generic_op.
 */
static int
sfs_del(char *file, uid_t sfs_uid)
{
	int	op = ANOUBIS_CHECKSUM_OP_DEL;

	if (opts & SFSSIG_OPT_SIG)
		op = ANOUBIS_CHECKSUM_OP_DELSIG;
	return sfs_generic_op(file, op, sfs_uid, sfs_add_del_callback);
}

/**
 * Perform an sfs get operation on a file.
 *
 * @param file The target file.
 * @param sfs_uid The sfs user-ID that should be used for checksum operations.
 * @return @see sfs_generic_op.
 */
static int
sfs_get(char *file, uid_t uid)
{
	return __sfs_get(file, uid, sfs_get_callback);
}

/**
 * Perform an sfs validate operation on a file.
 *
 * @param file The target file.
 * @param sfs_uid The sfs user-ID that should be used for checksum operations.
 * @return @see sfs_generic_op
 */
static int
sfs_validate(char *file, uid_t uid)
{
	return __sfs_get(file, uid, sfs_validate_callback);
}

/**
 * Generic wrapper for get validate and some list operations.
 * @param file The target file of the operation.
 * @param sfs_uid The sfs user-ID that should be used for checksum operations.
 * @param callback The callback function that handles successful get requests.
 * @return @see sfs_generic_op
 */
static int
__sfs_get(char *file, uid_t sfs_uid, sumop_callback_t callback)
{
	int	op = ANOUBIS_CHECKSUM_OP_GET2;

	if (opts & SFSSIG_OPT_SIG)
		op = ANOUBIS_CHECKSUM_OP_GETSIG2;
	return sfs_generic_op(file, op, sfs_uid, callback);
}

/**
 * Perform an sfs list operation. This function first interprets the list
 * operation as a file and list checksums for the file. Then the arguement
 * is interpreted as a directory and the directory contents are listed.
 * This is neccessary because the sfs tree might contain both kinds of
 * objects under the same name and we have no way of knowning which one
 * the user wants.
 *
 * @param arg The path name.
 * @param sfs_uid The user-ID that should be used for checksum related
 *     operations.
 * @return @see sfs_generic_op
 */
static int
sfs_list(char *arg, uid_t sfs_uid)
{
	if (list(arg, sfs_uid) == 0) {
		request_error = 0;
		return 0;
	/* If we list recursively we can expect that *arg is not a file */
	} else if (opts & SFSSIG_OPT_TREE) {
		return 0;
	} else {
		/* sfs_get with / produces Invalid Argument */
		if (strcmp(arg, "/") != 0)
			return __sfs_get(arg, sfs_uid, sfs_getnosum_callback);
	}

	return 1;
}

static int
list(char *arg, uid_t sfs_uid)
{
	struct anoubis_transaction	 *t;
	char				**result = NULL;
	char				 *new = NULL;
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
			return 1;
		}
		fprintf(stderr, "%s: Checksum Request failed: %d (%s)\n",
				arg, t->result, anoubis_strerror(t->result));
		anoubis_transaction_destroy(t);
		return 1;
	}
	result = anoubis_csum_list(t->msg, &sfs_cnt);
	if (!result) {
		if (opts & SFSSIG_OPT_DEBUG)
			fprintf(stderr, "anoubis_csum_list: no result\n");
		anoubis_transaction_destroy(t);
		return 1;
	}
	anoubis_transaction_destroy(t);

	if (opts & SFSSIG_OPT_FILTER)
		filter_file_list(sfs_cnt, result, thisuid, arg);

	for (i = 0; i < sfs_cnt; i++) {
		if (result[i] == NULL)
			continue;
		len = strlen(result[i]) - 1;
		if (opts & SFSSIG_OPT_DEBUG2)
			fprintf(stderr, "sfs_list: got file %s\n", result[i]);
		if (result[i][len] == '/') {
			if (opts & SFSSIG_OPT_TREE) {
				new = build_path(arg,result[i]);
				if (new == NULL)
					return 1;
				list(new, sfs_uid);
				free(new);
			}
			continue;
		}
		/* Avoid double slash at the beginning of root-dir */
		if (strcmp(arg, "/") != 0)
			printf("%s/%s\n", arg, result[i]);
		else
			printf("/%s\n", result[i]);
	}
	for (i = 0; i < sfs_cnt; i++) {
		if (result[i] == NULL)
			continue;
		free(result[i]);
	}
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
	char		**result = NULL;
	char		**alloc = NULL;
	DIR		*dirp;
	int		 ret = 0;
	int		 i, len, res_cnt = 0;

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
			alloc = realloc(result, (res_cnt+1) * sizeof(char *));
			if (alloc == NULL) {
				ret = 1;
				goto out;
			}
			result = alloc;
			result[res_cnt] = strdup(tmp);
			if (result[res_cnt] == NULL) {
				ret = 1;
				goto out;
			}
			res_cnt++;
			break;
		default:
			ret = 0;
			break;
		}
		if (tmp)
			free(tmp);
		tmp = NULL;

	}

	if (opts & SFSSIG_OPT_FILTER)
		filter_file_list(res_cnt, result, sfs_uid, NULL);

	for (i = 0; i < res_cnt; i++) {
		if (result[i] == NULL)
			continue;
		if (sfs_add(result[i], sfs_uid) != 0) {
			ret = 1;
			break;
		}
	}
out:
	closedir(dirp);
	if (tmp)
		free(tmp);

	if (result) {
		for (i = 0; i < res_cnt; i++) {
			if (result[i] == NULL)
				continue;
			free(result[i]);
		}
		free(result);
	}

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
	int		 list_cnt = 0;
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

			fprintf(stderr,
				"%s: Checksum Request failed: %d (%s)\n",
				tmp, t->result, anoubis_strerror(t->result));
			anoubis_transaction_destroy(t);
			return 1;
		}

		result = anoubis_csum_list(t->msg, &list_cnt);
		if (!result && list_cnt < 0) {
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
			list_cnt++;
			tmpalloc = realloc(result, list_cnt * sizeof(char *));
			if (!tmpalloc)
				goto out;
			result = tmpalloc;
			result[list_cnt-1] = strdup(d_ent->d_name);
			if (!result[list_cnt-1])
				goto out;

		}
		closedir(dirp);
		dirp = NULL;
	}
	if (tmp)
		free(tmp);
	tmp = NULL;

	if (opts & SFSSIG_OPT_FILTER)
		filter_file_list(list_cnt, result, sfs_uid, path);

	for (j = 0; j < list_cnt; j++) {
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
	if (dirp)
		closedir(dirp);
	if (tmp)
		free(tmp);
	if (result) {
		for (j = 0; j < list_cnt; j++) {
			if (result[j] != NULL)
				free(result[j]);
		}
		free(result);
	}
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<sfs_tree\n");

	return ret;
}

int
sfs_export(char *arg, uid_t uid)
{
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
	uid_t				*uid_result = NULL;
	char				*path = NULL;
	char				**result = NULL;
	unsigned char			**keyid_result = NULL;
	unsigned char			*keyid = NULL;
	int				 i, sfs_cnt = 0;
	int				 j, len, uid_cnt = 0;
	int				 keyid_cnt = 0, idlen = 0;
	int				 *ids = NULL, ret = 0;

	if (arg == NULL)
		return 1;
	if (opts & SFSSIG_OPT_DEBUG2)
		fprintf(stderr, "arg: %s\n", arg);
	if (!rec && (strcmp(arg, "/") != 0)) {
		if (get_entry(arg, NULL, 0, sfs_uid) < 0)
			/* XXX KM: What to do ??? ? */ return 1;
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
				    anoubis_strerror(t->result));
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
			uid_cnt = request_uids(path, &uid_result);
			if (uid_cnt > 0 && uid_result) {
				for (j = 0; j < uid_cnt; j++) {
					sfs_uid = uid_result[j];
					get_entry(path, NULL, 0, sfs_uid);
				}
			}
		}
		if (opts & SFSSIG_OPT_ALLCERT) {
			keyid_cnt = request_keyids(path, &keyid_result, &ids);
			if (keyid_cnt > 0) {
				for (j = 0; j < keyid_cnt; j++) {
					keyid = keyid_result[j];
					idlen = ids[j];
					get_entry(path, keyid, idlen, sfs_uid);
				}
				for(j = 0; j < keyid_cnt; j++)
					free(keyid_result[j]);
				free(keyid_result);
				keyid_result = NULL;
			} else if ((keyid_cnt < 0) && (opts & SFSSIG_OPT_DEBUG))
				fprintf(stderr, "Requesting keyids failed %s\n",
				    anoubis_strerror(-keyid_cnt));
		}
		if ((opts & (SFSSIG_OPT_ALLUID | SFSSIG_OPT_ALLCERT)) == 0)
			get_entry(path, NULL, 0, sfs_uid);
		free(path);
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
			cserr = sfs_csumop(entry->name,
			    ANOUBIS_CHECKSUM_OP_ADDSUM, sfs_uid, NULL, 0,
			    entry->checksum, ANOUBIS_CS_LEN, NULL);
			if (cserr) {
				fprintf(stderr, "Checksum request for "
				    "file=%s uid=%d failed: %s\n",
				    entry->name, entry->uid,
				    anoubis_strerror(-cserr));
			}
		}
	}
	if (entry->signature) {
		if (!entry->keyid) {
			fprintf(stderr, "Parse Error\n");
			sigerr = EINVAL;
		} else {
			sigerr = sfs_csumop(entry->name,
			    ANOUBIS_CHECKSUM_OP_ADDSIG, 0, entry->keyid,
			    entry->keylen, entry->signature, entry->siglen,
			    NULL);
			if (sigerr) {
				fprintf(stderr, "Signature request for "
				    "file=%s failed: %s\n", entry->name,
				    anoubis_strerror(-sigerr));
			}
		}
	}
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<add_entry\n");
	if (cserr)
		return cserr;
	return sigerr;
}

static int
get_entry(char *file, unsigned char *keyid_p, int idlen_p, uid_t sfs_uid)
{
	unsigned char			*keyid = NULL;
	int				 idlen = 0;
	int				 ret = 0;

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
		sfs_csumop(file, ANOUBIS_CHECKSUM_OP_GETSIG2, 0,
		    keyid, idlen, NULL, 0, sfs_export_callback);
	}

	ret = sfs_csumop(file, ANOUBIS_CHECKSUM_OP_GET2, sfs_uid, NULL, 0, NULL,
	    0, sfs_export_callback);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<get_entry\n");

	return (ret);
}

/**
 * Process all collected requests from the request_tree. It will set
 * process_error to 1 if an error occured. So sfssig will end as wished.
 *
 * param  n A request node which should be proceed.
 * return 0 if everythin is allright or
 *	  1 in case of an error.
 */
static int
process_a_request(struct sfs_request_node *n)
{
	struct anoubis_transaction *t = NULL;
	struct anoubis_csmulti_record   *rec;
	int ret, result = 0;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">process_a_request\n");

req_again:
	t = anoubis_client_csmulti_start(client, n->req);
	if (t == NULL) {
		fprintf(stderr, "Error while starting transaction\n");
		sfs_delete_request(req_tree, n);
		return 1;
	}
	while ((t->flags & ANOUBIS_T_DONE) == 0) {
		ret = anoubis_client_wait(client);
		if (ret < 0) {
			fprintf(stderr, "Error while waiting for "
			    "request %s\n",
			    anoubis_strerror(-ret));
			sfs_delete_request(req_tree, n);
			return 1;
		}
	}
	ret = t->result;
	anoubis_transaction_destroy(t);
	if (ret) {
		fprintf(stderr, "Transaction error (%d): %s\n",
		    ret, anoubis_strerror(ret));
		sfs_delete_request(req_tree, n);
		return 1;
	}

	if (n->req->openreqs)
		goto req_again;

	TAILQ_FOREACH(rec, &n->req->reqs, next) {
		if (n->sumop_callback) {
			ret = (* n->sumop_callback)(n->req, rec);
			if (ret != 0) {
				result = ret;
			}
		}
	}

	sfs_delete_request(req_tree, n);

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<process_a_request\n");

	return result;
}

/**
 * Starts a single filter request
 *
 * @param node of the request
 * @return 0 or 1 in case of an error
 */
static int
process_filter_request(struct sfs_request_node *n)
{
	struct anoubis_transaction *t = NULL;
	int ret;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">process_filter_request\n");

req_again:
	t = anoubis_client_csmulti_start(client, n->req);
	if (t == NULL) {
		fprintf(stderr, "Error while starting transaction\n");
		sfs_delete_request(filter_tree, n);
		return 1;
	}
	while ((t->flags & ANOUBIS_T_DONE) == 0) {
		ret = anoubis_client_wait(client);
		if (ret < 0) {
			fprintf(stderr, "Error while waiting for "
			    "request %s\n",
			    anoubis_strerror(-ret));
			sfs_delete_request(filter_tree, n);
			return 1;
		}
	}
	ret = t->result;
	anoubis_transaction_destroy(t);
	if (ret) {
		fprintf(stderr, "Transaction error (%d): %s\n",
		    ret, anoubis_strerror(ret));
		sfs_delete_request(filter_tree, n);
		return 1;
	}

	if (n->req->openreqs)
		goto req_again;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<process_filter_request\n");
	return 0;
}

/**
 * @param count of files in second parameter
 * @param list of files which should be filtered
 * @param uid for the filtering
 * @param path to the files if file liste of second parameter just containing
 *	  filenames
 * @return 0 or 1 in case of an error
 */
static int
filter_file_list(int cnt, char **list, uid_t sfs_uid, char *path)
{
	struct sfs_request_node *node = NULL;
	int i, ret = 0, len;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">filter_file_list\n");

	if (cnt <= 0 || list == NULL)
		return 0;

	for (i = 0; i < cnt; i++) {
		/* Skip empty entries... */
		if((len = strlen(list[i])) == 0)
			continue;
		/* ...and directories */
		if (list[i][len-1] == '/')
			continue;
		ret = filter_a_file(list[i], path, sfs_uid, as);
		if (ret < 0) {
			fprintf(stderr, "Filter error: %s\n",
			    anoubis_strerror(-ret));
			return -ret;
		} else if (ret == 0) {
			free(list[i]);
			list[i] = NULL;
		}
	}
	while((node = sfs_first_request(filter_tree)) != NULL) {
		if ((ret = process_filter_request(node)) != 0)
			return ret;
		filter_result(node, cnt, list, path);
		sfs_delete_request(filter_tree, node);
	}
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<filter_file_list\n");
	return 0;
}

/**
 * This function filters the result of the request. It goes through the
 * file list and calls for every file the filter callbacks and delets or keeps
 * the file depending of the result of the callback.
 *
 * @param node containing the result of one request
 * @param count of the file list
 * @param file list which is to be filtered
 * @param path to files in the file list, if file list contains filenames.
 * @return 0 or 1 in case of an errror
 */
static int
filter_result(struct sfs_request_node *node, int cnt, char **list, char *path)
{
	struct anoubis_csmulti_record *rec = NULL;
	char *arg;
	int i;

	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, ">filter_result\n");
	if (node == NULL || list == NULL)
		return 1;

	for (i = 0; i < cnt; i++) {
		if (list[i] == NULL)
			continue;

		if (path) {
			if ((arg = build_path(path, list[i])) == NULL)
				return 1;
		} else {
			arg = list[i];
		}

		TAILQ_FOREACH(rec, &node->req->reqs, next) {
			if (strcmp(rec->path, arg) != 0)
				continue;
			/*
			 * If callback returns 1 we keep the file in the list
			 * If callback returns 0 we removed from the list
			 */
			if ((* node->filter_callback)(node->req, rec) == 0) {
				free(list[i]);
				list[i] = NULL;
				break;
			}
		}
		if (path)
			free(arg);
	}
	if (opts & SFSSIG_OPT_DEBUG)
		fprintf(stderr, "<filter_result\n");
	return 0;
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

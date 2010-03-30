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
#include "version.h"

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/file.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <syslog.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include <openssl/rand.h>

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
#include <anoubis_sig.h>
#include <anoubis_transaction.h>
#include <anoubis_dump.h>

#include <anoubis_chat.h>
#include <anoubis_apnvm.h>
#include <anoubis_auth.h>

#include "anoubisctl.h"
#include "apn.h"

#define ANOUBISCTL_PRIO_ADMIN	0

void		usage(void) __dead;
static int	daemon_start(void);
static int	daemon_stop(void);
static int	daemon_status(void);
static int	daemon_reload(void);
static int	daemon_restart(void);
static int	daemon_version(void);
static int	send_passphrase(void);
static int	monitor(int argc, char **argv);
static int	load(char *, uid_t, unsigned int);
static int	dump(char *, uid_t, unsigned int);
static int	create_channel(unsigned int);
static void	destroy_channel(void);
static int	fetch_versions(int *, int *);
static int	moo(void);
static int	verify(const char *file, const char *signature);

typedef int (*func_int_t)(void);
typedef int (*func_arg_t)(char *, uid_t, unsigned int);

struct cmd {
	char	       *command;
	func_int_t	func;
	/*
	 * file:
	 * 0:	doesn't require parameters
	 * 1:	needs file, uid and prio arguments,
	 *	resolve relative path names (file must exist)
	 * 2:	needs file, uid and prio arguments,
	 *	don't resolve relative path names
	 * 3:   no params required and should not need .xanoubis
	 */
	int		args;
} commands[] = {
	{ "moo",	moo,			0 },
	{ "start",	daemon_start,		3 },
	{ "stop",	daemon_stop,		3 },
	{ "status",	daemon_status,		0 },
	{ "reload",	daemon_reload,		3 },
	{ "restart",	daemon_restart,		3 },
	{ "version",	daemon_version,		0 },
	{ "passphrase",	send_passphrase,	0 },
	{ "load",	(func_int_t)load,	1 },
	{ "dump",	(func_int_t)dump,	2 },
};

static const char		 def_cert[] = ".xanoubis/default.crt";
static const char		 def_pri[] =  ".xanoubis/default.key";
static int			 opts = 0;
static char			*keyfile = NULL;
static char			*certfile = NULL;
static struct anoubis_sig	*as = NULL;
static int			 passfd = -1;

static struct achat_channel	*channel;
struct anoubis_client		*client = NULL;
char				*sigfile = NULL;

extern char	*__progname;

__dead void
usage(void)
{
	unsigned int	i;

	fprintf(stderr, "usage: %s [-fnv] [-k key] [-c cert] [-p <prio>] "
	    "[-u uid] [-i fd] [-o sigfile] <command> [<file>]\n", __progname);
	fprintf(stderr, "    <command>:\n");
	for (i=1; i < sizeof(commands)/sizeof(struct cmd); i++) {
		switch (commands[i].args) {
		case 1:
		case 2:
			fprintf(stderr, "	%s <file>\n",
				commands[i].command);
			break;
		case 3:
		case 4:
			fprintf(stderr, "	%s\n", commands[i].command);
			break;
		}
	}
	fprintf(stderr, "       verify file signature\n");
	fprintf(stderr, "	monitor [ all ] [ delegate ] [ error=<num> ] "
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
	struct stat	 sb;
	int		 ch, done;
	unsigned int	 i, prio = 1;
	int		 error = 0;
	char		*command = NULL;
	char		*rulesopt = NULL;
	char		*homepath = NULL;
	char		 buf[PATH_MAX];
	struct passwd	*pwd = NULL;
	uid_t		 uid = geteuid();
	char		*prios[] = { "admin", "user", NULL };
	char		 tmp;
	struct cmd	*cmd = NULL;
	int		 need_xanoubis = 1;

	/*
	 * Drop privileges immediately. No need for setgid and it hurts
	 * anoubisctl start on OpenBSD.
	 */
	if (setresgid(getgid(), getgid(), getgid()) < 0) {
		perror("Failed to drop privileges");
		return 2;
	}
	if (argc < 2)
		usage();

	/* default to admin policies for root */
	if (uid == 0)
		prio = 0;

	while ((ch = getopt(argc, argv, "fnc:k:p:u:vi:o:")) != -1) {
		switch (ch) {
		case 'p':
			for (i=0; prios[i] != NULL; i++) {
				if (strncmp(optarg, prios[i],
					strlen(prios[i])) == 0) {
					prio = i;
					break;
				}
			}
			if (prios[i] == NULL) {
				fprintf(stderr, "invalid policy type\n");
				usage();
			}
			break;
		case 'u':
			if ((pwd = getpwnam(optarg)) != NULL)
				uid = pwd->pw_uid;
			else if (strspn(optarg, "0123456789") == strlen(optarg))
				uid = strtoul(optarg, NULL, 10);
			else if (!strcasecmp(optarg, "default"))
				uid = (uid_t)-1;
			else {
				fprintf(stderr, "invalid username\n");
				usage();
			}
			break;
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
		case 'c':
			opts |= ANOUBISCTL_OPT_CERT;
			opts |= ANOUBISCTL_OPT_SIGN;
			certfile = strdup(optarg);
			if (!certfile) {
				perror(optarg);
				return 1;
			}
			break;
		case 'k':
			opts |= ANOUBISCTL_OPT_SIGN;
			keyfile = strdup(optarg);
			if (!keyfile) {
				perror(optarg);
				return 1;
			}
			break;
		case 'o':
			opts |= ANOUBISCTL_OPT_SIGNONLY;
			sigfile = strdup(optarg);
			if (!sigfile) {
				perror(optarg);
				return 1;
			}
			break;
		case 'i':
			if (passfd >= 0)
				usage();
			if (sscanf(optarg, "%d%c", &passfd, &tmp) != 1)
				usage();
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

	for (i=0; i < sizeof(commands)/sizeof(struct cmd); i++) {
		if (strcmp(commands[i].command, command) == 0) {
			cmd = &commands[i];
			break;
		}
	}
	if (cmd && cmd->args == 3)
		need_xanoubis = 0;
	openlog(__progname, LOG_ODELAY|LOG_PERROR, LOG_USER);

	if (need_xanoubis) {
		if ((homepath = getenv("HOME")) == NULL) {
			fprintf(stderr,
			    "Error: HOME environment variable not set\n");
			return 1;
		}

		if (anoubis_ui_init() < 0) {
			fprintf(stderr,
			    "Error while initialising anoubis_ui\n");
			return 1;
		}

		error = anoubis_ui_readversion();
		if (error > ANOUBIS_UI_VER) {
			syslog(LOG_WARNING,
			    "Unsupported version (%d) of %s/" ANOUBIS_UI_DIR
			    " found.", error, homepath);
			return 1;
		}
		if (error < 0) {
			syslog(LOG_WARNING, "Error reading %s/" ANOUBIS_UI_DIR
			    " version: %s\n", homepath,
			    anoubis_strerror(-error));
			return 1;
		}

		if (certfile == NULL) {
			if (asprintf(&certfile, "%s/%s", homepath,
			    def_cert) < 0) {
				fprintf(stderr,
				    "Error while allocating memory\n");
				return 1;
			}
			if (stat(certfile, &sb) == 0) {
				opts |= ANOUBISCTL_OPT_SIGN;
			} else {
				free(certfile);
				certfile = NULL;
			}
		}
		/* You also need a keyfile if you want to sign a policy */
		if (keyfile == NULL) {
			if (asprintf(&keyfile, "%s/%s", homepath,
			    def_pri) < 0) {
				fprintf(stderr, "Error while allocating"
				    "memory\n");
				return 1;
			}
			if (stat(keyfile, &sb) == 0) {
				opts |= ANOUBISCTL_OPT_SIGN;
			} else {
				free(keyfile);
				keyfile = NULL;
			}
		}
	}
	if (passfd >= 0 && strcmp(command, "passphrase") != 0) {
		fprintf(stderr, "Command %s does not support option -i\n",
		    command);
		close(passfd);
		usage();
	}
	if (strcmp(command, "load") == 0 && (opts & ANOUBISCTL_OPT_SIGN) ) {
		if (keyfile == NULL) {
			fprintf(stderr, "To load signed policies to the daemon "
			    "you need to specify a keyfile\n");
			usage();
			/* NOTREACHED */
		}
		if (certfile == NULL) {
			fprintf(stderr, "To load signed policies to the daemon "
			    "you need to specify a certficate\n");
			usage();
			/* NOTREACHED */
		}
		error = anoubis_sig_create(&as, keyfile, certfile, pass_cb);
		if (error < 0) {
			fprintf(stderr, "Error while loading keyfile: %s"
			    " or certfile %s\n", keyfile, certfile);
			return 1;
		}
	} else if (strcmp(command, "verify") == 0) {
		if (certfile == NULL) {
			fprintf(stderr, "The verify commands requires a "
			    "ceritificate file\n");
			usage();
		}
		error = anoubis_sig_create(&as, NULL, certfile, NULL);
		if (error < 0) {
			fprintf(stderr, "Error while loading keyfile: %s\n",
			    keyfile);
			return 1;
		}
	}

	done = 0;
	error = 0;
	if (cmd) {
		switch (cmd->args) {
		case 1:
		case 2:
			if (rulesopt == NULL || argc != 1) {
				fprintf(stderr, "no rules file\n");
				error = 4;
				break;
			}
			if (strcmp(rulesopt, "-") != 0 && cmd->args != 2) {
				rulesopt = realpath(rulesopt, buf);
			}
			if (rulesopt == NULL) {
				perror("realpath");
				error = 4;
				break;
			}
			error = ((func_arg_t)cmd->func)(rulesopt, uid, prio);
			done = 1;
			break;
		case 0:
		case 3:
			if (rulesopt != NULL) {
				fprintf(stderr, "too many arguments\n");
				error = 4;
			} else {
				error = (cmd->func)();
				done = 1;
			}
			break;
		default:
			fprintf(stderr, "Internal error: cmd->args = %d\n",
			    cmd->args);
			error = 4;
		}
	}
	if (!done && strcmp(command, "verify") == 0) {
		done = 1;
		if (argc != 2)
			usage();
		error = verify(argv[0], argv[1]);
	}
	if (!done && strcmp(command, "monitor") == 0) {
		error = monitor(argc, argv);
		done = 1;
	}
	if (as)
		anoubis_sig_free(as);
	if (keyfile)
		free(keyfile);
	if (certfile)
		free(certfile);
	if (!done)
		usage();

	exit(error);
}

/*
 * NOTE: This function might be called from the background via
 * NOTE: daemon_restart. Do not use fprintf, instead use syslog for
 * NOTE: error reporting. Using perror will make sure that error message
 * NOTE: are also seen on stderr by the user.
 */
static int
daemon_start(void)
{
	int			 err;
	int			 error = 0;
	static const char	*command = PACKAGE_SBINDIR "/" PACKAGE_DAEMON;

	if ((err = system(command))) {
		if (err < 0)
		    syslog(LOG_ERR, "%s failed: %s", command,
			anoubis_strerror(errno));
		else
		    syslog(LOG_ERR, "%s failed with exit code %d", command,
			WEXITSTATUS(err));
		error = 2;
	}

	return error;
}

static int
daemon_stop(void)
{
	FILE * fp = fopen(PACKAGE_PIDFILE, "r");
	int i, pid;

	if (!fp) {
		fprintf(stderr, "Couldn't open " PACKAGE_PIDFILE ": %s\n",
			anoubis_strerror(errno));
		return 2;
	}
	if (flock(fileno(fp), LOCK_EX|LOCK_NB) != -1) {
		fprintf(stderr, "Unused " PACKAGE_PIDFILE " found\n");
		return 2;
	}
	if (fscanf(fp, "%d", &pid) != 1) {
		fprintf(stderr, "No valid pid found in " PACKAGE_PIDFILE "\n");
		return 2;
	}
	if (kill(pid, SIGTERM) != 0) {
		fprintf(stderr, "Couldn't send signal: %s\n",
				anoubis_strerror(errno));
		return 2;
	}
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

	error = create_channel(0);
	if (error == 0)
		printf(PACKAGE_DAEMON " is running\n");
	else
		printf(PACKAGE_DAEMON " is not running\n");
	destroy_channel();
	return error;
}

static int
daemon_reload(void)
{
	FILE * fp = fopen(PACKAGE_PIDFILE, "r");
	int pid;

	if (!fp) {
		fprintf(stderr, "Couldn't open " PACKAGE_PIDFILE ": %s\n",
			anoubis_strerror(errno));
		return 2;
	}
	if (flock(fileno(fp), LOCK_EX|LOCK_NB) != -1) {
		fprintf(stderr, "Unused " PACKAGE_PIDFILE " found\n");
		return 2;
	}
	if (fscanf(fp, "%d", &pid) != 1) {
		fprintf(stderr, "No valid pid found in " PACKAGE_PIDFILE "\n");
		return 2;
	}
	if (kill(pid, SIGHUP) != 0) {
		fprintf(stderr, "Couldn't send signal: %s\n",
				anoubis_strerror(errno));
		return 2;
	}
	return 0;
}

static int
daemon_restart(void)
{
	FILE	*fp = fopen(PACKAGE_PIDFILE, "r");
	int	 pid, child;
	int	 sfsfd, i;

	if (!fp) {
		fprintf(stderr, "Couldn't open " PACKAGE_PIDFILE ": %s. "
		    "Starting a new daemon\n", anoubis_strerror(errno));
		return daemon_start();
	} else {
		fcntl(fileno(fp), F_SETFD, FD_CLOEXEC);
	}
	if (flock(fileno(fp), LOCK_EX|LOCK_NB) != -1) {
		fprintf(stderr, "Unused " PACKAGE_PIDFILE " found. "
		    "Starting a new daemon\n");
		return daemon_start();
	}
	if (fscanf(fp, "%d", &pid) != 1) {
		fprintf(stderr, "No valid pid found in " PACKAGE_PIDFILE "\n");
		return 2;
	}
	sfsfd = open(PACKAGE_POLICYDIR "/sfs.version", O_RDONLY);
	if (sfsfd < 0) {
		perror("Failed to open " PACKAGE_POLICYDIR "/sfs.version");
		goto kill;
	}
	if (flock(sfsfd, LOCK_EX | LOCK_NB) == 0)
		goto kill;
	if (errno != EWOULDBLOCK) {
		perror("flock for " PACKAGE_POLICYDIR "/sfs.version failed");
		goto kill;
	}
	fprintf(stderr,
	    "Warning: Upgrade in progress. anoubisd restart delayed\n");
	child = fork();
	if (child < 0) {
		perror("fork");
		goto kill;
	}
	if (child > 0) {
		/* This is the parent process. Just idicate success. */
		return 0;
	}
	if (daemon(0, 0) < 0)
		perror("Daemonizing failed");
	while (1) {
		if (flock(sfsfd, LOCK_EX) == 0)
			break;
		if (errno != EINTR && errno != EAGAIN) {
			syslog(LOG_CRIT,
			    "flock failed. Restarting Anoubis Daemon NOW\n");
			break;
		}
	}
kill:
	if (kill(pid, SIGTERM) != 0) {
		syslog(LOG_CRIT,
		    "Couldn't send signal to anoubisd: %s\n",
		    anoubis_strerror(errno));
		return 2;
	}
	/* Make sure the the new anoubisd does not inherit this fd! */
	if (sfsfd >= 0)
		close(sfsfd);
	/* Wait at most 10 seconds for termination. */
	for(i=0; i<10; ++i) {
		if (kill(pid, 0) < 0 && errno == ESRCH)
			return daemon_start();
		sleep(1);
	}
	if (kill(pid, 0) < 0 && errno == ESRCH)
		return daemon_start();
	syslog(LOG_CRIT,
	    "Anoubis Daemon failed to terminate\n");
	return 2;
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
fetch_versions(int * apn_version, int * prot_version)
{

	struct anoubis_transaction	*t;
	struct anoubis_msg		*m;

	if (!client)
		return (3);

	t = anoubis_client_version_start(client);
	if (!t) {
		return (3);
	}

	while(1) {
		int ret = anoubis_client_wait(client);
		if (ret <= 0) {
			anoubis_transaction_destroy(t);
			return (3);
		}
		if (t->flags & ANOUBIS_T_DONE)
			break;
	}

	if (t->result) {
		fprintf(stderr, "Version Request failed: %d (%s)\n",
		    t->result, anoubis_strerror(t->result));
		anoubis_transaction_destroy(t);
		return (3);
	}

	m = t->msg;
	t->msg = NULL;
	anoubis_transaction_destroy(t);
	if (!m || !VERIFY_LENGTH(m, sizeof(Anoubis_VersionMessage))
	    || get_value(m->u.version->error) != 0) {
		fprintf(stderr, "Error retrieving version information\n");
		free_msg_list(m);
		return (3);
	}

	if (apn_version)
		*apn_version = get_value(m->u.version->apn);
	if (prot_version)
		*prot_version = get_value(m->u.version->protocol);

	free_msg_list(m);

	return (0);
}

static int
daemon_version(void)
{
	int		apn_daemon_version;
	int		apn_client_version;
	int		prot_daemon_version;
	int		prot_client_version;
	int		error = 0;

	error = create_channel(0);
	if (error) {
		fprintf(stderr, "Cannot connect to " PACKAGE_DAEMON "\n");
		return error;
	}

	error = fetch_versions(&apn_daemon_version, &prot_daemon_version);

	if (error) {
		fprintf(stderr, "fetch_versions() failed\n");
		destroy_channel();
		return (error);
	}

	/* Package version, independent from connection-state */
	fprintf(stdout, "Package: " PACKAGE_VERSION "\n");
	fprintf(stdout, "Build: " PACKAGE_BUILD "\n");

	/* Anoubis protocol version */
	prot_client_version = ANOUBIS_PROTO_VERSION;

	if (!client) {
		fprintf(stdout, "Anoubis protocol: disconnected (local: %i)\n",
		    prot_client_version);
	} else if (prot_client_version != prot_daemon_version) {
		fprintf(stdout, "Anoubis protocol: mismatch (local: %i -- "
		    "daemon: %i)\n", prot_client_version, prot_daemon_version);
	} else {
		fprintf(stdout, "Anoubis protocol: %i\n", prot_client_version);
	}

	/* APN-parser version */
	apn_client_version = apn_parser_version();

	if (!client) {
		fprintf(stdout, "APN parser: disconnected "
		    "(local: %i.%i)\n",
		    APN_PARSER_MAJOR(apn_client_version),
		    APN_PARSER_MINOR(apn_client_version));
	} else if (apn_client_version != apn_daemon_version) {
		fprintf(stdout, "APN parser: mismatch "
		    "(local: %i.%i ,daemon: %i.%i)\n",
		    APN_PARSER_MAJOR(apn_client_version),
		    APN_PARSER_MINOR(apn_client_version),
		    APN_PARSER_MAJOR(apn_daemon_version),
		    APN_PARSER_MINOR(apn_daemon_version));
	} else {
		fprintf(stdout, "APN parser: %i.%i\n",
		    APN_PARSER_MAJOR(apn_client_version),
		    APN_PARSER_MINOR(apn_client_version));
	}

	destroy_channel();

	return (0);
}

static int
dump(char *file, uid_t uid, unsigned int prio)
{
	int	error = 0;
	FILE * fp = NULL;
	struct anoubis_transaction * t;
	struct anoubis_msg * m;
	Policy_GetByUid req;
	size_t	len;

	error = create_channel(1);
	if (error) {
		fprintf(stderr, "Cannot connect to " PACKAGE_DAEMON "\n");
		return error;
	}
	set_value(req.ptype, ANOUBIS_PTYPE_GETBYUID);
	set_value(req.uid, uid);

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
		    t->result, anoubis_strerror(t->result));
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
	fprintf(fp, "# Policies for uid %d prio %d\n",
	    uid, prio);
	while (m) {
		struct anoubis_msg * tmp;
		len = m->length - CSUM_LEN - sizeof(Anoubis_PolicyReplyMessage);
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

	if (fp)
		fclose(fp);
	destroy_channel();
	return 0;
}

static int
make_version(struct apn_ruleset *ruleset)
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

	vm = apnvm_init(cvsroot, user, NULL);
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

	vmrc = apnvm_insert(vm, user, "active", ruleset, &md);
	if (vmrc != APNVM_OK) {
		fprintf(stderr, "Failed to create a new version\n");
		apnvm_destroy(vm);
		return (0);
	}

	apnvm_destroy(vm);
	return (1);
}

static int
load(char *rulesopt, uid_t uid, unsigned int prio)
{
	int fd;
	unsigned siglen = 0;
	struct stat statbuf;
	size_t length, total;
	char * buf = NULL;
	unsigned char *signature = NULL;
	Policy_SetByUid * req;
	struct anoubis_transaction *t;
	struct apn_ruleset *ruleset = NULL;
	int error, flags = 0;

	if (rulesopt == NULL) {
		fprintf(stderr, "Need a rules file");
		return 3;
	}

	if ((opts & ANOUBISCTL_OPT_SIGN) && (as == NULL)) {
		fprintf(stderr, "To load signed policies to the daemon you "
		    "need to specify a keyfile and a certficate\n");
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
	if (prio == ANOUBISCTL_PRIO_ADMIN && !(opts & ANOUBISCTL_OPT_SIGNONLY))
		flags |= APN_FLAG_NOASK;
	if (apn_parse(rulesopt, &ruleset, flags)) {
		if (ruleset) {
			if (apn_print_errors(ruleset, stderr) == 0)
				fprintf(stderr, "FATAL: parse error\n");
		} else
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

	if (!(opts & ANOUBISCTL_OPT_SIGNONLY)) {
		error = create_channel(1);
		if (error) {
			fprintf(stderr, "Cannot connect to " PACKAGE_DAEMON
			    "\n");
			apn_free_ruleset(ruleset);
			return error;
		}

		/*
		 * Do NOT abort on failure. make_version will print an error
		 * message but we should load the rule set anyway.
		 */
		make_version(ruleset);
	}
	apn_free_ruleset(ruleset);

	if (opts & ANOUBISCTL_OPT_SIGN) {
		signature = anoubis_sign_policy(as, rulesopt, &siglen);
		if (!signature) {
			fprintf(stderr, "Error while signing policy\n");
			return 3;
		}
	}

	if (opts & ANOUBISCTL_OPT_SIGNONLY) {
		size_t written = 0;
		if (!signature || siglen == 0) {
			fprintf(stderr, "BUG: No signature.\n");
			return 4;
		}
		if (strcmp("-", sigfile) != 0)
			fd = open(sigfile, O_WRONLY|O_CREAT|O_TRUNC,
			    S_IRUSR|S_IWUSR);
		else
			fd = 1;
		if (fd < 0) {
			perror("open");
			return 3;
		}
		while (written < siglen) {
			ssize_t count;
			count = write(fd, signature, siglen);
			if (count <= 0) {
				perror("write");
				return 3;
			}
			written += count;
		}
		if (fd != 1)
			return close(fd);
		else
			return 0;
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
	total = sizeof(*req) + length + siglen;
	req = malloc(total);
	if (!req) {
		fprintf(stderr, "Out of memory\n");
		if (length)
			munmap(buf, length);
		return 3;
	}
	set_value(req->ptype, ANOUBIS_PTYPE_SETBYUID);
	set_value(req->siglen, siglen);
	/*
	 * XXX CEH: Add posibility to set admin rules for a user and
	 * XXX CEH: default rules
	 */
	set_value(req->uid, uid);
	set_value(req->prio, prio);
	if (siglen)
		memcpy(req->payload, signature, siglen);
	if (length) {
		memcpy(req->payload + siglen, buf, length);
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
		    anoubis_strerror(t->result));
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
	uid_t uid = 0;
	int delegate = 0;
	char ch;

	for(i=0; i<argc; ++i) {
		if (strcasecmp(argv[i], "all") == 0) {
			if (uid)
				usage();
			uid = -1;
			continue;
		}
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

	error = create_channel(0);
	if (error) {
		fprintf(stderr, "Cannot create channel\n");
		return error;
	}
	if (uid == 0)
		uid = geteuid();
	t = anoubis_client_register_start(client, ++nexttok, uid, 0, 0);
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
		    t->result, anoubis_strerror(t->result));
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
			__anoubis_dump(m, "", NULL);
			fflush(stdout);
			if (get_value(m->u.notify->type) != ANOUBIS_N_ASK) {
				anoubis_msg_free(m);
				continue;
			}
			if (defaulterror >= 0) {
				error = anoubis_client_notifyreply(client,
				    m->u.notify->token, defaulterror, delegate);
				anoubis_msg_free(m);
				if (error < 0) {
					fprintf(stderr, "notify reply failed "
					    "with code %d", -error);
					destroy_channel();
					return 5;
				}
			} else {
				anoubis_msg_free(m);
			}
		}
		if (maxcount > 0 && count >= maxcount)
			break;
	}
	destroy_channel();
	return 0;
}

static int
send_passphrase(void)
{
	char				*pass;
	int				 error;
	struct anoubis_transaction	*t;
	char				 passbuf[1024];

	error = create_channel(0);
	if (error) {
		fprintf(stderr, "Cannot create channel\n");
		return error;
	}
	if (getuid() != 0) {
		fprintf(stderr, "Need root privileges to do this\n");
		return 5;
	}
	if (passfd < 0) {
		pass = getpass("Enter Passphrase for root's private key: ");
		if (!pass) {
			fprintf(stderr, "Failed to read password\n");
			return 5;
		}
	} else {
		int	off=0, ret;
		while(off < (int)sizeof(passbuf)) {
			ret = read(passfd, passbuf + off, sizeof(passbuf)-off);
			if (ret == 0)
				break;
			if (ret < 0) {
				memset(passbuf, 0, sizeof(passbuf));
				fprintf(stderr, "Failed to read passphrase");
				return 5;
			}
			off += ret;
		}
		/*
		 * Accept at most 1023 bytes of NUL and non-NUL terminated
		 * data from a passfd
		 */
		if (off >= (int)sizeof(passbuf))
			off = sizeof(passbuf)-1;
		passbuf[off] = 0;
		pass = passbuf;
		close(passfd);
	}
	t = anoubis_client_passphrase_start(client, pass);
	if (passfd < 0) {
		memset(pass, 0, strlen(pass));
	} else {
		memset(passbuf, 0, sizeof(passbuf));
	}
	if (!t) {
		fprintf(stderr, "Out of memory?");
		return 5;
	}
	while(1) {
		int ret = anoubis_client_wait(client);
		if (ret <= 0) {
			fprintf(stderr, "Communication error\n");
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
		perror("Communication error");
		return 5;
	}
	destroy_channel();
	return 0;
}

static int
auth_callback(struct anoubis_client *client __used, struct anoubis_msg *in,
    struct anoubis_msg **outp)
{
	int	rc	= -1;
	int	flags	=  0;

	/* Try to load key if absent. */
	if (as == NULL || as->pkey == NULL) {
		if (!certfile || !keyfile) {
			fprintf(stderr, "Key based authentication required "
			    "but no cert/key given\n");
			return -EPERM;
		}
		if (as)
			anoubis_sig_free(as);
		rc = anoubis_sig_create(&as, keyfile, certfile, pass_cb);
		if (rc < 0 || as == NULL || as->pkey == NULL) {
			fprintf(stderr, "Error while loading cert/key required"
			    " for key based authentication (error=%d)\n", -rc);
			return -EPERM;
		}
	}

	if ((opts & ANOUBISCTL_OPT_FORCE) != 0) {
		flags = ANOUBIS_AUTHFLAG_IGN_KEY_MISMATCH;
	}
	rc = anoubis_auth_callback(as, as, in, outp, flags);

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
		fprintf(stderr, "The daemon key and the key used by anoubisctl"
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
		    "An unknown error (%i) occured during authentication\n",
		    -rc);
		rc = -EINVAL;
		break;
	}

	return (rc);
}

/*
 * If check_parser_version is passed to create_channel() then
 * a check of APN versions is performed
 */
static int
create_channel(unsigned int check_parser_version)
{
	int				error = 0;
	int				apn_daemon_version;
	int				apn_client_version;
	achat_rc			rc;
	struct sockaddr_un		ss;

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
	ss.sun_family = AF_UNIX;

	strlcpy((&ss)->sun_path, PACKAGE_SOCKET,
	    sizeof((&ss)->sun_path));

	if (opts & ANOUBISCTL_OPT_VERBOSE2)
		fprintf(stderr, "acc_setaddr\n");
	rc = acc_setaddr(channel, (struct sockaddr_storage *)&ss,
	     sizeof(struct sockaddr_un));
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
	client = anoubis_client_create(channel, ANOUBIS_AUTH_TRANSPORTANDKEY,
	    &auth_callback);
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
		if (error == -EPROTONOSUPPORT)
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

	/*
	 * Compare the version of the daemon to our own one, so we can bailout
	 * in case of incompatible versions.
	 */

	if (check_parser_version) {

		apn_client_version = apn_parser_version();
		error = fetch_versions(&apn_daemon_version, NULL);

		if (error) {
			destroy_channel();
			errno = error;
			syslog(LOG_WARNING,
			    "Error fetching Anoubis daemon version");
			return (error);
		}

		if ((apn_daemon_version) != apn_client_version) {
			destroy_channel();
			syslog(LOG_WARNING, "APN parser version mismatch: "
			    "local: %i.%i, daemon: %i.%i",
			    APN_PARSER_MAJOR(apn_client_version),
			    APN_PARSER_MINOR(apn_client_version),
			    APN_PARSER_MAJOR(apn_daemon_version),
			    APN_PARSER_MINOR(apn_daemon_version));
			return 5;
		}
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

static int
moo(void)
{
	fprintf(stderr,
	"\n"
	" _ __ ___   ___   ___\n"
	"| '_ ` _ \\ / _ \\ / _ \\\n"
	"| | | | | | (_) | (_) |\n"
	"|_| |_| |_|\\___/ \\___/\n"
	"\n\n"
	"P.S. no real cows were harmed for this moo\n");

	return 0;
}

static int
verify(const char *filename, const char *signature)
{
	int		 fd, off = 0, ret;
	EVP_PKEY	*pubkey = X509_get_pubkey(as->cert);
	void		*sigbuf;
	int		 siglen;

	if (!pubkey) {
		fprintf(stderr, "Failed to read public key\n");
		return 1;
	}
	siglen = EVP_PKEY_size(pubkey);
	sigbuf = malloc(siglen);
	if (sigbuf == NULL) {
		fprintf(stderr, "Out of memory\n");
		EVP_PKEY_free(pubkey);
		return 1;
	}
	fd = open(signature, O_RDONLY);
	if (fd < 0) {
		free(sigbuf);
		fprintf(stderr, "Cannot open signature file %s\n", signature);
		EVP_PKEY_free(pubkey);
		return 1;
	}
	while (off < siglen) {
		ret = read(fd, sigbuf + off, siglen - off);
		if (ret < 0) {
			perror("Failed to read signature data\n");
			close(fd);
			free(sigbuf);
			EVP_PKEY_free(pubkey);
			return 1;
		}
		if (ret == 0) {
			fprintf(stderr, "Short signature file\n");
			close(fd);
			free(sigbuf);
			EVP_PKEY_free(pubkey);
			return 1;
		}
		off += ret;
	}
	close(fd);
	ret = anoubis_sig_verify_policy(filename, sigbuf, siglen, pubkey);
	if (ret != 1) {
		fprintf(stderr, "Signature verification failed\n");
		free(sigbuf);
		EVP_PKEY_free(pubkey);
		return 1;
	}
	free(sigbuf);
	EVP_PKEY_free(pubkey);
	printf("Signature verification ok\n");
	return 0;
}

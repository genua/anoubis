/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <anoubis_apnvm.h>
#include <anoubis_auth.h>
#include <anoubis_chat.h>
#include <anoubis_client.h>
#include <anoubis_errno.h>
#include <anoubis_msg.h>
#include <anoubis_playground.h>
#include <anoubis_sig.h>
#include <anoubis_transaction.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#ifdef LINUX
#include <bsdcompat.h>
#define __dead	__attribute__((__noreturn__))
#endif

/**
 * CLI option flag: force action
 */
#define PGCLI_OPT_FORCE 0x0001

/**
 * CLI option flag: use given certificate
 */
#define PGCLI_OPT_CERT 0x0002

/**
 * CLI option flag: use given key
 */
#define PGCLI_OPT_SIGN 0x0004

/**
 * Field width for playground id.
 */
#define PGCLI_OUTLEN_ID 8

/**
 * Field width for user id.
 */
#define PGCLI_OUTLEN_USER 5

/**
 * Field width for status.
 */
#define PGCLI_OUTLEN_STAT 8

/**
 * Field width for number of files.
 */
#define PGCLI_OUTLEN_FILES 5

/**
 * Field width for time.
 */
#define PGCLI_OUTLEN_TIME 20

/**
 * Field width for command.
 */
#define PGCLI_OUTLEN_COMMAND 8

/**
 * Field width for device.
 */
#define PGCLI_OUTLEN_DEV 8

/**
 * Field width for filename.
 */
#define PGCLI_OUTLEN_FILENAME 8

/**
 * Length of buffer for time printing.
 */
#define PGCLI_TBUF_LEN 32

/**
 * Macro for cleanup on channel create.
 */
#define PGCLI_CHANNELCREATE_WIPE(channel, rc, msg) \
	do { \
		if (rc != ACHAT_RC_OK) { \
			fprintf(stderr, msg); \
			acc_destroy(channel); \
			return (NULL); \
		} \
	} while (0)

/**
 * Macro for cleanup of channel and client.
 */
#define PGCLI_CONNECTION_WIPE(channel, client) \
	do { \
		anoubis_client_close(client); \
		anoubis_client_destroy(client); \
		acc_destroy(channel); \
	} while (0)

/**
 * Usage of program 'playground'.
 * This will print out usage information to stderr and exit.
 * The exit code is 1.
 * @param None.
 * @return Nothing.
 */
void usage(void) __dead;

/**
 * Anoubis playground CLI.
 * This program starts new playgrounds.
 * @param[in] 1st Number of arguments.
 * @param[in] 2nd Arrya of strings with command line arguments.
 * @return 0 on success.
 */
int main(int, char **);

#ifdef LINUX

/**
 * Initialize ui.
 * This function reads the persisten user data from his home.
 * @paran None.
 * @return 0 on success, 1 in case of error.
 */
static int pgcli_ui_init(void);

/**
 * Create anoubischant channel.
 * This function creates and opens a new chat channel to the daemon.
 * This is the first step in communication with the daemon
 * @param None.
 * @return The created channel or NULL on error.
 */
static struct achat_channel *pgcli_create_channel(void);

/**
 * Create client.
 * This function creates a new protocol client and connects it with the
 * daemon. After this function the protocol is initializes and other
 * messages cat be transmitted.
 * @param[in] 1st An open channel to the daemon.
 * @return A connected client or NULL in case of error.
 */
static struct anoubis_client *pgcli_create_client(struct achat_channel *);

/**
 * Free list of messages.
 * This function will iterate ofer the list of messages starting with
 * the given root element and will free each message in the list.
 * @param[in] 1st Root of list of messages.
 * @return Nothing.
 */
static void pgcli_msglist_free(struct anoubis_msg *);

/**
 * Command: list
 * This function implements the list command. Communication with the
 * daemon is established and the list of playgrounds is requested. The
 * received messages is printed by pgcli_list_print() and the communication
 * is closed.
 * @param None.
 * @return 0 on success an error else.
 * @see pgcli_list_print()
 */
static int pgcli_list(void);

/**
 * Print playground list.
 * This function will print the header information and iterates over the
 * given list of messages. Those a split in records for each playground
 * and printed by pgcli_list_print_record().
 * The printed headers are:\n
 *	- playground id
 *	- user id
 *	- status: active or inactive
 *	- number of (modified) files
 *	- time of creation
 *	- command\n\n
 * @param[in] 1st The first message in list.
 * @return Nothing.
 */
static void pgcli_list_print_msg(struct anoubis_msg *);

/**
 * Print playground list record.
 * This function prints the data of one playground record.
 * The result is a single line in the list of playgrounds.
 * No header information are printed.\n
 * The printed values are:\n
 *	- playground id
 *	- user id
 *	- status: active or inactive
 *	- number of (modified) files
 *	- time of creation
 *	- command\n\n
 * @param[in] 1st The record of one playground.
 * @return Nothing.
 */
static void pgcli_list_print_record(Anoubis_PgInfoRecord *);

/**
 * Command: files
 * This function implements the files command. Communication with the
 * daemon is established and the list of files of a playground is requested.
 * The  received messages is printed by pgcli_files_print() and the
 * communication is closed.
 * @param[in] 1st The id of playground in question.
 * @return 0 on success an error else.
 * @see pgcli_files_print()
 */
static int pgcli_files(anoubis_cookie_t);

/**
 * Print playground file list.
 * This function will print the header information and iterates over the
 * given list of messages. Those a split in records for each file
 * and printed by pgcli_files_print_record().
 * The printed headers are:\n
 *	- playground id
 *	- device number
 *	- path\n\n
 * @param[in] 1st The first message in list.
 * @return Nothing.
 */
static void pgcli_files_print_msg(struct anoubis_msg *);

/**
 * Print playground file record.
 * This function prints the data of one file record.
 * The result is a single line in the list of files.
 * No header information are printed.\n
 * The printed values are:\n
 *	- playground id
 *	- device number
 *	- path\n\n
 * @param[in] 1st The record of one playground.
 * @return Nothing.
 */
static void pgcli_files_print_record(Anoubis_PgFileRecord *);

static int auth_callback(struct anoubis_client *, struct anoubis_msg *,
    struct anoubis_msg **);

static int	 opts = 0;
static char	*keyFile = NULL;
static char	*certFile = NULL;

static const char defaultCertFile[] = ".xanoubis/default.crt";
static const char defaultKeyFile[]  =  ".xanoubis/default.key";

#endif /* LINUX */

extern char	*__progname;

__dead void
usage(void)
{
	fprintf(stderr, "usage: %s [-fh] [-k key] [-c cert] <command> "
	    "[<program>]\n\n", __progname);
	fprintf(stderr, "    -f:\tforce\n");
	fprintf(stderr, "    -h:\tthis help\n");
	fprintf(stderr, "    -k key:\tprivate key file\n");
	fprintf(stderr, "    -c cert:\tcertificate file\n\n");
	fprintf(stderr, "    <command>:\n");
	fprintf(stderr, "	start <program>\n");
	fprintf(stderr, "	list\n");
	fprintf(stderr, "	files <playground id>\n");

	exit(1);
}

int
main(int argc, char *argv[])
{
	int error = 0;

#ifdef OPENBSD

	fprintf(stderr, "Anoubis playground is not supported on OpenBSD.\n");
	fprintf(stderr, "Program %s (%d args) not started.\n", argv[0], argc);
	error = 1;

#else

	anoubis_cookie_t pgid = 0;
	int		 ch;
	char		*command = NULL;

	if (argc < 2) {
		usage();
		/* NOTREACHED */
	}

	/* Get command line arguments. */
	while ((ch = getopt(argc, argv, "fhc:k:")) != -1) {
		switch (ch) {
		case 'f':
			opts |= PGCLI_OPT_FORCE;
			break;
		case 'h':
			usage();
			break;
		case 'c':
			opts |= PGCLI_OPT_CERT;
			opts |= PGCLI_OPT_SIGN;
			certFile = optarg;
			if (!certFile) {
				perror(optarg);
				return 1;
			}
			break;
		case 'k':
			opts |= PGCLI_OPT_SIGN;
			keyFile = optarg;
			if (!keyFile) {
				perror(optarg);
				return 1;
			}
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	command = *argv++;
	argc--;

	openlog(__progname, LOG_ODELAY|LOG_PERROR, LOG_USER);

	/* Run command. */
	if (strcmp(command, "start") == 0) {
		if (argc < 1) {
			usage();
			/* NOTREACHED */
		}
		error = playground_start_exec(argv);
	} else if (strcmp(command, "list") == 0) {
		if (argc != 0) {
			usage();
			/* NOTREACHED */
		}
		error = pgcli_list();
	} else if (strcmp(command, "files") == 0) {
		if (argc < 1) {
			usage();
			/* NOTREACHED */
		}
		if (sscanf(argv[0], "%"PRIx64, &pgid) != 1) {
			usage();
			/* NOTREACHED */
		}
		error = pgcli_files(pgid);
	} else {
		usage();
	}

	if (error != 0) {
		errno = -error;
		perror(command);
	}

#endif /* OPENBSD */

	return (error);
}

#ifdef LINUX

int
pgcli_ui_init(void)
{
	int		 error = 0;
	struct stat	 sb;
	char		*homePath = NULL;

	if ((homePath = getenv("HOME")) == NULL) {
		fprintf(stderr, "Error: HOME environment variable not set\n");
		return 1;
	}

	if (anoubis_ui_init() < 0) {
		fprintf(stderr, "Error while initialising anoubis_ui\n");
		return 1;
	}

	error = anoubis_ui_readversion();
	if (error > ANOUBIS_UI_VER) {
		syslog(LOG_WARNING, "Unsupported version (%d) of %s/%s found.",
		    error, homePath, ANOUBIS_UI_DIR);
		return 1;
	}
	if (error < 0) {
		syslog(LOG_WARNING, "Error reading %s/%s version: %s\n",
		    homePath, ANOUBIS_UI_DIR, anoubis_strerror(-error));
		return 1;
	}

	if (certFile == NULL) {
		error = asprintf(&certFile, "%s/%s", homePath, defaultCertFile);
		if (error < 0) {
			fprintf(stderr, "Error while allocating memory\n");
			return 1;
		}
		if (stat(certFile, &sb) == 0) {
			opts |= PGCLI_OPT_SIGN;
		} else {
			free(certFile);
			certFile = NULL;
		}
	}

	if (keyFile == NULL) {
		error = asprintf(&keyFile, "%s/%s", homePath, defaultKeyFile);
		if (error < 0) {
			fprintf(stderr, "Error while allocating" "memory\n");
			return 1;
		}
		if (stat(keyFile, &sb) == 0) {
			opts |= PGCLI_OPT_SIGN;
		} else {
			free(keyFile);
			keyFile = NULL;
		}
	}

	return 0;
}

struct achat_channel *
pgcli_create_channel(void)
{
	achat_rc		 rc = 0;
	struct sockaddr_un	 ss;
	struct achat_channel	*channel = NULL;

	bzero(&ss, sizeof(ss));
	channel = acc_create();
	if (channel == NULL) {
		fprintf(stderr, "cannot create client channel\n");
		return (NULL);
	}

	rc = acc_settail(channel, ACC_TAIL_CLIENT);
	PGCLI_CHANNELCREATE_WIPE(channel, rc, "client settail failed\n");

	rc = acc_setsslmode(channel, ACC_SSLMODE_CLEAR);
	PGCLI_CHANNELCREATE_WIPE(channel, rc, "client setsslmode failed\n");

	ss.sun_family = AF_UNIX;
	strlcpy((&ss)->sun_path, PACKAGE_SOCKET, sizeof((&ss)->sun_path));

	rc = acc_setaddr(channel, (struct sockaddr_storage *)&ss,
	    sizeof(struct sockaddr_un));
	PGCLI_CHANNELCREATE_WIPE(channel, rc, "client setaddr failed\n");

	rc = acc_prepare(channel);
	PGCLI_CHANNELCREATE_WIPE(channel, rc, "client prepare failed\n");

	rc = acc_open(channel);
	PGCLI_CHANNELCREATE_WIPE(channel, rc, "client open failed\n");

	return (channel);
}

struct anoubis_client *
pgcli_create_client(struct achat_channel *channel)
{
	int			 rc = 0;
	struct anoubis_client	*client = NULL;

	client = anoubis_client_create(channel, ANOUBIS_AUTH_TRANSPORTANDKEY,
	    &auth_callback);
	if (client == NULL) {
		fprintf(stderr, "anoubis_client_create failed\n");
		return (NULL);
	}

	rc = anoubis_client_connect(client, ANOUBIS_PROTO_BOTH);
	if (rc == -EPROTONOSUPPORT) {
		syslog(LOG_WARNING, "Anoubis protocol mismatch: "
		    "local: %d (min %d) -- daemon: %d (min %d)",
		    ANOUBIS_PROTO_VERSION, ANOUBIS_PROTO_MINVERSION,
		    anoubis_client_serverversion(client),
		    anoubis_client_serverminversion(client));
	}
	if (rc != 0) {
		anoubis_client_destroy(client);
		errno = -rc;
		perror("anoubis_client_connect");
		return (NULL);
	}

	return (client);
}

void
pgcli_msglist_free(struct anoubis_msg *message)
{
	struct anoubis_msg *tmp;
	while (message) {
		tmp = message;
		message = message->next;
		anoubis_msg_free(tmp);
	}
}

int
pgcli_list(void)
{
	int rc = 0;

	struct achat_channel		*channel = NULL;
	struct anoubis_client		*client = NULL;
	struct anoubis_msg		*message = NULL;
	struct anoubis_transaction	*transaction = NULL;

	rc = pgcli_ui_init();
	if (rc != 0) {
		return (-1);
	}

	channel = pgcli_create_channel();
	if (channel == NULL) {
		return (-EIO);
	}

	client = pgcli_create_client(channel);
	if (client == NULL) {
		acc_destroy(channel);
		return (-EIO);
	}

	/* Connection established. Send message and wait for answer. */
	transaction = anoubis_client_pglist_start(client,
	    ANOUBIS_PGREC_PGLIST, 0);
	if (transaction == NULL) {
		PGCLI_CONNECTION_WIPE(channel, client);
		return (-EFAULT);
	}
	while (1) {
		rc = anoubis_client_wait(client);
		if (rc <= 0) {
			anoubis_transaction_destroy(transaction);
			PGCLI_CONNECTION_WIPE(channel, client);
			return (rc);
		}
		if (transaction->flags & ANOUBIS_T_DONE) {
			break;
		}
	}
	message = transaction->msg;
	transaction->msg = NULL;

	/* Print received list. */
	if (transaction->result == 0) {
		pgcli_list_print_msg(message);
	} else {
		fprintf(stderr, "Playground list request failed: %d (%s)\n",
		    transaction->result, anoubis_strerror(transaction->result));
	}

	/* Cleanup. */
	pgcli_msglist_free(message);
	anoubis_transaction_destroy(transaction);
	PGCLI_CONNECTION_WIPE(channel, client);

	return (0);
}

void
pgcli_list_print_msg(struct anoubis_msg *message)
{
	int			 offset = 0;
	int			 i	= 0;
	Anoubis_PgInfoRecord	*record = NULL;

	if (message == NULL ||
	    !VERIFY_LENGTH(message, sizeof(Anoubis_PgReplyMessage)) ||
	    get_value(message->u.pgreply->error) != 0) {
		fprintf(stderr, "Error retrieving playground list\n");
		return;
	}

	if (get_value(message->u.pgreply->rectype) != ANOUBIS_PGREC_PGLIST) {
		fprintf(stderr, "Error: wrong record type in received list.\n");
		return;
	}

	/* Print header */
	printf("%*s ", PGCLI_OUTLEN_ID, "PGID");
	printf("%*s ", PGCLI_OUTLEN_USER, "USER");
	printf("%*s ", PGCLI_OUTLEN_STAT, "STAT");
	printf("%*s ", PGCLI_OUTLEN_FILES, "FILES");
	printf("%*s ", PGCLI_OUTLEN_TIME, "TIME");
	printf("%-*s\n", PGCLI_OUTLEN_COMMAND, "COMMAND");

	/* Print table lines */
	while (message) {
		for (i=0; i<get_value(message->u.pgreply->nrec); ++i) {
			record = (Anoubis_PgInfoRecord *)(
			    message->u.pgreply->payload + offset);
			offset += get_value(record->reclen);

			pgcli_list_print_record(record);
		}

		message = message->next;
	}
}

void
pgcli_list_print_record(Anoubis_PgInfoRecord *record)
{
	char	timeBufer[PGCLI_TBUF_LEN];
	time_t	time = 0;

	printf("%*"PRIx64" ", PGCLI_OUTLEN_ID, get_value(record->pgid));
	printf("%*d ", PGCLI_OUTLEN_USER, get_value(record->uid));

	/* Print status: no processes == inactive */
	if (get_value(record->nrprocs) == 0) {
		printf("%*s ", PGCLI_OUTLEN_STAT, "inactive");
	} else {
		printf("%*s ", PGCLI_OUTLEN_STAT, "active");
	}

	printf("%*d ", PGCLI_OUTLEN_FILES, get_value(record->nrfiles));

	/* Print time of creation. */
	time = get_value(record->starttime);
	strftime(timeBufer, PGCLI_TBUF_LEN, "%F %T", localtime(&time));
	printf("%*s ", PGCLI_OUTLEN_TIME, timeBufer);

	printf("%-*s\n", PGCLI_OUTLEN_COMMAND, record->path);
}

int
pgcli_files(anoubis_cookie_t pgid)
{
	int rc = 0;

	struct achat_channel		*channel = NULL;
	struct anoubis_client		*client = NULL;
	struct anoubis_msg		*message = NULL;
	struct anoubis_transaction	*transaction = NULL;

	rc = pgcli_ui_init();
	if (rc != 0) {
		return (-1);
	}

	channel = pgcli_create_channel();
	if (channel == NULL) {
		return (-EIO);
	}

	client = pgcli_create_client(channel);
	if (client == NULL) {
		acc_destroy(channel);
		return (-EIO);
	}

	/* Connection established. Send message and wait for answer. */
	transaction = anoubis_client_pglist_start(client,
	    ANOUBIS_PGREC_FILELIST, pgid);
	if (transaction == NULL) {
		PGCLI_CONNECTION_WIPE(channel, client);
		return (-EFAULT);
	}
	while (1) {
		rc = anoubis_client_wait(client);
		if (rc <= 0) {
			anoubis_transaction_destroy(transaction);
			PGCLI_CONNECTION_WIPE(channel, client);
			return (rc);
		}
		if (transaction->flags & ANOUBIS_T_DONE) {
			break;
		}
	}
	message = transaction->msg;
	transaction->msg = NULL;

	/* Print received list. */
	if (transaction->result == 0) {
		pgcli_files_print_msg(message);
	} else {
		fprintf(stderr, "Playground file list request failed: "
		    "%d (%s)\n", transaction->result,
		    anoubis_strerror(transaction->result));
	}

	/* Cleanup. */
	pgcli_msglist_free(message);
	anoubis_transaction_destroy(transaction);
	PGCLI_CONNECTION_WIPE(channel, client);

	return (0);
}

void
pgcli_files_print_msg(struct anoubis_msg *message)
{
	int			 offset = 0;
	int			 i	= 0;
	Anoubis_PgFileRecord	*record = NULL;

	if (message == NULL ||
	    !VERIFY_LENGTH(message, sizeof(Anoubis_PgReplyMessage)) ||
	    get_value(message->u.pgreply->error) != 0) {
		fprintf(stderr, "Error retrieving playground list\n");
		return;
	}

	if (get_value(message->u.pgreply->rectype) != ANOUBIS_PGREC_FILELIST) {
		fprintf(stderr, "Error: wrong record type in received list.\n");
		return;
	}

	/* Print header */
	printf("%*s ", PGCLI_OUTLEN_ID, "PGID");
	printf("%*s ", PGCLI_OUTLEN_DEV, "DEV");
	printf("%-*s\n", PGCLI_OUTLEN_FILENAME, "FILE");

	/* Print table lines */
	while (message) {
		for (i=0; i<get_value(message->u.pgreply->nrec); ++i) {
			record = (Anoubis_PgFileRecord *)(
			    message->u.pgreply->payload + offset);
			offset += get_value(record->reclen);

			pgcli_files_print_record(record);
		}

		message = message->next;
	}
}

void
pgcli_files_print_record(Anoubis_PgFileRecord *record)
{
	printf("%*"PRIx64" ", PGCLI_OUTLEN_ID, get_value(record->pgid));
	printf("%*"PRIx64" ", PGCLI_OUTLEN_DEV, get_value(record->dev));
	printf("%-*s\n", PGCLI_OUTLEN_FILENAME, record->path);
}

static int
auth_callback(struct anoubis_client *client __used, struct anoubis_msg *in,
    struct anoubis_msg **outp)
{
	int	rc	= -1;
	int	flags	=  0;

	struct anoubis_sig *as = NULL;

	/* Try to load key if absent. */
	if (as == NULL || as->pkey == NULL) {
		if (!certFile || !keyFile) {
			fprintf(stderr, "Key based authentication required "
			    "but no cert/key given\n");
			return -EPERM;
		}
		if (as)
			anoubis_sig_free(as);
		rc = anoubis_sig_create(&as, keyFile, certFile, pass_cb);
		if (rc < 0 || as == NULL || as->pkey == NULL) {
			fprintf(stderr, "Error while loading cert/key required"
			    " for key based authentication (error=%d)\n", -rc);
			return -EPERM;
		}
	}

	if ((opts & PGCLI_OPT_FORCE) != 0) {
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

#endif /* LINUX */

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
#include <linux/anoubis_playground.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#ifdef LINUX
#include <bsdcompat.h>
#include <attr/xattr.h>
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
 * CLI option flag: generate verbose output.
 */
#define PGCLI_OPT_VERBOSE 0x0008

/**
 * CLI option flag: Ignore recommended scanner during commit
 */
#define PGCLI_OPT_IGNRECOMM 0x0010

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
 * Field width for inode number.
 */
#define PGCLI_OUTLEN_INODE 8

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
 * This function reads the persistent user data from his home.
 * @paran None.
 * @return 0 on success, 1 in case of error. The reason for the error
 *     is reported by the function itself.
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
 * Command: list
 * This function implements the list command. Communication with the
 * daemon is established and the list of playgrounds is requested. The
 * received messages is printed by pgcli_list_print() and the communication
 * is closed.
 * @param None.
 * @return 0 on success, a negative error code if an error occured,
 *     that must be reported by the caller and a positiv value if an error
 *     occured that must lead to a non-zero exist status but was already
 *     reported by this function.
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
 * @return 0 on success, a negative error code if an error occured,
 *     that must be reported by the caller and a positiv value if an error
 *     occured that must lead to a non-zero exist status but was already
 *     reported by this function.
 * @see pgcli_files_print()
 */
static int pgcli_files(anoubis_cookie_t);

/**
 * Command: remove
 * This function implements the remove command. It remove an entire
 * playground, i.e. it removes all files associated with that playground.
 *
 * @param[in] 1st The id of the playground in question.
 * @return 0 on success, a negative error code if an error occured,
 *     that must be reported by the caller and a positiv value if an error
 *     occured that must lead to a non-zero exit status but was already
 *     reported by this function.
 */
static int pgcli_remove(anoubis_cookie_t);

/**
 * Command: commit
 * This function implements the commit command. It removes the playground
 * label from a single file in a particular playground and moves the
 * file from its playground name to the real name.
 *
 * @param pgid The playground ID of the playground.
 * @param file The file name to commit.
 * @return 0 on success, a negative error code if an error occured,
 *     that must be reported by the caller and a positiv value if an error
 *     occured that must lead to a non-zero exit status but was already
 *     reported by this function.
 */
static int pgcli_commit(uint64_t pgid, const char *file);

/**
 * Command: delete
 * This functions implements the delete command. It deletes single files
 * within a playground.
 * @param[in] 1st The id of the playground in question.
 * @param[in] 2nd The number of file arguments in 3rd parameter
 * @param[in] 3rd The filenames of the files to delete.
 */
static int pgcli_delete(anoubis_cookie_t, int, char**);

/**
 * Print playground file list.
 * This function will print the header information and iterates over the
 * given list of messages. Those a split in records for each file
 * and printed by pgcli_files_print_record().
 * The printed headers are:\n
 *	- playground id
 *	- device number
 *	- inode number
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
 *	- inode number
 *	- path\n\n
 * @param[in] 1st The record of one playground.
 * @param[in] 2nd If this value is true the record is only printed if it
 *     references an existing file on a mounted file system. If this value
 *     is false the record is printed in any case but without a path name.
 * @return Zero if a record for the file was printed, a negative error
 *     code if no record was printed (usually because the file name references
 *     a different file).
 */
static int pgcli_files_print_record(Anoubis_PgFileRecord *, int);

static int auth_callback(struct anoubis_client *, struct anoubis_msg *,
    struct anoubis_msg **);

static int	 opts = 0;
static char	*keyFile = NULL;
static char	*certFile = NULL;

static const char defaultCertFile[] = ".xanoubis/default.crt";
static const char defaultKeyFile[]  = ".xanoubis/default.key";

#endif /* LINUX */

extern char	*__progname;

__dead void
usage(void)
{
	fprintf(stderr, "usage: %s [-fFh] [--ignore-recommended] [-k key] "
	    "[-c cert] <command> [<program>]\n\n", __progname);
	fprintf(stderr, "    -f                        force\n");
	fprintf(stderr, "    -F, --ignore-recommended  "
	    "ignore recommended scanners during commit\n");
	fprintf(stderr, "    -h                        this help\n");
	fprintf(stderr, "    -k key                    private key file\n");
	fprintf(stderr, "    -c cert                   certificate file\n\n");
	fprintf(stderr, "    <command>:\n");
	fprintf(stderr, "	start <program>\n");
	fprintf(stderr, "	list\n");
	fprintf(stderr, "	files <playground id>\n");
	fprintf(stderr, "	remove <playground id>\n");
	fprintf(stderr, "	commit <playground id> file ...\n");
	fprintf(stderr, "	delete <playground id> file ...\n");

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

	struct option options [] = {
		{ "ignore-recommended", no_argument, NULL, 'F' },
		{ 0, 0, 0, 0 }
	};

	/* Get command line arguments. */
	while ((ch = getopt_long(argc, argv, "+vfhc:k:F",
	    options, NULL)) != -1) {
		switch (ch) {
		case 'f':
			opts |= PGCLI_OPT_FORCE;
			break;
		case 'F':
			opts |= PGCLI_OPT_IGNRECOMM;
			break;
		case 'v':
			opts |= PGCLI_OPT_VERBOSE;
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

	/* Only continue if there is at least a command */
	if (!argc)
		usage();

	command = *argv++;
	argc--;

	if (strcmp(command, "commit") && (opts & PGCLI_OPT_IGNRECOMM)) {
		/* ignore-recommended option only allowed
		 * for the commit-command */
		usage();
		/* NOTREACHED */
	}

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
		if (argc != 1) {
			usage();
			/* NOTREACHED */
		}
		if (sscanf(argv[0], "%"PRIx64, &pgid) != 1) {
			usage();
			/* NOTREACHED */
		}
		error = pgcli_files(pgid);
	} else if (strcmp(command, "remove") == 0) {
		if (argc != 1)
			usage();
		if (sscanf(argv[0], "%"PRIx64, &pgid) != 1)
			usage();
		error = pgcli_remove(pgid);
	} else if (strcmp(command, "commit") == 0) {
		char		dummy;
		int		i;

		if (argc < 2)
			usage();
		if (sscanf(argv[0], "%"PRIx64"%c", &pgid, &dummy) != 1)
			usage();
		for (i=1; i<argc; ++i)
			error = pgcli_commit(pgid, argv[i]);
	} else if (strcmp(command, "delete") == 0) {
		char		dummy;

		if (argc < 2)
			usage();
		if (sscanf(argv[0], "%"PRIx64"%c", &pgid, &dummy) != 1)
			usage();
		argc--;
		argv++;
		error = pgcli_delete(pgid, argc, argv);
	} else {
		usage();
	}

	if (error < 0) {
		fprintf(stderr, "%s: %s\n", command, anoubis_strerror(-error));
	}

#endif /* OPENBSD */
	if (error)
		return 1;
	return 0;
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
			fprintf(stderr, "Error while allocating memory\n");
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

/**
 * Read message from the client connection until either an error occurs
 * or the transaction completes. If the transaction completes successfully
 * the result can be retrieved from the message in the transaction.
 * If an error occurs, the transaction is destroyed.
 *
 * @param client The anoubis client connection.
 * @param ta The transaction the complete (NULL results in -ENOMEM).
 * @param msgp If this pointer is not NULL it is used to store the
 *     result message(s) of the transaction. In this case the result
 *     messages are not freed even if an error occurs.
 * @return Zero if the transaction completed successfully, an anoubis
 *     error code that indicates the error.
 */
static int
anoubis_transaction_complete(struct anoubis_client *client,
    struct anoubis_transaction *ta, struct anoubis_msg **msgp)
{
	int		rc;

	if (ta == NULL)
		return -ENOMEM;
	while (1) {
		rc = anoubis_client_wait(client);
		if (rc <= 0) {
			if (msgp) {
				(*msgp) = ta->msg;
				ta->msg = NULL;
			}
			anoubis_transaction_destroy(ta);
			if (rc == 0)
				rc = -EPROTO;
			return rc;
		}
		if (ta->flags & ANOUBIS_T_DONE)
			break;
	}
	rc = -ta->result;
	if (msgp) {
		(*msgp) = ta->msg;
		ta->msg = NULL;
	}
	if (rc < 0)
		anoubis_transaction_destroy(ta);
	return rc;
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
	if (rc != 0)
		return 1;

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
	transaction = anoubis_client_list_start(client,
	    ANOUBIS_REC_PGLIST, 0);
	rc = anoubis_transaction_complete(client, transaction, NULL);
	if (rc < 0) {
		PGCLI_CONNECTION_WIPE(channel, client);
		return rc;
	}

	/* Print received list. */
	message = transaction->msg;
	pgcli_list_print_msg(message);

	/* Cleanup. */
	anoubis_transaction_destroy(transaction);
	PGCLI_CONNECTION_WIPE(channel, client);

	return rc;
}

void
pgcli_list_print_msg(struct anoubis_msg *message)
{
	int			 offset;
	int			 i	= 0;
	Anoubis_PgInfoRecord	*record = NULL;

	if (message == NULL ||
	    !VERIFY_LENGTH(message, sizeof(Anoubis_ListMessage)) ||
	    get_value(message->u.listreply->error) != 0) {
		fprintf(stderr, "Error retrieving playground list\n");
		return;
	}

	if (get_value(message->u.listreply->rectype) != ANOUBIS_REC_PGLIST) {
		fprintf(stderr, "Error: wrong record type in received list.\n");
		return;
	}

	/* Print header */
	printf("%*s ", PGCLI_OUTLEN_ID, "PGID");
	printf("%*s ", PGCLI_OUTLEN_USER, "USER");
	printf("%*s ", PGCLI_OUTLEN_STAT, "STATUS");
	printf("%*s ", PGCLI_OUTLEN_FILES, "FILES");
	printf("%*s ", PGCLI_OUTLEN_TIME, "TIME");
	printf("%-*s\n", PGCLI_OUTLEN_COMMAND, "COMMAND");

	/* Print table lines */
	while (message) {
		for (offset=i=0; i<get_value(message->u.listreply->nrec); ++i) {
			record = (Anoubis_PgInfoRecord *)(
			    message->u.listreply->payload + offset);
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
	if (rc != 0)
		return 1;

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
	transaction = anoubis_client_list_start(client,
	    ANOUBIS_REC_PGFILELIST, pgid);
	rc = anoubis_transaction_complete(client, transaction, NULL);
	if (rc < 0) {
		PGCLI_CONNECTION_WIPE(channel, client);
		return rc;
	}

	/* Print received list. */
	message = transaction->msg;
	pgcli_files_print_msg(message);

	/* Cleanup. */
	anoubis_transaction_destroy(transaction);
	PGCLI_CONNECTION_WIPE(channel, client);

	return rc;
}

/**
 * Validate that the specified playground exists, belongs to the current
 * user and is not active.
 * @param pgid The playground id to validate
 * @param channel The communcation channel to use.
 * @param client  The communication client to use.
 * @return Zero if playground was successfully validated, error else.
 */
int
pgcli_validate_playground(anoubis_cookie_t pgid, struct achat_channel *channel,
    struct anoubis_client *client)
{
	struct anoubis_msg *message;
	struct anoubis_transaction *transaction;

	/* Retrieve information about the playground. */
	transaction = anoubis_client_list_start(client,
	    ANOUBIS_REC_PGLIST, pgid);
	int rc = anoubis_transaction_complete(client, transaction, NULL);
	if (rc < 0) {
		PGCLI_CONNECTION_WIPE(channel, client);
		return rc;
	}

	message = transaction->msg;
	rc = 1;
	if (get_value(message->u.listreply->nrec) == 0) {
		fprintf(stderr, "Playground %" PRIx64 " does not exist\n",
		    pgid);
	} else {
		Anoubis_PgInfoRecord    *rec;

		rec = (Anoubis_PgInfoRecord *)message->u.listreply->payload;
		if (opts & PGCLI_OPT_FORCE) {
			rc = 0;
		} else if (get_value(rec->nrprocs)) {
			fprintf(stderr,  "Cannot delete active playground %"
			    PRIx64 "\n", pgid);
		} else if (geteuid() && geteuid() != get_value(rec->uid)) {
			fprintf(stderr, "UID does not match: %d != %d\n",
			    (int)(geteuid()), (int)(get_value(rec->uid)));
			rc = -EPERM;
		} else {
			rc = 0;
		}
	}
	anoubis_transaction_destroy(transaction);

	return rc;
}

/**
 * Try to delete all files that are reported in a series of messages that
 * contain ANOUBIS_REC_PGFILELIST records. A single line of text is printed
 * for each file/directory that is removed. Failure to remove a file in
 * the record does not lead to any error message. Instead the caller must
 * retry the removal until no more progress is made.
 *
 * @param msg The first message in the message list.
 * @return Zero if no file was removed, one if at least one file was
 *     removed.
 */
static int
pgcli_delete_filelist(struct anoubis_msg *msg)
{
	int	ret = 0;

	for(; msg; msg = msg->next) {
		int	i, offset;

		for (i=offset=0; i<get_value(msg->u.listreply->nrec); ++i) {
			Anoubis_PgFileRecord	*rec;
			char			*path;
			uint64_t		 dev, ino;

			rec = (Anoubis_PgFileRecord *)
			    (msg->u.listreply->payload + offset);
			offset += get_value(rec->reclen);
			dev = get_value(rec->dev);
			ino = get_value(rec->ino);
			if (rec->path[0] == 0 || pgfile_composename(&path,
			    dev, ino, rec->path) < 0) {
				if (opts & PGCLI_OPT_VERBOSE)
					printf(" ERROR invalid file data: "
					    "dev=0x%" PRIx64 " ino=0x%" PRIx64
					    " path=%s\n", dev, ino, rec->path);
				continue;
			}
			if (unlink(path) == 0
			    || (errno == EISDIR && rmdir(path) == 0)) {
				ret = 1;
				pgfile_normalize_file(path);
				printf(" UNLINKED %s\n", path);
			} else if (opts & PGCLI_OPT_VERBOSE) {
				pgfile_normalize_file(path);
				printf(" ERROR for %s: %s\n", path,
				    strerror(errno));
			}
			free(path);
		}
	}
	return ret;
}

int
pgcli_remove(anoubis_cookie_t pgid)
{
	int rc = 0;

	struct achat_channel		*channel = NULL;
	struct anoubis_client		*client = NULL;
	struct anoubis_transaction	*transaction = NULL;
	int				 noprogress;

	rc = pgcli_ui_init();
	if (rc != 0)
		return 1;

	channel = pgcli_create_channel();
	if (channel == NULL)
		return -EIO;

	client = pgcli_create_client(channel);
	if (client == NULL) {
		acc_destroy(channel);
		return -EIO;
	}

	/* Retrieve information about the playground. */
	rc = pgcli_validate_playground(pgid, channel, client);
	if (rc != 0)
		return rc;

	/* delete files until the playground is gone */
	noprogress = 0;
	while (noprogress < 2) {
		transaction = anoubis_client_list_start(client,
		    ANOUBIS_REC_PGFILELIST, pgid);
		rc = anoubis_transaction_complete(client, transaction, NULL);
		if (rc < 0) {
			if (rc == -ESRCH)
				rc = 0;
			break;
		}
		if (get_value(transaction->msg->u.listreply->nrec) == 0) {
			fprintf(stderr, "No more playground files but "
			    "playground is still active");
			rc = 1;
			break;
		}
		if (pgcli_delete_filelist(transaction->msg)) {
			noprogress = 0;
		} else {
			/*
			 * Try to get the file list once more. We have seen
			 * cases where the daemon does not catch up with the
			 * CLI fast enough.
			 */
			noprogress++;
		}
		anoubis_transaction_destroy(transaction);
	}
	if (noprogress == 2) {
		fprintf(stderr, "Could not remove all files in "
		    "the playground\n");
		rc = 1;
	}

	/* Cleanup. */
	PGCLI_CONNECTION_WIPE(channel, client);

	return rc;
}

/**
 * Locate one file in a playground.
 * Prints warnings if file does not exist or does not belong to the
 * playground
 * @param filelist  The anoubis filelist message with the files in the
 *                  selected playground.
 * @param filename  The filename to locate.
 * @return The playground file or NULL on error.
 */
char *
pgcli_find_file(struct anoubis_msg *filelist, const char *filename)
{
	while (filelist) {
		int record_cnt;
		int offset = 0;

		for (record_cnt = 0;
		    record_cnt < get_value(filelist->u.listreply->nrec);
		    ++record_cnt) {
			Anoubis_PgFileRecord *rec;
			char	*path, *npath;
			uint64_t dev, ino;

			rec = (Anoubis_PgFileRecord *)
			    (filelist->u.listreply->payload + offset);
			offset += get_value(rec->reclen);
			dev = get_value(rec->dev);
			ino = get_value(rec->ino);

			if ((rec->path[0] == 0) || pgfile_composename(
			    &path, dev, ino, rec->path) < 0) {
				/* this happens quite often for files that we
				 * just deleted and if this happens because the
				 * file is really unknown we don't care. */
				continue;
			}

			if ((npath = strdup(path)) == NULL) {
				free(path);
				return NULL;
			}
			pgfile_normalize_file(npath);
			if (strcmp(filename, npath) != 0) {
				/* wrong file, check next */
				free(path);
				free(npath);
				continue;
			}
			free(npath);
			return path;
		}

		filelist = filelist->next;
	}

	return(NULL);
}

static int
pgcli_delete(anoubis_cookie_t pgid, int filecnt, char** filenames)
{
	struct achat_channel        *channel = NULL;
	struct anoubis_client       *client = NULL;
	struct anoubis_transaction  *transaction = NULL;

	int rc, fileidx, errcnt = 0;
	char *path;

	rc = pgcli_ui_init();
	if (rc != 0)
		return 1;

	channel = pgcli_create_channel();
	if (channel == NULL)
		return -EIO;

	client = pgcli_create_client(channel);
	if (client == NULL) {
		acc_destroy(channel);
		return -EIO;
	}

	/* Retrieve information about the playground. */
	rc = pgcli_validate_playground(pgid, channel, client);
	if (rc != 0)
		return rc;

	transaction = anoubis_client_list_start(client,
	    ANOUBIS_REC_PGFILELIST, pgid);
	rc = anoubis_transaction_complete(client, transaction, NULL);
	if (rc < 0)
		return rc;

	/* check if files exist and delete them */
	for (fileidx=0; fileidx<filecnt; fileidx++) {
		path = pgcli_find_file(transaction->msg, filenames[fileidx]);
		if (path == NULL) {
			printf(" ERROR file not in playground: %s\n",
			    filenames[fileidx]);
			continue;
		}

		if ((unlink(path) == 0) ||
		    (errno == EISDIR && rmdir(path) == 0)) {
			printf(" UNLINKED %s\n", filenames[fileidx]);
		} else {
			printf(" ERROR for %s: %s\n", path,
			    strerror(errno));
			errcnt++;
		}
		free(path);
	}
	rc = errcnt? 1:0;

	/* Cleanup. */
	anoubis_transaction_destroy(transaction);
	PGCLI_CONNECTION_WIPE(channel, client);

	return rc;
}

void
pgcli_files_print_msg(struct anoubis_msg *message)
{
	int			 offset;
	int			 i = 0;
	Anoubis_PgFileRecord	*rec = NULL;
	Anoubis_PgFileRecord	*lastrec = NULL;
	int			 printed = 1;


	if (message == NULL ||
	    !VERIFY_LENGTH(message, sizeof(Anoubis_ListMessage)) ||
	    get_value(message->u.listreply->error) != 0) {
		fprintf(stderr, "Error retrieving playground list\n");
		return;
	}

	if (get_value(message->u.listreply->rectype)
	    != ANOUBIS_REC_PGFILELIST) {
		fprintf(stderr, "Error: wrong record type in received list.\n");
		return;
	}

	/* Print header */
	printf("%*s ", PGCLI_OUTLEN_ID, "PGID");
	printf("%*s ", PGCLI_OUTLEN_DEV, "DEV");
	printf("%*s ", PGCLI_OUTLEN_INODE, "INODE");
	printf("%s\n", "FILE");

	/* Print table lines */
	while (message) {
		for (offset=i=0; i<get_value(message->u.listreply->nrec); ++i) {
			rec = (Anoubis_PgFileRecord *)(
			    message->u.listreply->payload + offset);
			offset += get_value(rec->reclen);

			if (lastrec == NULL
			    || get_value(rec->dev) != get_value(lastrec->dev)
			    || get_value(rec->ino) != get_value(lastrec->ino)) {
				if (!printed)
					pgcli_files_print_record(lastrec, 0);
				printed = 0;
				lastrec = rec;
			}
			if (pgcli_files_print_record(rec, 1))
				printed = 1;
		}
		message = message->next;
	}
	if (lastrec && !printed)
		pgcli_files_print_record(lastrec, 0);
}

int
pgcli_files_print_record(Anoubis_PgFileRecord *record, int needpath)
{
	char		*path = NULL;
	uint64_t	 dev, ino;
	int		 error;

	dev = get_value(record->dev);
	ino = get_value(record->ino);
	error = pgfile_composename(&path, dev, ino, record->path);
	if (error) {
		if (needpath)
			return 0;
		path = anoubis_strerror(-error);
	} else {
		pgfile_normalize_file(path);
	}

	printf("%*"PRIx64" ", PGCLI_OUTLEN_ID, get_value(record->pgid));
	printf("%*"PRIx64" ", PGCLI_OUTLEN_DEV, dev);
	printf("%*"PRIx64" ", PGCLI_OUTLEN_INODE, ino);
	if (needpath) {
		printf("%s\n", path);
		free(path);
	} else {
		printf("(%s)\n", path);
	}
	return 1;
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

/**
 * Preparation method for the commit command, must be called
 * once to register for notifications.
 * @param client  The connected anoubis client structure for communication
 * @return  0 on success, errno else
 */
static int
pgcli_commit_file_prepare(struct anoubis_client *client) {
	struct anoubis_transaction *transaction;
	int rc;

	/* register for security label remove notifications */
	transaction = anoubis_client_register_start(client, 1 /*token*/,
	    geteuid(), 0 /*rule_id*/, ANOUBIS_SOURCE_PLAYGROUNDFILE);
	rc = anoubis_transaction_complete(client, transaction, NULL);

	return rc;
}

/**
 * Print formatted playground scanner results to stderr. This assumes
 * that the return code is either -EAGAIN (only recommended scanners
 * failed) or -EPERM (at least one required scanner failed).
 *
 * @param file The file we are trying to commit (for error messages)
 * @param m The result message of the commit transaction. This message
 *     contains the scanner report text.
 * @param rc The (negative!) return code of the commit transaction.
 * @return A negative error code if the caller must still print an error
 *     message or one if the error of the transaction was reported properly.
 *     This function never returns zero as it should only be called if
 *     an error occured and zero would mean success.
 */
static int
pgcli_commit_show_scanresult(const char *file, struct anoubis_msg *m, int rc)
{
	const char	**str;
	int		  i;

	if (m == NULL)
		return rc;
	str = anoubis_client_parse_pgcommit_reply(m);
	if (str == NULL)
		return -ENOMEM;
	for (i=0; str[i]; i+=2) {
		char		*line, *next, *orig;

		fprintf(stderr, "File %s: '%s' reports:\n", file, str[i]);
		if (str[i+1] == NULL) {
			fprintf(stderr, "| <nothing>\n");
			break;
		}
		if (strlen(str[i+1]) == 0) {
			fprintf(stderr, "| <nothing>\n");
			continue;
		}
		orig = line = strdup(str[i+1]);
		while((next = strsep(&line, "\n")) != NULL) {
			/*
			 * Avoid printing of trailing newline if the scanner
			 * terminates its output with a newline.
			 */
			if (line == NULL && *next == '\0')
				continue;
			fprintf(stderr, "| %s\n", next);
		}
		free(orig);
	}
	free(str);
	switch (rc) {
	case -EAGAIN:
		fprintf(stderr, "File not committed. Use --ignore-recommended "
		    "to override\n");
		break;
	case -EPERM:
		fprintf(stderr, "File cannot be committed.\n");
		break;
	default:
		return rc;
	}
	return 1;
}

/**
 * Commit one file, expects verified parameters.
 * This method will do the actual work to commit one file. It expects verified
 * parameters and should not be called directly. It will remove the security
 * label, wait for the daemon to acknowledge this and start the file scanning.
 * Finally it will rename the file(s).
 *
 * @param client    The anoubis client connection.
 * @param pgid      The playground id from which files are to be committed.
 * @param dev       The device of the file to commit
 * @param inode     The inode of the file to commit
 * @param filenames The filename(s) of the file to commit, array must be
 *                  terminated with a NULL pointer entry.
 * @return 0 on success
 */
static int
pgcli_commit_file(struct anoubis_client *client, uint64_t pgid, uint64_t dev,
    uint64_t inode, const char* filenames[], const char* abspaths[])
{
	struct anoubis_msg		*notification;
	struct pg_file_message		*filemsg;
	struct stat			 sb;
	struct anoubis_transaction	*transaction = NULL;
	int				 rc, offset;
	int				 have_notification;
	char				*abspath;
	int				 filenamecnt;
	struct anoubis_msg		*results = NULL;

	/* some validations */
	filenamecnt = 0;
	while (filenames[filenamecnt] != NULL) {
		filenamecnt++;
	}
	if (filenamecnt == 0) {
		/* can't commit file without at least one valid name */
		return -EINVAL;
	}

	char *errpath = strdup(abspaths[0]);
	pgfile_normalize_file(errpath);

	rc = pgfile_check(dev, inode, filenames, !(opts & PGCLI_OPT_FORCE));
	if (rc < 0) {
		switch (-rc) {
		case EMLINK:
			fprintf(stderr, "Cannot commit %" PRIx64 ":%s: "
			    "Target file exists and has multiple hard links\n"
			    "Use -f to overwrite\n", pgid, errpath);
			return 1;
		case EEXIST:
			rc = stat(errpath, &sb);
			if (rc < 0){
				return rc;
			}
			if (!S_ISDIR(sb.st_mode)){
				fprintf(stderr, "Cannot commit %" PRIx64 ":%s: "
				    "Target file exists\nUse -f to overwrite\n",
				    pgid, errpath);
				return 1;
			} else {
				fprintf(stderr, "Cannot commit %" PRIx64 ":%s: "
				    "Target directory exists\nUse -f to"
				    " overwrite\n", pgid, errpath);
				return 1;
			}
		case EBUSY:
			if (opts & PGCLI_OPT_FORCE) {
				fprintf(stderr, "Cannot find all hard links "
				    "for %" PRIx64 ":%s: Committing anyway\n",
				    pgid, errpath);
				break;
			}
			fprintf(stderr, "Cannot find all hard links "
			    "for %" PRIx64 ":%s: File will not be committed.\n"
			    "Use -f to overwrite\n", pgid, errpath);
			return 1;
		case EXDEV:
			fprintf(stderr, "Cannot commit file %" PRIx64 ":%s: "
			    "Device is not mounted.\n", pgid, errpath);
			return 1;
		default:
			return rc;
		}
	}

	/* [try to] remove the security label */
	if ((rc = lremovexattr(abspaths[0], "security.anoubis_pg") < 0)) {
		/* EINPROGRESS means success */
		if (errno != EINPROGRESS && errno != ENOTEMPTY && errno){
			/* unknown error */
			return -errno;
		} else if (errno == ENOTEMPTY) {
			/* parent dir not comitted */
			char	*dir, *slash;

			dir = strdup(abspaths[0]);
			if (!dir)
				return -ENOMEM;
			slash = strrchr(dir, '/');
			if (slash == dir)
				slash++;
			if (slash)
				(*slash) = 0;
			pgfile_normalize_file(dir);
			fprintf(stderr, "Cannot commit file %" PRIx64 ":%s: "
			    "Please commit its parent directory '%s' first!\n",
			    pgid, errpath, dir);
			free(dir);
			return 1;
		}
	} else {
		/* lremovexattr returned 0, which is not expected here */
		fprintf(stderr, "commit: Internal error (lremovexattr"
		    "returned unexpected result (%d))\n", rc);
		return 1;
	}

	/* wait until the daemon sends us a notification that the file is
	 * ready for scanning */
	have_notification = 0;
	while (!have_notification) {
		rc = anoubis_client_wait(client);
		if (rc < 0) {
			free(errpath);
			return rc;
		}

		while((notification = anoubis_client_getnotify(client))) {
			/* ignore incorrect messages */
			if ((get_value(notification->u.notify->type) !=
			    ANOUBIS_N_LOGNOTIFY) ||
			    (get_value(notification->u.notify->subsystem) !=
			    ANOUBIS_SOURCE_PLAYGROUNDFILE)) {
				anoubis_msg_free(notification);
				continue;
			}

			/* get the containing kernel message */
			offset = get_value(notification->u.notify->evoff);
			filemsg = (struct pg_file_message*)
			    (notification->u.notify->payload+offset);

			/* ignore incorrect messages */
			if ((filemsg->pgid != pgid) ||
			    (filemsg->dev != dev) ||
			    (filemsg->ino != inode) ||
			    (filemsg->op != ANOUBIS_PGFILE_SCAN)) {
				anoubis_msg_free(notification);
				continue;
			}

			/* kernel notify is for our requested file */
			/* store the the verified path within the device */
			rc = pgfile_composename(&abspath, dev, inode,
			    filemsg->path);
			if (rc < 0) {
				free(errpath);
				return rc;
			}

			anoubis_msg_free(notification);
			have_notification = 1;
			break;
		}
	}

	/* send the commit request to the daemon */
	transaction = anoubis_client_pgcommit_start(client, pgid, abspath,
	    (opts & PGCLI_OPT_IGNRECOMM));
	free(abspath);
	rc = anoubis_transaction_complete(client, transaction, &results);
	if (rc < 0) {
		if (rc == -EAGAIN || rc == -EPERM)
			rc = pgcli_commit_show_scanresult(errpath, results, rc);
		free(errpath);
		return rc;
	}
	free(errpath);
	/* rename the files */
	rc = pgfile_process(dev, inode, filenames);
	if (rc < 0)
		return rc;

	return 0;
}

static int
pgcli_commit(uint64_t pgid, const char* file)
{
	struct achat_channel		*channel     = NULL;
	struct anoubis_client		*client      = NULL;
	struct anoubis_transaction	*transaction = NULL;
	struct anoubis_msg		*filelist    = NULL;
	int		rc, filecount, file_index = 0;
	struct		stat stats;
	char		*abspath    = NULL;
	char		**filenames = NULL, **abspaths = NULL;

	/* establish connection to daemon */
	rc = pgcli_ui_init();
	if (rc != 0)
		return 1;
	channel = pgcli_create_channel();
	if (channel == NULL)
		return -EIO;
	client = pgcli_create_client(channel);
	if (client == NULL) {
		acc_destroy(channel);
		return -EIO;
	}

	/* first check if commit is allowed for specified playground */
	rc = pgcli_validate_playground(pgid, channel, client);
	if (rc != 0){
	    fprintf(stderr, "commit: operation is not allowed"
		" for this playground\n");
	    return 1;
	}

	/* request PG file list from daemon */
	transaction = anoubis_client_list_start(client,
	    ANOUBIS_REC_PGFILELIST, pgid);

	rc = anoubis_transaction_complete(client, transaction, NULL);
	if (rc < 0) {
		PGCLI_CONNECTION_WIPE(channel, client);
		return rc;
	}

	filelist = transaction->msg;
	rc = pgcli_commit_file_prepare(client);
	if (rc < 0) {
		fprintf(stderr, "commit: internal error! (prepare failed)\n");
		return rc;
	}

	if ((abspath = pgcli_find_file(filelist, file)) == NULL) {
		printf(" ERROR file not in playground: %s\n", file);
		return -ENOENT;
	}

	if (stat(abspath, &stats) == -1) {
		printf(" ERROR failed to stat '%s': %s\n",
		    abspath, strerror(errno));
		free(abspath);
		return -errno;
	}

	int iteration;
	for (iteration=0; iteration<2; iteration++){
		if (iteration == 0){
		    filecount = 0;
		} else {
			if (!filecount){
				fprintf(stderr, "commit: The specified"
				    " playground contains no files which could"
				    " be comitted!\n");
				return 1;
			}
			/* initialize filename array */
			filenames = malloc(sizeof(char*) * (filecount + 1));
			filenames[filecount] = 0;
			abspaths = malloc(sizeof(char*) * (filecount + 1));
			abspaths[filecount] = 0;
			file_index = 0;
		}

		while (filelist) {
			int record_cnt;
			int offset = 0;
			for (record_cnt = 0;
			    record_cnt < get_value(filelist->u.listreply->nrec);
			    ++record_cnt) {
				Anoubis_PgFileRecord *rec;
				uint64_t dev, ino;

				rec = (Anoubis_PgFileRecord *)
				    (filelist->u.listreply->payload + offset);
				offset += get_value(rec->reclen);
				dev = get_value(rec->dev);
				ino = get_value(rec->ino);

				if ( dev == expand_dev(stats.st_dev) &&
				    ino == stats.st_ino ) {
					if (iteration == 0){
						/* fist pass -> count files */
						filecount++;
					} else {
						/* second pass -> store name */
						char *path = rec->path;
						rc = pgfile_composename(
						    &abspath, dev, ino, path);
						if(rc < 0){
							return rc;
						}
						filenames[file_index] = path;
						abspaths[file_index]  = abspath;
						file_index++;
					}
				}
			}
			filelist = filelist->next;
		}
		/* rewind filelist */
		filelist = transaction->msg;
	}
	filenames[file_index] = 0;
	abspaths[file_index] = 0;

	/* initiate the actual commit procedure */
	rc = pgcli_commit_file(client, pgid, expand_dev(stats.st_dev),
	    stats.st_ino, (const char**)filenames, (const char**)abspaths);

	/* cleanup */
	anoubis_transaction_destroy(transaction);
	PGCLI_CONNECTION_WIPE(channel, client);
	free(filenames);
	free(abspaths);
	free(abspath);

	return rc;
}

#endif /* LINUX */

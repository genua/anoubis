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
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "anoubisctl.h"
#include "apn.h"


void		usage(void) __dead;
static int	daemon_start(void);
static int	daemon_stop(void);
static int	daemon_restart(void);
static int	daemon_status(void);
static int	load(char *);
static int	dump(char *);

typedef int (*func_void_t)(void);
typedef int (*func_char_t)(char *);

struct cmd {
	char	       *command;
	func_void_t	func;
	int		file;
} commands[] = {
	{ "start",   daemon_start,   0 },
	{ "stop",    daemon_stop,    0 },
	{ "restart", daemon_restart, 0 },
	{ "status",  daemon_status,  0 },
	{ "load",    (func_void_t)load, 1 },
	{ "dump",    (func_void_t)dump, 1 },
};

static int	opts = 0;

__dead void
usage(void)
{
	int	i;
	extern char	*__progname;

	fprintf(stderr, "usage: %s [-nv] <command> [<file>]\n", __progname);
	fprintf(stderr, "    <command>:\n");
	for (i=0; i < sizeof(commands)/sizeof(struct cmd); i++) {
		if (commands[i].file)
			fprintf(stderr, "        %s <file>\n",
				commands[i].command);
		else
			fprintf(stderr, "        %s\n", commands[i].command);

	}
	exit(1);
}

int
main(int argc, char *argv[])
{
	int	ch, i, done;
	int	error = 0;
	char	*command = NULL;
	char	*rulesopt = NULL;

	if (argc < 2)
		usage();

	while ((ch = getopt(argc, argv, "nv")) != -1) {
		switch (ch) {

		case 'n':
			opts |= ANOUBISCTL_OPT_NOACTION;
			break;

		case 'v':
			if (opts & ANOUBISCTL_OPT_VERBOSE)
				opts |= ANOUBISCTL_OPT_VERBOSE2;
			opts |= ANOUBISCTL_OPT_VERBOSE;
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
					error = 1;
				} else {
					error = ((func_char_t)commands[i].func)(
					    rulesopt);
					done = 1;
				}

			} else {

				if (rulesopt != NULL) {
					fprintf(stderr, "too many arguments\n");
					error = 1;
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
	int	error = 0;
	printf("start\n");
	return error;
}

static int
daemon_stop(void)
{
	int	error = 0;
	printf("stop\n");
	return error;
}

static int
daemon_restart(void)
{
	int	error = 0;
	printf("restart\n");
	return error;
}

static int
daemon_status(void)
{
	int	error = 0;
	printf("status\n");
	return error;
}

static int
dump(char *file)
{
	int	error = 0;
	printf("dump %s\n", file);
	return error;
}

static int
load(char *rulesopt)
{
	struct apn_ruleset	*ruleset;
	int	flags = 0;
	int	error = 0;

	if (rulesopt != NULL) {
		if (opts & ANOUBISCTL_OPT_VERBOSE)
			flags |= APN_FLAG_VERBOSE;
		if (opts & ANOUBISCTL_OPT_VERBOSE2)
			flags |= APN_FLAG_VERBOSE2;

		if (apn_parse(rulesopt, &ruleset, flags)) {
			error = 1;
			apn_print_errors(ruleset);
		}
	}

	if (ruleset)
		apn_free_ruleset(ruleset);

	return error;
}

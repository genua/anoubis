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

void		 usage(void) __dead;

__dead void
usage(void)
{
	extern char	*__progname;

	fprintf(stderr, "usage: %s [-nv] [-f file]\n", __progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct apnruleset	*ruleset;
	int			 ch, flags;
	int			 error = 0;
	int			 opts = 0;
	char			*rulesopt = NULL;

	if (argc < 2)
		usage();

	while ((ch = getopt(argc, argv, "f:nv")) != -1) {
		switch (ch) {
		case 'f':
			rulesopt = optarg;
			break;

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

	if (rulesopt != NULL) {
		flags = 0;
		if (opts & ANOUBISCTL_OPT_VERBOSE)
			flags |= APN_FLAG_VERBOSE;
		if (opts & ANOUBISCTL_OPT_VERBOSE2)
			flags |= APN_FLAG_VERBOSE2;

		if (apn_parse(rulesopt, &ruleset, flags))
			error = 1;
	}

	exit(error);
}

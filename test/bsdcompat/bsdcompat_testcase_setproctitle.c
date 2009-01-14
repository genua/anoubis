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

#include <stdarg.h>
#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

/*
 * Simple argv manipulation
 */
START_TEST(tc_setproctitle)
{
	int argc;
	char *argv[2];
	extern char *__progname;
	char ptitle[1024];
	char pstr[] = "123456789abcdef";

	// create fake argc/argv
	argc = 1;
	argv[0] = __progname;
	argv[1] = NULL;

	// minimal mockup of setproctitle
	snprintf(ptitle, sizeof(ptitle), "%s: %s", argv[0], pstr);

	// normal usage
	compat_init_setproctitle(argc, argv);
	setproctitle(pstr);

	fail_if(strncmp(argv[0], ptitle, sizeof(ptitle)) != 0,
		"output mismatch");
}
END_TEST

TCase *
bsdcompat_testcase_setproctitle(void)
{
	TCase	*testcase_setproctitle = tcase_create("setproctitle()");

	tcase_add_test(testcase_setproctitle, tc_setproctitle);

	return (testcase_setproctitle);
}

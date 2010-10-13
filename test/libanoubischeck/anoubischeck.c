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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "anoubischeck.h"

/**
 * Evaluates failures and type of failures from the given SRunner.
 * @param srunner The SRunner to run
 * @return 0 if all tests passed,
 *         1 if at least one test failed but no error occured,
 *         2 if at least one error occured.
 */
int
check_eval_srunner(SRunner *srunner)
{
	TestResult	**testresults;
	int		nfailed, i;
	int		nfailure, nerror, nunknown;

	nfailed = srunner_ntests_failed(srunner);
	if (nfailed == 0) {
		/* No failures and errors -> success */
		return (0);
	}

	nfailure = 0;
	nerror = 0;
	nunknown = 0;
	testresults = srunner_failures(srunner);

	/* Go through list of failed tests and count result-types */
	for (i = 0; i < nfailed; i++) {
		switch (tr_rtype(testresults[i])) {
		case CK_FAILURE:
			nfailure++;
			break;
		case CK_ERROR:
			nerror++;
			break;
		default:
			nunknown++;
			break;
		}
	}

	free(testresults);

	if (nerror > 0 || nunknown > 0)
		return (2); /* NORESULT */
	else
		return (1); /* ERROR */
}

/**
 * Utility function used in write_or_fail macro
 * @param fd The FD to write to
 * @param buf The buffer to write
 * @param count The number of bytes to write
 * @return 0 on failure, non-zero on success
 */
int
write_full(int fd, const void *buf, size_t count)
{
	ssize_t len;

	while (count > 0) {
		len = write(fd, buf, count);
		if (len == -1) {
			if (errno != EAGAIN && errno != EINTR) {
				perror("write");
				return 0;
			}
			else
				continue;
		}
		count -= len;
		buf += len;
	}

	return 1;
}

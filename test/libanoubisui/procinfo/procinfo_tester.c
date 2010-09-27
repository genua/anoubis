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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <anoubis_procinfo.h>

/**
 * Print process information for each process given on the command
 * line. Information printed is in this order:
 * ̣- process id
 * - parent process ID
 * - real, effective and saved user ID
 * - real, effective and saved group ID
 * - The short command
 * - The program part of the long command
 */
int
main(int argc, char **argv)
{
	struct anoubis_proc_handle		*handle;
	int					 i;
	int					 pid;

	handle = anoubis_proc_open();
	assert(handle != NULL);
	for (i=1; i<argc; ++i) {
		struct anoubis_proc	*p;
		int			 rc;
		char			 dummy;

		rc = sscanf(argv[i], "%d%c", &pid, &dummy);
		assert(rc == 1);
		p = anoubis_proc_get(handle, pid);
		if (p == NULL)
			continue;
#define INT "%" PRId32 " "
		printf(INT INT INT INT INT INT INT INT "%s %s\n",
		    p->pid, p->ppid, p->ruid, p->euid, p->suid,
		    p->rgid, p->egid, p->sgid, p->comm, p->command);
#undef INT
		anoubis_proc_destroy(p);
	}
	anoubis_proc_close(handle);
	return  0;
}

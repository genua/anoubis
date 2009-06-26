/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#include <config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <check.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "anoubisd.h"
#include "pe.h"

#ifdef LINUX
#include <linux/anoubis.h>
#include <bsdcompat.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

/*
 * XXX ch: These are global variables and methods declared by main.c of
 * XXX ch: anoubisd. Can we remove/replace this by a better solution
 * XXX ch: from sten?
 * XXX ch: Especially the struct must be kept in sync with it's definition
 * XXX ch: in the daemon. This is ugly and we need a better solution for this.
 */
gid_t		anoubisd_gid = 0;
u_int32_t	debug_flags = 0;
u_int32_t	debug_stderr = 0;
char		*logname = NULL;

__dead void
master_terminate(int error)
{
	fail_if(error != 0, "master_terminate with error: %d\n", error);
	exit(error);
}

anoubisd_msg_t *
msg_factory(int mtype, int size)
{
	fail("msg_factory(%d,%d)\n", mtype, size);
	return NULL;
}

struct pe_proc {
	TAILQ_ENTRY(pe_proc)	 entry;
	int			 refcount;
	pid_t			 pid;
	uid_t			 uid;
	uid_t			 sfsdisable_uid;
	pid_t			 sfsdisable_pid;
	unsigned int		 flags;

	struct pe_proc_ident	 ident;
	anoubis_cookie_t	 task_cookie;
	anoubis_cookie_t	 borrow_cookie;

	/* Per priority contexts */
	struct pe_context	*context[PE_PRIO_MAX];
	struct pe_context	*saved_ctx[PE_PRIO_MAX];
};

static struct pe_proc *
tc_PolicyEngine_proc_alloc(void)
{
	struct pe_proc	*proc;

	proc = calloc(1, sizeof(struct pe_proc));
	if (proc == NULL) {
		fail("Couldn't alloc proc OutOfMemory!");
		return (NULL);
	}

	proc->flags = 0;

	return (proc);
}

/*
 * Unit tests
 */
START_TEST(tc_PolicyEngine_upgrade_init)
{
	int		mark;
	struct pe_proc *proc;

	/* init */
	mark = 0;
	proc = tc_PolicyEngine_proc_alloc();

	/* test */
	mark = pe_proc_is_upgrade(NULL);
	fail_if(mark != 0, "Upgrade mark set within NULL.");

	mark = pe_proc_is_upgrade(proc);
	fail_if(mark != 0, "Upgrade mark set unexpectedly.");

	mark = pe_proc_is_upgrade_parent(NULL);
	fail_if(mark != 0, "Upgrade parent mark set within NULL.");

	mark = pe_proc_is_upgrade_parent(proc);
	fail_if(mark != 0, "Upgrade parent mark set unexpectedly.");

	/* cleanup */
	free(proc);
}
END_TEST

START_TEST(tc_PolicyEngine_upgrade_unset2set_zero)
{
	int		mark;
	struct pe_proc *proc;

	/* init */
	mark = 0;
	proc = tc_PolicyEngine_proc_alloc();

	/* test */
	pe_proc_upgrade_addmark(proc);

	mark = pe_proc_is_upgrade(proc);
	fail_if(mark == 0, "Upgrade mark not set.");

	mark = pe_proc_is_upgrade_parent(proc);
	fail_if(mark == 0, "Upgrade parent mark not set.");

	fail_if(proc->flags != 0x03, "Unrelated flags modified! "
	    "Expect 0x%x - got 0x%x", 0x03, proc->flags);

	/* cleanup */
	free(proc);
}
END_TEST

START_TEST(tc_PolicyEngine_upgrade_unset2set_tainted)
{
	int		mark;
	struct pe_proc *proc;

	/* init */
	mark = 0;
	proc = tc_PolicyEngine_proc_alloc();
	proc->flags = 0x80;

	/* test */
	pe_proc_upgrade_addmark(proc);

	mark = pe_proc_is_upgrade(proc);
	fail_if(mark == 0, "Upgrade mark not set.");

	mark = pe_proc_is_upgrade_parent(proc);
	fail_if(mark == 0, "Upgrade parent mark not set.");

	fail_if(proc->flags != 0x83, "Unrelated flags modified! "
	    "Expect 0x%x - got 0x%x", 0x83, proc->flags);

	/* cleanup */
	free(proc);
}
END_TEST

START_TEST(tc_PolicyEngine_upgrade_set2unset_zero)
{
	int		mark;
	struct pe_proc *proc;

	/* init */
	mark = 0;
	proc = tc_PolicyEngine_proc_alloc();
	proc->flags = 0x03;

	/* test */
	pe_proc_upgrade_clrmark(proc);

	mark = pe_proc_is_upgrade(proc);
	fail_if(mark != 0, "Upgrade mark still set.");

	mark = pe_proc_is_upgrade_parent(proc);
	fail_if(mark != 0, "Upgrade parent mark still set.");

	fail_if(proc->flags != 0x0, "Unrelated flags modified! "
	    "Expect 0x%x - got 0x%x", 0x0, proc->flags);

	/* cleanup */
	free(proc);
}
END_TEST
START_TEST(tc_PolicyEngine_upgrade_set2unset_tainted)
{
	int		mark;
	struct pe_proc *proc;

	/* init */
	mark = 0;
	proc = tc_PolicyEngine_proc_alloc();
	proc->flags = 0x83;

	/* test */
	pe_proc_upgrade_clrmark(proc);

	mark = pe_proc_is_upgrade(proc);
	fail_if(mark != 0, "Upgrade mark still set.");

	mark = pe_proc_is_upgrade_parent(proc);
	fail_if(mark != 0, "Upgrade parent mark still set.");

	fail_if(proc->flags != 0x80, "Unrelated flags modified! "
	    "Expect 0x%x - got 0x%x", 0x80, proc->flags);

	/* cleanup */
	free(proc);
}
END_TEST

/*
 * Testcase
 */
TCase *
anoubisd_testcase_pe(void)
{
	/* policy engine test case */
	TCase *tc_pe = tcase_create("PolicyEngine");

	tcase_add_test(tc_pe, tc_PolicyEngine_upgrade_init);
	tcase_add_test(tc_pe, tc_PolicyEngine_upgrade_unset2set_zero);
	tcase_add_test(tc_pe, tc_PolicyEngine_upgrade_unset2set_tainted);
	tcase_add_test(tc_pe, tc_PolicyEngine_upgrade_set2unset_zero);
	tcase_add_test(tc_pe, tc_PolicyEngine_upgrade_set2unset_tainted);

	return (tc_pe);
}

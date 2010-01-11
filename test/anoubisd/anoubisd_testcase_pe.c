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

#define _PE_PROC_INTERNALS_H_
#include <pe_proc_internals.h>

/* Shortcuts */
#define BOTH (PE_PROC_FLAGS_UPGRADE|PE_PROC_FLAGS_UPGRADE_PARENT)

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

	fail_if(proc->flags != BOTH, "Unrelated flags modified! "
	    "Expect 0x%x - got 0x%x", BOTH, proc->flags);

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
	proc->flags = ~BOTH;

	/* test */
	pe_proc_upgrade_addmark(proc);

	mark = pe_proc_is_upgrade(proc);
	fail_if(mark == 0, "Upgrade mark not set.");

	mark = pe_proc_is_upgrade_parent(proc);
	fail_if(mark == 0, "Upgrade parent mark not set.");

	fail_if(proc->flags != ~0U, "Unrelated flags modified! "
	    "Expect 0x%x - got 0x%x", ~0U, proc->flags);

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
	proc->flags = BOTH;

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
	proc->flags = ~0U;

	/* test */
	pe_proc_upgrade_clrmark(proc);

	mark = pe_proc_is_upgrade(proc);
	fail_if(mark != 0, "Upgrade mark still set.");

	mark = pe_proc_is_upgrade_parent(proc);
	fail_if(mark != 0, "Upgrade parent mark still set.");

	fail_if(proc->flags != ~BOTH, "Unrelated flags modified! "
	    "Expect 0x%x - got 0x%x", ~BOTH, proc->flags);

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

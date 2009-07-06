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
#include <check.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "anoubisd.h"
#include "pe.h"
#include "pe_filetree.h"
#include <anoubisd_unit.h>

#ifdef LINUX
#include <linux/anoubis.h>
#include <bsdcompat.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

int __assert_upgrade(int upgrade, int parent, ...)
{
	va_list			 ap;
	int			 cookie;
	struct pe_proc		*proc;

	va_start(ap, parent);
	while(1) {
		cookie = va_arg(ap, int);
		if (cookie == 0)
			break;
		proc = pe_proc_get(cookie);
		if (proc == NULL)
			return cookie;
		if (parent >= 0 && !!pe_proc_is_upgrade_parent(proc) != parent)
			return cookie;
		if (upgrade >= 0 && !!pe_proc_is_upgrade(proc) != upgrade)
			return cookie;
	}
	va_end(ap);
	return 0;
}

#define __ASSERT_UPGRADE(UP, PUP, ...)					\
	do {								\
		int __r = __assert_upgrade((UP), (PUP), __VA_ARGS__, 0);\
		fail_if(__r != 0, "Wrong upgrade flags for %d", __r);	\
	} while(0)

#define ASSERT_UPGRADE(...)		__ASSERT_UPGRADE( 1, -1, __VA_ARGS__)
#define ASSERT_PARENT(...)		__ASSERT_UPGRADE( 1,  1, __VA_ARGS__)
#define ASSERT_UPGRADE_NOPARENT(...)	__ASSERT_UPGRADE( 1,  0, __VA_ARGS__)
#define ASSERT_NOUPGRADE(...)		__ASSERT_UPGRADE( 0,  0, __VA_ARGS__)
#define ASSERT_NOPARENT(...)		__ASSERT_UPGRADE(-1,  0, __VA_ARGS__)

#define MARK_UPGRADE(cookie)						\
	do {								\
		struct pe_proc *__p = pe_proc_get(cookie);		\
		fail_if(__p == 0,					\
		    "No such process (cookie=%d)", cookie);		\
		pe_upgrade_start(__p);					\
		pe_proc_put(__p);					\
	} while(0)

#define END_UPGRADE(cookie)						\
	do {								\
		struct pe_proc *__p = pe_proc_get(cookie);		\
		fail_if(__p == 0,					\
		    "No such process (cookie=%d)", cookie);		\
		pe_upgrade_end(__p);					\
		pe_proc_put(__p);					\
	} while(0)

static eventdev_token	next_token = 1;

#define R	ANOUBIS_OPEN_FLAG_READ
#define W	ANOUBIS_OPEN_FLAG_WRITE
#define X	ANOUBIS_OPEN_FLAG_EXEC
#define F	ANOUBIS_OPEN_FLAG_FOLLOW

#define FLAG_MASK (R|W|X|F)

int
touch(int cookie, const char *path, int fake_csum, unsigned int flags)
{
	struct eventdev_hdr	*hdr;
	int			 size, ret;
	struct sfs_open_message	*sfs;
	anoubisd_reply_t	*reply;

	size = sizeof(struct eventdev_hdr) + sizeof(struct sfs_open_message)
	    + strlen(path);
	hdr = malloc(size);
	fail_if(hdr == NULL, "Out of memory");

	hdr->msg_size = size;
	hdr->msg_source = ANOUBIS_SOURCE_SFS;
	hdr->msg_flags = EVENTDEV_NEED_REPLY;
	hdr->msg_token = next_token++;
	hdr->msg_pid = 4711;
	hdr->msg_uid = 0;

	sfs = (struct sfs_open_message*)(hdr+1);
	sfs->common.task_cookie = cookie;
	sfs->ino = sfs->dev = 0;
	fail_if ((flags & FLAG_MASK) != flags, "Bad flags %x\n", flags);
	sfs->flags = flags;
	if (path) {
		sfs->flags |= ANOUBIS_OPEN_FLAG_PATHHINT;
		strcpy(sfs->pathhint, path);
	} else {
		sfs->pathhint[0] = 0;
	}
	if (fake_csum) {
		sfs->flags |= ANOUBIS_OPEN_FLAG_CSUM;
		memset(sfs->csum, 0, ANOUBIS_CS_LEN);
		memcpy(sfs->csum, &fake_csum, sizeof(fake_csum));
	}
	reply = test_pe_handle_sfs(hdr);
	fail_unless(reply != NULL,
	    "Did not get a valid reply from pe_handle_sfs");
	ret = reply->reply;
	free(reply);

	return ret;
}

#define WRITE(C,P)							\
	do {								\
		int __r = touch((C), "/" #P, 0, W);			\
		fail_if(__r, "touch for %s (task=%d) failed with %d",	\
		    "/" #P, (C), __r);					\
	} while(0)

#define WRITE_FAIL(C, P)						\
	do {								\
		int __r = touch((C), "/" #P, 0, W);			\
		fail_if(__r >= 0, "touch for %s (task=%d) "		\
		    "should not succeed", "/" #P (C));			\
	} while(0)

/*
 * Create a bunch of processes, set and clear upgrade marks and verify that
 * the resulting flags are as expected.
 */
START_TEST(tc_flags)
{
	pe_init();

	pe_proc_fork(0, 1, 0);
	pe_proc_fork(0, 2, 1);
	pe_proc_fork(0, 3, 1);
	pe_proc_fork(0, 4, 1);
	pe_proc_fork(0, 5, 1);

	pe_proc_fork(0, 21, 2);
	pe_proc_fork(0, 22, 2);
	pe_proc_fork(0, 31, 3);
	pe_proc_fork(0, 32, 3);
	pe_proc_fork(0, 41, 4);
	pe_proc_fork(0, 42, 4);
	pe_proc_fork(0, 51, 5);
	pe_proc_fork(0, 52, 5);

	MARK_UPGRADE(2);
	MARK_UPGRADE(31);

	pe_proc_fork(0, 23, 2);
	pe_proc_fork(0, 33, 3);
	pe_proc_fork(0, 311, 31);
	pe_proc_fork(0, 411, 41);
	pe_proc_exit(52);

	fail_if(pe_proc_get(52), "Process 52 should have terminated");
	ASSERT_PARENT(2, 31);
	ASSERT_UPGRADE_NOPARENT(23, 311);
	ASSERT_NOUPGRADE(1, 3, 4, 5, 21, 22, 32, 33, 41, 411, 42, 51);

	END_UPGRADE(311);
	pe_proc_exit(2);

	fail_if(pe_proc_get(2), "Process 2 should have terminated");
	ASSERT_PARENT(31);
	ASSERT_UPGRADE_NOPARENT(23);
	ASSERT_NOUPGRADE(1, 3, 4, 5, 21, 22, 311, 32, 33, 41, 411, 42, 51);

	pe_proc_exit(31);
	fail_if(pe_proc_get(31), "Process 31 should have terminated");
	ASSERT_NOUPGRADE(1, 3, 4, 5, 21, 22, 23, 311, 32, 33, 41, 411, 42, 51);

	MARK_UPGRADE(51);
	pe_proc_fork(0, 511, 51);

	ASSERT_NOUPGRADE(1, 3, 4, 5, 21, 22, 23, 311, 32, 33, 41, 411, 42,
	    51, 511);

	pe_upgrade_finish();

	MARK_UPGRADE(51);
	pe_proc_fork(0, 512, 51);

	ASSERT_PARENT(51);
	ASSERT_UPGRADE_NOPARENT(512);
	ASSERT_NOUPGRADE(1, 3, 4, 5, 21, 22, 23, 311, 32, 33, 41, 411, 42, 511);

	END_UPGRADE(51);
	ASSERT_NOUPGRADE(1, 3, 4, 5, 21, 22, 23, 311, 32, 33, 41, 411, 42,
	    51, 511,  512);

	pe_upgrade_finish();

	/* Cleanup for next test! */
	pe_shutdown();
}
END_TEST

static int
haschecksum(const char *path __attribute__((unused)))
{
	return 1;
}

struct pe_expect_path {
	const char *path;
	int cookie;
	int found;
};
#define ADD_PATH(P,C)	{ "/" #P, (C), 0 }
static struct pe_expect_path	tc_file_expect[]  = {
	ADD_PATH(b2, 0),
	ADD_PATH(c2, 0),
	ADD_PATH(c32, 0),
	ADD_PATH(c321, 321),
	ADD_PATH(c322, 0),
	ADD_PATH(d2, 0),
	ADD_PATH(d32, 0),
	{ NULL, 0, 0 }
};
#undef ADD_PATH

START_TEST(tc_files)
{
	struct pe_file_node	*cur;
	int			 i;

	pe_init();
	sfs_haschecksum_chroot_p = haschecksum;
	pe_proc_fork(0, 1, 0);
	pe_proc_fork(0, 2, 1);
	pe_proc_fork(0, 3, 1);
	pe_proc_fork(0, 4, 1);

	pe_proc_fork(0, 21, 2);
	pe_proc_fork(0, 31, 3);

	/* No upgraders */
	WRITE(1, a1);
	WRITE(2, a2);
	WRITE(3, a3);
	WRITE(4, a4);
	WRITE(21, a21);
	WRITE(31, a31);

	MARK_UPGRADE(2);

	pe_proc_fork(0, 22, 2);
	pe_proc_fork(0, 32, 3);

	/* Upgraders are 2 (Parent) and 22 (Child) */
	WRITE(1, b1);
	WRITE(2, b2);		/* Add (cookie 0) */
	WRITE(3, b3);
	WRITE(4, b4);
	WRITE(21, b21);
	WRITE(22, b22);		/* Add (cookie 22) */
	WRITE(31, b31);
	WRITE(32, b32);

	MARK_UPGRADE(32);

	/* Upgraders are 2 and 32 (Parent) and 22, 321, 322, 323 (Children) */
	pe_proc_fork(0, 321, 32);
	pe_proc_fork(0, 322, 32);
	pe_proc_fork(0, 323, 32);

	WRITE(1, c1);
	WRITE(2, c2);		/* Add (cooke 0) */
	WRITE(3, c3);
	WRITE(4, c4);
	WRITE(21, c21);
	WRITE(22, c22);		/* Add (cookie 22) */
	WRITE(31, c31);
	WRITE(32, c32);		/* Add (cookie 0) */
	WRITE(321, c321);	/* Add (cookie 321) */
	WRITE(322, c322);	/* Add (cookie 322) */
	WRITE(323, c323);	/* Add (cookie 323) */

	WRITE(32, c322);	/* Change cookie to 0 */

	pe_proc_exit(321);

	/* Upgraders are 2 and 32 (Parent) and 22, 322, 323 (Children) */

	WRITE(1, d1);
	WRITE(2, d2);		/* Add (cookie 0) */
	WRITE(3, d3);
	WRITE(4, d4);
	WRITE(21, d21);
	WRITE(22, d22);		/* Add (cookie 22) */
	WRITE(31, d31);
	WRITE(32, d32);		/* Add (cookie 0) */
	WRITE(322, d322);	/* Add (cooke 322) */
	WRITE(323, d323);	/* Add (cookie 323) */

	END_UPGRADE(2);
	END_UPGRADE(32);

	/*
	 * No upgraders anymore! Upgraders that are still alive are:
	 * 22, 322, 323. This removes several files from the list.
	 */

	WRITE(1, e1);
	WRITE(2, e2);
	WRITE(3, e3);
	WRITE(4, e4);
	WRITE(21, e21);
	WRITE(22, e22);
	WRITE(31, e31);
	WRITE(32, e32);
	WRITE(322, e322);
	WRITE(323, e323);

	/*
	 * The files that should be in the upgrade list are stored int
	 * tc_file_expect[]. Compare them to the real values.
	 */
	for (pe_upgrade_filelist_start(); (cur=pe_upgrade_filelist_get());
	    pe_upgrade_filelist_next()) {
		for (i=0; tc_file_expect[i].path; ++i) {
			if (strcmp(tc_file_expect[i].path, cur->path) == 0)
				break;
		}
		fail_if(tc_file_expect[i].path == NULL,
		    "Path %s in upgrade list but shouldn't", cur->path);
		fail_if(tc_file_expect[i].found,
		    "Path %s uplicate in upgrade list", cur->path);
		fail_if(tc_file_expect[i].cookie != (int)cur->task_cookie,
		    "Path %s has cookie %lld (should be %d)",
		    cur->path, cur->task_cookie, tc_file_expect[i].cookie);
		tc_file_expect[i].found = 1;
	}

	for (i=0; tc_file_expect[i].path; ++i) {
		fail_if(tc_file_expect[i].found == 0, "Path %s should be "
		    "in file list but isn't", tc_file_expect[i].path);
	}

	pe_upgrade_finish();
	pe_upgrade_filelist_start();
	fail_if(pe_upgrade_filelist_get() != NULL,
	    "Non empty upgrade list but not in update");

	sfs_haschecksum_chroot_p = NULL;
	pe_shutdown();
}
END_TEST

/*
 * Testcases
 */
TCase *
anoubisd_testcase_pe_upgrade(void)
{
	/* policy engine test case */
	TCase *tc = tcase_create("UpgradeLogic");

	tcase_add_test(tc, tc_flags);
	tcase_add_test(tc, tc_files);

	return (tc);
}

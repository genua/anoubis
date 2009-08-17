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

#include <stdio.h>
#include <check.h>
#include <anoubischeck.h>

#include <config.h>
#include <sys/types.h>

#ifdef LINUX
#include <linux/anoubis.h>
#include <bsdcompat.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#include <pe.h>
#include <pe_filetree.h>
#include <amsg.h>
#include <aqueue.h>
#include <cfg.h>
#include <pe.h>
#include <sfs.h>
#include <anoubisd.h>
#include <cert.h>

#include <anoubisd_unit.h>

#ifndef __used
#define __used __attribute__((unused))
#endif

#define DEFINELOG(NAME)				\
void NAME(const char * fmt, ...)		\
{						\
	va_list ap;				\
	va_start(ap, fmt);			\
	vfprintf(stderr, fmt, ap);		\
	fprintf(stderr, "\n");			\
	va_end(ap);				\
}

DEFINELOG(log_warn)
DEFINELOG(log_warnx)
DEFINELOG(log_info)
DEFINELOG(log_debug)

void
fatal(const char *msg)
{
	log_warnx("%s", msg);
	exit(1);
}

void
fatalx(const char *msg)
{
	fatal(msg);
}

void
master_terminate(int code __used)
{
	fatalx("MASTER TERMINATE");
}

void
send_policychange(u_int32_t uid __used, u_int32_t prio __used)
{
}

void
send_lognotify(struct eventdev_hdr *hdr __used, u_int32_t error __used,
    u_int32_t loglevel __used, u_int32_t rule_id __used,
    u_int32_t prio __used, u_int32_t sfsmatch __used)
{
}

int
send_policy_data(u_int64_t token __used, int fd __used)
{
	return 0;
}

struct cert *
cert_get_by_uid(uid_t uid __used)
{
	return NULL;
}

char *
cert_keyid_for_uid(uid_t uid __used)
{
	return NULL;
}

void
cert_init(int chroot __used)
{
}

void
cert_reconfigure(int chroot __used)
{
}

int
sfs_checksumop_chroot(const struct sfs_checksumop *csop __used,
    struct sfs_data *data __used)
{
	return 0;
}

void
sfs_freesfsdata(struct sfs_data *data __used)
{
}

int (*sfs_haschecksum_chroot_p)(const char *path) = NULL;
int
sfs_haschecksum_chroot(const char *path)
{
	if (sfs_haschecksum_chroot_p)
		return sfs_haschecksum_chroot_p(path);
	return 0;
}

struct apn_ruleset *
pe_user_get_ruleset(uid_t uid __used, unsigned int prio __used,
    struct pe_policy_db *p __used)
{
	return NULL;
}

void
pe_user_dump(void)
{
}

void
pe_user_reconfigure(void)
{
}

void
pe_user_flush_db(struct pe_policy_db *ppdb __used)
{
}

void
pe_user_init(void)
{
}

anoubisd_reply_t *
pe_dispatch_policy(struct anoubisd_msg_comm *comm __used)
{
	fatalx("pe_dispatch_policy not supported in unit tests");
	return NULL;
}

void
send_upgrade_start(void)
{
}

unsigned int debug_flags = 0;
unsigned long version = ANOUBISCORE_VERSION;
enum anoubisd_process_type anoubisd_process = 0;
struct anoubisd_config anoubisd_config;

extern TCase	*anoubisd_testcase_pe(void);
extern TCase	*anoubisd_testcase_pe_filetree(void);
extern TCase	*anoubisd_testcase_pe_upgrade(void);

Suite*
peunit_testsuite(void)
{
	Suite	*s = suite_create("PEUnit");

	suite_add_tcase(s, anoubisd_testcase_pe_filetree());
	suite_add_tcase(s, anoubisd_testcase_pe());
	suite_add_tcase(s, anoubisd_testcase_pe_upgrade());

	return s;
}

int main()
{
	int		 result;
	SRunner		*runner = srunner_create(peunit_testsuite());

	srunner_run_all(runner, CK_NORMAL);
	result = check_eval_srunner(runner);
	srunner_free(runner);

	return result;
}

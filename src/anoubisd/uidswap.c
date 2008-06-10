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
#include <sys/param.h>
#include <errno.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include "config.h"
#include "anoubisd.h"

int
restore_uid(struct credentials *cred)
{
	int err = 0;

	if (seteuid(cred->euid) < 0)
		err = -EPERM;
	if (setgroups(cred->ngroups, cred->groups) < 0)
		err = -EPERM;
	if (setegid(cred->egid) < 0)
		err = -EPERM;
	return err;
}

int
switch_uid(uid_t uid, struct credentials *cred)
{
	struct passwd	*pw = getpwuid(uid);
	int		 err;

	if (!pw)
		return -ESRCH;
	cred->euid = geteuid();
	if (cred->euid < 0)
		return -errno;
	cred->egid = getegid();
	if (cred->egid < 0)
		return -errno;
	cred->ngroups = getgroups(NGROUPS_MAX, cred->groups);
	if (cred->ngroups < 0)
		return -errno;
	/* Now we actually start to switch UIDs */
	if (initgroups(pw->pw_name, pw->pw_gid) < 0)
		goto restore;
	if (setegid(pw->pw_gid) < 0)
		goto restore;
	if (seteuid(pw->pw_uid) < 0)
		goto restore;
	return 0;
restore:
	err = -errno;
	if (restore_uid(cred < 0)) {
		log_warnx("FATAL: Failed to change user ID and "
		    "cannot change back");
		master_terminate(EPERM);
	}
	return err;
}

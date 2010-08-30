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
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "apnvm_tc_fixtures.h"
#include <anoubischeck.h>

#define XXX_V "head	1.2;\n\
access;\n\
symbols;\n\
locks; strict;\n\
comment	@# @;\n\
\n\
\n\
1.2\n\
date	2009.05.13.13.43.12;	author rdoer;	state Exp;\n\
branches;\n\
next	1.1;\n\
commitid	kkdCB9J6T2BuqINt;\n\
\n\
1.1\n\
date	2009.05.13.13.42.57;	author rdoer;	state Exp;\n\
branches;\n\
next	;\n\
commitid	LGmcZgfgfUfpqINt;\n\
\n\
\n\
desc\n\
@@\n\
\n\
\n\
1.2\n\
log\n\
@<apnvm-metadata>\n\
comment := 2nd revision\n\
autostore := 1\n\
</apnvm-metadata>\n\
@\n\
text\n\
@alf {\n\
2: any {\n\
1: default allow\n\
}\n\
}\n\
sfs {\n\
3: any self valid allow invalid deny unknown continue\n\
}\n\
sandbox {\n\
}\n\
context {\n\
}\n\
@\n\
\n\
\n\
1.1\n\
log\n\
@<apnvm-metadata>\n\
comment := 1st revision\n\
autostore := 0\n\
</apnvm-metadata>\n\
@\n\
text\n\
@d2 10\n\
a11 3\n\
	any {\n\
		default allow\n\
	}\n\
@\n\
\n\
\n"

#define YYY_V "head	1.3;\n\
access;\n\
symbols;\n\
locks; strict;\n\
comment	@# @;\n\
\n\
\n\
1.3\n\
date	2009.05.13.13.43.04;	author rdoer;	state Exp;\n\
branches;\n\
next	1.2;\n\
commitid	HfN2UpjL7EurqINt;\n\
\n\
1.2\n\
date	2009.05.13.13.43.00;	author rdoer;	state Exp;\n\
branches;\n\
next	1.1;\n\
commitid	JUo6fEnEda0qqINt;\n\
\n\
1.1\n\
date	2009.05.13.13.42.57;	author rdoer;	state Exp;\n\
branches;\n\
next	;\n\
commitid	LGmcZgfgfUfpqINt;\n\
\n\
\n\
desc\n\
@@\n\
\n\
\n\
1.3\n\
log\n\
@<apnvm-metadata>\n\
comment := 3rd revision\n\
autostore := 2\n\
</apnvm-metadata>\n\
@\n\
text\n\
@alf {\n\
	any {\n\
		default allow\n\
	}\n\
}\n\
@\n\
\n\
\n\
1.2\n\
log\n\
@<apnvm-metadata>\n\
comment := 2nd revision\n\
autostore := 1\n\
</apnvm-metadata>\n\
@\n\
text\n\
@a5 3\n\
sfs {\n\
	/bin/ping a193a2edb06ff39630fed8195c0b043651867b91fccc8db67e4222367736ba73\n\
}\n\
@\n\
\n\
\n\
1.1\n\
log\n\
@<apnvm-metadata>\n\
comment := 1st revision\n\
autostore := 0\n\
</apnvm-metadata>\n\
@\n\
text\n\
@d1 5\n\
d7 1\n\
a7 1\n\
	any self valid allow invalid deny\n\
@\n\
\n\
\n"

#define ZZZ_V "head	1.1;\n\
access;\n\
symbols;\n\
locks; strict;\n\
comment	@# @;\n\
\n\
\n\
1.1\n\
date	2009.05.13.13.43.04;	author rdoer;	state Exp;\n\
branches;\n\
next	;\n\
commitid	HfN2UpjL7EurqINt;\n\
\n\
\n\
desc\n\
@@\n\
\n\
\n\
1.1\n\
log\n\
@<apnvm-metadata>\n\
comment := 3rd revision\n\
autostore := 2\n\
</apnvm-metadata>\n\
@\n\
text\n\
@sfs {\n\
	any self valid allow invalid deny\n\
}\n\
@\n\
\n"

#define ACTIVE_V "head	1.3;\n\
access;\n\
symbols;\n\
locks; strict;\n\
comment	@# @;\n\
\n\
\n\
1.3\n\
date	2009.05.13.13.43.10;	author rdoer;	state Exp;\n\
branches;\n\
next	1.2;\n\
commitid	mTIMafjIX2PtqINt;\n\
\n\
1.2\n\
date	2009.05.13.13.43.08;	author rdoer;	state Exp;\n\
branches;\n\
next	1.1;\n\
commitid	f23txTMUXpbtqINt;\n\
\n\
1.1\n\
date	2009.05.13.13.43.06;	author rdoer;	state Exp;\n\
branches;\n\
next	;\n\
commitid	ijHXc6SwKxqsqINt;\n\
\n\
\n\
desc\n\
@@\n\
\n\
\n\
1.3\n\
log\n\
@<apnvm-metadata>\n\
comment := 3rd revision\n\
autostore := 2\n\
</apnvm-metadata>\n\
@\n\
text\n\
@alf {\n\
	any {\n\
		default allow\n\
	}\n\
}\n\
sfs {\n\
	/bin/ping a193a2edb06ff39630fed8195c0b043651867b91fccc8db67e4222367736ba73\n\
}\n\
@\n\
\n\
\n\
1.2\n\
log\n\
@<apnvm-metadata>\n\
comment := 2nd revision\n\
autostore := 1\n\
</apnvm-metadata>\n\
@\n\
text\n\
@d1 5\n\
d7 1\n\
a7 1\n\
	any self valid allow invalid deny\n\
@\n\
\n\
\n\
1.1\n\
log\n\
@<apnvm-metadata>\n\
comment := 1st revision\n\
autostore := 0\n\
</apnvm-metadata>\n\
@\n\
text\n\
@d1 2\n\
a2 4\n\
alf {\n\
	any {\n\
		default allow\n\
	}\n\
@\n\
\n\
\n"

static int
vm_tc_exec(const char* cmd, ...)
{
	va_list ap;
	char str[1024], syscmd[2048];
	int rc;

	va_start(ap, cmd);
	vsnprintf(str, sizeof(str), cmd, ap);
	va_end(ap);

	snprintf(syscmd, sizeof(syscmd), "(%s) >/dev/null 2>&1", str);

	rc = system(syscmd);
	return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

void
apnvm_setup(void)
{
	char path[PATH_MAX];
	char *s;
	int fd, rc;

	strcpy(apnvm_cvsroot, "/tmp/tc_vm_XXXXXX");
	strcpy(apnvm_user, "user1");
	s = mkdtemp(apnvm_cvsroot);

	vm_tc_exec("cvs -d \"%s\" init", apnvm_cvsroot);

	/* user1 */
	snprintf(path, sizeof(path), "%s/user1", apnvm_cvsroot);
	mkdir(path, S_IRWXU);

	/* user1/user2 */
	snprintf(path, sizeof(path), "%s/user1/user2", apnvm_cvsroot);
	mkdir(path, S_IRWXU);

	/* user1/user3 */
	snprintf(path, sizeof(path), "%s/user1/user3", apnvm_cvsroot);
	mkdir(path, S_IRWXU);

	/* user1/user2/xxx,v */
	snprintf(path, sizeof(path), "%s/user1/user2/xxx,v", apnvm_cvsroot);
	fd = open(path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	rc = write(fd, XXX_V, strlen(XXX_V));
	close(fd);

	/* user1/user2/yyy,v */
	snprintf(path, sizeof(path), "%s/user1/user2/yyy,v", apnvm_cvsroot);
	fd = open(path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	rc = write(fd, YYY_V, strlen(YYY_V));
	close(fd);

	/* user1/user2/zzz,v */
	snprintf(path, sizeof(path), "%s/user1/user2/zzz,v", apnvm_cvsroot);
	fd = open(path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	rc = write(fd, ZZZ_V, strlen(ZZZ_V));
	close(fd);

	/* user1/user3/active,v */
	snprintf(path, sizeof(path), "%s/user1/user3/active,v", apnvm_cvsroot);
	fd = open(path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	rc = write(fd, ACTIVE_V, strlen(ACTIVE_V));
	close(fd);
}

void
apnvm_teardown(void)
{
	vm_tc_exec("rm -rf \"%s\"", apnvm_cvsroot);
}

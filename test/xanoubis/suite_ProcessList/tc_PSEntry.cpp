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

#include <check.h>
#include "PSEntry.h"

START_TEST(psentry_test)
{
	PSEntry                 *entry;

	Anoubis_ProcRecord	*proc;
	struct anoubis_proc	 info;

	unsigned char data[] = {
	    0,0,0,0,		// reclen, dont care
	    0,0,0,1,		// pid
	    0,0,0,0,0,0,0,0,	// taskcookie, dont care
	    0,0,0,0,0,0,0,2,	// pgid
	    0,0,0,3,		// uid
	    0,0,0,4, 0,0,0,5,	// alfrule
	    0,0,0,6, 0,0,0,7,	// sbrule
	    0,0,0,8, 0,0,0,9,	// ctxrule
	    1,			// secureexec
	    // payload 3x(checksum, name)
	    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,
	    'a',0,
	    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,
	    'b',0,
	    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,
	    'c',0,
	};
	proc = (Anoubis_ProcRecord*)data;

	info.pid = 1;
	info.ppid = 20;
	info.ruid = 21;
	info.euid = 22;
	info.suid = 23;
	info.rgid = 24;
	info.egid = 25;
	info.sgid = 26;
	info.ngroups = 0;
	info.groups = NULL;
	info.comm = (char*)"d"; // ugly cast to make compiler happy

	/* create an instance */
	fprintf(stderr, "now creating PSEntry\n");

	entry = new PSEntry(proc, &info);

	fprintf(stderr, "dump new entry %ls\n",
	    entry->getProcessName().c_str());
	fprintf(stderr, "pid=%ls, ppid=%ls, secex=%d\n",
	    entry->getProcessId().c_str(), entry->getParentProcessId().c_str(),
	    entry->getSecureExec());
	fprintf(stderr, "uid=%ls, ruid=%ls, gid=%ls, rgid=%ls\n",
	    entry->getUID().c_str(), entry->getEUID().c_str(),
	    entry->getGID().c_str(), entry->getEGID().c_str());
	fprintf(stderr, "process: path=%ls, csum=%ls\n",
	    entry->getPathProcess().c_str(),
	    entry->getChecksumProcess().c_str());
	fprintf(stderr, "admin context: path=%ls, csum=%ls\n",
	    entry->getPathAdminContext().c_str(),
	    entry->getChecksumAdminContext().c_str());
	fprintf(stderr, "user context: path=%ls, csum=%ls\n",
	    entry->getPathUserContext().c_str(),
	    entry->getChecksumUserContext().c_str());

	/* make sure that all getters return what we created */
	fail_if(entry->getProcessId().Cmp(wxT("1")) != 0);
	fail_if(entry->getParentProcessId().Cmp(wxT("20")) != 0);
	fail_if(entry->getUID().Cmp(wxT("21"))  != 0);
	fail_if(entry->getEUID().Cmp(wxT("22")) != 0);
	fail_if(entry->getGID().Cmp(wxT("24"))  != 0);
	fail_if(entry->getEGID().Cmp(wxT("25")) != 0);
	fail_if(entry->getProcessName().Cmp(wxT("d")) != 0);

	fail_if(entry->getSecureExec() != true);
	fail_if(entry->getPathAdminContext().Cmp(wxT("a")) != 0);
	fail_if(entry->getPathUserContext().Cmp(wxT("b")) != 0);
	fail_if(entry->getPathProcess().Cmp(wxT("c")) != 0);
	fail_if(entry->getChecksumAdminContext().Cmp(wxT("0000000000000000"
	    "00000000000000000000000000000000000000000000000b")) != 0);
	fail_if(entry->getChecksumUserContext().Cmp(wxT("0000000000000000"
	    "00000000000000000000000000000000000000000000000c")) != 0);
	fail_if(entry->getChecksumProcess().Cmp(wxT("0000000000000000"
	    "00000000000000000000000000000000000000000000000d")) != 0);

	delete(entry);
}
END_TEST

TCase *
getTc_PSEntry(void)
{
	TCase *testCase;

	testCase = tcase_create("Process list");
	tcase_add_test(testCase, psentry_test);

	return (testCase);
}

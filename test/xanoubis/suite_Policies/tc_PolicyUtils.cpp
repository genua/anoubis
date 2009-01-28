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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>

#include <wx/string.h>
#include "PolicyUtils.h"

START_TEST(PolicyUtils_csumToString)
{
	unsigned char csum[MAX_APN_HASH_LEN];
	char cstxt[3+2*MAX_APN_HASH_LEN];
	wxString	s = wxT("some other text");
	int i;

	cstxt[0] = '0';
	cstxt[1] = 'x';
	for (i=0; i<MAX_APN_HASH_LEN; ++i) {
		int val = (15*(i+1)) % 256;
		int j = 2*i+3;
		int x;
		csum[i] = val;
		for (x=0; x<2; ++x) {
			if (val % 16 >= 10)
				cstxt[j] = 'a' - 10 + val % 16;
			else
				cstxt[j] = '0' + val % 16;
			val /= 16;
			j--;
		}
	}
	cstxt[2*MAX_APN_HASH_LEN+2] = 0;
	fail_if(strcmp(s.fn_str(), "some other text") != 0,
	    "Initialization of wxString failed, got '%s' instead of '%s'",
	    (const char *)s.fn_str(), "some other text");
	for (i = 1; i <= MAX_APN_HASH_LEN; ++i) {
		int ret = PolicyUtils::csumToString(csum, i, s);
		fail_unless(ret, "csumToString failed for len=%d", i);
		fail_unless(strlen(s.fn_str()) == 2U*i+2,
		    "Wrong length for csum, should be %d but is",
		    2*i+2, strlen(s.fn_str()));
		ret = strncmp(s.fn_str(), cstxt, strlen(s.fn_str()));
		fail_unless(ret == 0, "Wrong checksum '%s' returned instead "
		    "of '%*.*s' (csum[%d]=%d)", (const char *)s.fn_str(),
		    2*i+2, 2*i+2, cstxt, i-1, csum[i-1]);
	}
	fail_if(PolicyUtils::csumToString(csum, 0, s),
	    "csumToString with length 0 should fail");
	fail_if(PolicyUtils::csumToString(csum, i, s),
	    "csumToString with length %d should fail");
}
END_TEST

TCase *
getTc_PolicyUtils(void)
{
	TCase *testCase;

	testCase = tcase_create("PolicyUtils");
	tcase_add_test(testCase, PolicyUtils_csumToString);

	return (testCase);
}

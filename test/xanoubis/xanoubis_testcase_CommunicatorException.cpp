/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#include "anoubischat.h"
#include "CommunicatorException.h"

START_TEST(tc_CommunicatorException_creation)
{
	CommunicatorException *ce = new CommunicatorException();
	fail_if(ce == NULL,
	    "couldn't create instance of CommunicatorException");
	delete ce;
}
END_TEST

START_TEST(tc_CommunicatorException_throwEmpty)
{
	bool wasThrown = false;

	try {
		throw new CommunicatorException();
	} catch(CommunicatorException *ce) {
		fail_if(ce == NULL, "got empty exception pointer");
		wasThrown = true;
	}

	fail_if(wasThrown == false,
	    "empty communicator exception wasn't throwen");
}
END_TEST

START_TEST(tc_CommunicatorException_throwError)
{
	bool wasThrown = false;

	try {
		throw new CommunicatorException(ACHAT_RC_ERROR);
	} catch(CommunicatorException *ce) {
		fail_if(ce == NULL, "got empty exception pointer");
		fail_if(ce->getReason() != ACHAT_RC_ERROR,
		    "exception returned wrong reason!");
		wasThrown = true;
	}

	fail_if(wasThrown == false,
	    "empty communicator exception wasn't throwen");
}
END_TEST

TCase *
xanoubis_testcase_CommunicatorException(void)
{
	/* CommunicatorException test case */
	TCase *tc_ce = tcase_create("CommunicatorException");

	tcase_add_test(tc_ce, tc_CommunicatorException_creation);
	tcase_add_test(tc_ce, tc_CommunicatorException_throwEmpty);
	tcase_add_test(tc_ce, tc_CommunicatorException_throwError);

	return (tc_ce);
}

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

#include <check.h>
#include <string.h>

#include "anoubis_chat.h"

START_TEST(tc_recvmsg_parameter)
{
	struct achat_channel	c;
	char buf[ACHAT_MAX_MSGSIZE+1];
	size_t			size;

	bzero(&c, sizeof(c));
	size = 0;

	fail_if(acc_receivemsg(NULL, NULL, &size) != ACHAT_RC_INVALPARAM);
	fail_if(acc_receivemsg(&c, NULL, &size) != ACHAT_RC_INVALPARAM);
	fail_if(acc_receivemsg(&c, NULL, &size) != ACHAT_RC_INVALPARAM);
	fail_if(acc_receivemsg(&c, buf, &size) != ACHAT_RC_INVALPARAM);
	size = ACHAT_MAX_MSGSIZE + 1;
	fail_if(acc_receivemsg(&c, buf, &size) != ACHAT_RC_INVALPARAM);
}
END_TEST

TCase *
libanoubischat_testcase_recvmsg(void)
{
	TCase *tc_recvmsg = tcase_create("recvmsg");

	tcase_add_test(tc_recvmsg, tc_recvmsg_parameter);

	return (tc_recvmsg);
}

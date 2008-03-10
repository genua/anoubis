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

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <check.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bsdcompat.h>

void
dump(void *data, size_t len)
{
	unsigned long	*p;
	int		 i;

	p = (unsigned long *)data;

	if ((len % 4) != 0)
		return;

	for (i = 0; i < len / 4; i++)
		/* quick hack, but ok and portabel */
		printf("%08x", htonl(p[i]));

	printf("\n");
}

int
verify(void *data, size_t len, void *check)
{
	int	res;

	res = memcmp(data, check, len);
	if (res != 0) {
		printf("result should be\t");
		dump(check, len);
		printf("however result is\t");
		dump(data, len);
	}
	
	return (res);
}

/*
 * CIDR fully specified.
 */
START_TEST(tc_inet_net_pton_1)
{
	int		bits;
	struct in_addr	ina;
	char		s[] = "192.168.222.0/24";

	/* Result in network byte order */
	u_int8_t	check[] = { 0xc0, 0xa8, 0xde, 0x00 };

	bzero(&ina, sizeof(ina));
	bits = inet_net_pton(AF_INET, s, &ina, sizeof(ina));

	fail_if(bits != 24, "inet_net_pton for \"%s\" failed with bits %d, "
	    "should be 24", s, bits);
	fail_if(verify(&ina, sizeof(ina), check) != 0, "output mismatch");
}
END_TEST

/*
 * Host address, thus size must be 32.
 */
START_TEST(tc_inet_net_pton_2)
{
	int		bits;
	struct in_addr	ina;
	char		s[] = "192.168.222.24";

	/* Result in network byte order */
	u_int8_t	check[] = { 0xc0, 0xa8, 0xde, 0x18 };

	bzero(&ina, sizeof(ina));
	bits = inet_net_pton(AF_INET, s, &ina, sizeof(ina));

	fail_if(bits != 32, "inet_net_pton for \"%s\" failed with bits %d, "
	    "should be 32", s, bits);
	fail_if(verify(&ina, sizeof(ina), check) != 0, "output mismatch");
}
END_TEST

/*
 * Abbreviated CIDR.
 */
START_TEST(tc_inet_net_pton_3)
{
	int		bits;
	struct in_addr	ina;
	char		s[] = "192.100/16";

	/* Result in network byte order */
	u_int8_t	check[] = { 0xc0, 0x64, 0x00, 0x00 };

	bzero(&ina, sizeof(ina));
	bits = inet_net_pton(AF_INET, s, &ina, sizeof(ina));

	fail_if(bits != 16, "inet_net_pton for \"%s\" failed with bits %d, "
	    "should be 16", s, bits);
	fail_if(verify(&ina, sizeof(ina), check) != 0, "output mismatch");
}
END_TEST

TCase *
bsdcompat_testcase_inet_net_pton(void)
{
	TCase	*tc_inet_net_pton = tcase_create("inet_net_pton()");

	tcase_add_test(tc_inet_net_pton, tc_inet_net_pton_1);
	tcase_add_test(tc_inet_net_pton, tc_inet_net_pton_2);
	tcase_add_test(tc_inet_net_pton, tc_inet_net_pton_3);

	return (tc_inet_net_pton);
}

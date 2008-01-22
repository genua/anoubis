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

#ifndef _ACCUTILS_H_
#define _ACCUTILS_H_

#include <sys/cdefs.h>

#include "anoubischat.h"

#define ACC_CHKPARAM(cond) \
	do { \
		if (!(cond)) \
			return (ACHAT_RC_INVALPARAM); \
	} while (0)

#define ACC_CHKSTATE(channel, cs) \
	do { \
		if (channel->state != cs) \
			return (ACHAT_RC_WRONGSTATE); \
	} while (0)

#define accwrite (ssize_t (*)(int, void *, size_t))write

__BEGIN_DECLS
achat_rc acc_statetransit(struct achat_channel *, enum acc_state);
void acc_sockaddrcpy(struct sockaddr_storage *, struct sockaddr_storage *);
socklen_t acc_sockaddrsize(struct sockaddr_storage *);
achat_rc acc_io(struct achat_channel *, ssize_t (*f) (int, void *, size_t),
    char *, size_t);
__END_DECLS

#endif	/* _ACCUTILS_H_ */

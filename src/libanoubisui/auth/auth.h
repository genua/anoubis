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

#ifndef _AUTH_H_
#define _AUTH_H_

#include <openssl/rand.h>
#include <sys/types.h>
#include <sys/cdefs.h>

#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_sig.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

/*
 * Flag bits
 */
#define ANOUBIS_AUTHFLAG_IGN_KEY_MISMATCH	0x01

/*
 * Error codes
 */
#define ANOUBIS_AUTHERR_INVAL		0x3000 /* Invalid arguments */
#define ANOUBIS_AUTHERR_PKG		0x3004 /* Invalid package */
#define ANOUBIS_AUTHERR_KEY		0x3006 /* Invalid private key */
#define ANOUBIS_AUTHERR_CERT		0x3007 /* Invalid certificate */
#define ANOUBIS_AUTHERR_KEY_MISMATCH	0x3008 /* Certificate mismatch */
#define ANOUBIS_AUTHERR_RAND		0x3009 /* No entropy */
#define ANOUBIS_AUTHERR_NOMEM		0x300A /* Out of memory */
#define ANOUBIS_AUTHERR_SIGN		0x300C /* Error signing challenge */

__BEGIN_DECLS

/**
 * Callback for anoubis protocol to assemble authentication reply message.
 *
 * This method deals with authentication request messages and is intentional a
 * callback for all clients. An authentication reply message is assembled. The
 * received challenge and an random initilaize vector are signed and sent
 * back.
 * With given flags you can ignore mismatched key.
 *
 * @param[in] 1st Private key.
 * @param[in] 2nd Certificate.
 * @param[in] 3rd Authentication request message.
 * @param[out] 4th Assembled authentication reply message.
 * @param[in] 5th Flags
 * @return 0 on success or a negative ANOUBIS_AUTHERR_* error code.
 */
int anoubis_auth_callback(struct anoubis_sig *, struct anoubis_sig *,
    struct anoubis_msg *, struct anoubis_msg **, int);

__END_DECLS

#endif	/* _AUTH_H_ */

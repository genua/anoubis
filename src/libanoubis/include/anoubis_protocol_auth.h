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
#ifndef ANOUBIS_AUTH_H
#define ANOUBIS_AUTH_H

#include <sys/types.h>
#include <anoubis_protocol.h>
#include <anoubis_chat.h>

struct anoubis_msg;

#define ANOUBIS_AUTH_INIT		0
#define ANOUBIS_AUTH_SUCCESS		1
#define ANOUBIS_AUTH_FAILURE		2
#define ANOUBIS_AUTH_INPROGRESS		3

#define ANOUBIS_AUTH_TRANSPORT		0
#define ANOUBIS_AUTH_TRANSPORTANDKEY	1
#define ANOUBIS_AUTH_CHALLENGE		2
#define ANOUBIS_AUTH_CHALLENGEREPLY	3

typedef void (*anoubis_auth_callback_t)(void * caller);

struct anoubis_auth {
	int state;
	int auth_type;
	int error;
	uid_t uid;
	struct achat_channel * chan;
	anoubis_auth_callback_t finish_callback;
	void * cbdata;
	void * auth_private;
};

typedef struct {
	u32n	type;
	u32n	auth_type;
} __attribute__((packed)) Anoubis_AuthTransportMessage;

typedef struct {
	u32n	type;
	u32n	auth_type;
	u32n	challengelen;
	u32n	idlen;
	char	payload[0];
} __attribute__((packed)) Anoubis_AuthChallengeMessage;

typedef struct {
	u32n	type;
	u32n	auth_type;
	u32n	uid;
	u32n	ivlen;
	u32n	siglen;
	char	payload[0];
} __attribute__((packed)) Anoubis_AuthChallengeReplyMessage;

/*@null@*/ /*@only@*/
struct anoubis_auth * anoubis_auth_create(
    /*@dependent@*/ struct achat_channel * chan,
    anoubis_auth_callback_t finish, /*@dependent@*/ void * cbdata);
void anoubis_auth_destroy(/*@only@*/ struct anoubis_auth * auth);

#endif

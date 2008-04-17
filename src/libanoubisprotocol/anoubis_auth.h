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
#include <anoubischat.h>

struct anoubis_msg;

#define ANOUBIS_AUTH_INIT		0
#define ANOUBIS_AUTH_SUCCESS		1
#define ANOUBIS_AUTH_FAILURE		2
#define ANOUBIS_AUTH_INPROGRESS		3

#define ANOUBIS_AUTH_TRANSPORT	0

typedef void (*anoubis_auth_callback_t)(void * caller);

struct anoubis_auth {
	int state;
	int auth_type;
	int error;
	uid_t uid;
	/*@only@*/ /*@relnull@*/
	char * username;
	/*@dependent@*/
	struct achat_channel * chan;
	anoubis_auth_callback_t finish_callback;
	/*@dependent@*/
	void * cbdata;
};

typedef struct {
	u32n	type;
	u32n	auth_type;
} __attribute__((packed)) Anoubis_AuthTransportMessage;

/*@null@*/ /*@only@*/
struct anoubis_auth * anoubis_auth_create(
    /*@dependent@*/ struct achat_channel * chan,
    anoubis_auth_callback_t finish, /*@dependent@*/ void * cbdata);
void anoubis_auth_destroy(/*@only@*/ struct anoubis_auth * auth);
int anoubis_auth_process(struct anoubis_auth * auth,
    struct anoubis_msg * m);

#endif

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
#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <anoubis_msg.h>
#include <anoubis_auth.h>
#include <anoubis_errno.h>
#include <anoubischat.h>

struct anoubis_auth * anoubis_auth_create(struct achat_channel * chan,
    anoubis_auth_callback_t finish, void * cbdata)
{
	struct anoubis_auth * ret = malloc(sizeof(struct anoubis_auth));
	if (!ret)
		return NULL;
	ret->state = ANOUBIS_AUTH_INIT;
	ret->auth_type = -1;
	ret->uid = 0;
	ret->username = NULL;
	ret->error = 0;
	ret->chan = chan;
	ret->finish_callback = finish;
	ret->cbdata = cbdata;
	return ret;
}


void anoubis_auth_destroy(struct anoubis_auth * auth)
{
	if (auth->username)
		free(auth->username);
	free(auth);
}

int anoubis_auth_process(struct anoubis_auth * auth,
    struct anoubis_msg * m)
{
# ifndef S_SPLINT_S
	/*
	 * XXX tartler: this part doesn't parse with splint :(
	 */

	if (!VERIFY_LENGTH(m, sizeof(Anoubis_AuthTransportMessage))
	    || get_value(m->u.general->type) != ANOUBIS_C_AUTHDATA
	    || !auth || !auth->chan || auth->state != ANOUBIS_AUTH_INIT) {
		auth->error = ANOUBIS_E_INVAL;
		auth->state = ANOUBIS_AUTH_FAILURE;
		return -EINVAL;
	}
	if (auth->chan->euid == -1) {
		auth->error = ANOUBIS_E_PERM;
		auth->state = ANOUBIS_AUTH_FAILURE;
		auth->uid = -1;
	} else {
		auth->uid = auth->chan->euid;
		auth->state = ANOUBIS_AUTH_SUCCESS;
		auth->username = NULL; /* XXX for now. -- ceh */
		auth->state = ANOUBIS_AUTH_SUCCESS;
	}
	auth->finish_callback(auth->cbdata);
	return 0;
# endif
}

#ifndef ANOUBIS_AUTH_H
#define ANOUBIS_AUTH_H

#define ANOUBIS_AUTH_INIT	0
#define ANOUBIS_AUTH_SUCCESS	1
#define ANOUBIS_AUTH_FAILURE	2

#define ANOUBIS_AUTH_TRANSPORT	0

struct anoubis_auth {
	int state;
	int auth_type;
	uid_t uid;
	char * username;
};

struct anoubis_auth * anoubis_auth_create(void);
void * anoubis_auth_destroy(struct anoubis_auth * auth);
struct anoubis_msg * anoubis_auth_process(struct anoubis_auth * auth,
    struct anoubis_msg * m);

#endif

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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "accutils.h"
#include "anoubischat.h"

achat_rc
acc_statetransit(struct achat_channel *acc, enum acc_state newstate)
{
	ACC_CHKPARAM(acc != NULL);
	ACC_CHKPARAM((0 <= newstate)&&(newstate < 6));

	switch (newstate) {
	case ACC_STATE_NONE:
		acc->state = newstate;
		break;
	case ACC_STATE_INITIALISED:
		ACC_CHKSTATE(acc, ACC_STATE_NONE);
		if ((acc->addr.ss_family != 0)  &&
		    (acc->tail != ACC_TAIL_NONE) &&
		    (acc->sslmode != ACC_SSLMODE_NONE))
			acc->state = newstate;
		break;
	case ACC_STATE_NOTCONNECTED:
		ACC_CHKSTATE(acc, ACC_STATE_INITIALISED);
		switch (acc->tail) {
		case ACC_TAIL_SERVER:
			if (acc->sockfd >= 0)
				acc->state = newstate;
			break;
		case ACC_TAIL_CLIENT:
			if (acc->connfd >= 0)
				acc->state = newstate;
			break;
		default:
			break;
		}
		break;
	case ACC_STATE_ESTABLISHED:
		ACC_CHKSTATE(acc, ACC_STATE_NOTCONNECTED);
		if (acc->connfd >= 0)
			acc->state = newstate;
		break;
	case ACC_STATE_TRANSFERING:
		/* XXX by ch: missing requirements for state transission */
		return (ACHAT_RC_NYI);
		break;
	case ACC_STATE_CLOSED:
		acc->state = newstate;
		break;
	default:
		/* this should never be reached */
		return (ACHAT_RC_ERROR);
		break;
	}

	return (ACHAT_RC_OK);
}

void
acc_sockaddrcpy(struct sockaddr_storage *dest, struct sockaddr_storage *src)
{
	struct sockaddr_un *ssa_un = (struct sockaddr_un *)src;
	struct sockaddr_un *dsa_un = (struct sockaddr_un *)dest;
	struct sockaddr_in *ssa_in = (struct sockaddr_in *)src;
	struct sockaddr_in *dsa_in = (struct sockaddr_in *)dest;
	struct sockaddr_in6 *ssa_in6 = (struct sockaddr_in6 *)src;
	struct sockaddr_in6 *dsa_in6 = (struct sockaddr_in6 *)dest;

	switch (src->ss_family) {
	case AF_UNIX:
		dsa_un->sun_family = AF_UNIX;
#ifdef OPENBSD
		dsa_un->sin6_len = acc_sockaddrsize(src);
#endif /* OPENBSD */
		/* XXX HJH strlcpy ifdef OPENBSD */
		strncpy(dsa_un->sun_path, ssa_un->sun_path,
		    sizeof(dsa_un->sun_path) - 1);
		dsa_un->sun_path[sizeof(dsa_un->sun_path) - 1] = '\0';
		break;
	case AF_INET:
		dsa_in->sin_family = AF_INET;
#ifdef OPENBSD
		dsa_in6->sin_len = acc_sockaddrsize(src);
#endif /* OPENBSD */
		dsa_in->sin_addr.s_addr = ssa_in->sin_addr.s_addr;
		dsa_in->sin_port = ssa_in->sin_port;
		break;
	case AF_INET6:
		dsa_in6->sin6_family = AF_INET6;
#ifdef OPENBSD
		dsa_in6->sin6_len = acc_sockaddrsize(src);
#endif /* OPENBSD */
		memcpy(&dsa_in6->sin6_addr, &ssa_in6->sin6_addr,
		    sizeof(dsa_in6->sin6_addr));
		dsa_in6->sin6_port = ssa_in6->sin6_port;
		dsa_in6->sin6_scope_id = ssa_in6->sin6_scope_id;
		break;
	default:
		break;
	}
}

socklen_t
acc_sockaddrsize(struct sockaddr_storage *sa)
{
	size_t size = 0;

	switch (sa->ss_family) {
	case AF_UNIX:
		/* XXX HJH is this correct? */
		size = sizeof(((struct sockaddr_un*)sa)->sun_family) +
		    strlen(((struct sockaddr_un*)sa)->sun_path) + 1;
		break;
	case AF_INET:
		size = sizeof(struct sockaddr_in);
		break;
	case AF_INET6:
		size = sizeof(struct sockaddr_in6);
		break;
	default:
		/* we don't support other socket types yet */
		break;
	}

	return (size);
}

achat_rc
acc_io(struct achat_channel *acc, ssize_t (*f) (int, void *, size_t),
    char *buf, size_t *size)
{
	size_t   pos = 0;
	ssize_t  res = 0;
	achat_rc rc  = ACHAT_RC_OK;

	ACC_CHKPARAM(acc  != NULL);
	ACC_CHKPARAM(f    != NULL);
	ACC_CHKPARAM(buf  != NULL);
	ACC_CHKPARAM(*size >= 0);

	while ((*size > pos) && (rc == ACHAT_RC_OK)) {
		res = (f)(acc->connfd, buf + pos, *size - pos);
		switch (res) {
		case -1:
			/* XXX HJH EAGAIN and U_NONBLOCK -> busy loop? */
			if (errno == EINTR || errno == EAGAIN)
				continue;
			rc = ACHAT_RC_ERROR;
			break;
		case 0:
			rc = ACHAT_RC_EOF;
			break;
		default:
			if (res < 0) {
				rc = ACHAT_RC_ERROR;
				break;
			}
			pos += (size_t)res;
			break;
		}
	}

	*size = pos;
	return (rc);
}

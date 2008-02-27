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
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "accutils.h"
#include "anoubischat.h"

achat_rc
acc_prepare(struct achat_channel *acc)
{
	size_t size;
	int rc;

	ACC_CHKPARAM(acc != NULL);
	ACC_CHKSTATE(acc, ACC_STATE_INITIALISED);

	switch (acc->tail) {
	case ACC_TAIL_SERVER:
		size = acc_sockaddrsize(&acc->addr);

		acc->sockfd = socket(acc->addr.ss_family, SOCK_STREAM, 0);
		if (acc->sockfd == -1)
			return (ACHAT_RC_ERROR);

		if (acc->addr.ss_family == AF_UNIX)
			unlink(((struct sockaddr_un*)&(acc->addr))->sun_path);

		rc = bind(acc->sockfd, (struct sockaddr*)&(acc->addr), size);
		if (rc == -1)
			return (ACHAT_RC_ERROR);

		rc = listen(acc->sockfd, ACHAT_MAX_BACKLOG);
		if (rc == -1)
			return (ACHAT_RC_ERROR);
		break;
	case ACC_TAIL_CLIENT:
		acc->connfd = socket(acc->addr.ss_family, SOCK_STREAM, 0);
		if (acc->connfd == -1)
			return (ACHAT_RC_ERROR);
		break;
	default:
		return (ACHAT_RC_ERROR);
		break;
	}

	acc_statetransit(acc, ACC_STATE_NOTCONNECTED);

	return (ACHAT_RC_OK);
}

achat_rc
acc_open(struct achat_channel *acc)
{
	struct sockaddr_storage remote;
	socklen_t size;
	int rc;

	ACC_CHKPARAM(acc != NULL);
	ACC_CHKSTATE(acc, ACC_STATE_NOTCONNECTED);

	if (acc->tail == ACC_TAIL_SERVER) {
		size = sizeof(remote);
		/* retry if we were interrupted (e.g by a signal) */
		do {
			acc->connfd = accept(acc->sockfd,
			    (struct sockaddr *)&remote, &size);
		} while ((acc->connfd == -1) && (errno == EINTR));
		if (acc->connfd == -1)
			return (ACHAT_RC_ERROR);
	} else {
		size =  acc_sockaddrsize(&(acc->addr));
		/* retry if we were interrupted (e.g by a signal) */
		do {
			rc = connect(acc->connfd,
			    (struct sockaddr *)&acc->addr, size);
		} while (rc == EINTR);
		if (rc != 0) {
			close(acc->connfd);
			acc->connfd = -1;
			return (ACHAT_RC_ERROR);
		}
	}

	acc_statetransit(acc, ACC_STATE_ESTABLISHED);

	return (ACHAT_RC_OK);
}

struct achat_channel *
acc_opendup(struct achat_channel *acc)
{
	struct achat_channel	*nc;

	if (acc == NULL)
		return (NULL);

	nc = acc_create();
	if (nc == NULL)
		return (NULL);

	memcpy(nc, acc, sizeof(struct achat_channel));
	nc->sockfd = dup(acc->sockfd);
	if (nc->sockfd == -1) {
		acc_clear(nc);
		acc_destroy(nc);
		return (NULL);
	}

	if (acc_open(nc) != ACHAT_RC_OK) {
		acc_clear(nc);
		acc_destroy(nc);
		return (NULL);
	}

	return (nc);
}

achat_rc
acc_close(struct achat_channel *acc)
{
	ACC_CHKPARAM(acc != NULL);

	close(acc->sockfd);
	close(acc->connfd);

	if ((acc->addr.ss_family == AF_UNIX) && (acc->tail == ACC_TAIL_SERVER))
		unlink(((struct sockaddr_un*)&(acc->addr))->sun_path);

	acc_statetransit(acc, ACC_STATE_CLOSED);

	return (ACHAT_RC_OK);
}

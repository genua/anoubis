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

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#ifdef NEEDBSDCOMPAT
#include "bsdcompat.h"
#endif

#include <netinet/in.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "accutils.h"
#include "accbuffer.h"
#include "anoubischat.h"

achat_rc
acc_prepare(struct achat_channel *acc)
{
	socklen_t size;
	int rc;

	ACC_CHKPARAM(acc != NULL);
	ACC_CHKSTATE(acc, ACC_STATE_INITIALISED);

	switch (acc->tail) {
	case ACC_TAIL_SERVER:
		size = acc_sockaddrsize(&acc->addr);

		acc->fd = socket(acc->addr.ss_family, SOCK_STREAM, 0);
		if (acc->fd == -1)
			return (ACHAT_RC_ERROR);

		if (fcntl(acc->fd, F_SETFD, FD_CLOEXEC) == -1)
			return (ACHAT_RC_ERROR);

		if (acc->addr.ss_family == AF_UNIX)
			unlink(((struct sockaddr_un*)&(acc->addr))->sun_path);

		rc = bind(acc->fd, (struct sockaddr*)&(acc->addr), size);
		if (rc == -1)
			return (ACHAT_RC_ERROR);

		if (acc->addr.ss_family == AF_UNIX) {
			/*
			 * Flawfider suggests to use fchmod() instead, but
			 * that's not working for/on sockets; thus we
			 * stop flawfinder complaining about.
			 */
			/* Flawfinder: ignore */
			rc = chmod(
			    ((struct sockaddr_un*)&(acc->addr))->sun_path,
			    S_IRWXU | S_IRWXG | S_IRWXO);
			if (rc == -1)
				return (ACHAT_RC_ERROR);
		}

		rc = listen(acc->fd, ACHAT_MAX_BACKLOG);
		if (rc == -1)
			return (ACHAT_RC_ERROR);
		break;
	case ACC_TAIL_CLIENT:
		acc->fd = socket(acc->addr.ss_family, SOCK_STREAM, 0);
		if (acc->fd == -1)
			return (ACHAT_RC_ERROR);

		if (fcntl(acc->fd, F_SETFD, FD_CLOEXEC) == -1)
			return (ACHAT_RC_ERROR);

		break;
	default:
		return (ACHAT_RC_ERROR);
		break;
	}

	return acc_statetransit(acc, ACC_STATE_NOTCONNECTED);
}

achat_rc
acc_open(struct achat_channel *acc)
{
	socklen_t size;
	int rc;

	ACC_CHKPARAM(acc != NULL);
	ACC_CHKSTATE(acc, ACC_STATE_NOTCONNECTED);

	if (acc->tail == ACC_TAIL_SERVER) {
		struct achat_channel	*nc = acc_opendup(acc);

		if (nc == NULL)
			return (ACHAT_RC_ERROR);

		acc_close(acc);
		memcpy(acc, nc, sizeof(struct achat_channel));
		/* Change state back, otherwise acc_statetransit will fail */
		acc->state = ACC_STATE_NOTCONNECTED;

		/* Destroy temporary nc */
		/* set state to NONE to prevent closing the socket */
		nc->state = ACC_STATE_NONE;
		acc_destroy(nc);
	} else {
		size = acc_sockaddrsize(&(acc->addr));
		/* retry if we were interrupted (e.g by a signal) */
		do {
			rc = connect(acc->fd,
			    (struct sockaddr *)&acc->addr, size);
		} while (rc == EINTR);
		if (rc != 0) {
			close(acc->fd);
			acc->fd = -1;
			return (ACHAT_RC_ERROR);
		}
	}

	rc = acc_getpeerids(acc);
	if (rc != ACHAT_RC_OK) {
		close(acc->fd);
		acc->fd = -1;
		return (ACHAT_RC_ERROR);
	}

	return acc_statetransit(acc, ACC_STATE_ESTABLISHED);
}

struct achat_channel *
acc_opendup(struct achat_channel *acc)
{
	struct achat_channel	*nc;
	struct sockaddr_storage	remote;
	socklen_t		size;

	if (acc == NULL || acc->sendbuffer || acc->event)
		return (NULL);

	if (acc->tail != ACC_TAIL_SERVER)
		return (NULL); /* Operation only allowed on server-tail */

	nc = acc_create();
	if (nc == NULL)
		return (NULL);

	memcpy(nc, acc, sizeof(struct achat_channel));
	nc->tail = ACC_TAIL_CLIENT;

	/* retry if we were interrupted (e.g by a signal) */
	size = (socklen_t) sizeof(remote);
	do {
		nc->fd = accept(acc->fd,
		    (struct sockaddr *)&remote, &size);
	} while ((nc->fd == -1) && (errno == EINTR));
	if (nc->fd == -1) {
		acc_clear(nc);
		acc_destroy(nc);
		return (NULL);
	}

	if (acc_getpeerids(nc) != ACHAT_RC_OK) {
		acc_clear(nc);
		acc_destroy(nc);
		return (NULL);
	}

	if (acc_statetransit(nc, ACC_STATE_ESTABLISHED) != ACHAT_RC_OK) {
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

	if (acc->sendbuffer) {
		acc_bufferfree(acc->sendbuffer);
		free(acc->sendbuffer);
		acc->sendbuffer = NULL;
	}
	acc->event = NULL;
	close(acc->fd);

	if ((acc->addr.ss_family == AF_UNIX) && (acc->tail == ACC_TAIL_SERVER))
		unlink(((struct sockaddr_un*)&(acc->addr))->sun_path);

	return acc_statetransit(acc, ACC_STATE_CLOSED);
}

achat_rc
acc_getpeerids(struct achat_channel *acc)
{
	int	rc;

	ACC_CHKPARAM(acc != NULL);
	/*
	 * Currently we can retrieve the user information only from
	 * unix domain sockets. -- CH
	 */
	if (acc->addr.ss_family != PF_UNIX) {
		/* XXX HJH */
		acc->euid = -1;
		acc->egid = -1;
		return (ACHAT_RC_OK);
	}

	rc = getpeereid(acc->fd, &acc->euid, &acc->egid);
	if (rc == -1)
		return (ACHAT_RC_ERROR);

	return (ACHAT_RC_OK);
}

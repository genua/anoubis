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
#include "anoubis_chat.h"

/**
 * Calls <code>fcntl(2)</code> on a filedescriptor.
 *
 * @param fd The destination filedescriptor
 * @param flags Flags to be set on <code>fd</code>.
 * @return On success achat_rc::ACHAT_RC_OK is returned.
 */
static achat_rc
acc_fcntl(int fd, int flags)
{
	int cur_flags;

	if ((cur_flags = fcntl(fd, F_GETFL, 0)) == -1)
		return ACHAT_RC_ERROR;

	if (fcntl(fd, F_SETFL, cur_flags | flags) == -1)
		return ACHAT_RC_ERROR;
	else
		return ACHAT_RC_OK;
}

/**
 * Prepares the chat_channel.
 *
 * Depending on the @link achat_channel::tail tail @endlink and
 * @link achat_channel::addr address-information @endlink a (server-)socket is
 * created.
 *
 * @param acc The channel
 * @return On success achat_rc::ACHAT_RC_OK is returned. If the socket could
 *         not be created achat_rc::ACHAT_RC_ERROR is returned.
 */
achat_rc
acc_prepare(struct achat_channel *acc)
{
	int rc;

	ACC_CHKPARAM(acc != NULL);

	if (acc->sslmode == ACC_SSLMODE_NONE) {
		// SSL-mode not explicit set
		return (ACHAT_RC_ERROR);
	}

	/* You don't need to check for a valid acc->addr */
	/* If not set/supported, creation of socket will fail */

	switch (acc->tail) {
	case ACC_TAIL_SERVER:
		acc->fd = socket(acc->addr.ss_family, SOCK_STREAM, 0);
		if (acc->fd == -1)
			return (ACHAT_RC_ERROR);

		if (acc_fcntl(acc->fd, FD_CLOEXEC) != ACHAT_RC_OK)
			return (ACHAT_RC_ERROR);

		if (acc->addr.ss_family == AF_UNIX)
			unlink(((struct sockaddr_un*)&(acc->addr))->sun_path);

		rc = bind(acc->fd, (struct sockaddr*)&(acc->addr),
			acc->addrsize);
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

		if (acc_fcntl(acc->fd, FD_CLOEXEC) != ACHAT_RC_OK)
			return (ACHAT_RC_ERROR);

		if (acc->blocking == ACC_NON_BLOCKING) {
			if (acc_fcntl(acc->fd, O_NONBLOCK) != ACHAT_RC_OK)
				return (ACHAT_RC_ERROR);
		}

		break;
	default:
		/* Tail not set */
		return (ACHAT_RC_ERROR);
		break;
	}

	return (ACHAT_RC_OK);
}

/**
 * Opens the connection over the channel.
 *
 * The operation performed by the function depends on the
 * @link achat_channel::tail tail-mode @endlink:
 * - acc_tail::ACC_TAIL_SERVER A pending connection is accepted and a new
 *     client-channel with tail acc_tail::ACC_TAIL_CLIENT is created. The
 *     original server-channel is closed and replaced with the new
 *     client-channel. So, after a successful acc_open()-call, the
 *     <code>acc</code>-argument switches to a client-channel.
 * - acc_tail::ACC_TAIL_CLIENT The connection to the server is established.
 *
 * @param acc The channel
 * @return On success achat_rc::ACHAT_RC_OK is returned. If the connection
 *         could not be created, achat_rc::ACHAT_RC_ERROR is returned.
 */
achat_rc
acc_open(struct achat_channel *acc)
{
	int rc;

	ACC_CHKPARAM(acc != NULL);

	/* Will fail, if no socket is open */
	if (acc->fd == -1)
		return (ACHAT_RC_ERROR);

	if (acc->tail == ACC_TAIL_SERVER) {
		struct achat_channel	*nc = acc_opendup(acc);

		if (nc == NULL)
			return (ACHAT_RC_ERROR);

		/* Close the server-socket */
		acc_close(acc);

		memcpy(acc, nc, sizeof(struct achat_channel));

		/* Destroy temporary nc */
		nc->fd = -1; /* You still need the socket! Copied to acc */
		nc->sendbuffer = NULL;
		nc->recvbuffer = NULL;
		acc_destroy(nc);
	} else {
		/* retry if we were interrupted (e.g by a signal) */
		do {
			rc = connect(acc->fd,
			    (struct sockaddr *)&acc->addr, acc->addrsize);
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

	return (ACHAT_RC_OK);
}

/**
 * Creates and opens a duplicate of the server-channel.
 * A copy of the given channel is created. The new channel represents the
 * client-end of the communication. The tail-mode of the new channel is set to
 * achat_tail::ACC_TAIL_CLIENT and the server accepts a pending
 * client-connection.
 *
 * @param acc The server-channel
 * @return The client-channel. On error NULL is returned.
 */
struct achat_channel *
acc_opendup(struct achat_channel *acc)
{
	struct achat_channel	*nc;
	struct sockaddr_storage	remote;
	socklen_t		size;

	if (acc == NULL || acc->event)
		return (NULL);

	if (acc->tail != ACC_TAIL_SERVER)
		return (NULL); /* Operation only allowed on server-tail */

	nc = acc_create();
	if (nc == NULL)
		return (NULL);

	acc_settail(nc, ACC_TAIL_CLIENT);
	acc_setsslmode(nc, acc->sslmode);
	acc_setblockingmode(nc, acc->blocking);
	acc_setaddr(nc, &acc->addr, acc->addrsize);

	/* retry if we were interrupted (e.g by a signal) */
	size = (socklen_t) sizeof(remote);
	do {
		nc->fd = accept(acc->fd,
		    (struct sockaddr *)&remote, &size);
	} while ((nc->fd == -1) && (errno == EINTR || errno == EAGAIN));
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

	if (acc->blocking == ACC_NON_BLOCKING) {
		if (acc_fcntl(nc->fd, O_NONBLOCK) != ACHAT_RC_OK) {
			acc_clear(nc);
			acc_destroy(nc);
			return (NULL);
		}
	}

	return (nc);
}

/**
 * Closes the channel.
 *
 * @param acc The channel is close
 * @return On success achat_rc::ACHAT_RC_OK is returned.
 */
achat_rc
acc_close(struct achat_channel *acc)
{
	ACC_CHKPARAM(acc != NULL);

	acc_bufferclear(acc->sendbuffer);
	acc_bufferclear(acc->recvbuffer);
	acc->event = NULL;

	if (acc->fd == -1)
		return ACHAT_RC_OK;
	close(acc->fd);
	acc->fd = -1;

	if ((acc->addr.ss_family == AF_UNIX) && (acc->tail == ACC_TAIL_SERVER))
		unlink(((struct sockaddr_un*)&(acc->addr))->sun_path);

	return (ACHAT_RC_OK);
}

/**
 * Sets the effective user ID and group ID of the channel of the peer connected
 * to a UNIX domain socket.
 *
 * If the channel is not a UNIX domain socket, achat_channel::euid and
 * achat_channel::egid are set to -1.
 *
 * @param acc The channel
 * @return On success achat_rc::ACHAT_RC_OK is retured. If an error occured
 *         achat_rc::ACHAT_RC_ERROR is returned.
 */
achat_rc
acc_getpeerids(struct achat_channel *acc)
{
	int	rc;

	ACC_CHKPARAM(acc != NULL);
	/*
	 * Currently we can retrieve the user information only from
	 * unix domain sockets. -- CH
	 */
	if (acc->addr.ss_family != AF_UNIX) {
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

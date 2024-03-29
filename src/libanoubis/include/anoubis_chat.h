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

#ifndef _ANOUBISCHAT_H_
#define _ANOUBISCHAT_H_

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#define ACHAT_SERVER_PORT	4000
#define ACHAT_MAX_BACKLOG	5
#define ACHAT_MAX_MSGSIZE	81920		/* Be generous */

/**
 * Anoubis chat channel encryption mode.
 */
enum acc_sslmode {
	ACC_SSLMODE_NONE = 0,	/*!< Encryption-mode is not set. */
	ACC_SSLMODE_CLEAR,	/*!< Encryption is switched off. */
	ACC_SSLMODE_ENCIPHERED	/*!< Excryption is enabled. */
};

/**
 * Anoubis chat channel tail.
 */
enum acc_tail {
	ACC_TAIL_NONE = 0,	/*!< Tail-mode is not set. */
	ACC_TAIL_SERVER,	/*!< The channel acts as a server. */
	ACC_TAIL_CLIENT		/*!< The channel acts as a client. */
};

/**
 * Anoubis chat blocking mode.
 */
enum acc_blockingmode {
	ACC_BLOCKING = 0,	/*!< The channel runs in blocking-mode. */
	ACC_NON_BLOCKING	/*!< The channel runs in non-blocking mode. */
};

/**
 * Anoubis chat return codes.
 */
enum achat_rc {
	ACHAT_RC_OK = 0,	/*!< The operation was successful. */
	ACHAT_RC_ERROR,		/*!< The operation results into an error. */
	ACHAT_RC_NYI,		/*!< not yet implemented */
	ACHAT_RC_EOF,		/*!< connection end */
	ACHAT_RC_INVALPARAM,	/*!< invalid argument */
	ACHAT_RC_OOMEM,		/*!< out of memory */
	ACHAT_RC_PENDING,	/*!< Pending data to be send/receive */
	ACHAT_RC_NOSPACE	/*!< Not enough space to receive/send a
				     message */
};
typedef enum achat_rc achat_rc;

/* Forward declarations */
struct achat_buffer;
struct event;

/**
 * The anoubis chat channel.
 *
 * The channel contains all information you need to transfer data between
 * anoubisd and an anoubis-client.
 */
struct achat_channel {
	/**
	 * The encryption-mode.
	 * Specifies whether data are encrypted.
	 * @note Encryption is currently not implemented.
	 */
	enum acc_sslmode	sslmode;	/* CLEAR, ENCIPHERED */

	/**
	 * Channel-tail.
	 * Specifies whether the channel acts as a client or server.
	 */
	enum acc_tail		tail;

	/**
	 * Blocking-mode of the channel.
	 * Specifies whether the channel runs in (non-) blocking mode.
	 */
	enum acc_blockingmode	blocking;

	/**
	 * Address-information.
	 *
	 * Depends on the tail-mode:
	 * - acc_tail::ACC_TAIL_SERVER: The field contains bind-information.
	 * - acc_tail::ACC_TAIL_CLIENT: Where you want to go
	 */
	struct sockaddr_storage	addr;

	/**
	 * Size of achat_channel::addr.
	 */
	size_t			addrsize;

	/**
	 * The channel-socket.
	 */
	int			fd;

	/**
	 * Effective uid of users connect via a unix domain socket.
	 */
	uid_t			euid;

	/**
	 * Effective gid of users connect via a unix domain socket.
	 */
	gid_t			egid;

	/**
	 * Output-buffer.
	 * Data are written from the output-buffer into achat_channel::fd.
	 */
	struct achat_buffer	*sendbuffer;

	/**
	 * Input-buffer.
	 * Data are read from the achat_channel::fd into the input-buffer.
	 */
	struct achat_buffer	*recvbuffer;

	/**
	 * An event to be informed when you are able to flush the channel.
	 *
	 * You can setup an event here. When the filedescriptor of the channel
	 * is able to write, the event is triggered.
	 * <code>event_add(3)</code> is called, when acc_flush() cannot write
	 * all data. Now you are informed, when is is possible to call
	 * acc_flush() again.
	 */
	struct event		*event;
};

__BEGIN_DECLS

/* Subsystem Setup */
/*@null@*/ /*@out@*/
struct achat_channel *acc_create(void);
achat_rc /*@alt void@*/ acc_destroy(/*@only@*/struct achat_channel *);
achat_rc /*@alt void@*/ acc_clear(/*@out@*/struct achat_channel *);
achat_rc acc_settail(struct achat_channel *, enum acc_tail);
achat_rc acc_setsslmode(struct achat_channel *, enum acc_sslmode);
achat_rc acc_setblockingmode(struct achat_channel *, enum acc_blockingmode);
achat_rc acc_setaddr(struct achat_channel *, struct sockaddr_storage *, size_t);

/* Subsystem Connect */
achat_rc acc_prepare(struct achat_channel *);
achat_rc acc_open(struct achat_channel *);
/*@null@*/
struct achat_channel *acc_opendup(struct achat_channel *);
achat_rc /*@alt void@*/ acc_close(struct achat_channel *);
achat_rc acc_getpeerids(struct achat_channel *);

/* Subsystem Transmission */
achat_rc acc_sendmsg(struct achat_channel *, const char *, size_t);
achat_rc acc_receivemsg(struct achat_channel *, char *, size_t *);
achat_rc acc_flush(struct achat_channel *);

__END_DECLS

#endif	/* _ANOUBISCHAT_H_ */

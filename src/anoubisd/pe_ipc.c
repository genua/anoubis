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

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#ifdef OPENBSD
#include <sys/limits.h>
#endif

#include <arpa/inet.h>
#include <netinet/in.h>

#include <apn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef LINUX
#include <bsdcompat.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/anoubis_alf.h>
#include <dev/anoubis.h>
#endif

#include <sys/queue.h>

#include "anoubisd.h"
#include "pe.h"

void	pe_ipc_connect(struct ac_ipc_message *);
void	pe_ipc_destroy(struct ac_ipc_message *);

/**
 * Change context on connect(2) for AF_UNIX:
 * - If pe_context_decide forbids changing the context, we don't.
 * - Otherwise we borrow the context from the peer. The current
 *   context is saved and will be restored when the connection closes.
 * Connections are identified by a separate cookie that is assigned to
 * the connection. It uses the same type as a task cookie but is otherwise
 * not related to task cookies at all.
 *
 * @param msg The kernel event for the connect. The processes are extracted
 *     from the event by looking at the task cookies.
 */
void
pe_ipc_connect(struct ac_ipc_message *msg)
{
	struct pe_proc		*proc, *procp;

	if (msg == NULL)
		return;

	if ((proc = pe_proc_get(msg->source)) == NULL)
		return;
	if ((procp = pe_proc_get(msg->dest)) == NULL) {
		pe_proc_put(proc);
		return;
	}
	pe_context_borrow(proc, procp, msg->conn_cookie);
	pe_proc_put(proc);
	pe_proc_put(procp);
}

/**
 * Handle a disconnect event for a unix domain socket. This function
 * restores the saved context if the connect was previously used to
 * borrow a context from the peer.
 *
 * @param msg The kernel event for the disconnect. The task cookies in
 *     the event are used to find the processes, the connection cookie
 *     is used to destinguish different connections.
 */
void
pe_ipc_destroy(struct ac_ipc_message *msg)
{
	struct pe_proc		*proc;

	if (msg == NULL)
		return;

	if ((proc = pe_proc_get(msg->source)) == NULL)
		return;
	pe_context_restore(proc, msg->conn_cookie);
	pe_proc_put(proc);
}

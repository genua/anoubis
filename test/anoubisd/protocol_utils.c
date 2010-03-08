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

#include "config.h"
#include "version.h"

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <err.h>
#include <unistd.h>
#include <syslog.h>

#ifdef LINUX
#include <bsdcompat.h>
#endif
#ifdef OPENBSD
#include <sha2.h>
#endif

#include <anoubis_msg.h>
#include <anoubis_protocol.h>
#include <anoubis_errno.h>
#include <anoubis_client.h>
#include <anoubis_msg.h>
#include <anoubis_sig.h>
#include <anoubis_transaction.h>
#include <anoubis_dump.h>

#include <anoubis_chat.h>
#include <protocol_utils.h>

void
create_channel(struct achat_channel **chanp, struct anoubis_client **clientp,
    anoubis_client_auth_callback_t callback)
{
	int				 error = 0;
	achat_rc			 rc;
	struct sockaddr_un		 ss;
	struct achat_channel		*channel = NULL;
	struct anoubis_client		*client;
	int				 auth_type = ANOUBIS_AUTH_TRANSPORT;

	channel = acc_create();
	assert(channel);
	rc = acc_settail(channel, ACC_TAIL_CLIENT);
	assert(rc == ACHAT_RC_OK);
	rc = acc_setsslmode(channel, ACC_SSLMODE_CLEAR);
	assert(rc == ACHAT_RC_OK);
	bzero(&ss, sizeof(ss));
	ss.sun_family = AF_UNIX;
	strlcpy((&ss)->sun_path, PACKAGE_SOCKET, sizeof((&ss)->sun_path));
	rc = acc_setaddr(channel, (struct sockaddr_storage *)&ss,
	     sizeof(struct sockaddr_un));
	assert(rc == ACHAT_RC_OK);
	rc = acc_prepare(channel);
	assert(rc == ACHAT_RC_OK);
	rc = acc_open(channel);
	assert(rc == ACHAT_RC_OK);
	if (callback)
		auth_type = ANOUBIS_AUTH_TRANSPORTANDKEY;
	client = anoubis_client_create(channel, auth_type, callback);
	error = anoubis_client_connect(client, ANOUBIS_PROTO_BOTH);
	assert(error == 0);
	if (chanp)
		(*chanp) = channel;
	if (clientp)
		(*clientp) = client;
}

void
destroy_channel(struct achat_channel *channel, struct anoubis_client *client)
{
	if (client)
		anoubis_client_destroy(client);
	if (channel)
		acc_destroy(channel);
}

struct anoubis_sig *
create_sig(void)
{
	static const char	 pubkey[] = "./.xanoubis/default.crt";
	static const char	 privkey[] = "./.xanoubis/default.key";
	struct anoubis_sig	*ret;

	if (anoubis_sig_create(&ret, privkey, pubkey, pass_cb) < 0)
		return NULL;
	return ret;
}

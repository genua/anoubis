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
#include <protocol_utils.h>

#include <anoubis_chat.h>

static struct achat_channel	*channel = NULL;
struct anoubis_client		*client = NULL;

static void
create_dangling_policy_request(void)
{
	struct anoubis_msg	*m;
	Policy_SetByUid		 set;

	set_value(set.ptype, ANOUBIS_PTYPE_SETBYUID);
	set_value(set.siglen, 0);
	set_value(set.uid, geteuid());
	set_value(set.prio, 1 /* USER */);
	m = anoubis_msg_new(sizeof(Anoubis_PolicyRequestMessage) + sizeof(set));
	assert(m);
	set_value(m->u.policyrequest->type, ANOUBIS_P_REQUEST);
	set_value(m->u.policyrequest->flags, POLICY_FLAG_START);
	memcpy(m->u.policyrequest->payload, &set, sizeof(set));
	anoubis_msg_send(channel, m);
	anoubis_msg_free(m);
}

int main()
{
	int	i;

	for (i=0; i<10000; ++i) {
		if (i % 1000 == 0)
			printf("Connections: %d\n", i);
		create_channel(&channel, &client, NULL);
		create_dangling_policy_request();
		destroy_channel(channel, client);
	}
	return 0;
}

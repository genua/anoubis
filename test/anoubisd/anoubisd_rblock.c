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

static struct achat_channel	*ch[20] = { NULL, };
struct anoubis_client		*cl[20] = { NULL, };

/*
 * Each iteration creates a new connections and then sends an imcomplete
 * message to the peer. The peer should be able to handle this without
 * blocking. We have to close the connections after a few iterations
 * because the daemon has a connections limit.
 */

int main()
{
	int	i;
	int	buf[10];

	for (i=0; i<10000; ++i) {
		int	ret;
		if (i % 20 == 0) {
			int j;
			printf("Iterations: %d\n", i);
			for (j=0; j<20; ++j)
				destroy_channel(ch[j], cl[j]);
		}
		create_channel(&ch[i%20], &cl[i%20], NULL);
		buf[0] = htonl(300);
		ret = write(ch[i%20]->fd, buf, sizeof(buf));
		assert(ret == sizeof(buf));
	}
	return 0;
}

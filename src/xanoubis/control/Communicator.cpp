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

#include "Communicator.h"
#include "CommunicatorException.h"

Communicator::Communicator()
{
	acc_clear(this);
}

Communicator::~Communicator()
{
	acc_close(this);
}

void
Communicator::setTail(enum acc_tail tail)
{
	achat_rc rc;

	rc = acc_settail(this, tail);
	if (rc != ACHAT_RC_OK)
		throw new CommunicatorException(rc);
}

void
Communicator::setSslMode(enum acc_sslmode sslmode)
{
	achat_rc rc;

	rc = acc_setsslmode(this, sslmode);
	if (rc != ACHAT_RC_OK)
		throw new CommunicatorException(rc);
}

void
Communicator::setAddr(struct sockaddr_storage *addr)
{
	achat_rc rc;

	rc = acc_setaddr(this, addr);
	if (rc != ACHAT_RC_OK)
		throw new CommunicatorException(rc);
}

void
Communicator::prepare(void)
{
	achat_rc rc;

	rc = acc_prepare(this);
	if (rc != ACHAT_RC_OK)
		throw new CommunicatorException(rc);
}

void
Communicator::open(void)
{
	achat_rc rc;

	rc = acc_open(this);
	if (rc != ACHAT_RC_OK)
		throw new CommunicatorException(rc);
}

void
Communicator::close(void)
{
	achat_rc rc;

	rc = acc_close(this);
	if (rc != ACHAT_RC_OK)
		throw new CommunicatorException(rc);
}

short
Communicator::getPort(void)
{
	struct sockaddr_storage  ss;
	struct sockaddr_in      *ss_sin = (struct sockaddr_in *)&ss;
	socklen_t                sslen;

	sslen = sizeof(ss);
	bzero(&ss, sslen);
	if (getsockname(sockfd, (struct sockaddr *)&ss, &sslen) == -1)
		throw new CommunicatorException(ACHAT_RC_ERROR);
	if (ss_sin->sin_port == 0)
		throw new CommunicatorException(ACHAT_RC_ERROR);

	return (ss_sin->sin_port);
}

void
Communicator::sendMessage(const char *message, size_t size)
{
	achat_rc rc;

	rc = acc_sendmsg(this, message, size);
	if (rc != ACHAT_RC_OK)
		throw new CommunicatorException(rc);
}

void
Communicator::receiveMessage(char *messageStore, size_t size)
{
	achat_rc rc;

	rc = acc_receivemsg(this, messageStore, size);
	if (rc != ACHAT_RC_OK)
		throw new CommunicatorException(rc);
}

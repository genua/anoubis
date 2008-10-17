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

#include <config.h>

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include <client/anoubis_client.h>
#include <client/anoubis_transaction.h>
#include <anoubis_protocol.h>

#include "ComCsumGetTask.h"
#include "ComHandler.h"
#include "TaskEvent.h"

ComCsumGetTask::ComCsumGetTask(void)
{
	setFile(wxT(""));
}

ComCsumGetTask::ComCsumGetTask(const wxString &file)
{
	setFile(file);
}

wxString
ComCsumGetTask::getFile(void)
{
	return (this->file_);
}

void
ComCsumGetTask::setFile(const wxString &file)
{
	this->file_ = file;
}

wxEventType
ComCsumGetTask::getEventType(void) const
{
	return (anTASKEVT_CSUM_GET);
}

void
ComCsumGetTask::exec(void)
{
	struct anoubis_transaction	*ta;
	struct anoubis_msg		*reqmsg;
	char				path[file_.Len() + 1];

	resetComTaskResult();

	strlcpy(path, this->file_.fn_str(), sizeof(path));

	/* Create request */
	ta = anoubis_client_csumrequest_start(getComHandler()->getClient(),
	    ANOUBIS_CHECKSUM_OP_GET, (char*)path, NULL, 0);
	if(!ta) {
		setComTaskResult(RESULT_COM_ERROR);
		return;
	}

	/* Wait for completition */
	while (!(ta->flags & ANOUBIS_T_DONE)) {
		if (!getComHandler()->waitForMessage()) {
			setComTaskResult(RESULT_COM_ERROR);
			return;
		}
	}

	if (ta->result) {
		setComTaskResult(RESULT_REMOTE_ERROR);
		anoubis_transaction_destroy(ta);
		return;
	}

	reqmsg = ta->msg;
	ta->msg = NULL;
	anoubis_transaction_destroy(ta);

	if(!reqmsg || !VERIFY_LENGTH(reqmsg,
	    sizeof(Anoubis_PolicyReplyMessage)) ||
	    get_value(reqmsg->u.policyreply->error) != 0) {
		setComTaskResult(RESULT_REMOTE_ERROR);
		return;
	}

	for (unsigned int i = 0; i < ANOUBIS_CS_LEN; ++i)
		this->cs_[i] = reqmsg->u.ackpayload->payload[i];

	setComTaskResult(RESULT_SUCCESS);
}

size_t
ComCsumGetTask::getCsum(u_int8_t *csum, size_t size) const
{
	/* How many bytes to write? */
	size_t n = (size < ANOUBIS_CS_LEN) ? size : ANOUBIS_CS_LEN;

	memcpy(csum, this->cs_, n);

	return (n);
}

wxString
ComCsumGetTask::getCsumStr(void) const
{
	wxString str;

	for (int i = 0; i < ANOUBIS_CS_LEN; i++)
		str += wxString::Format(wxT("%.2x"), this->cs_[i]);

	return (str);
}

void
ComCsumGetTask::resetComTaskResult(void)
{
	ComTask::resetComTaskResult();

	for (int i = 0; i < ANOUBIS_CS_LEN; i++)
		this->cs_[i] = 0;
}

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

#include <anoubis_client.h>
#include <anoubis_transaction.h>

#include "ComCsumDelTask.h"
#include "TaskEvent.h"

ComCsumDelTask::ComCsumDelTask(void)
{
}

ComCsumDelTask::ComCsumDelTask(const wxString &file)
{
	setPath(file);
}

wxEventType
ComCsumDelTask::getEventType(void) const
{
	return (anTASKEVT_CSUM_DEL);
}

void
ComCsumDelTask::exec(void)
{
	int				req_op;
	char				path[PATH_MAX];

	ta_ = NULL;

	if (haveKeyId())
		req_op = ANOUBIS_CHECKSUM_OP_DELSIG;
	else
		req_op = ANOUBIS_CHECKSUM_OP_DEL;

	/*
	 * receive path to be send to anoubisd
	 * true := continue, if the file does not exist, you might want to
	 *         remove an orphaned checksum
	 */
	if (!resolvePath(path, true)) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(errno);
		return;
	}

	/* Create request */
	ta_ = anoubis_client_csumrequest_start(getClient(), req_op, (char*)path,
	    getKeyId(), 0, getKeyIdLen(), 0, ANOUBIS_CSUM_NONE);
	if(!ta_) {
		setComTaskResult(RESULT_COM_ERROR);
	}
}

bool
ComCsumDelTask::done(void)
{
	if (ta_ == NULL)
		return (true);
	if ((ta_->flags & ANOUBIS_T_DONE) == 0)
		return (false);
	if (ta_->result) {
		setComTaskResult(RESULT_REMOTE_ERROR);
		setResultDetails(ta_->result);
	} else {
		setComTaskResult(RESULT_SUCCESS);
	}
	anoubis_transaction_destroy(ta_);
	ta_ = NULL;
	return (true);
}

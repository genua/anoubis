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

#include <client/anoubis_client.h>
#include <client/anoubis_transaction.h>

#include "ComVersionTask.h"
#include "TaskEvent.h"

ComVersionTask::ComVersionTask(void)
{
}

int
ComVersionTask::getApnVersion(void) const
{
	return (this->apnVersion_);
}

wxEventType
ComVersionTask::getEventType(void) const
{
	return (anTASKEVT_VERSION);
}

void
ComVersionTask::exec(void)
{
	resetComTaskResult();
	ta_ = anoubis_client_version_start(getClient());

	if(!ta_)
		setComTaskResult(RESULT_COM_ERROR);
}

bool
ComVersionTask::done(void)
{
	if (ta_ == NULL)
		return (true);
	if (ta_->flags & ANOUBIS_T_DONE) {
		if (ta_->result) {
			setComTaskResult(RESULT_REMOTE_ERROR);
			setResultDetails(ta_->result);
		} else if (processVersionMessage(ta_->msg))
			setComTaskResult(RESULT_SUCCESS);
		else
			setComTaskResult(RESULT_REMOTE_ERROR);

		anoubis_transaction_destroy(ta_);
		ta_ = NULL;

		return (true);
	} else
		return (false);
}

void
ComVersionTask::resetComTaskResult(void)
{
	ComTask::resetComTaskResult();
	this->apnVersion_ = 0;
}

bool
ComVersionTask::processVersionMessage(struct anoubis_msg *m)
{
	if (!m || !VERIFY_LENGTH(m, sizeof(Anoubis_VersionMessage))) {
		/* Invalid message */
		setResultDetails(EINVAL);
		return (false);
	}

	int error;
	if ((error = get_value(m->u.version->error)) != 0) {
		/* Remote error */
		setResultDetails(error);
		return (false);
	}

	this->apnVersion_ = get_value(m->u.version->apn);

	return (true);
}

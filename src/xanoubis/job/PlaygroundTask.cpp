/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include <anoubis_client.h>
#include <anoubis_transaction.h>

#include "PlaygroundTask.h"

PlaygroundTask::PlaygroundTask(uint32_t listtype, uint64_t pgid)
{
	this->listtype_ = listtype;
	this->result_ = 0;
	this->pgid_ = pgid;
	this->ta_ = 0;
}

PlaygroundTask::~PlaygroundTask(void)
{
	reset();
}

void
PlaygroundTask::exec(void)
{
	reset();

	ta_ = anoubis_client_pglist_start(
	    getClient(), this->listtype_, this->pgid_);

	if (ta_ == NULL) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(ENOMEM);
		return;
	}
}

bool
PlaygroundTask::done(void)
{
	/* An error occured. We are done. */
	if (getComTaskResult() != RESULT_INIT)
		return true;

	if (ta_ && (ta_->flags & ANOUBIS_T_DONE) == 0) {
		/* Not finished yet */
		return false;
	} else if (ta_) {
		if (ta_->result) {
			/* Finished with an error */
			setComTaskResult(RESULT_REMOTE_ERROR);
			setResultDetails(ta_->result);
			anoubis_transaction_destroy(ta_);
			ta_ = NULL;

			return true;
		}

		if (ta_->msg == NULL ||
		    !VERIFY_LENGTH(ta_->msg, sizeof(Anoubis_PgReplyMessage)) ||
		    get_value(ta_->msg->u.pgreply->error) != 0) {
			/* Message verification failed */
			setComTaskResult(RESULT_REMOTE_ERROR);
			setResultDetails(get_value(ta_->msg->u.pgreply->error));
			anoubis_transaction_destroy(ta_);
			ta_ = NULL;

			return true;
		}

		/* Success */
		result_ = ta_->msg;
		ta_->msg = 0;

		anoubis_transaction_destroy(ta_);
		ta_ = NULL;
		setComTaskResult(RESULT_SUCCESS);
	}

	return (false);
}

void
PlaygroundTask::reset(void)
{
	setComTaskResult(RESULT_INIT);
	setResultDetails(0);

	if (result_) {
		anoubis_msg_free(result_);
		result_ = 0;
	}

	if (ta_) {
		anoubis_transaction_destroy(ta_);
		ta_ = 0;
	}
}

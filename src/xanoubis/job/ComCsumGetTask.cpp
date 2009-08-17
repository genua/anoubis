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
#include "TaskEvent.h"

ComCsumGetTask::ComCsumGetTask(void)
{
	this->cs_ = 0;
	this->cs_len_ = 0;
}

ComCsumGetTask::ComCsumGetTask(const wxString &file)
{
	this->cs_ = 0;
	this->cs_len_ = 0;

	setPath(file);
}

ComCsumGetTask::~ComCsumGetTask(void)
{
	resetCsum();
}

wxEventType
ComCsumGetTask::getEventType(void) const
{
	return (anTASKEVT_CSUM_GET);
}

void
ComCsumGetTask::exec(void)
{
	int				req_op;
	char				path[PATH_MAX];

	resetComTaskResult();
	ta_ = NULL;

	if (haveKeyId())
		req_op = ANOUBIS_CHECKSUM_OP_GETSIG2;
	else
		req_op = ANOUBIS_CHECKSUM_OP_GET2;

	/*
	 * Receive path to be send to anoubisd
	 * true := continue if the path does not exist, you might want to
	 *         fetch an orphaned checksum.
	 */
	if (!resolvePath(path, true)) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(errno);
		return;
	}

	/* Create request */
	ta_ = anoubis_client_csumrequest_start(getClient(), req_op,
	    (char*)path, getKeyId(), 0, getKeyIdLen(), 0, ANOUBIS_CSUM_NONE);
	if(!ta_) {
		setComTaskResult(RESULT_COM_ERROR);
	}
}

bool
ComCsumGetTask::done(void)
{
	struct anoubis_msg		*reqmsg;
	int				 len;
	const void			*sigdata = NULL;

	if (ta_ == NULL)
		return (true);
	if ((ta_->flags & ANOUBIS_T_DONE) == 0)
		return (false);

	if (ta_->result) {
		/* Any other error-code is interpreted as an real error */
		setComTaskResult(RESULT_REMOTE_ERROR);
		setResultDetails(ta_->result);
		anoubis_transaction_destroy(ta_);
		ta_ = NULL;
		return (true);
	}

	reqmsg = ta_->msg;
	ta_->msg = NULL;
	anoubis_transaction_destroy(ta_);
	ta_ = NULL;

	if (haveKeyId()) {
		len = anoubis_extract_sig_type(reqmsg, ANOUBIS_SIG_TYPE_SIG,
		    &sigdata);
		if (len > 0 && len != ANOUBIS_CS_LEN)
			goto err;
	} else {
		/*
		 * XXX CEH: Might want to look at ANOUBIS_SIG_TYPE_UPGRADE
		 * XXX CEH: as well. It should just be there.
		 */
		len = anoubis_extract_sig_type(reqmsg, ANOUBIS_SIG_TYPE_CS,
		    &sigdata);
	}
	if (len < 0)
		goto err;
	this->cs_len_ = len;
	if (len) {
		this->cs_ = (u_int8_t *)malloc(this->cs_len_);
		memcpy(cs_, sigdata, len);
	}
	setComTaskResult(RESULT_SUCCESS);
	anoubis_msg_free(reqmsg);
	return (true);
err:
	anoubis_msg_free(reqmsg);
	return (true);
}

size_t
ComCsumGetTask::getCsumLen(void) const
{
	return (this->cs_len_);
}

size_t
ComCsumGetTask::getCsum(u_int8_t *csum, size_t size) const
{
	if (getComTaskResult() == RESULT_SUCCESS && getResultDetails() == 0) {
		if (size >= cs_len_) {
			memcpy(csum, this->cs_, cs_len_);
			return (cs_len_);
		}
	}

	/*
	 * No checksumk received from anoubisd or destination-buffer not large
	 * enough.
	 */
	return (0);
}

wxString
ComCsumGetTask::getCsumStr(void) const
{
	if (getComTaskResult() == RESULT_SUCCESS && getResultDetails() == 0) {
		wxString str;

		for (unsigned int i = 0; i < cs_len_; i++)
			str += wxString::Format(wxT("%.2x"), this->cs_[i]);

		return (str);
	} else
		return (wxEmptyString);
}

void
ComCsumGetTask::resetComTaskResult(void)
{
	ComTask::resetComTaskResult();

	resetCsum();
}

void
ComCsumGetTask::resetCsum(void)
{
	if (this->cs_ != 0) {
		free(this->cs_);
		this->cs_ = 0;
		this->cs_len_ = 0;

	}
}

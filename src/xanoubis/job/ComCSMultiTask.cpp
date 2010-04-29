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

#include <anoubis_protocol.h>
#include "ComCSMultiTask.h"

ComCSMultiTask::ComCSMultiTask(void)
{
	csreq_ = NULL;
	sigreq_ = NULL;
	keyid_ = NULL;
	kidlen_ = 0;
	ta_ = NULL;
	paths_.clear();
}

ComCSMultiTask::~ComCSMultiTask(void)
{
	if (keyid_)
		free(keyid_);
	if (csreq_)
		anoubis_csmulti_destroy(csreq_);
	if (sigreq_)
		anoubis_csmulti_destroy(sigreq_);
	if (ta_)
		anoubis_transaction_destroy(ta_);
}

int
ComCSMultiTask::addPathToRequest(struct anoubis_csmulti_request *req,
    const char *path)
{
	int		ret;

	if (req == NULL)
		return 0;
	ret = anoubis_csmulti_add(req, path, NULL, 0);
	if (ret == 0)
		return 0;
	return anoubis_csmulti_add_error(req, path, -ret);
}

int
ComCSMultiTask::addPathToRequest(struct anoubis_csmulti_request *req,
    const char *path, uint8_t *csdata, unsigned int cslen)
{
	int	ret;

	if (req == NULL)
		return 0;
	if (!csdata || !cslen)
		return -ENOMEM;
	ret = anoubis_csmulti_add(req, path, csdata, cslen);
	if (ret == 0)
		return 0;
	ret = anoubis_csmulti_add_error(req, path, -ret);
	return ret;
}

int
ComCSMultiTask::getChecksumError(unsigned int idx) const
{
	struct anoubis_csmulti_record	*record;

	if (!csreq_)
		return 0;
	record = anoubis_csmulti_find(csreq_, idx);
	if (!record)
		return 0;
	return record->error;
}

int
ComCSMultiTask::getSignatureError(unsigned int idx) const
{
	struct anoubis_csmulti_record	*record;

	if (!sigreq_)
		return 0;
	record = anoubis_csmulti_find(sigreq_, idx);
	if (!record)
		return 0;
	return record->error;
}

bool
ComCSMultiTask::setKeyId(const u_int8_t *keyid, unsigned int kidlen)
{
	if (keyid_)
		free(keyid_);
	keyid_ = (uint8_t *)malloc(kidlen);
	if (keyid_ == NULL)
		return false;
	kidlen_ = kidlen;
	memcpy(keyid_, keyid, kidlen);
	return true;
}

bool
ComCSMultiTask::haveKeyId(void) const
{
	return keyid_ && kidlen_;
}

unsigned int
ComCSMultiTask::getPathCount(void) const
{
	return paths_.size();
}

void
ComCSMultiTask::addPath(const wxString path)
{
	paths_.push_back(path);
}

wxString
ComCSMultiTask::getPath(unsigned int idx) const
{
	if (idx < paths_.size())
		return paths_[idx];
	return wxEmptyString;
}

void
ComCSMultiTask::createRequests(int op)
{
	int	sigop;

	switch(op) {
	case ANOUBIS_CHECKSUM_OP_ADDSUM:
		sigop = ANOUBIS_CHECKSUM_OP_ADDSIG;
		break;
	case ANOUBIS_CHECKSUM_OP_DEL:
		sigop = ANOUBIS_CHECKSUM_OP_DELSIG;
		break;
	case ANOUBIS_CHECKSUM_OP_GET2:
		sigop = ANOUBIS_CHECKSUM_OP_GETSIG2;
		break;
	default:
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(EINVAL);
		return;
	}
	if (csreq_ == NULL) {
		csreq_ = anoubis_csmulti_create(op, geteuid(), NULL, 0);
		if (csreq_ == NULL)
			goto nomem;
	}
	if (sigreq_ == NULL && keyid_ && kidlen_) {
		sigreq_ = anoubis_csmulti_create(sigop, 0, keyid_, kidlen_);
		if (sigreq_ == NULL)
			goto nomem;
	}
	return;

nomem:
	setComTaskResult(RESULT_LOCAL_ERROR);
	setResultDetails(ENOMEM);
}

bool
ComCSMultiTask::done(void)
{
	/* An error occured. We are done. */
	if (getComTaskResult() != RESULT_INIT)
		return true;
	if (ta_ && (ta_->flags & ANOUBIS_T_DONE) == 0) {
		return false;
	} else if (ta_) {
		if (ta_->result) {
			setComTaskResult(RESULT_REMOTE_ERROR);
			setResultDetails(ta_->result);
			anoubis_transaction_destroy(ta_);
			ta_ = NULL;
			return true;
		}
		anoubis_transaction_destroy(ta_);
		ta_ = NULL;
	}
	if (csreq_ && csreq_->openreqs) {
		ta_ = anoubis_client_csmulti_start(getClient(), csreq_);
		if (ta_ == NULL)
			goto nomem;
		return false;
	}
	if (sigreq_ && sigreq_->openreqs) {
		ta_ = anoubis_client_csmulti_start(getClient(), sigreq_);
		if (ta_ == NULL)
			goto nomem;
		return false;
	}
	setComTaskResult(RESULT_SUCCESS);
	return true;

nomem:
	setComTaskResult(RESULT_LOCAL_ERROR);
	setResultDetails(ENOMEM);
	return true;
}

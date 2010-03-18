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
#include <anoubis_protocol.h>

#include "ComCsumGetTask.h"
#include "TaskEvent.h"


ComCsumGetTask::ComCsumGetTask(void)
{
	csreq_ = NULL;
	sigreq_ = NULL;
	keyid_ = NULL;
	kidlen_ = 0;
	ta_ = 0;
	paths_.clear();
}

ComCsumGetTask::~ComCsumGetTask(void)
{
	if (ta_)
		anoubis_transaction_destroy(ta_);
	if (keyid_)
		free(keyid_);
	if (csreq_)
		anoubis_csmulti_destroy(csreq_);
	if (sigreq_)
		anoubis_csmulti_destroy(sigreq_);
}

void
ComCsumGetTask::addPath(const wxString &path)
{
	paths_.push_back(path);
}

wxString
ComCsumGetTask::getPath(unsigned int idx) const
{
	if (idx < paths_.size())
		return paths_[idx];
	return wxEmptyString;
}

size_t
ComCsumGetTask::getPathCount(void) const
{
	return paths_.size();
}

wxEventType
ComCsumGetTask::getEventType(void) const
{
	return anTASKEVT_CSUM_GET;
}

void
ComCsumGetTask::createRequests(void)
{
	if (csreq_ == NULL) {
		csreq_ = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_GET2,
		    geteuid(), NULL, 0);
		if (csreq_ == NULL)
			goto nomem;
	}
	if (sigreq_ == NULL && keyid_ && kidlen_) {
		sigreq_ = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_GETSIG2,
		    0, keyid_, kidlen_);
		if (sigreq_ == NULL)
			goto nomem;
	}
	return;

nomem:
	setComTaskResult(RESULT_LOCAL_ERROR);
	setResultDetails(ENOMEM);
}

int
ComCsumGetTask::addPathToRequest(struct anoubis_csmulti_request *req,
    const char *path)
{
	int	ret;

	if (req == NULL)
		return 0;
	ret = anoubis_csmulti_add(req, path, NULL, 0);
	if (ret == 0)
		return 0;
	return anoubis_csmulti_add_error(req, path, -ret);
}

void
ComCsumGetTask::exec(void)
{
	int		ret;

	createRequests();
	if (getComTaskResult() != RESULT_INIT)
		return;
	for (unsigned int idx = 0; idx < paths_.size(); ++idx) {
		ret = addPathToRequest(csreq_, paths_[idx].fn_str());
		if (ret < 0)
			goto fatal;
		if (sigreq_) {
			ret = addPathToRequest(sigreq_, paths_[idx].fn_str());
			if (ret < 0)
				goto fatal;
		}
	}
	ta_ = NULL;
	done();
	return;

fatal:
	setComTaskResult(RESULT_LOCAL_ERROR);
	setResultDetails(-ret);
}

bool
ComCsumGetTask::done(void)
{
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

int
ComCsumGetTask::getChecksumError(unsigned int idx) const
{
	struct anoubis_csmulti_record   *record;

	if (!csreq_)
		return 0;
	record = anoubis_csmulti_find(csreq_, idx);
	if (!record)
		return 0;
	return record->error;
}

int
ComCsumGetTask::getSignatureError(unsigned int idx) const
{
	struct anoubis_csmulti_record   *record;

	if (!sigreq_)
		return 0;
	record = anoubis_csmulti_find(sigreq_, idx);
	if (!record)
		return 0;
	return record->error;
}

struct anoubis_csentry *
ComCsumGetTask::get_checksum_entry(unsigned int idx, int type) const
{
	struct anoubis_csmulti_record	*record;
	struct anoubis_csmulti_request	*request = NULL;
	struct anoubis_csentry		*entry = NULL;

	switch (type) {
	case ANOUBIS_SIG_TYPE_CS:
		request = csreq_;
		break;
	case ANOUBIS_SIG_TYPE_SIG:
	case ANOUBIS_SIG_TYPE_UPGRADECS:
		request = sigreq_;
		break;
	}
	if (!request)
		return NULL;
	record = anoubis_csmulti_find(request, idx);
	if (!record)
		return NULL;
	switch (type) {
	case ANOUBIS_SIG_TYPE_CS:
		entry = record->u.get.csum;
		break;
	case ANOUBIS_SIG_TYPE_SIG:
		entry = record->u.get.sig;
		break;
	case ANOUBIS_SIG_TYPE_UPGRADECS:
		entry = record->u.get.upgrade;
		break;
	}
	return entry;
}

size_t
ComCsumGetTask::getChecksumLen(unsigned int idx, int type) const
{
	anoubis_csentry	*entry = get_checksum_entry(idx, type);

	if (!entry)
		return 0;
	return get_value(entry->cslen);
}

bool
ComCsumGetTask::getChecksumData(unsigned int idx, int type,
    const u_int8_t *&buffer, size_t &buflen) const
{
	anoubis_csentry	*entry = get_checksum_entry(idx, type);
	size_t			 len;

	if (!entry) {
		buffer = NULL;
		buflen = 0;
		return false;
	}
	len = get_value(entry->cslen);
	if (!len) {
		buffer = NULL;
		buflen = 0;
		return false;
	}
	buflen = len;
	buffer = (const u_int8_t *)entry->csdata;
	return true;
}

bool
ComCsumGetTask::setKeyId(const u_int8_t *keyid, unsigned int kidlen)
{
	if (keyid_)
		free(keyid_);
	keyid_ = (u_int8_t *)malloc(kidlen);
	if (keyid_ == NULL)
		return false;
	kidlen_ = kidlen;
	memcpy(keyid_, keyid, kidlen);
	return true;
}

bool
ComCsumGetTask::haveKeyId(void) const
{
	return keyid_ && kidlen_;
}

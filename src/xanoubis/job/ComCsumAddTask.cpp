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

#include <sys/types.h>

#ifdef LINUX
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/anoubis.h>
#endif

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include <anoubis_client.h>
#include <anoubis_transaction.h>
#include <anoubis_csum.h>
#include <anoubis_protocol.h>
#include <anoubis_sig.h>

#include "ComCsumAddTask.h"
#include "TaskEvent.h"

ComCsumAddTask::ComCsumAddTask(void)
{
	type_ = Task::TYPE_CSUMCALC;
	privKey_ = 0;
	resetComTaskResult();
	csreq_ = NULL;
	sigreq_ = NULL;
	keyId_ = NULL;
	kidLen_ = 0;
	ta_ = NULL;
}

ComCsumAddTask::~ComCsumAddTask(void)
{
	if (csreq_)
		anoubis_csmulti_destroy(csreq_);
	if (sigreq_)
		anoubis_csmulti_destroy(sigreq_);
	if (keyId_)
		free(keyId_);
	if (ta_)
		anoubis_transaction_destroy(ta_);
}

void
ComCsumAddTask::addPath(wxString path)
{
	paths_.push_back(path);
}

void
ComCsumAddTask::setPrivateKey(struct anoubis_sig *privKey)
{
	this->privKey_ = privKey;
}

wxEventType
ComCsumAddTask::getEventType(void) const
{
	return anTASKEVT_CSUM_ADD;
}

void
ComCsumAddTask::createRequests(void)
{
	if (csreq_ == NULL) {
		csreq_ = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_ADDSUM,
		    geteuid(), NULL, 0);
		if (csreq_ == NULL)
			goto nomem;
	}
	if (sigreq_ == NULL && keyId_ && kidLen_) {
		sigreq_ = anoubis_csmulti_create(ANOUBIS_CHECKSUM_OP_ADDSIG,
		    0, keyId_, kidLen_);
		if (sigreq_ == NULL)
			goto nomem;
	}
	return;

nomem:
	setComTaskResult(RESULT_LOCAL_ERROR);
	setResultDetails(ENOMEM);
}

int
ComCsumAddTask::addPathToRequest(struct anoubis_csmulti_request *req,
    const char *path, u_int8_t *csdata, unsigned int cslen)
{
	int	ret;

	if (req == NULL)
		return 0;
	if (csdata) {
		ret = anoubis_csmulti_add(req, path, csdata, cslen);
		if (ret == 0)
			return 0;
	} else {
		ret = -ENOMEM;
	}
	ret = anoubis_csmulti_add_error(req, path, -ret);
	return ret;
}

int
ComCsumAddTask::addErrorPath(const char *path, int error)
{
	int	ret;

	if (csreq_) {
		ret = anoubis_csmulti_add_error(csreq_, path, error);
		if (ret < 0)
			return ret;
	}
	if (sigreq_) {
		ret = anoubis_csmulti_add_error(sigreq_, path, error);
		if (ret < 0)
			return ret;
	}
	return 0;
}

void
ComCsumAddTask::exec(void)
{
	int				 ret;

	/* Checksum calculation is done in the checksum thread. */
	if (type_ == Task::TYPE_CSUMCALC) {
		createRequests();
		if (getComTaskResult() != RESULT_INIT)
			return;
		type_ = Task::TYPE_COM;
		for (unsigned int i=0; i<paths_.size(); ++i) {
			u_int8_t	cs[ANOUBIS_CS_LEN];
			int		tmplen = ANOUBIS_CS_LEN;

			ret = anoubis_csum_link_calc(paths_[i].fn_str(),
			    cs, &tmplen);
			if (ret < 0) {
				ret = addErrorPath(paths_[i].fn_str(), -ret);
				if (ret < 0)
					goto fatal;
				continue;
			}
			ret = addPathToRequest(csreq_, paths_[i].fn_str(),
			    cs, ANOUBIS_CS_LEN);
			if (ret < 0)
				goto fatal;
			if (privKey_) {
				u_int8_t		*sig;
				unsigned int		 siglen;

				sig = anoubis_sign_csum(privKey_, cs, &siglen);
				ret = addPathToRequest(sigreq_,
				    paths_[i].fn_str(), sig, siglen);
				free(sig);
				if (ret < 0)
					goto fatal;
			}
		}
		privKey_ = NULL;
		return;
	}

	/* This is required if in case of an import. */
	createRequests();
	/* createRequests failed or some earlier error occured. We're done. */
	if (getComTaskResult() != RESULT_INIT)
		return;

	ta_ = NULL;
	done();
	return;

fatal:
	setComTaskResult(RESULT_LOCAL_ERROR);
	setResultDetails(-ret);
}

bool
ComCsumAddTask::done(void)
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

size_t
ComCsumAddTask::getCsum(unsigned int idx, u_int8_t *csum, size_t size) const
{
	struct anoubis_csmulti_record	*record;

	if (getComTaskResult() != RESULT_SUCCESS)
		return 0;
	record = anoubis_csmulti_find(csreq_, idx);
	if (!record || record->error)
		return 0;
	if (size < ANOUBIS_CS_LEN || record->u.add.cslen < ANOUBIS_CS_LEN)
		return 0;
	memcpy(csum, record->u.add.csdata, ANOUBIS_CS_LEN);
	return ANOUBIS_CS_LEN;
}

void
ComCsumAddTask::resetComTaskResult(void)
{
	ComTask::resetComTaskResult();
}

int
ComCsumAddTask::getChecksumError(unsigned int idx) const
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
ComCsumAddTask::getSignatureError(unsigned int idx) const
{
	struct anoubis_csmulti_record	*record;

	if (!sigreq_)
		return 0;
	record = anoubis_csmulti_find(sigreq_, idx);
	if (!record)
		return 0;
	return record->error;
}

unsigned int
ComCsumAddTask::getPathCount(void) const
{
	return paths_.size();
}

wxString
ComCsumAddTask::getPath(unsigned int idx) const
{
	if (idx < paths_.size())
		return paths_[idx];
	return wxEmptyString;
}

bool
ComCsumAddTask::setKeyId(const u_int8_t *id, unsigned int len)
{
	if (keyId_)
		free(keyId_);
	keyId_ = (u_int8_t*)malloc(len);
	if (keyId_ == NULL)
		return false;
	kidLen_ = len;
	memcpy(keyId_, id, len);
	return true;
}

bool
ComCsumAddTask::haveSignatures(void)
{
	return sigreq_ != NULL;
}

/* XXX CEH: This should pack multiple request into on CSMULTI request. */
void
ComCsumAddTask::setSfsEntry(const struct sfs_entry *entry)
{
	/*
	 * If we get our data from an sfs_entry there is no need to
	 * calculate the checksum.
	 */
	type_ = Task::TYPE_COM;
	addPath(wxString::FromAscii(entry->name));
	if (entry->signature && entry->keyid && entry->keylen)
		setKeyId(entry->keyid, entry->keylen);
	createRequests();
	if (entry->checksum)
		addPathToRequest(csreq_, paths_[0].fn_str(),
		    entry->checksum, ANOUBIS_CS_LEN);
	if (entry->signature)
		addPathToRequest(sigreq_, paths_[0].fn_str(),
		    entry->signature, entry->siglen);
}

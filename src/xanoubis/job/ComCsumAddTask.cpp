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
	privKey_ = 0;
	sig_ = 0;
	sigLen_ = 0;
	csLen_ = 0;
	resetComTaskResult();
	csumSent_ = false;
	csumSentOk_ = false;
	sigSent_ = false;
	sigSentOk_ = false;
	type_ = Task::TYPE_CSUMCALC;
	realCsumCalculated_ = false;
}

ComCsumAddTask::ComCsumAddTask(const wxString &file)
{
	privKey_ = 0;
	setPath(file);
}

ComCsumAddTask::~ComCsumAddTask(void)
{
	if (sig_)
		free(sig_);
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
ComCsumAddTask::exec(void)
{
	int				 result;

	/* Checksum calculation is done in the checksum thread. */
	if (type_ == Task::TYPE_CSUMCALC) {
		type_ = Task::TYPE_COM;
		if (csLen_ == 0) {
			int	tmplen = ANOUBIS_CS_LEN;
			result = csumCalc(cs_, &tmplen);
			if (result) {
				csLen_ = 0;
				setComTaskResult(RESULT_LOCAL_ERROR);
				setResultDetails(result);
				return;
			}
			csLen_ = tmplen;
			realCsumCalculated_ = true;
		}
		if(privKey_) {
			sig_ = anoubis_sign_csum(privKey_, cs_, &sigLen_);
			privKey_ = NULL;
			if (!sig_) {
				sigLen_ = 0;
				setComTaskResult(RESULT_LOCAL_ERROR);
				setResultDetails(ENOMEM);
				return;
			}
		}
		return;
	}

	/* Some earlier error occured. We're done. */
	if (getComTaskResult() != RESULT_INIT)
		return;

	ta_ = NULL;
	done();
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
		if (sigSent_)
			sigSentOk_ = true;
		else
			csumSentOk_ = true;
		anoubis_transaction_destroy(ta_);
		ta_ = NULL;
	}
	if (csLen_ != 0 && !csumSent_) {
		ta_ = anoubis_client_csumrequest_start(getClient(),
		    ANOUBIS_CHECKSUM_OP_ADDSUM, getPath().fn_str(),
		    cs_, csLen_, 0, 0, 0);
		if (ta_ == NULL)
			goto nomem;
		csumSent_ = true;
		return false;
	}
	if (sigLen_ != 0 && getKeyIdLen() != 0 && !sigSent_) {
		u_int8_t		*payload;

		payload = (u_int8_t*)malloc(sigLen_ + getKeyIdLen());
		if (payload == NULL)
			goto nomem;
		memcpy(payload, getKeyId(), getKeyIdLen());
		memcpy(payload + getKeyIdLen(), sig_, sigLen_);
		ta_ = anoubis_client_csumrequest_start(getClient(),
		    ANOUBIS_CHECKSUM_OP_ADDSIG, getPath().fn_str(),
		    payload, sigLen_, getKeyIdLen(), 0, 0);
		free(payload);
		if (ta_ == NULL)
			goto nomem;
		sigSent_ = true;
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
ComCsumAddTask::getCsum(u_int8_t *csum, size_t size) const
{
	if (!realCsumCalculated_)
		return 0;
	if (getComTaskResult() == RESULT_SUCCESS && getResultDetails() == 0) {
		if (size >= ANOUBIS_CS_LEN) {
			memcpy(csum, this->cs_, ANOUBIS_CS_LEN);
			return (ANOUBIS_CS_LEN);
		}
	}
	/*
	 * No checksumk received from anoubisd or destination-buffer not large
	 * enough.
	 */
	return (0);
}

void
ComCsumAddTask::resetComTaskResult(void)
{
	ComTask::resetComTaskResult();
}

bool
ComCsumAddTask::checksumOk(void)
{
	return csumSentOk_;
}

bool
ComCsumAddTask::signatureOk(void)
{
	return sigSentOk_;
}

void
ComCsumAddTask::setSfsEntry(const struct sfs_entry *entry)
{
	/*
	 * If we get our data from an sfs_entry there is no need to
	 * calculate the checksum.
	 */
	type_ = Task::TYPE_COM;
	setPath(wxString::FromAscii(entry->name));
	if (entry->checksum) {
		csLen_ = ANOUBIS_CS_LEN;
		memcpy(cs_, entry->checksum, ANOUBIS_CS_LEN);
	}
	if (entry->signature && entry->keyid && entry->siglen) {
		setKeyId(entry->keyid, entry->keylen);
		sig_ = (u_int8_t *)malloc(entry->siglen);
		sigLen_ = entry->siglen;
		memcpy(sig_, entry->signature, sigLen_);
	}
}

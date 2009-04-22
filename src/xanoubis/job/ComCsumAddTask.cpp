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

#include <client/anoubis_client.h>
#include <client/anoubis_transaction.h>
#include <csum/csum.h>
#include <anoubis_protocol.h>
#include <anoubis_sig.h>

#include "ComCsumAddTask.h"
#include "TaskEvent.h"

ComCsumAddTask::ComCsumAddTask(void)
{
	this->privKey_ = 0;
	this->sfsEntry_ = 0;
	this->forceSendCsum_ = false;
}

ComCsumAddTask::ComCsumAddTask(const wxString &file)
{
	this->privKey_ = 0;
	this->sfsEntry_ = 0;
	this->forceSendCsum_ = false;

	setPath(file);
}

ComCsumAddTask::~ComCsumAddTask(void)
{
}

bool
ComCsumAddTask::havePrivateKey(void) const
{
	return (this->privKey_ != 0);
}

void
ComCsumAddTask::setPrivateKey(struct anoubis_sig *privKey)
{
	this->privKey_ = privKey;
}

bool
ComCsumAddTask::haveSfsEntry(void) const
{
	return (this->sfsEntry_ != 0);
}

void
ComCsumAddTask::setSfsEntry(struct sfs_entry *entry, bool sendCsum)
{
	this->sfsEntry_ = entry;
	this->forceSendCsum_ = sendCsum;

	if (!sendCsum && sfsEntry_->keyid != 0) {
		/* Setup keyid, used later for communication */
		setKeyId(sfsEntry_->keyid, sfsEntry_->keylen);
	}
}

wxEventType
ComCsumAddTask::getEventType(void) const
{
	return (anTASKEVT_CSUM_ADD);
}

void
ComCsumAddTask::exec(void)
{
	/* Request to add a checksum for a file */
	int				req_op;
	u_int8_t			*payload;
	int				payloadLen;
	char				path[PATH_MAX];

	resetComTaskResult();
	ta_ = NULL;

	/* receive path to be send to anoubisd */
	if (!resolvePath(path)) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(errno);
	}

	if (haveSfsEntry()) {
		payload = createSfsMsg(&payloadLen, &req_op);

		if (payload == 0) {
			setComTaskResult(RESULT_LOCAL_ERROR);
			setResultDetails(errno);

			return;
		}
	} else if (haveKeyId() && (privKey_ != 0)) {
		payload = createSigMsg(&payloadLen);

		/* Reset private key, don't need it anymore */
		privKey_ = 0;

		if (payload == 0) {
			setComTaskResult(RESULT_LOCAL_ERROR);
			setResultDetails(errno);

			return;
		}

		req_op = ANOUBIS_CHECKSUM_OP_ADDSIG;
	} else {
		payload = createCsMsg(&payloadLen);
		if (payload == 0) {
			setComTaskResult(RESULT_LOCAL_ERROR);
			setResultDetails(errno);

			return;
		}

		req_op = ANOUBIS_CHECKSUM_OP_ADDSUM;
	}

	/* Send request to daemon */
	ta_ =  anoubis_client_csumrequest_start(getClient(), req_op, path,
	    payload, payloadLen - getKeyIdLen(), getKeyIdLen(), 0,
	    ANOUBIS_CSUM_NONE);

	free(payload);

	if(!ta_) {
		setComTaskResult(RESULT_COM_ERROR);
	}
}

bool
ComCsumAddTask::done(void) {
	if (ta_ == NULL)
		return (true);
	if (ta_->flags & ANOUBIS_T_DONE) {
		/* Result */
		if (ta_->result) {
			setComTaskResult(RESULT_REMOTE_ERROR);
			setResultDetails(ta_->result);
		} else
			setComTaskResult(RESULT_SUCCESS);

		anoubis_transaction_destroy(ta_);
		ta_ = NULL;
		return true;
	} else {
		return false;
	}
}

size_t
ComCsumAddTask::getCsum(u_int8_t *csum, size_t size) const
{
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

	for (int i = 0; i < ANOUBIS_CS_LEN; i++)
		this->cs_[i] = 0;
}

u_int8_t *
ComCsumAddTask::createSfsMsg(int *payload_len, int *req_op)
{
	u_int8_t *payload;

	if (forceSendCsum_ && (sfsEntry_->checksum != 0)) {
		/* Requested to send the checksum */
		*payload_len = ANOUBIS_CS_LEN;
		payload = (u_int8_t *)malloc(*payload_len);

		if (payload != 0)
			memcpy(payload, sfsEntry_->checksum, *payload_len);

		*req_op = ANOUBIS_CHECKSUM_OP_ADDSUM;
	} else if (!forceSendCsum_ && (sfsEntry_->signature != 0)) {
		/* Requested to send the signature */
		*payload_len = sfsEntry_->siglen + sfsEntry_->keylen;
		payload = (u_int8_t *)malloc(
		    sfsEntry_->siglen + sfsEntry_->keylen);

		if (payload != 0) {
			memcpy(payload, sfsEntry_->keyid, sfsEntry_->keylen);
			memcpy(payload + sfsEntry_->keylen,
			    sfsEntry_->signature, sfsEntry_->siglen);
		}

		*req_op = ANOUBIS_CHECKSUM_OP_ADDSIG;
	} else {
		/* Invalid combination */
		errno = EINVAL;
		payload = 0;
	}

	return (payload);
}

u_int8_t *
ComCsumAddTask::createCsMsg(int *payload_len)
{
	u_int8_t	*payload;
	int		result;

	if (payload_len == 0) {
		errno = EINVAL;
		return (0);
	}

	/* Need a message, which can hold the checksum */
	*payload_len = ANOUBIS_CS_LEN;
	payload = (u_int8_t *)malloc(*payload_len);
	if (payload == 0)
		return (0);

	/* Calculate checksum */
	result = csumCalc(cs_, payload_len);
	if (result != 0) {
		errno = result;
		free(payload);
		return (0);
	}

	/* Copy checksum into payload */
	memcpy(payload, cs_, *payload_len);

	/* Success */
	return (payload);
}

u_int8_t *
ComCsumAddTask::createSigMsg(int *payload_len)
{
	u_int8_t	*sig, *payload;
	int		cs_len;
	unsigned int	sig_len;
	int		result;

	if ((privKey_ == 0) || !haveKeyId() || (payload_len == 0)) {
		errno = EINVAL;
		return (0);
	}

	/* Calculate the checksum of the file */
	cs_len = ANOUBIS_CS_LEN;
	result = csumCalc(cs_, &cs_len);
	if (result != 0) {
		errno = result;
		return (0);
	}

	/* Sign the checksum */
	sig = anoubis_sign_csum(privKey_, cs_, &sig_len);
	if (sig == 0)
		return (0);

	/* Need a message, which can hold key-id and signature */
	*payload_len = getKeyIdLen() + sig_len;
	payload = (u_int8_t *)malloc(*payload_len);
	if (payload == 0) {
		free(sig);
		return (0);
	}

	/* Build the message: keyid followed by signature */
	memcpy(payload, getKeyId(), getKeyIdLen());
	memcpy(payload + getKeyIdLen(), sig, sig_len);
	free(sig);

	/* Success */
	return (payload);
}

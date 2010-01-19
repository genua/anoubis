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

#include <anoubis_client.h>
#include <anoubis_transaction.h>
#include <anoubis_protocol.h>
#include <apn.h>


#include "ComPolicyRequestTask.h"
#include "PolicyRuleSet.h"
#include "TaskEvent.h"

ComPolicyRequestTask::ComPolicyRequestTask(void)
{
	this->msg_ = 0;

	setRequestParameter(0, 0);
}

ComPolicyRequestTask::ComPolicyRequestTask(int prio, uid_t uid)
{
	this->msg_ = 0;

	setRequestParameter(prio, uid);
}

ComPolicyRequestTask::~ComPolicyRequestTask()
{
	resetComTaskResult();
}

int
ComPolicyRequestTask::getPriority(void) const
{
	return (this->prio_);
}

uid_t
ComPolicyRequestTask::getUid(void) const
{
	return (this->uid_);
}

void
ComPolicyRequestTask::setRequestParameter(int prio, uid_t uid)
{
	prio_ = prio;
	uid_ = uid;
}

wxEventType
ComPolicyRequestTask::getEventType(void) const
{
	return (anTASKEVT_POLICY_REQUEST);
}

void
ComPolicyRequestTask::exec(void)
{
	Policy_GetByUid			req;

	resetComTaskResult();
	ta_ = NULL;

	/* Prepare and set request message */
	set_value(req.ptype, ANOUBIS_PTYPE_GETBYUID);
	set_value(req.uid, this->uid_);
	set_value(req.prio, this->prio_);

	ta_ = anoubis_client_policyrequest_start(getClient(),
	    &req, sizeof(req));

	if(ta_ == NULL) {
		setComTaskResult(RESULT_COM_ERROR);
	}
}

bool
ComPolicyRequestTask::done(void)
{
	if (ta_ == NULL)
		return (true);
	if ((ta_->flags & ANOUBIS_T_DONE) == 0)
		return (false);

	if(ta_->result) {
		setComTaskResult(RESULT_REMOTE_ERROR);
		setResultDetails(ta_->result);
		return (true);
	} else {
		setComTaskResult(RESULT_SUCCESS);
	}

	/* Take reply out of transaction and finally destroy transaction */
	this->msg_ = ta_->msg;
	ta_->msg = 0;
	anoubis_transaction_destroy(ta_);
	ta_ = NULL;
	return (true);
}

struct apn_ruleset *
ComPolicyRequestTask::getPolicyApn(void) const
{
	struct anoubis_msg *tmp;

	/* Calculate number of needed iovec-strutures */
	int iovcnt = 0;
	for (tmp = this->msg_; tmp != NULL; tmp = tmp->next)
		iovcnt++;

	struct iovec iov[iovcnt];

	/* Copy data into iov */
	tmp = this->msg_;
	for (int i = 0; i < iovcnt; i++) {
		if (!VERIFY_LENGTH(tmp,
		    sizeof(Anoubis_PolicyReplyMessage)) ||
		    get_value(tmp->u.policyreply->error) != 0) {
			break;
		}
		iov[i].iov_len = tmp->length - CSUM_LEN
		    - sizeof(Anoubis_PolicyReplyMessage);
		iov[i].iov_base = tmp->u.policyreply->payload;
		tmp = tmp->next;
	}

	/* Parse apn */
	struct apn_ruleset *ruleset = NULL;
	int result = apn_parse_iovec("com", iov, iovcnt, &ruleset, 0);

	if ((result != 0) && (ruleset != NULL)) {
		apn_free_ruleset(ruleset);
		ruleset = NULL;
	}

	return (ruleset);
}

PolicyRuleSet *
ComPolicyRequestTask::getPolicy(void)
{
	PolicyRuleSet	   *ruleSet = NULL;
	struct apn_ruleset *rs = getPolicyApn();

	if (rs != 0) {
		ruleSet = new PolicyRuleSet(prio_, uid_, rs);
	}

	if (ruleSet != NULL) {
		ruleSet->setDaemonRuleSet();
	}

	return (ruleSet);
}

void
ComPolicyRequestTask::resetComTaskResult(void)
{
	ComTask::resetComTaskResult();

	/* Destroy reply-message */
	if (this->msg_ != 0) {
		struct anoubis_msg *tmp = this->msg_;

		while(tmp) {
			struct anoubis_msg *next = tmp->next;
			anoubis_msg_free(tmp);
			tmp = next;
		}

		this->msg_ = 0;
	}
}

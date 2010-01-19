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

#include <wx/ffile.h>
#include <wx/filename.h>

#include <anoubis_protocol.h>
#include <anoubis_sig.h>
#include <anoubis_client.h>
#include <anoubis_transaction.h>

#include "ComPolicySendTask.h"
#include "PolicyRuleSet.h"
#include "PolicyCtrl.h"
#include "TaskEvent.h"

ComPolicySendTask::ComPolicySendTask(void)
{
	this->policy_content_ = 0;
	this->uid_ = 0;
	this->prio_ = 0;
	this->privKey_ = 0;
	this->ruleSetId_ = 0;
}

ComPolicySendTask::~ComPolicySendTask(void)
{
	if (policy_content_ != 0)
		free(policy_content_);
}

bool
ComPolicySendTask::setPolicy(PolicyRuleSet *policy)
{
	uid_ = policy->getUid();
	prio_ = policy->getPriority();

	if (policy_content_ != 0)
		free(policy_content_);

	wxString content;
	if (policy->toString(content)) {
		policy_content_ = (char *)malloc(content.Len() + 1);
		strlcpy(policy_content_, content.fn_str(), content.Len() + 1);
		ruleSetId_ = policy->getRuleSetId();

		return (true);
	} else {
		policy_content_ = 0;
		return (false);
	}
}

bool
ComPolicySendTask::setPolicy(struct apn_ruleset *policy, uid_t uid, int prio)
{
	uid_ = uid;
	prio_ = prio;

	if (policy_content_ != 0)
		free(policy_content_);

	wxString content;
	if (getPolicyContent(policy, content)) {
		policy_content_ = (char *)malloc(content.Len() + 1);
		strlcpy(policy_content_, content.fn_str(), content.Len() + 1);
		ruleSetId_ = 0;

		return (true);
	} else {
		policy_content_ = 0;
		return (false);
	}
}

uid_t
ComPolicySendTask::getUid(void) const
{
	return (this->uid_);
}

int
ComPolicySendTask::getPriority(void) const
{
	return (this->prio_);
}

bool
ComPolicySendTask::havePrivateKey(void) const
{
	return (this->privKey_ != 0);
}

void
ComPolicySendTask::setPrivateKey(struct anoubis_sig *privKey)
{
	this->privKey_ = privKey;
}

wxEventType
ComPolicySendTask::getEventType(void) const
{
	return (anTASKEVT_POLICY_SEND);
}

void
ComPolicySendTask::exec(void)
{
	/*
	 * Load policy of xanoubis in anoubisd and activate the loaded
	 * policy
	 */
	Policy_SetByUid			*ureq = 0;
	unsigned char			*signature;
	unsigned int			 sig_len;
	size_t				 total = 0;

	resetComTaskResult();
	ta_ = NULL;

	if (policy_content_ == 0) {
		/* No policy assigned, leave task in init-state */
		return;
	}

	if (havePrivateKey()) {
		/* Signing policy requested */
		sig_len = strlen(policy_content_);
		signature = anoubis_sign_policy_buf(
		    this->privKey_, policy_content_, &sig_len);

		this->privKey_ = 0; /* Reset assigned, not used anymore */

		if (signature == 0) {
			setComTaskResult(RESULT_LOCAL_ERROR);
			setResultDetails(-sig_len);

			return;
		}
	} else {
		/* No signature */
		signature = 0;
		sig_len = 0;
	}

	/* Prepare request-message */
	total = sizeof(*ureq) + strlen(policy_content_) + sig_len;
	ureq = (Policy_SetByUid *)malloc(total);

	if (!ureq) {
		setComTaskResult(RESULT_OOM);
		return;
	}
	/*
	 * A valid policy has at least an alf and an sfs block. Thus it
	 * must have a length of at least 10 bytes. We refuse to send smaller
	 * policies here. However, the caller should not have requested
	 * a policy send in the first place.
	 */
	if (strlen(policy_content_) < 10) {
		setComTaskResult(RESULT_LOCAL_ERROR);
		setResultDetails(EINVAL);
		return;
	}

	set_value(ureq->ptype, ANOUBIS_PTYPE_SETBYUID);
	set_value(ureq->siglen, sig_len);
	set_value(ureq->uid, this->uid_);
	set_value(ureq->prio, (geteuid() == 0) ? this->prio_ : 1);

	if (sig_len > 0)
		memcpy(ureq->payload, signature, sig_len);

	memcpy(ureq->payload + sig_len, policy_content_,
	    strlen(policy_content_));

	/* Send message */
	ta_ = anoubis_client_policyrequest_start(getClient(), ureq, total);
	if (!ta_) {
		setComTaskResult(RESULT_COM_ERROR);
	}
}

bool
ComPolicySendTask::done(void)
{
	PolicyCtrl	*policyCtrl;
	PolicyRuleSet	*ruleSet;

	if (ta_ == NULL)
		return (true);
	if ((ta_->flags & ANOUBIS_T_DONE) == 0)
		return (false);

	/* Result */
	if (ta_->result) {
		setComTaskResult(RESULT_REMOTE_ERROR);
		setResultDetails(ta_->result);
	} else {
		setComTaskResult(RESULT_SUCCESS);
		if (ruleSetId_ != 0) {
			policyCtrl = PolicyCtrl::getInstance();
			ruleSet = policyCtrl->getRuleSet(ruleSetId_);
			if (ruleSet != NULL) {
				ruleSet->clearModified();
			}
		}
	}
	anoubis_transaction_destroy(ta_);
	ta_ = NULL;
	ruleSetId_ = 0;
	return (true);
}

bool
ComPolicySendTask::getPolicyContent(struct apn_ruleset *rs, wxString &policy)
{
	wxString tmpFile = wxFileName::CreateTempFileName(wxT(""));
	wxString content = wxT("");
	FILE *f;

	/* Write to temp. file */
	f = fopen(tmpFile.fn_str(), "w");
	if (f == 0)
		return (false);

	int result = apn_print_ruleset(rs, 0, f);

	fflush(f);
	fclose(f);

	if (result != 0) {
		wxRemoveFile(tmpFile);
		return (false);
	}

	/* Read from same file again */
	f = fopen(tmpFile.fn_str(), "r");
	if (f == 0) {
		wxRemoveFile(tmpFile);
		return (false);
	}

	while (!feof(f)) {
		char buf[256];
		int nread = fread(buf, 1, sizeof(buf), f);

		if (nread >= 0)
			content += wxString(buf, wxConvFile, nread);
		else
			break;
	}

	fclose(f);

	wxRemoveFile(tmpFile);

	policy = content;
	return (true);
}

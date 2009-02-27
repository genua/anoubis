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
#include <client/anoubis_client.h>
#include <client/anoubis_transaction.h>

#include "ComHandler.h"
#include "ComPolicySendTask.h"
#include "PolicyRuleSet.h"
#include "TaskEvent.h"

ComPolicySendTask::ComPolicySendTask(void)
{
	this->policy_rs_ = 0;
	this->apn_rs_ = 0;
	this->uid_ = 0;
	this->prio_ = 0;
	this->privKey_ = 0;
}

ComPolicySendTask::ComPolicySendTask(PolicyRuleSet *policy)
{
	policy_rs_ = NULL;	/* setPolicy will try an unlock. */
	setPolicy(policy);
	this->privKey_ = 0;
}

ComPolicySendTask::~ComPolicySendTask(void)
{
	if (policy_rs_)
		policy_rs_->unlock();
}

ComPolicySendTask::ComPolicySendTask(
    struct apn_ruleset *policy, uid_t uid, int prio)
{
	policy_rs_ = NULL;
	setPolicy(policy, uid, prio);
	this->privKey_ = 0;
}

PolicyRuleSet *
ComPolicySendTask::getPolicy(void) const
{
	return (this->policy_rs_);
}

struct apn_ruleset *
ComPolicySendTask::getPolicyApn(void) const
{
	return (this->apn_rs_);
}

void
ComPolicySendTask::setPolicy(PolicyRuleSet *policy)
{
	if (policy_rs_)
		policy_rs_->unlock();
	if (policy)
		policy->lock();
	policy_rs_ = policy;
	apn_rs_ = 0;
	uid_ = policy->getUid();
	prio_ = policy->getPriority();
}

void
ComPolicySendTask::setPolicy(struct apn_ruleset *policy, uid_t uid, int prio)
{
	if (policy_rs_)
		policy_rs_->unlock();
	policy_rs_ = 0;
	apn_rs_ = policy;
	uid_ = uid;
	prio_ = prio;
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
	struct anoubis_transaction	*ta = 0;
	Policy_SetByUid			*ureq = 0;
	wxString			content;
	unsigned char			*signature;
	unsigned int			sig_len;
	size_t				total = 0;

	resetComTaskResult();
	content = getPolicyContent();

	char c_content[content.Len() + 1];
	strlcpy(c_content, content.fn_str(), content.Len() + 1);

	if (havePrivateKey()) {
		/* Signing policy requested */
		sig_len = content.Len();
		signature = anoubis_sign_policy_buf(
		    this->privKey_, c_content, &sig_len);

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
	total = sizeof(*ureq) + content.Len() + sig_len;
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
	if (content.Len() < 10) {
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
	if (!content.IsEmpty())
		memcpy(ureq->payload + sig_len, c_content, content.Len());

	/* Send message */
	ta = anoubis_client_policyrequest_start(
	    getComHandler()->getClient(), ureq, total);
	if (!ta) {
		setComTaskResult(RESULT_COM_ERROR);
		return;
	}

	/* Wait for completition */
	while (!(ta->flags & ANOUBIS_T_DONE)) {
		if (!getComHandler()->waitForMessage()) {
			setComTaskResult(RESULT_COM_ERROR);
			return;
		}
	}

	/* Result */
	if (ta->result) {
		setComTaskResult(RESULT_REMOTE_ERROR);
		setResultDetails(ta->result);
	} else
		setComTaskResult(RESULT_SUCCESS);

	anoubis_transaction_destroy(ta);
}

wxString
ComPolicySendTask::getPolicyContent(void) const
{
	if (policy_rs_ != 0)
		return policy_rs_->toString();

	if (apn_rs_ != 0) {
		wxString tmpFile = wxFileName::CreateTempFileName(wxT(""));
		wxString content = wxT("");
		FILE *f;

		/* Write to temp. file */
		f = fopen(tmpFile.fn_str(), "w");
		if (f == 0)
			return (content);

		int result = apn_print_ruleset(apn_rs_, 0, f);

		fflush(f);
		fclose(f);

		if (result != 0) {
			wxRemoveFile(tmpFile);
			return (content);
		}

		/* Read from same file again */
		f = fopen(tmpFile.fn_str(), "r");
		if (f == 0) {
			wxRemoveFile(tmpFile);
			return (content);
		}

		while (!feof(f)) {
			char buf[256];
			int nread = fread(buf, 1, sizeof(buf), f);

			if (nread >= 0)
				content += wxString(buf, wxConvFile, nread);
			else
				break;
		}

		wxRemoveFile(tmpFile);

		return (content);
	}

	return wxT("");
}

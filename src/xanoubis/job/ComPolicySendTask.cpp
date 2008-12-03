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
}

ComPolicySendTask::ComPolicySendTask(PolicyRuleSet *policy)
{
	setPolicy(policy);
}

ComPolicySendTask::ComPolicySendTask(
    struct apn_ruleset *policy, uid_t uid, int prio)
{
	setPolicy(policy, uid, prio);
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
	policy_rs_ = policy;
	apn_rs_ = 0;
	uid_ = policy->getUid();
	prio_ = policy->getPriority();
}

void
ComPolicySendTask::setPolicy(struct apn_ruleset *policy, uid_t uid, int prio)
{
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
	size_t				total = 0;

	resetComTaskResult();
	content = getPolicyContent();

	/* Prepare request-message */
	total =	sizeof(*ureq) + content.Len();
	ureq = (Policy_SetByUid *)malloc(total);

	if (!ureq) {
		setComTaskResult(RESULT_OOM);
		return;
	}

	set_value(ureq->ptype, ANOUBIS_PTYPE_SETBYUID);
	set_value(ureq->uid, this->uid_);
	set_value(ureq->prio, (geteuid() == 0) ? this->prio_ : 1);

	if (!content.IsEmpty())
		memcpy(ureq->payload, content.fn_str(), content.Len());

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

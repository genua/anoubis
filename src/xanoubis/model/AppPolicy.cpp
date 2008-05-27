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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/param.h>
#include <sys/socket.h>

#ifndef LINUX
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

#include <apn.h>

#include "Policy.h"
#include "AlfPolicy.h"
#include "PolicyVisitor.h"

#include "AppPolicy.h"

AppPolicy::AppPolicy(struct apn_rule *appRule) : Policy(NULL)
{
	struct apn_alfrule	*alfRule;
	struct apn_sfsrule	*sfsRule;
	AlfPolicy		*newAlf;
	SfsPolicy		*newSfs;

	context_ = NULL;
	appRule_ = appRule;

	switch (appRule_->type) {
	case APN_ALF:
		alfRule = appRule_->rule.alf;
		while (alfRule) {
			newAlf = new AlfPolicy(this, alfRule);
			ruleList_.Append(newAlf);
			if (alfRule->type == APN_ALF_CTX) {
				context_ = newAlf;
			}
			alfRule = alfRule->next;
		}
		break;
	case APN_SFS:
		sfsRule = appRule_->rule.sfs;
		while (sfsRule) {
			newSfs = new SfsPolicy(this, sfsRule);
			ruleList_.Append(newSfs);
			sfsRule = sfsRule->next;
		}
		break;
	case APN_SB:
	case APN_VS:
	default:
		break;
	}
}

AppPolicy::~AppPolicy(void)
{
	ruleList_.DeleteContents(true);
	/* the apn structure will be deleted by PolicyRuleSet */
}

void
AppPolicy::accept(PolicyVisitor& visitor)
{
	PolicyList::iterator	 i;

	visitor.visitAppPolicy(this);

	if (visitor.shallBeenPropagated() == true) {
		for (i=ruleList_.begin(); i != ruleList_.end(); ++i) {
			(*i)->accept(visitor);
		}
	}
}

wxString
AppPolicy::getBinaryName(void)
{
	if (appRule_->app == NULL)
		return wxString::From8BitData("any");
	return wxString::From8BitData(appRule_->app->name);
}

wxString
AppPolicy::getHashTypeName(void)
{
	wxString result;

	if (appRule_->app == NULL)
		return wxString::From8BitData("");
	switch (appRule_->app->hashtype) {
	case APN_HASH_SHA256:
		result = wxT("SHA256");
		break;
	default:
		result = wxT("(unknown)");
		break;
	}

	return (result);
}

wxString
AppPolicy::getHashValue(void)
{
	wxString	result;
	unsigned int	length;

	length = 0;
	result = wxT("0x");

	if (appRule_->app == NULL)
		return wxString::From8BitData("any");
	switch (appRule_->app->hashtype) {
	case APN_HASH_SHA256:
		length = APN_HASH_SHA256_LEN;
		break;
	default:
		length = 0;
		result = wxT("(unknown hash type)");
		break;
	}

	for (unsigned int i=0; i<length; i++) {
		result += wxString::Format(wxT("%2.2x"),
		    (unsigned char)appRule_->app->hashvalue[i]);
	}

	return (result);
}

AlfPolicy *
AppPolicy::getContext(void)
{
	return (context_);
}

bool
AppPolicy::hasContext(void)
{
	if (context_ == NULL) {
		return (false);
	} else {
		return (true);
	}
}

int
AppPolicy::getType(void)
{
	return(appRule_->type);
}

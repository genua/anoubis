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

#include "FilterPolicy.h"

IMPLEMENT_CLASS(FilterPolicy, Policy);

FilterPolicy::FilterPolicy(AppPolicy *parent, struct apn_rule *rule)
    : Policy((parent==NULL ? NULL : parent->getParentRuleSet()), rule)
{
	parentPolicy_ = parent;
}

AppPolicy *
FilterPolicy::getParentPolicy(void) const
{
	return (parentPolicy_);
}

struct apn_rule *
FilterPolicy::createApnRule(void)
{
	struct apn_rule *rule;

	rule = (struct apn_rule *)calloc(1, sizeof(struct apn_rule));

	return (rule);
}

void
FilterPolicy::setModified(void)
{
	startChange();

	Policy::setModified();
	if (parentPolicy_ != NULL) {
		parentPolicy_->setModified();
	}

	finishChange();
}

wxString
FilterPolicy::getLogName(void) const
{
	return (logToString(getLogNo()));
}

wxString
FilterPolicy::getActionName(void) const
{
	return (actionToString(getActionNo()));
}

wxString
FilterPolicy::logToString(int logNo) const
{
	wxString log;

	switch (logNo) {
	case APN_LOG_NONE:
		log = wxT("none");
		break;
	case APN_LOG_NORMAL:
		log = wxT("normal");
		break;
	case APN_LOG_ALERT:
		log = wxT("alert");
		break;
	default:
		log = _("(unknown)");
		break;
	}

	return (log);
}

wxString
FilterPolicy::actionToString(int actionNo) const
{
	wxString action;

	switch (actionNo) {
	case APN_ACTION_ALLOW:
		action = wxT("allow");
		break;
	case APN_ACTION_DENY:
		action = wxT("deny");
		break;
	case APN_ACTION_ASK:
		action = wxT("ask");
		break;
	default:
		action = _("(unknown)");
		break;
	}

	return (action);
}

wxString
FilterPolicy::getRulePrefix(void) const
{
	return wxEmptyString;
}

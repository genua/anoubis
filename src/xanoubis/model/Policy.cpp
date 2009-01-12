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

#include "Policy.h"

#include "PolicyRuleSet.h"
#include <wx/datetime.h>

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(PolicyList);

IMPLEMENT_CLASS(Policy, Subject);

Policy::Policy(PolicyRuleSet *ruleSet, struct apn_rule *rule)
{
	modified_	= false;
	parentRuleSet_	= ruleSet;
	rule_		= rule;
}

Policy::~Policy(void)
{
	/* No need to clean apn_rule; This is done by the ruleset. */
}

bool
Policy::isModified(void) const
{
	return (modified_);
}

void
Policy::setModified(void)
{
	startChange();

	modified_ = true;
	if (parentRuleSet_ != NULL) {
		parentRuleSet_->setModified();
	}

	finishChange();
}

void
Policy::clearModified(void)
{
	if (modified_) {
		startChange();
		modified_ = false;
		finishChange();
	}
}

PolicyRuleSet *
Policy::getParentRuleSet(void) const
{
	return (parentRuleSet_);
}

int
Policy::getApnRuleId(void) const
{
	if (rule_ != NULL) {
		return (rule_->apn_id);
	}

	return (-1);
}

bool
Policy::hasScope(void) const
{
	if ((rule_ != NULL) && (rule_->scope != NULL)) {
		return (true);
	}

	return (false);
}

wxString
Policy::getScopeName(void) const
{
	wxString	scope;
	wxString	task;
	wxString	timeout;

	scope = wxEmptyString;
	if ((rule_ != NULL) && hasScope()) {
		if (rule_->scope->timeout != 0) {
			wxDateTime date(rule_->scope->timeout);
			timeout = date.Format();
		}
		if (rule_->scope->task != 0) {
			task.Printf(wxT("%llu"),
			   (unsigned long long)rule_->scope->task);
		}
		if (!timeout.IsEmpty() && task.IsEmpty()) {
			scope.Printf(wxT("task %s"), task.c_str());
		} else if (timeout.IsEmpty() && !task.IsEmpty()) {
			scope.Printf(wxT("until %s"), timeout.c_str());
		} else {
			scope.Printf(wxT("until %s and task %s"),
			    timeout.c_str(), task.c_str());
		}
	}

	return (scope);
}

/* XXX ch: I don't like this, better solution needed here */
void
Policy::setRuleEditorIndex(unsigned long idx)
{
	ruleEditorIndex_ = idx;
}

/* XXX ch: I don't like this, better solution needed here */
unsigned long
Policy::getRuleEditorIndex(void) const
{
	return (ruleEditorIndex_);
}

struct apn_rule *
Policy::getApnRule(void) const
{
	return (rule_);
}

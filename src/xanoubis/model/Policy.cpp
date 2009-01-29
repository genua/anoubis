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

bool
Policy::canMoveUp(void) const
{
	if (apn_can_move_up(getApnRule()))
		return (true);
	return (false);
}

bool
Policy::canMoveDown(void) const
{
	if (apn_can_move_down(getApnRule()))
		return (true);
	return (false);
}

bool
Policy::moveUp(void)
{
	PolicyRuleSet	*prs = getParentRuleSet();

	if (!prs || apn_move_up(getApnRule()) != 0)
		return (false);
	prs->refresh();
	prs->setModified();
	return (true);
}

bool
Policy::moveDown(void)
{
	PolicyRuleSet	*prs = getParentRuleSet();

	if (!prs || apn_move_down(getApnRule()) != 0)
		return (false);
	prs->refresh();
	prs->setModified();
	return (true);
}

/*
 * Memory management:
 * apn_remove will free the rule that was removed. This includes
 * any rules that might be dependent on the rule itself (i.e. filters
 * an an ALF application block).
 * The call to prs->refresh() will make sure that the PolicyRuleSet
 * no longer references the apn_rule that has been freed. The remaining
 * rules in the APN rule set still belong to the PolicyRuleSet for
 * memory management purposes and will ultimately be freed by the
 * destructor of prs.
 */
bool
Policy::remove(void)
{
	long		 id = getApnRuleId();
	PolicyRuleSet	*prs = getParentRuleSet();
	if (!prs || apn_remove(prs->getApnRuleSet(), id) != 0)
		return (false);
	prs->refresh();
	prs->setModified();
	return (true);
}

struct apn_rule *
Policy::cloneRule(void)
{
	return apn_copy_one_rule(rule_);
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

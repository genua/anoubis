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
#include <wx/ffile.h>
#include <wx/string.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

Policy::Policy(PolicyRuleSet *ruleSet, struct apn_rule *rule)
{
	modified_	= false;
	parentRuleSet_	= ruleSet;
	rule_		= rule;
	if (rule_ != NULL) {
		rule_->userdata	= this;
	}
}

Policy::~Policy(void)
{
	if (rule_ != NULL) {
		rule_->userdata = NULL;
	}
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
	sendPolicyChangeEvent();

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
	wxString	task = wxEmptyString;
	wxString	timeout = wxEmptyString;

	scope = wxEmptyString;
	if ((rule_ != NULL) && hasScope()) {
		if (rule_->scope->timeout != 0) {
			wxDateTime date(rule_->scope->timeout);
			timeout = date.Format();
		}
		if (rule_->scope->task != 0) {
			task.Printf(wxT("%" PRIu64),
			   (unsigned long long)rule_->scope->task);
		}
		if (!timeout.IsEmpty() && task.IsEmpty()) {
			scope.Printf(_("until %ls"), timeout.c_str());
		} else if (timeout.IsEmpty() && !task.IsEmpty()) {
			scope.Printf(_("task %ls"), task.c_str());
		} else {
			scope.Printf(_("until %ls for task %ls"),
			    timeout.c_str(), task.c_str());
		}
	}

	return (scope);
}

bool
Policy::getFlag(unsigned int flag) const
{
	return ((rule_ != 0) ? (rule_->flags & flag) : false);
}

void
Policy::setFlag(unsigned int flag, bool value)
{
	if ((rule_ == 0) || (getFlag(flag) == value)) {
		/* Nothing to do */
		return;
	}

	startChange();

	if (value)
		rule_->flags |= flag;
	else
		rule_->flags &= ~flag;

	setModified();

	finishChange();
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
Policy::canDelete(void) const
{
	return (true);
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
	rule_ = NULL;
	prs->refresh();
	prs->setModified();
	return (true);
}

bool
Policy::clone(void)
{
	long			 id = getApnRuleId();
	PolicyRuleSet		*prs = getParentRuleSet();
	struct apn_rule		*copy;
	struct apn_ruleset	*rs;

	if (prs == NULL)
		return false;
	rs = prs->getApnRuleSet();
	if (rs == NULL)
		return false;
	copy = apn_copy_one_rule(rule_);
	if (!copy)
		return false;
	if (apn_insert(rs, copy, id) != 0) {
		apn_free_one_rule(copy, NULL);
		return false;
	}
	prs->refresh();
	return true;
}

wxString
Policy::toString(void) const
{
	int	 result;
	wxString content;
	wxString tmpFileName;
	wxFFile	 tmpFile;

	result = 1;
	content = wxEmptyString;
	tmpFileName = wxFileName::CreateTempFileName(wxEmptyString);

	if (!tmpFile.Open(tmpFileName, wxT("w"))) {
		/* Couldn't open / fill tmp file. */
		return (wxEmptyString);
	}

	/* Write the ruleset to tmp file. */
	result = apn_print_rule(getApnRule(), 0, tmpFile.fp());
	tmpFile.Flush();
	tmpFile.Close();

	if (result != 0) {
		/* Something went wrong during file filling! */
		wxRemoveFile(tmpFileName);
		return (wxEmptyString);
	}

	if (tmpFile.Open(tmpFileName, wxT("r"))) {
		if (!tmpFile.ReadAll(&content)) {
			/* Error during file read - clear read stuff. */
			content = wxEmptyString;
		}
		tmpFile.Close();
	}

	wxRemoveFile(tmpFileName);

	return (content);
}

struct apn_rule *
Policy::getApnRule(void) const
{
	return (rule_);
}

extern "C" {

void policy_destructor(void *arg)
{
	Policy	*pol = (Policy *)arg;

	pol->rule_ = NULL;
}

}

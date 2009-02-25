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

#include "DefaultFilterPolicy.h"

#include "PolicyVisitor.h"
#include "PolicyRuleSet.h"

IMPLEMENT_CLASS(DefaultFilterPolicy, FilterPolicy);

DefaultFilterPolicy::DefaultFilterPolicy(AppPolicy *parentPolicy,
    struct apn_rule *rule) : FilterPolicy(parentPolicy, rule)
{
}

DefaultFilterPolicy::DefaultFilterPolicy(AppPolicy *parentPolicy)
    : FilterPolicy(parentPolicy, DefaultFilterPolicy::createApnRule())
{
}

wxString
DefaultFilterPolicy::getTypeIdentifier(void) const
{
	return (wxT("Default"));
}

struct apn_rule *
DefaultFilterPolicy::createApnRule(void)
{
	struct apn_rule *rule;

	rule = FilterPolicy::createApnRule();
	if (rule != NULL) {
		rule->apn_type = APN_DEFAULT;
	}

	return (rule);
}

bool
DefaultFilterPolicy::createApnInserted(AppPolicy *parent, unsigned int id)
{
	int		 rc;
	struct apn_rule *rule;
	PolicyRuleSet	*ruleSet;

	if (parent == NULL) {
		return (false);
	}

	ruleSet = parent->getParentRuleSet();
	if (ruleSet == NULL) {
		return (false);
	}

	rule = DefaultFilterPolicy::createApnRule();
	if (rule == NULL) {
		return (false);
	}

	/* No 'insert-before'-id given: insert on top by using block-id . */
	if (id == 0) {
		id = parent->getApnRuleId();
	}

	if (parent->IsKindOf(CLASSINFO(AlfAppPolicy))) {
		rc = apn_insert_alfrule(ruleSet->getApnRuleSet(), rule, id);
	} else if (parent->IsKindOf(CLASSINFO(SfsAppPolicy))) {
		/* Sfs has it's own default policies */
		rc = -1;
	} else if (parent->IsKindOf(CLASSINFO(ContextAppPolicy))) {
		rc = apn_insert_ctxrule(ruleSet->getApnRuleSet(), rule, id);
	} else if (parent->IsKindOf(CLASSINFO(SbAppPolicy))) {
		rc = apn_insert_sbrule(ruleSet->getApnRuleSet(), rule, id);
	} else {
		/* Unknown parent type - nothing to do */
		rc = -1;
	}

	if (rc != 0) {
		apn_free_one_rule(rule, NULL);
		return (false);
	}

	return (true);
}

void
DefaultFilterPolicy::accept(PolicyVisitor &visitor)
{
	visitor.visitDefaultFilterPolicy(this);
}

bool
DefaultFilterPolicy::setLogNo(int log)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.apndefault.log = log;
	setModified();
	finishChange();

	return (true);
}

int
DefaultFilterPolicy::getLogNo(void) const
{
	int		 log;
	struct apn_rule	*rule;

	log  = -1;
	rule = getApnRule();

	if (rule != NULL) {
		log = rule->rule.apndefault.log;
	}

	return (log);
}

bool
DefaultFilterPolicy::setActionNo(int action)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.apndefault.action = action;
	setModified();
	finishChange();

	return (true);
}

int
DefaultFilterPolicy::getActionNo(void) const
{
	int		 action;
	struct apn_rule	*rule;

	action = -1;
	rule   = getApnRule();

	if (rule != NULL) {
		action = rule->rule.apndefault.action;
	}

	return (action);
}

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

#include "SfsFilterPolicy.h"
#include "PolicyVisitor.h"
#include "PolicyRuleSet.h"

IMPLEMENT_CLASS(SfsFilterPolicy, FilterPolicy);

SfsFilterPolicy::SfsFilterPolicy(AppPolicy *parentPolicy, struct apn_rule *rule)
    : FilterPolicy(parentPolicy, rule)
{
}

wxString
SfsFilterPolicy::getTypeIdentifier(void) const
{
	return (wxT("SFS"));
}

struct apn_rule *
SfsFilterPolicy::createApnRule(void)
{
	struct apn_rule *rule;

	rule = FilterPolicy::createApnRule();
	if (rule != NULL) {
		rule->apn_type = APN_SFS_ACCESS;
		rule->rule.sfsaccess.subject.type = APN_CS_UID_SELF;
	}

	return (rule);
}

bool
SfsFilterPolicy::createApnInserted(AppPolicy *parent, unsigned int id)
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

	rule = SfsFilterPolicy::createApnRule();
	if (rule == NULL) {
		return (false);
	}

	/* No 'insert-before'-id given: insert on top by using block-id . */
	if (id == 0) {
		id = parent->getApnRuleId();
	}

	rc = apn_insert_sfsrule(ruleSet->getApnRuleSet(), rule, id);

	if (rc != 0) {
		apn_free_one_rule(rule, NULL);
		return (false);
	}

	return (true);
}

void
SfsFilterPolicy::accept(PolicyVisitor &visitor)
{
	visitor.visitSfsFilterPolicy(this);
}

bool
SfsFilterPolicy::setActionNo(int)
{
	/* Sfs filter policies has no action. */
	return (false);
}

int
SfsFilterPolicy::getActionNo(void) const
{
	/* Sfs filter policies has no action. */
	return (-1);
}

bool
SfsFilterPolicy::setLogNo(int)
{
	/* Sfs filter policies has no log. */
	return (false);
}

int
SfsFilterPolicy::getLogNo(void) const
{
	/* Sfs filter policies has no log. */
	return (-1);
}

bool
SfsFilterPolicy::setPath(wxString path)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if ((rule == NULL) || path.IsEmpty()){
		return (false);
	}

	startChange();
	if (rule->rule.sfsaccess.path != NULL) {
		free(rule->rule.sfsaccess.path);
		rule->rule.sfsaccess.path = NULL;
		setModified();
	}

	if (path.Cmp(wxT("any")) != 0) {
		rule->rule.sfsaccess.path = strdup(path.fn_str());
		setModified();
	}

	finishChange();
	return (true);
}

wxString
SfsFilterPolicy::getPath(void) const
{
	wxString	 path = getRulePrefix();

	if (path.IsEmpty()) {
		path = wxT("any");
	}
	return path;
}

wxString
SfsFilterPolicy::getRulePrefix(void) const
{
	struct apn_rule		*rule = getApnRule();

	if (rule && rule->rule.sfsaccess.path) {
		return wxString::From8BitData(rule->rule.sfsaccess.path);
	} else {
		return wxEmptyString;
	}
}

bool
SfsFilterPolicy::setSubjectSelf(bool selfSigned)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	cleanSubject(rule);

	if (selfSigned) {
		rule->rule.sfsaccess.subject.type = APN_CS_KEY_SELF;
	} else {
		rule->rule.sfsaccess.subject.type = APN_CS_UID_SELF;
	}

	setModified();
	finishChange();

	return (true);
}

bool
SfsFilterPolicy::setSubjectUid(uid_t uid)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	cleanSubject(rule);

	rule->rule.sfsaccess.subject.type = APN_CS_UID;
	rule->rule.sfsaccess.subject.value.uid = uid;

	setModified();
	finishChange();

	return (true);
}

bool
SfsFilterPolicy::setSubjectKey(wxString key)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	cleanSubject(rule);

	rule->rule.sfsaccess.subject.type = APN_CS_KEY;
	rule->rule.sfsaccess.subject.value.keyid = strdup(key.fn_str());

	setModified();
	finishChange();

	return (true);
}

int
SfsFilterPolicy::getSubjectTypeNo(void) const
{
	int		 type;
	struct apn_rule *rule;

	type = APN_CS_NONE;
	rule = getApnRule();
	if (rule != NULL) {
		type = rule->rule.sfsaccess.subject.type;
	}

	return (type);
}

wxString
SfsFilterPolicy::getSubjectName(void) const
{
	wxString	 subjectName;
	struct apn_rule *rule;

	subjectName = wxEmptyString;
	rule = getApnRule();
	if (rule != NULL) {
		switch (rule->rule.sfsaccess.subject.type) {
		case APN_CS_KEY_SELF:
			subjectName = wxT("signed-self");
			break;
		case APN_CS_UID_SELF:
			subjectName = wxT("self");
			break;
		case APN_CS_KEY:
			subjectName = wxString::From8BitData(
			    rule->rule.sfsaccess.subject.value.keyid);
			subjectName.Prepend(wxT("key "));
			break;
		case APN_CS_UID:
			subjectName.Printf(wxT("uid %d"),
			    rule->rule.sfsaccess.subject.value.uid);
			break;
		case APN_CS_NONE:
			subjectName = wxT("none");
			break;
		case APN_CS_CSUM:
			/* Not used by sfs policies. */
			/* FALLTHROUGH */
		default:
			break;
		}
	}

	return (subjectName);
}

bool
SfsFilterPolicy::setValid(int action, int log)
{
	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();

	setValidAction(action);
	setValidLog(log);

	setModified();
	finishChange();

	return (true);
}

bool
SfsFilterPolicy::setInvalid(int action, int log)
{
	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();

	setInvalidAction(action);
	setInvalidLog(log);

	setModified();
	finishChange();

	return (true);
}

bool
SfsFilterPolicy::setUnknown(int action, int log)
{
	if (getApnRule() == NULL) {
		return (false);
	}

	startChange();

	setUnknownAction(action);
	setUnknownLog(log);

	setModified();
	finishChange();

	return (true);
}

bool
SfsFilterPolicy::setValidAction(int action)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();

	rule->rule.sfsaccess.valid.action = action;

	setModified();
	finishChange();

	return (true);
}

int
SfsFilterPolicy::getValidActionNo(void) const
{
	int		 action;
	struct apn_rule *rule;

	action = -1;
	rule   = getApnRule();

	if (rule != NULL) {
		action = rule->rule.sfsaccess.valid.action;
	}

	return (action);
}

wxString
SfsFilterPolicy::getValidActionName(void) const
{
	return (actionToString(getValidActionNo()));
}

bool
SfsFilterPolicy::setValidLog(int log)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();

	rule->rule.sfsaccess.valid.log = log;

	setModified();
	finishChange();

	return (true);
}

int
SfsFilterPolicy::getValidLogNo(void) const
{
	int		 log;
	struct apn_rule *rule;

	log  = -1;
	rule = getApnRule();

	if (rule != NULL) {
		log = rule->rule.sfsaccess.valid.log;
	}

	return (log);
}

wxString
SfsFilterPolicy::getValidLogName(void) const
{
	return (logToString(getValidLogNo()));
}

bool
SfsFilterPolicy::setInvalidAction(int action)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();

	rule->rule.sfsaccess.invalid.action = action;

	setModified();
	finishChange();

	return (true);
}

int
SfsFilterPolicy::getInvalidActionNo(void) const
{
	int		 action;
	struct apn_rule *rule;

	action = -1;
	rule   = getApnRule();

	if (rule != NULL) {
		action = rule->rule.sfsaccess.invalid.action;
	}

	return (action);
}

wxString
SfsFilterPolicy::getInvalidActionName(void) const
{
	return (actionToString(getInvalidActionNo()));
}

bool
SfsFilterPolicy::setInvalidLog(int log)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();

	rule->rule.sfsaccess.invalid.log = log;

	setModified();
	finishChange();

	return (true);
}

int
SfsFilterPolicy::getInvalidLogNo(void) const
{
	int		 log;
	struct apn_rule *rule;

	log  = -1;
	rule = getApnRule();

	if (rule != NULL) {
		log = rule->rule.sfsaccess.invalid.log;
	}

	return (log);
}

wxString
SfsFilterPolicy::getInvalidLogName(void) const
{
	return (logToString(getInvalidLogNo()));
}

bool
SfsFilterPolicy::setUnknownAction(int action)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();

	rule->rule.sfsaccess.unknown.action = action;

	setModified();
	finishChange();

	return (true);
}

int
SfsFilterPolicy::getUnknownActionNo(void) const
{
	int		 action;
	struct apn_rule *rule;

	action = -1;
	rule   = getApnRule();

	if (rule != NULL) {
		action = rule->rule.sfsaccess.unknown.action;
	}

	return (action);
}

wxString
SfsFilterPolicy::getUnknownActionName(void) const
{
	return (actionToString(getUnknownActionNo()));
}

bool
SfsFilterPolicy::setUnknownLog(int log)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();

	rule->rule.sfsaccess.unknown.log = log;

	setModified();
	finishChange();

	return (true);
}

int
SfsFilterPolicy::getUnknownLogNo(void) const
{
	int		 log;
	struct apn_rule *rule;

	log  = -1;
	rule = getApnRule();

	if (rule != NULL) {
		log = rule->rule.sfsaccess.unknown.log;
	}

	return (log);
}

wxString
SfsFilterPolicy::getUnknownLogName(void) const
{
	return (logToString(getUnknownLogNo()));
}

void
SfsFilterPolicy::cleanSubject(struct apn_rule *rule)
{
	if (rule == NULL) {
		return;
	}

	switch (rule->rule.sfsaccess.subject.type) {
	case APN_CS_KEY:
		free(rule->rule.sfsaccess.subject.value.keyid);
		rule->rule.sfsaccess.subject.value.keyid = NULL;
		break;
	case APN_CS_UID:
		rule->rule.sfsaccess.subject.value.uid = 0 - 1;
		break;
	case APN_CS_CSUM:
		free(rule->rule.sfsaccess.subject.value.csum);
		rule->rule.sfsaccess.subject.value.csum = NULL;
		break;
	default:
		break;
	}
}

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

#include "SbAccessFilterPolicy.h"
#include "PolicyVisitor.h"
#include "PolicyRuleSet.h"

IMPLEMENT_CLASS(SbAccessFilterPolicy, FilterPolicy);

SbAccessFilterPolicy::SbAccessFilterPolicy(AppPolicy *parentPolicy,
    struct apn_rule *rule) : FilterPolicy(parentPolicy, rule)
{
}

wxString
SbAccessFilterPolicy::getTypeIdentifier(void) const
{
	return (wxT("SB"));
}

void
SbAccessFilterPolicy::accept(PolicyVisitor &visitor)
{
	visitor.visitSbAccessFilterPolicy(this);
}

struct apn_rule *
SbAccessFilterPolicy::createApnRule(void)
{
	struct apn_rule *rule;

	rule = FilterPolicy::createApnRule();
	if (rule != NULL) {
		rule->apn_type = APN_SB_ACCESS;
		rule->rule.sbaccess.amask = APN_SBA_ALL;
	}

	return (rule);
}

bool
SbAccessFilterPolicy::createApnInserted(AppPolicy *parent, unsigned int id)
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

	rule = SbAccessFilterPolicy::createApnRule();
	if (rule == NULL) {
		return (false);
	}

	/* No 'insert-before'-id given: insert on top by using block-id . */
	if (id == 0) {
		id = parent->getApnRuleId();
	}

	rc = apn_insert_sbrule(ruleSet->getApnRuleSet(), rule, id);

	if (rc != 0) {
		apn_free_one_rule(rule, NULL);
		return (false);
	}

	return (true);
}

bool
SbAccessFilterPolicy::setActionNo(int action)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.sbaccess.action = action;
	setModified();
	finishChange();

	return (true);
}

int
SbAccessFilterPolicy::getActionNo(void) const
{
	int		 action;
	struct apn_rule *rule;

	action = -1;
	rule   = getApnRule();

	if (rule != NULL) {
		action = rule->rule.sbaccess.action;
	}

	return (action);
}

bool
SbAccessFilterPolicy::setLogNo(int log)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.sbaccess.log = log;
	setModified();
	finishChange();

	return (true);
}

int
SbAccessFilterPolicy::getLogNo(void) const
{
	int		 log;
	struct apn_rule *rule;

	log = -1;
	rule   = getApnRule();

	if (rule != NULL) {
		log = rule->rule.sbaccess.log;
	}

	return (log);
}

bool
SbAccessFilterPolicy::setPath(wxString path)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if ((rule == NULL) || path.IsEmpty()){
		return (false);
	}

	startChange();
	if (rule->rule.sbaccess.path != NULL) {
		free(rule->rule.sbaccess.path);
		rule->rule.sbaccess.path = NULL;
		setModified();
	}

	if (path.Cmp(wxT("any")) != 0) {
		rule->rule.sbaccess.path = strdup(path.fn_str());
		setModified();
	}

	finishChange();
	return (true);
}

wxString
SbAccessFilterPolicy::getPath(void) const
{
	wxString	 path = getRulePrefix();

	if (path.IsEmpty()) {
		path = wxT("any");
	}
	return path;
}

bool
SbAccessFilterPolicy::setSubjectSelf(bool selfSigned)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	cleanSubject(rule);

	if (selfSigned) {
		rule->rule.sbaccess.cs.type = APN_CS_KEY_SELF;
	} else {
		rule->rule.sbaccess.cs.type = APN_CS_UID_SELF;
	}

	setModified();
	finishChange();

	return (true);
}

bool
SbAccessFilterPolicy::setSubjectUid(uid_t uid)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	cleanSubject(rule);

	rule->rule.sbaccess.cs.type = APN_CS_UID;
	rule->rule.sbaccess.cs.value.uid = uid;

	setModified();
	finishChange();

	return (true);
}

bool
SbAccessFilterPolicy::setSubjectKey(wxString key)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	cleanSubject(rule);

	rule->rule.sbaccess.cs.type = APN_CS_KEY;
	rule->rule.sbaccess.cs.value.keyid = strdup(key.fn_str());

	setModified();
	finishChange();

	return (true);
}

bool
SbAccessFilterPolicy::setSubjectCsum(wxString csumString)
{
	unsigned char	 csum[MAX_APN_HASH_LEN];
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	memset(csum, 0, MAX_APN_HASH_LEN);
	PolicyUtils::stringToCsum(csumString, csum, MAX_APN_HASH_LEN);

	startChange();
	cleanSubject(rule);

	rule->rule.sbaccess.cs.type = APN_CS_CSUM;
	/* A previous value was freed by cleanSubject() */
	rule->rule.sbaccess.cs.value.csum = (u_int8_t *)calloc(
	    MAX_APN_HASH_LEN, sizeof(unsigned char));
	memcpy(rule->rule.sbaccess.cs.value.csum, csum, MAX_APN_HASH_LEN);

	setModified();
	finishChange();

	return (true);
}

int
SbAccessFilterPolicy::getSubjectTypeNo(void) const
{
	int		 type;
	struct apn_rule *rule;

	type = APN_CS_NONE;
	rule = getApnRule();
	if (rule != NULL) {
		type = rule->rule.sbaccess.cs.type;
	}

	return (type);
}

wxString
SbAccessFilterPolicy::getSubjectName(void) const
{
	wxString	 subjectName;
	struct apn_rule *rule;

	subjectName = wxEmptyString;
	rule = getApnRule();
	if (rule != NULL) {
		switch (rule->rule.sbaccess.cs.type) {
		case APN_CS_KEY_SELF:
			subjectName = wxT("signed-self");
			break;
		case APN_CS_UID_SELF:
			subjectName = wxT("self");
			break;
		case APN_CS_KEY:
			subjectName = wxString::From8BitData(
			    rule->rule.sbaccess.cs.value.keyid);
			subjectName.Prepend(wxT("key "));
			break;
		case APN_CS_UID:
			subjectName.Printf(wxT("uid %d"),
			    rule->rule.sbaccess.cs.value.uid);
			break;
		case APN_CS_NONE:
			subjectName = wxT("none");
			break;
		case APN_CS_CSUM:
			PolicyUtils::csumToString(
			    rule->rule.sbaccess.cs.value.csum,
			    MAX_APN_HASH_LEN, subjectName);
			subjectName.Prepend(wxT("csum "));
			break;
		default:
			break;
		}
	}

	return (subjectName);
}

bool
SbAccessFilterPolicy::setAccessMask(unsigned int mask)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.sbaccess.amask = mask;
	setModified();
	finishChange();

	return (true);
}

unsigned int
SbAccessFilterPolicy::getAccessMaskNo(void) const
{
	unsigned int	 mask;
	struct apn_rule *rule;

	mask = 0;
	rule = getApnRule();
	if (rule != NULL) {
		mask = rule->rule.sbaccess.amask;
	}

	return (mask);
}

wxString
SbAccessFilterPolicy::getAccessMaskName(void) const
{
	unsigned int	maskNo;
	wxString	maskName;

	maskNo = getAccessMaskNo();
	if ((maskNo & APN_SBA_READ) > 0) {
		maskName.Append(wxT("r"));
	}
	if ((maskNo & APN_SBA_WRITE) > 0) {
		maskName.Append(wxT("w"));
	}
	if ((maskNo & APN_SBA_EXEC) > 0) {
		maskName.Append(wxT("x"));
	}

	return (maskName);
}

void
SbAccessFilterPolicy::cleanSubject(struct apn_rule *rule)
{
	if (rule == NULL) {
		return;
	}

	switch (rule->rule.sbaccess.cs.type) {
	case APN_CS_KEY:
		free(rule->rule.sbaccess.cs.value.keyid);
		rule->rule.sbaccess.cs.value.keyid = NULL;
		break;
	case APN_CS_UID:
		rule->rule.sbaccess.cs.value.uid = 0 - 1;
		break;
	case APN_CS_CSUM:
		free(rule->rule.sbaccess.cs.value.csum);
		rule->rule.sbaccess.cs.value.csum = NULL;
		break;
	default:
		break;
	}
}

wxString
SbAccessFilterPolicy::getRulePrefix(void) const
{
	struct apn_rule		*rule = getApnRule();
	if (rule && rule->rule.sbaccess.path) {
		return wxString::From8BitData(rule->rule.sbaccess.path);
	} else {
		return wxEmptyString;
	}
}

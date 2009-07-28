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

#include "ContextFilterPolicy.h"
#include "PolicyVisitor.h"
#include "PolicyRuleSet.h"

IMPLEMENT_CLASS(ContextFilterPolicy, FilterPolicy);

ContextFilterPolicy::ContextFilterPolicy(AppPolicy *parentPolicy,
    struct apn_rule *rule) : FilterPolicy(parentPolicy, rule)
{
}

ContextFilterPolicy::ContextFilterPolicy(ContextAppPolicy *parentPolicy)
    : FilterPolicy(parentPolicy, ContextFilterPolicy::createApnRule())
{
}

wxString
ContextFilterPolicy::getTypeIdentifier(void) const
{
	return (wxT("CTX"));
}

void
ContextFilterPolicy::accept(PolicyVisitor &visitor)
{
	visitor.visitContextFilterPolicy(this);
}

struct apn_rule *
ContextFilterPolicy::createApnRule(void)
{
	struct apn_rule *rule;

	rule = FilterPolicy::createApnRule();
	if (rule != NULL) {
		rule->apn_type = APN_CTX_RULE;
	}

	return (rule);
}

bool
ContextFilterPolicy::createApnInserted(AppPolicy *parent, unsigned int id)
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

	rule = ContextFilterPolicy::createApnRule();
	if (rule == NULL) {
		return (false);
	}

	/* No 'insert-before'-id given: insert on top by using block-id . */
	if (id == 0) {
		id = parent->getApnRuleId();
	}

	rc = apn_insert_ctxrule(ruleSet->getApnRuleSet(), rule, id);

	if (rc != 0) {
		apn_free_one_rule(rule, NULL);
		return (false);
	}

	return (true);
}

bool
ContextFilterPolicy::setActionNo(int)
{
	/* Context filter policies has no action. */
	return (false);
}

int
ContextFilterPolicy::getActionNo(void) const
{
	/* Context filter policies has no action. */
	return (-1);
}

bool
ContextFilterPolicy::setLogNo(int)
{
	/* Context filter policies has no log. */
	return (false);
}

int
ContextFilterPolicy::getLogNo(void) const
{
	/* Context filter policies has no log. */
	return (-1);
}

bool
ContextFilterPolicy::setContextTypeNo(int contextType)
{
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (false);
	}

	startChange();
	rule->rule.apncontext.type = contextType;
	setModified();
	finishChange();

	return (true);
}

int
ContextFilterPolicy::getContextTypeNo(void) const
{
	int		 contextType;
	struct apn_rule *rule;

	contextType = -1;
	rule	    = getApnRule();

	if (rule != NULL) {
		contextType = rule->rule.apncontext.type;
	}

	return (contextType);
}

wxString
ContextFilterPolicy::getContextTypeName (void) const
{
	wxString contextType;

	switch (getContextTypeNo()) {
	case APN_CTX_NEW:
		contextType = wxT("new");
		break;
	case APN_CTX_OPEN:
		contextType = wxT("open");
		break;
	default:
		contextType = _("(unknown)");
		break;
	}

	return (contextType);
}

unsigned int
ContextFilterPolicy::getBinaryCount(void) const
{
	unsigned int	 count;
	struct apn_app	*app;
	struct apn_rule *rule;

	count = 0;
	app   = NULL;
	rule  = getApnRule();

	if (rule != NULL) {
		app = rule->rule.apncontext.application;
		while (app != NULL) {
			count++;
			app = app->next;
		}
	}

	return (count);
}

bool
ContextFilterPolicy::setBinaryName(wxString binary, unsigned int idx)
{
	struct apn_app	*app;
	struct apn_rule *rule;

	rule = getApnRule();

	if ((rule == NULL) || (idx >= getBinaryCount()) || binary.IsEmpty()) {
		return (false);
	}

	if (binary.Cmp(wxT("any")) == 0) {
		startChange();
		apn_free_app(rule->rule.apncontext.application);
		rule->rule.apncontext.application = NULL;
		setModified();
		finishChange();
		return (true);
	}

	app = seekAppByIndex(idx);
	if (app == NULL) {
		return (false);
	}

	startChange();
	free(app->name);
	app->name = strdup(binary.To8BitData());
	app->hashtype = APN_HASH_SHA256;
	setModified();
	finishChange();

	return (true);
}

bool
ContextFilterPolicy::addToBinaryListAt(int pos, wxString name)
{
	bool		 result;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL) {
		return (false);
	}

	startChange();
	result = PolicyUtils::addToAppListAt(
	    &(rule->rule.apncontext.application), pos, name);
	setModified();
	finishChange();

	return (result);
}

wxString
ContextFilterPolicy::getBinaryName(unsigned int idx) const
{
	wxString	 binary;
	struct apn_app	*app;
	struct apn_rule *rule;

	rule = getApnRule();

	if ((rule == NULL) || (idx > getBinaryCount())) {
		return (wxEmptyString);
	}

	if (rule->rule.apncontext.application == NULL) {
		return (wxT("any"));
	}

	binary = wxEmptyString;
	app = seekAppByIndex(idx);

	if (app != NULL) {
		binary = wxString::From8BitData(app->name);
	}

	return (binary);
}

wxString
ContextFilterPolicy::getBinaryName(void) const
{
	return (PolicyUtils::listToString(getBinaryList()));
}

wxArrayString
ContextFilterPolicy::getBinaryList(void) const
{
	wxArrayString	 list;
	struct apn_rule *rule;

	rule = getApnRule();
	if (rule == NULL) {
		return (list);
	}

	list =  PolicyUtils::getAppList(rule->rule.apncontext.application);

	return (list);
}

bool
ContextFilterPolicy::addBinary(const wxString & binary)
{
	return addToBinaryListAt(getBinaryCount(), binary);
}

bool
ContextFilterPolicy::removeBinary(unsigned int index)
{
	bool		 result;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL) {
		return (false);
	}

	startChange();
	result = PolicyUtils::removeFromAppListAt(
	    &rule->rule.apncontext.application, index);
	setModified();
	finishChange();

	return (result);
}

bool
ContextFilterPolicy::isAny(void) const
{
	struct apn_rule *rule;

	rule = getApnRule();

	return !(rule && rule->rule.apncontext.application);
}

bool
ContextFilterPolicy::setHashValueNo(unsigned char csum[MAX_APN_HASH_LEN],
    unsigned int idx)
{
	bool		 rc;
	int		 len;
	struct apn_app	*app;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL) {
		return (false);
	}

	app = seekAppByIndex(idx);
	if (app == NULL) {
		return (false);
	}

	switch (app->hashtype) {
	case APN_HASH_SHA256:
	case APN_HASH_NONE:
		len = APN_HASH_SHA256_LEN;
		break;
	default:
		len = 0;
		rc  = false;
		break;
	}

	if (len > 0) {
		startChange();
		memcpy(app->hashvalue, csum, len);
		app->hashtype = APN_HASH_SHA256;
		setModified();
		finishChange();
		rc = true;
	}

	return (rc);
}

bool
ContextFilterPolicy::setHashValueString(const wxString & csumString,
    unsigned int idx)
{
	unsigned char csum[MAX_APN_HASH_LEN];

	if (csumString.IsEmpty()) {
		return (false);
	}

	memset(csum, 0, MAX_APN_HASH_LEN);
	PolicyUtils::stringToCsum(csumString, csum, MAX_APN_HASH_LEN);

	return (setHashValueNo(csum, idx));
}

bool
ContextFilterPolicy::getHashValueNo(unsigned int idx, unsigned char *csum,
    int len) const
{
	int		 needLength;
	struct apn_app	*app;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL) {
		return (false);
	}

	app = seekAppByIndex(idx);
	if (app == NULL) {
		return (false);
	}

	switch (app->hashtype) {
	case APN_HASH_SHA256:
		needLength = APN_HASH_SHA256_LEN;
		break;
	case APN_HASH_NONE:
		/* FALLTHROUGH */
	default:
		needLength = 0;
		return (false);
		break;
	}

	if (len >= needLength) {
		memcpy(csum, app->hashvalue, needLength);
		return (true);
	} else {
		return (false);
	}
}

wxString
ContextFilterPolicy::getHashValueName(unsigned int idx) const
{
	unsigned char	csum[MAX_APN_HASH_LEN];
	wxString	csumString;

	memset(csum, 0, MAX_APN_HASH_LEN);
	csumString = wxEmptyString;

	if (getHashValueNo(idx, csum, MAX_APN_HASH_LEN)) {
		PolicyUtils::csumToString(csum, MAX_APN_HASH_LEN,
		    csumString);
	}

	return (csumString);
}

wxArrayString
ContextFilterPolicy::getHashValueList(void) const
{
	unsigned int	idx;
	wxArrayString	hashValueList;

	/* This is not the best implementation, but it works. */
	for (idx = 0; idx < getBinaryCount(); idx++) {
		hashValueList.Add(getHashValueName(idx));
	}

	return (hashValueList);
}

/*
 * XXX ch: Move this to utils, 'cause used for AppPolicy, ContextFilterPolicy
 * XXX ch: and CsumCalcTask.
 */
wxArrayString
ContextFilterPolicy::calculateCurrentChecksums(void) const
{
	wxString	binary;
	wxString	csumString;
	wxArrayString	binaryList;
	wxArrayString	csumList;

	binaryList = getBinaryList();

	for (size_t i=0; i<binaryList.GetCount(); i++) {
		binary = binaryList.Item(i);

		if (!binary.IsEmpty() && binary != wxT("any")) {
			if (PolicyUtils::fileToCsum(binary, csumString)) {
				csumList.Add(csumString);
			} else {
				csumList.Add(wxT(""));
			}
		}
	}

	return (csumList);
}

struct apn_app *
ContextFilterPolicy::seekAppByIndex(unsigned int idx) const
{
	unsigned int	 seekIdx;
	struct apn_app	*app;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL) {
		return (NULL);
	}

	seekIdx = 0;
	app = rule->rule.apncontext.application;

	/* Search for app with given index. */
	while ((app != NULL) && (seekIdx != idx)) {
		seekIdx++;
		app = app->next;
	}

	return (app);
}

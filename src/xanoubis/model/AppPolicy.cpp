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

#include "AppPolicy.h"

#include "PolicyVisitor.h"
#include "PolicyUtils.h"

IMPLEMENT_CLASS(AppPolicy, Policy);

AppPolicy::AppPolicy(PolicyRuleSet *ruleSet, struct apn_rule *rule)
    : Policy(ruleSet, rule)
{
	filterList_.Clear();
}

AppPolicy::~AppPolicy(void)
{
	filterList_.DeleteContents(true);
	filterList_.Clear();
}

struct apn_app *
AppPolicy::createApnApp(void)
{
	struct apn_app *app;

	app = (struct apn_app *)calloc(1, sizeof(struct apn_app));
	if (app != NULL) {
		app->hashtype = APN_HASH_NONE;
	}

	return (app);
}

struct apn_rule *
AppPolicy::createApnRule(void)
{
	struct apn_rule *rule;

	rule = (struct apn_rule *)calloc(1, sizeof(struct apn_rule));

	return (rule);
}

void
AppPolicy::acceptOnFilter(PolicyVisitor &visitor)
{
	PolicyList::iterator i;

	if (visitor.shallBeenPropagated() == true) {
		for (i=filterList_.begin(); i != filterList_.end(); i++) {
			(*i)->accept(visitor);
		}
	}
}

size_t
AppPolicy::getFilterPolicyCount(void) const
{
	return (filterList_.GetCount());
}

unsigned int
AppPolicy::getBinaryCount(void) const
{
	unsigned int	 count;
	struct apn_app	*app;
	struct apn_rule *rule;

	count = 0;
	app   = NULL;
	rule  = getApnRule();

	if (rule != NULL) {
		app = rule->app;
		while (app != NULL) {
			count++;
			app = app->next;
		}
	}

	return (count);
}

bool
AppPolicy::setBinaryName(wxString binary, unsigned int idx)
{
	struct apn_app	*app;
	struct apn_rule *rule;

	rule = getApnRule();

	if ((rule == NULL) || (idx >= getBinaryCount()) || binary.IsEmpty()) {
		return (false);
	}

	if (binary.Cmp(wxT("any")) == 0) {
		startChange();
		apn_free_app(rule->app);
		rule->app = NULL;
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
	setModified();
	finishChange();

	return (true);
}

bool
AppPolicy::setBinaryList(wxArrayString binaryList)
{
	bool		 result;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL) {
		return (false);
	}

	startChange();
	result = PolicyUtils::setAppList(&(rule->app), binaryList);
	setModified();
	finishChange();

	return (result);
}

wxString
AppPolicy::getBinaryName(unsigned int idx) const
{
	wxString	 binary;
	struct apn_app	*app;
	struct apn_rule *rule;

	rule = getApnRule();

	if ((rule == NULL) || (idx > getBinaryCount())) {
		return (wxEmptyString);
	}

	if (rule->app == NULL) {
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
AppPolicy::getBinaryName(void) const
{
	return (PolicyUtils::listToString(getBinaryList()));
}

wxArrayString
AppPolicy::getBinaryList(void) const
{
	wxArrayString	 binaryList;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL) {
		return (binaryList);
	}

	binaryList = PolicyUtils::getAppList(rule->app);

	return (binaryList);
}

bool
AppPolicy::isAnyBlock(void) const
{
	struct apn_rule *rule;

	rule = getApnRule();

	if ((rule == NULL) || (rule->app == NULL)) {
		/*
		 * The any-block is defined as a empty app pointer
		 * (rule_->app == NULL). Thus returning 'true' on
		 * rule_ == NULL seems not to be correct, but what
		 * else would be handy?
		 */
		return (true);
	}

	return (false);
}

bool
AppPolicy::setHashTypeNo(int hashType, unsigned int idx)
{
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

	startChange();
	app->hashtype = hashType;
	setModified();
	finishChange();

	return (true);
}

bool
AppPolicy::setAllToHashTypeNo(int hashType)
{
	struct apn_app	*app;
	struct apn_rule *rule;

	rule = getApnRule();

	if ((rule == NULL) || (rule->app == NULL)) {
		return (false);
	}

	startChange();
	app = rule->app;
	while (app != NULL) {
		app->hashtype = hashType;
		app = app->next;
	}

	setModified();
	finishChange();

	return (true);
}

int
AppPolicy::getHashTypeNo(unsigned int idx) const
{
	struct apn_app	*app;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL) {
		return (-1);
	}

	app = seekAppByIndex(idx);
	if (app == NULL) {
		return (-1);
	}

	return (app->hashtype);
}

wxString
AppPolicy::getHashTypeName(unsigned int idx) const
{
	return (hashTypeToString(getHashTypeNo(idx)));
}

wxArrayString
AppPolicy::getHashTypeList(void) const
{
	wxArrayString	 hashTypeList;
	struct apn_app	*app;
	struct apn_rule *rule;

	hashTypeList.Clear();
	rule = getApnRule();

	if (rule == NULL) {
		return (hashTypeList);
	}

	if (rule->app == NULL) {
		hashTypeList.Add(hashTypeToString(APN_HASH_NONE));
	} else {
		app = rule->app;
		while (app != NULL) {
			hashTypeList.Add(hashTypeToString(app->hashtype));
			app = app->next;
		}
	}

	return (hashTypeList);
}

bool
AppPolicy::setHashValueNo(unsigned char csum[MAX_APN_HASH_LEN],
    unsigned int idx)
{
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

	startChange();
	switch (app->hashtype) {
	case APN_HASH_SHA256:
		len = APN_HASH_SHA256_LEN;
		break;
	case APN_HASH_NONE:
		/* FALLTHROUGH */
	default:
		len = 0;
		break;
	}

	memcpy(app->hashvalue, csum, len);
	setModified();
	finishChange();

	return (true);
}

bool
AppPolicy::setHashValueString(wxString csumString, unsigned int idx)
{
	unsigned char csum[MAX_APN_HASH_LEN];

	memset(csum, 0, MAX_APN_HASH_LEN);
	PolicyUtils::stringToCsum(csumString, csum, MAX_APN_HASH_LEN);

	return (setHashValueNo(csum, idx));
}

bool
AppPolicy::getHashValueNo(unsigned int idx, unsigned char *csum, int len) const
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
AppPolicy::getHashValueName(unsigned int idx) const
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
AppPolicy::getHashValueList(void) const
{
	unsigned int	idx;
	wxArrayString	hashValueList;

	/* This is not the best implementation, but it works. */
	for (idx = 0; idx > getBinaryCount(); idx++) {
		hashValueList.Add(getHashValueName(idx));
	}

	return (hashValueList);
}

struct apn_app *
AppPolicy::seekAppByIndex(unsigned int idx) const
{
	unsigned int	 seekIdx;
	struct apn_app	*app;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL) {
		return (NULL);
	}

	seekIdx = 0;
	app = rule->app;

	/* Search for app with given index. */
	while ((app != NULL) && (seekIdx != idx)) {
		seekIdx++;
		app = app->next;
	}

	return (app);
}

wxString
AppPolicy::hashTypeToString(int hashType) const
{
	wxString hashTypeString;

	switch (hashType) {
	case APN_HASH_SHA256:
		hashTypeString = wxT("SHA256");
		break;
	case APN_HASH_NONE:
		hashTypeString = wxT("NONE");
		break;
	default:
		hashTypeString = _("(unknown)");
		break;
	}

	return (hashTypeString);
}

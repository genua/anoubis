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
#include "PolicyRuleSet.h"

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
		app->subject.type = APN_CS_NONE;
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
	if (app->subject.type == APN_CS_NONE) {
		app->subject.type = APN_CS_CSUM;
		app->subject.value.csum = (u_int8_t *)calloc(ANOUBIS_CS_LEN,
		    sizeof(u_int8_t));
	}
	setModified();
	finishChange();

	return (true);
}

bool
AppPolicy::addToBinaryListAt(int pos, wxString name)
{
	bool		 result;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL) {
		return (false);
	}

	startChange();
	result = PolicyUtils::addToAppListAt(&(rule->app), pos, name);
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
AppPolicy::addBinary(const wxString & binary)
{
	return (addToBinaryListAt(getBinaryCount(), binary));
}

bool
AppPolicy::removeBinary(unsigned int index)
{
	bool		 result;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL) {
		return (false);
	}

	startChange();
	result = PolicyUtils::removeFromAppListAt(&(rule->app), index);
	setModified();
	finishChange();

	return (result);
}

bool
AppPolicy::removeBinary(const wxString &binary)
{
	wxArrayString	binaryList = getBinaryList();
	int	index;

	index = binaryList.Index(binary);
	if (index == wxNOT_FOUND) {
		return (false);
	}
	return removeBinary(index);
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
AppPolicy::setHashValueNo(unsigned char csum[MAX_APN_HASH_LEN],
    unsigned int idx)
{
	struct apn_app	*app;
	struct apn_rule *rule;
	u_int8_t	*csbuf;

	rule = getApnRule();
	if (rule == NULL)
		return false;

	app = seekAppByIndex(idx);
	if (app == NULL)
		return false;

	csbuf = (u_int8_t *)calloc(APN_HASH_SHA256_LEN, sizeof(u_int8_t));
	if (csbuf == NULL)
		return false;
	memcpy(csbuf, csum, APN_HASH_SHA256_LEN);

	startChange();
	apn_free_subject(&app->subject);
	app->subject.type = APN_CS_CSUM;
	app->subject.value.csum = csbuf;
	setModified();
	finishChange();

	return true;
}

bool
AppPolicy::setHashValueString(const wxString & csumString, unsigned int idx)
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
AppPolicy::getHashValueNo(unsigned int idx, unsigned char *csum, int len) const
{
	struct apn_app	*app;
	struct apn_rule *rule;

	rule = getApnRule();

	if (rule == NULL)
		return false;

	app = seekAppByIndex(idx);
	if (app == NULL)
		return false;

	if (app->subject.type != APN_CS_CSUM)
		return false;

	if (len < APN_HASH_SHA256_LEN)
		return false;
	memcpy(csum, app->subject.value.csum, APN_HASH_SHA256_LEN);

	return  true;
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
AppPolicy::calculateCurrentChecksums(void) const
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

int
AppPolicy::getSubjectTypeNo(unsigned int idx) const
{
	struct apn_app	*app = seekAppByIndex(idx);

	if (!app)
		return APN_CS_NONE;
	return app->subject.type;
}

wxString
AppPolicy::getSubjectName(unsigned int idx) const
{
	struct apn_app	*app = seekAppByIndex(idx);

	if (!app)
		return wxEmptyString;
	return PolicyUtils::getSubjectName(&app->subject);
}

bool
AppPolicy::setSubjectNone(unsigned int idx)
{
	struct apn_app *app = seekAppByIndex(idx);

	if (!app)
		return false;
	startChange();
	PolicyUtils::cleanSubject(&app->subject);
	setModified();
	finishChange();

	return true;
}

bool
AppPolicy::setSubjectSelf(unsigned int idx, bool selfSigned)
{
	struct apn_app *app = seekAppByIndex(idx);

	if (!app)
		return false;

	startChange();
	PolicyUtils::cleanSubject(&app->subject);

	if (selfSigned) {
		app->subject.type = APN_CS_KEY_SELF;
	} else {
		app->subject.type = APN_CS_UID_SELF;
	}

	setModified();
	finishChange();

	return true;
}

bool
AppPolicy::setSubjectUid(unsigned int idx, uid_t uid)
{
	struct apn_app *app = seekAppByIndex(idx);

	if (!app)
		return false;

	startChange();
	PolicyUtils::cleanSubject(&app->subject);

	app->subject.type = APN_CS_UID;
	app->subject.value.uid = uid;

	setModified();
	finishChange();

	return true;
}

bool
AppPolicy::setSubjectKey(unsigned int idx, wxString key)
{
	struct apn_app *app = seekAppByIndex(idx);

	if (!app)
		return false;

	startChange();
	PolicyUtils::cleanSubject(&app->subject);

	app->subject.type = APN_CS_KEY;
	app->subject.value.keyid = strdup(key.fn_str());

	setModified();
	finishChange();

	return true;
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

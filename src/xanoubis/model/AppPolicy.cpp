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
#include "PolicyRowProvider.h"
#include "PolicyRuleSet.h"

AppPolicy::AppPolicy(PolicyRuleSet *ruleSet, struct apn_rule *rule)
    : Policy(ruleSet, rule)
{
	this->rowProvider_ = new PolicyRowProvider(this);
}

AppPolicy::~AppPolicy(void)
{
	while (!filterList_.empty()) {
		FilterPolicy *policy = filterList_.back();
		filterList_.pop_back();
		delete policy;
	}

	delete rowProvider_;
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
	std::vector<FilterPolicy *>::const_iterator i;

	if (visitor.shallBeenPropagated() == true) {
		for (i=filterList_.begin(); i != filterList_.end(); i++) {
			(*i)->accept(visitor);
		}
	}
}

AnRowProvider *
AppPolicy::getRowProvider(void) const
{
	return (this->rowProvider_);
}

size_t
AppPolicy::getFilterPolicyCount(void) const
{
	return (filterList_.size());
}

FilterPolicy *
AppPolicy::getFilterPolicyAt(unsigned int index) const
{
	if (!filterList_.empty() && index < filterList_.size())
		return filterList_[index];
	else
		return 0;
}

int
AppPolicy::getIndexOfFilterPolicy(FilterPolicy *policy) const
{
	if (policy == 0)
		return (-1);

	for (unsigned int i = 0; i < filterList_.size(); i++) {
		if (filterList_[i] == policy)
			return (i);
	}

	return (-1);
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

void
AppPolicy::filterListAppend(FilterPolicy *policy)
{
	filterList_.push_back(policy);
	rowProvider_->sizeChangeEvent(filterList_.size());
}

void
AppPolicy::filterListPrepend(FilterPolicy *policy)
{
	filterList_.insert(filterList_.begin(), policy);
	rowProvider_->sizeChangeEvent(filterList_.size());
}

void
AppPolicy::sendPolicyChangeEvent(void)
{
	PolicyRuleSet *parent = getParentRuleSet();

	if (parent != 0) {
		int idx = parent->getIndexOfPolicy(this);

		if (idx != -1) {
			wxCommandEvent event(anEVT_ROW_UPDATE);
			event.SetInt(idx);
			event.SetExtraLong(idx);
			parent->getRowProvider()->ProcessEvent(event);
		}
	}
}

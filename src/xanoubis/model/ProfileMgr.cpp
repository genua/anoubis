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

#include "ProfileMgr.h"

ProfileMgr::ProfileMgr(void)
{
	currentProfile_ = PROFILE_NONE;
	ruleSetList_.clear();
}

ProfileMgr::~ProfileMgr(void)
{
	currentProfile_ = PROFILE_NONE;
	ruleSetList_.clear();
}

bool
ProfileMgr::acquireRuleSet(long id)
{
	std::list<RuleSetListEntry *>::iterator	it;
	bool					result;

	result = false;

	for (it=ruleSetList_.begin(); it!=ruleSetList_.end(); it++) {
		if ((*it)->getId() == id) {
			(*it)->acquire();
			result = true;
			break;
		}
	}

	return (result);
}

bool
ProfileMgr::releaseRuleSet(long id)
{
	std::list<RuleSetListEntry *>::iterator	it;
	bool					result;

	result = false;

	for (it=ruleSetList_.begin(); it!=ruleSetList_.end(); it++) {
		if ((*it)->getId() == id) {
			(*it)->release();
			result = true;
			break;
		}
	}

	return (result);
}

PolicyRuleSet *
ProfileMgr::getRuleSet(long id) const
{
	std::list<RuleSetListEntry *>::const_iterator	 it;
	PolicyRuleSet					*result;

	result = NULL;

	for (it=ruleSetList_.begin(); it!=ruleSetList_.end(); it++) {
		if ((*it)->getId() == id) {
			result = ((*it)->getRuleSet());
			break;
		}
	}

	return (result);
}

ProfileMgr::profile_t
ProfileMgr::getProfile(void) const
{
	return (currentProfile_);
}

void
ProfileMgr::setProfile(profile_t profile)
{
	currentProfile_ = profile;
}

long
ProfileMgr::getUserRsId(profile_t profile) const
{
	return (seekId(profile, false));
}

long
ProfileMgr::getUserRsId(void) const
{
	return (getUserRsId(currentProfile_));
}

long
ProfileMgr::getAdminRsId(profile_t profile) const
{
	return (seekId(profile, true));
}

long
ProfileMgr::getAdminRsId(void) const
{
	return (getAdminRsId(currentProfile_));
}


bool
ProfileMgr::storeRuleSet(profile_t profile, PolicyRuleSet* ruleSet)
{
	std::list<RuleSetListEntry *>::iterator	 it;
	RuleSetListEntry			*entry;

	/*
	 * If the ruleSet is already stored in our list, we just re-map
	 * it with the given profile.
	 */
	for (it=ruleSetList_.begin(); it!=ruleSetList_.end(); it++) {
		if ((*it)->getId() == ruleSet->getId()) {
			(*it)->setProfile(profile);
			return (true);
		}
	}

	/* Clean list from unused rule sets. */
	wipeList();

	/*
	 * Ok, the rule set is a new one. If the profile is not NONE,
	 * we re-map it's assigned rule set to profile NONE and put
	 * the new rule set into our list.
	 */
	if (profile != PROFILE_NONE) {
		for (it=ruleSetList_.begin(); it!=ruleSetList_.end(); it++) {
			if (((*it)->getProfile() == profile) &&
			    ((*it)->isAdmin()    == ruleSet->isAdmin())) {
				(*it)->setProfile(PROFILE_NONE);
				break;
			}
		}
	}

	entry = new RuleSetListEntry(profile, ruleSet);
	if (entry == NULL) {
		return (false);
	}

	ruleSetList_.push_back(entry);
	return (true);
}

bool
ProfileMgr::storeRuleSet(PolicyRuleSet* ruleSet)
{
	return (storeRuleSet(currentProfile_, ruleSet));
}

void
ProfileMgr::wipeList(void)
{
	std::list<RuleSetListEntry *>::iterator	it;

	it = ruleSetList_.begin();
	while (it!=ruleSetList_.end()) {
		if (((*it)->getProfile() == PROFILE_NONE) &&
		    !(*it)->isUsed()) {
			/* erase() returns iterator to next element */
			it = ruleSetList_.erase(it);
		} else {
			it++;
		}
	}
}

long
ProfileMgr::seekId(profile_t profile, bool isAdmin) const
{
	std::list<RuleSetListEntry *>::const_iterator	it;
	long						result;

	result = -1;

	for (it=ruleSetList_.begin(); it!=ruleSetList_.end(); it++) {
		if (((*it)->getProfile() == profile) &&
		    ((*it)->isAdmin()    == isAdmin)    ) {
			result = (*it)->getId();
			break;
		}
	}

	return (result);
}

/*
 * Methods of the nested RuleSetListEntry class
 */

ProfileMgr::RuleSetListEntry::RuleSetListEntry(profile_t profile,
    PolicyRuleSet *policyRuleSet)
{
	referenceCount_ = 0;
	profile_	= profile;
	ruleSet_	= policyRuleSet;
}

ProfileMgr::RuleSetListEntry::~RuleSetListEntry(void)
{
	delete ruleSet_;
	ruleSet_ = NULL;
	referenceCount_ = 0;
	profile_ = PROFILE_NONE;
}

void
ProfileMgr::RuleSetListEntry::acquire(void)
{
	referenceCount_ += 1;
}

void
ProfileMgr::RuleSetListEntry::release(void)
{
	if (referenceCount_ > 0) {
		referenceCount_ -= 1;
	}
}

long
ProfileMgr::RuleSetListEntry::getId(void) const
{
	return (ruleSet_->getId());
}

bool
ProfileMgr::RuleSetListEntry::isAdmin(void) const
{
	return (ruleSet_->isAdmin());
}

bool
ProfileMgr::RuleSetListEntry::isUsed(void) const
{
	return ((referenceCount_ > 0) ? true : false);
}

ProfileMgr::profile_t
ProfileMgr::RuleSetListEntry::getProfile(void) const
{
	return (profile_);
}

void
ProfileMgr::RuleSetListEntry::setProfile(profile_t profile)
{
	profile_ = profile;
}

PolicyRuleSet *
ProfileMgr::RuleSetListEntry::getRuleSet(void) const
{
	return (ruleSet_);
}

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

#include "ProfileCtrl.h"

#include "Singleton.cpp"

ProfileCtrl::~ProfileCtrl(void)
{
}

ProfileCtrl *
ProfileCtrl::getInstance(void)
{
	return (Singleton<ProfileCtrl>::instance());
}

ProfileMgr::profile_t
ProfileCtrl::getProfile(void) const
{
	return (profileManager_.getProfile());
}

wxString
ProfileCtrl::getProfileName(void) const
{
	return (profileManager_.getProfileName());
}

void
ProfileCtrl::switchProfile(ProfileMgr::profile_t profile)
{
	profileManager_.setProfile(profile);
	/*
	 * XXX: ch ToDo
	 * If this is going to be used, we have to
	 * activateRuleSet(profileManager_.getRuleSet());
	 */
}

long
ProfileCtrl::getUserId(ProfileMgr::profile_t profile) const
{
	return (seekId(profile, false, geteuid()));
}

long
ProfileCtrl::getUserId(void) const
{
	return (getUserId(profileManager_.getProfile()));
}

long
ProfileCtrl::getAdminId(ProfileMgr::profile_t profile, uid_t uid) const
{
	return (seekId(profile, true, uid));
}

long
ProfileCtrl::getAdminId(uid_t uid) const
{
	return (getAdminId(profileManager_.getProfile(), uid));
}

bool
ProfileCtrl::lockToShow(long id, void *caller)
{
	bool result;

	result = false;

	result = profileManager_.acquireRuleSet(id);
	if (result == false) {
		/* No rule set with the given id was stored. */
		return (false);
	}

	showList_.insert(std::pair<long, void *>(id, caller));

	return (result);
}

bool
ProfileCtrl::unlockFromShow(long id, void *caller)
{
	std::multimap<long, void *>::iterator	it;
	std::multimap<long, void *>::iterator	itup;
	std::multimap<long, void *>::iterator	itlow;
	bool					foundCaller;

	if (showList_.count(id) == 0) {
		/* No rule set with this id was locked. */
		return (false);
	}

	foundCaller = false;
	itlow = showList_.lower_bound(id);
	itup  = showList_.upper_bound(id);
	for (it=itlow; it!=itup; it++) {
		if ((*it).second == caller) {
			/*
			 * The break is important here, because 'it' already
			 * points to the matching pair of 'id' and 'caller'.
			 * Thus breaking here keeps the value of 'it', so we
			 * can use it to erase it.
			 */
			foundCaller = true;
			break;
		}
	}
	if (!foundCaller) {
		/* This rule set wasn't locked for showing by caller. */
		return (false);
	}

	if (profileManager_.releaseRuleSet(id) == false) {
		/* No rule set with the given id was stored. */
		return (false);
	}

	showList_.erase(it);
	return (true);
}

bool
ProfileCtrl::store(ProfileMgr::profile_t profile, PolicyRuleSet *ruleSet)
{
	return (profileManager_.storeRuleSet(profile, ruleSet));
}

bool
ProfileCtrl::store(PolicyRuleSet *ruleSet)
{
	return (profileManager_.storeRuleSet(ruleSet));
}

PolicyRuleSet *
ProfileCtrl::getRuleSetToShow(long id, void *caller) const
{
	std::multimap<long, void *>::const_iterator	it;
	std::multimap<long, void *>::const_iterator	itup;
	std::multimap<long, void *>::const_iterator	itlow;
	bool					foundCaller;

	if (showList_.count(id) == 0) {
		/* No rule set with this id was locked. */
		return (NULL);
	}

	foundCaller = false;
	itlow = showList_.lower_bound(id);
	itup  = showList_.upper_bound(id);
	for (it=itlow; it!=itup; it++) {
		if ((*it).second == caller) {
			foundCaller = true;
			break;
		}
	}
	if (!foundCaller) {
		/* This rule set wasn't locked for showing by caller. */
		return (NULL);
	}

	return (profileManager_.getRuleSet(id));
}

long
ProfileCtrl::seekId(ProfileMgr::profile_t profile, bool isAdmin,
    uid_t uid) const
{
	if (isAdmin) {
		return (profileManager_.getAdminRsId(profile, uid));
	} else {
		return (profileManager_.getUserRsId(profile));
	}
}

ProfileCtrl::ProfileCtrl(void) : Singleton<ProfileCtrl>()
{
	showList_.clear();
}

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

#ifndef _PROFILECTRL_H_
#define _PROFILECTRL_H_

#include <map>
#include <wx/string.h>

#include "Singleton.h"
#include "ProfileMgr.h"

/**
 * Profile controller.
 *
 * With this class, we control the profile manager. It's, implemented as
 * singleton pattern, available at any place. Don't use the profile manager
 * directly.
 * If we want to implement something like "access control" or an exclusive
 * write-lock, do it here.
 */
class ProfileCtrl : public Singleton<ProfileCtrl>
{
	public:
		/**
		 * Destructor of ProfileCtrl.
		 * This will clean up the whole mess. It needs to be public,
		 * but you are not allowed to use it (delete).
		 * @param None.
		 * @return Nothing.
		 */
		~ProfileCtrl(void);

		/**
		 * Get object.
		 * This returns the (only) object of this class
		 * (see singleton pattern).
		 * @param None.
		 * @return It self.
		 */
		static ProfileCtrl *getInstance(void);

		/**
		 * Get profile.
		 * Retrieve the current profile. This is delegated to the
		 * profile manager.
		 * @param None.
		 * @return The current profile.
		 * @see ProfileMgr::getProfile()
		 */
		ProfileMgr::profile_t getProfile(void) const;

		/**
		 * Get name of profile.
		 * Get the readable name of the current profile.
		 * @param None.
		 * @return The name of the current profile.
		 * @see getProfile()
		 */
		wxString getProfileName(void) const;

		/**
		 * Switch to new profile.
		 * Change the current profile to the given one.
		 * This is delegated to the profile manager.
		 * @param[in] 1st The new profile.
		 * @return Nothing.
		 * @see ProfileMgr::switchProfile()
		 */
		void switchProfile(ProfileMgr::profile_t);

		/**
		 * Get user rule set id.
		 * Get the id of the current user rule set mapped to the
		 * given profile.
		 * @param[in] 1st The profile in question.
		 * @return The id of the rule set or -1 if nothing found.
		 */
		long getUserId(ProfileMgr::profile_t) const;

		/**
		 * Get user rule set id.
		 * Get the id of the current user rule set mapped to the
		 * current profile.
		 * @param None.
		 * @return The id of the rule set or -1 if nothing found.
		 * @overload
		 */
		long getUserId(void) const;

		/**
		 * Get admin rule set id.
		 * Get the id of the current admin rule set mapped to the
		 * given profile and the specified user.
		 * @param[in] 1st The profile in question.
		 * @param[in] 2nd The uid of the user in question.
		 * @return The id of the rule set or -1 if nothing found.
		 */
		long getAdminId(ProfileMgr::profile_t, uid_t) const;

		/**
		 * Get admin rule set id.
		 * Get the id of the current admin rule set mapped to the
		 * current profile and the specified user.
		 * @param[in] 1st The uid of the user in question..
		 * @return The id of the rule set or -1 if nothing found.
		 * @overload
		 */
		long getAdminId(uid_t) const;

		/**
		 * Lock rule set for showing.
		 *
		 * This is mandatory to get access to a rule set via
		 * getRuleSetToShow(). You can lock the same rule set more
		 * than once. Different callers may lock the same rule set
		 * for the same time, so you are not allowed to modify the
		 * locked rule set. As caller just pass the 'this' value.
		 *
		 * @param[in] 1st The id of the rule set in question.
		 * @param[in] 2nd The caller, this is you.
		 * @return True on success.
		 *
		 * @see unlockFromShow()
		 * @see getRuleSetToShow()
		 */
		bool lockToShow(long, void *);

		/**
		 * Unlock rule set from showing.
		 *
		 * After the call of this method, you must not use the
		 * rule set or it's policies any more. (They may been
		 * deleted if the last lock is released).
		 *
		 * @param[in] 1st The id of the rule set in question.
		 * @param[in] 2nd The caller, this is you.
		 * @return True on success.
		 *
		 * @see lockToShow()
		 */
		bool unlockFromShow(long, void *);

		/**
		 * Store rule set.
		 *
		 * Store a given rule set to the (also given profile).
		 * This is directly delegated to the concerning profile
		 * manager.
		 *
		 * @param[in] 1st The profile in question.
		 * @param[in] 2nd The rule set to store.
		 * @return True on success.
		 *
		 * @see ProfileMgr::store()
		 */
		bool store(ProfileMgr::profile_t, PolicyRuleSet *);

		/**
		 * Store rule set.
		 *
		 * Store a given rule set to the current profile.
		 * This is directly delegated to the concerning profile
		 * manager.
		 *
		 * @param[in] 1st The rule set to store.
		 * @return True on success.
		 *
		 * @see ProfileMgr::store()
		 * @overload
		 */
		bool store(PolicyRuleSet *);

		/**
		 * Get rule set for showing.
		 *
		 * Get access to the rule set with the given id. The rule set
		 * must not be modified and it's mandatory to previously lock
		 * the concerning rule set. As caller just pass the 'this'
		 * value.
		 *
		 * @param[in] 1st The id of the rule set in question.
		 * @param[in] 2nd The caller, this is you.
		 * @return The requested rule set or NULL if none was found.
		 *
		 * @see lockToShow()
		 */
		PolicyRuleSet *getRuleSetToShow(long, void*) const;

	protected:
		/**
		 * Constructor of ProfileCtrl.
		 */
		ProfileCtrl(void);

	private:
		ProfileMgr			profileManager_;
		std::multimap<long, void *>	showList_;

		/**
		 * Seek for rule set id.
		 * Seek for a rule set mapped to the given profile and
		 * (not) matching user/admin status. This is delegated
		 * to the concerning profile manager.
		 *
		 * @param[in] 1st The profile in question.
		 * @param[in] 2nd The answer of isAdmin() must match this.
		 * @param[in] 3rd The concerned user by his id.
		 * @return The id of the rule set or -1 if none was found.
		 */
		long seekId(ProfileMgr::profile_t, bool, uid_t) const;

	friend class Singleton<ProfileCtrl>;
};

#endif	/* _PROFILECTRL_H_ */

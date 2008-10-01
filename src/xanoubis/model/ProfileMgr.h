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

#ifndef _PROFILEMGR_H_
#define _PROFILEMGR_H_

/**
 * We use a std::list here, because it's not possible to nest
 * the list-elements of wxList as private sub-class.
 */
#include <list>

#include "PolicyRuleSet.h"

/**
 * This class is for managing profiles.
 * You can store as many rule sets as you want to profile NONE.
 * To all other profiles only two rule sets (one for each user
 * or admin set) are storeable. If a rule set already exists, it's
 * remaped to profile NONE.
 * Be aware that unused/not-acquired rule sets mapped to profile
 * NONE are deleted during a store operation.
 */
class ProfileMgr
{
	public:
		/** These profiles are available. */
		enum profile_t {
			PROFILE_NONE = -1,
			PROFILE_HIGH,
			PROFILE_MEDIUM,
			PROFILE_ADMIN
		};

		/**
		 * Constructor of ProfileMgr.
		 */
		ProfileMgr(void);

		/**
		 * Destructor of ProfileMgr.
		 */
		~ProfileMgr(void);

		/**
		 * Acquire a rule set.
		 * This increments the reference counter of a given rule set.
		 * @param[in] 1st The id of rule set to aquire.
		 * @return True on success.
		 * @see releaseRuleSet()
		 */
		bool acquireRuleSet(long);

		/**
		 * Release a rule set.
		 * This decrements the reference counter of a given rule set.
		 * @param[in] 1st The id of rule set to release.
		 * @return True on success.
		 * @see acquireRuleSet()
		 */
		bool releaseRuleSet(long);

		/**
		 * Get a rule set.
		 * Get the rule set with the given id.
		 * @param[in] 1st The id of requested rule set.
		 * @return The rule set or NULL if no rule set was found.
		 */
		PolicyRuleSet *getRuleSet(long) const;

		/**
		 * Get current profile.
		 * @param None.
		 * @return The current profile.
		 * @see setProfile()
		 */
		profile_t getProfile(void) const;

		/**
		 * Get the name of the current profile.
		 * Get the readable name of the current profile.
		 * @param None.
		 * @return The name of the current profile.
		 * @see getProfile()
		 */
		wxString getProfileName(void) const;

		/**
		 * Set current profile.
		 * @param[in] 1st The new profile.
		 * @return Nothing.
		 * @see getProfile()
		 */
		void setProfile(profile_t);

		/**
		 * Get user rule set id.
		 * Get the id of the rule set assigned to the given
		 * profile and beeing the user rule set.
		 * @param[in] 1st The profile in question.
		 * @return The id of the rule set or -1 if nothing found.
		 */
		long getUserRsId(profile_t) const;

		/**
		 * Get user rule set id.
		 * Get the id of the rule set assigned to the current
		 * profile and beeing the user rule set.
		 * @param None.
		 * @return The id of the rule set or -1 if nothing found.
		 * @override
		 */
		long getUserRsId(void) const;

		/**
		 * Get admin rule set id.
		 * Get the id of the rule set assigned to the given profile,
		 * the user by it's id and beeing the admin rule set.
		 * @param[in] 1st The profile in question.
		 * @param[in] 2nd The uid of the user.
		 * @return The id of the rule set or -1 if nothing found.
		 */
		long getAdminRsId(profile_t, uid_t) const;

		/**
		 * Get admin rule set id.
		 * Get the id of the rule set assigned to the current profile,
		 * the user by it's id and beeing the admin rule set.
		 * @param[in] 1st The uid of the user.
		 * @return The id of the rule set or -1 if nothing found.
		 * @override
		 */
		long getAdminRsId(uid_t) const;

		/**
		 * Store rule set.
		 * Store a given rule set to the (also given) profile.
		 *
		 * If the rule set is already stored, but mapped to a different
		 * profile, it's re-mapped to the given profile.
		 *
		 * If the rule set is new and an other rule set is mapped to
		 * given profile, the other rule set is mapped to profile NONE
		 * and the given rule set is mapped to the profile.
		 *
		 * If the profile is NONE, the rule set is just stored.
		 *
		 * This method does _not_ acquire the just stored rule set.
		 *
		 * During every storage operation, the list will been wiped
		 * and rule sets mapped to profile NONE and not used (not
		 * acquired) are deleted.
		 *
		 * @param[in] 1st The profile in question.
		 * @param[in] 2nd The rule set to store.
		 * @return True on success.
		 */
		bool storeRuleSet(profile_t, PolicyRuleSet *);

		/**
		 * Store rule set.
		 * Store the given rule set to the current profile.
		 * @param[in] 1st The rule set to store.
		 * @return True on success.
		 * @override
		 */
		bool storeRuleSet(PolicyRuleSet *);

	private:
		/**
		 * Nested subclass to store meta information about rule sets.
		 * It's been organized within a std::list.
		 */
		class RuleSetListEntry
		{
			public:
				/**
				 * Constructor of RuleSetListEntry.
				 * @param[in] 1st The mapped profile.
				 * @param[in] 2nd The stored rule set.
				 */
				RuleSetListEntry(profile_t, PolicyRuleSet *);

				/**
				 * Destructor of RuleSetListEntry.
				 * @param None.
				 */
				~RuleSetListEntry(void);

				/**
				 * Increment the reference count.
				 * @param None.
				 * @return Nothing.
				 */
				void acquire(void);

				/**
				 * Decrement the reference count.
				 * @param None.
				 * @return Nothing.
				 */
				void release(void);

				/**
				 * Delegate getId() to the assigned rule set.
				 * @param None.
				 * @return The id of the stored rule set.
				 */
				long getId(void) const;

				/**
				 * Delegate isAdmin() to the assigned rule set.
				 * @param None.
				 * @return True if it's an admin rule set.
				 */
				bool isAdmin(void) const;

				/**
				 * Answers the question weather the assigned
				 * rule set belongs to the given user.
				 * @param[in] 1st The uid of the user.
				 * @return True if the id's are matching.
				 */
				bool belongsTo(uid_t) const;

				/**
				 * Get use-status.
				 * @param None.
				 * @return True if reference count > 0
				 */
				bool isUsed(void) const;

				/**
				 * Get the profile for the rule set.
				 * @param None.
				 * @return Mapped profile.
				 * @see setProfile()
				 */
				profile_t getProfile(void) const;

				/**
				 * Remap the rule set to the new profile.
				 * @param[in] 1st The new profile.
				 * @return Nothing.
				 * @see getProfile()
				 */
				void setProfile(profile_t);

				/**
				 * Get the assigned rule set.
				 * @param None.
				 * @return The rule set.
				 */
				PolicyRuleSet *getRuleSet(void) const;

			private:
				unsigned int	 referenceCount_;
				profile_t	 profile_;
				PolicyRuleSet	*ruleSet_;
		};

		profile_t			currentProfile_;
		std::list<RuleSetListEntry *>	ruleSetList_;

		/**
		 * Wipe rule set list.
		 * This method is called during a store operation. It searches
		 * the list for unused/not-acquired rule sets mapped to the
		 * profile NONE and deletes the found ones.
		 * @param None.
		 * @return None.
		 * @see storeRuleSet()
		 */
		void wipeList(void);

		/**
		 * Seek for rule set id.
		 * This method is used to get the id of a rule set mapped to the
		 * given profile, the user by it's id and beeing admin rule set
		 * according to the given flag.
		 * @param[in] 1st The profile to search in.
		 * @param[in] 2nd The flag if the admin rule set is requested.
		 * @param[in] 3rd The user in question.
		 * @return The id of the rule set or -1 if none was found.
		 * @see getUserRsId()
		 * @see getAdminRsId()
		 */
		long seekId(profile_t, bool, uid_t) const;
};

#endif	/* _PROFILEMGR_H_ */

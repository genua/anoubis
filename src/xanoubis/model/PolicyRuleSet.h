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

#ifndef _POLICYRULESET_H_
#define _POLICYRULESET_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>
#include <wx/string.h>

#include "EscalationNotify.h"
#include "Subject.h"

#include "libapn.h"
#include "Policy.h"
#include "AppPolicy.h"
#include "FilterPolicy.h"
#include "AlfAppPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "AlfFilterPolicy.h"
#include "ContextAppPolicy.h"
#include "ContextFilterPolicy.h"
#include "DefaultFilterPolicy.h"
#include "SbAccessFilterPolicy.h"
#include "SbAppPolicy.h"
#include "SfsAppPolicy.h"
#include "SfsFilterPolicy.h"

/**
 */
class PolicyRuleSet : public Subject
{
	public:
		/**
		 * Constructor of PolicyRuleSet.
		 * @param[in] 1st Priority of ruleset (admin or user ruleset).
		 * @param[in] 2nd UserId of ruleset.
		 * @param[in] 3rd Native apn ruleset.
		 * @return Nothing.
		 */
		PolicyRuleSet(int, uid_t, struct apn_ruleset *);

		/**
		 * Constructor of PolicyRuleSet.
		 * @param[in] 1st Priority of ruleset (admin or user ruleset).
		 * @param[in] 2nd UserId of ruleset.
		 * @param[in] 3rd Filename where to read policies from.
		 * @return Nothing.
		 */
		PolicyRuleSet(int, uid_t, const wxString &);

		/**
		 * Destructor of PolicyRuleSet.
		 */
		~PolicyRuleSet(void);

		/**
		 * Locks the ruleset.
		 *
		 * You are able to replace an active user- or admin-policy. If
		 * a ruleset is locked, it is not deleted by PolicyCtrl, but
		 * is not addressable anymore (not fetchable by
		 * PolicyCtrl::getUserId(), PolicyCtrl::getAdminId()). This
		 * is useful, if you need a reference to an already replaced
		 * ruleset.
		 *
		 * To release the lock, call unlock(). To test the lock-state,
		 * call isLocked().
		 *
		 * @see unlock()
		 * @see isLocked()
		 */
		void lock(void);

		/**
		 * Releases the ruleset-lock again.
		 *
		 * When a ruleset is unlocked again, PolicyCtrl is allowed to
		 * destroy the ruleset.
		 */
		void unlock(void);

		/**
		 * Tests whether the ruleset is locked.
		 *
		 * If the ruleset is locked, the PolicyCtrl is not allowed to
		 * destroy the instance.
		 *
		 * @return true if locked, false otherwise.
		 */
		bool isLocked(void) const;

		/**
		 * Is ruleset of admin rules?
		 * @param None.
		 * @return True if it's the admin ruleset.
		 * @see getPriority()
		 */
		bool isAdmin(void) const;

		/**
		 * Does the ruleset contails errors? Is this ruleset valid?
		 * @param None.
		 * @return True on syntax and/or parser errors.
		 */
		bool hasErrors(void) const;

		/**
		 * Return true if rule editing of this escalation
		 * is supported by the ruleset
		 * @param The escalation
		 * @return true if editing is supported.
		 */
		void addRuleInformation(EscalationNotify *);

		/**
		 * Get PolicyRuleSet id.
		 * Use this to fetch the id of this ruleset. This id is used
		 * to identify this ruleset and unique during runtime.\n
		 * It's set by the constructor using wxNewId().
		 * @param None.
		 * @return The id.
		 */
		long getRuleSetId(void) const;

		/**
		 * Get the native apn_ruleset.
		 * Don't use this method! If you can't avoid it, do it with
		 * care! Main purpose of this method is to get apn_ruleset
		 * e.g by VersionCtrl without detour of tmp file.
		 * @param None.
		 * @return The native apn_ruleset structure.
		 */
		struct apn_ruleset *getApnRuleSet(void) const;

		/**
		 * Get uid of user this ruleset belongs to.
		 * @param None.
		 * @return The uid of user this ruleset belongs to.
		 */
		uid_t getUid(void) const;

		/**
		 * Get priority of this ruleset.
		 * If the priority is '0', this is the admin ruleset.
		 * @param None.
		 * @return The priority of this ruleset.
		 * @see isAdmin()
		 */
		int getPriority(void) const;

		/**
		 * Set origin.
		 * Overwrite the origin set during construction.
		 * <b>!!! USE WITH CARE !!!</b>
		 * @param[in] 1st The new origin strring.
		 * @return Nothing.
		 */
		void setOrigin(wxString);

		/**
		 * Get the origin of this ruleset.
		 * @param None.
		 * @return The origin string.
		 */
		wxString getOrigin(void) const;

		/**
		 * Mark this ruleset as comming from the daemon.
		 * @param None.
		 * @return Nothing.
		 */
		void setDaemonRuleSet(void);

		/**
		 * Was this rulset fetched from
		 * @param None.
		 * @return True if rulset comes from daemon.
		 */
		bool isDaemonRuleSet(void) const;

		/**
		 * Accept a visitor.
		 * This is the starting point of visiton the whole
		 * policy structure.
		 * @param[in] 1st The visitor.
		 * @return Nothing.
		 */
		void accept(PolicyVisitor &);

		/**
		 * Ruleset to string.
		 * This method returns the ruleset as string. It's
		 * implementation is quite ugly, because libapn does not
		 * provide a suitable method. Thus the long way round
		 * by using tmp file was coded.
		 * @param policy Filled with the serialization-result. If the
		 *               serialization failed, the reference is left
		 *               untouched.
		 * @return True on success
		 */
		bool toString(wxString &) const;

		/**
		 * Write ruleset to file.
		 * @param[in] 1st The file to write the ruleset to.
		 * @return True on success.
		 */
		bool exportToFile(const wxString &) const;

		/**
		 * Set ruleset modified flag.
		 * Mark the ruleset as been modified.
		 * @param None.
		 * @return Nothing.
		 */
		void setModified(void);

		/**
		 * Clear ruleset modified flag.
		 * This will reset the isModified flag of the ruleset.
		 * @param None.
		 * @return Nothing.
		 */
		void clearModified (void);

		/**
		 * Is this ruleset modified?
		 * @param None.
		 * @return True if ruleset was modified.
		 */
		bool isModified(void) const;

		/**
		 * Set the in-progress flag. This is set while a task
		 * to send the policy to the daemon is in progress.
		 * @param None.
		 * @return Nothing.
		 */
		void setInProgress(void);

		/**
		 * Clear the in-progress flag.
		 * @param None.
		 * @return Nothing.
		 */
		void clearInProgress(void);

		/**
		 * Return true if the in-progress flag is set.
		 * @param None.
		 * @return The current value of the in-progress flag.
		 */
		bool isInProgress(void) const;

		/**
		 * Refresh the policy ruleset after the underlying
		 * apn_ruleset has been modified.
		 * @param None.
		 * @return None.
		 */
		void refresh(void);

		/**
		 * Returns the AnRowProvider assigned to the ruleset.
		 *
		 * Use this row-provider if you want to display the ruleset.
		 * The row-provider returns the app-policies of the ruleset.
		 * Thus displaying the ruleset means to list the app-policies.
		 *
		 * @return The row-provider of the ruleset
		 */
		AnRowProvider *getRowProvider(void) const;

		/**
		 * Get the number of application policies.
		 * @param None.
		 * @return The number of application policies.
		 */
		size_t getAppPolicyCount(void) const;

		/**
		 * Returns the application policy at the given index.
		 *
		 * If the index is out of range, NULL is returned.
		 *
		 * @param[in] 1st The requested index
		 * @return Application policy at the given index or NULL if the
		 *         index is out of range.
		 */
		AppPolicy *getPolicyAt(unsigned int) const;

		/**
		 * Returns the index of the given policy.
		 *
		 * @param[in] 1st The policy you are searching for
		 * @return The index of the policy. If the policy is not part
		 *         of the ruleset, -1 is returned.
		 */
		int getIndexOfPolicy(AppPolicy *) const;

		/**
		 * Search for alf application policy.
		 * @param[in] 1st The application name to search for.
		 * @return Found policy or NULL.
		 */
		AlfAppPolicy *searchAlfAppPolicy(wxString) const;

		/**
		 * Search for context application policy.
		 * @param[in] 1st The application name to search for.
		 * @return Found policy or NULL.
		 */
		ContextAppPolicy *searchContextAppPolicy(wxString) const;

		/**
		 * Search for sandbox application policy.
		 * @param[in] 1st The application name to search for.
		 * @return Found policy or NULL.
		 */
		SbAppPolicy *searchSandboxAppPolicy(wxString) const;

		/**
		 * Create new policy and insert it.
		 * This leads to a re-creation of the policy structure.
		 * @param[in] 1st Apn type of policy to create.
		 * @param[in] 2nd Insert before this id (may be zero).
		 * @param[in] 3rd Parent for new filter policy (may be NULL).
		 * @return Suggestion for new index (-1 on error).
		 */
		long createPolicy(unsigned int, unsigned int, AppPolicy *);

		/**
		 * Add a given app policy on top.
		 * Insert the apn_rule of the given policy to the apn ruleset
		 * and insert the policy to the list of app policys.
		 * In both cases do an 'insert-head'.
		 * @param[in] 1st The app policy policy to insert.
		 * @return True on success.
		 */
		bool prependAppPolicy(AppPolicy *);

		/**
		 * Create answer policy.
		 * Some answers of escalation have to be done as new rules in
		 * the current ruleset. This method will create those rules.
		 * @param[in] 1st The escalation in question.
		 * @return True on sucess
		 */
		bool createAnswerPolicy(EscalationNotify *);

	private:
		int			 refCnt_;
		int			 priority_;
		wxString		 origin_;
		long			 ruleSetId_;
		uid_t			 uid_;
		bool			 hasErrors_;
		bool			 isModified_;
		bool			 isInProgress_;
		bool			 isDaemonRuleSet_;
		PolicyRowProvider	*rowProvider_;
		struct apn_ruleset	*ruleSet_;
		std::vector<Policy *>	 alfList_;
		std::vector<Policy *>	 sfsList_;
		std::vector<Policy *>	 ctxList_;
		std::vector<Policy *>	 sbList_;

		/**
		 * Clean the queues
		 */
		void clean(void);

		/**
		 * Create ruleset from file.
		 * @param[in] 1st Name of file to be parsed.
		 */
		void create(wxString);

		/**
		 * Create policies from parsed ruleset.
		 */
		void create(struct apn_ruleset *);

		struct apn_rule *assembleAlfPolicy(AlfFilterPolicy *,
		   EscalationNotify *);

		void status(const wxString &);
};

#endif	/* _POLICYRULESET_H_ */

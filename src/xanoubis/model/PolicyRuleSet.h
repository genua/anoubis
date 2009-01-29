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
		 * @param[in] 4th If true check access permissions.
		 * @return Nothing.
		 */
		PolicyRuleSet(int, uid_t, const wxString &, bool);

		/**
		 * Destructor of PolicyRuleSet.
		 */
		~PolicyRuleSet(void);

		/**
		 * Locks the ruleset.
		 *
		 * You are able to replace an active user- or admin-policy. If
		 * a ruleset is locked, it is not deleted by ProfileCtrl, but
		 * is not addressable anymore (not fetchable by
		 * ProfileCtrl::getUserId(), ProfileCtrl::getAdminId()). This
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
		 * When a ruleset is unlocked again, ProfileCtrl is allowed to
		 * destroy the ruleset.
		 */
		void unlock(void);

		/**
		 * Tests whether the ruleset is locked.
		 *
		 * If the ruleset is locked, the ProfileCtrl is not allowed to
		 * destroy the instance.
		 *
		 * @return true if locked, false otherwise.
		 */
		bool isLocked(void) const;

		/**
		 * Is ruleset empty?
		 * @param None.
		 * @return True if the ruleset contains no rules.
		 */
		bool isEmpty(void) const;

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
		 * @param None.
		 * @return The ruleset as string.
		 */
		wxString toString(void) const;

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
		 * Refresh the policy ruleset after the underlying
		 * apn_ruleset has been modified.
		 * @param None.
		 * @return None.
		 */
		void refresh(void);

		/**
		 * Get the number of application policies.
		 * @param[in] None.
		 * @return The number of application policies.
		 */
		size_t getAppPolicyCount(void) const;

		void createAnswerPolicy(EscalationNotify *);

		/*
		 * XXX ch: re-enable this while adding functionality
		 * XXX ch: to the RuleEditor
		 *
		int createAlfAppPolicy(int id) {
			return createAppPolicy(APN_ALF, id);
		};
		int createSBAppPolicy(int id) {
			return createAppPolicy(APN_SB, id);
		};
		int createCtxAppPolicy(int id) {
			return createAppPolicy(APN_CTX, id);
		};
		int createAlfPolicy(int);
		int createCtxNewPolicy(int);
		int createSfsPolicy(int);

		bool findMismatchHash(void);
		bool deletePolicy(int);
		*/

	private:
		int			 refCnt_;
		int			 priority_;
		wxString		 origin_;
		long			 ruleSetId_;
		uid_t			 uid_;
		bool			 hasErrors_;
		bool			 isModified_;
		struct apn_ruleset	*ruleSet_;
		PolicyList		 alfList_;
		PolicyList		 sfsList_;
		PolicyList		 ctxList_;
		PolicyList		 sbList_;

		/**
		 * Clean the queues
		 */
		void clean(void);

		/**
		 * Create ruleset from file.
		 * @param[in] 1st Name of file to be parsed.
		 * @param[in] 2nd Shall check permissions of file.
		 */
		void create(wxString, bool);

		/**
		 * Create policies from parsed ruleset.
		 */
		void create(struct apn_ruleset *);

		/*
		 * XXX ch: re-enable this while adding functionality
		 * XXX ch: to the RuleEditor
		 *
		bool hasLocalHost(wxArrayString);
		int createAppPolicy(int type, int id);
		*/

		struct apn_rule *assembleAlfPolicy(AlfFilterPolicy *,
		   EscalationNotify *);

		void log(const wxString &);
		void status(const wxString &);
};

#endif	/* _POLICYRULESET_H_ */

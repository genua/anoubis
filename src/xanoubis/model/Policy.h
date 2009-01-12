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

#ifndef _POLICY_H_
#define _POLICY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/list.h>
#include <wx/string.h>

#include "libapn.h"
#include "Subject.h"

class PolicyVisitor;
class PolicyRuleSet;

/**
 * This is the base class of all policies. It carris the apn_rule and the
 * subclasses will provide getter and setter methods for the various types.
 * All policies are just proxy classes. The concerning apn_rule will be
 * handled by the apn_ruleset itself.
 */
class Policy : public Subject
{
	DECLARE_CLASS(Policy);

	public:
		/**
		 * Constructor of a policy.
		 * @param[in] 1st The ruleset this policy belongs to.
		 * @param[in] 2nd The apn_rule this policy should represent.
		 */
		Policy(PolicyRuleSet *, struct apn_rule *);

		/**
		 * Destructor of a policy.
		 */
		virtual ~Policy(void);

		/**
		 * Test if a policy was modified.
		 * @param None.
		 * @return True if policy was modified.
		 */
		bool isModified(void) const;

		/**
		 * Mark this policy as been modified.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void setModified(void);

		/**
		 * Set this policy as not been modified (e.g after save).
		 * @param None.
		 * @return Nothing.
		 */
		void clearModified(void);

		/**
		 * Get the parent ruleset.
		 * @param None.
		 * @return The ruleset this policy belongs to.
		 */
		PolicyRuleSet *getParentRuleSet(void) const;

		/**
		 * Accept a visitor.
		 * @param[in] 1st The visitor.
		 * @return Nothing.
		 */
		virtual void accept(PolicyVisitor &) = 0;

		/**
		 * Get a string with the type of this policy.
		 * @param None.
		 * @return A string with the type.
		 */
		virtual wxString getTypeIdentifier(void) const = 0;

		/**
		 * Get the native/apn rule id.
		 * @param None.
		 * @return The apn id of this policy.
		 */
		int getApnRuleId(void) const;

		/**
		 * Is this a temporary policy / has it a scope?
		 * @param None.
		 * @return True if scope is present.
		 */
		bool hasScope(void) const;

		/**
		 * Get the scope.
		 * @param None.
		 * @return A string with the scope.
		 */
		wxString getScopeName(void) const;

		/**
		 * XXX ch: I don't like this, better solution needed here
		 */
		void setRuleEditorIndex(unsigned long);
		unsigned long getRuleEditorIndex(void) const;

	protected:
		/**
		 * Get the apn_rule structure.
		 * @param None.
		 * @return 1st The apn_rule
		 */
		struct apn_rule *getApnRule(void) const;

	private:
		bool		 modified_;	 /**< Modified flag. */
		PolicyRuleSet	*parentRuleSet_; /**< The belonging ruleset. */
		struct apn_rule	*rule_;		 /**< Native/apn rule. */
		/** XXX ch: I don't like this, better solution needed here */
		unsigned long	 ruleEditorIndex_;
};

WX_DECLARE_LIST(Policy, PolicyList);

#endif	/* _POLICY_H_ */

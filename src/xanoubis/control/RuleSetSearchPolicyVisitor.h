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

#ifndef _RULESETSEARCHPOLICYVISITOR_H_
#define _RULESETSEARCHPOLICYVISITOR_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/string.h>

#include "PolicyVisitor.h"

#include "Policy.h"
#include "AlfAppPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "AlfFilterPolicy.h"
#include "ContextAppPolicy.h"
#include "ContextFilterPolicy.h"
#include "DefaultFilterPolicy.h"
#include "SbAccessFilterPolicy.h"
#include "SbAppPolicy.h"
#include "SfsAppPolicy.h"
#include "SfsDefaultFilterPolicy.h"
#include "SfsFilterPolicy.h"

/**
 * PolicyVisitor for PolicyRuleSet.
 * Use this visitor for searching a special policy within a PolicyRuleSet.
 * You can search for a special id or a checksum.
 */
class RuleSetSearchPolicyVisitor : public PolicyVisitor
{
	public:
		/**
		 * Constructor of RuleSetSearchPolicyVisitor.
		 * Use this to search for a policy with the given apn_id.
		 * @param[in] 1st The id to search for.
		 * @return Nothing.
		 */
		RuleSetSearchPolicyVisitor(int);

		/**
		 * Constructor of RuleSetSearchPolicyVisitor.
		 * Use this to search for a policy with the given checksum.
		 * @param[in] 1st The checksum to search for.
		 * @return Nothing.
		 */
		RuleSetSearchPolicyVisitor(wxString);

		/**
		 * Get found policy.
		 * If a matching policy was found, you can use this method
		 * to fetch it. Use hasMatchingPolicy() to see if the search
		 * was successfull.
		 * @param None.
		 * @return The found policy.
		 * @see hasMatchingPolicy()
		 */
		Policy *getMatchingPolicy(void);

		/**
		 * Was a policy found.
		 * After the search was done, this method can tell you if a
		 * policy was found. Use getMatchingPolicy() to fetch it.
		 * @param None.
		 * @return True if a policy was matched.
		 * @see getMatchingPolicy()
		 */
		bool hasMatchingPolicy(void);

		/**
		 * Visit a AlfAppPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfAppPolicy(AlfAppPolicy *);

		/**
		 * Visit a AlfCapabilityFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfCapabilityFilterPolicy(
		    AlfCapabilityFilterPolicy *);

		/**
		 * Visit a AlfFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfFilterPolicy(AlfFilterPolicy *);

		/**
		 * Visit a ContextAppPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitContextAppPolicy(ContextAppPolicy *);

		/**
		 * Visit a ContextFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitContextFilterPolicy(ContextFilterPolicy *);

		/**
		 * Visit a DefaultFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitDefaultFilterPolicy(DefaultFilterPolicy *);

		/**
		 * Visit a SbAccessFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSbAccessFilterPolicy(SbAccessFilterPolicy *);

		/**
		 * Visit a SbAppPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSbAppPolicy(SbAppPolicy *);

		/**
		 * Visit a SfsAppPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSfsAppPolicy(SfsAppPolicy *);

		/**
		 * Visit a SfsFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSfsFilterPolicy(SfsFilterPolicy *);

		/**
		 * Visit a SfsDefaultFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSfsDefaultFilterPolicy(
		    SfsDefaultFilterPolicy *);

	private:
		int		 seekId_;	/**< Search for this id */
		wxString	 seekHash_;	/**< Search for this checksum */
		Policy		*matchingPolicy_;	/**< Search result. */

		/**
		 * Search for id.
		 * This method does the work of comparing the id of a (given)
		 * policy with the id to search for. If they match the policy
		 * is stored for later access. If a policy was found no
		 * further comparison is done - first match strategy.
		 * @param[in] 1st The policy to query for it's id.
		 * @return Nothing.
		 */
		void	compare(Policy *);

		/**
		 * Search for checksum.
		 * This method does the work of comparing the checksum of a
		 * (given) policy with the checksum to search for. If they
		 * match the policy is stored for later access. If a policy
		 * was found no further comparison is done - first match
		 * strategy.
		 * @param[in] 1st The policy to query for it's checksum.
		 * @return Nothing.
		 */
		void	compareHash(AppPolicy *);
};

#endif	/* _RULESETSEARCHPOLICYVISITOR_H_ */

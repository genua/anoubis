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

#ifndef _ALFAPPPOLICY_H_
#define _ALFAPPPOLICY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "AppPolicy.h"
#include "AlfFilterPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "DefaultFilterPolicy.h"

/**
 * This is the alf application policy.
 */
class AlfAppPolicy : public AppPolicy
{
	public:
		/**
		 * Constructor of a AlfAppPolicy.
		 * This should be used during creation of a PolicyRuleSet,
		 * when the concerning apn_rule already exists.
		 * @param[in] 1st The ruleset this policy belongs to.
		 * @param[in] 2nd The apn_rule this policy should represent.
		 */
		AlfAppPolicy(PolicyRuleSet *, struct apn_rule *);

		/**
		 * Constructor of an empty AlfAppPolicy.
		 * It's the duty of the caller to add this policy to the
		 * ruleset by prependAppPolicy().
		 * @param[in] 1st The ruleset this policy belongs to.
		 */
		AlfAppPolicy(PolicyRuleSet *);

		/**
		 * Get the policy type as string.
		 * @param None.
		 * @return String with the policy type.
		 */
		virtual wxString getTypeIdentifier(void) const;

		/**
		 * Create native /apn rule.
		 * @param None.
		 * @return A single apn rule.
		 */
		static struct apn_rule *createApnRule(void);

		/**
		 * Create apn rule and insert to apn ruleset.
		 * This will create a new apn_rule (by createApnRule()).
		 * This new rule is inserted to the apn_ruleset.
		 * On success the apn_ruleset is modified and out of sync
		 * with the policy object tree. It's the responsibility of
		 * the caller to re-create the object tree.
		 * @param[in] 1st The policy rule set.
		 * @param[in] 2nd Insert before this id (on top if id=0)
		 * @return True on success.
		 */
		static bool createApnInserted(PolicyRuleSet *, unsigned int);

		/**
		 * Accept a visitor.
		 * @param[in] 1st The visitor.
		 * @return Nothing.
		 */
		virtual void accept(PolicyVisitor &);

		/**
		 * Add a given filter on top.
		 * Insert the apn_rule of the given policy to the apn ruleset
		 * and insert the policy to the list of filters.
		 * In both cases do an 'insert-head'.
		 * @param[in] 1st The filter policy to insert.
		 * @return True on success.
		 */
		virtual bool prependFilterPolicy(FilterPolicy *);
};

#endif	/* _ALFAPPPOLICY_H_ */

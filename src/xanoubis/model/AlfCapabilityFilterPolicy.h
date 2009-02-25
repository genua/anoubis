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

#ifndef _ALFCAPABILITYFILTERPOLICY_H_
#define _ALFCAPABILITYFILTERPOLICY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/string.h>

#include "FilterPolicy.h"
class AlfAppPolicy;

/**
 * This is an ALF capability policy.
 */
class AlfCapabilityFilterPolicy : public FilterPolicy
{
	DECLARE_CLASS(AlfCapabilityFilterPolicy)

	public:
		/**
		 * Constructor of a AlfCapabilityFilterPolicy.
		 * @param[in] 1st The parent alf application policy.
		 * @param[in] 2nd The apn_rule this policy should represent.
		 */
		AlfCapabilityFilterPolicy(AppPolicy *, struct apn_rule *);

		/**
		 * Constructor of an empty AlfCapabilityFilterPolicy.
		 * It's the duty of the caller to add this policy to the
		 * parent AlfAppPolicy by prependFilterPolicy().
		 * @param[in] 1st The parent alf application policy.
		 */
		AlfCapabilityFilterPolicy(AlfAppPolicy *);

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
		 * This new rule is inserted to the apn_ruleset been
		 * determined by the given parent. On success the apn_ruleset
		 * is modified and out of sync with the policy object tree.
		 * It's the responsibility of the caller to re-create the
		 * object tree.
		 * @param[in] 1st The parent policy.
		 * @param[in] 2nd Insert before this id (on top if id=0)
		 * @return True on success.
		 */
		static bool createApnInserted(AppPolicy *, unsigned int);

		/**
		 * Accept a visitor.
		 * @param[in] 1st The visitor.
		 * @return Nothing.
		 */
		virtual void accept(PolicyVisitor &);

		/**
		 * Set the apn log type.
		 * @param[in] 1st The apn log type.
		 * @return True on success.
		 */
		virtual bool setLogNo(int);

		/**
		 * Get the apn log type.
		 * @param None.
		 * @return The apn log type.
		 */
		virtual int getLogNo(void) const;

		/**
		 * Set the apn action type.
		 * @param[in] 1st The apn action type.
		 * @return True on success.
		 */
		virtual bool setActionNo(int);

		/**
		 * Get the apn action type.
		 * @param None.
		 * @return The apn action type.
		 */
		virtual int getActionNo(void) const;

		/**
		 * Set the apn capability type number.
		 * @param[in] 1st The new type number.
		 * @return True on success.
		 */
		bool setCapabilityTypeNo(int);

		/**
		 * Get the apn capability type number.
		 * @param None.
		 * @return The apn type number.
		 */
		int getCapabilityTypeNo(void) const;

		/**
		 * Get the string representation of capability type.
		 * @param None.
		 * @return String with capability type.
		 */
		wxString getCapabilityTypeName(void) const;
};

#endif	/* _ALFCAPABILITYFILTERPOLICY_H_ */

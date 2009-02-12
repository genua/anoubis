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

#ifndef _FILTERPOLICY_H_
#define _FILTERPOLICY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/string.h>
#include <wx/intl.h>

#include "Policy.h"
#include "AppPolicy.h"

/**
 * This is the base class of all filter policies.
 */
class FilterPolicy : public Policy
{
	DECLARE_CLASS(FilterPolicy)

	public:
		/**
		 * Constructor of a FilterPolicy.
		 * @param[in] 1st The parent application policy.
		 * @param[in] 2nd The apn_rule this policy should represent.
		 */
		FilterPolicy(AppPolicy *, struct apn_rule *);

		/**
		 * Get the parent application policy.
		 * @param None.
		 * @return The parent application policy.
		 */
		AppPolicy *getParentPolicy(void) const;

		/**
		 * Create native /apn rule.
		 * @param None.
		 * @return A single apn rule.
		 */
		static struct apn_rule *createApnRule(void);

		/**
		 * Mark this policy as been modified.
		 * @param None.
		 * @return Nothing.
		 */
		virtual void setModified(void);

		/**
		 * Set the apn log type.
		 * @param[in] 1st The apn log type.
		 * @return True on success.
		 */
		virtual bool setLogNo(int) = 0;

		/**
		 * Get the apn log type.
		 * @param None.
		 * @return The apn log type.
		 */
		virtual int getLogNo(void) const = 0;

		/**
		 * Get the string representation of log type.
		 * @param None.
		 * @return The string of log type.
		 */
		wxString getLogName(void) const;

		/**
		 * Set the apn action type.
		 * @param[in] 1st The apn action type.
		 * @return True on success.
		 */
		virtual bool setActionNo(int) = 0;

		/**
		 * Get the apn action type.
		 * @param None.
		 * @return The apn action type.
		 */
		virtual int getActionNo(void) const = 0;

		/**
		 * Get the string representation of action type.
		 * @param None.
		 * @return The string of action type.
		 */
		wxString getActionName(void) const;

		/**
		 * Return the path prefix of the rule if it has one.
		 * @param None.
		 * @return The path prefix as a wxString or an empty string.
		 * Derived classes must override this method.
		 */
		virtual wxString getRulePrefix(void) const;

	protected:
		/**
		 * Translate the apn log numbers APN_LOG_*
		 * to human readable string.
		 * @param[in] 1st The number to translate.
		 * @return The readable string.
		 */
		wxString logToString(int) const;

		/**
		 * Translate the apn action numbers APN_ACTION_*
		 * to human readable string.
		 * @param[in] 1st The number to translate.
		 * @return The readable string.
		 */
		wxString actionToString(int) const;

	private:
		AppPolicy *parentPolicy_; /**< The parent application. */
};

#endif	/* _FILTERPOLICY_H_ */

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

#ifndef _SBACCESSFILTERPOLICY_H_
#define _SBACCESSFILTERPOLICY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FilterPolicy.h"
class SbAppPolicy;

/**
 * This is the sandbox access filter policy.
 */
class SbAccessFilterPolicy : public FilterPolicy
{
	public:
		/**
		 * Constructor of a SbAccessFilterPolicy.
		 * @param[in] 1st The parent alf application policy.
		 * @param[in] 2nd The apn_rule this policy should represent.
		 */
		SbAccessFilterPolicy(AppPolicy *, struct apn_rule *);

		/**
		 * Constructor of an empty SbAccessFilterPolicy.
		 * It's the duty of the caller to add this policy to the
		 * parent SbAppPolicy by prependFilterPolicy().
		 * @param[in] 1st The parent sandbox application policy.
		 */
		SbAccessFilterPolicy(SbAppPolicy *);

		/**
		 * Get the policy type as string.
		 * @param None.
		 * @return String with the policy type.
		 */
		virtual wxString getTypeIdentifier(void) const;

		/**
		 * Accept a visitor.
		 * @param[in] 1st The visitor.
		 * @return Nothing.
		 */
		virtual void accept(PolicyVisitor &);

		/**
		 * Create native / apn application structure.
		 * @param None.
		 * @return A single, clean applcation structure.
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
		 * Set the apn log type.
		 * @param[in] 1st The apn log type.
		 * @return True on success.
		 */
		virtual bool setLogNo(int);

		/**
		 * Get the apn log number.
		 * @param None.
		 * @return The apn log number.
		 */
		virtual int getLogNo(void) const;

		/**
		 * Set the apn action type.
		 * @param[in] 1st The apn action type.
		 * @return True on success.
		 */
		virtual bool setActionNo(int);

		/**
		 * Get the apn action number.
		 * @param None.
		 * @return The apn action number.
		 */
		virtual int getActionNo(void) const;

		/**
		 * Set policy path.
		 * @param[in] 1st The new path.
		 * @return True on success.
		 */
		bool setPath(wxString);

		/**
		 * Get policy path.
		 * @param None.
		 * @return The policy path.
		 */
		wxString getPath(void) const;

		/**
		 * Set subject to none.
		 * This also modifies the subject type.
		 * @param None.
		 * @return True on success.
		 */
		bool setSubjectNone(void);
		/**
		 * Set subject self.
		 * This also modifies the subject type.
		 * @param[in] 1st True if you want to set SELF-SIGNED (default)
		 * @return True on success.
		 */
		bool setSubjectSelf(bool);

		/**
		 * Set the subject user id.
		 * This also modifies the subject type.
		 * @param[in] 1st ID of user.
		 * @return True on success.
		 */
		bool setSubjectUid(uid_t);

		/**
		 * Set the subject key.
		 * This also modifies the subject type.
		 * @param[in] 1st The key.
		 * @return True on success.
		 */
		bool setSubjectKey(wxString);

		/**
		 * Get the subject type.
		 * @param None.
		 * @return The type of subject.
		 */
		int getSubjectTypeNo(void) const;

		/**
		 * Get readable subject type.
		 * @param None.
		 * @return The type of subject.
		 */
		wxString getSubjectName(void) const;

		/**
		 * Set the new access mask of this policy.
		 * @param[in] 1st New mask (a combination of APN_SBA_*).
		 * @return True on success.
		 */
		bool setAccessMask(unsigned int);

		/**
		 * Get the access mask number.
		 * @param None.
		 * @return The mask as a combination of APN_SBA_*
		 */
		unsigned int getAccessMaskNo(void) const;

		/**
		 * Get the access mask name.
		 * @param None.
		 * @return The "readable" mask.
		 */
		wxString getAccessMaskName(void) const;

		/**
		 * Return the rule prefix of a sandbox rule as a wxString
		 * @param None.
		 * @return The prefix.
		 */
		wxString getRulePrefix(void) const;

};

#endif	/* _SBACCESSFILTERPOLICY_H_ */

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

#ifndef _CONTEXTFILTERPOLICY_H_
#define _CONTEXTFILTERPOLICY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FilterPolicy.h"
#include "PolicyUtils.h"
class ContextAppPolicy;

/**
 * This is an Context filter policy.
 */
class ContextFilterPolicy : public FilterPolicy
{
	public:
		/**
		 * Constructor of a ContextFilterPolicy.
		 * @param[in] 1st The parent context application policy.
		 * @param[in] 2nd The apn_rule this policy should represent.
		 */
		ContextFilterPolicy(AppPolicy *, struct apn_rule *);

		/**
		 * Constructor of an empty ContextFilterPolicy.
		 * It's the duty of the caller to add this policy to the
		 * parent ContextAppPolicy by prependFilterPolicy().
		 * @param[in] 1st The parent context application policy.
		 */
		ContextFilterPolicy(ContextAppPolicy *);

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
		 * Set context type.
		 * @param[in] 1st The apn context type.
		 * @return True on success.
		 */
		bool setContextTypeNo(int);

		/**
		 * Get the apn context type.
		 * @param None.
		 * @return The apn context type.
		 */
		int getContextTypeNo(void) const;

		/**
		 * Get readable context type.
		 * @param None.
		 * @return The context type.
		 */
		wxString getContextTypeName (void) const;

		/**
		 * Count the binaries of this context policy.
		 * @param None.
		 * @return The number of binaries.
		 */
		unsigned int getBinaryCount(void) const;

		/**
		 * Change the name of one binary.
		 * @param[in] 1st The new binary.
		 * @param[in] 2nd The index of binary to change.
		 * @return True on success.
		 */
		bool setBinaryName(wxString, unsigned int);

		/**
		 * Add a binary to the list of binaries before the
		 * policy with the given position.
		 * @param[in] 1st The position of the binary (zero based)
		 * @param[in] 2nd The name of the new binary.
		 * @return True on success.
		 */
		bool addToBinaryListAt(int, wxString);

		/**
		 * Get the name of a specified binary.
		 * @param[in] 1st The index of the binary in question.
		 * @return The name or a empty string.
		 */
		wxString getBinaryName(unsigned int) const;

		/**
		 * Get the name of (all) binary(s).
		 * @param None.
		 * @return The binary (maybe more than one, comma seperated).
		 */
		wxString getBinaryName(void) const;

		/**
		 * Get the name of (all) binary(s).
		 * @param None.
		 * @return The binaries (as list).
		 */
		wxArrayString getBinaryList(void) const;

		/**
		 * Add a binary.
		 * This will add the given binary at the end of the list
		 * of binaries. In case of 'any' nothing special is done.
		 * @param[in] 1st The binary name.
		 * @return True on success.
		 */
		bool addBinary(const wxString &);

		/**
		 * Remove a binary.
		 * This will remove the binary with the given index.
		 * In case we remove the last binary, 'any' is set.
		 * @param[in] 1st The index of the binary to remove.
		 * @return True on success.
		 */
		bool removeBinary(unsigned int);

		/**
		 * Does this an any-policy?
		 * @param None.
		 * @return True if it's an any-policy.
		 */
		bool isAny(void) const;

		/**
		 * Get the subject type.
		 * @param[in] 1st The application index.
		 * @return The type of subject.
		 */
		int getSubjectTypeNo(unsigned int) const;

		/**
		 * Get readable subject type.
		 * @param[in] 1st The application index.
		 * @return The type of subject.
		 */
		wxString getSubjectName(unsigned int) const;

		/**
		 * Set subject of an application to none.
		 * This also modifies the subject type.
		 * @param[in] 1st Index of the application.
		 * @return True on success.
		 */
		bool setSubjectNone(unsigned int);
		/**
		 * Set subject of an application to self.
		 * This also modifies the subject type.
		 * @param[in] 1st The index of the application.
		 * @param[in] 2nd True if you want to set SELF-SIGNED (default)
		 * @return True on success.
		 */
		bool setSubjectSelf(unsigned int, bool);

		/**
		 * Set the subject user id of an application.
		 * This also modifies the subject type.
		 * @param[in] 1st The index of the application.
		 * @param[in] 2nd ID of user.
		 * @return True on success.
		 */
		bool setSubjectUid(unsigned int, uid_t);

		/**
		 * Set the subject key of an application.
		 * This also modifies the subject type.
		 * @param[in] 1st The index of the application.
		 * @param[in] 2nd The key.
		 * @return True on success.
		 */
		bool setSubjectKey(unsigned int, wxString);

	private:
		/**
		 * Seek apn application structure by index.
		 * @param[in] 1st The indes in question.
		 * @return The found structure or NULL.
		 */
		struct apn_app *seekAppByIndex(unsigned int) const;
};

#endif	/* _CONTEXTFILTERPOLICY_H_ */

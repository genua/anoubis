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

#ifndef _SFSFILTERPOLICY_H_
#define _SFSFILTERPOLICY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FilterPolicy.h"

/**
 * This is a SFS access filter policy.
 */
class SfsFilterPolicy : public FilterPolicy
{
	public:
		/**
		 * Constructor of SfsFilterPolicy.
		 * @param[in] 1st The parent sfs application policy.
		 * @param[in] 2nd The apn_rule this policy should represent.
		 */
		SfsFilterPolicy(AppPolicy *, struct apn_rule *);

		/**
		 * Get the policy type as string.
		 * @param None.
		 * @return String with the policy type.
		 */
		virtual wxString getTypeIdentifier(void) const;

		/**
		 * Create native / apn rule.
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
		 * <b>SFS filter policies has no log!</b>
		 * @param[in] 1st The apn log type.
		 * @return False.
		 */
		virtual bool setLogNo(int);

		/**
		 * Get the apn log type.
		 * <b>SFS filter policies has no log!</b>
		 * @param None.
		 * @return -1
		 */
		virtual int getLogNo(void) const;

		/**
		 * Set the apn action type.
		 * <b>SFS filter policies has no action!</b>
		 * @param[in] 1st The apn action type.
		 * @return False.
		 */
		virtual bool setActionNo(int);

		/**
		 * Get the apn action type.
		 * <b>SFS filter policies has no action!</b>
		 * @param None.
		 * @return -1
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

		/*
		 * Get the rule prefix of an SFS default rule as a wxString.
		 * @param None.
		 * @return The rule prefix.
		 * This function differs from getPath in that it does not
		 * return "any" in case of an empty path.
		 */
		wxString getRulePrefix(void) const;

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
		 * Set action and log in valid case.
		 * @param[in] 1st The new apn action number.
		 * @param[in] 2nd Tne new apn log level.
		 * @return True.
		 */
		bool setValid(int, int);

		/**
		 * Set action and log in invalid case.
		 * @param[in] 1st The new apn action number.
		 * @param[in] 2nd Tne new apn log level.
		 * @return True.
		 */
		bool setInvalid(int, int);

		/**
		 * Set action and log in unknown case.
		 * @param[in] 1st The new apn action number.
		 * @param[in] 2nd Tne new apn log level.
		 * @return True.
		 */
		bool setUnknown(int, int);

		/**
		 * Set action in valid case.
		 * @param[in] 1st The new apn action number.
		 * @return True on success.
		 */
		bool setValidAction(int);

		/**
		 * Get action number of valid case.
		 * @param None.
		 * @return The apn action number.
		 */
		int getValidActionNo(void) const;

		/**
		 * Get action name of valid case.
		 * @param None.
		 * @return The apn action name.
		 */
		wxString getValidActionName(void) const;

		/**
		 * Set log in valid case.
		 * @param[in] 1st The new apn log level
		 * @return True on success.
		 */
		bool setValidLog(int);

		/**
		 * Get log number of valid case.
		 * @param None.
		 * @return The apn log level number.
		 */
		int getValidLogNo(void) const;

		/**
		 * Get log name of valid case.
		 * @param None.
		 * @return The apn log level name.
		 */
		wxString getValidLogName(void) const;

		/**
		 * Set action in invalid case.
		 * @param[in] 1st The new apn action number.
		 * @return True on success.
		 */
		bool setInvalidAction(int);

		/**
		 * Get action number of invalid case.
		 * @param None.
		 * @return The apn action number.
		 */
		int getInvalidActionNo(void) const;

		/**
		 * Get action name of invalid case.
		 * @param None.
		 * @return The apn action name.
		 */
		wxString getInvalidActionName(void) const;

		/**
		 * Set log in invalid case.
		 * @param[in] 1st The new apn log level.
		 * @return True on success.
		 */
		bool setInvalidLog(int);

		/**
		 * Get log number of invalid case.
		 * @param None.
		 * @return The apn log level number.
		 */
		int getInvalidLogNo(void) const;

		/**
		 * Get log name of invalid case.
		 * @param None.
		 * @return The apn log level name.
		 */
		wxString getInvalidLogName(void) const;

		/**
		 * Set action in unknown case.
		 * @param[in] 1st The new apn action number.
		 * @return True on success.
		 */
		bool setUnknownAction(int);

		/**
		 * Get action number of unknown case.
		 * @param None.
		 * @return The apn action number.
		 */
		int getUnknownActionNo(void) const;

		/**
		 * Get action name of unknown case.
		 * @param None.
		 * @return The apn action name.
		 */
		wxString getUnknownActionName(void) const;

		/**
		 * Set log in unknown case.
		 * @param[in] 1st The new apn log level.
		 * @return True on success.
		 */
		bool setUnknownLog(int);

		/**
		 * Get log number of unknown case.
		 * @param None.
		 * @return The apn log level number.
		 */
		int getUnknownLogNo(void) const;

		/**
		 * Get log name of unknown case.
		 * @param None.
		 * @return The apn log level name.
		 */
		wxString getUnknownLogName(void) const;

		/**
		 * translate String to Action and call
		 * the given Method to set the value
		 * @param[in] 1st Action-String
		 * @param[in] 2nd Funtion Pointer
		 * @return None.
		 */
		void translateAction(const wxString&,
		     bool (SfsFilterPolicy::*)(int));
		/**
		 * Translate String to Log-Level and
		 * call the given Method to set the value
		 * @param[in] 1st Action-String
		 * @param[in] 2nd Funtion Pointer
		 * @return None.
		 */
		void translateLog(const wxString&,
		     bool (SfsFilterPolicy::*)(int));

		/**
		 * Set action in valid case by name.
		 * @param[in] 1st The new apn action name.
		 * @return True on success.
		 */
		void setValidActionName(const wxString &);

		/**
		 * Set log in valid case by name.
		 * @param[in] 1st The new apn log level name.
		 * @return True on success.
		 */
		void setValidLogName(const wxString &);

		/**
		 * Set action in invalid case by name.
		 * @param[in] 1st The new apn action name.
		 * @return True on success.
		 */
		void setInvalidActionName(const wxString &);

		/**
		 * Set log in invalid case by name.
		 * @param[in] 1st The new apn log level name.
		 * @return True on success.
		 */
		void setInvalidLogName(const wxString &);

		/**
		 * Set action in unknown case by name.
		 * @param[in] 1st The new apn action name.
		 * @return True on success.
		 */
		void setUnknownActionName(const wxString &);

		/**
		 * Set log in unknown case by name.
		 * @param[in] 1st The new apn log level name.
		 * @return True on success.
		 */
		void setUnknownLogName(const wxString &);
};

#endif	/* _SFSFILTERPOLICY_H_ */

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
	DECLARE_CLASS(SfsFilterPolicy)

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
		 * Get action of valid case.
		 * @param None.
		 * @return The apn action.
		 */
		int getValidAction(void);

		/**
		 * Set log in valid case.
		 * @param[in] 1st The new apn log level
		 * @return True on success.
		 */
		bool setValidLog(int);

		/**
		 * Get log of valid case.
		 * @param None.
		 * @return The apn log level.
		 */
		int getValidLog(void);

		/**
		 * Set action in invalid case.
		 * @param[in] 1st The new apn action number.
		 * @return True on success.
		 */
		bool setInvalidAction(int);

		/**
		 * Get action of invalid case.
		 * @param None.
		 * @return The apn action.
		 */
		int getInvalidAction(void);

		/**
		 * Set log in invalid case.
		 * @param[in] 1st The new apn log level.
		 * @return True on success.
		 */
		bool setInvalidLog(int);

		/**
		 * Get log of invalid case.
		 * @param None.
		 * @return The apn log level.
		 */
		int getInvalidLog(void);

		/**
		 * Set action in unknown case.
		 * @param[in] 1st The new apn action number.
		 * @return True on success.
		 */
		bool setUnknownAction(int);

		/**
		 * Get action of unknown case.
		 * @param None.
		 * @return The apn action.
		 */
		int getUnknownAction(void);

		/**
		 * Set log in unknown case.
		 * @param[in] 1st The new apn log level.
		 * @return True on success.
		 */
		bool setUnknownLog(int);

		/**
		 * Get log of unknown case.
		 * @param None.
		 * @return The apn log level.
		 */
		int getUnknownLog(void);

	private:
		/**
		 * Clean the subject structure.
		 * @param[in] 1st The structure to clean.
		 * @return Nothing.
		 */
		void cleanSubject(struct apn_rule *);
};

#endif	/* _SFSFILTERPOLICY_H_ */

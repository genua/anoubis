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

#ifndef _APPPOLICY_H_
#define _APPPOLICY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>

#include <wx/arrstr.h>
#include <wx/string.h>

#include "AnListClass.h"
#include "AnRowProvider.h"
#include "Policy.h"

/* Foreward declaration for prepending filter policies. */
class FilterPolicy;
class PolicyRowProvider;

/**
 * This is the base class of all application policies.
 */
class AppPolicy : public Policy
{
	public:
		/**
		 * Constructor of a AppPolicy.
		 * @param[in] 1st The ruleset this policy belongs to.
		 * @param[in] 2nd The apn_rule this policy should represent.
		 */
		AppPolicy(PolicyRuleSet *, struct apn_rule *);

		/**
		 * Destructor for a AppPolicy.
		 * @param None
		 * @return Nothing
		 */
		~AppPolicy(void);

		/**
		 * Create native / apn application structure.
		 * @param None.
		 * @return A single, clean applcation structure.
		 */
		static struct apn_app *createApnApp(void);

		/**
		 * Create native /apn rule.
		 * @param None.
		 * @return A single apn rule.
		 */
		static struct apn_rule *createApnRule(void);

		/**
		 * Accept a visitor.
		 * @param[in] 1st The visitor.
		 * @return Nothing.
		 */
		virtual void accept(PolicyVisitor &) = 0;

		/**
		 * Propagate visitor to childs/filters.
		 * @param[in] 1st The visitor.
		 * @return Nothing.
		 */
		void acceptOnFilter(PolicyVisitor &);

		/**
		 * Returns the AnRowProvider assigned to the policy.
		 *
		 * Use this row-provider if you want to display the app-policy.
		 * The row-provider returns the filter-policies of the
		 * app-policy. Thus displaying the app-policy means to list the
		 * filter-policies.
		 *
		 * @return The row-provider of the app-policy
		 */
		AnRowProvider *getRowProvider(void) const;

		/**
		 * Count the filters of this application policy.
		 * @param None.
		 * @return Number of filters.
		 */
		size_t getFilterPolicyCount(void) const;

		/**
		 * Returns the filter-policy at the given index.
		 *
		 * If the index is out of range, NULL is returned.
		 *
		 * @param[in] 1st The requested index
		 * @return Filter-policy at the given index or NULL if the
		 *         index is out of range.
		 */
		FilterPolicy *getFilterPolicyAt(unsigned int) const;

		/**
		 * Returns the index of the given policy.
		 *
		 * @param[in] 1st The policy you are searching for
		 * @return The index of the policy. If the policy is not part
		 *         of this policy, -1 is returned.
		 */
		int getIndexOfFilterPolicy(FilterPolicy *) const;

		/**
		 * Count the binaries of this application policy.
		 * @param None.
		 * @return The number of binaries.
		 */
		unsigned int getBinaryCount(void) const;

		/**
		 * Add a given filter on top.
		 * Insert the apn_rule of the given policy to the apn ruleset
		 * and insert the policy to the list of filters.
		 * In both cases do an 'insert-head'.
		 * @param[in] 1st The filter policy to insert.
		 * @return True on success.
		 */
		virtual bool prependFilterPolicy(FilterPolicy *) = 0;

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
		 * Remove a binary.
		 * This will remove the binary with the given name.
		 * In case we remove the last binary, 'any' ist set.
		 * @param[in] 1st The name of the binary. to remove.
		 * @return True on success.
		 */
		bool removeBinary(const wxString &);

		/**
		 * Does this policy represent an any-block?
		 * @param None.
		 * @return True if it's an any-block.
		 */
		bool isAnyBlock(void) const;

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

	protected:
		/**
		 * Appends a filter-policy at the policy-list.
		 *
		 * @param[in] 1st The policy to be appended
		 */
		void filterListAppend(FilterPolicy *);

		/**
		 * Prepends a filter-policy at the front of the policy-list.
		 *
		 * @param[in] 1st The policy to be prepended
		 */
		void filterListPrepend(FilterPolicy *);

		/**
		 * Implementation of Policy::sendPolicyChangeEvent().
		 *
		 * Send the event to PolicyRuleSet::getRowProvider().
		 */
		void sendPolicyChangeEvent(void);

	private:
		/**
		 * Seek apn application structure by index.
		 * @param[in] 1st The indes in question.
		 * @return The found structure or NULL.
		 */
		struct apn_app *seekAppByIndex(unsigned int) const;

		/**
		 * List contains instances of child-filter-policies.
		 */
		std::vector<FilterPolicy *> filterList_;

		/**
		 * The row-provider of the policy.
		 */
		PolicyRowProvider *rowProvider_;

		friend class PolicyRuleSet;
};

#endif	/* _APPPOLICY_H_ */

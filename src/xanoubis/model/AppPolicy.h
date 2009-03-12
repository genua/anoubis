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

#include <wx/arrstr.h>
#include <wx/string.h>

#include "Policy.h"

/* Foreward declaration for prepending filter policies. */
class FilterPolicy;

/**
 * This is the base class of all application policies.
 */
class AppPolicy : public Policy
{
	DECLARE_CLASS(AppPolicy)

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
		 * Count the filters of this application policy.
		 * @param None.
		 * @return Number of filters.
		 */
		size_t getFilterPolicyCount(void) const;

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
		 * Set the list of binaries.
		 * This will erase the current list completely and creates
		 * a new list of apn_app's filled with the entries of the
		 * given list.
		 * @param[in] 1st The list of new binaries.
		 * @return True on success.
		 */
		bool setBinaryList(wxArrayString);

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
		 * Change the hash type to binary with given index.
		 * @param[in] 1st The new hash type.
		 * @param[in] 2nd The index of binary to change.
		 * @return True on success.
		 */
		bool setHashTypeNo(int, unsigned int);

		/**
		 * Changet the hash type of all binaries.
		 * @param[in] 1st The new hash type.
		 * @return True on success.
		 */
		bool setAllToHashTypeNo(int);

		/**
		 * Get the apn hash type of  binary of given index.
		 * @param[in] 1st The index of the binary.
		 * @return The apn hash type of that binary.
		 * @see getHashTypeName()
		 */
		int getHashTypeNo(unsigned int) const;

		/**
		 * Get the string represnetation of hash type.
		 * @param[in] 1st The index of the binary.
		 * @return The string with the hash type.
		 * @see getHashTypeNo()
		 * @see getHashTypeList()
		 */
		wxString getHashTypeName(unsigned int) const;

		/**
		 * Get the string represnetation of hash types of all binaries.
		 * @param None.
		 * @return A list of all hash types.
		 * @see getHashTypeName()
		 */
		wxArrayString getHashTypeList(void) const;

		/**
		 * Set the hash value to binary of given index.
		 * @param[in] 1st The new hash value.
		 * @param[in] 2nd The index of the binary.
		 * @return True on success.
		 */
		bool setHashValueNo(unsigned char *, unsigned int);

		/**
		 * Set the hash value to binary of given index.
		 * @param[in] 1st The new hash value.
		 * @param[in] 2nd The index of the binary.
		 * @return True on success.
		 */
		bool setHashValueString(const wxString &, unsigned int);

		/**
		 * Get the apn hash value of binary of given index.
		 * @param[in] 1st The index of binary in question.
		 * @param[out] 2nd The place where to put the apn hash value.
		 * @param[in] 3rd The size of the given place.
		 * @return True on success.
		 * @see getHashValueName()
		 */
		bool getHashValueNo(unsigned int, unsigned char *, int) const;

		/**
		 * Get the string representation of hash value.
		 * @param[in] 1st The index of binary in quesiton.
		 * @return The value as string on success or an empyt string.
		 * @see getHashValueNo()
		 * @see getHashValueList()
		 */
		wxString getHashValueName(unsigned int) const;

		/**
		 * Get the entire list of hash values.
		 * @param None.
		 * @return The list of hash values (maybe empty).
		 */
		wxArrayString getHashValueList(void) const;

	protected:
		PolicyList filterList_;	/**< List of our filter policies. */

	private:
		/**
		 * Seek apn application structure by index.
		 * @param[in] 1st The indes in question.
		 * @return The found structure or NULL.
		 */
		struct apn_app *seekAppByIndex(unsigned int) const;

		friend class PolicyRuleSet;
};

#endif	/* _APPPOLICY_H_ */

/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#ifndef _ModSbMainPanelImpl_
#define _ModSbMainPanelImpl_

#include "AnEvents.h"
#include "Observer.h"

#include "Policy.h"
#include "ContextFilterPolicy.h"
#include "DefaultFilterPolicy.h"
#include "SbAccessFilterPolicy.h"
#include "SbAppPolicy.h"
#include "ModSbPanelsBase.h"

class ModSbMainPanelImpl : public Observer, public ModSbMainPanelBase
{
	public:
		/**
		 * Constructor of ModSbMainPanelImpl.
		 * @param[in] 1st The parent window and ID.
		 */
		ModSbMainPanelImpl(wxWindow*, wxWindowID);

		/**
		 * Destructor of ModSbMainPanelImpl.
		 * @param None.
		 */
		~ModSbMainPanelImpl(void);

		/**
		 * Adds a default filter policy.
		 * This should be used by ModSbAddPolicyVisitor only.
		 * @param[in] 1st Concerning default filter policy.
		 * @return Nothing.
		 */
		void addDefaultFilterPolicy(DefaultFilterPolicy*);

		/**
		 * Adds a Sb access filter policy.
		 * This should be used by ModSbAddPolicyVisitor only.
		 * @param[in] 1st Concerning Sb access filter policy.
		 * @return Nothing.
		 */
		void addSbAccessFilterPolicy(SbAccessFilterPolicy*);

		/**
		 * Adds a Sb application policy.
		 * This should be used by ModSbAddPolicyVisitor only.
		 * @param[in] 1st Concerning Sb application policy.
		 * @return Nothing.
		 */
		void addSbAppPolicy(SbAppPolicy*);

		/**
		 * This is called when an observed policy was modified.
		 * @param[in] 1st The changed policy (aka subject)
		 * @return Nothing.
		 */
		virtual void update(Subject *);

		/**
		 * This is called when an observed policy is about to
		 * be destroyed.
		 * @param[in] 1st The changed policy (aka subject)
		 * @return Nothing.
		 */
		virtual void updateDelete(Subject *);

	private:
		/**
		 * The columns used within ModSb
		 */
		enum modSbListColumns {
			COLUMN_PROG = 0,	/**< name of application. */
			COLUMN_PATH,		/**< path to binary. */
			COLUMN_SUB,		/**< the subject. */
			COLUMN_ACTION,		/**< type of filter action. */
			COLUMN_MASK,		/**< the access mask. */
			COLUMN_SCOPE,		/**< scope of the filter. */
			COLUMN_USER,		/**< user of ruleset. */
			COLUMN_EOL		/**< End - Of - List. */
		};

		long	userRuleSetId_;		/**< Id of our RuleSet . */
		long	adminRuleSetId_;	/**< Id of our admin RuleSet. */

		/**
		 * The heading line of the columns
		 */
		wxString	columnNames_[COLUMN_EOL];

		/**
		 * Handle the event on loading RuleSet
		 * This just receives the event and updates the id's of
		 * user and admin ruleset.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onLoadRuleSet(wxCommandEvent&);

		/**
		 * Remove a given row from the list.
		 * @param[in] 1st The index of the row in question.
		 * @return Nothing.
		 */
		void removeListRow(long);

		/**
		 * Find the row of a given policy.
		 * @param[in] 1st The policy to search for.
		 * @return The index of found row or -1.
		 */
		long findListRow(Policy *);

		/**
		 * Appends a new policy to the RuleSet
		 * Updates Columns regarding User/Admin policy and additionally
		 * registers the policy for observation
		 */
		long ruleListAppend(Policy *);

		/**
		 * Update row.
		 * Updates the values of a row showing a DefaultFilterPolicy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateDefaultFilterPolicy(long);

		/**
		 * Update row.
		 * Updates the values of a row showing a Sb AccessFilterPolicy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateSbAccessFilterPolicy(long);

		/**
		 * Update row.
		 * Updates the values of a row showing a Sb AppPolicy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateSbAppPolicy(long);

		/**
		 * Update RuleSet.
		 * Updates and shows the new RuleSet after operations like
		 * 'delete' modified it.
		 * @param None.
		 * @return Nothing.
		 */
		void updateShowRuleset(void);
};

#endif	/* _ModSbMainPanelImpl_ */

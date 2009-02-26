/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#ifndef __ModAlfMainPanelImpl__
#define __ModAlfMainPanelImpl__

#include "AnEvents.h"
#include "Observer.h"

#include "AppPolicy.h"
#include "AlfAppPolicy.h"
#include "AlfFilterPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "DefaultFilterPolicy.h"
#include "ModAlfPanelsBase.h"

class ModAlfMainPanelImpl : public Observer, public ModAlfMainPanelBase
{
	public:
		/**
		 * Add alf capabiliyt filter policy.
		 * This should be used by ModAlfAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addAlfCapabilityFilterPolicy(AlfCapabilityFilterPolicy *);

		/**
		 * Add alf filter policy.
		 * This should be used by ModAlfAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addAlfFilterPolicy(AlfFilterPolicy *);

		/**
		 * Add alf application policy.
		 * This should be used by ModAlfAddPolicyVisitor only.
		 * @param[in] 1st Concerning an alf app policy
		 * @return Nothing.
		 */
		void addAlfAppPolicy(AlfAppPolicy *);

		/**
		 * Add default filter policy.
		 * This should be used by ModAlfAddPolicyVisitor only.
		 * @param[in] 1st Concerning app policy
		 * @return Nothing.
		 */
		void addDefaultFilterPolicy(DefaultFilterPolicy *);

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

		/**
		 * Constructor of ModAlfMainPanelImpl.
		 * @param[in] 1st The parent window and ID.
		 */
		ModAlfMainPanelImpl(wxWindow*, wxWindowID);

		/**
		 * Destructor of ModAlfMainPanelImpl.
		 * @param None.
		 */
		~ModAlfMainPanelImpl(void);

	private:
		/**
		 * The columns used within ModAlf
		 */
		enum modAlfListColumns {
			COLUMN_PROG,		/**< name of application. */
			COLUMN_SERVICE,		/**< type of service. */
			COLUMN_ROLE,		/**< role type. */
			COLUMN_ACTION,		/**< type of filter action. */
			COLUMN_SCOPE,		/**< scope of the filter. */
			COLUMN_USER,		/**< user of ruleset. */
			COLUMN_EOL		/**< End Of List */
		};

		long	userRuleSetId_;	 /**< Id of our RuleSet . */
		long	adminRuleSetId_; /**< Id of our admin RuleSet. */

		/**
		 * The heading line of the columns
		 */
		wxString	columnNames_[COLUMN_EOL];

		/**
		 * Appends a new policy to the RuleSet
		 * Updates Columns regarding User/Admin policy and additionally
		 * registers the policy for observation
		 */
		long ruleListAppend(Policy *);

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
		 * Update row.
		 * Updates the values of a row showing an alf capability
		 * filter policy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateAlfCapabilityFilterPolicy(long);

		/**
		 * Update row.
		 * Updates the values of a row showing an alf filter policy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateAlfFilterPolicy(long);

		/**
		 * Update row.
		 * Updates the values of a row showing an application policy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateAlfAppPolicy(long);

		/**
		 * Update row.
		 * Updates the values of a row showing a default filter policy.
		 * @param[in] 1st The index of row in question.
		 * @return Nothing.
		 */
		void updateDefaultFilterPolicy(long);

		/**
		 * Update RuleSet.
		 * Updates and shows the new RuleSet after operations like
		 * 'delete' modified it.
		 * @param None.
		 * @return Nothing.
		 */
		void updateShowRuleset(void);
};

#endif /* __ModAlfMainPanelImpl__ */

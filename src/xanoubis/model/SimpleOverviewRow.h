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

#ifndef _SIMPLEOVERVIEWROW_H_
#define _SIMPLEOVERVIEWROW_H_

#include "Observer.h"

class AppPolicy;
class FilterPolicy;
class PolicyRuleSet;

/**
 * This class contains all informations for the AnListCtrl
 * to build the lists in the overviews. Every row is represented
 * by an instance of this class.
 */
class SimpleOverviewRow : private Observer
{
	public:
		/**
		 * Constructor, creating the simple Overview.
		 *
		 * @param int The Index of the application policy.
		 * @param AppPolicy The application policy itself.
		 * @param int The Index of the filter policy.
		 * @param FilterPolicy The filter Policy itself.
		 */
		SimpleOverviewRow(PolicyRuleSet *, unsigned int, AppPolicy *,
		    unsigned int, FilterPolicy *);

		/**
		 * D'tor.
		 */
		~SimpleOverviewRow(void);

		/**
		 * Returns the ruleset.
		 *
		 * This is the parent of all policies assigned to this class.
		 *
		 * @return The parent ruleset
		 */
		PolicyRuleSet *getRuleSet(void) const;

		/**
		 * Returns the policy of one specific filter.
		 *
		 * @return The filter policy.
		 */
		FilterPolicy *getFilterPolicy(void) const;

		/**
		 * Returns the Index of the filter policy.
		 *
		 * @return The Index of the filter.
		 */
		unsigned int getFilterPolicyIndex(void) const;

		/**
		 * Returns one specific application policy.
		 *
		 * @return The application policy.
		 */
		AppPolicy *getApplicationPolicy(void) const;

		/**
		 * Returns the Index of one specific application policy.
		 *
		 * @return The Index of an application policy.
		 */
		unsigned int getApplicationPolicyIndex(void) const;

		/**
		 * Checks, if the row can display a binary.
		 *
		 * It depends on on the number of binaries in the
		 * application-policy.
		 *
		 * @return true if the row can display a binary, false
		 *         otherwise.
		 */
		bool haveBinary(void) const;

	private:
		/**
		 * Index of a filter policy.
		 */
		unsigned int	filterIndex_;

		/**
		 * Index of an application policy
		 */
		unsigned int	applicationIndex_;

		/**
		 * The ruleset
		 */
		PolicyRuleSet	*ruleSet_;

		/**
		 * Policy of an application.
		 */
		AppPolicy	*applicationPolicy_;

		/**
		 * Policy of a filter.
		 */
		FilterPolicy	*filterPolicy_;

		/**
		 * Invoked, if the assigned AppPolicy or FilterPolicy is
		 * changed.
		 */
		void update(Subject *);

		/**
		 * Invoked, if the assigned AppPolicy or FilterPolicy is
		 * changed.
		 *
		 * Resets the related associations to NULL, to still have
		 * a valid reference.
		 */
		void updateDelete(Subject *);
};

#endif	/* _SIMPLEOVERVIEWROW_H_ */

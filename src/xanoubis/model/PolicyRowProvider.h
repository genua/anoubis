/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#ifndef _POLICYROWPROVIDER_H_
#define _POLICYROWPROVIDER_H_

#include <AnRowProvider.h>

class AppPolicy;
class PolicyRuleSet;

/**
 * The row-provider for AppPolicy resp. PolicyRuleSet.
 */
class PolicyRowProvider : public AnRowProvider
{
	public:
		/**
		 * Implementation of AnRowProvider::getRow().
		 *
		 * Depending whether a PolicyRuleSet or a AppPolicy is assigned
		 * to the object, it returns the AppPolicy or the FilterPolicy
		 * at the ginen index.
		 *
		 * @param idx The index.
		 * @return AppPolicy resp. FilterPolicy at the given index
		 * @see PolicyRuleSet::getPolicyAt()
		 * @see AppPolicy::getFilterPolicyAt()
		 */
		AnListClass *getRow(unsigned int idx) const;

		/**
		 * Implementation of AnRowProvider::getSize().
		 *
		 * Depending whether a PolicyRuleSet or a AppPolicy is assigned
		 * to the object, it returns the number of assigned AppPolicy or
		 * FilterPolicy-instances.
		 *
		 * @return Number of instances
		 * @see PolicyRuleSet::getAppPolicyCount()
		 * @see AppPolicy::getFilterPolicyCount()
		 */
		int getSize(void) const;

	private:
		/**
		 * Default-c'tor.
		 *
		 * Made private because nobody (except AppPolicy and
		 * PolicyRuleSet) should be able to create an instance of the
		 * class.
		 */
		PolicyRowProvider(void) {}

		/**
		 * Creates a row-provider with an assigned AppPolicy.
		 *
		 * The instance acts as a row-provider for an AppPolicy.
		 *
		 * @param policy The app-policy to be assigned
		 */
		PolicyRowProvider(AppPolicy *);

		/**
		 * Creates a row-provider with an assigned PolicyRuleSet.
		 *
		 * The instance acts as a row-provider for a PolicyRuleSet.
		 *
		 * @param ruleset The ruleset to be assigned
		 */
		PolicyRowProvider(PolicyRuleSet *);

		/**
		 * Copy-c'tor.
		 *
		 * Made private because nobody (except AppPolicy and
		 * PolicyRuleSet) should be able to copy instances of the
		 * class.
		 */
		PolicyRowProvider(const PolicyRowProvider &) :
		    AnRowProvider() {}

		/**
		 * D'tor.
		 *
		 * Made private because nobody (except AppPolicy and
		 * PolicyRuleSet) should be able to destroy instances of the
		 * class.
		 */
		~PolicyRowProvider(void) {}

		/**
		 * The assigned AppPolicy (if any).
		 */
		AppPolicy	*appPolicy_;

		/**
		 * The assigned PolicyRuleSet (if any).
		 */
		PolicyRuleSet	*ruleset_;

	friend class AppPolicy;
	friend class PolicyRuleSet;
};

#endif	/* _POLICYROWPROVIDER_H_ */

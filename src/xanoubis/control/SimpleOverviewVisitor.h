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

#ifndef _SIMPLEOVERVIEWVISITOR_H_
#define _SIMPLEOVERVIEWVISITOR_H_

#include <vector>

#include "PolicyVisitor.h"

class SimpleOverviewRow;

/**
 * Visitor walks through a specific type of AppPolicy.
 *
 * The visitor walks through a specifiy type of AppPolicy-instances
 * (AlfAppPolicy, ContextAppPolicy, SbAppPolicy, SfsAppPolicy) and collects
 * all children of these application-policies. For each filter-policy a
 * SimpleOverviewRow is created.
 */
class SimpleOverviewVisitor : public PolicyVisitor
{
	public:
		/**
		 * Creates a SimpleOverviewVisitor-instance.
		 *
		 * @param classInfo Type of AppPolicy to visit
		 */
		SimpleOverviewVisitor(wxClassInfo *);

		/**
		 * D'tor.
		 */
		~SimpleOverviewVisitor(void);

		/**
		 * Returns the result of the visit-operation.
		 *
		 * The resulting list can be cleared using the clear()-method.
		 *
		 * @return Result of the scan-operation.
		 */
		const std::vector<SimpleOverviewRow *> &getResult(void) const;

		/**
		 * Resets and prepares the visitor for the next scan-operation.
		 *
		 * @param ruleSet The ruleset to scan the next time.
		 */
		void reset(PolicyRuleSet *);

		/**
		 * Clears the resulting list from the last scan operation.
		 */
		void clear(void);

		//@{
		/**
		 * Invoked if an AppPolicy was detected.
		 */
		void visitAlfAppPolicy(AlfAppPolicy *);
		void visitContextAppPolicy(ContextAppPolicy *);
		void visitSbAppPolicy(SbAppPolicy *);
		void visitSfsAppPolicy(SfsAppPolicy *);
		//@}

		//@{
		/**
		 * Invoked if a FilterPolicy was detected.
		 */
		void visitAlfCapabilityFilterPolicy(
		    AlfCapabilityFilterPolicy *);
		void visitAlfFilterPolicy(AlfFilterPolicy *);
		void visitContextFilterPolicy(ContextFilterPolicy *);
		void visitDefaultFilterPolicy(DefaultFilterPolicy *);
		void visitSbAccessFilterPolicy(SbAccessFilterPolicy *);
		void visitSfsFilterPolicy(SfsFilterPolicy *);
		void visitSfsDefaultFilterPolicy(SfsDefaultFilterPolicy *);
		//@}

	private:
		/**
		 * List contains result of scan-operation
		 */
		std::vector<SimpleOverviewRow *> filterList_;

		/**
		 * Type-information about AppPolicy to scan
		 */
		wxClassInfo *classInfo_;

		/**
		 * The currently scanned ruleset.
		 */
		PolicyRuleSet *currentRuleSet_;

		/**
		 * The currently scanned application-policy.
		 */
		AppPolicy *currentApp_;

		/**
		 * Index of currently scanned application-policy.
		 * Increased with each new application.
		 */
		unsigned int appIndex_;

		/**
		 * Index of currently scanned filter-policy.
		 * Increated with each new filter and resetted if a new
		 * application-policy is reached.
		 */
		unsigned int filterIndex_;

		/**
		 * Generic visit-operation for application-policies.
		 */
		void visitAppPolicy(AppPolicy *);

		/**
		 * Generic visit-operation for filter-policies.
		 */
		void visitFilterPolicy(FilterPolicy *);
};

#endif	/* _SIMPLEOVERVIEWVISITOR_H_ */

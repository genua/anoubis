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

#ifndef _MODSFSADDPOLICYVISITOR_H_
#define _MODSFSADDPOLICYVISITOR_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "PolicyVisitor.h"
#include "ModSfsMainPanelImpl.h"

#include "Policy.h"
#include "AlfAppPolicy.h"
#include "AlfCapabilityFilterPolicy.h"
#include "AlfFilterPolicy.h"
#include "ContextAppPolicy.h"
#include "ContextFilterPolicy.h"
#include "DefaultFilterPolicy.h"
#include "SbAccessFilterPolicy.h"
#include "SbAppPolicy.h"
#include "SfsAppPolicy.h"
#include "SfsDefaultFilterPolicy.h"
#include "SfsFilterPolicy.h"

/**
 * PolicyVisitor for ModSfs.
 * This visitor adds the visited policies to the list of ModSfs (where the
 * policies can been viewed).
 */
class ModSfsAddPolicyVisitor : public PolicyVisitor
{
	public:
		/**
		 * Constructor of ModSfsAddPolicyVisitor.
		 * @param 1st The main panel of ModSfs.
		 * @return Nothing.
		 */
		ModSfsAddPolicyVisitor(ModSfsMainPanelImpl *);

		/**
		 * Visit a AlfAppPolicy.
		 * ModSfs does not deal with AlfAppPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfAppPolicy(AlfAppPolicy *);

		/**
		 * Visit a AlfCapabilityFilterPolicy.
		 * ModSfs does not deal with AlfCapabilityFilterPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfCapabilityFilterPolicy(
		    AlfCapabilityFilterPolicy *);

		/**
		 * Visit a AlfFilterPolicy.
		 * ModSfs does not deal with AlfFilterPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfFilterPolicy(AlfFilterPolicy *);

		/**
		 * Visit a ContextAppPolicy.
		 * ModSfs does not deal with ContextAppPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitContextAppPolicy(ContextAppPolicy *);

		/**
		 * Visit a ContextFilterPolicy.
		 * ModSfs does not deal with ContextFilterPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitContextFilterPolicy(
		    ContextFilterPolicy *);

		/**
		 * Visit a DefaultFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitDefaultFilterPolicy(
		    DefaultFilterPolicy *);

		/**
		 * Visit a SbAccessFilterPolicy.
		 * ModSfs does not deal with SbAccessFilterPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSbAccessFilterPolicy(
		    SbAccessFilterPolicy *);

		/**
		 * Visit a SbAppPolicy.
		 * ModSfs does not deal with SbAppPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSbAppPolicy(SbAppPolicy *);

		/**
		 * Visit a SfsAppPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSfsAppPolicy(SfsAppPolicy *);

		/**
		 * Visit a SfsFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSfsFilterPolicy(SfsFilterPolicy *);

		/**
		 * Visit a SfsDefaultFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSfsDefaultFilterPolicy(
		    SfsDefaultFilterPolicy *);

	private:
		ModSfsMainPanelImpl	*sfsPanel_;	/**< The main panel. */

		/**
		 * RuleList append.
		 * This method does the work of adding the given policy to
		 * the wxListCtrl of the main panel of ModSfs.
		 * @param[in] 1st The policy to add.
		 * @return The index within the list of the added policy.
		 */
		long ruleListAppend(Policy *);
};

#endif	/* _MODSFSADDPOLICYVISITOR_H_ */

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

#ifndef _MODSBADDPOLICYVISITOR_H_
#define _MODSBADDPOLICYVISITOR_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "PolicyVisitor.h"
#include "ModSbMainPanelImpl.h"

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

class ModSbAddPolicyVisitor : public PolicyVisitor
{
	public:
		/**
		 * Constructor of ModSbAddPolicyVisitor.
		 * @param 1st The main panel of ModSb.
		 * @return Nothing.
		 */
		ModSbAddPolicyVisitor(ModSbMainPanelImpl *);

		/**
		 * Visit a AlfAppPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfAppPolicy(AlfAppPolicy *);

		/**
		 * Visit a AlfCapabilityFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfCapabilityFilterPolicy(
		    AlfCapabilityFilterPolicy *);

		/**
		 * Visit a AlfFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitAlfFilterPolicy(AlfFilterPolicy *);

		/**
		 * Visit a ContextAppPolicy.
		 * ModAlf does not deal with ContextAppPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitContextAppPolicy(ContextAppPolicy *);

		/**
		 * Visit a ContextFilterPolicy.
		 * ModAlf does not deal with ContextFilterPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitContextFilterPolicy(ContextFilterPolicy *);

		/**
		 * Visit a DefaultFilterPolicy.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitDefaultFilterPolicy(DefaultFilterPolicy *);

		/**
		 * Visit a SbAccessFilterPolicy.
		 * ModAlf does not deal with SbAccessFilterPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSbAccessFilterPolicy(SbAccessFilterPolicy *);

		/**
		 * Visit a SbAppPolicy.
		 * ModAlf does not deal with SbAppPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSbAppPolicy(SbAppPolicy *);

		/**
		 * Visit a SfsAppPolicy.
		 * ModAlf does not deal with SfsAppPolicies.
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSfsAppPolicy(SfsAppPolicy *);

		/**
		 * Visit a SfsFilterPolicy.
		 * ModAlf does not deal with SfsFilterPolicies.
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
		ModSbMainPanelImpl	*sbPanel_;	/**< The main panel. */
};

#endif	/* _MODSBADDPOLICYVISITOR_H_ */

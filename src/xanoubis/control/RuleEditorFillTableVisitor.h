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

#ifndef _RULEEDITORFILLTABLEVISITOR_H_
#define _RULEEDITORFILLTABLEVISITOR_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "PolicyVisitor.h"
#include "DlgRuleEditor.h"

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
#include "SfsFilterPolicy.h"

/**
 * PolicyVisitor for RuleEditor.
 * This visitor clears the selected line of wxListCtrl and fills
 * it with the corresponding / updated content of the visited policy.
 * The default behaviour of this visitor is not to propagate.
 */
class RuleEditorFillTableVisitor : public PolicyVisitor
{
	public:
		/**
		 * Constructor of RuleEditorFillTableVisitor.
		 * @param[in] 1st The rule editor.
		 * @param[in] 2nd The selected line.
		 * @return Nothing.
		 */
		RuleEditorFillTableVisitor(DlgRuleEditor *, long);

		/*
		 * XXX ch: this will be fixed with the next functionality change
		 */

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
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitContextAppPolicy(ContextAppPolicy *);

		/**
		 * Visit a ContextFilterPolicy.
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
		 * @param[in] 1st Policy to visit.
		 */
		virtual void visitSbAccessFilterPolicy(SbAccessFilterPolicy *);

		/**
		 * Visit a SbAppPolicy.
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

	protected:
		long		 selectedLine_;	/**< Modify this line. */
		DlgRuleEditor	*ruleEditor_;	/**< Keep the RuleEditor. */

		/**
		 * Clear this line.
		 * @param[in] 1st The new policy for this line.
		 * @param[in] 2nd The line to modify.
		 */
		void clean(Policy *, long);

		/**
		 * Fill table with AppPolicy.
		 * @param[in] 1st The policy providing content for the table.
		 * @param[in] 2nd The line to modify.
		 */
		void showAppPolicy(AppPolicy *, long);

		/**
		 * Fill table with AlfCapabilityFilterPolicy.
		 * @param[in] 1st The policy providing content for the table.
		 * @param[in] 2nd The line to modify.
		 */
		void showAlfCapabilityFilterPolicy(AlfCapabilityFilterPolicy *,
		    long);

		/**
		 * Fill table with AlfFilterPolicy.
		 * @param[in] 1st The policy providing content for the table.
		 * @param[in] 2nd The line to modify.
		 */
		void showAlfFilterPolicy(AlfFilterPolicy *, long);

		/**
		 * Fill table with ContextFilterPolicy.
		 * @param[in] 1st The policy providing content for the table.
		 * @param[in] 2nd The line to modify.
		 */
		void showContextFilterPolicy(ContextFilterPolicy *, long);

		/**
		 * Fill table with DefaultFilterPolicy.
		 * @param[in] 1st The policy providing content for the table.
		 * @param[in] 2nd The line to modify.
		 */
		void showDefaultFilterPolicy(DefaultFilterPolicy *, long);

		/**
		 * Fill table with SbAccessFilterPolicy.
		 * @param[in] 1st The policy providing content for the table.
		 * @param[in] 2nd The line to modify.
		 */
		void showSbAccessFilterPolicy(SbAccessFilterPolicy *, long);

		/**
		 * Fill table with SfsFilterPolicy.
		 * @param[in] 1st The policy providing content for the table.
		 * @param[in] 2nd The line to modify.
		 */
		void showSfsFilterPolicy(SfsFilterPolicy *, long);
};

#endif	/* _RULEEDITORFILLTABLEVISITOR_H_ */

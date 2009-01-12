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

#ifndef _RULEEDITORFILLWIDGETSVISITOR_H_
#define _RULEEDITORFILLWIDGETSVISITOR_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "DlgRuleEditor.h"
#include "PolicyVisitor.h"

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
 * PolicyVistior for RuleEditor.
 * This visitor fills the widgets of the rule editor.
 * As the RuleEditor has to be adapted this visitor is fixed
 * within that change.
 * XXX ch: fix this in the RuleEditor change see Bug #964
 */
class RuleEditorFillWidgetsVisitor : public PolicyVisitor
{
	private:
		DlgRuleEditor	*ruleEditor_;
		/*
		 *
		 * XXX ch: this will be fixed with the next functionality change
		 *
		void clear(void);
		void showAction(int);
		void showLog(int);
		void showProtocol(int);
		void showAddrFamily(int);
		void showCapability(int);
		void showSrcAddr(wxArrayString);
		void showDstAddr(wxArrayString);
		void showSrcPort(wxString);
		void showDstPort(wxString);
		void showDirection(int);
		void showUser(uid_t);
		*/
	public:
		RuleEditorFillWidgetsVisitor(DlgRuleEditor *);

/*
 * XXX ch: this will be fixed with the next functionality change
 */

		virtual void visitAlfAppPolicy(AlfAppPolicy*) {};
		virtual void visitAlfCapabilityFilterPolicy(AlfCapabilityFilterPolicy*) {};
		virtual void visitAlfFilterPolicy(AlfFilterPolicy*) {};
		virtual void visitContextAppPolicy(ContextAppPolicy*) {};
		virtual void visitContextFilterPolicy(ContextFilterPolicy*) {};
		virtual void visitDefaultFilterPolicy(DefaultFilterPolicy*) {};
		virtual void visitSbAccessFilterPolicy(SbAccessFilterPolicy*) {};
		virtual void visitSbAppPolicy(SbAppPolicy*) {};
		virtual void visitSfsAppPolicy(SfsAppPolicy*) {};
		virtual void visitSfsFilterPolicy(SfsFilterPolicy*) {};
};

#endif	/* _RULEEDITORFILLWIDGETSVISITOR_H_ */

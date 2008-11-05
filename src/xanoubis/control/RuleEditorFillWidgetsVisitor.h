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

#include "DlgRuleEditor.h"
#include "PolicyVisitor.h"

class RuleEditorFillWidgetsVisitor : public PolicyVisitor
{
	private:
		DlgRuleEditor	*ruleEditor_;

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

	public:
		RuleEditorFillWidgetsVisitor(DlgRuleEditor *);
		~RuleEditorFillWidgetsVisitor(void);

		virtual void visitAppPolicy(AppPolicy *);
		virtual void visitAlfPolicy(AlfPolicy *);
		virtual void visitCtxPolicy(CtxPolicy *);
		virtual void visitSfsPolicy(SfsPolicy *);
		virtual void visitVarPolicy(VarPolicy *);
};

#endif	/* _RULEEDITORFILLWIDGETSVISITOR_H_ */

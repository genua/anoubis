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

#include "RuleEditorAddPolicyVisitor.h"
#include "main.h"

RuleEditorAddPolicyVisitor::RuleEditorAddPolicyVisitor(
    DlgRuleEditor *ruleEditor) : RuleEditorFillTableVisitor(ruleEditor, 0)
{
	setPropagation(true);
}

RuleEditorAddPolicyVisitor::~RuleEditorAddPolicyVisitor(void)
{
}

/*
 * XXX ch: this will be fixed with the next functionality change
 */
#if 0
long
RuleEditorAddPolicyVisitor::appendPolicy(Policy *policy)
{
	long		 idx;
	wxString	 ruleType;
	PolicyRuleSet	*rs;

	idx = ruleEditor_->ruleListCtrl->GetItemCount();
	ruleType = wxString::Format(wxT("%d"), policy->getId());

	ruleEditor_->ruleListCtrl->InsertItem(idx, ruleType);
	ruleEditor_->ruleListCtrl->SetItemPtrData(idx, (wxUIntPtr)policy);

	rs = policy->getRsParent();
	if (rs != NULL) {
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_USER,
		    wxGetApp().getUserNameById(rs->getUid()));

	}

	if (isAdmin()) {
		ruleEditor_->ruleListCtrl->SetItemBackgroundColour(idx,
		    wxTheColourDatabase->Find(wxT("LIGHT GREY")));
	}

	return (idx);
}

void
RuleEditorAddPolicyVisitor::visitAppPolicy(AppPolicy *appPolicy)
{
	/* Never visit the SFS dummy application block itself! */
	if (appPolicy->getType() == APN_SFS)
		return;
	if (appPolicy->getType() == APN_ALF
	    || appPolicy->getType() == APN_CTX) {
		showApp(appPolicy, appendPolicy(appPolicy));
	}
}

void
RuleEditorAddPolicyVisitor::visitAlfPolicy(AlfPolicy *alfPolicy)
{
	showAlf(alfPolicy, appendPolicy(alfPolicy));
}

void
RuleEditorAddPolicyVisitor::visitCtxPolicy(CtxPolicy *ctxPolicy)
{
	showCtx(ctxPolicy, appendPolicy(ctxPolicy));
}

void
RuleEditorAddPolicyVisitor::visitSfsPolicy(SfsPolicy *sfsPolicy)
{
	showSfs(sfsPolicy, appendPolicy(sfsPolicy));
}

void
RuleEditorAddPolicyVisitor::visitVarPolicy(VarPolicy *variable)
{
	 showVar(variable, appendPolicy(variable));
}
#endif

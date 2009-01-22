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

#include "RuleEditorFillTableVisitor.h"
#include "main.h"

#define ADMIN_FLAG	wxT("(A)")

RuleEditorFillTableVisitor::RuleEditorFillTableVisitor(
    DlgRuleEditor *ruleEditor, long selectedLine)
{
	ruleEditor_ = ruleEditor;
	selectedLine_ = selectedLine;
	setPropagation(false);
}

void
RuleEditorFillTableVisitor::visitAlfAppPolicy(AlfAppPolicy *policy)
{
	showAppPolicy(policy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitAlfCapabilityFilterPolicy(
    AlfCapabilityFilterPolicy *policy)
{
	showAlfCapabilityFilterPolicy(policy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitAlfFilterPolicy(AlfFilterPolicy *policy)
{
	showAlfFilterPolicy(policy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitContextAppPolicy(ContextAppPolicy *policy)
{
	showAppPolicy(policy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitContextFilterPolicy(
    ContextFilterPolicy *policy)
{
	showContextFilterPolicy(policy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitDefaultFilterPolicy(
    DefaultFilterPolicy *policy)
{
	showDefaultFilterPolicy(policy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitSbAccessFilterPolicy(
    SbAccessFilterPolicy *policy)
{
	showSbAccessFilterPolicy(policy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitSbAppPolicy(SbAppPolicy *policy)
{
	showAppPolicy(policy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitSfsAppPolicy(SfsAppPolicy *policy)
{
	showAppPolicy(policy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitSfsFilterPolicy(SfsFilterPolicy *policy)
{
	showSfsFilterPolicy(policy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitSfsDefaultFilterPolicy(
    SfsDefaultFilterPolicy *)
{
}

void
RuleEditorFillTableVisitor::clean(Policy *WXUNUSED(policy), long WXUNUSED(idx))
{
/*
 * XXX ch: this will be fixed with the next functionality change
 */
#if 0
	for (int i=0; i<RULEDITOR_LIST_COLUMN_EOL; i++) {
		ruleEditor_->ruleListCtrl->SetItem(idx, i, wxEmptyString);
	}

	if (policy->getParentRuleSet()->isAdmin()) {
		ruleEditor_->ruleListCtrl->SetItemBackgroundColour(idx,
		    wxTheColourDatabase->Find(wxT("LIGHT GREY")));
	}

	if (policy->hasScope()) {
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_SCOPE, policy->getScopeName());
	}

	ruleEditor_->ruleListCtrl->SetItem(idx, 0,
	    wxString::Format(wxT("%d"), idx));
#endif
}

void
RuleEditorFillTableVisitor::showAppPolicy(AppPolicy *WXUNUSED(policy),
    long WXUNUSED(idx))
{
/*
 * XXX ch: this will be fixed with the next functionality change
 */
#if 0
	wxString	 ruleType;
	PolicyRuleSet	*ruleset;

	clean(policy, idx);
	policy->setRuleEditorIndex(idx);

	ruleType = policy->getTypeIdentifier();

	ruleset = policy->getParentRuleSet();
	if (ruleset != NULL) {
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_USER,
		    wxGetApp().getUserNameById(ruleset->getUid()));
	}
	if ((ruleset != NULL) && ruleset->isAdmin()) {
		ruleType += ADMIN_FLAG;
	}

	/* XXX ch: this does not work for lists */
	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_RULE,
	    ruleType);
	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_BIN,
	    policy->getBinaryName());
	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_HASHT,
	   policy->getHashTypeName(0));
	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_HASH,
	   policy->getHashValueName(0));
#endif
}

void
RuleEditorFillTableVisitor::showAlfCapabilityFilterPolicy(
    AlfCapabilityFilterPolicy *WXUNUSED(policy), long WXUNUSED(idx))
{
/*
 * XXX ch: this will be fixed with the next functionality change
 */
#if 0
	wxString	 ruleType;
	wxListCtrl	*list;
	PolicyRuleSet	*ruleset;

	clean(policy, idx);
	policy->setRuleEditorIndex(idx);

	list = ruleEditor_->ruleListCtrl;
	ruleset = policy->getParentRuleSet();
	if (ruleset != NULL) {
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_USER,
		    wxGetApp().getUserNameById(ruleset->getUid()));
	}

	ruleType = policy->getTypeIdentifier();
	if ((ruleset != NULL) && ruleset->isAdmin()) {
		ruleType += ADMIN_FLAG;
	}
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_RULE, ruleType);

	list->SetItem(idx, RULEDITOR_LIST_COLUMN_TYPE,
	    policy->getTypeIdentifier());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_LOG, policy->getLogName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_ACTION,
	    policy->getActionName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_CAP,
	    policy->getCapabilityTypeName());
#endif
}

void
RuleEditorFillTableVisitor::showAlfFilterPolicy(
    AlfFilterPolicy *WXUNUSED(policy), long WXUNUSED(idx))
{
/*
 * XXX ch: this will be fixed with the next functionality change
 */
#if 0
	wxString	 ruleType;
	wxListCtrl	*list;
	PolicyRuleSet	*ruleset;

	clean(policy, idx);
	policy->setRuleEditorIndex(idx);

	list = ruleEditor_->ruleListCtrl;
	ruleset = policy->getParentRuleSet();
	if (ruleset != NULL) {
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_USER,
		    wxGetApp().getUserNameById(ruleset->getUid()));
	}

	ruleType = policy->getTypeIdentifier();
	if ((ruleset != NULL) && ruleset->isAdmin()) {
		ruleType += ADMIN_FLAG;
	}
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_RULE, ruleType);

	list->SetItem(idx, RULEDITOR_LIST_COLUMN_TYPE,
	    policy->getTypeIdentifier());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_ACTION,
	    policy->getActionName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_LOG, policy->getLogName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_AF,
	    policy->getAddrFamilyName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_PROTO,
	    policy->getProtocolName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_DIR,
	    policy->getDirectionName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_FHOST,
	    policy->getFromHostName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_FPORT,
	    policy->getFromPortName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_THOST,
	    policy->getToHostName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_TPORT,
	    policy->getToPortName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_STATETIMEOUT,
	    policy->getStateTimeoutName());
#endif
}

void
RuleEditorFillTableVisitor::showContextFilterPolicy(
    ContextFilterPolicy *WXUNUSED(policy), long WXUNUSED(idx))
{
/*
 * XXX ch: this will be fixed with the next functionality change
 */
#if 0
	wxString	 ruleType;
	wxListCtrl	*list;
	PolicyRuleSet	*ruleset;

	clean(policy, idx);
	policy->setRuleEditorIndex(idx);

	list = ruleEditor_->ruleListCtrl;
	ruleset = policy->getParentRuleSet();
	if (ruleset != NULL) {
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_USER,
		    wxGetApp().getUserNameById(ruleset->getUid()));
	}

	ruleType = policy->getTypeIdentifier();
	if ((ruleset != NULL) && ruleset->isAdmin()) {
		ruleType += ADMIN_FLAG;
	}
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_RULE, ruleType);
	/* XXX ch: this is fixed in the RuleEditor change
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_BIN, policy->getBinaryName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_HASHT,
	    policy->getHashTypeName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_HASH, policy->getHashValue());
	*/
#endif
}

void
RuleEditorFillTableVisitor::showDefaultFilterPolicy(
    DefaultFilterPolicy *WXUNUSED(policy), long WXUNUSED(idx))
{
/*
 * XXX ch: this will be fixed with the next functionality change
 */
#if 0
	wxString	 ruleType;
	wxListCtrl	*list;
	PolicyRuleSet	*ruleset;

	clean(policy, idx);
	policy->setRuleEditorIndex(idx);

	list = ruleEditor_->ruleListCtrl;
	ruleset = policy->getParentRuleSet();
	if (ruleset != NULL) {
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_USER,
		    wxGetApp().getUserNameById(ruleset->getUid()));
	}

	ruleType = policy->getTypeIdentifier();
	if ((ruleset != NULL) && ruleset->isAdmin()) {
		ruleType += ADMIN_FLAG;
	}
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_RULE, ruleType);

	list->SetItem(idx, RULEDITOR_LIST_COLUMN_TYPE,
	    policy->getTypeIdentifier());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_LOG, policy->getLogName());
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_ACTION,
	    policy->getActionName());
#endif
}

void
RuleEditorFillTableVisitor::showSbAccessFilterPolicy(
    SbAccessFilterPolicy *WXUNUSED(policy), long WXUNUSED(idx))
{
/*
 * XXX ch: this will be fixed with the next functionality change
 */
#if 0
	wxString	 ruleType;
	wxListCtrl	*list;
	PolicyRuleSet	*ruleset;

	clean(policy, idx);
	policy->setRuleEditorIndex(idx);

	list = ruleEditor_->ruleListCtrl;
	ruleset = policy->getParentRuleSet();
	if (ruleset != NULL) {
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_USER,
		    wxGetApp().getUserNameById(ruleset->getUid()));
	}

	ruleType = policy->getTypeIdentifier();
	if ((ruleset != NULL) && ruleset->isAdmin()) {
		ruleType += ADMIN_FLAG;
	}
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_RULE, ruleType);
#endif
}

void
RuleEditorFillTableVisitor::showSfsFilterPolicy(
    SfsFilterPolicy *WXUNUSED(policy), long WXUNUSED(idx))
{
/*
 * XXX ch: this will be fixed with the next functionality change
 */
#if 0
	wxString	 ruleType;
	wxListCtrl	*list;
	PolicyRuleSet	*ruleset;

	clean(policy, idx);
	policy->setRuleEditorIndex(idx);

	list = ruleEditor_->ruleListCtrl;
	ruleset = policy->getParentRuleSet();
	if (ruleset != NULL) {
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_USER,
		    wxGetApp().getUserNameById(ruleset->getUid()));
	}

	ruleType = policy->getTypeIdentifier();
	if ((ruleset != NULL) && ruleset->isAdmin()) {
		ruleType += ADMIN_FLAG;
	}
	list->SetItem(idx, RULEDITOR_LIST_COLUMN_RULE, ruleType);
#endif
}

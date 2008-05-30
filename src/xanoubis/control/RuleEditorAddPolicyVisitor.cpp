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

RuleEditorAddPolicyVisitor::RuleEditorAddPolicyVisitor(DlgRuleEditor *ruleEditor)
{
	ruleEditor_ = ruleEditor;
}

RuleEditorAddPolicyVisitor::~RuleEditorAddPolicyVisitor(void)
{
}

long
RuleEditorAddPolicyVisitor::appendPolicy(Policy *policy)
{
	long idx;

	idx = ruleEditor_->ruleListCtrl->GetItemCount();
	ruleEditor_->ruleListCtrl->InsertItem(idx, wxString::Format(wxT("%d"),
	    idx));

	ruleEditor_->ruleListCtrl->SetItemPtrData(idx, (wxUIntPtr)policy);

	return (idx);
}

void
RuleEditorAddPolicyVisitor::visitAppPolicy(AppPolicy *appPolicy)
{
	long idx;

	if (appPolicy->getType() == APN_ALF) {
		idx = appendPolicy(appPolicy);

		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_APP, appPolicy->getBinaryName());
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_RULE, _("APP"));
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_BIN, appPolicy->getBinaryName());
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_HASHT, appPolicy->getHashTypeName());
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_HASH, appPolicy->getHashValue());

	}
}

void
RuleEditorAddPolicyVisitor::visitAlfPolicy(AlfPolicy *alfPolicy)
{
	long		 idx;
	int		 type;
	wxListCtrl	*list;

	idx = appendPolicy(alfPolicy);
	type = alfPolicy->getTypeNo();
	list = ruleEditor_->ruleListCtrl;

	list->SetItem(idx, RULEDITOR_LIST_COLUMN_RULE,_("ALF"));

	switch (type) {
	case APN_ALF_FILTER:
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_TYPE,
		    alfPolicy->getTypeName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_ACTION,
		    alfPolicy->getActionName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_LOG,
		    alfPolicy->getLogName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_AF,
		    alfPolicy->getAddrFamilyName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_PROTO,
		    alfPolicy->getProtocolName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_DIR,
		    alfPolicy->getDirectionName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_FHOST,
		    alfPolicy->getFromHostName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_FPORT,
		    alfPolicy->getFromPortName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_THOST,
		    alfPolicy->getToHostName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_TPORT,
		    alfPolicy->getToPortName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_STATETIMEOUT,
		    alfPolicy->getStateTimeout());
		break;
	case APN_ALF_CAPABILITY:
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_TYPE,
		    alfPolicy->getTypeName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_LOG,
		    alfPolicy->getLogName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_ACTION,
		    alfPolicy->getActionName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_CAP,
		    alfPolicy->getCapTypeName());
		break;
	case APN_ALF_DEFAULT:
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_TYPE,
		    alfPolicy->getTypeName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_LOG,
		    alfPolicy->getLogName());
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_ACTION,
		    alfPolicy->getActionName());
		break;
	case APN_ALF_CTX:
		list->SetItem(idx, RULEDITOR_LIST_COLUMN_CTX,
		    alfPolicy->getContextName());
		break;
	default:
		break;
	}
}

void
RuleEditorAddPolicyVisitor::visitSfsPolicy(SfsPolicy *sfsPolicy)
{
	long idx;

	idx = appendPolicy(sfsPolicy);

	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_RULE,
			_("SFS"));
	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_BIN,
			sfsPolicy->getBinaryName());
	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_HASHT,
			sfsPolicy->getHashTypeName());
	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_HASH,
			sfsPolicy->getHashValue());

}


void
RuleEditorAddPolicyVisitor::visitVarPolicy(VarPolicy *variable)
{
}

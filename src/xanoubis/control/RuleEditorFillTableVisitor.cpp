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

RuleEditorFillTableVisitor::RuleEditorFillTableVisitor(
    DlgRuleEditor *ruleEditor, long selectedLine)
{
	ruleEditor_ = ruleEditor;
	selectedLine_ = selectedLine;
	setPropagation(false);
}

RuleEditorFillTableVisitor::~RuleEditorFillTableVisitor(void)
{
}

void
RuleEditorFillTableVisitor::clean(long idx)
{
	for (int i=0; i<RULEDITOR_LIST_COLUMN_EOL; i++) {
		ruleEditor_->ruleListCtrl->SetItem(idx, i, wxEmptyString);
	}
	ruleEditor_->ruleListCtrl->SetItem(idx, 0,
	    wxString::Format(wxT("%d"), idx));
}

void
RuleEditorFillTableVisitor::showApp(AppPolicy *appPolicy, long idx)
{
	clean(idx);
	if (appPolicy->getType() == APN_ALF) {
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_APP, appPolicy->getApplicationName());
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_RULE, wxT("APP"));
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_BIN, appPolicy->getBinaryName());
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_HASHT, appPolicy->getHashTypeName());
		ruleEditor_->ruleListCtrl->SetItem(idx,
		    RULEDITOR_LIST_COLUMN_HASH, appPolicy->getHashValue());
	}
}

void
RuleEditorFillTableVisitor::showAlf(AlfPolicy *alfPolicy, long idx)
{
	int		 type;
	wxListCtrl	*list;

	type = alfPolicy->getTypeNo();
	list = ruleEditor_->ruleListCtrl;

	clean(idx);

	list->SetItem(idx, RULEDITOR_LIST_COLUMN_RULE, wxT("ALF"));
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
RuleEditorFillTableVisitor::showSfs(SfsPolicy *sfsPolicy, long idx)
{
	clean(idx);

	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_RULE,
	    wxT("SFS"));
	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_BIN,
	    sfsPolicy->getBinaryName());
	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_HASHT,
	    sfsPolicy->getHashTypeName());
	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_HASH,
	    sfsPolicy->getHashValue());
	ruleEditor_->ruleListCtrl->SetItem(idx, RULEDITOR_LIST_COLUMN_LOG,
	    sfsPolicy->getLogName());
}

void
RuleEditorFillTableVisitor::showVar(VarPolicy *, long)
{
	/* XXX ch: vars currently not supported */
}

void
RuleEditorFillTableVisitor::visitAppPolicy(AppPolicy *appPolicy)
{
	showApp(appPolicy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitAlfPolicy(AlfPolicy *alfPolicy)
{
	showAlf(alfPolicy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitSfsPolicy(SfsPolicy *sfsPolicy)
{
	showSfs(sfsPolicy, selectedLine_);
}

void
RuleEditorFillTableVisitor::visitVarPolicy(VarPolicy *varPolicy)
{
	showVar(varPolicy, selectedLine_);
}

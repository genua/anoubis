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

#include "ModSfsAddPolicyVisitor.h"

ModSfsAddPolicyVisitor::ModSfsAddPolicyVisitor(ModSfsMainPanelImpl *sfsPanel)
{
	sfsPanel_ = sfsPanel;
}

void
ModSfsAddPolicyVisitor::visitAlfAppPolicy(AlfAppPolicy *)
{
	/* ModSfs does not deal with AlfAppPolicies. */
}

void
ModSfsAddPolicyVisitor::visitAlfCapabilityFilterPolicy(
		    AlfCapabilityFilterPolicy *)
{
	/* ModSfs does not deal with AlfCapabilityFilterPolicies. */
}

void
ModSfsAddPolicyVisitor::visitAlfFilterPolicy(AlfFilterPolicy *)
{
	/* ModSfs does not deal with AlfFilterPolicies. */
}

void
ModSfsAddPolicyVisitor::visitContextAppPolicy(ContextAppPolicy *)
{
	/* ModSfs does not deal with ContextAppPolicies. */
}

void
ModSfsAddPolicyVisitor::visitContextFilterPolicy(
		    ContextFilterPolicy *)
{
	/* ModSfs does not deal with ContextFilterPolicies. */
}

void
ModSfsAddPolicyVisitor::visitDefaultFilterPolicy(DefaultFilterPolicy *policy)
{
	long		 idx;
	wxListCtrl	*list;

	idx = ruleListAppend(policy);
	list = sfsPanel_->lst_Rules;

	/*
	 * XXX ch: fix this with next change
	list->SetItem(idx, MODSFS_LIST_COLUMN_PROG,
	    sfsPolicy->getBinaryName());
	list->SetItem(idx, MODSFS_LIST_COLUMN_HASHT,
	    sfsPolicy->getHashTypeName());
	list->SetItem(idx, MODSFS_LIST_COLUMN_HASH,
	   sfsPolicy->getHashValue());
	*/
}

void
ModSfsAddPolicyVisitor::visitSbAccessFilterPolicy(
		    SbAccessFilterPolicy *)
{
	/* ModSfs does not deal with SbAccessFilterPolicies. */
}

void
ModSfsAddPolicyVisitor::visitSbAppPolicy(SbAppPolicy *)
{
	/* ModSfs does not deal with SbAppPolicies. */
}

void
ModSfsAddPolicyVisitor::visitSfsAppPolicy(SfsAppPolicy *policy)
{
	long		 idx;
	wxListCtrl	*list;

	idx = ruleListAppend(policy);
	list = sfsPanel_->lst_Rules;

	/*
	 * XXX ch: fix this with next change
	list->SetItem(idx, MODSFS_LIST_COLUMN_PROG,
	    sfsPolicy->getBinaryName());
	list->SetItem(idx, MODSFS_LIST_COLUMN_HASHT,
	    sfsPolicy->getHashTypeName());
	list->SetItem(idx, MODSFS_LIST_COLUMN_HASH,
	   sfsPolicy->getHashValue());
	*/
}

void
ModSfsAddPolicyVisitor::visitSfsFilterPolicy(SfsFilterPolicy *policy)
{
	long		 idx;
	wxListCtrl	*list;

	idx = ruleListAppend(policy);
	list = sfsPanel_->lst_Rules;

	/*
	 * XXX ch: fix this with next change
	list->SetItem(idx, MODSFS_LIST_COLUMN_PROG,
	    sfsPolicy->getBinaryName());
	list->SetItem(idx, MODSFS_LIST_COLUMN_HASHT,
	    sfsPolicy->getHashTypeName());
	list->SetItem(idx, MODSFS_LIST_COLUMN_HASH,
	   sfsPolicy->getHashValue());
	*/
}

long
ModSfsAddPolicyVisitor::ruleListAppend(Policy *policy)
{
	long idx;

	idx = sfsPanel_->lst_Rules->GetItemCount();
	sfsPanel_->lst_Rules->InsertItem(idx, wxString::Format(wxT("%d"),
	    idx));

	sfsPanel_->lst_Rules->SetItemPtrData(idx, (wxUIntPtr)policy);

	return (idx);
}

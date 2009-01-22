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

#include "ModAlfAddPolicyVisitor.h"

#include "main.h"
#include "PolicyRuleSet.h"

ModAlfAddPolicyVisitor::ModAlfAddPolicyVisitor(ModAlfMainPanelImpl *alfPanel)
{
	alfPanel_ = alfPanel;
}

void
ModAlfAddPolicyVisitor::visitAlfAppPolicy(AlfAppPolicy *policy)
{
	long		 idx;
	wxListCtrl	*list;

	idx = ruleListAppend(policy);
	list = alfPanel_->lst_Rules;

	list->SetItem(idx, MODALF_LIST_COLUMN_PROG, policy->getBinaryName());
}

void
ModAlfAddPolicyVisitor::visitAlfCapabilityFilterPolicy(
    AlfCapabilityFilterPolicy *policy)
{
	long		 idx;
	wxListCtrl	*list;

	idx = ruleListAppend(policy);
	list = alfPanel_->lst_Rules;

	list->SetItem(idx, MODALF_LIST_COLUMN_ACTION, policy->getActionName());
	list->SetItem(idx, MODALF_LIST_COLUMN_SERVICE,
	    wxT("capability ") + policy->getCapabilityTypeName());
}

void
ModAlfAddPolicyVisitor::visitAlfFilterPolicy(AlfFilterPolicy *policy)
{
	long		 idx;
	wxListCtrl	*list;

	idx = ruleListAppend(policy);
	list = alfPanel_->lst_Rules;

	list->SetItem(idx, MODALF_LIST_COLUMN_ACTION, policy->getActionName());
	list->SetItem(idx, MODALF_LIST_COLUMN_ROLE,policy->getRoleName());
	list->SetItem(idx, MODALF_LIST_COLUMN_SERVICE,
	    policy->getServiceName());
}

void
ModAlfAddPolicyVisitor::visitContextAppPolicy(ContextAppPolicy *)
{
	/* ModAlf does not deal with ContextAppPolicies. */
}

void
ModAlfAddPolicyVisitor::visitContextFilterPolicy(
    ContextFilterPolicy *)
{
	/* ModAlf does not deal with ContextFilterPolicies. */
}


void
ModAlfAddPolicyVisitor::visitDefaultFilterPolicy(DefaultFilterPolicy *policy)
{
	long		 idx;
	wxListCtrl	*list;

	idx = ruleListAppend(policy);
	list = alfPanel_->lst_Rules;

	list->SetItem(idx, MODALF_LIST_COLUMN_ACTION, policy->getActionName());
	list->SetItem(idx, MODALF_LIST_COLUMN_SERVICE, wxT("default"));
}

void
ModAlfAddPolicyVisitor::visitSbAccessFilterPolicy(SbAccessFilterPolicy *)
{
	/* ModAlf does not deal with SbAccessFilterPolicies. */
}

void
ModAlfAddPolicyVisitor::visitSbAppPolicy(SbAppPolicy *)
{
	/* ModAlf does not deal with SbAppPolicies. */
}

void
ModAlfAddPolicyVisitor::visitSfsAppPolicy(SfsAppPolicy *)
{
	/* ModAlf does not deal with SfsAppPolicies. */
}

void
ModAlfAddPolicyVisitor::visitSfsFilterPolicy(SfsFilterPolicy *)
{
	/* ModAlf does not deal with SfsFilterPolicies. */
}

void
ModAlfAddPolicyVisitor::visitSfsDefaultFilterPolicy(SfsDefaultFilterPolicy *)
{
	/* ModAlf does not deal with SfsFilterPolicies. */
}

long
ModAlfAddPolicyVisitor::ruleListAppend(Policy *policy)
{
	long		 idx;
	wxString	 ruleType;
	wxString	 userName;
	PolicyRuleSet	*ruleset;

	idx = alfPanel_->lst_Rules->GetItemCount();
	alfPanel_->lst_Rules->InsertItem(idx, wxString::Format(wxT("%d"),
	    idx));

	alfPanel_->lst_Rules->SetItemPtrData(idx, (wxUIntPtr)policy);

	ruleset = policy->getParentRuleSet();
	if (ruleset->isAdmin()) {
		userName = wxGetApp().getUserNameById(ruleset->getUid());
		ruleType.Printf(_("admin ruleset of %s"), userName.c_str());
		alfPanel_->lst_Rules->SetItemBackgroundColour(idx,
		    wxTheColourDatabase->Find(wxT("LIGHT GREY")));
	} else {
		ruleType = wxGetUserId();
	}
	alfPanel_->lst_Rules->SetItem(idx, MODALF_LIST_COLUMN_ADMIN, ruleType);

	if (policy->hasScope()) {
		alfPanel_->lst_Rules->SetItem(idx, MODALF_LIST_COLUMN_SCOPE,
		    wxT("T"));
	}

	return (idx);
}

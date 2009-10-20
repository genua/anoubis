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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "AnEvents.h"
#include "main.h"
#include "ModSbAddPolicyVisitor.h"
#include "ModSbMainPanelImpl.h"
#include "Policy.h"
#include "PolicyRuleSet.h"
#include "PolicyCtrl.h"

ModSbMainPanelImpl::ModSbMainPanelImpl(wxWindow* parent,
    wxWindowID id) : Observer(NULL), ModSbMainPanelBase(parent, id)
{
	columnNames_[COLUMN_PROG] = _("Program");
	columnNames_[COLUMN_PATH] = _("Path");
	columnNames_[COLUMN_SUB] = _("Subject");
	columnNames_[COLUMN_ACTION] = _("Action");
	columnNames_[COLUMN_MASK] = _("Mask");
	columnNames_[COLUMN_SCOPE] = _("Temporary");
	columnNames_[COLUMN_USER] = _("User");

	userRuleSetId_ = -1;
	adminRuleSetId_ = -1;

	for (int i=0; i<COLUMN_EOL; i++) {
		lst_Rules->InsertColumn(i, columnNames_[i], wxLIST_FORMAT_LEFT,
		     wxLIST_AUTOSIZE);
	}

	/*
	 * Adjust column width (Bug #1321).
	 * Tablewidth - sum of fix columns = remaining size.
	 * 758 - (90 + 70 + 50 + 90 + 50) = 408 / 2 = 204
	 */
	lst_Rules->SetColumnWidth(COLUMN_PROG,		204);
	lst_Rules->SetColumnWidth(COLUMN_PATH,		204);
	lst_Rules->SetColumnWidth(COLUMN_SUB,		 90);
	lst_Rules->SetColumnWidth(COLUMN_ACTION,	 70);
	lst_Rules->SetColumnWidth(COLUMN_MASK,		 50);
	lst_Rules->SetColumnWidth(COLUMN_SCOPE,		 90);
	lst_Rules->SetColumnWidth(COLUMN_USER,		 50);

	AnEvents::getInstance()->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModSbMainPanelImpl::onLoadRuleSet),
	    NULL, this);
}

ModSbMainPanelImpl::~ModSbMainPanelImpl()
{
	AnEvents::getInstance()->Disconnect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModSbMainPanelImpl::onLoadRuleSet),
	    NULL, this);
}

void
ModSbMainPanelImpl::addDefaultFilterPolicy(DefaultFilterPolicy *policy)
{
	long	idx;

	idx = ruleListAppend(policy);
	updateDefaultFilterPolicy(idx);
}

void
ModSbMainPanelImpl::addSbAccessFilterPolicy(SbAccessFilterPolicy *policy)
{
	long	idx;

	idx = ruleListAppend(policy);
	updateSbAccessFilterPolicy(idx);
}

void
ModSbMainPanelImpl::addSbAppPolicy(SbAppPolicy *policy)
{
	long	idx;

	idx = ruleListAppend(policy);
	updateSbAppPolicy(idx);
}

void
ModSbMainPanelImpl::update(Subject *subject)
{
	long		idx;
	AppPolicy	*parent;
	FilterPolicy	*filter;

	if (subject->IsKindOf(CLASSINFO(SbAccessFilterPolicy))) {
		idx = findListRow((Policy *)subject);
		if (idx != -1)
			updateSbAccessFilterPolicy(idx);
	} else if (subject->IsKindOf(CLASSINFO(SbAppPolicy))) {
		idx = findListRow((Policy *)subject);
		if (idx != -1)
			updateSbAppPolicy(idx);
	} else if (subject->IsKindOf(CLASSINFO(DefaultFilterPolicy))) {
		idx = findListRow((Policy *)subject);
		filter = wxDynamicCast(subject, FilterPolicy);
		parent = filter->getParentPolicy();
		if ((idx != -1) && (parent != NULL)) {
			if (parent->IsKindOf(CLASSINFO(SbAppPolicy)))
				updateDefaultFilterPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(PolicyRuleSet))) {
		if (lst_Rules->GetItemCount() == 0)
			updateShowRuleset();
	} else {
		/* Unknown subject type - do nothing */
	}
}

void
ModSbMainPanelImpl::updateDelete(Subject *subject)
{
	long	idx = -1;
	idx = findListRow((Policy *)subject);

	if (idx != -1) {
		removeListRow(idx);
	}
}

void
ModSbMainPanelImpl::onLoadRuleSet(wxCommandEvent& event)
{
	ModSbAddPolicyVisitor	addVisitor(this);
	PolicyRuleSet		*ruleSet;
	PolicyCtrl		*policyCtrl;

	policyCtrl = PolicyCtrl::getInstance();

	/* clear the whole list */
	for (int i = lst_Rules->GetItemCount() - 1; i >= 0; i--) {
		removeListRow(i);
	}

	/* release old ones */
	ruleSet = policyCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->unlock();
		removeSubject(ruleSet);
	}

	ruleSet = policyCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->unlock();
		removeSubject(ruleSet);
	}

	userRuleSetId_ = policyCtrl->getUserId();
	adminRuleSetId_ = policyCtrl->getAdminId(geteuid());

	/* get the new ones */
	ruleSet = policyCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->lock();
		addSubject(ruleSet);
		ruleSet->accept(addVisitor);
	}

	ruleSet = policyCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->lock();
		addSubject(ruleSet);
		ruleSet->accept(addVisitor);
	}

	event.Skip();
}

void
ModSbMainPanelImpl::removeListRow(long rowIdx)
{
	Policy	*policy;

	policy = wxDynamicCast((void*)lst_Rules->GetItemData(rowIdx), Policy);
	if (policy != NULL) {
		/* deregister for observing policy */
		removeSubject(policy);
	}

	lst_Rules->DeleteItem(rowIdx);
}

long
ModSbMainPanelImpl::findListRow(Policy *policy)
{
	for (int i = lst_Rules->GetItemCount(); i >= 0; i--) {
		if (policy == (Policy *)lst_Rules->GetItemData(i)) {
			return (i);
		}
	}

	return (-1);
}

long
ModSbMainPanelImpl::ruleListAppend(Policy *policy)
{
	long		idx;
	wxString	ruleType;
	wxString	userName;
	PolicyRuleSet	*ruleset;

	idx = lst_Rules->GetItemCount();
	lst_Rules->InsertItem(idx, wxEmptyString, idx);
	lst_Rules->SetItemPtrData(idx, (wxUIntPtr)policy);

	/* register for observing policy */
	addSubject(policy);

	ruleset = policy->getParentRuleSet();
	if (ruleset->isAdmin()) {
		userName = wxGetApp().getUserNameById(ruleset->getUid());
		ruleType.Printf(_("admin ruleset of %ls"), userName.c_str());
		lst_Rules->SetItemBackgroundColour(idx,
				wxTheColourDatabase->Find(wxT("LIGHT GREY")));
	} else {
		ruleType = wxGetUserId();
	}
	lst_Rules->SetItem(idx, COLUMN_USER, ruleType);

	if (policy->hasScope()) {
		lst_Rules->SetItem(idx, COLUMN_SCOPE, wxT("T"));
	}

	return (idx);
}

void
ModSbMainPanelImpl::updateDefaultFilterPolicy(long idx)
{
	DefaultFilterPolicy *policy;

	policy = wxDynamicCast((void*)lst_Rules->GetItemData(idx),
	    DefaultFilterPolicy);

	if (policy == NULL) {
		return;
	}

	lst_Rules->SetItem(idx, COLUMN_ACTION, policy->getActionName());
	lst_Rules->SetItem(idx, COLUMN_PATH, wxT("default"));
}

void
ModSbMainPanelImpl::updateSbAccessFilterPolicy(long idx)
{
	void			*data;
	SbAccessFilterPolicy	*sbPolicy;
	FilterPolicy		*policy;

	data = (void*)lst_Rules->GetItemData(idx);
	policy = wxDynamicCast(data, FilterPolicy);
	sbPolicy =  wxDynamicCast(data, SbAccessFilterPolicy);

	if (policy == NULL) {
		return;
	}

	if (policy->IsKindOf(CLASSINFO(SbAccessFilterPolicy)) ||
	     policy->IsKindOf(CLASSINFO(DefaultFilterPolicy))) {
	     lst_Rules->SetItem(idx, COLUMN_ACTION, policy->getActionName());
	}

	if (policy->IsKindOf(CLASSINFO(SbAccessFilterPolicy))) {
		lst_Rules->SetItem(idx, COLUMN_PATH, sbPolicy->getPath());
		lst_Rules->SetItem(idx, COLUMN_SUB, sbPolicy->getSubjectName());
		lst_Rules->SetItem(idx, COLUMN_MASK,
		    sbPolicy->getAccessMaskName());
	}
}

void
ModSbMainPanelImpl::updateSbAppPolicy(long idx)
{
	SbAppPolicy *policy;

	policy = wxDynamicCast((void*)lst_Rules->GetItemData(idx), SbAppPolicy);

	if (policy == NULL) {
		return;
	}

	lst_Rules->SetItem(idx, COLUMN_PROG, policy->getBinaryName());
}

void
ModSbMainPanelImpl::updateShowRuleset(void)
{
	ModSbAddPolicyVisitor  addVisitor(this);
	PolicyCtrl              *policyCtrl;
	PolicyRuleSet           *ruleSet;

	policyCtrl = PolicyCtrl::getInstance();

	/* get the new ones */
	ruleSet = policyCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->accept(addVisitor);
	}

	ruleSet = policyCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->accept(addVisitor);
	}
}

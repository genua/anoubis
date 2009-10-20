/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#include <sys/param.h>
#include <sys/socket.h>

#ifndef LINUX
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

#include <apn.h>

#include "AnEvents.h"
#include "main.h"
#include "ModAlfAddPolicyVisitor.h"
#include "ModAlfMainPanelImpl.h"
#include "Policy.h"
#include "PolicyRuleSet.h"
#include "PolicyCtrl.h"

ModAlfMainPanelImpl::ModAlfMainPanelImpl(wxWindow* parent,
    wxWindowID id) : Observer(NULL), ModAlfMainPanelBase(parent, id)
{
	columnNames_[COLUMN_PROG] = _("Program");
	columnNames_[COLUMN_SERVICE] = _("Service");
	columnNames_[COLUMN_ROLE] = _("Role");
	columnNames_[COLUMN_ACTION] = _("Action");
	columnNames_[COLUMN_SCOPE] = _("Temporary");
	columnNames_[COLUMN_USER] = _("User");

	userRuleSetId_ = -1;
	adminRuleSetId_ = -1;

	for (int i=0; i<COLUMN_EOL; i++) {
		lst_Rules->InsertColumn(i, columnNames_[i],
		    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
	}

	/*
	 * Adjust column width (Bug #1321).
	 * Tablewidth - sum of fix columns = remaining size.
	 * 758 - (170 + 55 + 70 + 90 + 50) = 323
	 */
	lst_Rules->SetColumnWidth(COLUMN_PROG,		323);
	lst_Rules->SetColumnWidth(COLUMN_SERVICE,	170);
	lst_Rules->SetColumnWidth(COLUMN_ROLE,		 55);
	lst_Rules->SetColumnWidth(COLUMN_ACTION,	 70);
	lst_Rules->SetColumnWidth(COLUMN_SCOPE,		 90);
	lst_Rules->SetColumnWidth(COLUMN_USER,		 50);

	AnEvents::getInstance()->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModAlfMainPanelImpl::onLoadRuleSet),
	    NULL, this);
}

ModAlfMainPanelImpl::~ModAlfMainPanelImpl(void)
{
	AnEvents::getInstance()->Disconnect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModAlfMainPanelImpl::onLoadRuleSet),
	    NULL, this);
}

void
ModAlfMainPanelImpl::addAlfAppPolicy(AlfAppPolicy *policy)
{
	long	idx;

	idx = ruleListAppend(policy);
	updateAlfAppPolicy(idx);
}

void
ModAlfMainPanelImpl::addAlfFilterPolicy(AlfFilterPolicy *policy)
{
	long	idx;

	idx = ruleListAppend(policy);
	updateAlfFilterPolicy(idx);
}

void
ModAlfMainPanelImpl::addAlfCapabilityFilterPolicy(
    AlfCapabilityFilterPolicy *policy)
{
	long	idx;

	idx = ruleListAppend(policy);
	updateAlfCapabilityFilterPolicy(idx);
}

void
ModAlfMainPanelImpl::addDefaultFilterPolicy(DefaultFilterPolicy *policy)
{
	long		idx;

	idx = ruleListAppend(policy);
	updateDefaultFilterPolicy(idx);
}

void
ModAlfMainPanelImpl::onLoadRuleSet(wxCommandEvent& event)
{
	ModAlfAddPolicyVisitor	 addVisitor(this);
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

long
ModAlfMainPanelImpl::ruleListAppend(Policy *policy)
{
	long             idx;
	wxString         ruleType;
	wxString         userName;
	PolicyRuleSet   *ruleset;

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
ModAlfMainPanelImpl::removeListRow(long rowIdx)
{
	Policy  *policy;

	policy = wxDynamicCast((void*)lst_Rules->GetItemData(rowIdx), Policy);
	if (policy != NULL) {
		/* deregister for observing policy */
		removeSubject(policy);
	}

	lst_Rules->DeleteItem(rowIdx);
}


long
ModAlfMainPanelImpl::findListRow(Policy *policy)
{
	for (int i = lst_Rules->GetItemCount(); i >= 0; i--) {
		if (policy == (Policy *)lst_Rules->GetItemData(i)) {
			return (i);
		}
	}

	return (-1);
}

void
ModAlfMainPanelImpl::updateAlfCapabilityFilterPolicy(long idx)
{
	AlfCapabilityFilterPolicy *policy;

	policy = wxDynamicCast((void *)lst_Rules->GetItemData(idx),
	    AlfCapabilityFilterPolicy);
	if (policy == NULL) {
		return;
	}
	lst_Rules->SetItem(idx, COLUMN_ACTION, policy->getActionName());
	lst_Rules->SetItem(idx, COLUMN_SERVICE, wxT("capability ") +
	    policy->getCapabilityTypeName());
}

void
ModAlfMainPanelImpl::updateAlfFilterPolicy(long idx)
{
	AlfFilterPolicy *policy;

	policy = wxDynamicCast((void*)lst_Rules->GetItemData(idx),
	    AlfFilterPolicy);
	if (policy == NULL) {
		return;
	}
	lst_Rules->SetItem(idx, COLUMN_ACTION, policy->getActionName());
	lst_Rules->SetItem(idx, COLUMN_ROLE, policy->getRoleName());
	lst_Rules->SetItem(idx, COLUMN_SERVICE, policy->getServiceName());
}

void
ModAlfMainPanelImpl::updateAlfAppPolicy(long idx)
{
	AlfAppPolicy *policy;

	policy = wxDynamicCast((void*)lst_Rules->GetItemData(idx),
	    AlfAppPolicy);
	if (policy == NULL) {
		return;
	}
	lst_Rules->SetItem(idx, COLUMN_PROG, policy->getBinaryName());
}

void
ModAlfMainPanelImpl::updateDefaultFilterPolicy(long idx)
{
	DefaultFilterPolicy *policy;

	policy = wxDynamicCast((void*)lst_Rules->GetItemData(idx),
	    DefaultFilterPolicy);
	if (policy == NULL) {
		return;
	}
	lst_Rules->SetItem(idx, COLUMN_ACTION, policy->getActionName());
	lst_Rules->SetItem(idx, COLUMN_SERVICE, wxT("default"));
}

void
ModAlfMainPanelImpl::updateShowRuleset(void)
{
	ModAlfAddPolicyVisitor	addVisitor(this);
	PolicyCtrl		*policyCtrl;
	PolicyRuleSet		*ruleSet;

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

void
ModAlfMainPanelImpl::updateDelete(Subject *subject)
{
	long	idx = -1;
	idx = findListRow((Policy *)subject);

	if (idx != -1) {
		removeListRow(idx);
	}
}

void
ModAlfMainPanelImpl::update(Subject *subject)
{
	long		idx;
	AppPolicy	*parent;
	FilterPolicy	*filter;

	if (subject->IsKindOf(CLASSINFO(AlfAppPolicy))) {
		idx = findListRow((Policy *)subject);
		if (idx != -1) {
			updateAlfAppPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(AlfFilterPolicy))) {
		idx = findListRow((Policy *)subject);
		if (idx != -1) {
			updateAlfFilterPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(AlfCapabilityFilterPolicy))) {
		idx = findListRow((Policy *)subject);
		if (idx != -1) {
			updateAlfCapabilityFilterPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(DefaultFilterPolicy))) {
		idx = findListRow((Policy *)subject);
		filter = wxDynamicCast(subject, FilterPolicy);
		parent = filter->getParentPolicy();
		if ((idx != -1) && (parent != NULL)) {
			if (parent->IsKindOf(CLASSINFO(AlfAppPolicy))) {
				updateDefaultFilterPolicy(idx);
			}
		}
	} else if (subject->IsKindOf(CLASSINFO(PolicyRuleSet))) {
		/* get the new ruleset and update the view */
		if (lst_Rules->GetItemCount() == 0) {
			updateShowRuleset();
		}
	} else {
		/* Unknown subject type - do nothing */
	}
}

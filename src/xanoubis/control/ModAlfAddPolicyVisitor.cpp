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
#include <wx/string.h>

#include <apn.h>

#include "ModAlfMainPanelImpl.h"
#include "Policy.h"
#include "PolicyVisitor.h"
#include "AppPolicy.h"
#include "AlfPolicy.h"
#include "VarPolicy.h"
#include "main.h"

#include "ModAlfAddPolicyVisitor.h"

ModAlfAddPolicyVisitor::ModAlfAddPolicyVisitor(ModAlfMainPanelImpl *alfPanel)
{
	alfPanel_ = alfPanel;
}

ModAlfAddPolicyVisitor::~ModAlfAddPolicyVisitor(void)
{
}

long
ModAlfAddPolicyVisitor::ruleListAppend(Policy *policy)
{
	long		idx;
	wxString	ruleType;

	idx = alfPanel_->lst_Rules->GetItemCount();
	alfPanel_->lst_Rules->InsertItem(idx, wxString::Format(wxT("%d"),
	    idx));

	alfPanel_->lst_Rules->SetItemPtrData(idx, (wxUIntPtr)policy);

	if (isAdmin()) {
		ruleType  = wxT("admin ruleset of ");
		ruleType += wxGetApp().getUserNameById(
		    policy->getRsParent()->getUid());
		alfPanel_->lst_Rules->SetItemBackgroundColour(idx,
		    wxTheColourDatabase->Find(wxT("LIGHT GREY")));

	} else {
		ruleType = wxGetUserId();
	}
	alfPanel_->lst_Rules->SetItem(idx, MODALF_LIST_COLUMN_ADMIN, ruleType);

	return (idx);
}

void
ModAlfAddPolicyVisitor::visitAppPolicy(AppPolicy *appPolicy)
{
	long		 idx;
	wxListCtrl	*list;

	/* Never visit the SFS dummy application block itself! */
	if (appPolicy->getType() == APN_SFS)
		return;
	if(appPolicy->getType() == APN_ALF) {
		/* fill list of all rules */
		idx = ruleListAppend(appPolicy);
		list = alfPanel_->lst_Rules;

		list->SetItem(idx, MODALF_LIST_COLUMN_APP,
		    appPolicy->getBinaryName());
		list->SetItem(idx, MODALF_LIST_COLUMN_PROG,
		    appPolicy->getBinaryName());
		if (appPolicy->hasContext()) {
			list->SetItem(idx, MODALF_LIST_COLUMN_CTX,
			    appPolicy->getContext()->getContextName());
		}
	}
}

void
ModAlfAddPolicyVisitor::visitAlfPolicy(AlfPolicy *alfPolicy)
{
	long		 idx;
	wxListCtrl	*list;

	idx = ruleListAppend(alfPolicy);
	list = alfPanel_->lst_Rules;

	if (alfPolicy->hasScope()) {
		list->SetItem(idx, MODALF_LIST_COLUMN_SCOPE,
		    wxT("T"));
	}

	switch (alfPolicy->getTypeNo()) {
	case APN_ALF_FILTER:
		/* fill list of all rules */
		list->SetItem(idx, MODALF_LIST_COLUMN_ACTION,
		    alfPolicy->getActionName());
		list->SetItem(idx, MODALF_LIST_COLUMN_ROLE,
		    alfPolicy->getRoleName());
		list->SetItem(idx, MODALF_LIST_COLUMN_SERVICE,
		    alfPolicy->getServiceName());
		break;
	case APN_ALF_CAPABILITY:
		list->SetItem(idx, MODALF_LIST_COLUMN_ACTION,
		    alfPolicy->getActionName());
		list->SetItem(idx, MODALF_LIST_COLUMN_SERVICE,
		    wxT("capability ") + alfPolicy->getCapTypeName());
		break;
	case APN_DEFAULT:
		/* fill list of all rules */
		list->SetItem(idx, MODALF_LIST_COLUMN_ACTION,
		    alfPolicy->getActionName());
		list->SetItem(idx, MODALF_LIST_COLUMN_SERVICE, wxT("default"));
		break;
	case APN_ALF_CTX:
		break;
	default:
		break;
	}
}

void
ModAlfAddPolicyVisitor::visitSfsPolicy(SfsPolicy*)
{
	/* no SFS policies shown in ALF module */
}

void
ModAlfAddPolicyVisitor::visitVarPolicy(VarPolicy*)
{
}

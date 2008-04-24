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

#include "ModSfsMainPanelImpl.h"
#include "Policy.h"
#include "PolicyVisitor.h"
#include "AppPolicy.h"
#include "AlfPolicy.h"
#include "VarPolicy.h"

#include "ModSfsAddPolicyVisitor.h"

ModSfsAddPolicyVisitor::ModSfsAddPolicyVisitor(ModSfsMainPanelImpl *sfsPanel)
{
	sfsPanel_ = sfsPanel;
}

ModSfsAddPolicyVisitor::~ModSfsAddPolicyVisitor()
{
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

void
ModSfsAddPolicyVisitor::visitAppPolicy(AppPolicy *appPolicy)
{
	/* no app policies shown in SFS module */
}

void
ModSfsAddPolicyVisitor::visitAlfPolicy(AlfPolicy *alfPolicy)
{
	/* no ALF policies shown in SFS module */
}

void
ModSfsAddPolicyVisitor::visitSfsPolicy(SfsPolicy *sfsPolicy)
{
	long		 idx;
	wxListCtrl	*list;

	idx = ruleListAppend(sfsPolicy);
	list = sfsPanel_->lst_Rules;

	list->SetItem(idx, MODSFS_LIST_COLUMN_APP,
	    sfsPolicy->getAppName());
	list->SetItem(idx, MODSFS_LIST_COLUMN_PROG,
	    sfsPolicy->getBinaryName());
	list->SetItem(idx, MODSFS_LIST_COLUMN_HASHT,
	    sfsPolicy->getHashTypeName());
	list->SetItem(idx, MODSFS_LIST_COLUMN_HASH,
	   sfsPolicy->getHashValue());
}

void
ModSfsAddPolicyVisitor::visitVarPolicy(VarPolicy *variable)
{
}

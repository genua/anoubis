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

#include <apn.h>

#include "AnEvents.h"
#include "Policy.h"
#include "ModSfsAddPolicyVisitor.h"

#include "ModSfsMainPanelImpl.h"

ModSfsMainPanelImpl::ModSfsMainPanelImpl(wxWindow* parent,
    wxWindowID id) : ModSfsMainPanelBase(parent, id)
{
	columnNames_[MODSFS_LIST_COLUMN_PRIO] = _("Prio");
	columnNames_[MODSFS_LIST_COLUMN_PROG] = _("Program");
	columnNames_[MODSFS_LIST_COLUMN_HASHT] = _("Hash-Type");
	columnNames_[MODSFS_LIST_COLUMN_HASH] = _("Hash");

	userRuleSet_ = NULL;
	adminRuleSet_ = NULL;

	for (int i=0; i<MODSFS_LIST_COLUMN_EOL; i++) {
		lst_Rules->InsertColumn(i, columnNames_[i], wxLIST_FORMAT_LEFT,
		    wxLIST_AUTOSIZE);
	}

	parent->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModSfsMainPanelImpl::OnLoadRuleSet),
	    NULL, this);

	initSfsMain();
	SfsMainDirCtrl->Connect(wxEVT_COMMAND_TREE_SEL_CHANGED,
	    wxTreeEventHandler(ModSfsMainPanelImpl::OnSfsMainDirCtrlSelChanged),
	    NULL, this);
}

void
ModSfsMainPanelImpl::OnLoadRuleSet(wxCommandEvent& event)
{
	ModSfsAddPolicyVisitor	 addVisitor(this);
	PolicyRuleSet		*ruleSet;

	lst_Rules->DeleteAllItems();
	ruleSet = (PolicyRuleSet *)event.GetClientData();
	if (ruleSet->isAdmin()) {
		adminRuleSet_ = ruleSet;
	} else {
		userRuleSet_ = ruleSet;
	}

	if (userRuleSet_ != NULL) {
		addVisitor.setAdmin(false);
		userRuleSet_->accept(addVisitor);
	}
	if (adminRuleSet_!= NULL) {
		addVisitor.setAdmin(true);
		adminRuleSet_->accept(addVisitor);
	}

	/* trigger new * calculation of column width */
	for (int i=0; i<MODSFS_LIST_COLUMN_EOL; i++) {
		lst_Rules->SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);
	}
	event.Skip();
}

void
ModSfsMainPanelImpl::OnSfsMainDirCtrlSelChanged(wxTreeEvent &)
{
	/* Update CurrPathLabel accordingly */
	SfsMainCurrPathLabel->SetLabel(SfsMainDirCtrl->GetPath());
}

void
ModSfsMainPanelImpl::initSfsMain()
{
	/* Insert columns into list-ctrl */
	SfsMainListCtrl->InsertColumn(MODSFSMAIN_FILELIST_COLUMN_FILE,
		_("File"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
	SfsMainListCtrl->InsertColumn(MODSFSMAIN_FILELIST_COLUMN_CHECKSUM,
		_("Checksum"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
	SfsMainListCtrl->InsertColumn(MODSFSMAIN_FILELIST_COLUMN_SIGNATURE,
		_("Signature"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);

	/* Setting up CurrPathLabel with initial path */
	SfsMainCurrPathLabel->SetLabel(SfsMainDirCtrl->GetPath());
}

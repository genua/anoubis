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
#include "main.h"
#include "ModSfsAddPolicyVisitor.h"
#include "Policy.h"
#include "PolicyRuleSet.h"
#include "ProfileCtrl.h"
#include "SfsCtrl.h"

#include "ModSfsMainPanelImpl.h"

ModSfsMainPanelImpl::ModSfsMainPanelImpl(wxWindow* parent,
    wxWindowID id) : ModSfsMainPanelBase(parent, id)
{
	columnNames_[MODSFS_LIST_COLUMN_PRIO] = _("Prio");
	columnNames_[MODSFS_LIST_COLUMN_PROG] = _("Program");
	columnNames_[MODSFS_LIST_COLUMN_HASHT] = _("Hash-Type");
	columnNames_[MODSFS_LIST_COLUMN_HASH] = _("Hash");

	userRuleSetId_  = -1;
	adminRuleSetId_ = -1;

	for (int i=0; i<MODSFS_LIST_COLUMN_EOL; i++) {
		lst_Rules->InsertColumn(i, columnNames_[i], wxLIST_FORMAT_LEFT,
		    wxLIST_AUTOSIZE);
	}

	parent->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModSfsMainPanelImpl::OnLoadRuleSet),
	    NULL, this);

	initSfsMain();
}

ModSfsMainPanelImpl::~ModSfsMainPanelImpl()
{
	destroySfsMain();
}

void
ModSfsMainPanelImpl::OnLoadRuleSet(wxCommandEvent& event)
{
	ModSfsAddPolicyVisitor	 addVisitor(this);
	PolicyRuleSet		*ruleSet;
	ProfileCtrl		*profileCtrl;

	profileCtrl = ProfileCtrl::getInstance();

	lst_Rules->DeleteAllItems();

	profileCtrl->unlockFromShow(userRuleSetId_, this);
	profileCtrl->unlockFromShow(adminRuleSetId_, this);
	userRuleSetId_ = profileCtrl->getUserId();
	if (profileCtrl->lockToShow(userRuleSetId_, this)) {
		ruleSet = profileCtrl->getRuleSetToShow(userRuleSetId_, this);
		if (ruleSet != NULL) {
			addVisitor.setAdmin(false);
			ruleSet->accept(addVisitor);
		}
	}

	adminRuleSetId_ = profileCtrl->getAdminId(geteuid());
	if (profileCtrl->lockToShow(adminRuleSetId_, this)) {
		ruleSet = profileCtrl->getRuleSetToShow(adminRuleSetId_, this);
		if (ruleSet != NULL) {
			addVisitor.setAdmin(true);
			ruleSet->accept(addVisitor);
		}
	}

	if (geteuid() == 0) {
		unsigned long	uid;
		long		rsid;
		wxArrayString	userList = wxGetApp().getListOfUsersId();

		for (size_t i=0; i<userList.GetCount(); i++) {
			userList.Item(i).ToULong(&uid);
			rsid = profileCtrl->getAdminId((uid_t)uid);
			if (rsid == -1) {
				continue;
			}
			if (!profileCtrl->lockToShow(rsid, this)) {
				continue;
			}
			ruleSet = profileCtrl->getRuleSetToShow(rsid, this);
			if (ruleSet != NULL) {
				addVisitor.setAdmin(true);
				ruleSet->accept(addVisitor);
			}
		}
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
	/* Update controller */
	sfsCtrl_->setPath(SfsMainDirCtrl->GetPath());
}

void
ModSfsMainPanelImpl::OnSfsDirChanged(wxCommandEvent&)
{
	/* Display changes */
	updateSfsList();

	/* Update CurrPathLabel accordingly */
	SfsMainCurrPathLabel->SetLabel(SfsMainDirCtrl->GetPath());
}

void
ModSfsMainPanelImpl::OnSfsEntryChanged(wxCommandEvent &event)
{
	updateSfsEntry(event.GetInt()); /* Update the entry */
}

void
ModSfsMainPanelImpl::OnSfsMainFilterButtonClicked(wxCommandEvent&)
{
	sfsCtrl_->setFilter(SfsMainFilterTextCtrl->GetValue());
}

void
ModSfsMainPanelImpl::OnSfsMainInverseCheckboxClicked(wxCommandEvent&)
{
	sfsCtrl_->setFilterInversed(SfsMainFilterInvertCheckBox->IsChecked());
}

void
ModSfsMainPanelImpl::OnSfsMainValidateButtonClicked(wxCommandEvent&)
{
	if (!sfsCtrl_->validateAll()) {
		wxGetApp().status(
		    _("Error: xanoubis is not connected to the daemon"));
	}
}

void
ModSfsMainPanelImpl::OnSfsMainApplyButtonClicked(wxCommandEvent&)
{
	int selection = SfsMainListCtrl->GetNextItem(-1,
	    wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	if (selection == -1) {
		/* No selection */
		return;
	}

	bool result = false;

	switch (SfsMainActionChoice->GetCurrentSelection()) {
	case 0:
		result = sfsCtrl_->registerChecksum(selection);
		break;
	case 1:
		result = sfsCtrl_->unregisterChecksum(selection);
		break;
	case 2:
		result = sfsCtrl_->validate(selection);
		break;
	case 3:
		result = sfsCtrl_->updateChecksum(selection);
		break;
	}

	if (!result) {
		wxGetApp().status(
		    _("Error: xanoubis is not connected to the daemon"));
	}
}

void
ModSfsMainPanelImpl::initSfsMain()
{
	sfsCtrl_ = new SfsCtrl;

	sfsListOkIcon_ = wxGetApp().loadIcon(wxT("General_ok_16.png"));
	sfsListWarnIcon_ = wxGetApp().loadIcon(wxT("General_problem_16.png"));
	sfsListErrorIcon_ = wxGetApp().loadIcon(wxT("General_error_16.png"));

	sfsListImageList_.Add(wxNullIcon);
	sfsListImageList_.Add(*sfsListOkIcon_);
	sfsListImageList_.Add(*sfsListWarnIcon_);
	sfsListImageList_.Add(*sfsListErrorIcon_);

	sfsCtrl_->Connect(anEVT_SFSDIR_CHANGED,
	    wxCommandEventHandler(ModSfsMainPanelImpl::OnSfsDirChanged),
	    NULL, this);
	sfsCtrl_->Connect(anEVT_SFSENTRY_CHANGED,
	    wxCommandEventHandler(ModSfsMainPanelImpl::OnSfsEntryChanged),
	    NULL, this);

	SfsMainDirCtrl->Connect(wxEVT_COMMAND_TREE_SEL_CHANGED,
	    wxTreeEventHandler(ModSfsMainPanelImpl::OnSfsMainDirCtrlSelChanged),
	    NULL, this);
	SfsMainFilterButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(
	       ModSfsMainPanelImpl::OnSfsMainFilterButtonClicked),
	    NULL, this);
	SfsMainFilterInvertCheckBox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
	    wxCommandEventHandler(
	       ModSfsMainPanelImpl::OnSfsMainInverseCheckboxClicked),
	    NULL, this);
	SfsMainFilterTextCtrl->Connect(wxEVT_COMMAND_TEXT_ENTER,
	    wxCommandEventHandler(
	       ModSfsMainPanelImpl::OnSfsMainFilterButtonClicked),
	    NULL, this);
	SfsMainFilterValidateButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(
	       ModSfsMainPanelImpl::OnSfsMainValidateButtonClicked),
	    NULL, this);
	SfsMainActionButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(
	       ModSfsMainPanelImpl::OnSfsMainApplyButtonClicked),
	    NULL, this);

	/* Insert columns into list-ctrl */
	SfsMainListCtrl->InsertColumn(MODSFSMAIN_FILELIST_COLUMN_FILE,
	    _("File"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
	SfsMainListCtrl->InsertColumn(MODSFSMAIN_FILELIST_COLUMN_CHECKSUM,
	    _("Checksum"), wxLIST_FORMAT_CENTRE, wxLIST_AUTOSIZE_USEHEADER);
	SfsMainListCtrl->InsertColumn(MODSFSMAIN_FILELIST_COLUMN_SIGNATURE,
	    _("Signature"), wxLIST_FORMAT_CENTRE, wxLIST_AUTOSIZE_USEHEADER);

	/* Adjust initial width of MODSFSMAIN_FILELIST_COLUMN_FILE */
	int fileColumnWidth = SfsMainListCtrl->GetClientSize().GetWidth();
	fileColumnWidth -= SfsMainListCtrl->
	    GetColumnWidth(MODSFSMAIN_FILELIST_COLUMN_CHECKSUM);
	fileColumnWidth -= SfsMainListCtrl->
	    GetColumnWidth(MODSFSMAIN_FILELIST_COLUMN_SIGNATURE);
	SfsMainListCtrl->SetColumnWidth(MODSFSMAIN_FILELIST_COLUMN_FILE,
	    fileColumnWidth);

	/* Assign icons to list */
	SfsMainListCtrl->SetImageList(&sfsListImageList_, wxIMAGE_LIST_SMALL);

	/* Setting up CurrPathLabel with initial path */
	SfsMainCurrPathLabel->SetLabel(SfsMainDirCtrl->GetPath());
	sfsCtrl_->setPath(SfsMainDirCtrl->GetPath());
}

void
ModSfsMainPanelImpl::destroySfsMain()
{
	delete sfsCtrl_;
	delete sfsListOkIcon_;
	delete sfsListWarnIcon_;
	delete sfsListErrorIcon_;
}

void
ModSfsMainPanelImpl::updateSfsList()
{
	SfsDirectory &dir = sfsCtrl_->getSfsDirectory();

	SfsMainListCtrl->DeleteAllItems();

	for (unsigned int i = 0; i < dir.getNumEntries(); i++) {
		SfsMainListCtrl->InsertItem(i, wxEmptyString);
		updateSfsEntry(i);
	}
}

void
ModSfsMainPanelImpl::updateSfsEntry(int idx)
{
	SfsDirectory	&dir = sfsCtrl_->getSfsDirectory();
	SfsEntry	&entry = dir.getEntry(idx);
	wxString	checksumInfo;
	int		checksumIconIndex = 0, signatureIconIndex = 0;

	switch (entry.getChecksumAttr()) {
	case SfsEntry::SFSENTRY_CHECKSUM_NOT_VALIDATED:
		checksumInfo = _("not validated");
		checksumIconIndex = 0;
		break;
	case SfsEntry::SFSENTRY_CHECKSUM_UNKNOWN:
		checksumInfo = _("not registered");
		checksumIconIndex = 0;
		break;
	case SfsEntry::SFSENTRY_CHECKSUM_NOMATCH:
		checksumInfo = wxEmptyString;
		checksumIconIndex = 3;
		break;
	case SfsEntry::SFSENTRY_CHECKUM_MATCH:
		checksumInfo = wxEmptyString;
		checksumIconIndex = 1;
		break;
	}

	switch (entry.getSignatureAttr()) {
	case SfsEntry::SFSENTRY_SIGNATURE_UNKNOWN:
		signatureIconIndex = 0;
		break;
	case SfsEntry::SFSENTRY_SIGNATURE_INVALID:
		signatureIconIndex = 2;
		break;
	case SfsEntry::SFSENTRY_SIGNATURE_NOMATCH:
		signatureIconIndex = 3;
		break;
	case SfsEntry::SFSENTRY_SIGNATURE_MATCH:
		signatureIconIndex = 1;
		break;
	}

	SfsMainListCtrl->SetItem(idx,
	    MODSFSMAIN_FILELIST_COLUMN_FILE, entry.getFileName());
	SfsMainListCtrl->SetItem(idx,
	    MODSFSMAIN_FILELIST_COLUMN_CHECKSUM, checksumInfo,
	    checksumIconIndex);
	SfsMainListCtrl->SetItem(idx,
	    MODSFSMAIN_FILELIST_COLUMN_SIGNATURE, wxEmptyString,
	    signatureIconIndex);
}

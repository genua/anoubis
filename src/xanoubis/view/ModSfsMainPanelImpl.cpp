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

#include <wx/msgdlg.h>
#include <wx/textdlg.h>

#include "AnEvents.h"
#include "main.h"
#include "KeyCtrl.h"
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

	AnEvents::getInstance()->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModSfsMainPanelImpl::OnLoadRuleSet),
	    NULL, this);

	initSfsMain();
}

ModSfsMainPanelImpl::~ModSfsMainPanelImpl()
{
	AnEvents::getInstance()->Disconnect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModSfsMainPanelImpl::OnLoadRuleSet),
	    NULL, this);

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

	userRuleSetId_ = profileCtrl->getUserId();
	ruleSet = profileCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->accept(addVisitor);
	}

	adminRuleSetId_ = profileCtrl->getAdminId(geteuid());
	ruleSet = profileCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->accept(addVisitor);
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

			ruleSet = profileCtrl->getRuleSet(rsid);
			if (ruleSet != NULL) {
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
ModSfsMainPanelImpl::OnSfsMainDirTraversalChecked(wxCommandEvent&)
{
	SfsDirectory &dir = sfsCtrl_->getSfsDirectory();
	dir.setDirTraversal(SfsMainDirTraversalCheckbox->GetValue());

	/* Display changes */
	updateSfsList();
}

void
ModSfsMainPanelImpl::OnSfsEntryChanged(wxCommandEvent &event)
{
	updateSfsEntry(event.GetInt()); /* Update the entry */
}

void
ModSfsMainPanelImpl::OnSfsError(wxCommandEvent&)
{
	const wxArrayString &errors = sfsCtrl_->getErrors();
	wxString errStr;

	if (errors.Count() > 1) {
		for (unsigned int i = 0; i < errors.Count() - 1; i++) {
			errStr += errors[i];
			errStr += wxT("\n");
		}
	}

	if (!errors.IsEmpty())
		errStr += errors.Last();

	wxMessageBox(errStr, _("SFS error"), wxOK|wxICON_ERROR, this);
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
	SfsCtrl::CommandResult result = sfsCtrl_->validateAll();

	switch (result) {
	case SfsCtrl::RESULT_NOTCONNECTED:
		wxGetApp().status(
		    _("Error: xanoubis is not connected to the daemon"));
		break;
	case SfsCtrl::RESULT_NEEDPASS:
		wxGetApp().status(
		    _("Error: A passphrase is required."));
		break;
	case SfsCtrl::RESULT_INVALIDARG:
		wxGetApp().status(
		    _("Error: An invalid argument was supplied"));
		break;
	case SfsCtrl::RESULT_BUSY:
		wxGetApp().status(
		    _("Error: Sfs is still busy with another operation."));
		break;
	case SfsCtrl::RESULT_EXECUTE:
		/* Success */
		break;
	}
}

void
ModSfsMainPanelImpl::OnSfsMainApplyButtonClicked(wxCommandEvent&)
{
	int		selection = -1;
	IndexArray	selectionArray;

	while (true) {
		selection = SfsMainListCtrl->GetNextItem(selection,
		    wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		if (selection == -1) {
			/* No selection */
			break;
		}

		selectionArray.Add(selection);
	}

	applySfsAction(selectionArray);
}

void
ModSfsMainPanelImpl::applySfsAction(const IndexArray &selection)
{
	SfsCtrl::CommandResult result = SfsCtrl::RESULT_EXECUTE;

	do {
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

		if (result == SfsCtrl::RESULT_NEEDPASS) {
			KeyCtrl *keyCtrl = KeyCtrl::getInstance();
			PrivKey &privKey = keyCtrl->getPrivateKey();

			wxPasswordEntryDialog dlg(this,
			    _("Enter the passphrase of your private key:"),
			    _("Enter passphrase"));

			if (dlg.ShowModal() != wxID_OK) /* Canceled */
				return;

			/*
			 * Load the private key into memory using the entered
			 * passphrase.
			 */
			if (!privKey.load(dlg.GetValue())) {
				/* XXX Bad error handling */
				wxMessageBox(
				    _("Could not load the private key."),
				    _("SFS error"), wxOK|wxICON_ERROR, this);
				return;
			}
		}
	} while (result == SfsCtrl::RESULT_NEEDPASS);

	switch (result) {
	case SfsCtrl::RESULT_NOTCONNECTED:
		wxGetApp().status(
		  _("Error: xanoubis is not connected to the daemon"));
		break;
	case SfsCtrl::RESULT_NEEDPASS:
		wxGetApp().status(
		  _("Error: A passphrase is still required."));
		break;
	case SfsCtrl::RESULT_INVALIDARG:
		wxGetApp().status(
		  _("Error: An invalid argument was supplied"));
		break;
	case SfsCtrl::RESULT_BUSY:
		wxGetApp().status(
		 _("Error: Sfs is still busy with another operation."));
		break;
	case SfsCtrl::RESULT_EXECUTE:
		/* Success */
		break;
	}
}

void
ModSfsMainPanelImpl::OnSfsMainSigEnabledClicked(wxCommandEvent&)
{
	bool enable = SfsMainSignFilesCheckBox->IsChecked();

	if (!sfsCtrl_->setSignatureEnabled(enable)) {
		wxString msg;

		if (enable)
			msg = _("Failed to enable signed checksums.\n"
			    "Please check the configuration of your "
			    "certificate and private key!");
		else
			msg = _("Failed to disable signed checksums.");

		wxMessageBox(msg, _("SFS error"), wxOK|wxICON_ERROR, this);
		SfsMainSignFilesCheckBox->SetValue(!enable); /* Toggle back */
	}
}

void
ModSfsMainPanelImpl::OnSfsMainKeyLoaded(wxCommandEvent&)
{
	bool keysUsable = KeyCtrl::getInstance()->canUseLocalKeys();
	bool sigEnabled = sfsCtrl_->isSignatureEnabled();

	SfsMainSignFilesCheckBox->Enable(keysUsable);
	SfsMainSignFilesCheckBox->SetValue(sigEnabled);
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
	sfsCtrl_->Connect(anEVT_SFSENTRY_ERROR,
	    wxCommandEventHandler(ModSfsMainPanelImpl::OnSfsError),
	    NULL, this);
	AnEvents::getInstance()->Connect(anEVT_LOAD_KEY,
	    wxCommandEventHandler(ModSfsMainPanelImpl::OnSfsMainKeyLoaded),
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

	SfsDirectory &dir = sfsCtrl_->getSfsDirectory();
	SfsMainDirTraversalCheckbox->SetValue(dir.isDirTraversalEnabled());
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
	wxString	baseDir = dir.getPath();
	wxString	checksumInfo, signatureInfo;
	int		checksumIconIndex = 0, signatureIconIndex = 0;

	switch (entry.getChecksumState(SfsEntry::SFSENTRY_CHECKSUM)) {
	case SfsEntry::SFSENTRY_NOT_VALIDATED:
		checksumInfo = _("not validated");
		checksumIconIndex = 0;
		break;
	case SfsEntry::SFSENTRY_MISSING:
		checksumInfo = _("not registered");
		checksumIconIndex = 0;
		break;
	case SfsEntry::SFSENTRY_INVALID:
		checksumInfo = _("invalid");
		checksumIconIndex = 2;
		break;
	case SfsEntry::SFSENTRY_NOMATCH:
		checksumInfo = wxEmptyString;
		checksumIconIndex = 3;
		break;
	case SfsEntry::SFSENTRY_MATCH:
		checksumInfo = wxEmptyString;
		checksumIconIndex = 1;
		break;
	}

	switch (entry.getChecksumState(SfsEntry::SFSENTRY_SIGNATURE)) {
	case SfsEntry::SFSENTRY_NOT_VALIDATED:
		signatureInfo = _("not validated");
		signatureIconIndex = 0;
		break;
	case SfsEntry::SFSENTRY_MISSING:
		signatureInfo = _("not registered");
		signatureIconIndex = 0;
		break;
	case SfsEntry::SFSENTRY_INVALID:
		signatureInfo = _("invalid");
		signatureIconIndex = 2;
		break;
	case SfsEntry::SFSENTRY_NOMATCH:
		signatureIconIndex = 3;
		break;
	case SfsEntry::SFSENTRY_MATCH:
		signatureIconIndex = 1;
		break;
	}

	SfsMainListCtrl->SetItem(idx,
	    MODSFSMAIN_FILELIST_COLUMN_FILE, entry.getRelativePath(baseDir));
	SfsMainListCtrl->SetItem(idx,
	    MODSFSMAIN_FILELIST_COLUMN_CHECKSUM, checksumInfo,
	    checksumIconIndex);
	SfsMainListCtrl->SetItem(idx,
	    MODSFSMAIN_FILELIST_COLUMN_SIGNATURE, signatureInfo,
	    signatureIconIndex);
}

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

#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>

#include "main.h"
#include "KeyCtrl.h"
#include "ModSfsAddPolicyVisitor.h"
#include "ModSfsListCtrl.h"
#include "ModSfsMainPanelImpl.h"
#include "SfsCtrl.h"

ModSfsMainPanelImpl::ModSfsMainPanelImpl(wxWindow* parent,
    wxWindowID id) : Observer(NULL), ModSfsMainPanelBase(parent, id)
{
	columnNames_[COLUMN_PATH] = _("Path");
	columnNames_[COLUMN_SUB] = _("Subject");
	columnNames_[COLUMN_VA] = _("Valid action");
	columnNames_[COLUMN_IA] = _("Invalid action");
	columnNames_[COLUMN_UA] = _("Unknown action");
	columnNames_[COLUMN_SCOPE] = _("Temporary");
	columnNames_[COLUMN_USER] = _("User");

	userRuleSetId_  = -1;
	adminRuleSetId_ = -1;

	for (int i=0; i<COLUMN_EOL; i++) {
		lst_Rules->InsertColumn(i, columnNames_[i], wxLIST_FORMAT_LEFT,
		    wxLIST_AUTOSIZE_USEHEADER);
	}

	AnEvents::getInstance()->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModSfsMainPanelImpl::onLoadRuleSet),
	    NULL, this);
	Hide();

	initSfsOptions();
	initSfsMain();
}

ModSfsMainPanelImpl::~ModSfsMainPanelImpl(void)
{
	AnEvents::getInstance()->Disconnect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModSfsMainPanelImpl::onLoadRuleSet),
	    NULL, this);

	saveSfsOptions();
	destroySfsMain();
}

void
ModSfsMainPanelImpl::addSfsDefaultFilterPolicy(SfsDefaultFilterPolicy *policy)
{
	long    idx;

	idx = ruleListAppend(policy);
	updateSfsDefaultFilterPolicy(idx);
}

void
ModSfsMainPanelImpl::addSfsFilterPolicy(SfsFilterPolicy *policy)
{
	long    idx;

	idx = ruleListAppend(policy);
	updateSfsFilterPolicy(idx);
}

void
ModSfsMainPanelImpl::update(Subject *subject)
{
	long	idx;

	if (subject->IsKindOf(CLASSINFO(SfsFilterPolicy))) {
		idx = findListRow((Policy *)subject);
		if (idx != -1) {
			updateSfsFilterPolicy(idx);
		}
	} else if (subject->IsKindOf(CLASSINFO(SfsDefaultFilterPolicy))) {
		idx = findListRow((Policy *)subject);
		if (idx != -1) {
			updateSfsDefaultFilterPolicy(idx);
		}
	} else {
		/* Unknown subject type - do nothing */
	}
}

void
ModSfsMainPanelImpl::updateDelete(Subject *subject)
{
	long    idx = -1;

	idx = findListRow((Policy *)subject);
	if (idx != -1) {
		lst_Rules->SetItemPtrData(idx, (wxUIntPtr)0);
	}
}

void
ModSfsMainPanelImpl::OnSfsMainDirCtrlSelChanged(wxTreeEvent &)
{
	/* Update controller */
	currentOperation_ = OP_SHOW_ALL;
	sfsCtrl_->setPath(SfsMainDirCtrl->GetPath());
}

void
ModSfsMainPanelImpl::OnSfsOperationFinished(wxCommandEvent&)
{
	if (currentOperation_ == OP_SHOW_CHANGED)
		SfsMainListCtrl->refreshList(ModSfsListCtrl::SHOW_CHANGED);
	else if (currentOperation_ == OP_SHOW_CHECKSUMS)
		SfsMainListCtrl->refreshList(ModSfsListCtrl::SHOW_CHECKSUM);
	else if (currentOperation_ == OP_SHOW_ORPHANED)
		SfsMainListCtrl->refreshList(ModSfsListCtrl::SHOW_ALL);
}

void
ModSfsMainPanelImpl::OnSfsDirChanged(wxCommandEvent&)
{
	/*
	 * Directory was changed somewhere else, not by selecting the directory
	 * in the dir-ctrl.
	 */
	if (SfsMainDirCtrl->GetPath() != sfsCtrl_->getPath())
		SfsMainDirCtrl->SetPath(sfsCtrl_->getPath());

	/* Display changes */
	SfsMainListCtrl->refreshList(ModSfsListCtrl::SHOW_EXISTING);

	/* Update CurrPathLabel accordingly */
	SfsMainCurrPathLabel->SetLabel(SfsMainDirCtrl->GetPath());
}

void
ModSfsMainPanelImpl::OnSfsMainDirTraversalChecked(wxCommandEvent&)
{
	sfsCtrl_->setRecursive(SfsMainDirTraversalCheckbox->GetValue());
}

void
ModSfsMainPanelImpl::OnSfsEntryChanged(wxCommandEvent &event)
{
	int sfsIndex = event.GetInt();
	int listIndex = SfsMainListCtrl->getListIndexOf(sfsIndex);
	SfsMainListCtrl->refreshEntry(listIndex);
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
	currentOperation_ = OP_SHOW_ALL;

	SfsDirectory &dir = sfsCtrl_->getSfsDirectory();
	unsigned int listSize = SfsMainListCtrl->GetItemCount();

	if (listSize < dir.getNumEntries())
		SfsMainListCtrl->refreshList(ModSfsListCtrl::SHOW_EXISTING);

	applySfsValidateAll(false);
}

void
ModSfsMainPanelImpl::OnSfsMainApplyButtonClicked(wxCommandEvent&)
{
	applySfsAction(SfsMainListCtrl->getSelectedIndexes());
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
ModSfsMainPanelImpl::applySfsValidateAll(bool orphaned)
{
	SfsCtrl::CommandResult result = sfsCtrl_->validateAll(orphaned);

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
ModSfsMainPanelImpl::OnSfsMainSearchOrphanedClicked(wxCommandEvent&)
{
	currentOperation_ = OP_SHOW_ORPHANED;
	applySfsValidateAll(true);
}

void
ModSfsMainPanelImpl::OnSfsMainShowAllChecksumsClicked(wxCommandEvent&)
{
	currentOperation_ = OP_SHOW_CHECKSUMS;
	applySfsValidateAll(false);
}

void
ModSfsMainPanelImpl::OnSfsMainShowChangedClicked(wxCommandEvent&)
{
	currentOperation_ = OP_SHOW_CHANGED;
	applySfsValidateAll(false);
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
ModSfsMainPanelImpl::OnPrivKeyValidityChanged(wxCommandEvent&)
{
	/* Test whether "Until session end" is selected */
	bool sessionEnd = (PrivKeyValidityChoice->GetCurrentSelection() == 0);
	int validity = !sessionEnd ? PrivKeyValiditySpinCtrl->GetValue() : 0;

	privKeyParamsUpdate(PrivKeyPathText->GetValue(), sessionEnd, validity);
}

void
ModSfsMainPanelImpl::OnPrivKeyEntered(wxCommandEvent&)
{
	privKeyParamsUpdate(
	    PrivKeyPathText->GetValue(),
	    PrivKeyValidityChoice->GetCurrentSelection() == 0,
	    PrivKeyValiditySpinCtrl->GetValue());
}

void
ModSfsMainPanelImpl::OnPrivKeyChooseClicked(wxCommandEvent&)
{
	wxFileDialog dlg(this,
	    _("Choose the file, where your private key is stored."));

	if (dlg.ShowModal() == wxID_OK) {
		privKeyParamsUpdate(
		    dlg.GetPath(),
		    PrivKeyValidityChoice->GetCurrentSelection() == 0,
		    PrivKeyValiditySpinCtrl->GetValue());
	}
}

void
ModSfsMainPanelImpl::OnPrivKeyValidityPeriodChanged(wxSpinEvent&)
{
	/* Change validity settings of private key */
	privKeyParamsUpdate(
	    PrivKeyPathText->GetValue(),
	    PrivKeyValidityChoice->GetCurrentSelection() == 0,
	    PrivKeyValiditySpinCtrl->GetValue());
}

void
ModSfsMainPanelImpl::OnCertEntered(wxCommandEvent&)
{
	certificateParamsUpdate(CertPathText->GetValue());
}

void
ModSfsMainPanelImpl::OnCertChooseClicked(wxCommandEvent&)
{
	wxFileDialog dlg(this,
	    _("Choose the file, where your certificate is stored."));

	if (dlg.ShowModal() == wxID_OK)
		certificateParamsUpdate(dlg.GetPath());
}

void
ModSfsMainPanelImpl::initSfsMain(void)
{
	sfsCtrl_ = new SfsCtrl;
	SfsMainListCtrl->setSfsCtrl(sfsCtrl_);

	currentOperation_ = OP_NOP;

	sfsCtrl_->Connect(anEVT_SFSOPERATION_FINISHED,
	    wxCommandEventHandler(ModSfsMainPanelImpl::OnSfsOperationFinished),
	    NULL, this);
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

	/* Setting up CurrPathLabel with initial path */
	SfsMainCurrPathLabel->SetLabel(SfsMainDirCtrl->GetPath());
	sfsCtrl_->setPath(SfsMainDirCtrl->GetPath());

	SfsMainDirTraversalCheckbox->SetValue(sfsCtrl_->isRecursive());
}

void
ModSfsMainPanelImpl::destroySfsMain(void)
{
	delete sfsCtrl_;
}

void
ModSfsMainPanelImpl::initSfsOptions(void)
{
	wxConfig *options = wxGetApp().getUserOptions();
	KeyCtrl *keyCtrl = KeyCtrl::getInstance();
	wxString oldPath = options->GetPath();  /* Original path */

	/* Private key */
	PrivKey &privKey = keyCtrl->getPrivateKey();
	wxString keyPath = privKey.getFile();
	int validity = privKey.getValidity();

	options->SetPath(wxT("/Options/PrivateKey"));
	options->Read(wxT("path"), &keyPath);
	options->Read(wxT("validity"), &validity);

	privKeyParamsUpdate(keyPath, (validity == 0), validity);

	/* Certificate */
	LocalCertificate &cert = keyCtrl->getLocalCertificate();
	wxString certPath = cert.getFile();

	options->SetPath(wxT("/Options/LocalCertificate"));
	options->Read(wxT("path"), &certPath);

	certificateParamsUpdate(certPath);

	/* Reset to original path */
	options->SetPath(oldPath);
}

void
ModSfsMainPanelImpl::saveSfsOptions(void)
{
	wxConfig *options = wxGetApp().getUserOptions();
	wxString oldPath = options->GetPath(); /* Original path */

	/* Private key */
	PrivKey &privKey = KeyCtrl::getInstance()->getPrivateKey();

	options->SetPath(wxT("/Options/PrivateKey"));
	options->Write(wxT("path"), privKey.getFile());
	options->Write(wxT("validity"), privKey.getValidity());

	/* Certificate */
	LocalCertificate &cert = KeyCtrl::getInstance()->getLocalCertificate();

	options->SetPath(wxT("/Options/LocalCertificate"));
	options->Write(wxT("path"), cert.getFile());

	/* Reset to original path */
	options->SetPath(oldPath);
}

void
ModSfsMainPanelImpl::privKeyParamsUpdate(const wxString &path, bool sessionEnd,
    int validity)
{
	PrivKey &privKey = KeyCtrl::getInstance()->getPrivateKey();

	privKey.setFile(path);
	privKey.setValidity(validity);

	if (!path.IsEmpty() && !privKey.canLoad()) {
		wxMessageBox(
		    _("Cannot load the private key!\n"
		      "The file you specified does not exist."),
		    _("Load private key"), wxOK | wxICON_ERROR, this);
	}

	/* Update view */
	PrivKeyPathText->SetValue(path);

	PrivKeyValiditySpinCtrl->Enable(!sessionEnd);
	PrivKeyValidityText->Enable(!sessionEnd);

	PrivKeyValidityChoice->SetSelection(sessionEnd ? 0 : 1);
	PrivKeyValiditySpinCtrl->SetValue(validity);

	/* Inform any listener about change of key-configuration */
	wxCommandEvent event(anEVT_LOAD_KEY);
	event.SetInt(0); /* 0 := private key */
	wxPostEvent(AnEvents::getInstance(), event);
}

void
ModSfsMainPanelImpl::certificateParamsUpdate(const wxString &path)
{
	LocalCertificate &cert = KeyCtrl::getInstance()->getLocalCertificate();

	cert.setFile(path);

	if (!path.IsEmpty()) {
		if (!cert.load()) {
			wxMessageBox(wxString::Format(
			    _("Failed to load certificate from\n%s."),
			    cert.getFile().c_str()),
			    _("Load certificate"), wxOK | wxICON_ERROR, this);
		}
	} else {
		/* Reset certificate */
		cert.unload();
	}

	/* Update view */
	CertPathText->SetValue(cert.getFile());
	CertFingerprintText->SetLabel(cert.getFingerprint());
	CertDnText->SetLabel(cert.getDistinguishedName());

	/* Inform any listener about change of configuration */
	wxCommandEvent event(anEVT_LOAD_KEY);
	event.SetInt(1); /* 1 := certificate */

	wxPostEvent(AnEvents::getInstance(), event);
}

long
ModSfsMainPanelImpl::findListRow(Policy *policy)
{
	for (int i = lst_Rules->GetItemCount(); i >= 0; i--) {
		if (policy == (Policy *)lst_Rules->GetItemData(i)) {
			return (i);
		}
	}

	return (-1);
}

void
ModSfsMainPanelImpl::removeListRow(long idx)
{
	Policy	*policy;

	policy = wxDynamicCast((void*)lst_Rules->GetItemData(idx), Policy);
	if (policy != NULL) {
		/* deregister for observing policy */
		removeSubject(policy);
	}

	lst_Rules->DeleteItem(idx);
}

void
ModSfsMainPanelImpl::onLoadRuleSet(wxCommandEvent& event)
{
	ModSfsAddPolicyVisitor	 addVisitor(this);
	PolicyRuleSet		*ruleSet;
	ProfileCtrl		*profileCtrl;

	profileCtrl = ProfileCtrl::getInstance();

	/* clear the whole list */
	for (int i = lst_Rules->GetItemCount() - 1; i >= 0; i--) {
		removeListRow(i);
	}

	/* release old ones */
	ruleSet = profileCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->unlock();
		removeSubject(ruleSet);
	}

	ruleSet = profileCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->unlock();
		removeSubject(ruleSet);
	}

	userRuleSetId_ = profileCtrl->getUserId();
	adminRuleSetId_ = profileCtrl->getAdminId(geteuid());

	/* get the new ones */
	ruleSet = profileCtrl->getRuleSet(userRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->lock();
		ruleSet->accept(addVisitor);
	}

	ruleSet = profileCtrl->getRuleSet(adminRuleSetId_);
	if (ruleSet != NULL) {
		ruleSet->lock();
		ruleSet->accept(addVisitor);
	}

	/* trigger new calculation of column width */
	for (int i=0; i<COLUMN_EOL; i++) {
		lst_Rules->SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);
	}

	event.Skip();
}

long
ModSfsMainPanelImpl::ruleListAppend(Policy *policy)
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
		ruleType.Printf(_("admin ruleset of %s"), userName.c_str());
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
ModSfsMainPanelImpl::updateSfsDefaultFilterPolicy(long idx)
{
	void			*data;
	SfsDefaultFilterPolicy	*dftPolicy;

	data = (void*)lst_Rules->GetItemData(idx);
	dftPolicy = wxDynamicCast(data, SfsDefaultFilterPolicy);

	if (dftPolicy == NULL) {
		return;
	}

	lst_Rules->SetItem(idx, COLUMN_PATH, dftPolicy->getPath());
	lst_Rules->SetItem(idx, COLUMN_VA, dftPolicy->getActionName());
	/* COLUMN_USER is handled by ruleListAppend */
}

void
ModSfsMainPanelImpl::updateSfsFilterPolicy(long idx)
{
	void			*data;
	SfsFilterPolicy		*sfsPolicy;

	data = (void*)lst_Rules->GetItemData(idx);
	sfsPolicy = wxDynamicCast(data, SfsFilterPolicy);

	if (sfsPolicy == NULL) {
		return;
	}

	lst_Rules->SetItem(idx, COLUMN_PATH,
	    sfsPolicy->getPath());
	lst_Rules->SetItem(idx, COLUMN_SUB,
	    sfsPolicy->getSubjectName());
	lst_Rules->SetItem(idx, COLUMN_VA,
	    sfsPolicy->getValidActionName());
	lst_Rules->SetItem(idx, COLUMN_IA,
	    sfsPolicy->getInvalidActionName());
	lst_Rules->SetItem(idx, COLUMN_UA,
	    sfsPolicy->getUnknownActionName());
	/* COLUMN_USER and COLUMN_SCOPE is handled by ruleListAppend */
}

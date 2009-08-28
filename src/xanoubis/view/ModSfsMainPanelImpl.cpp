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

#include "wx/utils.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>

#include "AnPickFromFs.h"
#include "main.h"
#include "JobCtrl.h"
#include "KeyCtrl.h"
#include "ModSfsAddPolicyVisitor.h"
#include "ModSfsListCtrl.h"
#include "ModSfsMainPanelImpl.h"

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
	comEnabled_ = false;

	for (int i=0; i<COLUMN_EOL; i++) {
		lst_Rules->InsertColumn(i, columnNames_[i], wxLIST_FORMAT_LEFT,
		    wxLIST_AUTOSIZE_USEHEADER);
	}

	AnEvents::getInstance()->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModSfsMainPanelImpl::onLoadRuleSet),
	    NULL, this);
	AnEvents::getInstance()->Connect(anEVT_SFSBROWSER_SHOW,
	    wxCommandEventHandler(ModSfsMainPanelImpl::onSfsBrowserShow),
	    NULL, this);
	JobCtrl::getInstance()->Connect(anTASKEVT_REGISTER,
	    wxTaskEventHandler(ModSfsMainPanelImpl::OnDaemonRegistration),
	    NULL, this);
	Hide();

	addSubject(keyPicker);
	keyPicker->setTitle(_("Configure private key:"));
	keyPicker->setButtonLabel(_("Choose Key"));
	keyPicker->setMode(AnPickFromFs::MODE_FILE);

	addSubject(certificatePicker);
	certificatePicker->setTitle(_("Configure certificate:"));
	certificatePicker->setButtonLabel(_("Choose certificate"));
	certificatePicker->setMode(AnPickFromFs::MODE_FILE);

	initSfsOptions();
	initSfsMain();
}

ModSfsMainPanelImpl::~ModSfsMainPanelImpl(void)
{
	JobCtrl::getInstance()->Disconnect(anTASKEVT_REGISTER,
	    wxTaskEventHandler(ModSfsMainPanelImpl::OnDaemonRegistration),
	    NULL, this);
	AnEvents::getInstance()->Disconnect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModSfsMainPanelImpl::onLoadRuleSet),
	    NULL, this);
	AnEvents::getInstance()->Disconnect(anEVT_SFSBROWSER_SHOW,
	    wxCommandEventHandler(ModSfsMainPanelImpl::onSfsBrowserShow),
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
	} else if (subject == keyPicker) {
		privKeyParamsUpdate(
		    keyPicker->getFileName(),
		    PrivKeyValidityChoice->GetCurrentSelection() == 0,
		    PrivKeyValiditySpinCtrl->GetValue());
	} else if (subject == certificatePicker) {
		certificateParamsUpdate(certificatePicker->getFileName());
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
	wxBeginBusyCursor();
	sfsCtrl_->setPath(SfsMainDirCtrl->GetPath());
	wxEndBusyCursor();
}

void
ModSfsMainPanelImpl::OnSfsListSelected(wxListEvent &)
{
	enableSfsControls(false); /* false: No operation running */
}

void
ModSfsMainPanelImpl::OnSfsListDeselected(wxListEvent &)
{
	enableSfsControls(false); /* false: No operation running */
}

void
ModSfsMainPanelImpl::OnDaemonRegistration(TaskEvent &event)
{
	ComRegistrationTask *task =
	    dynamic_cast<ComRegistrationTask*>(event.getTask());

	if (task != 0) {
		comEnabled_ =
		   (task->getAction() == ComRegistrationTask::ACTION_REGISTER);
		event.Skip();
	} else
		comEnabled_ = false;

	enableSfsControls(false); /* false: No operation running */
}

void
ModSfsMainPanelImpl::OnSfsOperationFinished(wxCommandEvent&)
{
	/* Enable the controls again */
	enableSfsControls(false); /* false: No operation running */
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
	SfsMainListCtrl->refreshList();

	/* Update CurrPathLabel accordingly */
	SfsMainCurrPathLabel->SetLabel(SfsMainDirCtrl->GetPath());
}

void
ModSfsMainPanelImpl::OnSfsMainDirTraversalChecked(wxCommandEvent&)
{
	wxBeginBusyCursor();
	sfsCtrl_->setRecursive(SfsMainDirTraversalCheckbox->GetValue());
	wxEndBusyCursor();
}

void
ModSfsMainPanelImpl::OnSfsEntryChanged(wxCommandEvent &event)
{
	SfsMainListCtrl->refreshEntry(event.GetInt());
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
	wxBeginBusyCursor();
	sfsCtrl_->setFilter(SfsMainFilterTextCtrl->GetValue());
	wxEndBusyCursor();
}

void
ModSfsMainPanelImpl::OnSfsMainInverseCheckboxClicked(wxCommandEvent&)
{
	wxBeginBusyCursor();
	sfsCtrl_->setFilterInversed(SfsMainFilterInvertCheckBox->IsChecked());
	wxEndBusyCursor();
}

void
ModSfsMainPanelImpl::OnSfsMainValidateButtonClicked(wxCommandEvent&)
{
	SfsCtrl::CommandResult result = sfsCtrl_->validateAll();

	/* Controls are disabled if operation is executed */
	enableSfsControls(result == SfsCtrl::RESULT_EXECUTE);

	handleSfsCommandResult(result);
}

void
ModSfsMainPanelImpl::OnSfsMainApplyButtonClicked(wxCommandEvent&)
{
	SfsCtrl::CommandResult result = SfsCtrl::RESULT_EXECUTE;
	IndexArray selection = SfsMainListCtrl->getSelectedIndexes();

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

	/* Controls are disabled if operation is executed */
	enableSfsControls(result == SfsCtrl::RESULT_EXECUTE);

	handleSfsCommandResult(result);
}

void
ModSfsMainPanelImpl::OnSfsMainImportClicked(wxCommandEvent&)
{
	wxFileDialog dlg(this, _("Choose the import-file"));
	if (dlg.ShowModal() != wxID_OK) {
		/* Operation canceled */
		return;
	}

	SfsCtrl::CommandResult result =
	    sfsCtrl_->importChecksums(dlg.GetPath());

	handleSfsCommandResult(result);
}

void
ModSfsMainPanelImpl::OnSfsMainExportClicked(wxCommandEvent&)
{
	IndexArray selection = SfsMainListCtrl->getSelectedIndexes();

	if (selection.IsEmpty()) {
		/* No selection -> no export */
		return;
	}

	wxFileDialog dlg(this, _("Choose the export-destination"),
	    wxEmptyString, wxEmptyString, wxT("*.*"),
	    wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	if (dlg.ShowModal() != wxID_OK) {
		/* Operation canceled -> no export */
		return;
	}

	SfsCtrl::CommandResult result =
	    sfsCtrl_->exportChecksums(selection, dlg.GetPath());

	handleSfsCommandResult(result);
}

void
ModSfsMainPanelImpl::handleSfsCommandResult(SfsCtrl::CommandResult result)
{
	wxString msg = wxEmptyString;

	switch (result) {
	case SfsCtrl::RESULT_NOTCONNECTED:
		wxGetApp().status(
		  _("Error: xanoubis is not connected to the daemon"));
		break;
	case SfsCtrl::RESULT_NEEDKEY:
		wxGetApp().status(
		  _("Error: Failed to load the private key."));
		break;
	case SfsCtrl::RESULT_INVALIDARG:
		wxGetApp().status(
		  _("Error: An invalid argument was supplied"));
		break;
	case SfsCtrl::RESULT_BUSY:
		wxGetApp().status(
		 _("Error: Sfs is still busy with another operation."));
		break;
	case SfsCtrl::RESULT_WRONG_PASS:
		wxGetApp().status(
		    _("Wrong password!"));
		msg = _("The entered password is incorrect.");
		wxMessageBox(msg, _("SFS error"), wxOK|wxICON_ERROR, this);
		break;
	case SfsCtrl::RESULT_NOOP:
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
ModSfsMainPanelImpl::OnSfsMainDirViewChoiceSelected(wxCommandEvent &event)
{
	switch (event.GetSelection()) {
	case 0:
		sfsCtrl_->setEntryFilter(SfsCtrl::FILTER_STD);
		break;
	case 1:
		sfsCtrl_->setEntryFilter(SfsCtrl::FILTER_CHECKSUMS);
		break;
	case 2:
		sfsCtrl_->setEntryFilter(SfsCtrl::FILTER_CHANGED);
		break;
	case 3:
		sfsCtrl_->setEntryFilter(SfsCtrl::FILTER_ORPHANED);
		break;
	case 4:
		sfsCtrl_->setEntryFilter(SfsCtrl::FILTER_UPGRADED);
		break;
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
ModSfsMainPanelImpl::OnPrivKeyValidityChanged(wxCommandEvent&)
{
	/* Test whether "Until session end" is selected */
	bool sessionEnd = (PrivKeyValidityChoice->GetCurrentSelection() == 0);
	int validity = !sessionEnd ? PrivKeyValiditySpinCtrl->GetValue() : 0;

	privKeyParamsUpdate(keyPicker->getFileName(), sessionEnd, validity);
}

void
ModSfsMainPanelImpl::OnPrivKeyValidityPeriodChanged(wxSpinEvent&)
{
	/* Change validity settings of private key */
	privKeyParamsUpdate(
	    keyPicker->getFileName(),
	    PrivKeyValidityChoice->GetCurrentSelection() == 0,
	    PrivKeyValiditySpinCtrl->GetValue());
}

void
ModSfsMainPanelImpl::initSfsMain(void)
{
	sfsCtrl_ = new SfsCtrl;
	SfsMainListCtrl->setSfsCtrl(sfsCtrl_);

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
	wxBeginBusyCursor();
	sfsCtrl_->setPath(SfsMainDirCtrl->GetPath());
	wxEndBusyCursor();

	/* Make sure you have a selection for applying an sfs-operation */
	SfsMainActionChoice->SetSelection(0);

	SfsMainDirTraversalCheckbox->SetValue(sfsCtrl_->isRecursive());
}

void
ModSfsMainPanelImpl::destroySfsMain(void)
{
	delete sfsCtrl_;
}

void
ModSfsMainPanelImpl::enableSfsControls(bool sfsOpRunning)
{
	bool haveSelection = (SfsMainListCtrl->GetSelectedItemCount() > 0);

	SfsMainFilterValidateButton->Enable(comEnabled_ && !sfsOpRunning);
	SfsMainDirViewChoice->Enable(comEnabled_ && !sfsOpRunning);
	SfsMainImportButton->Enable(comEnabled_ && !sfsOpRunning);
	SfsMainExportButton->Enable(
	    comEnabled_ && !sfsOpRunning && haveSelection);
	SfsMainActionChoice->Enable(
	    comEnabled_ && !sfsOpRunning && haveSelection);
	SfsMainActionButton->Enable(
	    comEnabled_ && !sfsOpRunning && haveSelection);

	SfsMainFilterTextCtrl->Enable(!sfsOpRunning);
	SfsMainFilterButton->Enable(!sfsOpRunning);
	SfsMainDirTraversalCheckbox->Enable(!sfsOpRunning);
	SfsMainFilterInvertCheckBox->Enable(!sfsOpRunning);
}

void
ModSfsMainPanelImpl::initSfsOptions(void)
{
	wxConfig *options = wxGetApp().getUserOptions();
	KeyCtrl *keyCtrl = KeyCtrl::getInstance();
	wxString oldPath = options->GetPath();  /* Original path */

	/* Private key */
	PrivKey &privKey = keyCtrl->getPrivateKey();
	wxString keyPath = wxEmptyString;
	int validity = privKey.getValidity();

	options->SetPath(wxT("/Options/PrivateKey"));
	options->Read(wxT("path"), &keyPath);
	options->Read(wxT("validity"), &validity);

	if (keyPath.IsEmpty()) {
		keyPath = privKey.getFile();
	}
	privKeyParamsUpdate(keyPath, (validity == 0), validity);

	/* Certificate */
	LocalCertificate &cert = keyCtrl->getLocalCertificate();
	wxString certPath = wxEmptyString;

	options->SetPath(wxT("/Options/LocalCertificate"));
	options->Read(wxT("path"), &certPath);

	if (certPath.IsEmpty()) {
		certPath = cert.getFile();
	}
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
	keyPicker->setFileName(path);

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
			    _("Failed to load certificate from\n%ls."),
			    cert.getFile().c_str()),
			    _("Load certificate"), wxOK | wxICON_ERROR, this);
		}
	} else {
		/* Reset certificate */
		cert.unload();
	}

	/* Update view */
	certificatePicker->setFileName(cert.getFile());
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
		ruleSet->accept(addVisitor);
	}

	ruleSet = policyCtrl->getRuleSet(adminRuleSetId_);
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

void
ModSfsMainPanelImpl::onSfsBrowserShow(wxCommandEvent& event)
{
	/* select Browser tab of ModSfs */
	note_MainSfs->SetSelection(1);
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
	lst_Rules->SetItem(idx, COLUMN_VA, wxString::Format(_("default %ls"),
	    dftPolicy->getActionName().c_str()));
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

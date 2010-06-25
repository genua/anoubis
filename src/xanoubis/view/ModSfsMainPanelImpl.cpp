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

#include <wx/log.h>

#include "AnEvents.h"
#include "AnGrid.h"
#include "AnPickFromFs.h"
#include "AnMessageDialog.h"
#include "AnIconList.h"
#include "JobCtrl.h"
#include "main.h"
#include "ModSfsListCtrl.h"
#include "ModSfsMainPanelImpl.h"
#include "PolicyRuleSet.h"
#include "SfsOverviewAttrProvider.h"
#include "SfsOverviewTable.h"
#include "SimpleOverviewRow.h"
#include "ModSfsGenerateKeyDlg.h"
#include "MainUtils.h"

ModSfsMainPanelImpl::ModSfsMainPanelImpl(wxWindow* parent, wxWindowID id)
    : ModSfsMainPanelBase(parent, id), Observer(NULL)
{
	wxSize		 indent;
	wxSizerItem	*spacer;
	wxIcon		 icon;

	comEnabled_ = false;

	SfsOverviewTable *table = new SfsOverviewTable;
	table->SetAttrProvider(new SfsOverviewAttrProvider);
	lst_Rules->SetTable(table, true, wxGrid::wxGridSelectRows);
	lst_Rules->setCursorVisibility(false);

	lst_Rules->SetColSize(0, 198);
	lst_Rules->SetColSize(1, 95);
	lst_Rules->SetColSize(2, 95);
	lst_Rules->SetColSize(3, 105);
	lst_Rules->SetColSize(4, 125);
	lst_Rules->SetColSize(5, 90);
	lst_Rules->SetColSize(6, 50);

	AnEvents::getInstance()->Connect(anEVT_SFSBROWSER_SHOW,
	    wxCommandEventHandler(ModSfsMainPanelImpl::onSfsBrowserShow),
	    NULL, this);
	JobCtrl::getInstance()->Connect(anEVT_COM_CONNECTION,
	   wxCommandEventHandler(ModSfsMainPanelImpl::OnConnectionStateChange),
	   NULL, this);
	Hide();

	addSubject(keyPicker);
	keyPicker->setTitle(_("Configure private key:"));
	keyPicker->setButtonLabel(_("Browse ..."));
	keyPicker->setMode(AnPickFromFs::MODE_FILE
	    | AnPickFromFs::MODE_NOINFO | AnPickFromFs::MODE_NORESOLVE);

	addSubject(certificatePicker);
	certificatePicker->setTitle(_("Configure certificate:"));
	certificatePicker->setButtonLabel(_("Browse ..."));
	certificatePicker->setMode(AnPickFromFs::MODE_FILE
	    | AnPickFromFs::MODE_NOINFO | AnPickFromFs::MODE_NORESOLVE);

	/* Get initial indent size. */
	spacer = certDetailsIndentSizer->GetItem((size_t)0);
	indent = keyPicker->getTitleSize();

	/* Set indentation. */
	keyPicker->setTitleMinSize(indent);
	certificatePicker->setTitleMinSize(indent);
	passphraseValidityLabel->SetMinSize(indent);

	/* Adjust indent by indentation of details block. */
	indent.DecBy(spacer->GetSize().GetWidth(), 0);
	certFingerprintLabel->SetMinSize(indent);

	/* Set icon for key warning message */
	icon = AnIconList::getInstance()->GetIcon(AnIconList::ICON_PROBLEM_48);
	keyWarningIcon->SetIcon(icon);

	Layout();
	Refresh();

	initSfsOptions();
	initSfsMain();
}

ModSfsMainPanelImpl::~ModSfsMainPanelImpl(void)
{
	JobCtrl::getInstance()->Disconnect(anEVT_COM_CONNECTION,
	   wxCommandEventHandler(ModSfsMainPanelImpl::OnConnectionStateChange),
	   NULL, this);
	AnEvents::getInstance()->Disconnect(anEVT_SFSBROWSER_SHOW,
	    wxCommandEventHandler(ModSfsMainPanelImpl::onSfsBrowserShow),
	    NULL, this);

	/*
	 * Disconnect page changing event from notebook to prevent segfault
	 * on program close!
	 */
	note_MainSfs->Disconnect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING,
	    wxNotebookEventHandler(ModSfsMainPanelImpl::onSfsTabChange),
	    NULL, this);

	saveSfsOptions();
	destroySfsMain();
}

void
ModSfsMainPanelImpl::update(Subject *subject)
{
	if (subject == keyPicker) {
		int	validity = 0;

		if (privKeyValidityChoice->GetCurrentSelection())
			validity = privKeyValiditySpinCtrl->GetValue();
		privKeyParamsUpdate(keyPicker->getFileName(), validity);
	} else if (subject == certificatePicker) {
		bool changed =
		    certificateParamsUpdate(certificatePicker->getFileName());

		if (changed) {
			AnMessageDialog *dlg = new AnMessageDialog(this,
			    _("You have selected a new certificate. To be "
			    "able to use the key, you need to activate it! "
			    "Please inform your administrator."),
			    _("Certificate notice"),
			    wxOK | wxICON_INFORMATION);
			dlg->onNotifyCheck(wxT("/Options/ShowCertMessage"));
			dlg->ShowModal();
			dlg->Destroy();
		}
	} else {
		/* Unknown subject type - do nothing */
	}

	/* Check key config after path changes */
	if (subject == keyPicker || subject == certificatePicker) {
		checkKeyConfiguration();
	}
}

void
ModSfsMainPanelImpl::updateDelete(Subject *)
{
}

wxString
ModSfsMainPanelImpl::compareKeyPair(void)
{
	KeyCtrl *keyCtrl = KeyCtrl::instance();
	PrivKey &privKey = keyCtrl->getPrivateKey();
	LocalCertificate &localCert = keyCtrl->getLocalCertificate();

	/* Load private key if not already loaded! */
	if (!privKey.isLoaded() && privKey.canLoad())
		keyCtrl->loadPrivateKey();

	/* Load certificate if not already loaded! */
	if (!localCert.isLoaded() && localCert.canLoad())
		localCert.load();

	if (!localCert.canLoad() && !privKey.canLoad())
		return _("No keys are configured. "
		    "You will not be able to make use of signatures.");
	if (!privKey.isLoaded()) {
		if (privKey.canLoad())
			return _("Failed to load private key");
		else
			return _("You have no private key configured. "
			    "This may lead to unexpected problems.");
	}
	if (!localCert.isLoaded()) {
		if (localCert.canLoad())
			return _("Failed too load certificate");
		else
			return _("You have no certificate configured. "
			    "This may lead to unexpected problems.");
	}

	/* Get the 'real' key data */
	struct anoubis_sig *key = privKey.getKey();
	struct anoubis_sig *cert = localCert.getCertificate();

	/* Compare the keys */
	if (anoubis_sig_keycmp(key, cert) != 0)
		return _("Your private key and certificate do not match. "
		    "This may lead to unexpected problems.");
	return wxEmptyString;
}

void
ModSfsMainPanelImpl::onSfsTabChange(wxNotebookEvent& event)
{
	/* Check key config if target is keysTab */
	if (note_MainSfs->GetPage(event.GetSelection()) == keysTab) {
		checkKeyConfiguration();
	}
}

void
ModSfsMainPanelImpl::checkKeyConfiguration(void)
{
	wxString	msg = compareKeyPair();

	if (msg == wxEmptyString) {
		keyWarningPanel->Hide();
	} else {
		keyWarningText->SetLabel(msg);
		keyWarningPanel->Show();
	}
	Layout();
	Refresh();
}

void
ModSfsMainPanelImpl::OnGridCellLeftDClick(wxGridEvent& event)
{
	wxCommandEvent		showEvent(anEVT_SHOW_RULE);
	SfsOverviewTable	*table;
	SimpleOverviewRow	*tableRow;
	Policy			*policy;
	PolicyRuleSet		*ruleset;

	table = dynamic_cast<SfsOverviewTable *> (lst_Rules->GetTable());
	if (table == NULL)
		return;

	tableRow = table->getRowAt(event.GetRow());

	policy = tableRow->getFilterPolicy();
	if (policy == NULL)
		policy = tableRow->getApplicationPolicy();
	if (policy == NULL)
		return;

	ruleset = policy->getParentRuleSet();
	if (ruleset == NULL)
		return;

	showEvent.SetInt(ruleset->isAdmin());
	showEvent.SetExtraLong(policy->getApnRuleId());
	wxPostEvent(AnEvents::getInstance(), showEvent);
}

void
ModSfsMainPanelImpl::OnSfsPathChanged(wxCommandEvent &)
{
	wxString enteredPath = SfsMainPathCtrl->GetValue();
	wxString message = wxEmptyString;

	/* Remove whitespaces from both sides */
	enteredPath.Trim();
	enteredPath.Trim(false);

	if (wxDirExists(enteredPath) == true) {
		/* Update tree */
		wxBeginBusyCursor();
		sfsCtrl_->setPath(enteredPath);
		wxEndBusyCursor();
	} else {
		/* stay in old directory */
		SfsMainPathCtrl->ChangeValue(SfsMainDirCtrl->GetPath());
		/* Errormessage */
		message = _("The given path does not exist.");
		anMessageBox(message, _("SFS error"), wxOK|wxICON_ERROR, this);
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
ModSfsMainPanelImpl::OnConnectionStateChange(wxCommandEvent &event)
{
	JobCtrl::ConnectionState state =
	    (JobCtrl::ConnectionState)event.GetInt();
	comEnabled_ = (state == JobCtrl::CONNECTED);

	if (!comEnabled_ &&
	    (sfsCtrl_->getEntryFilter() != SfsCtrl::FILTER_STD)) {
		/*
		 * You are disconnected and the current selected filter cannot
		 * be used in disconnected state! Switch back to local
		 * filesystem.
		 */
		anMessageBox(_("The SFS Browser currently displays an invalid "
		    "view because no daemon connection is established. The "
		    "selection is switched back to the "
		    "\"Standard File Browsing\" view."),
		    _("SFS warning"), wxOK | wxICON_EXCLAMATION, this);

		SfsMainDirViewChoice->SetSelection(0);
		SfsMainDirTraversalCheckbox->SetValue(false);

		sfsCtrl_->setEntryFilter(SfsCtrl::FILTER_STD, false);
	}

	enableSfsControls(false); /* false: No operation running */

	event.Skip();
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

	/* Update CurrPathLabel accordingly */
	SfsMainPathCtrl->ChangeValue(SfsMainDirCtrl->GetPath());
}

void
ModSfsMainPanelImpl::OnSfsMainDirTraversalChecked(wxCommandEvent&)
{
	wxBeginBusyCursor();
	sfsCtrl_->setRecursive(SfsMainDirTraversalCheckbox->GetValue());
	wxEndBusyCursor();
}

void
ModSfsMainPanelImpl::OnSfsError(wxCommandEvent&)
{
	const wxArrayString	&errors = sfsCtrl_->getErrors();
	int			 needSummary = (errors.Count() > 1);

	for (unsigned int i = 0; i < errors.Count(); i++) {
		wxLogError(errors[i]);
	}
	if (needSummary)
		wxLogError(_("Multiple errors occured "
		    "while processing the SFS request"));
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
	enableSfsControls(true);
	SfsCtrl::CommandResult result = sfsCtrl_->validateAll();
	handleSfsCommandResult(result);
}

void
ModSfsMainPanelImpl::OnSfsMainApplyButtonClicked(wxCommandEvent&)
{
	SfsCtrl::CommandResult result = SfsCtrl::RESULT_EXECUTE;
	IndexArray selection = SfsMainListCtrl->getSelectedIndexes();

	enableSfsControls(true);
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
		MainUtils::instance()->status(
		  _("Error: xanoubis is not connected to the daemon"));
		break;
	case SfsCtrl::RESULT_NEEDKEY:
		MainUtils::instance()->status(
		  _("Error: Failed to load the private key."));
		break;
	case SfsCtrl::RESULT_INVALIDARG:
		MainUtils::instance()->status(
		  _("Error: An invalid argument was supplied"));
		break;
	case SfsCtrl::RESULT_BUSY:
		MainUtils::instance()->status(
		 _("Error: Sfs is still busy with another operation."));
		break;
	case SfsCtrl::RESULT_WRONG_PASS:
		MainUtils::instance()->status(
		    _("Wrong password!"));
		msg = _("The entered password is incorrect.");
		anMessageBox(msg, _("SFS error"), wxOK|wxICON_ERROR, this);
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

		anMessageBox(msg, _("SFS error"), wxOK|wxICON_ERROR, this);
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
	bool keysUsable = KeyCtrl::instance()->canUseLocalKeys();
	bool sigEnabled = sfsCtrl_->isSignatureEnabled();

	SfsMainSignFilesCheckBox->Enable(keysUsable);
	SfsMainSignFilesCheckBox->SetValue(sigEnabled);
}

void
ModSfsMainPanelImpl::onPrivKeyValidityChanged(wxCommandEvent&)
{
	/* Test whether "Until session end" is selected */
	int	validity = 0;

	if (privKeyValidityChoice->GetCurrentSelection() != 0)
		validity = privKeyValiditySpinCtrl->GetValue();
	privKeyParamsUpdate(keyPicker->getFileName(), validity);
}

void
ModSfsMainPanelImpl::onPrivKeyValidityPeriodChanged(wxSpinEvent&)
{
	int	validity = 0;

	/* Change validity settings of private key */
	if (privKeyValidityChoice->GetCurrentSelection() != 0)
		validity = privKeyValiditySpinCtrl->GetValue();
	privKeyParamsUpdate(keyPicker->getFileName(), validity);
}

void
ModSfsMainPanelImpl::onGenerateKeyPairButton(wxCommandEvent&)
{
	/* Display Generate-Keypair-Dialogue */
	ModSfsGenerateKeyDlg *dlg = new ModSfsGenerateKeyDlg(this);
	int result = dlg->ShowModal();
	int validity = 0;

	if (result != wxOK) {
		/* Cancel button was used. */
		dlg->Destroy();
		return;
	}

	if (privKeyValidityChoice->GetCurrentSelection() != 0)
		validity = privKeyValiditySpinCtrl->GetValue();
	if (keyPicker->getFileName().IsEmpty() &&
	    certificatePicker->getFileName().IsEmpty()) {
		/* This is the first keypair, configure it */
		privKeyParamsUpdate(dlg->getPathPrivateKey(), validity);
		certificateParamsUpdate(dlg->getPathCertificate());

		AnMessageDialog *infoDlg = new AnMessageDialog(this,
		    _("You have created a new keypair. To be "
		    "able to use it, you need to activate it! "
		    "Please inform your administrator."),
		    _("Certificate notice"),
		    wxOK | wxICON_INFORMATION);
		infoDlg->onNotifyCheck(wxT("/Options/ShowCertMessage"));
		infoDlg->ShowModal();
		infoDlg->Destroy();
	} else {
		result = anMessageBox(_("A keypair has already been "
		    "configured. Would you like to use the newly generated "
		    "keypair from now on?\n\nIf so, the certificate needs to "
		    "be activated. Please inform your administrator."),
		    _("Certificate notice"),
		    wxYES_NO | wxICON_QUESTION, this);

		if (result == wxYES) {
			privKeyParamsUpdate(dlg->getPathPrivateKey(), validity);
			certificateParamsUpdate(dlg->getPathCertificate());
		}
	}

	dlg->Destroy();
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
	sfsCtrl_->Connect(anEVT_SFSENTRY_ERROR,
	    wxCommandEventHandler(ModSfsMainPanelImpl::OnSfsError),
	    NULL, this);
	AnEvents::getInstance()->Connect(anEVT_LOAD_KEY,
	    wxCommandEventHandler(ModSfsMainPanelImpl::OnSfsMainKeyLoaded),
	    NULL, this);

	/* Setting up CurrPathLabel with initial path */
	SfsMainPathCtrl->ChangeValue(SfsMainDirCtrl->GetPath());
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
	static int	lastSelection = 0;
	int		nextSelection;
	bool		haveSelection;

	/**
	 * NOTE: This code is performance critical:
	 * We get a "line selected" event for every new line in a selection
	 * and at least some wxWidgets versions have linear implemenations
	 * of GetNextItem and/or GetSelectedItemCount. Thus we must not start
	 * the search for selected Items at the beginning of the list but
	 * at the last element that was found to be selected. This is stored
	 * in the static variable lastSelection. GetNextItem starts its
	 * search at the item immediately _after_ the index given as the
	 * first parameter.
	 */

	/*
	 * First make sure that the start point is less than the number of
	 * elements in the list.
	 */
	if (lastSelection >= SfsMainListCtrl->GetItemCount()-1)
		lastSelection = 0;
	/*
	 * Start the search at the last element that we found to be
	 * selected. The -1 is neccessary because the search only starts
	 * after the index given as the first parameter to GetNextIndex.
	 */
	nextSelection = SfsMainListCtrl->GetNextItem(lastSelection - 1,
	    wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	/*
	 * If we did not find a selected Item and did not start at the
	 * beginning, do a search that starts at the beginning, now.
	 */
	if (nextSelection == -1 && lastSelection != 0) {
		lastSelection = 0;
		nextSelection = SfsMainListCtrl->GetNextItem(lastSelection - 1,
		    wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	}
	/* Update lastSelection and set haveSelection. */
	if (nextSelection >= 0) {
		lastSelection = nextSelection;
		haveSelection = true;
	} else {
		haveSelection = false;
	}

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
	KeyCtrl *keyCtrl = KeyCtrl::instance();
	wxString oldPath = wxConfig::Get()->GetPath();  /* Original path */

	/* Private key */
	PrivKey &privKey = keyCtrl->getPrivateKey();
	wxString keyPath = wxEmptyString;
	int validity = privKey.getValidity();

	wxConfig::Get()->SetPath(wxT("/Options/PrivateKey"));
	wxConfig::Get()->Read(wxT("path"), &keyPath);
	wxConfig::Get()->Read(wxT("validity"), &validity);

	if (keyPath.IsEmpty()) {
		keyPath = privKey.getFile();
	}
	privKeyParamsUpdate(keyPath, validity);

	/* Certificate */
	LocalCertificate &cert = keyCtrl->getLocalCertificate();
	wxString certPath = wxEmptyString;

	wxConfig::Get()->SetPath(wxT("/Options/LocalCertificate"));
	wxConfig::Get()->Read(wxT("path"), &certPath);

	if (certPath.IsEmpty()) {
		certPath = cert.getFile();
	}
	certificateParamsUpdate(certPath);

	/* Reset to original path */
	wxConfig::Get()->SetPath(oldPath);
}

void
ModSfsMainPanelImpl::saveSfsOptions(void)
{
	wxString oldPath = wxConfig::Get()->GetPath(); /* Original path */

	/* Private key */
	PrivKey &privKey = KeyCtrl::instance()->getPrivateKey();

	wxConfig::Get()->SetPath(wxT("/Options/PrivateKey"));
	wxConfig::Get()->Write(wxT("path"), privKey.getFile());
	wxConfig::Get()->Write(wxT("validity"), privKey.getValidity());

	/* Certificate */
	LocalCertificate &cert = KeyCtrl::instance()->getLocalCertificate();

	wxConfig::Get()->SetPath(wxT("/Options/LocalCertificate"));
	wxConfig::Get()->Write(wxT("path"), cert.getFile());

	/* Reset to original path */
	wxConfig::Get()->SetPath(oldPath);
}

void
ModSfsMainPanelImpl::privKeyParamsUpdate(const wxString &path, int validity)
{
	KeyCtrl *keyCtrl = KeyCtrl::instance();
	PrivKey &privKey = keyCtrl->getPrivateKey();
	wxString oldPath = privKey.getFile();

	privKey.setFile(path);
	privKey.setValidity(validity);

	if (!path.IsEmpty() && !privKey.canLoad()) {
		anMessageBox(
		    _("Cannot load the private key!\n"
		      "The file you specified does not exist."),
		    _("Load private key"), wxOK | wxICON_ERROR, this);
		privKey.setFile(oldPath);
	} else {
		/* The path changed. Forget about the old key. */
		privKey.unload();
	}

	/* Update view */
	keyPicker->setFileName(privKey.getFile());

	privKeyValiditySpinCtrl->Enable(validity != 0);
	privKeyValidityText->Enable(validity != 0);

	privKeyValidityChoice->SetSelection(validity ? 1 : 0);
	if (validity)
		privKeyValiditySpinCtrl->SetValue(validity);

	/* Inform any listener about change of key-configuration */
	wxCommandEvent event(anEVT_LOAD_KEY);
	event.SetInt(0); /* 0 := private key */
	wxPostEvent(AnEvents::getInstance(), event);
}

bool
ModSfsMainPanelImpl::certificateParamsUpdate(const wxString &path)
{
	wxDateTime	  time;
	LocalCertificate &cert = KeyCtrl::instance()->getLocalCertificate();
	wxString oldPath = cert.getFile();

	bool changed = oldPath != path;
	cert.setFile(path);

	if (path.IsEmpty()) {
		/* Reset certificate */
		cert.unload();
		changed = false;
	} else if (!cert.canLoad()) {
		anMessageBox(
		    _("Cannot load the certificate!\n"
		      "The file you specified does not exist."),
		    _("Load private key"), wxOK | wxICON_ERROR, this);
		/* revert to the old certificate */
		changed = false;
		cert.setFile(oldPath);
	} else if (!cert.load()) {
		anMessageBox(wxString::Format(
		    _("Failed to load certificate from\n%ls."),
		    cert.getFile().c_str()),
		    _("Load certificate"), wxOK | wxICON_ERROR, this);
		/* revert to the old certificate */
		changed = false;
		cert.setFile(oldPath);
		cert.load();
	}

	/* Update view */
	certificatePicker->setFileName(cert.getFile());
	certFingerprintText->SetLabel(cert.getFingerprint());
	time = cert.getValidity();
	if (!cert.isLoaded()) {
		certValidityText->SetLabel(wxEmptyString);
	} else if (time.IsValid()) {
		certValidityText->SetLabel(time.FormatDate());
	} else {
		certValidityText->SetLabel(_("[invalid date]"));
	}
	certCountryText->SetLabel(cert.getCountry());
	certStateText->SetLabel(cert.getState());
	certLocalityText->SetLabel(cert.getLocality());
	certOrgaText->SetLabel(cert.getOrganization());
	certOrgaUnitText->SetLabel(cert.getOrganizationalUnit());
	certCnText->SetLabel((cert.getCommonName()));
	certEmailText->SetLabel(cert.getEmailAddress());

	/* Inform any listener about change of configuration */
	wxCommandEvent event(anEVT_LOAD_KEY);
	event.SetInt(1); /* 1 := certificate */

	wxPostEvent(AnEvents::getInstance(), event);

	return (changed);
}

void
ModSfsMainPanelImpl::onSfsBrowserShow(wxCommandEvent& event)
{
	/* select Browser tab of ModSfs */
	note_MainSfs->SetSelection(1);

	sfsCtrl_->setEntryFilter(SfsCtrl::FILTER_UPGRADED, true);
	SfsMainDirTraversalCheckbox->SetValue(true);
	SfsMainDirViewChoice->SetSelection(4);
	event.Skip();
}

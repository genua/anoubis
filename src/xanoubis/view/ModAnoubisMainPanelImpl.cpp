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

#include <sys/types.h>

#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis_sfs.h>
#endif

#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#include <dev/anoubis_alf.h>
#include <dev/anoubis_sfs.h>
#endif

#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/tooltip.h>

#include "main.h"
#include "AnEvents.h"
#include "ApnVersion.h"
#include "DlgProfileSelection.h"
#include "KeyCtrl.h"
#include "ModAnoubis.h"
#include "ModAnoubisMainPanelImpl.h"
#include "Notification.h"
#include "LogNotify.h"
#include "AlertNotify.h"
#include "EscalationNotify.h"
#include "NotifyAnswer.h"
#include "VersionCtrl.h"
#include "VersionListCtrl.h"

#include "apn.h"


#define ZERO_SECONDS	0
#define TEN_SECONDS	10

#define OPTIONS_TAB	3

#define SHOWSLOT(slotNo,field,value) \
	do { \
		slotLabelText##slotNo->Show(); \
		slotLabelText##slotNo->SetLabel(field); \
		slotValueText##slotNo->Show(); \
		slotValueText##slotNo->SetLabel(value); \
	} while (0)

#define HIDESLOT(slotNo) \
	do { \
		slotLabelText##slotNo->Hide(); \
		slotValueText##slotNo->Hide(); \
	} while (0)

ModAnoubisMainPanelImpl::ModAnoubisMainPanelImpl(wxWindow* parent,
    wxWindowID id) : ModAnoubisMainPanelBase(parent, id), Observer(NULL)
{
	AnEvents *anEvents;
	NotificationCtrl	*notifyCtrl;

	//list_ = NOTIFY_LIST_NOTANSWERED;
	currentNotify_ = NULL;
	userOptions_ = wxGetApp().getUserOptions();
	anEvents = AnEvents::getInstance();

	notifyCtrl = NotificationCtrl::instance();
	listPerspective_ = notifyCtrl->getPerspective(
		NotificationCtrl::LIST_NOTANSWERED);
	if (listPerspective_ != NULL)
		it_ = listPerspective_->begin();
	else
		it_ = 0;

	/* read and restore Escalations Settings */
	readOptions();

	/* Initialization of profiles */
	profileTabInit();
	profileTabUpdate();

	/* Initialize list of versions */
	versionListUpdate();

	anEvents->Connect(anEVT_ESCALATIONS_SHOW,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnEscalationsShow),
	    NULL, this);
	anEvents->Connect(anEVT_ANOUBISOPTIONS_SHOW,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnAnoubisOptionShow),
	    NULL, this);
	anEvents->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnLoadRuleSet),
	    NULL, this);
	anEvents->Connect(anEVT_ESCALATION_RULE_ERROR, wxCommandEventHandler(
	    ModAnoubisMainPanelImpl::OnEscalationRuleError), NULL, this);
	anEvents->Connect(anEVT_ANOUBISOPTIONS_UPDATE, wxCommandEventHandler(
	    ModAnoubisMainPanelImpl::OnAnoubisOptionsUpdate), NULL, this);
	tb_MainAnoubisNotify->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
	    wxNotebookEventHandler(
	       ModAnoubisMainPanelImpl::OnNotebookTabChanged),
	    NULL, this);
	pathcomp_.clear();
	pathKeep_ = minPathKeep_ = 0;
	addSubject(listPerspective_);
	update();
}

ModAnoubisMainPanelImpl::~ModAnoubisMainPanelImpl(void)
{
	AnEvents *anEvents;

	anEvents = AnEvents::getInstance();

	if (listPerspective_ != NULL) {
		removeSubject(listPerspective_);
	}

	/* write Escalations Settings */
	userOptions_->Write(wxT("/Options/SendEscalations"),
	    cb_SendEscalations->IsChecked());
	userOptions_->Write(wxT("/Options/NoEscalationsTimeout"),
	    cb_NoEscalationTimeout->IsChecked());
	userOptions_->Write(wxT("/Options/EscalationTimeout"),
	    m_spinEscalationNotifyTimeout->GetValue());

	/* write Alert Settings */
	userOptions_->Write(wxT("/Options/SendAlerts"),
	    cb_SendAlerts->IsChecked());
	userOptions_->Write(wxT("/Options/NoAlertTimeout"),
	    cb_NoAlertTimeout->IsChecked());
	userOptions_->Write(wxT("/Options/AlertTimeout"),
	    m_spinAlertNotifyTimeout->GetValue());
	userOptions_->Write(wxT("/Options/AutoChecksumCheck"),
	    controlAutoCheck->IsChecked());
	userOptions_->Write(wxT("/Options/AutoConnect"),
	    autoConnectBox->IsChecked());
	if (loadedProfile != wxEmptyString) {
		userOptions_->Write(wxT("/Options/LoadedProfile"),
		    loadedProfile);
	}

	/* write Autostart Settings */
	userOptions_->Write(wxT("/Options/Autostart"),
	    cb_DoAutostart->IsChecked());

	/* write Notification of Upgrade Settings */
	userOptions_->Write(wxT("/Options/ShowUpgradeMessage"),
	    cb_ShowUpgradeMsg->GetValue());

	/* write ToolTip Settings */
	userOptions_->Write(wxT("/Options/EnableToolTips"),
	    toolTipCheckBox->GetValue());
	userOptions_->Write(wxT("/Options/ToolTipTimeout"),
	    toolTipSpinCtrl->GetValue());

	anEvents->Disconnect(anEVT_ESCALATIONS_SHOW,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnEscalationsShow),
	    NULL, this);
	anEvents->Disconnect(anEVT_ANOUBISOPTIONS_SHOW,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnAnoubisOptionShow),
	    NULL, this);
	anEvents->Disconnect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnLoadRuleSet),
	    NULL, this);
	anEvents->Disconnect(anEVT_ESCALATION_RULE_ERROR, wxCommandEventHandler(
	    ModAnoubisMainPanelImpl::OnEscalationRuleError), NULL, this);
	anEvents->Disconnect(anEVT_ANOUBISOPTIONS_UPDATE, wxCommandEventHandler(
	    ModAnoubisMainPanelImpl::OnAnoubisOptionsUpdate), NULL, this);
}

void
ModAnoubisMainPanelImpl::readOptions(void)
{
	bool AutoChecksum = false;
	bool AutoConnect = false;
	bool DoAutostart = false;
	bool EnableToolTips = true;
	bool NoAlertTimeout = false;
	bool NoEscalationTimeout = true;
	bool SendAlert = false;
	bool SendEscalation = true;
	bool ShowUpgradeMessage = true;

	int AlertTimeout = 10;
	int EscalationTimeout = 0;
	int ToolTipTimeout = 1;

	/* read the stored Option Settings */
	userOptions_->Read(wxT("/Options/SendAlerts"), &SendAlert);
	userOptions_->Read(wxT("/Options/SendEscalations"), &SendEscalation);
	userOptions_->Read(wxT("/Options/ShowUpgradeMessage"),
	    &ShowUpgradeMessage);

	userOptions_->Read(wxT("/Options/NoEscalationsTimeout"),
	    &NoEscalationTimeout);
	userOptions_->Read(wxT("/Options/EscalationTimeout"),
	    &EscalationTimeout);

	userOptions_->Read(wxT("/Options/NoAlertTimeout"), &NoAlertTimeout);
	userOptions_->Read(wxT("/Options/AlertTimeout"), &AlertTimeout);

	userOptions_->Read(wxT("/Options/Autostart"), &DoAutostart);
	wxGetApp().autoStart(DoAutostart);

	userOptions_->Read(wxT("/Options/AutoChecksumCheck"), &AutoChecksum);
	userOptions_->Read(wxT("/Options/AutoConnect"), &AutoConnect);

	userOptions_->Read(wxT("/Options/EnableToolTips"), &EnableToolTips);
	userOptions_->Read(wxT("/Options/ToolTipTimeout"), &ToolTipTimeout);

	/* restore the stored Notifications Options */
	cb_SendEscalations->SetValue(SendEscalation);
	cb_ShowUpgradeMsg->SetValue(ShowUpgradeMessage);
	cb_NoEscalationTimeout->SetValue(NoEscalationTimeout);
	m_spinEscalationNotifyTimeout->SetValue(EscalationTimeout);

	cb_SendAlerts->SetValue(SendAlert);
	cb_NoAlertTimeout->SetValue(NoAlertTimeout);
	m_spinAlertNotifyTimeout->SetValue(AlertTimeout);

	cb_DoAutostart->SetValue(DoAutostart);

	controlAutoCheck->SetValue(AutoChecksum);
	wxCommandEvent showEvent(anEVT_SEND_AUTO_CHECK);
	showEvent.SetInt(AutoChecksum);
	wxPostEvent(AnEvents::getInstance(), showEvent);

	autoConnectBox->SetValue(AutoConnect);
	wxGetApp().connectCommunicator(AutoConnect);

	toolTipCheckBox->SetValue(EnableToolTips);
	toolTipSpinCtrl->SetValue(ToolTipTimeout);
	toolTipParamsUpdate();

	/* set widgets visability and send options event */
	setOptionsWidgetsVisability();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::setOptionsWidgetsVisability(void)
{
	/* synchronize Escalation widgets */
	if (!cb_SendEscalations->IsChecked()) {
		m_spinEscalationNotifyTimeout->Disable();
		cb_NoEscalationTimeout->Disable();
		tx_EscalationNotifyTimeoutLabel->Disable();
	} else {
		if (cb_NoEscalationTimeout->IsChecked()) {
			cb_NoEscalationTimeout->Enable();
			m_spinEscalationNotifyTimeout->Disable();
			tx_EscalationNotifyTimeoutLabel->Disable();
		} else {
			cb_NoEscalationTimeout->Enable();
			m_spinEscalationNotifyTimeout->Enable();
			tx_EscalationNotifyTimeoutLabel->Enable();
		}
	}

	/* synchronize Alert widgets */
	if (!cb_SendAlerts->IsChecked()) {
		m_spinAlertNotifyTimeout->Disable();
		cb_NoAlertTimeout->Disable();
		tx_AlertNotifyTimeoutLabel->Disable();
	} else {
		if (cb_NoAlertTimeout->IsChecked()) {
			cb_NoAlertTimeout->Enable();
			m_spinAlertNotifyTimeout->Disable();
			tx_AlertNotifyTimeoutLabel->Disable();
		} else {
			cb_NoAlertTimeout->Enable();
			m_spinAlertNotifyTimeout->Enable();
			tx_AlertNotifyTimeoutLabel->Enable();
		}
	}
}

void
ModAnoubisMainPanelImpl::profileTabInit(void)
{
	wxConfig	*userOptions = wxGetApp().getUserOptions();
	profileList->InsertColumn(0, wxT(""),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
	profileList->InsertColumn(1, _("Profile"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);

	selectedProfile = wxEmptyString;
	loadedProfile = wxEmptyString;
	userOptions->Read(wxT("/Options/LoadedProfile"), &loadedProfile);
	fillProfileList();

	/* Adjust width of profile-column */
	int width = profileList->GetClientSize().GetWidth();
	width -= profileList->GetColumnWidth(0);
	profileList->SetColumnWidth(1, width);

}

void
ModAnoubisMainPanelImpl::profileTabUpdate(void)
{
	PolicyCtrl *policyCtrl = PolicyCtrl::getInstance();
	bool haveSelected = (selectedProfile != wxEmptyString);
	bool haveLoaded = (loadedProfile != wxEmptyString);

	if (haveSelected)
		selectedProfileText->SetLabel(selectedProfile);
	else
		selectedProfileText->SetLabel(_("none"));

	if (haveLoaded)
		loadedProfileText->SetLabel(loadedProfile);
	else
		loadedProfileText->SetLabel(_("none"));

	profileDeleteButton->Enable(
	    policyCtrl->isProfileWritable(selectedProfile));
	profileLoadButton->Enable(haveSelected);
	profileActivateButton->Enable(haveSelected);
}

void
ModAnoubisMainPanelImpl::fillProfileList(void)
{
	PolicyCtrl *policyCtrl = PolicyCtrl::getInstance();
	wxArrayString profiles = policyCtrl->getProfileList();

	profileList->DeleteAllItems();

	for (unsigned int i = 0; i < profiles.GetCount(); i++) {
		profileList->InsertItem(i, wxEmptyString);

		if (!policyCtrl->isProfileWritable(profiles[i]))
			profileList->SetItem(i, 0, _("read-only"));

		profileList->SetItem(i, 1, profiles[i]);
	}
}

void
ModAnoubisMainPanelImpl::OnLoadRuleSet(wxCommandEvent &event)
{
	versionListUpdateFromSelection();
	event.Skip();
}

void
ModAnoubisMainPanelImpl::OnProfileDeleteClicked(wxCommandEvent &)
{
	PolicyCtrl *policyCtrl = PolicyCtrl::getInstance();

	if (policyCtrl->removeProfile(selectedProfile)) {
		selectedProfile = wxEmptyString;
		fillProfileList();
		profileTabUpdate();
	} else {
		wxMessageBox(
		    wxString::Format(_("Failed to remove \"%ls\"."),
		       selectedProfile.c_str()),
		    _("Remove profile"), wxOK | wxICON_ERROR, this);
	}
}

void
ModAnoubisMainPanelImpl::OnProfileLoadClicked(wxCommandEvent &)
{
	PolicyCtrl *policyCtrl = PolicyCtrl::getInstance();

	if (policyCtrl->importFromProfile(selectedProfile)) {
		loadedProfile = selectedProfile;
		profileTabUpdate();
	} else {
		wxMessageBox(
		    wxString::Format(_("Failed to import from \"%ls\"."),
		       selectedProfile.c_str()),
		    _("Load profile"), wxOK | wxICON_ERROR, this);
	}
}

void
ModAnoubisMainPanelImpl::OnProfileSaveClicked(wxCommandEvent &)
{
	DlgProfileSelection dlg(loadedProfile, this);

	if (dlg.ShowModal() == wxID_OK) {
		PolicyCtrl *policyCtrl = PolicyCtrl::getInstance();
		wxString profile = dlg.getSelectedProfile();

		if (!policyCtrl->isProfileWritable(profile)) {
			wxMessageBox(
			    wxString::Format(
			       _("The profile \"%ls\" is not writable!"),
			       profile.c_str()),
			    _("Save profile"), wxOK | wxICON_ERROR, this);
			return;
		}

		if (policyCtrl->exportToProfile(profile)) {
			fillProfileList();
			profileTabUpdate();
		} else {
			wxMessageBox(
			    wxString::Format(_("Failed to export to \"%ls\"."),
			       profile.c_str()),
			    _("Save profile"), wxOK | wxICON_ERROR, this);
		}
	}
}

void
ModAnoubisMainPanelImpl::OnProfileActivateClicked(wxCommandEvent &)
{
	PolicyCtrl *policyCtrl = PolicyCtrl::getInstance();
	PolicyCtrl::PolicyResult polRes;
	long userId;

	if (!policyCtrl->importFromProfile(selectedProfile)) {
		wxMessageBox(
		    wxString::Format(_("Failed to import from \"%ls\"."),
		       selectedProfile.c_str()),
		    _("Activate profile"), wxOK | wxICON_ERROR, this);
		return;
	}

	userId = policyCtrl->getUserId();
	if (userId == -1) {
		wxMessageBox(
		    _("Could not obtain user-policy."),
		    _("Activate profile"), wxOK | wxICON_ERROR, this);
		return;
	}

	polRes = policyCtrl->sendToDaemon(userId);
	if (polRes == PolicyCtrl::RESULT_POL_ERR) {
		wxMessageBox(
		    _("Could not activate user-policy.\n"
		      "No connection to anoubisd."),
		    _("Activate profile"), wxOK | wxICON_ERROR, this);
		return;
	} else if (polRes == PolicyCtrl::RESULT_POL_WRONG_PASS) {
		wxMessageBox(
		      _("The entered password is incorrect."),
		    _("Activate profile"), wxOK | wxICON_ERROR, this);
		return;
	}

	loadedProfile = selectedProfile;
	profileTabUpdate();
}

void
ModAnoubisMainPanelImpl::OnProfileSelectionChanged(wxListEvent &event)
{
	/* Extract name of selected profile from list */
	int index = event.GetIndex();
	wxListItem listItem;

	listItem.SetId(index);
	listItem.SetColumn(1);
	profileList->GetItem(listItem);

	selectedProfile = listItem.GetText();
	profileTabUpdate();
}

void
ModAnoubisMainPanelImpl::versionListUpdate(void)
{
	VersionCtrl	*versionCtrl;
	PolicyCtrl	*policyCtrl;
	wxString	 profile;

	versionCtrl = VersionCtrl::getInstance();
	policyCtrl = PolicyCtrl::getInstance();
	profile = versionCtrl->getVersionProfile();

	VersionProfileChoice->Clear();
	VersionProfileChoice->Append(policyCtrl->getProfileList());
	if (profile.IsEmpty()) {
		VersionProfileChoice->SetSelection(0);
	} else {
		VersionProfileChoice->SetStringSelection(profile);
	}

	versionListUpdateFromSelection();
}

void
ModAnoubisMainPanelImpl::versionListUpdateFromSelection(void)
{
	VersionCtrl	*versionCtrl;
	PolicyCtrl	*policyCtrl;
	wxString	profile;

	if (VersionActivePolicyRadioButton->GetValue())
		profile = wxT("active");
	else
		profile = VersionProfileChoice->GetStringSelection();

	versionCtrl = VersionCtrl::getInstance();
	policyCtrl = PolicyCtrl::getInstance();

	if (!versionCtrl->isInitialized()) {
		wxMessageBox(_("Repository not initialized."),
		    _("Update versions"), wxOK | wxICON_ERROR, this);
		return;
	}

	if (!versionCtrl->isPrepared()) {
		wxMessageBox(_("Failed to prepare versioning system."),
		    _("Update versions"), wxOK | wxICON_ERROR, this);
		return;
	}

	if (profile.IsEmpty())
		return;
	if (!versionCtrl->fetchVersionList(profile)) {
		wxMessageBox(_("Failed to fetch versioning information."),
		    _("Update versions"), wxOK | wxICON_ERROR, this);
		return;
	}

	versionListCtrl->update();
}

void
ModAnoubisMainPanelImpl::displayMessage(void)
{
	if (!currentNotify_)
		return;

	SHOWSLOT(1, _("Time:"), currentNotify_->getTime());
	SHOWSLOT(2, _("Module:"), currentNotify_->getModule());
	SHOWSLOT(3, _("Program:"), currentNotify_->getOrigin());
	SHOWSLOT(4, _("On behalf of:"), currentNotify_->getCtxOrigin());
	if (currentNotify_->getModule() == wxT("ALF")) {
		SHOWSLOT(5, _("Traffic:"), currentNotify_->getPath());
	} else {
		SHOWSLOT(5, _("Path:"), currentNotify_->getPath());
	}
	SHOWSLOT(6, _("Operation:"), currentNotify_->getOperation());
}

void
ModAnoubisMainPanelImpl::sendNotifierOptionsEvents(WXTYPE type, bool show,
    long timeout)
{
	wxCommandEvent showEvent(type);

	showEvent.SetInt(show);
	showEvent.SetExtraLong(timeout);

	wxPostEvent(AnEvents::getInstance(), showEvent);
}

void
ModAnoubisMainPanelImpl::sendNotifierOptions(void)
{
	bool show;
	long timeout;

	/* handle and pass Escalation Options */
	show = cb_SendEscalations->IsChecked();
	if (show && cb_NoEscalationTimeout->IsChecked()) {
		timeout = 0;
	} else {
		timeout = m_spinEscalationNotifyTimeout->GetValue();
	}
	sendNotifierOptionsEvents(anEVT_ESCALATIONNOTIFY_OPTIONS, show,
	    timeout);

	/* handle and pass Alert Options */
	 show = cb_SendAlerts->IsChecked();
	 if (show && cb_NoAlertTimeout->IsChecked()) {
		 timeout = 0;
	 } else {
		 timeout = m_spinAlertNotifyTimeout->GetValue();
	 }
	 sendNotifierOptionsEvents(anEVT_ALERTNOTIFY_OPTIONS, show,
			 timeout);
}

void
ModAnoubisMainPanelImpl::update(Subject *)
{
	update();
}

void
ModAnoubisMainPanelImpl::updateDelete(Subject *)
{
}

void
ModAnoubisMainPanelImpl::update(void)
{
	wxString		 s;
	size_t			 maxElementNo;
	size_t			 elementNo;
	EscalationNotify	*eNotify = NULL;

	if (currentNotify_) {
		/*
		 * A DaemonAnswerNotify can remove an Escalation from
		 * the list. Reset currentNotify_ in this case.
		 */
		eNotify = dynamic_cast<EscalationNotify *>(currentNotify_);
		if (eNotify && eNotify->isAnswered()
		    //&& list_ == NOTIFY_LIST_NOTANSWERED
		    )
			currentNotify_ = NULL;
	}

	if (listPerspective_) {
		maxElementNo = listPerspective_->getSize();
	} else {
		maxElementNo = 0;
	}
	elementNo = 0;

	if (maxElementNo > 0) {
		if (currentNotify_ == NULL) {
			it_ = listPerspective_->begin();
			NotificationCtrl *notifyCtrl;
			notifyCtrl = NotificationCtrl::instance();
			currentNotify_ = notifyCtrl->getNotification(*it_);
		}
		elementNo = it_ - listPerspective_->begin();
	} else {
		currentNotify_ = NULL;
	}

	s.Printf(wxT("%d"), maxElementNo);
	tx_maxNumber->SetLabel(s);
	if (maxElementNo > 0)
		s.Printf(wxT("%d"), elementNo + 1);
	tx_currNumber->SetLabel(s);

	if ((maxElementNo > 0) && (elementNo == 0)) {
		bt_first->Disable();
		bt_previous->Disable();
		bt_next->Enable();
		bt_last->Enable();
	} else if (maxElementNo == 0) {
		bt_first->Disable();
		bt_previous->Disable();
		bt_next->Disable();
		bt_last->Disable();
	} else if ((maxElementNo == elementNo + 1) && (elementNo > 0)) {
		bt_first->Enable();
		bt_previous->Enable();
		bt_next->Disable();
		bt_last->Disable();
	} else {
		bt_first->Enable();
		bt_previous->Enable();
		bt_next->Enable();
		bt_last->Enable();
	}

	/*
	 * Only update the escalation window if the escalation actually
	 * changed.
	 */
	if (currentNotify_ == savedNotify_)
		return;
	savedNotify_ = currentNotify_;
	if (currentNotify_ != NULL) {
		displayMessage();
	} else {
		HIDESLOT(1);
		HIDESLOT(2);
		HIDESLOT(3);
		HIDESLOT(4);
		HIDESLOT(5);
		HIDESLOT(6);
	}

	if ((currentNotify_ != NULL) && IS_ESCALATIONOBJ(currentNotify_)) {
		eNotify = dynamic_cast<EscalationNotify *>(currentNotify_);
		if (!eNotify->isAnswered()) {
			wxString	module = eNotify->getModule();
			pn_Escalation->Show();
			pn_EscalationAlf->Hide();
			pn_EscalationSb->Hide();
			pn_EscalationSfs->Hide();
			initPathLabel();
			if (module == wxT("ALF")) {
				pn_EscalationAlf->Show();
			} else if (module == wxT("SANDBOX")) {
				pn_EscalationSb->Show();
			} else if (module == wxT("SFS")) {
				pn_EscalationSfs->Show();
			}
			editOptionsEnable();
			pn_EscalationOptions->Layout();
			pn_Escalation->Layout();
			tb_MainAnoubisNotify->Layout();
			tb_MainAnoubisNotification->Layout();
			tx_answerValue->Hide();
		} else {
			pn_Escalation->Hide();
			tx_answerValue->SetLabel(
			    eNotify->getAnswer()->getAnswer());
			tx_answerValue->Show();
		}
	} else {
		pn_Escalation->Hide();
		tx_answerValue->Hide();
	}
	Layout();
	Refresh();
}

void
ModAnoubisMainPanelImpl::OnTypeChoosen(wxCommandEvent& event)
{
	NotificationCtrl::ListPerspectives	 perspectiveType;
	NotificationCtrl			*notifyCtrl;

	notifyCtrl = NotificationCtrl::instance();
	currentNotify_ = NULL;

	/* keep this in sync with ch_type elements */
	switch (event.GetSelection()) {
	case 0:
		perspectiveType = NotificationCtrl::LIST_NOTANSWERED;
		break;
	case 1:
		perspectiveType = NotificationCtrl::LIST_MESSAGES;
		break;
	case 2:
		perspectiveType = NotificationCtrl::LIST_ANSWERED;
		break;
	case 3:
		perspectiveType = NotificationCtrl::LIST_ALL;
		break;
	default:
		perspectiveType = NotificationCtrl::LIST_NONE;
		break;
	}

	removeSubject(listPerspective_);

	listPerspective_ = notifyCtrl->getPerspective(perspectiveType);
	if (listPerspective_ != NULL) {
		it_ = listPerspective_->begin();
	} else {
		it_ = 0;
	}

	addSubject(listPerspective_);
	update();
}

void
ModAnoubisMainPanelImpl::OnFirstBtnClick(wxCommandEvent&)
{
	NotificationCtrl	*notifyCtlr;

	notifyCtlr = NotificationCtrl::instance();
	if (listPerspective_ != NULL) {
		it_ = listPerspective_->begin();
		currentNotify_ = notifyCtlr->getNotification(*it_);
	} else {
		it_ = 0;
		currentNotify_ = NULL;
	}

	update();
}

void
ModAnoubisMainPanelImpl::OnPreviousBtnClick(wxCommandEvent&)
{
	NotificationCtrl	*notifyCtlr;

	notifyCtlr = NotificationCtrl::instance();
	if (it_ != 0) {
		it_--;
		currentNotify_ = notifyCtlr->getNotification(*it_);
	} else {
		currentNotify_ = NULL;
	}

	update();
}

void
ModAnoubisMainPanelImpl::OnNextBtnClick(wxCommandEvent&)
{
	NotificationCtrl	*notifyCtlr;

	notifyCtlr = NotificationCtrl::instance();
	if (it_ != 0) {
		it_++;
		currentNotify_ = notifyCtlr->getNotification(*it_);
	} else {
		currentNotify_ = NULL;
	}

	update();
}

void
ModAnoubisMainPanelImpl::OnLastBtnClick(wxCommandEvent&)
{
	NotificationCtrl	*notifyCtlr;

	notifyCtlr = NotificationCtrl::instance();

	if (listPerspective_ != NULL) {
		/* get last */
		it_ = listPerspective_->end();
		it_--;
		currentNotify_ = notifyCtlr->getNotification(*it_);
	} else {
		it_ = 0;
		currentNotify_ = NULL;
	}

	update();
}

void
ModAnoubisMainPanelImpl::setAlfOptions(EscalationNotify *current,
    NotifyAnswer *answer)
{
	unsigned long	flags = 0;
	int		proto;

	proto = current->getProtocolNo();
	if (proto == IPPROTO_TCP) {
		flags = 0;
	} else  if (proto == IPPROTO_UDP) {
		flags = ALF_EV_ALLDIR;
	} else {
		return;
	}
	if (rb_EscalationAlf1->GetValue()) {
		/* Nothing */
	} else if (rb_EscalationAlf2->GetValue()) {
		flags |= ALF_EV_ALLPEER;
	} else if (rb_EscalationAlf3->GetValue()) {
		flags |= ALF_EV_ALLPORT;
	} else if (rb_EscalationAlf4->GetValue()) {
		flags |= (ALF_EV_ALLPORT|ALF_EV_ALLPEER);
	} else {
		flags = 0;
	}
	answer->setFlags(flags);
}

void
ModAnoubisMainPanelImpl::setSfsOptions(EscalationNotify *current,
    NotifyAnswer *answer)
{
	answer->setPrefix(tx_EscalationSfsPath->GetLabel());
	answer->setFlags(current->getSfsmatch());
}

void
ModAnoubisMainPanelImpl::setSbOptions(EscalationNotify *, NotifyAnswer *answer)
{
	unsigned long flags = 0;

	answer->setPrefix(tx_EscalationSbPath->GetLabel());
	if (ck_EscalationSbRead->GetValue()) {
		flags |= APN_SBA_READ;
	}
	if (ck_EscalationSbWrite->GetValue()) {
		flags |= APN_SBA_WRITE;
	}
	if (ck_EscalationSbExec->GetValue()) {
		flags |= APN_SBA_EXEC;
	}
	answer->setFlags(flags);
}

void
ModAnoubisMainPanelImpl::answer(bool permission)
{
	ModAnoubis		*module;
	NotifyAnswer		*answer;
	EscalationNotify	*current;
	wxString		 escalationType;

	current = dynamic_cast<EscalationNotify *>(currentNotify_);
	if (current) {
		module = (ModAnoubis *)(wxGetApp().getModule(ANOUBIS));

		if (rb_EscalationOnce->GetValue()) {
			answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE,
			    permission);
		} else if (rb_EscalationProcess->GetValue()) {
			answer = new NotifyAnswer(NOTIFY_ANSWER_PROCEND,
			    permission);
		} else if (rb_EscalationTime->GetValue()) {
			enum timeUnit	unit;
			unit = (enum timeUnit)
			    ch_EscalationTimeUnit->GetCurrentSelection();
			answer = new NotifyAnswer(NOTIFY_ANSWER_TIME,
			    permission, spin_EscalationTime->GetValue(), unit);
		} else if (rb_EscalationAlways->GetValue()) {
			answer = new NotifyAnswer(NOTIFY_ANSWER_FOREVER,
			    permission);
		} else {
			answer = new NotifyAnswer(NOTIFY_ANSWER_NONE,
			    permission);
		}
		answer->setEditor(ck_EscalationEditor->GetValue());
		if (current->allowOptions()) {
			escalationType = current->getModule();
			if (escalationType == wxT("ALF")) {
				setAlfOptions(current, answer);
			} else if (escalationType == wxT("SFS")) {
				setSfsOptions(current, answer);
			} else if (escalationType == wxT("SANDBOX")) {
				setSbOptions(current, answer);
			}
		}
		NotificationCtrl::instance()->answerEscalationNotify(
		    current, answer);
	}
	currentNotify_ = NULL;
	update();
}

void
ModAnoubisMainPanelImpl::OnAllowBtnClick(wxCommandEvent&)
{
	answer(true);
}

void
ModAnoubisMainPanelImpl::OnDenyBtnClick(wxCommandEvent&)
{
	answer(false);
}

void
ModAnoubisMainPanelImpl::OnEscalationOnceButton(wxCommandEvent&)
{
	setPathLabel();
	editOptionsEnable();
}

void
ModAnoubisMainPanelImpl::OnEscalationProcessButton(wxCommandEvent&)
{
	setPathLabel();
	editOptionsEnable();
}

void
ModAnoubisMainPanelImpl::OnEscalationTimeoutButton(wxCommandEvent&)
{
	setPathLabel();
	editOptionsEnable();
}

void
ModAnoubisMainPanelImpl::OnEscalationAlwaysButton(wxCommandEvent&)
{
	setPathLabel();
	editOptionsEnable();
}

void
ModAnoubisMainPanelImpl::OnEscalationDisable(wxCommandEvent&)
{
	setOptionsWidgetsVisability();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnAlertDisable(wxCommandEvent&)
{
	setOptionsWidgetsVisability();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnEscalationNoTimeout(wxCommandEvent&)
{
	setOptionsWidgetsVisability();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnAlertNoTimeout(wxCommandEvent&)
{
	setOptionsWidgetsVisability();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnEscalationTimeout(wxSpinEvent&)
{
	setOptionsWidgetsVisability();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnAlertTimeout(wxSpinEvent&)
{
	setOptionsWidgetsVisability();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnEscalationsShow(wxCommandEvent& event)
{
	/* select tab "Notifications" */
	tb_MainAnoubisNotify->ChangeSelection(0);

	if (event.GetString().Cmp(wxT("ESCALATIONS")) == 0) {
		/* select "current requests" */
		ch_type->SetSelection(0);
		/* XXX ch
		if (list_ != NOTIFY_LIST_NOTANSWERED) {
			currentNotify_ = NULL;
			list_ = NOTIFY_LIST_NOTANSWERED;
			}*/
	}

	if (event.GetString().Cmp(wxT("ALERTS")) == 0) {
		/* select "current messages" */
		ch_type->SetSelection(1);
		/*
		  XXX ch:
		if (list_ != NOTIFY_LIST_MESSAGE) {
			list_ = NOTIFY_LIST_MESSAGE;
			currentNotify_ = NULL;
		}
		*/
	}
	update();

	event.Skip();
}

void
ModAnoubisMainPanelImpl::OnAnoubisOptionShow(wxCommandEvent& event)
{
	/* select the "Options" tab */
	tb_MainAnoubisNotify->ChangeSelection(OPTIONS_TAB);

	event.Skip();
}

void
ModAnoubisMainPanelImpl::OnAutoCheck(wxCommandEvent& event)
{
	wxCommandEvent showEvent(anEVT_SEND_AUTO_CHECK);

	showEvent.SetInt(event.IsChecked());

	wxPostEvent(AnEvents::getInstance(), showEvent);
}

void ModAnoubisMainPanelImpl::OnDoAutostart(wxCommandEvent& event)
{
	wxGetApp().autoStart(event.IsChecked());
}

void
ModAnoubisMainPanelImpl::OnNotebookTabChanged(wxNotebookEvent&)
{
	if (tb_MainAnoubisNotify->GetCurrentPage() == tb_MainAnoubisVersions) {
		/*
		 * When the versioning-tab becomes active,
		 * reload list of versions.
		 */
		versionListUpdate();
	}
}

void
ModAnoubisMainPanelImpl::OnVersionListCtrlSelected(wxListEvent&)
{
	int idx = versionListCtrl->getFirstSelection();

	if (idx == -1)
		return;

	VersionCtrl *versionCtrl = VersionCtrl::getInstance();

	/* Selected version */
	ApnVersion *version = versionCtrl->getVersion(idx);

	/* Update comment-field with selected version*/
	VersionShowCommentTextCtrl->SetValue(version->getComment());
}

void
ModAnoubisMainPanelImpl::OnVersionRestoreButtonClick(wxCommandEvent&)
{
	VersionCtrl	*versionCtrl = VersionCtrl::getInstance();
	PolicyCtrl	*policyCtrl = PolicyCtrl::getInstance();
	bool		useActiveProfile;
	wxString	profile;
	int		idx = versionListCtrl->getFirstSelection();

	if (idx == -1)
		return;

	if (VersionActivePolicyRadioButton->GetValue()) {
		useActiveProfile = true;
		profile = wxT("active");
	} else {
		useActiveProfile = false;
		profile = VersionProfileChoice->GetStringSelection();
	}
	if (profile.IsEmpty()) {
		wxMessageBox(_("No profile selected"), _("Restore version"),
		    wxOK | wxICON_ERROR, this);
		return;
	}

	/* Fetch ruleset from versioning-system */
	struct apn_ruleset *apn_rs = versionCtrl->fetchRuleSet(idx, profile);
	if (apn_rs == 0) {
		wxString msg;

		if (useActiveProfile)
			msg = _("Failed to fetch the active policy.");
		else
			msg.Printf(_(
			    "Failed to fetch the policy\n"
			    "of the \"%ls\" profile"), profile.c_str());

		wxMessageBox(msg, _("Restore version"),
		    wxOK | wxICON_ERROR, this);
		return;
	}

	/* Import policy into application */
	PolicyRuleSet *rs = new PolicyRuleSet(1, geteuid(), apn_rs);
	if (useActiveProfile) {
		if (!policyCtrl->importPolicy(rs))
			wxMessageBox(_("Failed to import the active policy."),
			    _("Restore version"), wxOK | wxICON_ERROR, this);
	} else {
		if (!policyCtrl->importPolicy(rs, profile)) {
			wxString msg = wxString::Format(_(
			    "Failed to import the policy\n"
			    "of the \"%ls\" profile"), profile.c_str());

			wxMessageBox(msg, _("Restore version"),
			    wxOK | wxICON_ERROR, this);
		}
		if (!versionCtrl->createVersion(rs, profile,
		    _("automatically created by restore"), true)) {
			wxMessageBox(_("Old version imported but could not "
			    "create a new version"), _("Restore version"),
			    wxOK | wxICON_ERROR, this);
		}
		versionListUpdateFromSelection();
		delete rs;
	}
}

void
ModAnoubisMainPanelImpl::OnVersionExportButtonClick(wxCommandEvent&)
{
	int idx = versionListCtrl->getFirstSelection();

	if (idx == -1)
		return;

	/* The profile */
	wxString profile;
	if (VersionActivePolicyRadioButton->GetValue())
		profile = wxT("active");
	else
		profile = VersionProfileChoice->GetStringSelection();

	/* Ask for a destination file */
	wxFileDialog fileDlg(this,
	    _("Choose a file to write the policies to:"),
	    wxT(""), wxT(""), wxT("*.*"), wxFD_SAVE);
	if (fileDlg.ShowModal() != wxID_OK)
		return;

	VersionCtrl *versionCtrl = VersionCtrl::getInstance();
	if (!versionCtrl->exportVersion(idx, profile, fileDlg.GetPath())) {
		wxString msg = _("Failed to export a version!");
		wxString title = _("Export version");

		wxMessageBox(msg, title, wxOK | wxICON_ERROR, this);
	}
}

void
ModAnoubisMainPanelImpl::OnVersionDeleteButtonClick(wxCommandEvent&)
{
	VersionCtrl	*versionCtrl = VersionCtrl::getInstance();
	int		selection = -1;

	while (true) {
		selection = versionListCtrl->getNextSelection(selection);

		if (selection == -1) {
			/* No selection */
			break;
		}
		if (!versionCtrl->deleteVersion(selection)) {
			wxString msg = _("Failed to remove the selected"
			    " version!");
			wxString title = _("Remove version");

			wxMessageBox(msg, title, wxOK | wxICON_ERROR, this);
		}

	}
	versionListUpdate();
}

void
ModAnoubisMainPanelImpl::OnVersionShowButtonClick(wxCommandEvent&)
{
	/*
	 * XXX RD
	 *
	 * The button is still there but hidden.
	 * You are able to display a ruleset in rule-editor but...
	 *
	 * using wxGetApp().importPolicyRuleSet() will overwrite the set of
	 * rulesets managed by the GUI. And this is not we what to have.
	 *
	 * We have to think about a general show-option. In th meanwhile the
	 * Show-button is hidden.
	 */

	/*VersionCtrl *versionCtrl = VersionCtrl::getInstance();

	wxCommandEvent showEvent(anEVT_RULEEDITOR_SHOW);
	showEvent.SetInt(1);
	wxPostEvent(AnEvents::getInstance(), showEvent);

	struct apn_ruleset *rs = versionCtrl->fetchRuleSet(idx);
	wxGetApp().importPolicyRuleSet(1, rs);*/
}

void
ModAnoubisMainPanelImpl::OnVersionActivePolicyClicked(wxCommandEvent &)
{
	VersionProfileChoice->Enable(false);
	versionListUpdateFromSelection();
}

void
ModAnoubisMainPanelImpl::OnVersionProfilePolicyClicked(wxCommandEvent &)
{
	VersionProfileChoice->Enable(true);
	versionListUpdateFromSelection();
}

void
ModAnoubisMainPanelImpl::OnVersionProfileChoice(wxCommandEvent &)
{
	versionListUpdateFromSelection();
}


void
ModAnoubisMainPanelImpl::OnToolTipCheckBox(wxCommandEvent &)
{
	toolTipParamsUpdate();
}

void
ModAnoubisMainPanelImpl::OnToolTipSpinCtrl(wxSpinEvent &)
{
	toolTipParamsUpdate();
}

void
ModAnoubisMainPanelImpl::toolTipParamsUpdate(void)
{
	bool enable = toolTipCheckBox->GetValue();

	if (enable) {
		toolTipSpinCtrl->Enable();
	} else {
		toolTipSpinCtrl->Disable();
	}
	wxToolTip::Enable(enable);
	if (enable) {
		int delay = toolTipSpinCtrl->GetValue();
		wxToolTip::SetDelay(1000*delay);
	}
}

/*
 * Different visibility groups:
 * enableright: Enable the options panel on the right side.
 *     - The current duration radio button selection must not be once.
 *     - The ruleset must allow edits.
 *     - The type of escalation must support edit options.
 * enabledurationbuttons: Enable the duration radio buttons on the left
 *     - The ruleset must allow edits.
 * enableeditor: Enable the editor checkbox.
 *     - The current duration radio button selection must not be once.
 *     - The ruleset must allow edits.
 */
void
ModAnoubisMainPanelImpl::editOptionsEnable(void)
{
	EscalationNotify	*current = NULL;
	bool			 enableright = true;
	bool			 enableeditor = true;
	bool			 enabledurationbuttons = true;
	wxString		 module = currentNotify_->getModule();

	enableright = !rb_EscalationOnce->GetValue();
	enableeditor = !rb_EscalationOnce->GetValue();
	if (currentNotify_) {
		current = dynamic_cast<EscalationNotify *>(currentNotify_);
	}
	if (!current || !current->allowEdit()) {
		enabledurationbuttons = false;
		enableeditor = false;
		enableright = false;
	} else if (!current->allowOptions()) {
		enableright = false;
	}
	if (module == wxT("ALF")) {
		pn_EscalationAlf->Enable(enableright);
	} else if (module == wxT("SFS")) {
		pn_EscalationSfs->Enable(enableright);
	} else if (module == wxT("SANDBOX")) {
		pn_EscalationSb->Enable(enableright);
	}
	ck_EscalationEditor->Enable(enableeditor);
	if (!enabledurationbuttons && !rb_EscalationOnce->GetValue()) {
		rb_EscalationOnce->SetValue(true);
	}
	rb_EscalationAlways->Enable(enabledurationbuttons);
	rb_EscalationTime->Enable(enabledurationbuttons);
	rb_EscalationProcess->Enable(enabledurationbuttons);
	if (enabledurationbuttons && rb_EscalationTime->GetValue()) {
		spin_EscalationTime->Enable();
		ch_EscalationTimeUnit->Enable();
	} else {
		spin_EscalationTime->Disable();
		ch_EscalationTimeUnit->Disable();
	}
	pn_EscalationOptions->Layout();
	pn_Escalation->Layout();
	tb_MainAnoubisNotify->Layout();
	tb_MainAnoubisNotification->Layout();
	Layout();
}

void
ModAnoubisMainPanelImpl::initPathLabel(void)
{
	wxString		 path, module;
	EscalationNotify	*current;
	unsigned int		 i, slash, start;

	minPathKeep_ = 0;
	pathKeep_ = 0;
	pathcomp_.clear();

	module = currentNotify_->getModule();
	if (module != wxT("SFS") && module != wxT("SANDBOX"))
		return;
	current = dynamic_cast<EscalationNotify *>(currentNotify_);
	if (!current)
		return;
	path = current->rulePath();
	if (!path.IsEmpty()) {
		slash = 1;
		for (i=0; i<path.Len(); ++i) {
			if (path[i] == '/') {
				slash = 1;
			} else {
				if (slash) {
					slash = 0;
					minPathKeep_++;
				}
			}
		}
	}
	path = current->filePath();
	if (!path.IsEmpty()) {
		start = 0;
		slash = 1;
		for (i=0; i<path.Len(); ++i) {
			if (path[i] == '/') {
				if (!slash) {
					pathcomp_.push_back(
					    path(start, i-start));
				}
				slash = 1;
			} else {
				if (slash) {
					slash = 0;
					start = i;
				}
			}
		}
		if (!slash) {
			pathcomp_.push_back(path(start, i-start));
		}
	}
	if (pathcomp_.size()) {
		pathKeep_ = pathcomp_.size() - 1;
	} else {
		pathKeep_ = 0;
	}
	setPathLabel();
	if (currentNotify_->getModule() == wxT("SANDBOX")) {
		ck_EscalationSbRead->SetValue(current->isRead());
		ck_EscalationSbWrite->SetValue(current->isWrite());
		ck_EscalationSbExec->SetValue(current->isExec());
	}
}

void
ModAnoubisMainPanelImpl::OnEscalationSfsPathLeft(wxCommandEvent &)
{
	pathKeep_--;
	setPathLabel();
}

void
ModAnoubisMainPanelImpl::OnEscalationSfsPathRight(wxCommandEvent &)
{
	pathKeep_++;
	setPathLabel();
}

void
ModAnoubisMainPanelImpl::OnEscalationSbPathLeft(wxCommandEvent &)
{
	pathKeep_--;
	setPathLabel();
}

void
ModAnoubisMainPanelImpl::OnEscalationSbPathRight(wxCommandEvent &)
{
	pathKeep_++;
	setPathLabel();
}

void
ModAnoubisMainPanelImpl::OnEscalationRuleError(wxCommandEvent &)
{
	wxMessageBox(_("Failed to create a rule from escalation"),
	    _("Error"), wxICON_ERROR);
}

void
ModAnoubisMainPanelImpl::setPathLabel(void)
{
	unsigned int		i;
	wxString		path, module;
	bool			canleft = true, canright = true;

	module = currentNotify_->getModule();
	if (module != wxT("SFS") && module != wxT("SANDBOX"))
		return;
	if (pathKeep_ < minPathKeep_)
		pathKeep_ = minPathKeep_;
	if (pathKeep_ > pathcomp_.size())
		pathKeep_ = pathcomp_.size();
	if (pathKeep_ == 0) {
		path = wxT("/");
	} else {
		path = wxT("");
		for (i=0; i<pathKeep_; ++i) {
			path += wxT("/");
			path += pathcomp_[i];
		}
	}
	if (pathKeep_ <= minPathKeep_)
		canleft = false;
	if (pathKeep_ >= pathcomp_.size())
		canright = false;
	if (module == wxT("SFS")) {
		tx_EscalationSfsPath->SetLabel(path);
		bt_EscalationSfsPathLeft->Enable(canleft);
		bt_EscalationSfsPathRight->Enable(canright);
		pn_EscalationSfs->Layout();
	} else {
		tx_EscalationSbPath->SetLabel(path);
		bt_EscalationSbPathLeft->Enable(canleft);
		bt_EscalationSbPathRight->Enable(canright);
		pn_EscalationSb->Layout();
	}
}

void
ModAnoubisMainPanelImpl::OnAnoubisOptionsUpdate(wxCommandEvent& event)
{
	bool showUpgradeMessage = true;
	userOptions_->Read(wxT
	    ("/Options/ShowUpgradeMessage"),&showUpgradeMessage);
	cb_ShowUpgradeMsg->SetValue(showUpgradeMessage);
	event.Skip();
}

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

#include <anoubis_errno.h>

#include <wx/filedlg.h>
#include <wx/tooltip.h>

#include "AnListColumn.h"
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
#include "PolicyCtrl.h"
#include "ProfileListCtrl.h"

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

	currentNotify_ = NULL;
	savedNotify_ = NULL;
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

	/* Adjust column width (Bug #1321). */
	versionListCtrl->getColumn(1)->setWidth(205);
	versionListCtrl->getColumn(2)->setWidth(205);

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
	wxConfig::Get()->Write(wxT("/Options/SendEscalations"),
	    cb_SendEscalations->IsChecked());
	wxConfig::Get()->Write(wxT("/Options/NoEscalationsTimeout"),
	    cb_NoEscalationTimeout->IsChecked());
	wxConfig::Get()->Write(wxT("/Options/EscalationTimeout"),
	    m_spinEscalationNotifyTimeout->GetValue());

	/* write Alert Settings */
	wxConfig::Get()->Write(wxT("/Options/SendAlerts"),
	    cb_SendAlerts->IsChecked());
	wxConfig::Get()->Write(wxT("/Options/NoAlertTimeout"),
	    cb_NoAlertTimeout->IsChecked());
	wxConfig::Get()->Write(wxT("/Options/AlertTimeout"),
	    m_spinAlertNotifyTimeout->GetValue());
	wxConfig::Get()->Write(wxT("/Options/AutoConnect"),
	    autoConnectBox->IsChecked());
	if (loadedProfile != wxEmptyString) {
		wxConfig::Get()->Write(wxT("/Options/LoadedProfile"),
		    loadedProfile);
	}

	/* write Autostart Settings */
	wxConfig::Get()->Write(wxT("/Options/Autostart"),
	    cb_DoAutostart->IsChecked());

	/* write Notification of Upgrade Settings */
	wxConfig::Get()->Write(wxT("/Options/ShowUpgradeMessage"),
	    cb_ShowUpgradeMsg->GetValue());
	wxConfig::Get()->Write(wxT("/Options/ShowKernelUpgradeMessage"),
	    cb_ShowKernelMsg->GetValue());


	/* write ToolTip Settings */
	wxConfig::Get()->Write(wxT("/Options/EnableToolTips"),
	    toolTipCheckBox->GetValue());
	wxConfig::Get()->Write(wxT("/Options/ToolTipTimeout"),
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
	bool autoConnect = false;
	bool doAutostart = false;
	bool enableToolTips = true;
	bool noAlertTimeout = false;
	bool noEscalationTimeout = true;
	bool sendAlert = false;
	bool sendEscalation = true;
	bool showUpgradeMessage = true;
	bool showKernelUpgradeMessage = true;
	bool showKeyGenInfoMessage = true;

	int alertTimeout = 10;
	int escalationTimeout = 0;
	int toolTipTimeout = 1;

	/* read the stored Option Settings */
	wxConfig::Get()->Read(wxT("/Options/SendAlerts"), &sendAlert);
	wxConfig::Get()->Read(wxT("/Options/SendEscalations"), &sendEscalation);
	wxConfig::Get()->Read(wxT("/Options/ShowUpgradeMessage"),
	    &showUpgradeMessage);
	wxConfig::Get()->Read(wxT("/Options/ShowKernelUpgradeMessage"),
	    &showKernelUpgradeMessage);
	wxConfig::Get()->Read(wxT("/Options/ShowCertMessage"),
	    &showKeyGenInfoMessage);

	wxConfig::Get()->Read(wxT("/Options/NoEscalationsTimeout"),
	    &noEscalationTimeout);
	wxConfig::Get()->Read(wxT("/Options/EscalationTimeout"),
	    &escalationTimeout);

	wxConfig::Get()->Read(wxT("/Options/NoAlertTimeout"), &noAlertTimeout);
	wxConfig::Get()->Read(wxT("/Options/AlertTimeout"), &alertTimeout);

	wxConfig::Get()->Read(wxT("/Options/Autostart"), &doAutostart);
	wxGetApp().autoStart(doAutostart);

	wxConfig::Get()->Read(wxT("/Options/AutoConnect"), &autoConnect);

	wxConfig::Get()->Read(wxT("/Options/EnableToolTips"), &enableToolTips);
	wxConfig::Get()->Read(wxT("/Options/ToolTipTimeout"), &toolTipTimeout);

	/* restore the stored Notifications Options */
	cb_SendEscalations->SetValue(sendEscalation);
	cb_ShowUpgradeMsg->SetValue(showUpgradeMessage);
	cb_ShowKernelMsg->SetValue(showKernelUpgradeMessage);
	cb_ShowKeyGenInfoMsg->SetValue(showKeyGenInfoMessage);

	cb_NoEscalationTimeout->SetValue(noEscalationTimeout);
	m_spinEscalationNotifyTimeout->SetValue(escalationTimeout);

	cb_SendAlerts->SetValue(sendAlert);
	cb_NoAlertTimeout->SetValue(noAlertTimeout);
	m_spinAlertNotifyTimeout->SetValue(alertTimeout);

	cb_DoAutostart->SetValue(doAutostart);

	autoConnectBox->SetValue(autoConnect);
	wxGetApp().connectCommunicator(autoConnect);

	toolTipCheckBox->SetValue(enableToolTips);
	toolTipSpinCtrl->SetValue(toolTipTimeout);
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
	selectedProfile = wxEmptyString;
	loadedProfile = wxEmptyString;
	wxConfig::Get()->Read(wxT("/Options/LoadedProfile"), &loadedProfile);
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
		profileTabUpdate();
	} else {
		anMessageBox(
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
		anMessageBox(
		    wxString::Format(_("Failed to import from \"%ls\"."),
		       selectedProfile.c_str()),
		    _("Load profile"), wxOK | wxICON_ERROR, this);
	}
}

void
ModAnoubisMainPanelImpl::OnProfileSaveClicked(wxCommandEvent &)
{
	int errnum;
	wxString error_msg;

	DlgProfileSelection *dlg =
	    new DlgProfileSelection(loadedProfile, this);

	if (dlg->ShowModal() == wxID_OK) {
		PolicyCtrl *policyCtrl = PolicyCtrl::getInstance();
		wxString profile = dlg->getSelectedProfile();

		if (!policyCtrl->isProfileWritable(profile)) {
			anMessageBox(
			    wxString::Format(
			       _("The profile \"%ls\" is not writable!"),
			       profile.c_str()),
			    _("Save profile"), wxOK | wxICON_ERROR, this);
			dlg->Destroy();
			return;
		}

		if ((errnum = policyCtrl->exportToProfile(profile)) == 0) {
			profileTabUpdate();
		} else {
			error_msg = wxString::From8BitData(
			    anoubis_strerror(-errnum));
			anMessageBox(
			    wxString::Format(_("Failed to export to \"%ls\": "
				"%ls."), profile.c_str(), error_msg.c_str()),
			    _("Save profile"), wxOK | wxICON_ERROR, this);
		}
	}

	dlg->Destroy();
}

void
ModAnoubisMainPanelImpl::OnProfileActivateClicked(wxCommandEvent &)
{
	PolicyCtrl *policyCtrl = PolicyCtrl::getInstance();
	PolicyCtrl::PolicyResult polRes;
	long userId;

	if (!policyCtrl->importFromProfile(selectedProfile)) {
		anMessageBox(
		    wxString::Format(_("Failed to import from \"%ls\"."),
		       selectedProfile.c_str()),
		    _("Activate profile"), wxOK | wxICON_ERROR, this);
		return;
	}

	userId = policyCtrl->getUserId();
	if (userId == -1) {
		anMessageBox(
		    _("Could not obtain user-policy."),
		    _("Activate profile"), wxOK | wxICON_ERROR, this);
		return;
	}

	polRes = policyCtrl->sendToDaemon(userId);
	if (polRes == PolicyCtrl::RESULT_POL_ERR) {
		anMessageBox(
		    _("Could not activate user-policy.\n"
		      "No connection to anoubisd."),
		    _("Activate profile"), wxOK | wxICON_ERROR, this);
		return;
	} else if (polRes == PolicyCtrl::RESULT_POL_WRONG_PASS) {
		anMessageBox(
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
	profileListCtrl->GetItem(listItem);

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

	for (int i = 0; i < policyCtrl->getSize(); i++)
		VersionProfileChoice->Append(
		    policyCtrl->getProfile(i)->getProfileName());

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
		anMessageBox(_("Repository not initialized."),
		    _("Update versions"), wxOK | wxICON_ERROR, this);
		return;
	}

	if (!versionCtrl->isPrepared()) {
		anMessageBox(_("Failed to prepare versioning system."),
		    _("Update versions"), wxOK | wxICON_ERROR, this);
		return;
	}

	if (profile.IsEmpty())
		return;
	if (!versionCtrl->fetchVersionList(profile)) {
		anMessageBox(_("Failed to fetch versioning information."),
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
	NotificationCtrl	*notifyCtrl = NotificationCtrl::instance();

	if (tb_MainAnoubisNotify->GetCurrentPage()
	   == tb_MainAnoubisNotification) {
		wxCommandEvent	showEvent(anEVT_OPEN_ALERTS);
		showEvent.SetInt(0);
		showEvent.SetExtraLong(false);
		wxPostEvent(AnEvents::getInstance(), showEvent);
	}

	if (currentNotify_) {
		/*
		 * A DaemonAnswerNotify can remove an Escalation from
		 * the list. Reset currentNotify_ in this case.
		 */
		eNotify = dynamic_cast<EscalationNotify *>(currentNotify_);
		if (eNotify && eNotify->isAnswered() &&
		    listPerspective_ == notifyCtrl->getPerspective(
		    NotificationCtrl::LIST_NOTANSWERED))
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

	/*
	 * If the list is not empty and the current element is not the
	 * first element in the list enable buttons for previous and first
	 * element.
	 */
	if (maxElementNo > 0 && elementNo > 0) {
		bt_first->Enable();
		bt_previous->Enable();
	} else {
		bt_first->Disable();
		bt_previous->Disable();
	}
	/*
	 * If the list is not empty and the current element is not the
	 * last element in the list enable buttons for the next and last
	 * element.
	 */
	if (maxElementNo > 0 && elementNo + 1 < maxElementNo) {
		bt_next->Enable();
		bt_last->Enable();
	} else {
		bt_next->Disable();
		bt_last->Disable();
	}

	/*
	 * Only update the escalation window if the escalation actually
	 * changed.
	 */
	if (currentNotify_ == savedNotify_) {
		/*
		 * Fix for bug #1363:
		 * Escalation view not properly initialized
		 * If current notify == saved notify == NULL
		 * we have to hide all concerning widgets.
		 */
		if (currentNotify_ == NULL) {
			HIDESLOT(1);
			HIDESLOT(2);
			HIDESLOT(3);
			HIDESLOT(4);
			HIDESLOT(5);
			HIDESLOT(6);
			pn_Escalation->Hide();
			tx_answerValue->Hide();
			Layout();
			Refresh();
		}
		return;
	}
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
	tb_MainAnoubisNotification->Layout();
	Layout();
	Refresh();
}

void
ModAnoubisMainPanelImpl::setPerspective(NotificationCtrl::ListPerspectives p)
{
	NotificationCtrl			*notifyCtrl;

	notifyCtrl = NotificationCtrl::instance();
	currentNotify_ = NULL;

	removeSubject(listPerspective_);

	listPerspective_ = notifyCtrl->getPerspective(p);
	if (listPerspective_ != NULL) {
		it_ = listPerspective_->begin();
	} else {
		it_ = 0;
	}
	if (it_) {
		currentNotify_ = notifyCtrl->getNotification(*it_);
	} else {
		currentNotify_ = NULL;
	}

	addSubject(listPerspective_);
	update();
}

void
ModAnoubisMainPanelImpl::OnTypeChoosen(wxCommandEvent& event)
{
	NotificationCtrl::ListPerspectives	perspectiveType;

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
	setPerspective(perspectiveType);
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
	if (proto == IPPROTO_TCP || proto == IPPROTO_SCTP) {
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
		setPerspective(NotificationCtrl::LIST_NOTANSWERED);
	}

	if (event.GetString().Cmp(wxT("ALERTS")) == 0) {
		wxCommandEvent	showEvent(anEVT_OPEN_ALERTS);
		showEvent.SetInt(0);
		wxPostEvent(AnEvents::getInstance(), showEvent);
		/* select "current messages" */
		ch_type->SetSelection(1);
		setPerspective(NotificationCtrl::LIST_MESSAGES);
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
ModAnoubisMainPanelImpl::OnEnableUpgradeMsg(wxCommandEvent&)
{
	wxConfig::Get()->Write(wxT("/Options/ShowUpgradeMessage"),
	    cb_ShowUpgradeMsg->GetValue());
}

void
ModAnoubisMainPanelImpl::OnEnableKernelMsg(wxCommandEvent&)
{
	wxConfig::Get()->Write(wxT("/Options/ShowKernelUpgradeMessage"),
	    cb_ShowKernelMsg->GetValue());
}

void
ModAnoubisMainPanelImpl::OnEnableInformationMsg(wxCommandEvent&)
{
	wxConfig::Get()->Write(wxT("/Options/ShowCertMessage"),
	    cb_ShowKeyGenInfoMsg->GetValue());
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

	if (tb_MainAnoubisNotify->GetCurrentPage()
	   == tb_MainAnoubisNotification) {
		wxCommandEvent	showEvent(anEVT_OPEN_ALERTS);
		showEvent.SetInt(0);
		wxPostEvent(AnEvents::getInstance(), showEvent);
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
		anMessageBox(_("No profile selected"), _("Restore version"),
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

		anMessageBox(msg, _("Restore version"),
		    wxOK | wxICON_ERROR, this);
		return;
	}

	/* Import policy into application */
	PolicyRuleSet *rs = new PolicyRuleSet(1, geteuid(), apn_rs);
	if (useActiveProfile) {
		if (!policyCtrl->importPolicy(rs))
			anMessageBox(_("Failed to import the active policy."),
			    _("Restore version"), wxOK | wxICON_ERROR, this);
	} else {
		if (!policyCtrl->importPolicy(rs, profile)) {
			wxString msg = wxString::Format(_(
			    "Failed to import the policy\n"
			    "of the \"%ls\" profile"), profile.c_str());

			anMessageBox(msg, _("Restore version"),
			    wxOK | wxICON_ERROR, this);
		}
		if (!versionCtrl->createVersion(rs, profile,
		    _("automatically created by restore"), true)) {
			anMessageBox(_("Old version imported but could not "
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

		anMessageBox(msg, title, wxOK | wxICON_ERROR, this);
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

			anMessageBox(msg, title, wxOK | wxICON_ERROR, this);
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
	if (current->getBinaryName() == wxEmptyString) {
		enabledurationbuttons = false;
		enableeditor = false;
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
	anMessageBox(_("Failed to create a rule from escalation"),
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
	bool showKernelUpgradeMessage = true;
	bool showKeyGenInfoMessage = true;

	wxConfig::Get()->Read(wxT
	    ("/Options/ShowUpgradeMessage"),&showUpgradeMessage);
	wxConfig::Get()->Read(wxT
	    ("/Options/ShowKernelUpgradeMessage"),&showKernelUpgradeMessage);
	wxConfig::Get()->Read(wxT("/Options/ShowCertMessage"),
	    &showKeyGenInfoMessage);

	cb_ShowUpgradeMsg->SetValue(showUpgradeMessage);
	cb_ShowKernelMsg->SetValue(showKernelUpgradeMessage);
	cb_ShowKeyGenInfoMsg->SetValue(showKeyGenInfoMessage);

	event.Skip();
}

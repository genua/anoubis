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
    wxWindowID id) : ModAnoubisMainPanelBase(parent, id)
{
	AnEvents *anEvents;

	list_ = NOTIFY_LIST_NOTANSWERED;
	currentNotify_ = NULL;
	userOptions_ = wxGetApp().getUserOptions();
	anEvents = AnEvents::getInstance();

	/* read and restore Escalations Settings */
	readOptions();

	/* Initialization of profiles */
	profileTabInit();

	/* Initialize list of versions */
	versionListInit();
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
	tb_MainAnoubisNotify->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
	    wxNotebookEventHandler(
	       ModAnoubisMainPanelImpl::OnNotebookTabChanged),
	    NULL, this);
}

ModAnoubisMainPanelImpl::~ModAnoubisMainPanelImpl(void)
{
	AnEvents *anEvents;

	anEvents = AnEvents::getInstance();

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

	/* write Autostart Settings */
	userOptions_->Write(wxT("/Options/Autostart"),
	    cb_DoAutostart->IsChecked());

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
}

void
ModAnoubisMainPanelImpl::readOptions(void)
{
	bool SendEscalation = true;
	bool NoEscalationTimeout = true;
	int EscalationTimeout = 0;
	bool SendAlert = false;
	bool NoAlertTimeout = false;
	bool DoAutostart = true;
	int AlertTimeout = 10;
	bool AutoChecksum = false;
	bool AutoConnect = false;
	bool EnableToolTips = true;
	int ToolTipTimeout = 1;

	/* read the stored Option Settings */
	userOptions_->Read(wxT("/Options/SendEscalations"), &SendEscalation);
	userOptions_->Read(wxT("/Options/NoEscalationsTimeout"),
	    &NoEscalationTimeout);
	userOptions_->Read(wxT("/Options/EscalationTimeout"),
	    &EscalationTimeout);

	userOptions_->Read(wxT("/Options/SendAlerts"), &SendAlert);
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
	profileList->InsertColumn(0, wxT(""),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
	profileList->InsertColumn(1, _("Profile"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);

	selectedProfile = wxEmptyString;
	loadedProfile = wxEmptyString;
	fillProfileList();

	/* Adjust width of profile-column */
	int width = profileList->GetClientSize().GetWidth();
	width -= profileList->GetColumnWidth(0);
	profileList->SetColumnWidth(1, width);

}

void
ModAnoubisMainPanelImpl::profileTabUpdate(void)
{
	ProfileCtrl *profileCtrl = ProfileCtrl::getInstance();
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
	    profileCtrl->isProfileWritable(selectedProfile));
	profileLoadButton->Enable(haveSelected);
	profileActivateButton->Enable(haveSelected);
}

void
ModAnoubisMainPanelImpl::fillProfileList(void)
{
	ProfileCtrl *profileCtrl = ProfileCtrl::getInstance();
	wxArrayString profiles = profileCtrl->getProfileList();

	profileList->DeleteAllItems();

	for (unsigned int i = 0; i < profiles.GetCount(); i++) {
		profileList->InsertItem(i, wxEmptyString);

		if (!profileCtrl->isProfileWritable(profiles[i]))
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
	ProfileCtrl *profileCtrl = ProfileCtrl::getInstance();

	if (profileCtrl->removeProfile(selectedProfile)) {
		selectedProfile = wxEmptyString;
		fillProfileList();
		profileTabUpdate();
	} else {
		wxMessageBox(
		    wxString::Format(_("Failed to remove \"%s\"."),
		       selectedProfile.c_str()),
		    _("Remove profile"), wxOK | wxICON_ERROR, this);
	}
}

void
ModAnoubisMainPanelImpl::OnProfileLoadClicked(wxCommandEvent &)
{
	ProfileCtrl *profileCtrl = ProfileCtrl::getInstance();

	if (profileCtrl->importFromProfile(selectedProfile)) {
		loadedProfile = selectedProfile;
		profileTabUpdate();
	} else {
		wxMessageBox(
		    wxString::Format(_("Failed to import from \"%s\"."),
		       selectedProfile.c_str()),
		    _("Load profile"), wxOK | wxICON_ERROR, this);
	}
}

void
ModAnoubisMainPanelImpl::OnProfileSaveClicked(wxCommandEvent &)
{
	DlgProfileSelection dlg(loadedProfile, this);

	if (dlg.ShowModal() == wxID_OK) {
		ProfileCtrl *profileCtrl = ProfileCtrl::getInstance();
		wxString profile = dlg.getSelectedProfile();

		if (!profileCtrl->isProfileWritable(profile)) {
			wxMessageBox(
			    wxString::Format(
			       _("The profile \"%s\" is not writable!"),
			       profile.c_str()),
			    _("Save profile"), wxOK | wxICON_ERROR, this);
			return;
		}

		if (profileCtrl->exportToProfile(profile)) {
			fillProfileList();
			profileTabUpdate();
		} else {
			wxMessageBox(
			    wxString::Format(_("Failed to export to \"%s\"."),
			       profile.c_str()),
			    _("Save profile"), wxOK | wxICON_ERROR, this);
		}
	}
}

void
ModAnoubisMainPanelImpl::OnProfileActivateClicked(wxCommandEvent &)
{
	ProfileCtrl *profileCtrl = ProfileCtrl::getInstance();
	long userId;

	if (!profileCtrl->importFromProfile(selectedProfile)) {
		wxMessageBox(
		    wxString::Format(_("Failed to import from \"%s\"."),
		       selectedProfile.c_str()),
		    _("Activate profile"), wxOK | wxICON_ERROR, this);
		return;
	}

	userId = profileCtrl->getUserId();
	if (userId == -1) {
		wxMessageBox(
		    _("Could not obtain user-policy."),
		    _("Activate profile"), wxOK | wxICON_ERROR, this);
		return;
	}

	if (!profileCtrl->sendToDaemon(userId)) {
		wxMessageBox(
		    _("Could not activate user-policy.\n"
		      "No connection to anoubisd."),
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
ModAnoubisMainPanelImpl::versionListInit(void)
{
	/* Initialize table */
	columnNames_[MODANOUBIS_VMLIST_COLUMN_TYPE] = _("Type");
	columnNames_[MODANOUBIS_VMLIST_COLUMN_DATE] = _("Date");
	columnNames_[MODANOUBIS_VMLIST_COLUMN_TIME] = _("Time");
	columnNames_[MODANOUBIS_VMLIST_COLUMN_USER] = _("User");
	columnNames_[MODANOUBIS_VMLIST_COLUMN_VERSION] = _("Version");

	for (int i=0; i<MODANOUBIS_VMLIST_COLUMN_EOL; i++) {
		VersionListCtrl->InsertColumn(i, columnNames_[i],
		    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
	}

	versionListSetMsg(_("(Initializing version management...)"));
}

void
ModAnoubisMainPanelImpl::versionListUpdate(void)
{
	VersionCtrl	*versionCtrl;
	ProfileCtrl	*profileCtrl;
	wxString	 profile;

	versionCtrl = VersionCtrl::getInstance();
	profileCtrl = ProfileCtrl::getInstance();
	profile = versionCtrl->getVersionProfile();

	VersionProfileChoice->Clear();
	VersionProfileChoice->Append(profileCtrl->getProfileList());
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
	ProfileCtrl	*profileCtrl;
	unsigned int	count = 0;
	wxString	profile;

	if (VersionActivePolicyRadioButton->GetValue())
		profile = wxT("active");
	else
		profile = VersionProfileChoice->GetStringSelection();

	versionCtrl = VersionCtrl::getInstance();
	profileCtrl = ProfileCtrl::getInstance();

	VersionListCtrl->DeleteAllItems();

	if (!versionCtrl->isInitialized()) {
		versionListSetMsg(_("(Repository not initialized.)"));
		return;
	}

	if (!versionCtrl->isPrepared()) {
		versionListSetMsg(_("(Failed to prepare versioning system.)"));
		return;
	}

	if (profile.IsEmpty())
		return;
	if (!versionCtrl->fetchVersionList(profile)) {
		versionListSetMsg(
		    _("(Failed to fetch versioning information.)"));
		return;
	}

	count = versionCtrl->getNumVersions();

	if (count == 0) {
		versionListSetMsg(_("(No versioning information available.)"));
		return;
	}

	for (unsigned int i = 0; i < count; i++) {
		const ApnVersion &version = versionCtrl->getVersion(i);

		VersionListCtrl->InsertItem(i, wxEmptyString);

		VersionListCtrl->SetItem(i, MODANOUBIS_VMLIST_COLUMN_TYPE,
		    (version.isAutoStore() ? _("Auto") : _("Manual")));
		VersionListCtrl->SetItem(i, MODANOUBIS_VMLIST_COLUMN_DATE,
		    version.getTimestamp().FormatDate());
		VersionListCtrl->SetItem(i, MODANOUBIS_VMLIST_COLUMN_TIME,
		    version.getTimestamp().FormatTime());
		VersionListCtrl->SetItem(i, MODANOUBIS_VMLIST_COLUMN_USER,
		    version.getUsername());
		VersionListCtrl->SetItem(i, MODANOUBIS_VMLIST_COLUMN_VERSION,
		    wxString::Format(wxT("%d"), version.getVersionNo()));
	}

	VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_TYPE,
	    wxLIST_AUTOSIZE);
	VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_DATE,
	    wxLIST_AUTOSIZE);
	VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_TIME,
	    wxLIST_AUTOSIZE);
	VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_USER,
	    wxLIST_AUTOSIZE);
	VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_VERSION,
	    wxLIST_AUTOSIZE_USEHEADER);
}

void
ModAnoubisMainPanelImpl::versionListSetMsg(const wxString &msg)
{
	VersionListCtrl->InsertItem(0, wxEmptyString);
	VersionListCtrl->SetItem(0, MODANOUBIS_VMLIST_COLUMN_DATE, msg);
	VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_DATE,
	    wxLIST_AUTOSIZE);
}

int ModAnoubisMainPanelImpl::versionListCanAccess(bool needSelection) const
{
	VersionCtrl *versionCtrl = VersionCtrl::getInstance();

	if (!versionCtrl->isPrepared())
		return (needSelection ? -1 : false);

	if (needSelection) {
		if (versionCtrl->getNumVersions() == 0)
			return (needSelection ? -1 : false);

		/* Search for the selected index */
		return VersionListCtrl->GetNextItem(
		    -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	}
	else
		return (true);
}

void
ModAnoubisMainPanelImpl::displayAlert(void)
{
	AlertNotify	*aNotify;

	aNotify = (AlertNotify *)currentNotify_;

	SHOWSLOT(1, _("Time:"), aNotify->getTime());
	SHOWSLOT(2, _("Module:"), aNotify->getModule());
	if (aNotify->getModule().Cmp(wxT("ALF")) == 0) {
		SHOWSLOT(3, _("Traffic:"), aNotify->getPath());
	} else {
		SHOWSLOT(3, _("Path:"), aNotify->getPath());
	}
	SHOWSLOT(4, _("Operation:"), aNotify->getOperation());
	SHOWSLOT(5, _("Origin:"), aNotify->getOrigin());
	SHOWSLOT(6, _("Checksum:"), aNotify->getCheckSum());
}

void
ModAnoubisMainPanelImpl::displayEscalation(void)
{
	EscalationNotify	*eNotify;

	eNotify = (EscalationNotify *)currentNotify_;

	SHOWSLOT(1, _("Time:"), eNotify->getTime());
	SHOWSLOT(2, _("Module:"), eNotify->getModule());
	if (eNotify->getModule().Cmp(wxT("ALF")) == 0) {
		SHOWSLOT(3, _("Traffic:"), eNotify->getPath());
	} else {
		SHOWSLOT(3, _("Path:"), eNotify->getPath());
	}
	SHOWSLOT(4, _("Operation:"), eNotify->getOperation());
	SHOWSLOT(5, _("Origin:"), eNotify->getOrigin());
	SHOWSLOT(6, _("Checksum:"), eNotify->getCheckSum());
}

void
ModAnoubisMainPanelImpl::displayLog(void)
{
	LogNotify	*lNotify;

	lNotify = (LogNotify *)currentNotify_;

	SHOWSLOT(1, _("Time:"), lNotify->getTime());
	SHOWSLOT(2, _("Module:"), lNotify->getModule());
	if (lNotify->getModule().Cmp(wxT("ALF")) == 0) {
		SHOWSLOT(3, _("Traffic:"), lNotify->getPath());
	} else {
		SHOWSLOT(3, _("Path:"), lNotify->getPath());
	}
	SHOWSLOT(4, _("Operation:"), lNotify->getOperation());
	SHOWSLOT(5, _("Origin:"), lNotify->getOrigin());
	SHOWSLOT(6, _("Checksum:"), lNotify->getCheckSum());
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
ModAnoubisMainPanelImpl::update(void)
{
	ModAnoubis	*module;
	wxString	 s;
	size_t		 maxElementNo;
	size_t		 elementNo;

	module = (ModAnoubis *)(wxGetApp().getModule(ANOUBIS));
	maxElementNo = module->getListSize(list_);
	elementNo = 0;

	if (maxElementNo > 0) {
		if (currentNotify_ == NULL) {
			currentNotify_ = module->getFirst(list_);
		}
		elementNo = module->getElementNo(list_);
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

	if (currentNotify_ != NULL) {
		if IS_ALERTOBJ(currentNotify_)
			displayAlert();
		if IS_ESCALATIONOBJ(currentNotify_)
			displayEscalation();
		if IS_LOGOBJ(currentNotify_)
			displayLog();
	} else {
		HIDESLOT(1);
		HIDESLOT(2);
		HIDESLOT(3);
		HIDESLOT(4);
		HIDESLOT(5);
		HIDESLOT(6);
	}

	if ((currentNotify_ != NULL) && IS_ESCALATIONOBJ(currentNotify_)) {
		EscalationNotify *eNotify = (EscalationNotify *)currentNotify_;

		if (!eNotify->isAnswered()) {
			pn_question->Show();
			tx_answerValue->Hide();
		} else {
			pn_question->Hide();
			tx_answerValue->SetLabel(
			    eNotify->getAnswer()->getAnswer());
			tx_answerValue->Show();
		}
	} else {
		pn_question->Hide();
		tx_answerValue->Hide();
	}
	Layout();
}

void
ModAnoubisMainPanelImpl::OnTypeChoosen(wxCommandEvent& event)
{
	currentNotify_ = NULL;

	/* keep this in sync with ch_type elements */
	switch (event.GetSelection()) {
	case 0:
		list_ = NOTIFY_LIST_NOTANSWERED;
		break;
	case 1:
		list_ = NOTIFY_LIST_MESSAGE;
		break;
	case 2:
		list_ = NOTIFY_LIST_ANSWERED;
		break;
	case 3:
		list_ = NOTIFY_LIST_ALL;
		break;
	default:
		list_ = NOTIFY_LIST_NONE;
		break;
	}
	update();
}

void
ModAnoubisMainPanelImpl::OnFirstBtnClick(wxCommandEvent&)
{
	ModAnoubis *module;

	module = (ModAnoubis *)(wxGetApp().getModule(ANOUBIS));
	currentNotify_ = module->getFirst(list_);

	update();
}

void
ModAnoubisMainPanelImpl::OnPreviousBtnClick(wxCommandEvent&)
{
	ModAnoubis *module;

	module = (ModAnoubis *)(wxGetApp().getModule(ANOUBIS));
	currentNotify_ = module->getPrevious(list_);

	update();
}

void
ModAnoubisMainPanelImpl::OnNextBtnClick(wxCommandEvent&)
{
	ModAnoubis *module;

	module = (ModAnoubis *)(wxGetApp().getModule(ANOUBIS));
	currentNotify_ = module->getNext(list_);

	update();
}

void
ModAnoubisMainPanelImpl::OnLastBtnClick(wxCommandEvent&)
{
	ModAnoubis *module;

	module = (ModAnoubis *)(wxGetApp().getModule(ANOUBIS));
	currentNotify_ = module->getLast(list_);

	update();
}

void
ModAnoubisMainPanelImpl::answer(bool permission)
{
	ModAnoubis	*module;
	NotifyAnswer	*answer;

	if (IS_ESCALATIONOBJ(currentNotify_)) {
		module = (ModAnoubis *)(wxGetApp().getModule(ANOUBIS));

		if (rb_number->GetValue()) {
			answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE,
			    permission);
		} else if (rb_procend->GetValue()) {
			answer = new NotifyAnswer(NOTIFY_ANSWER_PROCEND,
			    permission);
		} else if (rb_time->GetValue()) {
			answer = new NotifyAnswer(NOTIFY_ANSWER_TIME,
			    permission, sc_time->GetValue(),
			    (enum timeUnit)ch_time->GetCurrentSelection());
		} else if (rb_always->GetValue()) {
			answer = new NotifyAnswer(NOTIFY_ANSWER_FOREVER,
			    permission);
		} else {
			answer = new NotifyAnswer(NOTIFY_ANSWER_NONE,
			    permission);
		}

		module->answerEscalationNotify(
		    (EscalationNotify *)currentNotify_, answer);
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
		list_ = NOTIFY_LIST_NOTANSWERED;
	}

	if (event.GetString().Cmp(wxT("ALERTS")) == 0) {
		/* select "current messages" */
		ch_type->SetSelection(1);
		list_ = NOTIFY_LIST_MESSAGE;
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
	int idx;

	if ((idx = versionListCanAccess(true)) == -1)
		return;

	VersionCtrl *versionCtrl = VersionCtrl::getInstance();

	/* Selected version */
	const ApnVersion &version = versionCtrl->getVersion(idx);

	/* Update comment-field with selected version*/
	VersionShowCommentTextCtrl->SetValue(version.getComment());
}

void
ModAnoubisMainPanelImpl::OnVersionRestoreButtonClick(wxCommandEvent&)
{
	VersionCtrl	*versionCtrl = VersionCtrl::getInstance();
	ProfileCtrl	*profileCtrl = ProfileCtrl::getInstance();
	bool		useActiveProfile;
	wxString	profile;
	int		idx;

	if ((idx = versionListCanAccess(true)) == -1)
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
			    "of the \"%s\" profile"), profile.c_str());

		wxMessageBox(msg, _("Restore version"),
		    wxOK | wxICON_ERROR, this);
		return;
	}

	/* Import policy into application */
	PolicyRuleSet *rs = new PolicyRuleSet(1, geteuid(), apn_rs);
	if (useActiveProfile) {
		if (!profileCtrl->importPolicy(rs))
			wxMessageBox(_("Failed to import the active policy."),
			    _("Restore version"), wxOK | wxICON_ERROR, this);
	} else {
		if (!profileCtrl->importPolicy(rs, profile)) {
			wxString msg = wxString::Format(_(
			    "Failed to import the policy\n"
			    "of the \"%s\" profile"), profile.c_str());

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
	int idx;

	if ((idx = versionListCanAccess(true)) == -1)
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
	int idx;

	if ((idx = versionListCanAccess(true)) == -1)
		return;

	/* On success update list of displayed version, has changed */
	VersionCtrl *versionCtrl = VersionCtrl::getInstance();
	if (!versionCtrl->deleteVersion(idx)) {
		wxString msg = _("Failed to remove the selected version!");
		wxString title = _("Remove version");

		wxMessageBox(msg, title, wxOK | wxICON_ERROR, this);
	}
	else
		versionListUpdate();
}

void
ModAnoubisMainPanelImpl::OnVersionShowButtonClick(wxCommandEvent&)
{
	int idx;

	if ((idx = versionListCanAccess(true)) == -1)
		return;

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

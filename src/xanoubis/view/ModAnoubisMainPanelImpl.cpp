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

#define __STDC_FORMAT_MACROS
#include <stdint.h>
#include <inttypes.h>
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

#include "AlertNotify.h"
#include "AnEvents.h"
#include "AnListProperty.h"
#include "ApnVersion.h"
#include "DlgProfileSelection.h"
#include "EscalationNotify.h"
#include "KeyCtrl.h"
#include "JobCtrl.h"
#include "LogNotify.h"
#include "MainUtils.h"
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
#include "PolicyRuleSet.h"
#include "ProfileListCtrl.h"
#include "PSListCtrl.h"
#include "PSEntry.h"
#include "DefaultConversions.h"

#include "apn.h"
#include "main.h"


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

#define ADDCOLUMN(title, function, type, conversion, width) \
	do { \
		psList->addColumn( \
		    new AnFmtListProperty<PSEntry, type>( \
		    title, &PSEntry::function, NULL, conversion), width); \
	} while (0)

#define ADDRULECOLUMN(title, user, admin, width) \
	do { \
		psList->addColumn(new RuleIdListProperty( \
		    title, &PSEntry::user, &PSEntry::admin), width); \
	} while (0)

class RuleIdListProperty : public AnListProperty
{
	public:
		RuleIdListProperty(const wxString &header,
		    unsigned long (PSEntry::*um)(void) const,
		    unsigned long (PSEntry::*am)(void) const)
		{
			header_ = header;
			um_ = um;
			am_ = am;
		}

		wxString getHeader(void) const
		{
			return (header_);
		}

		wxString getText(AnListClass *obj) const
		{
			PSEntry *entry = dynamic_cast<PSEntry *>(obj);

			if (entry != 0 && um_ != 0 && am_ != 0) {
				unsigned long userId = (entry->*um_)();
				unsigned long adminId = (entry->*am_)();

				if (userId > 0 && adminId > 0)
					return _("User/Admin");
				else if (userId > 0 && adminId == 0)
					return _("User");
				else if (userId == 0 && adminId > 0)
					return _("Admin");
			}

			return (wxEmptyString);
		}

		AnIconList::IconId getIcon(AnListClass *) const
		{
			return (AnIconList::ICON_NONE);
		}

	private:
		wxString header_;
		unsigned long (PSEntry::*um_)(void) const;
		unsigned long (PSEntry::*am_)(void) const;
};

ModAnoubisMainPanelImpl::ModAnoubisMainPanelImpl(wxWindow* parent,
    wxWindowID id) : ModAnoubisMainPanelBase(parent, id), Observer(NULL)
{
	AnEvents *anEvents;
	NotificationCtrl	*notifyCtrl;
	PSListCtrl		*psListCtrl;

	currentNotify_ = NULL;
	savedNotify_ = NULL;
	anEvents = AnEvents::instance();

	notifyCtrl = NotificationCtrl::instance();
	listPerspective_ = notifyCtrl->getPerspective(
		NotificationCtrl::LIST_NOTANSWERED);
	elementNoHash_[listPerspective_] = 0;
	sizeListNotAnswered_ = 0;

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
	JobCtrl::instance()->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(
	    ModAnoubisMainPanelImpl::onConnectionStateChange), NULL, this);

	anEvents->Connect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnLoadRuleSet),
	    NULL, this);
	anEvents->Connect(anEVT_ANOUBISOPTIONS_UPDATE, wxCommandEventHandler(
	    ModAnoubisMainPanelImpl::OnAnoubisOptionsUpdate), NULL, this);

	tb_MainAnoubisNotify->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
	    wxNotebookEventHandler(
	       ModAnoubisMainPanelImpl::OnNotebookTabChanged),
	    NULL, this);
	pathcomp_.clear();
	pathKeep_ = minPathKeep_ = 0;
	addSubject(listPerspective_);

	psList->setStateKey(wxT("/State/PSListCtrl"));
	/* configure Process List tab */
	ADDCOLUMN(_("Process ID"), getProcessId, const wxString, NULL, 80);
	ADDCOLUMN(_("User"), getEUID, long, &uidToString, 100);
	ADDRULECOLUMN(_("ALF"), getAlfRuleUser, getAlfRuleAdmin, 90);
	ADDRULECOLUMN(_("SB"), getSbRuleUser, getSbRuleAdmin, 90);
	ADDRULECOLUMN(_("CTX"), getCtxRuleUser, getCtxRuleAdmin, 90);
	ADDCOLUMN(_("Playground ID"), getPlaygroundId, uint64_t,
	    &pgidToString, 100);
	ADDCOLUMN(_("Command"), getShortProcessName, const wxString, NULL, 180);

	psListCtrl = PSListCtrl::instance();
	psListCtrl->Connect(anEVT_PSLIST_ERROR, wxCommandEventHandler(
	    ModAnoubisMainPanelImpl::OnPSListError), NULL, this);
	psList->setRowProvider(psListCtrl->getPSListProvider());

	update();
}

ModAnoubisMainPanelImpl::~ModAnoubisMainPanelImpl(void)
{
	AnEvents *anEvents;

	anEvents = AnEvents::instance();

	if (listPerspective_ != NULL)
		removeSubject(listPerspective_);

	anEvents->Disconnect(anEVT_ESCALATIONS_SHOW,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnEscalationsShow),
	    NULL, this);
	anEvents->Disconnect(anEVT_ANOUBISOPTIONS_SHOW,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnAnoubisOptionShow),
	    NULL, this);
	JobCtrl::instance()->Disconnect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(
	    ModAnoubisMainPanelImpl::onConnectionStateChange), NULL, this);
	anEvents->Disconnect(anEVT_LOAD_RULESET,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnLoadRuleSet),
	    NULL, this);
	anEvents->Disconnect(anEVT_ANOUBISOPTIONS_UPDATE, wxCommandEventHandler(
	    ModAnoubisMainPanelImpl::OnAnoubisOptionsUpdate), NULL, this);
	PSListCtrl::instance()->Disconnect(anEVT_PSLIST_ERROR,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnPSListError),
	    NULL, this);
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
	MainUtils::instance()->autoStart(doAutostart);

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
	MainUtils::instance()->connectCommunicator(autoConnect);

	toolTipCheckBox->SetValue(enableToolTips);
	toolTipSpinCtrl->SetValue(toolTipTimeout);
	toolTipParamsUpdate();

	/* set widgets visability and send options event */
	setOptionsWidgetsVisibility();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::setOptionsWidgetsVisibility(void)
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
	PolicyCtrl *policyCtrl = PolicyCtrl::instance();
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
ModAnoubisMainPanelImpl::onConnectionStateChange(wxCommandEvent &event)
{
	bool	connected = (event.GetInt() == JobCtrl::CONNECTED);

	psReloadButton->Enable(connected);
	psList->clearSelection();
	if (connected)  {
		PSListCtrl::instance()->updatePSList();
	} else {
		PSListCtrl::instance()->clearPSList();
	}
	event.Skip();
}

void
ModAnoubisMainPanelImpl::OnLoadRuleSet(wxCommandEvent &event)
{
	versionListUpdateFromSelection();
	event.Skip();
}

void
ModAnoubisMainPanelImpl::OnProfileDeleteClicked(wxCommandEvent &event)
{
	PolicyCtrl *policyCtrl = PolicyCtrl::instance();

	event.Skip();
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
ModAnoubisMainPanelImpl::OnProfileLoadClicked(wxCommandEvent &event)
{
	PolicyCtrl *policyCtrl = PolicyCtrl::instance();

	event.Skip();
	if (policyCtrl->importFromProfile(selectedProfile)) {
		loadedProfile = selectedProfile;
		wxConfig::Get()->Write(wxT("/Options/LoadedProfile"),
		    loadedProfile);
		profileTabUpdate();
	} else {
		anMessageBox(
		    wxString::Format(_("Failed to import from \"%ls\"."),
		       selectedProfile.c_str()),
		    _("Load profile"), wxOK | wxICON_ERROR, this);
	}
}

void
ModAnoubisMainPanelImpl::OnProfileSaveClicked(wxCommandEvent &event)
{
	int errnum;
	wxString error_msg;

	event.Skip();
	DlgProfileSelection *dlg =
	    new DlgProfileSelection(loadedProfile, this);

	if (dlg->ShowModal() == wxID_OK) {
		PolicyCtrl *policyCtrl = PolicyCtrl::instance();
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
ModAnoubisMainPanelImpl::OnProfileActivateClicked(wxCommandEvent &event)
{
	PolicyCtrl *policyCtrl = PolicyCtrl::instance();
	PolicyCtrl::PolicyResult polRes;
	long userId;

	event.Skip();
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
	wxConfig::Get()->Write(wxT("/Options/LoadedProfile"), loadedProfile);
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

	versionCtrl = VersionCtrl::instance();
	policyCtrl = PolicyCtrl::instance();
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
	wxString	profile;

	if (VersionActivePolicyRadioButton->GetValue())
		profile = wxT("active");
	else
		profile = VersionProfileChoice->GetStringSelection();

	versionCtrl = VersionCtrl::instance();

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

	wxPostEvent(AnEvents::instance(), showEvent);
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
	sendNotifierOptionsEvents(anEVT_ALERTNOTIFY_OPTIONS, show, timeout);
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
	long			 maxElementNo;
	long			 elementNo;
	EscalationNotify	*eNotify = NULL;
	NotificationCtrl	*notifyCtrl = NotificationCtrl::instance();
	NotificationPerspective	*listPerspectiveNotAnswered;

	if (tb_MainAnoubisNotify->GetCurrentPage()
	   == tb_MainAnoubisNotification) {
		wxCommandEvent	showEvent(anEVT_OPEN_ALERTS);
		showEvent.SetInt(0);
		showEvent.SetExtraLong(false);
		wxPostEvent(AnEvents::instance(), showEvent);
	}

	/*
	 * We may need the listPerspective_ of LIST_NOTANSWERED
	 *
	 * We need to adjust the remembered size of LIST_NOTANSWERED
	 * if a new escalation has been added.
	 */
	listPerspectiveNotAnswered = NotificationCtrl::instance()->
	    getPerspective(NotificationCtrl::LIST_NOTANSWERED);
	if (sizeListNotAnswered_ < listPerspectiveNotAnswered->getSize())
		sizeListNotAnswered_ = listPerspectiveNotAnswered->getSize();

	if (currentNotify_) {
		/*
		 * A DaemonAnswerNotify can remove an Escalation from
		 * the list. Reset currentNotify_ in this case.
		 */
		eNotify = dynamic_cast<EscalationNotify *>(currentNotify_);
		if (eNotify && eNotify->isAnswered()) {
			if (listPerspective_ == listPerspectiveNotAnswered) {
				currentNotify_ = NULL;
			} else {
				/*
				 * ONLY if the LIST_NOTANSWERED has been
				 * changed by removing an element &&
				 * removed element was an entry 'before'
				 * the element to be shown from
				 * LIST_NOTANSWERED &&
				 * index to element to be shown could be
				 * decremented
				 *
				 * -> decrement
				 */
				if ((listPerspectiveNotAnswered->getSize() <
				    sizeListNotAnswered_)
				    &&
				    (listPerspective_->getIndex(
					listPerspectiveNotAnswered->getId(
					elementNoHash_[
					    listPerspectiveNotAnswered])
				    ) - 1 >
				    listPerspective_->getIndex(
					currentNotify_->getId()))
				    &&
				    (elementNoHash_[listPerspectiveNotAnswered]
				    != 0))
				{
					elementNoHash_[
					    listPerspectiveNotAnswered] -= 1;
				}
			}
		} else {
			/*
			 * If an element from LIST_NOTANSWERED had been removed
			 * and the current element to be shown is not answered,
			 * the removal has been caused by a timeout.
			 * Thus we have to decrement the index.
			 */
			if ((listPerspectiveNotAnswered->getSize() <
			    sizeListNotAnswered_) &&
			    (elementNoHash_[listPerspectiveNotAnswered] != 0))
				elementNoHash_[listPerspectiveNotAnswered] -= 1;
		}
	}

	elementNo = elementNoHash_[listPerspective_];

	if (listPerspective_) {
		maxElementNo = listPerspective_->getSize();
	} else {
		maxElementNo = 0;
	}

	if (maxElementNo > 0) {
		if (elementNo > maxElementNo - 1)
			elementNo = maxElementNo - 1;
		if (currentNotify_ == NULL) {
			currentNotify_ = notifyCtrl->getNotification(
				listPerspective_->getId(elementNo)
			);
		}
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
	if (maxElementNo > 0 && elementNo + 1 < (long) maxElementNo) {
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
	if (currentNotify_ == savedNotify_ && currentNotify_ == NULL) {
		/*
		 * Fix for bug #1363:
		 * Escalation view not properly initialized
		 * If current notify == saved notify == NULL
		 * we have to hide all concerning widgets.
		 */
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

/**
 * Stringify user id, static method for use in AnFmtListProperty conversion.
 * @return user name, empty if uid is -1
 */
wxString
ModAnoubisMainPanelImpl::uidToString(long uid)
{
	if (uid == -1) {
		return wxEmptyString;
	} else {
		return MainUtils::instance()->getUserNameById(uid);
	}
}

/**
 * Stringify playground id, static method for use in AnFmtListProperty.
 * @return playground id in hex, empty if pgid is 0
 */
wxString
ModAnoubisMainPanelImpl::pgidToString(uint64_t pgid)
{
	if (pgid == 0) {
		return wxEmptyString;
	} else {
		return wxString::Format(wxT("%x"), pgid);
	}
}

void
ModAnoubisMainPanelImpl::setPerspective(NotificationCtrl::ListPerspectives p)
{
	NotificationCtrl			*notifyCtrl;

	notifyCtrl = NotificationCtrl::instance();

	if (listPerspective_ == notifyCtrl->getPerspective(
	    NotificationCtrl::LIST_NOTANSWERED))
		sizeListNotAnswered_ = listPerspective_->getSize();

	removeSubject(listPerspective_);

	listPerspective_ = notifyCtrl->getPerspective(p);

	if (p == NotificationCtrl::LIST_NOTANSWERED)
		sizeListNotAnswered_ = listPerspective_->getSize();

	if (listPerspective_->getSize() > 0)
		currentNotify_ = notifyCtrl->getNotification(
		    listPerspective_->getId(
			elementNoHash_[listPerspective_]));
	else
		currentNotify_ = NULL;

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
ModAnoubisMainPanelImpl::OnFirstBtnClick(wxCommandEvent &event)
{
	NotificationCtrl	*notifyCtrl;

	event.Skip();
	notifyCtrl = NotificationCtrl::instance();
	if (listPerspective_ != NULL) {
		elementNoHash_[listPerspective_] = 0;
		currentNotify_ = notifyCtrl->getNotification(
		    listPerspective_->getId(0));
	} else {
		currentNotify_ = NULL;
	}

	update();
}

void
ModAnoubisMainPanelImpl::OnPreviousBtnClick(wxCommandEvent &event)
{
	NotificationCtrl	*notifyCtrl;

	event.Skip();
	notifyCtrl = NotificationCtrl::instance();
	if (elementNoHash_[listPerspective_] != 0) {
		elementNoHash_[listPerspective_] -= 1;
		currentNotify_ = notifyCtrl->getNotification(
		    listPerspective_->getId(
			elementNoHash_[listPerspective_]));
	} else {
		currentNotify_ = NULL;
	}

	update();
}

void
ModAnoubisMainPanelImpl::OnNextBtnClick(wxCommandEvent &event)
{
	NotificationCtrl	*notifyCtrl;

	event.Skip();
	notifyCtrl = NotificationCtrl::instance();
	if (elementNoHash_[listPerspective_] <
	    listPerspective_->getSize() - 1)
	{
		elementNoHash_[listPerspective_] += 1;
		currentNotify_ = notifyCtrl->getNotification(
		    listPerspective_->getId(
			elementNoHash_[listPerspective_]));
	} else {
		currentNotify_ = NULL;
	}

	update();
}

void
ModAnoubisMainPanelImpl::OnLastBtnClick(wxCommandEvent &event)
{
	NotificationCtrl	*notifyCtrl;

	event.Skip();
	notifyCtrl = NotificationCtrl::instance();

	if (listPerspective_ != NULL) {
		/* get last */
		elementNoHash_[listPerspective_] =
		    listPerspective_->getSize()  - 1;
		currentNotify_ = notifyCtrl->getNotification(
		    listPerspective_->getId(
			elementNoHash_[listPerspective_]));
	} else {
		elementNoHash_[listPerspective_] = 0;
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

NotifyAnswer *
ModAnoubisMainPanelImpl::assembleTimeAnswer(bool permit)
{
	enum timeUnit	unit;
	long		value;
	wxString	selection;

	value = spin_EscalationTime->GetValue();
	selection = ch_EscalationTimeUnit->GetStringSelection();
	if (selection == _("Seconds")) {
		unit = TIMEUNIT_SECOND;
	} else if (selection == _("Minutes")) {
		unit = TIMEUNIT_MINUTE;
	} else if (selection == _("Hours")) {
		unit = TIMEUNIT_HOUR;
	} else if (selection == _("Days")) {
		unit = TIMEUNIT_DAY;
	} else {
		/* This should never happen. */
		unit = TIMEUNIT_SECOND;
	}

	return (new NotifyAnswer(NOTIFY_ANSWER_TIME, permit, value, unit));
}

void
ModAnoubisMainPanelImpl::answerEscalation(bool permit)
{
	bool			 rc = false;
	wxString		 escalationType;
	NotifyAnswer		*answer = NULL;
	EscalationNotify	*escalation = NULL;
	NotificationCtrl	*notificationCtrl = NULL;

	notificationCtrl = NotificationCtrl::instance();
	escalation = dynamic_cast<EscalationNotify *>(currentNotify_);
	if (escalation == NULL) {
		return;
	}

	/* Create answer */
	if (rb_EscalationOnce->GetValue()) {
		answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE, permit);
	} else if (rb_EscalationProcess->GetValue()) {
		answer = new NotifyAnswer(NOTIFY_ANSWER_PROCEND, permit);
	} else if (rb_EscalationTime->GetValue()) {
		answer = assembleTimeAnswer(permit);
	} else if (rb_EscalationAlways->GetValue()) {
		answer = new NotifyAnswer(NOTIFY_ANSWER_FOREVER, permit);
	} else {
		answer = new NotifyAnswer(NOTIFY_ANSWER_NONE, permit);
	}

	/* Set additional answer options */
	answer->setEditor(ck_EscalationEditor->GetValue());
	if (escalation->allowOptions()) {
		escalationType = escalation->getModule();
		if (escalationType == wxT("ALF")) {
			setAlfOptions(escalation, answer);
		} else if (escalationType == wxT("SFS")) {
			setSfsOptions(escalation, answer);
		} else if (escalationType == wxT("SANDBOX")) {
			setSbOptions(escalation, answer);
		}
	}

	escalation->setAnswer(answer);
	rc = notificationCtrl->answerEscalation(escalation, true);
	if (rc != true) {
		anMessageBox(_("Failed to create a rule from escalation"),
		    _("Error"), wxICON_ERROR);
	}
	currentNotify_ = NULL;
	update();
}

void
ModAnoubisMainPanelImpl::onAlfUserEditClicked( wxCommandEvent& event )
{
	unsigned long		ruleid = 0;
	const PSEntry		*entry = NULL;
	wxCommandEvent		showEvent(anEVT_SHOW_RULE);

	entry = PSListCtrl::instance()->getEntry(psList->getFirstSelection());
	event.Skip();
	ruleid = entry->getAlfRuleUser();

	if (ruleid == 0 ){
		alfUserEditButton->Disable();
	} else {
	    showEvent.SetInt(0); /* != Admin */
	    showEvent.SetExtraLong(ruleid);
	    wxPostEvent(AnEvents::instance(), showEvent);
	}
}

void
ModAnoubisMainPanelImpl::onAlfAdminEditClicked( wxCommandEvent& event )
{
	unsigned long		ruleid = 0;
	const PSEntry		*entry = NULL;
	wxCommandEvent		showEvent(anEVT_SHOW_RULE);

	entry = PSListCtrl::instance()->getEntry(psList->getFirstSelection());
	event.Skip();
	ruleid = entry->getAlfRuleAdmin();

	if (ruleid == 0 ){
		alfAdminEditButton->Disable();
	} else {
	    showEvent.SetInt(1); /* is Admin */
	    showEvent.SetExtraLong(ruleid);
	    wxPostEvent(AnEvents::instance(), showEvent);
	}
}

void
ModAnoubisMainPanelImpl::onSbUserEditClicked( wxCommandEvent& event )
{
	unsigned long		ruleid = 0;
	const PSEntry		*entry = NULL;
	wxCommandEvent		showEvent(anEVT_SHOW_RULE);

	entry = PSListCtrl::instance()->getEntry(psList->getFirstSelection());
	event.Skip();
	ruleid = entry->getSbRuleUser();

	if (ruleid == 0 ){
		sbUserEditButton->Disable();
	} else {
		showEvent.SetInt(0); /* != Admin */
		showEvent.SetExtraLong(ruleid);
		wxPostEvent(AnEvents::instance(), showEvent);
	}
}

void
ModAnoubisMainPanelImpl::onSbAdminEditClicked( wxCommandEvent& event )
{
	unsigned long		ruleid = 0;
	const PSEntry		*entry = NULL;
	wxCommandEvent		showEvent(anEVT_SHOW_RULE);

	entry = PSListCtrl::instance()->getEntry(psList->getFirstSelection());
	event.Skip();
	ruleid = entry->getSbRuleAdmin();

	if (ruleid == 0 ){
		sbAdminEditButton->Disable();
	} else {
		showEvent.SetInt(1); /* is Admin */
		showEvent.SetExtraLong(ruleid);
		wxPostEvent(AnEvents::instance(), showEvent);
	}
}

void
ModAnoubisMainPanelImpl::onCtxUserEditClicked( wxCommandEvent& event )
{
	unsigned long		ruleid = 0;
	const PSEntry		*entry = NULL;
	wxCommandEvent		showEvent(anEVT_SHOW_RULE);

	entry = PSListCtrl::instance()->getEntry(psList->getFirstSelection());
	event.Skip();
	ruleid = entry->getCtxRuleUser();

	if (ruleid == 0 ){
		ctxUserEditButton->Disable();
	} else {
		showEvent.SetInt(0); /* != Admin */
		showEvent.SetExtraLong((unsigned long)ruleid);
		wxPostEvent(AnEvents::instance(), showEvent);
	}
}

void
ModAnoubisMainPanelImpl::onCtxAdminEditClicked( wxCommandEvent& event )
{
	unsigned long		ruleid = 0;
	const PSEntry		*entry = NULL;
	wxCommandEvent		showEvent(anEVT_SHOW_RULE);

	entry = PSListCtrl::instance()->getEntry(psList->getFirstSelection());
	event.Skip();
	ruleid = entry->getCtxRuleAdmin();

	if (ruleid == 0 ){
		ctxAdminEditButton->Disable();
	} else {
		showEvent.SetInt(1); /* is Admin */
		showEvent.SetExtraLong(ruleid);
		wxPostEvent(AnEvents::instance(), showEvent);
	}
}

void
ModAnoubisMainPanelImpl::OnAllowBtnClick(wxCommandEvent &event)
{
	event.Skip();
	answerEscalation(true);
}

void
ModAnoubisMainPanelImpl::OnDenyBtnClick(wxCommandEvent &event)
{
	event.Skip();
	answerEscalation(false);
}

void
ModAnoubisMainPanelImpl::OnEscalationOnceButton(wxCommandEvent &event)
{
	event.Skip();
	setPathLabel();
	editOptionsEnable();
}

void
ModAnoubisMainPanelImpl::OnEscalationProcessButton(wxCommandEvent &event)
{
	event.Skip();
	setPathLabel();
	editOptionsEnable();
}

void
ModAnoubisMainPanelImpl::OnEscalationTimeoutButton(wxCommandEvent &event)
{
	event.Skip();
	setPathLabel();
	editOptionsEnable();
}

void
ModAnoubisMainPanelImpl::OnEscalationAlwaysButton(wxCommandEvent &event)
{
	event.Skip();
	setPathLabel();
	editOptionsEnable();
}

void
ModAnoubisMainPanelImpl::OnEscalationDisable(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/SendEscalations"),
	    cb_SendEscalations->IsChecked());
	setOptionsWidgetsVisibility();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnAlertDisable(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/SendAlerts"),
	    cb_SendAlerts->IsChecked());
	setOptionsWidgetsVisibility();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnEscalationNoTimeout(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/NoEscalationsTimeout"),
	    cb_NoEscalationTimeout->IsChecked());
	setOptionsWidgetsVisibility();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnAlertNoTimeout(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/NoAlertTimeout"),
	    cb_NoAlertTimeout->IsChecked());
	setOptionsWidgetsVisibility();
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnEscalationTimeout(wxSpinEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/EscalationTimeout"),
	    m_spinEscalationNotifyTimeout->GetValue());
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnEscalationTimeoutText(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/EscalationTimeout"),
	    m_spinEscalationNotifyTimeout->GetValue());
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnAlertTimeout(wxSpinEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/AlertTimeout"),
	    m_spinAlertNotifyTimeout->GetValue());
	sendNotifierOptions();
}

void
ModAnoubisMainPanelImpl::OnAlertTimeoutText(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/AlertTimeout"),
	    m_spinAlertNotifyTimeout->GetValue());
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
		wxPostEvent(AnEvents::instance(), showEvent);
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
ModAnoubisMainPanelImpl::OnEnableUpgradeMsg(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/ShowUpgradeMessage"),
	    cb_ShowUpgradeMsg->GetValue());
}

void
ModAnoubisMainPanelImpl::OnEnableKernelMsg(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/ShowKernelUpgradeMessage"),
	    cb_ShowKernelMsg->GetValue());
}

void
ModAnoubisMainPanelImpl::OnEnableInformationMsg(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/ShowCertMessage"),
	    cb_ShowKeyGenInfoMsg->GetValue());
}

void
ModAnoubisMainPanelImpl::OnDoAutostart(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/Autostart"),
	    cb_DoAutostart->IsChecked());
	MainUtils::instance()->autoStart(event.IsChecked());
}

void
ModAnoubisMainPanelImpl::OnAutoConnect(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/AutoConnect"),
	    autoConnectBox->IsChecked());
}

/*
 * NOTE: apparently wxNotebookEvents are propagated to the parent window,
 * NOTE: at least if there is no appropriate handler installed. This means
 * NOTE: that events that originate from the process browser notebook will
 * NOTE: end up here, too. Hence the check for the event object.
 */
void
ModAnoubisMainPanelImpl::OnNotebookTabChanged(wxNotebookEvent &event)
{
	event.Skip();
	if (event.GetEventObject() != tb_MainAnoubisNotify)
		return;
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
		wxPostEvent(AnEvents::instance(), showEvent);
	}
	/* You are only interested in the PsBrowser */
	if (tb_MainAnoubisNotify->GetPage(event.GetSelection()) ==
	    tb_PsBrowser) {
		/* Fetch only if connected */
		psList->clearSelection();
		if (JobCtrl::instance()->isConnected())
			PSListCtrl::instance()->updatePSList();
		else
			PSListCtrl::instance()->clearPSList();
	}
}

void
ModAnoubisMainPanelImpl::OnVersionListCtrlSelected(wxListEvent &event)
{
	int idx = versionListCtrl->getFirstSelection();

	event.Skip();
	if (idx == -1)
		return;

	VersionCtrl *versionCtrl = VersionCtrl::instance();

	/* Selected version */
	ApnVersion *version = versionCtrl->getVersion(idx);

	/* Update comment-field with selected version*/
	VersionShowCommentTextCtrl->SetValue(version->getComment());
}

void
ModAnoubisMainPanelImpl::OnVersionRestoreButtonClick(wxCommandEvent &event)
{
	VersionCtrl	*versionCtrl = VersionCtrl::instance();
	PolicyCtrl	*policyCtrl = PolicyCtrl::instance();
	bool		useActiveProfile;
	wxString	profile;
	int		idx = versionListCtrl->getFirstSelection();

	event.Skip();
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
ModAnoubisMainPanelImpl::OnVersionExportButtonClick(wxCommandEvent &event)
{
	int idx = versionListCtrl->getFirstSelection();

	event.Skip();
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

	VersionCtrl *versionCtrl = VersionCtrl::instance();
	if (!versionCtrl->exportVersion(idx, profile, fileDlg.GetPath())) {
		wxString msg = _("Failed to export a version!");
		wxString title = _("Export version");

		anMessageBox(msg, title, wxOK | wxICON_ERROR, this);
	}
}

void
ModAnoubisMainPanelImpl::OnVersionDeleteButtonClick(wxCommandEvent &event)
{
	VersionCtrl	*versionCtrl = VersionCtrl::instance();
	int		selection = -1;

	event.Skip();
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
ModAnoubisMainPanelImpl::OnVersionShowButtonClick(wxCommandEvent &event)
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

	event.Skip();
	/*VersionCtrl *versionCtrl = VersionCtrl::instance();

	wxCommandEvent showEvent(anEVT_RULEEDITOR_SHOW);
	showEvent.SetInt(1);
	wxPostEvent(AnEvents::instance(), showEvent);

	struct apn_ruleset *rs = versionCtrl->fetchRuleSet(idx);
	wxGetApp().importPolicyRuleSet(1, rs);*/
}

void
ModAnoubisMainPanelImpl::OnVersionActivePolicyClicked(wxCommandEvent &event)
{
	event.Skip();
	VersionProfileChoice->Enable(false);
	versionListUpdateFromSelection();
}

void
ModAnoubisMainPanelImpl::OnVersionProfilePolicyClicked(wxCommandEvent &event)
{
	event.Skip();
	VersionProfileChoice->Enable(true);
	versionListUpdateFromSelection();
}

void
ModAnoubisMainPanelImpl::OnVersionProfileChoice(wxCommandEvent &event)
{
	event.Skip();
	versionListUpdateFromSelection();
}


void
ModAnoubisMainPanelImpl::OnToolTipCheckBox(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/EnableToolTips"),
	    toolTipCheckBox->GetValue());
	toolTipParamsUpdate();
}

void
ModAnoubisMainPanelImpl::OnToolTipSpinCtrl(wxSpinEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/ToolTipTimeout"),
	    toolTipSpinCtrl->GetValue());
	toolTipParamsUpdate();
}

void
ModAnoubisMainPanelImpl::OnToolTipSpinCtrlText(wxCommandEvent &event)
{
	event.Skip();
	wxConfig::Get()->Write(wxT("/Options/ToolTipTimeout"),
	    toolTipSpinCtrl->GetValue());
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
ModAnoubisMainPanelImpl::OnEscalationSfsPathLeft(wxCommandEvent &event)
{
	event.Skip();
	pathKeep_--;
	setPathLabel();
}

void
ModAnoubisMainPanelImpl::OnEscalationSfsPathRight(wxCommandEvent &event)
{
	event.Skip();
	pathKeep_++;
	setPathLabel();
}

void
ModAnoubisMainPanelImpl::OnEscalationSbPathLeft(wxCommandEvent &event)
{
	event.Skip();
	pathKeep_--;
	setPathLabel();
}

void
ModAnoubisMainPanelImpl::OnEscalationSbPathRight(wxCommandEvent &event)
{
	event.Skip();
	pathKeep_++;
	setPathLabel();
}

void
ModAnoubisMainPanelImpl::OnPSListColumnButtonClick(wxCommandEvent& event)
{
	event.Skip();
	psList->showColumnVisibilityDialog();
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

void
ModAnoubisMainPanelImpl::onPsReloadClicked(wxCommandEvent &event)
{
	event.Skip();
	psList->clearSelection();
	PSListCtrl::instance()->updatePSList();
}

void
ModAnoubisMainPanelImpl::OnPSListItemSelected(wxListEvent &event)
{
	event.Skip();
	updatePSDetails();
}

void
ModAnoubisMainPanelImpl::OnPSListItemDeselected(wxListEvent &event)
{
	event.Skip();
	updatePSDetails();
}

void
ModAnoubisMainPanelImpl::OnPSListError(wxCommandEvent &event)
{
	const std::vector<wxString> &errors =
	    PSListCtrl::instance()->getErrors();
	event.Skip();

	for (unsigned int i=0; i<errors.size(); i++) {
		wxLogError(errors[i]);
	}

	if (errors.size() > 1) {
		wxLogError(_("Multiple errors occured while fetching the "
		    "process list"));
	}
	PSListCtrl::instance()->clearErrors();
}



/* Expects an empty rule set. */
static void
setRules(wxTextCtrl *ctrl, PolicyRuleSet *ruleset, unsigned int ruleid,
    int ruletype)
{
	struct apn_ruleset		*rs;
	struct apn_rule			*rule;
	AppPolicy			*app;

	if (ruleid == 0)
		return;
	if (ruleset == NULL || (rs = ruleset->getApnRuleSet()) == NULL)
		return;
	rule = apn_find_rule(rs, ruleid);
	if (rule == NULL || rule->apn_type != ruletype)
		return;
	app = dynamic_cast<AppPolicy *>((Policy *)rule->userdata);
	if (app == NULL)
		return;
	ctrl->AppendText(app->toString());
}

void
ModAnoubisMainPanelImpl::updatePSDetails(void)
{
	const PSEntry		*entry = NULL;
	wxString		 secure = wxT("");
	wxString		 pgidstr = wxT("");
	PolicyRuleSet		*admin, *user;
	PolicyCtrl		*pctrl = PolicyCtrl::instance();

	entry = PSListCtrl::instance()->getEntry(psList->getFirstSelection());
	/* Always clear the text controls and maybe fill them later. */
	psAlfUserPolicy->Clear();
	psAlfAdminPolicy->Clear();
	psSbUserPolicy->Clear();
	psSbAdminPolicy->Clear();
	psCtxUserPolicy->Clear();
	psCtxAdminPolicy->Clear();

	/* Enable edit-buttons only if you have an entry + corresponding rule */
	alfUserEditButton->Enable(entry != 0 && entry->getAlfRuleUser() != 0);
	alfAdminEditButton->Enable(entry != 0 && entry->getAlfRuleAdmin() != 0);
	sbUserEditButton->Enable(entry != 0 && entry->getSbRuleUser() != 0);
	sbAdminEditButton->Enable(entry != 0 && entry->getSbRuleAdmin() != 0);
	ctxUserEditButton->Enable(entry != 0 && entry->getCtxRuleUser() != 0);
	ctxAdminEditButton->Enable(entry != 0 && entry->getCtxRuleAdmin() != 0);

	if (entry) {
		uint64_t	pgid = entry->getPlaygroundId();

		secure = DefaultConversions::toYesNo(entry->getSecureExec());
		if (pgid == 0) {
			pgidstr = _("no");
		} else {
			pgidstr = wxString::Format(_("yes (ID: %llx)"),
			    (long long)pgid);
		}
	}
#define	S(NAME,METHOD) do {					\
	NAME->SetLabel(entry?entry->METHOD():wxT(""));		\
} while(0)
#define U(NAME,METHOD) do {					\
	NAME->SetLabel(entry?MainUtils::instance()->		\
	    getUserNameById(entry->METHOD()): wxT(""));		\
} while (0)
#define G(NAME,METHOD) do {                                     \
	NAME->SetLabel(entry?MainUtils::instance()->            \
	    getGroupNameById(entry->METHOD()): wxT(""));        \
} while (0)
	S(psDetailsCommandText, getLongProcessName);
	S(psDetailsPidText, getProcessId);
	S(psDetailsPpidText, getParentProcessId);
	U(psDetailsRealUidText, getUID);
	G(psDetailsRealGidText, getGID);
	U(psDetailsEffectiveUidText, getEUID);
	G(psDetailsEffectiveGidText, getEGID);
	psDetailsSecureExecText->SetLabel(secure);
	psDetailsPlaygroundText->SetLabel(pgidstr);
	S(psPathAppText, getPathProcess);
	S(psPathCsumText, getChecksumProcess);
	S(psPathUserCtxPathText, getPathUserContext);
	S(psPathUserCtxCsumText, getChecksumUserContext);
	S(psPathAdminCtxPathText, getPathAdminContext);
	S(psPathAdminCtxCsumText, getChecksumAdminContext);
#undef S
#undef U
#undef G

	if (entry == NULL)
		return;
	admin = pctrl->getRuleSet(pctrl->getAdminId(geteuid()));
	user = pctrl->getRuleSet(pctrl->getUserId());
	setRules(psAlfUserPolicy, user, entry->getAlfRuleUser(), APN_ALF);
	setRules(psAlfAdminPolicy, admin, entry->getAlfRuleAdmin(), APN_ALF);
	setRules(psSbUserPolicy, user, entry->getSbRuleUser(), APN_SB);
	setRules(psSbAdminPolicy, admin, entry->getSbRuleAdmin(), APN_SB);
	setRules(psCtxUserPolicy, user, entry->getCtxRuleUser(), APN_CTX);
	setRules(psCtxAdminPolicy, admin, entry->getCtxRuleAdmin(), APN_CTX);
}

bool
ModAnoubisMainPanelImpl::Show(bool show)
{
	bool result = ModAnoubisMainPanelBase::Show(show);

	/* If the PS-list is currently selected, you need to update the PSList
	 * now! */

	if (!show)
		return (result);

	int selected = tb_MainAnoubisNotify->GetSelection();

	if (selected == -1)
		return (result);

	wxWindow *selectedPage = tb_MainAnoubisNotify->GetPage(selected);

	if (selectedPage != tb_PsBrowser)
		return (result);

	/* Fetch only if connected */
	psList->clearSelection();
	if (JobCtrl::instance()->isConnected())
		PSListCtrl::instance()->updatePSList();
	else
		PSListCtrl::instance()->clearPSList();

	return (result);
}

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

/* XXX ch: for demo usage only */
#include <apnvm.h>
#include <wx/datetime.h>

#include "main.h"
#include "AnEvents.h"
#include "ModAnoubis.h"
#include "ModAnoubisMainPanelImpl.h"
#include "Notification.h"
#include "LogNotify.h"
#include "AlertNotify.h"
#include "EscalationNotify.h"
#include "NotifyAnswer.h"

#define ZERO_SECONDS	0
#define TEN_SECONDS	10

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
	list_ = NOTIFY_LIST_NOTANSWERED;
	currentNotify_ = NULL;
	userOptions_ = wxGetApp().getUserOptions();

	/* read and restore Escalations Settings */
	readOptions();

	/* get and show list of versions */
	updateVersionList();

	parent->Connect(anEVT_ESCALATIONS_SHOW,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnEscalationsShow),
	    NULL, this);
	parent->Connect(anEVT_ANOUBISOPTIONS_SHOW,
	    wxCommandEventHandler(ModAnoubisMainPanelImpl::OnAnoubisOptionShow),
	    NULL, this);
}

ModAnoubisMainPanelImpl::~ModAnoubisMainPanelImpl(void)
{
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
}

void
ModAnoubisMainPanelImpl::readOptions(void)
{
	bool SendEscalation = true;
	bool NoEscalationTimeout = true;
	int EscalationTimeout = 0;
	bool SendAlert = false;
	bool NoAlertTimeout = false;
	int AlertTimeout = 10;
	bool AutoChecksum = false;

	/* read the stored Notifications Options */
	userOptions_->Read(wxT("/Options/SendEscalations"), &SendEscalation);
	userOptions_->Read(wxT("/Options/NoEscalationsTimeout"),
	    &NoEscalationTimeout);
	userOptions_->Read(wxT("/Options/EscalationTimeout"),
	    &EscalationTimeout);

	userOptions_->Read(wxT("/Options/SendAlerts"), &SendAlert);
	userOptions_->Read(wxT("/Options/NoAlertTimeout"), &NoAlertTimeout);
	userOptions_->Read(wxT("/Options/AlertTimeout"), &AlertTimeout);

	userOptions_->Read(wxT("/Options/AutoChecksumCheck"), &AutoChecksum);

	/* restore the stored Notifications Options */
	cb_SendEscalations->SetValue(SendEscalation);
	cb_NoEscalationTimeout->SetValue(NoEscalationTimeout);
	m_spinEscalationNotifyTimeout->SetValue(EscalationTimeout);

	cb_SendAlerts->SetValue(SendAlert);
	cb_NoAlertTimeout->SetValue(NoAlertTimeout);
	m_spinAlertNotifyTimeout->SetValue(AlertTimeout);

	controlAutoCheck->SetValue(AutoChecksum);
	wxCommandEvent showEvent(anEVT_SEND_AUTO_CHECK);
	showEvent.SetInt(AutoChecksum);
	wxGetApp().sendEvent(showEvent);

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

/* XXX ch: this metohod (updateVersionList) is for demo usage only */
void
ModAnoubisMainPanelImpl::updateVersionList(void)
{
	apnvm		*vm;
	apnvm_result	 rc;
	wxString	 repository;
	wxString	 userName;

	struct apnvm_version		*version;
	struct apnvm_version_head	 version_head;

	/* Initialize table */
	columnNames_[MODANOUBIS_VMLIST_COLUMN_ID] = _("ID");
	columnNames_[MODANOUBIS_VMLIST_COLUMN_DATE] = _("Date");
	columnNames_[MODANOUBIS_VMLIST_COLUMN_TIME] = _("Time");

	for (int i=0; i<MODANOUBIS_VMLIST_COLUMN_EOL; i++) {
		VersionListCtrl->InsertColumn(i, columnNames_[i],
		    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);
	}
	VersionListCtrl->InsertItem(0, wxEmptyString);
	VersionListCtrl->SetItem(0, MODANOUBIS_VMLIST_COLUMN_DATE,
	    _("(Initializing version management...)"));

	/* XXX ch: don't use hard-coded path like this. */
	repository = wxGetApp().getDataDir() + wxT("/.xanoubis/apnvmroot");
	userName = wxGetUserId();
	 
	/* Initialize version management */
	vm = apnvm_init(repository.fn_str(), userName.fn_str());
	if (vm == NULL) {
		VersionListCtrl->SetItem(0, MODANOUBIS_VMLIST_COLUMN_DATE,
		    _("(Repository not initialized.)"));
		VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_DATE,
		    wxLIST_AUTOSIZE);
		return;
	}
	rc = apnvm_prepare(vm);
	if (rc != APNVM_OK) {
		VersionListCtrl->SetItem(0, MODANOUBIS_VMLIST_COLUMN_DATE,
		    _("(Repository not initialized.)"));
		VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_DATE,
		    wxLIST_AUTOSIZE);
		apnvm_destroy(vm);
		return;
	}
	TAILQ_INIT(&version_head);

	/* Claim versions */
	rc = apnvm_list(vm, userName.fn_str(), &version_head);
	if (rc != APNVM_OK) {
		VersionListCtrl->SetItem(0, MODANOUBIS_VMLIST_COLUMN_DATE,
		    _("(Repository not initialized.)"));
		VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_DATE,
		    wxLIST_AUTOSIZE);
		apnvm_destroy(vm);
		return;
	}

	if (TAILQ_EMPTY(&version_head)) {
		VersionListCtrl->SetItem(0, MODANOUBIS_VMLIST_COLUMN_DATE,
		    _("(No versions found.)"));
		VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_DATE,
		    wxLIST_AUTOSIZE);
		apnvm_destroy(vm);
		return;
	}

	VersionListCtrl->DeleteAllItems();
	TAILQ_FOREACH(version, &version_head, entries) {
		long		idx;
		wxDateTime	date(version->tstamp);

		idx = VersionListCtrl->GetItemCount();
		VersionListCtrl->InsertItem(idx, wxString::Format(wxT("%d"),
		    idx));
		VersionListCtrl->SetItem(idx, MODANOUBIS_VMLIST_COLUMN_DATE,
		    date.FormatDate());
		VersionListCtrl->SetItem(idx, MODANOUBIS_VMLIST_COLUMN_TIME,
		    date.FormatTime());
	}
	VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_ID,
	    wxLIST_AUTOSIZE_USEHEADER);
	VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_DATE,
	    wxLIST_AUTOSIZE);
	VersionListCtrl->SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_TIME,
	    wxLIST_AUTOSIZE);

	/* Clean version list */
	version = TAILQ_FIRST(&version_head);
	while (version != NULL) {
		TAILQ_REMOVE(&version_head, version, entries);
		free(version);
		version = TAILQ_FIRST(&version_head);
	}

	apnvm_destroy(vm);
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
	wxGetApp().sendEvent(showEvent);
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
	/* select tab "Options" */
	tb_MainAnoubisNotify->ChangeSelection(1);

	event.Skip();
}

void
ModAnoubisMainPanelImpl::OnAutoCheck(wxCommandEvent& event)
{
	wxCommandEvent showEvent(anEVT_SEND_AUTO_CHECK);
	showEvent.SetInt(event.IsChecked());
	wxGetApp().sendEvent(showEvent);
}

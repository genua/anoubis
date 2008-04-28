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

#include "main.h"
#include "AnEvents.h"
#include "ModAnoubis.h"
#include "ModAnoubisMainPanelImpl.h"
#include "Notification.h"
#include "LogNotify.h"
#include "AlertNotify.h"
#include "EscalationNotify.h"
#include "NotifyAnswer.h"

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
	systemNotifyEnabled_ = false;
	systemNotifyTimeout_ = TEN_SECONDS;
}

void
ModAnoubisMainPanelImpl::displayAlert(void)
{
	AlertNotify	*aNotify;

	aNotify = (AlertNotify *)currentNotify_;

	SHOWSLOT(1, wxT("Time:"), aNotify->getTime());
	SHOWSLOT(2, wxT("Module:"), aNotify->getModule());
	if (aNotify->getModule().Cmp(wxT("ALF")) == 0) {
		SHOWSLOT(3, wxT("Traffic:"), aNotify->getPath());
	} else {
		SHOWSLOT(3, wxT("Path:"), aNotify->getPath());
	}
	SHOWSLOT(4, wxT("Operation:"), aNotify->getOperation());
	SHOWSLOT(5, wxT("Origin:"), aNotify->getOrigin());
	SHOWSLOT(6, wxT("Checksum:"), aNotify->getCheckSum());
}

void
ModAnoubisMainPanelImpl::displayEscalation(void)
{
	EscalationNotify	*eNotify;

	eNotify = (EscalationNotify *)currentNotify_;

	SHOWSLOT(1, wxT("Time:"), eNotify->getTime());
	SHOWSLOT(2, wxT("Module:"), eNotify->getModule());
	if (eNotify->getModule().Cmp(wxT("ALF")) == 0) {
		SHOWSLOT(3, wxT("Traffic:"), eNotify->getPath());
	} else {
		SHOWSLOT(3, wxT("Path:"), eNotify->getPath());
	}
	SHOWSLOT(4, wxT("Operation:"), eNotify->getOperation());
	SHOWSLOT(5, wxT("Origin:"), eNotify->getOrigin());
	SHOWSLOT(6, wxT("Checksum:"), eNotify->getCheckSum());
}

void
ModAnoubisMainPanelImpl::displayLog(void)
{
	LogNotify	*lNotify;

	lNotify = (LogNotify *)currentNotify_;

	SHOWSLOT(1, wxT("Time:"), lNotify->getTime());
	SHOWSLOT(2, wxT("Module:"), lNotify->getModule());
	if (lNotify->getModule().Cmp(wxT("ALF")) == 0) {
		SHOWSLOT(3, wxT("Traffic:"), lNotify->getPath());
	} else {
		SHOWSLOT(3, wxT("Path:"), lNotify->getPath());
	}
	SHOWSLOT(4, wxT("Operation:"), lNotify->getOperation());
	SHOWSLOT(5, wxT("Origin:"), lNotify->getOrigin());
	SHOWSLOT(6, wxT("Checksum:"), lNotify->getCheckSum());
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

	s.Printf(_T("%d"), maxElementNo);
	tx_maxNumber->SetLabel(s);
	if (maxElementNo > 0)
		s.Printf(_T("%d"), elementNo + 1);
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
ModAnoubisMainPanelImpl::OnFirstBtnClick(wxCommandEvent& event)
{
	ModAnoubis *module;

	module = (ModAnoubis *)(wxGetApp().getModule(ANOUBIS));
	currentNotify_ = module->getFirst(list_);

	update();
}

void
ModAnoubisMainPanelImpl::OnPreviousBtnClick(wxCommandEvent& event)
{
	ModAnoubis *module;

	module = (ModAnoubis *)(wxGetApp().getModule(ANOUBIS));
	currentNotify_ = module->getPrevious(list_);

	update();
}

void
ModAnoubisMainPanelImpl::OnNextBtnClick(wxCommandEvent& event)
{
	ModAnoubis *module;

	module = (ModAnoubis *)(wxGetApp().getModule(ANOUBIS));
	currentNotify_ = module->getNext(list_);

	update();
}

void
ModAnoubisMainPanelImpl::OnLastBtnClick(wxCommandEvent& event)
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
			answer = new NotifyAnswer(NOTIFY_ANSWER_COUNT,
			    permission, sc_number->GetValue());
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
ModAnoubisMainPanelImpl::OnAllowBtnClick(wxCommandEvent& event)
{
	answer(true);
}

void
ModAnoubisMainPanelImpl::OnDenyBtnClick(wxCommandEvent& event)
{
	answer(false);
}

void
ModAnoubisMainPanelImpl::OnToggleNotification(wxCommandEvent& event)
{
	if(event.IsChecked()) {
		systemNotifyEnabled_ = true;
	} else {
		systemNotifyEnabled_ = false;
	}

	wxCommandEvent  showEvent(anEVT_SYSNOTIFICATION_OPTIONS);
	showEvent.SetInt(systemNotifyEnabled_);
	showEvent.SetExtraLong(systemNotifyTimeout_);
	wxGetApp().sendEvent(showEvent);
}

void
ModAnoubisMainPanelImpl::OnNotificationTimeout(wxSpinEvent& event)
{
	systemNotifyTimeout_ = event.GetPosition();

	wxCommandEvent  showEvent(anEVT_SYSNOTIFICATION_OPTIONS);
	showEvent.SetInt(systemNotifyEnabled_);
	showEvent.SetExtraLong(systemNotifyTimeout_);
	wxGetApp().sendEvent(showEvent);
}

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
#include "ModAnoubis.h"
#include "ModAnoubisMainPanelImpl.h"
#include "NotifyList.h"
#include "NotifyAnswer.h"

#define SHOWSLOT(slotNo,field,value) \
	do { \
		tx_fieldSlot##slotNo->Show(); \
		tx_fieldSlot##slotNo->SetLabel(field); \
		tx_valueSlot##slotNo->Show(); \
		tx_valueSlot##slotNo->SetLabel(value); \
	} while (0)

#define HIDESLOT(slotNo) \
	do { \
		tx_fieldSlot##slotNo->Hide(); \
		tx_valueSlot##slotNo->Hide(); \
	} while (0)

ModAnoubisMainPanelImpl::ModAnoubisMainPanelImpl(wxWindow* parent,
    wxWindowID id) : ModAnoubisMainPanelBase(parent, id)
{
	list_ = NOTIFY_LIST_NOTANSWERED;
	currentNotify_ = NULL;
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
		s.Printf(_T("%d"), currentNotify_->getId());
		SHOWSLOT(1,wxT("Id:"),s);
		SHOWSLOT(2,wxT("Modul:"),currentNotify_->getModule());
		if (!currentNotify_->getModule().Cmp(wxT("ALF"))) {
			SHOWSLOT(3,wxT("Transport:"),
			    currentNotify_->getAlfTransport());
			SHOWSLOT(4,wxT("Operation:"),
			    currentNotify_->getAlfOp());
			SHOWSLOT(5,wxT("PID / UID:"),
			    currentNotify_->getAlfWho());
		}
		if (!currentNotify_->getModule().Cmp(wxT("SFS"))) {
			SHOWSLOT(3,wxT("Checksum:"),
			    currentNotify_->getSfsChkSum());
			SHOWSLOT(4,wxT("Operation:"),
			    currentNotify_->getSfsOp());
			SHOWSLOT(5,wxT("Path:"),currentNotify_->getSfsPath());
		}
	} else {
		HIDESLOT(1);
		HIDESLOT(2);
		HIDESLOT(3);
		HIDESLOT(4);
		HIDESLOT(5);
	}

	if ((currentNotify_ != NULL) &&
	    currentNotify_->isAnswerAble() &&
	    !currentNotify_->isAnswered()    ) {
		pn_question->Show();
		tx_answerValue->Hide();
	} else if ((currentNotify_ != NULL) &&
	    currentNotify_->isAnswerAble() &&
	    currentNotify_->isAnswered()     ) {
		pn_question->Hide();
		tx_answerValue->SetLabel(
		    currentNotify_->getAnswer()->getAnswer());
		tx_answerValue->Show();
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
ModAnoubisMainPanelImpl::answer(bool allow)
{
	ModAnoubis	*module;
	NotifyAnswer	*answer;

	module = (ModAnoubis *)(wxGetApp().getModule(ANOUBIS));

	if (rb_number->GetValue()) {
		answer = new NotifyAnswer(NOTIFY_ANSWER_COUNT, allow,
		    sc_number->GetValue());
	} else if (rb_procend->GetValue()) {
		answer = new NotifyAnswer(NOTIFY_ANSWER_PROCEND, allow);
	} else if (rb_time->GetValue()) {
		answer = new NotifyAnswer(NOTIFY_ANSWER_TIME, allow,
		    sc_time->GetValue(),
		    (enum timeUnit)ch_time->GetCurrentSelection());
	} else if (rb_always->GetValue()) {
		answer = new NotifyAnswer(NOTIFY_ANSWER_FOREVER, allow);
	} else {
		answer = new NotifyAnswer(NOTIFY_ANSWER_NONE, allow);
	}

	module->answerNotification(currentNotify_, answer);
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

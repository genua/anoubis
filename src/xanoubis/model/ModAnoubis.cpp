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

#include <wx/stdpaths.h>
#include <wx/string.h>

#include "AnEvents.h"
#include "Notification.h"
#include "EscalationNotify.h"
#include "AlertNotify.h"
#include "LogNotify.h"
#include "NotifyAnswer.h"
#include "main.h"
#include "Module.h"
#include "ModAnoubis.h"
#include "ModAnoubisMainPanelImpl.h"
#include "ModAnoubisOverviewPanelImpl.h"

#define ITERATION_NEXT(list,idx,res) \
	do { \
		if (idx < list.GetCount() - 1) \
			idx++; \
		res = list.Item(idx); \
	} while (0)

#define ITERATION_PREVIOUS(list,idx,res) \
	do { \
		if (idx > 0) \
			idx--; \
		res = list.Item(idx); \
	} while (0)

#define ITERATION_FIRST(list,idx,res) \
	do { \
		idx = 0; \
		res = list.Item(idx); \
	} while (0)

#define ITERATION_LAST(list,idx,res) \
	do { \
		idx = list.GetCount() - 1; \
		res = list.Item(idx); \
	} while (0)

ModAnoubis::ModAnoubis(wxWindow *parent) : Module()
{
	name_ = wxString(_("Anoubis"));
	nick_ = wxString(_("Anoubis"));
	mainPanel_ = new ModAnoubisMainPanelImpl(parent,
	    MODANOUBIS_ID_MAINPANEL);
	overviewPanel_ = new ModAnoubisOverviewPanelImpl(parent,
	    MODANOUBIS_ID_OVERVIEWPANEL);

	loadIcon(_T("ModAnoubis_black_48.png"));
	mainPanel_->Hide();
	overviewPanel_->Hide();

	notAnsweredListIdx_ = 0;
	alertListIdx_ = 0;
	logListIdx_ = 0;
	answeredListIdx_ = 0;
	allListIdx_ = 0;

	parent->Connect(anEVT_ADD_NOTIFICATION,
	    wxCommandEventHandler(ModAnoubis::OnAddNotification), NULL, this);
}

ModAnoubis::~ModAnoubis(void)
{
	notAnsweredList_.DeleteContents(true);
	alertList_.DeleteContents(true);
	logList_.DeleteContents(true);
	answeredList_.DeleteContents(true);
	allList_.DeleteContents(true);
	delete mainPanel_;
	delete overviewPanel_;
	delete icon_;
}

int
ModAnoubis::getBaseId(void)
{
	return (MODANOUBIS_ID_BASE);
}

int
ModAnoubis::getToolbarId(void)
{
	return (MODANOUBIS_ID_TOOLBAR);
}

void
ModAnoubis::update(void)
{
}

void
ModAnoubis::OnAddNotification(wxCommandEvent& event)
{
	Notification *notify;

	notify = (Notification *)(event.GetClientObject());
	insertNotification(notify);
	event.Skip();
}

void
ModAnoubis::insertNotification(Notification *newNotify)
{
	if (IS_ESCALATIONOBJ(newNotify)) {
		notAnsweredList_.Append(newNotify);
		allList_.Append(newNotify);
		wxCommandEvent  showEvent(anEVT_OPEN_ESCALATIONS);
		showEvent.SetInt(notAnsweredList_.GetCount());
		wxGetApp().sendEvent(showEvent);
	} else if (IS_ALERTOBJ(newNotify)) {
		alertList_.Append(newNotify);
		allList_.Append(newNotify);
		wxCommandEvent  showEvent(anEVT_OPEN_ALERTS);
		showEvent.SetInt(alertList_.GetCount());
		wxGetApp().sendEvent(showEvent);
	} else {
		logList_.Append(newNotify);
		/*
		 * Normal logs are hidden in ModAnoubis, thus we don't add
		 * them to the  list of all notifies.
		 */
	}
	((ModAnoubisMainPanelImpl *)mainPanel_)->update();
}

void
ModAnoubis::answerEscalationNotify(EscalationNotify *notify,
    NotifyAnswer *answer)
{
	if ((notify != NULL) && IS_ESCALATIONOBJ(notify)) {
		notAnsweredList_.DeleteObject(notify);
		answeredList_.Append(notify);
		notify->answer(answer);
	}
}

size_t
ModAnoubis::getElementNo(enum notifyListTypes listType)
{
	size_t elementNo;

	switch (listType) {
	case NOTIFY_LIST_NOTANSWERED:
		elementNo = notAnsweredListIdx_;
		break;
	case NOTIFY_LIST_MESSAGE:
		elementNo = alertListIdx_;
		break;
	case NOTIFY_LIST_ANSWERED:
		elementNo = answeredListIdx_;
		break;
	case NOTIFY_LIST_NONE:
		/* FALLTHROUGH */
	case NOTIFY_LIST_ALL:
		/* FALLTHROUGH */
	default:
		elementNo = allListIdx_;
		break;
	}

	return (elementNo);
}

size_t
ModAnoubis::getListSize(enum notifyListTypes listType)
{
	size_t listSize;

	switch (listType) {
	case NOTIFY_LIST_NOTANSWERED:
		listSize = notAnsweredList_.GetCount();
		break;
	case NOTIFY_LIST_MESSAGE:
		listSize = alertList_.GetCount();
		break;
	case NOTIFY_LIST_ANSWERED:
		listSize = answeredList_.GetCount();
		break;
	case NOTIFY_LIST_NONE:
		/* FALLTHROUGH */
	case NOTIFY_LIST_ALL:
		/* FALLTHROUGH */
	default:
		listSize = allList_.GetCount();
		break;
	}

	return (listSize);
}

Notification *
ModAnoubis::getFirst(enum notifyListTypes listType)
{
	wxNotifyListNode *element;

	switch (listType) {
	case NOTIFY_LIST_NOTANSWERED:
		ITERATION_FIRST(notAnsweredList_,notAnsweredListIdx_,element);
		break;
	case NOTIFY_LIST_MESSAGE:
		ITERATION_FIRST(alertList_,alertListIdx_,element);
		break;
	case NOTIFY_LIST_ANSWERED:
		ITERATION_FIRST(answeredList_,answeredListIdx_,element);
		break;
	case NOTIFY_LIST_NONE:
		/* FALLTHROUGH */
	case NOTIFY_LIST_ALL:
		/* FALLTHROUGH */
	default:
		ITERATION_FIRST(allList_,allListIdx_,element);
		break;
	}

	return (element->GetData());
}

Notification *
ModAnoubis::getPrevious(enum notifyListTypes listType)
{
	wxNotifyListNode *element;

	switch (listType) {
	case NOTIFY_LIST_NOTANSWERED:
		ITERATION_PREVIOUS(notAnsweredList_,notAnsweredListIdx_,\
		    element);
		break;
	case NOTIFY_LIST_MESSAGE:
		ITERATION_PREVIOUS(alertList_,alertListIdx_,element);
		break;
	case NOTIFY_LIST_ANSWERED:
		ITERATION_PREVIOUS(answeredList_,answeredListIdx_,element);
		break;
	case NOTIFY_LIST_NONE:
		/* FALLTHROUGH */
	case NOTIFY_LIST_ALL:
		/* FALLTHROUGH */
	default:
		ITERATION_PREVIOUS(allList_,allListIdx_,element);
		break;
	}

	return (element->GetData());
}

Notification *
ModAnoubis::getNext(enum notifyListTypes listType)
{
	wxNotifyListNode *element;

	switch (listType) {
	case NOTIFY_LIST_NOTANSWERED:
		ITERATION_NEXT(notAnsweredList_,notAnsweredListIdx_,element);
		break;
	case NOTIFY_LIST_MESSAGE:
		ITERATION_NEXT(alertList_,alertListIdx_,element);
		break;
	case NOTIFY_LIST_ANSWERED:
		ITERATION_NEXT(answeredList_,answeredListIdx_,element);
		break;
	case NOTIFY_LIST_NONE:
		/* FALLTHROUGH */
	case NOTIFY_LIST_ALL:
		/* FALLTHROUGH */
	default:
		ITERATION_NEXT(allList_,allListIdx_,element);
		break;
	}

	return (element->GetData());
}

Notification *
ModAnoubis::getLast(enum notifyListTypes listType)
{
	wxNotifyListNode *element;

	switch (listType) {
	case NOTIFY_LIST_NOTANSWERED:
		ITERATION_LAST(notAnsweredList_,notAnsweredListIdx_,element);
		break;
	case NOTIFY_LIST_MESSAGE:
		ITERATION_LAST(alertList_,alertListIdx_,element);
		break;
	case NOTIFY_LIST_ANSWERED:
		ITERATION_LAST(answeredList_,answeredListIdx_,element);
		break;
	case NOTIFY_LIST_NONE:
		/* FALLTHROUGH */
	case NOTIFY_LIST_ALL:
		/* FALLTHROUGH */
	default:
		ITERATION_LAST(allList_,allListIdx_,element);
		break;
	}

	return (element->GetData());
}

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
#include "DaemonAnswerNotify.h"
#include "main.h"
#include "Module.h"
#include "ModAnoubis.h"
#include "ModAnoubisMainPanelImpl.h"
#include "ModAnoubisOverviewPanelImpl.h"
#include "JobCtrl.h"

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
	name_ = wxString(wxT("Anoubis"));
	nick_ = wxString(wxT("Anoubis"));
	overviewPanel_ = new ModAnoubisOverviewPanelImpl(parent,
	    MODANOUBIS_ID_OVERVIEWPANEL);
	mainPanel_ = new ModAnoubisMainPanelImpl(parent,
	    MODANOUBIS_ID_MAINPANEL);

	loadIcon(wxT("ModAnoubis_black_48.png"));
	mainPanel_->Hide();
	overviewPanel_->Hide();

	notAnsweredListIdx_ = 0;
	notAnsweredAlf_ = 0;
	notAnsweredSfs_ = 0;
	notAnsweredSb_ = 0;
	alertListIdx_ = 0;
	logListIdx_ = 0;
	answeredListIdx_ = 0;
	allListIdx_ = 0;

	AnEvents::getInstance()->Connect(anEVT_ADD_NOTIFICATION,
	    wxCommandEventHandler(ModAnoubis::OnAddNotification), NULL, this);
	JobCtrl::getInstance()->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(ModAnoubis::OnDaemonDisconnect), NULL, this);
}

ModAnoubis::~ModAnoubis(void)
{
	AnEvents::getInstance()->Disconnect(anEVT_ADD_NOTIFICATION,
	    wxCommandEventHandler(ModAnoubis::OnAddNotification), NULL, this);

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
	Notification	*notify;

	notify = (Notification *)(event.GetClientObject());
	insertNotification(notify);
	/*
	 * A DaemonAnswerNotify is no longer needed. Do not skip it.
	 */
	if (IS_DAEMONANSWEROBJ(notify)) {
		delete notify;
	} else {
		event.Skip();
	}
}

void
ModAnoubis::OnDaemonDisconnect(wxCommandEvent& event)
{
	NotifyAnswer		*answer;
	EscalationNotify	*eNotify;

	event.Skip();
	if (event.GetInt() != JobCtrl::CONNECTION_CONNECTED) {
		NotifyList::iterator	it;
		while(1) {
			it = notAnsweredList_.begin();
			if (it == notAnsweredList_.end())
				break;
			eNotify = dynamic_cast<EscalationNotify *>(*it);
			if (!eNotify) {
				/* Should never happen, but let's play safe. */
				notAnsweredList_.DeleteObject(*it);
			} else {
				answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE,
				    false);
				answerEscalationNotify(eNotify, answer, false);
			}
		}
	}
}

void
ModAnoubis::insertNotification(Notification *newNotify)
{
	wxString	 module;
	AnEvents	*anEvents;
	ProfileCtrl	*profileCtrl;
	PolicyRuleSet	*rs = NULL;

	anEvents = AnEvents::getInstance();

	if (IS_ESCALATIONOBJ(newNotify)) {
		EscalationNotify	*eNotify;

		profileCtrl = ProfileCtrl::getInstance();
		if (geteuid() == newNotify->getUid()) {
			if (newNotify->isAdmin()) {
				rs = profileCtrl->getRuleSet(
				    profileCtrl->getAdminId(geteuid()));
			} else {
				rs = profileCtrl->getRuleSet(
				    profileCtrl->getUserId());
			}
		}
		eNotify = dynamic_cast<EscalationNotify *>(newNotify);
		if (eNotify && rs) {
			rs->addRuleInformation(eNotify);
		}
		notAnsweredList_.Append(newNotify);
		allList_.Append(newNotify);
		module = newNotify->getModule();
		if(module.IsSameAs(wxT("ALF"))) {
			notAnsweredAlf_++;
			wxCommandEvent showEvent(anEVT_OPEN_ALF_ESCALATIONS);
			showEvent.SetInt(notAnsweredAlf_);
			wxPostEvent(anEvents, showEvent);
		} else if(module.IsSameAs(wxT("SFS"))) {
			notAnsweredSfs_++;
			wxCommandEvent showEvent(anEVT_OPEN_SFS_ESCALATIONS);
			showEvent.SetInt(notAnsweredSfs_);
			wxPostEvent(anEvents, showEvent);
		} else if (module.IsSameAs(wxT("SANDBOX"))) {
			notAnsweredSb_++;
			wxCommandEvent showEvent(anEVT_OPEN_SB_ESCALATIONS);
			showEvent.SetInt(notAnsweredSb_);
			wxPostEvent(anEvents, showEvent);
		} else {
			/* Default do nothing */
		}
		wxCommandEvent  showEvent(anEVT_OPEN_ESCALATIONS);
		showEvent.SetInt(notAnsweredList_.GetCount());
		wxPostEvent(anEvents, showEvent);
	} else if (IS_DAEMONANSWEROBJ(newNotify)) {
		DaemonAnswerNotify	*dNotify;
		EscalationNotify	*origNotify;

		dNotify = dynamic_cast<DaemonAnswerNotify *>(newNotify);
		origNotify = fixupEscalationAnswer(dNotify->getType(),
		    dNotify->getToken(), dNotify->getError());
		if (origNotify) {
			wxCommandEvent	event(anEVT_ADD_NOTIFYANSWER);
			event.SetClientObject((wxClientData*)origNotify);
			wxPostEvent(anEvents, event);
		}
	} else if (IS_ALERTOBJ(newNotify)) {
		alertList_.Append(newNotify);
		allList_.Append(newNotify);
		wxCommandEvent  showEvent(anEVT_OPEN_ALERTS);
		showEvent.SetInt(alertList_.GetCount());
		wxPostEvent(anEvents, showEvent);
	} else {
		logList_.Append(newNotify);
		/*
		 * Normal logs are hidden in ModAnoubis, thus we don't add
		 * them to the  list of all notifies.
		 */
	}
	((ModAnoubisMainPanelImpl *)mainPanel_)->update();
}

EscalationNotify *
ModAnoubis::fixupEscalationAnswer(int type, anoubis_token_t token, int error)
{
	NotifyList::iterator	 it;
	NotifyAnswer		*answer;
	bool			 allow = (error == 0);
	EscalationNotify	*eNotify;

	if (type == ANOUBIS_N_RESOTHER) {
		/* Search it in the list of unanswered requests. */
		for (it = notAnsweredList_.begin();
		    it != notAnsweredList_.end(); it++) {
			eNotify = dynamic_cast<EscalationNotify *>(*it);
			if (eNotify && eNotify->getToken() == token) {
				answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE,
				    allow);
				answerEscalationNotify(eNotify, answer);
				return eNotify;
			}
		}
	}
	/*
	 * RESYOU or not a reply to an unanswered request. Find the request
	 * in the list of answerd requests. Search backwards because the
	 * request is likely to be at the end of the list and the list can
	 * get long over time.
	 */
	it = answeredList_.end();
	while(1) {
		if (it == answeredList_.begin())
			break;
		it--;
		eNotify = dynamic_cast<EscalationNotify *>(*it);
		if (eNotify && eNotify->getToken() == token) {
			answer = eNotify->getAnswer();
			if (answer->wasAllowed() != allow) {
				eNotify->setAnswer(NULL);
				delete answer;
				answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE,
				    allow);
				eNotify->setAnswer(answer);
			}
			return eNotify;
		}
	}
	return NULL;
}

void
ModAnoubis::answerEscalationNotify(EscalationNotify *notify,
    NotifyAnswer *answer, bool sendAnswer)
{
	wxString	 module;
	AnEvents	*anEvents;

	anEvents = AnEvents::getInstance();

	if ((notify != NULL) && IS_ESCALATIONOBJ(notify)) {
		notAnsweredList_.DeleteObject(notify);
		answeredList_.Append(notify);
		if (sendAnswer) {
			notify->answer(answer);
		} else {
			notify->setAnswer(answer);
		}
		module = notify->getModule();
		if (module.IsSameAs(wxT("ALF"))){
			notAnsweredAlf_--;
			wxCommandEvent showEvent(anEVT_OPEN_ALF_ESCALATIONS);
			showEvent.SetInt(notAnsweredAlf_);
			wxPostEvent(anEvents, showEvent);
		} else if (module.IsSameAs(wxT("SFS"))) {
			notAnsweredSfs_--;
			wxCommandEvent showEvent(anEVT_OPEN_SFS_ESCALATIONS);
			showEvent.SetInt(notAnsweredSfs_);
			wxPostEvent(anEvents, showEvent);
		} else if (module.IsSameAs(wxT("SANDBOX"))) {
			notAnsweredSb_--;
			wxCommandEvent showEvent(anEVT_OPEN_SB_ESCALATIONS);
			showEvent.SetInt(notAnsweredSb_);
			wxPostEvent(anEvents, showEvent);
		} else {
			/* Default do nothing */
		}
		wxCommandEvent  showEvent(anEVT_OPEN_ESCALATIONS);
		showEvent.SetInt(notAnsweredList_.GetCount());
		wxPostEvent(anEvents, showEvent);
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

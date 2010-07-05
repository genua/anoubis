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

#include "AlertNotify.h"
#include "DaemonAnswerNotify.h"
#include "EscalationNotify.h"
#include "JobCtrl.h"
#include "LogNotify.h"
#include "MainUtils.h"
#include "NotificationCtrl.h"
#include "PolicyCtrl.h"
#include "PolicyRuleSet.h"
#include "Singleton.cpp"
#include "StatusNotify.h"

NotificationCtrl::~NotificationCtrl(void)
{
	Disconnect(anEVT_UPDATE_PERSPECTIVE, wxCommandEventHandler(
	    NotificationCtrl::onUpdatePerspectives), NULL, this);
	JobCtrl::getInstance()->Disconnect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(NotificationCtrl::onDaemonDisconnect),
	    NULL, this);

	notificationHash_.clear();
}

void
NotificationCtrl::addNotification(Notification *notification)
{
	long		id;
	wxCommandEvent	updateEvent(anEVT_UPDATE_PERSPECTIVE);

	id = notification->getId();

	notificationHashMutex_.Lock();
	notificationHash_[id] = notification;
	notificationHashMutex_.Unlock();

	/*
	 * We need to update the perspectives as well. But we can't
	 * do it right here, because it's possible to be within the
	 * ComThread and not in the GuiMainThread.
	 * Thus we delay this by sending an event to ourself.
	 * See Bug #1298 and
	 * http://stackoverflow.com/questions/407004/notifications-in-wxwidgets
	 */
	updateEvent.SetExtraLong(id);

	if (wxThread::This()->IsMain()) {
		/* You are in the main-thread, just process the event */
		ProcessEvent(updateEvent);
	} else {
		/*
		 * You are not in the main-thread, schedule event for later
		 * delivery.
		 */
		wxPostEvent(this, updateEvent);
	}
}

Notification *
NotificationCtrl::getNotification(long id)
{
	NotifyHash::const_iterator it;

	notificationHashMutex_.Lock();
	it = notificationHash_.find(id);
	notificationHashMutex_.Unlock();

	if (it != notificationHash_.end()) {
		return ((*it).second);
	}

	return (NULL);
}

NotificationPerspective *
NotificationCtrl::getPerspective(enum ListPerspectives list)
{
	switch (list) {
	case LIST_NONE:
		return (NULL);
		break;
	case LIST_NOTANSWERED:
		return (&escalationsNotAnswered_);
		break;
	case LIST_ANSWERED:
		return (&escalationsAnswered_);
		break;
	case LIST_MESSAGES:
		return (&messages_);
		break;
	case LIST_STAT:
		return (&stats_);
		break;
	case LIST_ALL:
		return (&allNotifications_);
		break;
	}

	return (NULL);
}

void
NotificationCtrl::answerEscalationNotify(EscalationNotify *notify,
    NotifyAnswer *answer, bool sendAnswer)
{
	long		 id;
	wxString	 module;
	AnEvents	*anEvents;

	anEvents = AnEvents::getInstance();

	if ((notify != NULL) && IS_ESCALATIONOBJ(notify)) {
		id = notify->getId();
		escalationsNotAnswered_.removeId(id);
		escalationsAnswered_.addId(id);
		if (sendAnswer) {
			notify->answer(answer);
		} else {
			notify->setAnswer(answer);
		}

		module = notify->getModule();
		escalationCount_[module] -= 1;

		if (module.IsSameAs(wxT("ALF"))){
			wxCommandEvent showEvent(anEVT_OPEN_ALF_ESCALATIONS);
			showEvent.SetInt(escalationCount_[module]);
			wxPostEvent(anEvents, showEvent);
		} else if (module.IsSameAs(wxT("SFS"))) {
			wxCommandEvent showEvent(anEVT_OPEN_SFS_ESCALATIONS);
			showEvent.SetInt(escalationCount_[module]);
			wxPostEvent(anEvents, showEvent);
		} else if (module.IsSameAs(wxT("SANDBOX"))) {
			wxCommandEvent showEvent(anEVT_OPEN_SB_ESCALATIONS);
			showEvent.SetInt(escalationCount_[module]);
			wxPostEvent(anEvents, showEvent);
		} else {
			/* Default do nothing */
		}
		wxCommandEvent  showEvent(anEVT_OPEN_ESCALATIONS);
		showEvent.SetInt(escalationsNotAnswered_.getSize());
		showEvent.SetExtraLong(0);
		wxPostEvent(anEvents, showEvent);
	}
}


NotificationCtrl::NotificationCtrl(void) : Singleton<NotificationCtrl>()
{
	notificationHash_.clear();

	JobCtrl::getInstance()->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(NotificationCtrl::onDaemonDisconnect),
	    NULL, this);
	Connect(anEVT_UPDATE_PERSPECTIVE, wxCommandEventHandler(
	    NotificationCtrl::onUpdatePerspectives), NULL, this);
}

void
NotificationCtrl::onDaemonDisconnect(wxCommandEvent & event)
{
	NotifyAnswer		*answer;
	Notification		*notify;
	EscalationNotify	*escalation;

	event.Skip();

	if (event.GetInt() != JobCtrl::CONNECTED) {
		while (escalationsNotAnswered_.getSize() > 0) {
			notify = getNotification(
				escalationsNotAnswered_.getId(0)
			);
			escalation = dynamic_cast<EscalationNotify *>(notify);
			if (escalation == NULL) {
				/* Should never happen, but let's play safe. */
				escalationsNotAnswered_.removeId(
					escalationsNotAnswered_.getId(0)
				);
			} else {
				answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE,
				    false);
				answerEscalationNotify(escalation, answer,
				    false);
			}
		}
	}

	/* remove the old status messages */
	while (stats_.getSize() > 0) {
		stats_.removeId(stats_.getId(0));
	}
	//XXX ch: fix this while cleaning MVC-pattern
	if (MainUtils::instance()->getModule(ALF))
		MainUtils::instance()->getModule(ALF)->update();
	if (MainUtils::instance()->getModule(SFS))
		MainUtils::instance()->getModule(SFS)->update();
	if (MainUtils::instance()->getModule(SB))
		MainUtils::instance()->getModule(SB)->update();
	if (MainUtils::instance()->getModule(ANOUBIS))
		MainUtils::instance()->getModule(ANOUBIS)->update();
}

void
NotificationCtrl::onUpdatePerspectives(wxCommandEvent & event)
{
	long		 id;
	Notification	*notification;

	id =  event.GetExtraLong();
	notification = getNotification(id);

	if (notification == NULL) {
		/* This is strange and should never happen. */
		return;
	}

	/* sorted enqueueing of notifications */
	if (IS_ESCALATIONOBJ(notification)) {
		addEscalationNotify(notification);
	} else if (IS_DAEMONANSWEROBJ(notification)) {
		addDaemonAnswerNotify(notification);
	} else if (IS_ALERTOBJ(notification)) {
		addAlertNotify(notification);
	} else if (IS_STATUSOBJ(notification)) {
		/* remove the old status message */
		if (stats_.getSize() > 0) {
			stats_.removeId(stats_.getId(0));
		}
		stats_.addId(id);
	} else {
		/* Imho we shouldn't reach this point. */
		allNotifications_.addId(id);
	}
}

void
NotificationCtrl::addEscalationNotify(Notification *notification)
{
	wxString		 module;
	AnEvents		*anEvents;
	PolicyCtrl		*policyCtrl;
	PolicyRuleSet		*rs;
	EscalationNotify	*escalation;

	module = wxEmptyString;
	anEvents = AnEvents::getInstance();
	policyCtrl = PolicyCtrl::getInstance();
	rs = NULL;
	escalation = NULL;

	if (geteuid() == notification->getUid()) {
		if (notification->isAdmin()) {
			rs = policyCtrl->getRuleSet(
			    policyCtrl->getAdminId(geteuid()));
		} else {
			rs = policyCtrl->getRuleSet(policyCtrl->getUserId());
		}
	}

	escalation = dynamic_cast<EscalationNotify *>(notification);
	if (escalation && rs) {
		rs->addRuleInformation(escalation);
	}

	escalationsNotAnswered_.addId(notification->getId());
	allNotifications_.addId(notification->getId());

	module = notification->getModule();
	escalationCount_[module] += 1;

	if(module.IsSameAs(wxT("ALF"))) {
		wxCommandEvent showEvent(anEVT_OPEN_ALF_ESCALATIONS);
		showEvent.SetInt(escalationCount_[module]);
		wxPostEvent(anEvents, showEvent);
	} else if(module.IsSameAs(wxT("SFS"))) {
		wxCommandEvent showEvent(anEVT_OPEN_SFS_ESCALATIONS);
		showEvent.SetInt(escalationCount_[module]);
		wxPostEvent(anEvents, showEvent);
	} else if (module.IsSameAs(wxT("SANDBOX"))) {
		wxCommandEvent showEvent(anEVT_OPEN_SB_ESCALATIONS);
		showEvent.SetInt(escalationCount_[module]);
		wxPostEvent(anEvents, showEvent);
	} else {
		/* Default do nothing */
	}

	wxCommandEvent  showEvent(anEVT_OPEN_ESCALATIONS);
	showEvent.SetInt(escalationsNotAnswered_.getSize());
	showEvent.SetExtraLong(1);
	wxPostEvent(anEvents, showEvent);
}

void
NotificationCtrl::addDaemonAnswerNotify(Notification *notification)
{
	DaemonAnswerNotify	*dNotify;
	EscalationNotify	*origNotify;

	dNotify = dynamic_cast<DaemonAnswerNotify *>(notification);
	origNotify = fixupEscalationAnswer(dNotify->getType(),
	    dNotify->getToken(), dNotify->getError());

	if (origNotify) {
		wxCommandEvent	event(anEVT_ADD_NOTIFYANSWER);
		event.SetClientObject((wxClientData*)origNotify);
		wxPostEvent(AnEvents::instance(), event);
	}
}

void
NotificationCtrl::addAlertNotify(Notification *notification)
{
	wxCommandEvent  showEvent(anEVT_OPEN_ALERTS);

	messages_.addId(notification->getId());
	allNotifications_.addId(notification->getId());

	showEvent.SetInt(messages_.getSize());
	showEvent.SetExtraLong(true);
	wxPostEvent(AnEvents::instance(), showEvent);
}

EscalationNotify *
NotificationCtrl::fixupEscalationAnswer(int type, anoubis_token_t token,
    int error)
{
	long				 currentElement;
	bool				 allow = (error == 0);
	NotifyAnswer			*answer;
	Notification			*notify;
	EscalationNotify		*escalation;

	if (type == ANOUBIS_N_RESOTHER) {
		/* Search it in the list of unanswered requests. */
		for (currentElement = 0;
		     currentElement != escalationsNotAnswered_.getSize();
		     currentElement++)
		{
			notify = getNotification(
				escalationsNotAnswered_.getId(currentElement));
			escalation = dynamic_cast<EscalationNotify *>(notify);
			if ((escalation != NULL) &&
			    (escalation->getToken() == token)) {
				answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE,
				    allow);
				answerEscalationNotify(escalation, answer);
				return (escalation);
			}
		}
	}
	/*
	 * RESYOU or not a reply to an unanswered request. Find the request
	 * in the list of answerd requests. Search backwards because the
	 * request is likely to be at the end of the list and the list can
	 * get long over time.
	 */
	currentElement = escalationsNotAnswered_.getSize();
	while (currentElement > 0) {
		currentElement--;
		notify = getNotification(
			escalationsNotAnswered_.getId(currentElement));
		escalation = dynamic_cast<EscalationNotify *>(notify);
		if ((escalation != NULL) && (escalation->getToken() == token)) {
			answer = escalation->getAnswer();
			if (answer && (answer->wasAllowed() != allow)) {
				/*
				 * XXX ch: extend NotifyAnswer for an easyer
				 * XXX ch: way to overwrite an answer...
				 */
				escalation->setAnswer(NULL);
				delete answer;
				answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE,
				    allow);
				escalation->setAnswer(answer);
			}
			return (escalation);
		}
	}

	return NULL;
}

/* Explicit instantiation of Singleton Baseclass methods. */
template NotificationCtrl *
Singleton<NotificationCtrl>::instance();
template NotificationCtrl *
Singleton<NotificationCtrl>::existingInstance();

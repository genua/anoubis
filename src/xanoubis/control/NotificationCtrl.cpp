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
#include "Debug.h"
#include "EscalationNotify.h"
#include "JobCtrl.h"
#include "LogNotify.h"
#include "MainUtils.h"
#include "NotificationCtrl.h"
#include "PlaygroundFileNotify.h"
#include "PolicyCtrl.h"
#include "PolicyRuleSet.h"
#include "StatusNotify.h"

#include "wx/utils.h"

#include "Singleton.cpp"
template class Singleton<NotificationCtrl>;

NotificationCtrl::~NotificationCtrl(void)
{
	std::map<long, Notification *>::iterator	it;

	Disconnect(anEVT_UPDATE_PERSPECTIVE, wxCommandEventHandler(
	    NotificationCtrl::onUpdatePerspectives), NULL, this);
	JobCtrl::instance()->Disconnect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(NotificationCtrl::onDaemonDisconnect),
	    NULL, this);

	for (it=notificationHash_.begin(); it!=notificationHash_.end(); ++it)
		delete it->second;
	notificationHash_.clear();
}

void
NotificationCtrl::addNotification(Notification *notification)
{
	long		id;
	wxCommandEvent	updateEvent(anEVT_UPDATE_PERSPECTIVE);
	bool		wakeup = false;

	id = notification->getId();

	notificationHashMutex_.Lock();
	notificationHash_[id] = notification;
	if (dynamic_cast<PlaygroundFileNotify *>(notification)) {
		playgroundFileNotifyId_ = id;
		wakeup = true;
	}
	notificationHashMutex_.Unlock();
	if (wakeup)
		JobCtrl::existingInstance()->wakeupComThread();

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
	std::map<long, Notification *>::const_iterator	 it;
	Notification					*ret = NULL;

	notificationHashMutex_.Lock();
	it = notificationHash_.find(id);
	if (it != notificationHash_.end())
		ret = it->second;
	notificationHashMutex_.Unlock();

	return  ret;
}

PlaygroundFileNotify *
NotificationCtrl::getPlaygroundFileNotify(void)
{
	Notification					*notify = NULL;

	notificationHashMutex_.Lock();
	if (playgroundFileNotifyId_ != wxID_NONE) {
		std::map<long, Notification *>::iterator	 it;

		it = notificationHash_.find(playgroundFileNotifyId_);
		if (it != notificationHash_.end())
			notify = it->second;
		playgroundFileNotifyId_ = wxID_NONE;
	}
	notificationHashMutex_.Unlock();

	return notify ? dynamic_cast<PlaygroundFileNotify *>(notify) : NULL;
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

bool
NotificationCtrl::havePendingEscalation(const wxString &module) const
{
	Str2LongHash::const_iterator it = escalationCount_.find(module);

	if (it != escalationCount_.end()) {
		long count = (*it).second;
		return (count > 0);
	} else
		return false;
}

bool
NotificationCtrl::answerEscalation(EscalationNotify *escalation,
    bool sendAnswer, wxString &errMsg)
{
	bool		 rc = false;
	long		 id = -1;
	NotifyAnswer	*answer = NULL;

	if (escalation == NULL) {
		errMsg = _("No escalation...???");
		return false;
	}

	answer = escalation->getAnswer();
	if (answer == NULL) {
		errMsg = _("Can't access answer of escalation.");
		Debug::err(wxT("Can't access answer of escalation."));
		return false;
	}

	if (answer->causeTmpRule() || answer->causePermRule()) {
		PolicyRuleSet		*ruleSet;
		PolicyCtrl		*policyCtrl = PolicyCtrl::instance();

		if (escalation->isAdmin()) {
			id = policyCtrl->getAdminId(geteuid());
		} else {
			id = policyCtrl->getUserId();
		}

		ruleSet = policyCtrl->getRuleSet(id);
		if (ruleSet == NULL) {
			Debug::err(wxT("Can't access user ruleset."));
			errMsg = _("Can't access user ruleset.");
			goto out;
		}


		/* Create policy */
		if (!ruleSet->createAnswerPolicy(escalation)) {
			if (ruleSet->isModified())
				errMsg = _("Failed to create a policy based on "
				    "your decision.\nYour ruleset is modified. "
				    "Please send your ruleset to the daemon or "
				    "reload your policies from the daemon. "
				    "Then try again.");
			else
				errMsg = _("Failed to create a policy based on "
				    "your decision.");
			goto out;
		}
		/*
		 * We are going to activate the modified rule set.
		 * Even though the result is 'ok' the policy may not have
		 * reached the daemon yet. But all jobs are done in sequence
		 * and the answer to the event is done later, thus this is
		 * ok.
		 */
		if (policyCtrl->sendToDaemon(ruleSet->getRuleSetId()) !=
		    PolicyCtrl::RESULT_POL_OK)
			goto out;
	}
	rc = true;
out:
	/*
	 * Mark as answered. Always do this even if creating the
	 * escalation rule failed.
	 */
	id = escalation->getId();
	escalationsNotAnswered_.removeId(id);
	escalationsAnswered_.addId(id);

	/* Update view */
	sendUpdateEvent(escalation);

	/* Answer event */
	if (sendAnswer == true)
		JobCtrl::instance()->answerNotification(escalation);

	return rc;
}

NotificationCtrl::NotificationCtrl(void) : Singleton<NotificationCtrl>()
{
	notificationHash_.clear();

	JobCtrl::instance()->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(NotificationCtrl::onDaemonDisconnect),
	    NULL, this);
	Connect(anEVT_UPDATE_PERSPECTIVE, wxCommandEventHandler(
	    NotificationCtrl::onUpdatePerspectives), NULL, this);
	playgroundFileNotifyId_ = wxID_NONE;
}

void
NotificationCtrl::onDaemonDisconnect(wxCommandEvent & event)
{
	NotifyAnswer		*answer;
	Notification		*notify;
	EscalationNotify	*escalation;
	wxString		 msg;

	event.Skip();

	if (event.GetInt() != JobCtrl::CONNECTED) {
		while (escalationsNotAnswered_.getSize() > 0) {
			notify = getNotification(
			    escalationsNotAnswered_.getId(0));
			escalation = dynamic_cast<EscalationNotify *>(notify);
			if (escalation == NULL) {
				/* Should never happen, but let's play safe. */
				escalationsNotAnswered_.removeId(
				    escalationsNotAnswered_.getId(0));
			} else {
				answer = new NotifyAnswer(NOTIFY_ANSWER_ONCE,
				    false);
				escalation->setAnswer(answer);
				answerEscalation(escalation, false, msg);
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
	if (MainUtils::instance()->getModule(PG))
		MainUtils::instance()->getModule(PG)->update();
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
		addAlertNotify(dynamic_cast<AlertNotify *>(notification));
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
	anEvents = AnEvents::instance();
	policyCtrl = PolicyCtrl::instance();
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
	} else if (module.IsSameAs(wxT("PLAYGROUND"))) {
		wxCommandEvent showEvent(anEVT_OPEN_PLAYGROUND_ESCALATIONS);
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
NotificationCtrl::addAlertNotify(AlertNotify *notification)
{
	wxCommandEvent  showEvent(anEVT_OPEN_ALERTS);

	messages_.addId(notification->getId());
	allNotifications_.addId(notification->getId());

	showEvent.SetInt(messages_.getSize());
	showEvent.SetExtraLong(true);
	wxPostEvent(AnEvents::instance(), showEvent);

	if (notification->getSubsystem() == ANOUBIS_SOURCE_PLAYGROUNDPROC) {
		wxCommandEvent	pgevent(anEVT_PLAYGROUND_FORCED);

		pgevent.SetExtraLong(notification->getId());
		wxPostEvent(AnEvents::instance(), pgevent);
	}
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
	wxString			 msg;

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
				escalation->setAnswer(answer);
				answerEscalation(escalation, true, msg);
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

void
NotificationCtrl::sendUpdateEvent(EscalationNotify *notify)
{
	wxString	 module;
	AnEvents	*anEvents;

	anEvents = AnEvents::instance();

	Debug::trace(wxT("NotificationCtrl::answerEscalationNotify"));

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
	wxPostEvent(anEvents, showEvent);
}

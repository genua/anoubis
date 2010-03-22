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

#ifndef _NOTIFICATIONCTRL_H_
#define _NOTIFICATIONCTRL_H_

#include <wx/thread.h>

#include "AnEvents.h"
#include "Notification.h"
#include "NotificationPerspective.h"
#include "Singleton.h"
#include "EscalationNotify.h"

WX_DECLARE_STRING_HASH_MAP(long, Str2LongHash);

/**
 * This is the notification control class.\n
 * It's purpose is to encapsulate handling of notifications.
 * It stores all notifications. You can access a special
 * notification to get it's information or to answer it.
 * With the class NotificationPerspective you can get a
 * special view to a subset of notifications (see getList()).
 */
class NotificationCtrl : public wxEvtHandler, public Singleton<NotificationCtrl>
{
	public:
		/**
		 * These perspectives we want to provide.
		 * XXX ch: refactor/join with Notification::notifyListTypes
		 */
		enum ListPerspectives {
			LIST_NONE = 0,		/**< no list */
			LIST_NOTANSWERED,	/**< new escalations */
			LIST_ANSWERED,		/**< answered escalations */
			LIST_MESSAGES,		/**< messages */
			LIST_STAT,		/**< status notifies */
			LIST_ALL,		/**< all notifies */
		};

		/**
		 * Destructor of NotificationCtrl.
		 * @param None.
		 */
		~NotificationCtrl(void);

		/**
		 * Add new notification.
		 *
		 * This stores new/given notification and updates
		 * the related perspectives.\n
		 * This should only be called from the communicator
		 * with newly received notifications.
		 * @param[in] 1st The new notification.
		 * @return Nothing.
		 */
		void addNotification(Notification *);

		/**
		 * Get stored notification
		 *
		 * Retrive a stored notification by it's id.
		 * @param[in] 1st The id of notification.
		 * @return The notification or NULL
		 */
		Notification *getNotification(long);

		/**
		 * Get notification perspective.
		 *
		 * This will return a special view to a subset of
		 * notifications.
		 * @param[in] 1st Type of perspective
		 * @return The requested perspective or NULL.
		 */
		NotificationPerspective *getPerspective(enum ListPerspectives);

		/**
		 * Mark given escalation as been answered.
		 * @param[in] 1st The escalation.
		 * @param[in] 2nd The answer.
		 * @param[in] 3rd True if the answer shall be sent.
		 * @return Nothing.
		 */
		void answerEscalationNotify(EscalationNotify *, NotifyAnswer *,
		    bool sendAnswer = true);

	protected:
		/**
		 * Constructor of NotificationCtrl.
		 * @param None.
		 */
		NotificationCtrl(void);

	private:
		/**
		 * The list of all received notifications.
		 */
		NotifyHash notificationHash_;

		/**
		 * Mutex to protect access to notificationHash_.
		 */
		wxMutex notificationHashMutex_;

		/**
		 * Cached statistics how many open escalations to a
		 * specific type/module.
		 */
		Str2LongHash escalationCount_;

		/**
		 * Special perspective collecting not answered
		 * escalation notifies.
		 */
		NotificationPerspective escalationsNotAnswered_;

		/**
		 * Special perspective collecting answered escalation
		 * notifies.
		 */
		NotificationPerspective escalationsAnswered_;

		/**
		 * Special perspective collecting log and alert notifies.
		 */
		NotificationPerspective messages_;

		/**
		 * Special perspective collecting status notifies.
		 */
		NotificationPerspective stats_;

		/**
		 * Special perspective collecting all notifies.
		 */
		NotificationPerspective allNotifications_;

		/**
		 * Handle events of disconnect.
		 */
		void onDaemonDisconnect(wxCommandEvent &);

		/**
		 * Handle events of update perspective.
		 * The event carries the id of the new notification.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onUpdatePerspectives(wxCommandEvent &);

		/**
		 * Enqueue escalation notifies.
		 */
		void addEscalationNotify(Notification *);

		/**
		 * Enqueue daemon answer notifies.
		 */
		void addDaemonAnswerNotify(Notification *notification);

		/**
		 * Enqueue alert notifies.
		 */
		void addAlertNotify(Notification *notification);

		/**
		 * Handle answer results.
		 */
		EscalationNotify *fixupEscalationAnswer(int, anoubis_token_t,
		    int);

	friend class Singleton<NotificationCtrl>;
};

#endif	/* _NOTIFICATIONCTRL_H_ */

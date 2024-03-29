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

#include <map>
#include <wx/thread.h>

#include "AnEvents.h"
#include "Notification.h"
#include "NotificationPerspective.h"
#include "Singleton.h"
#include "EscalationNotify.h"
#include "PlaygroundFileNotify.h"
#include "AlertNotify.h"

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
		 * Tests whether there are any unanswered escalations of the
		 * given module.
		 *
		 * @param module The requested module (e.g. ALF, SFS, SANDBOX,
		 *               PLAYGROUND).
		 * @return If you have at least one unanswered escalatiobn of
		 *         the given module, true is returned.
		 */
		bool havePendingEscalation(const wxString &) const;

		/**
		 * Answer an escalation.
		 * In fact the answer object has to be assigned to the
		 * escalation already. This method will cause the creation of
		 * policies if nessesary and sends them to the daemon. After
		 * That the causing event is answered.
		 * @param escalation The escalation in question.
		 * @param sendAnswer True if answer of causing event shall be
		 *                   send daemon.
		 * @param errMsg In case of an error, the method fills the
		 *               string with a more detailed explanation. On
		 *               success, the string is untouched.
		 * @return True on success.
		 */
		bool answerEscalation(EscalationNotify *, bool, wxString &);

		/**
		 * Return the latest playground file notification. Each
		 * notification is returned at most once from this function,
		 * the next return will return NULL.
		 *
		 * @param None.
		 * @return The playground file notification or NULL if
		 *     there is none.
		 */
		PlaygroundFileNotify *getPlaygroundFileNotify(void);
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
		std::map<long, Notification *> notificationHash_;

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
		void addAlertNotify(AlertNotify *notification);

		/**
		 * Handle answer results.
		 */
		EscalationNotify *fixupEscalationAnswer(int, anoubis_token_t,
		    int);

		/**
		 * The ID of the latest playground file notification.
		 * This is set to zero once the playground notification is
		 * processed by a commit task.
		 */
		long	playgroundFileNotifyId_;

		/**
		 * Send update events to view.
		 * This method just sends the concerning events about
		 * new escalation. Main recipient is view.
		 * @param[in] 1st The concerning escalation.
		 * @return Nothing.
		 */
		void sendUpdateEvent(EscalationNotify *);

	/* Allow singlton class access to protected constructor. */
	friend class Singleton<NotificationCtrl>;
};

#endif	/* _NOTIFICATIONCTRL_H_ */

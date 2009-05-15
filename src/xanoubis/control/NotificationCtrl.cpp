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

#include "NotificationCtrl.h"
#include "Singleton.cpp"

NotificationCtrl::~NotificationCtrl(void)
{
	notificationHash_.clear();
}

NotificationCtrl *
NotificationCtrl::instance(void)
{
	return Singleton<NotificationCtrl>::instance();
}

void
NotificationCtrl::addNotification(Notification *notification)
{
	long id;

	id = notification->getId();

	notificationHashMutex_.Lock();
	notificationHash_[id] = notification;
	allNotifications_.addId(id);
	/* XXX ch: still missing: typ check and add to other lists */
	notificationHashMutex_.Unlock();
}

NotificationPerspective *
NotificationCtrl::getPerspective(enum ListPerspectives list)
{
	/*
	 * XXX ch: Only allPerspective gets filled by addNotification()
	 * XXX ch: thus returning other perspectives is disabled for now.
	 */
	switch (list) {
	case LIST_NONE:
		return (NULL);
		break;
	case LIST_NOTANSWERED:
		//return (&escalationsNotAnswered_);
		break;
	case LIST_ANSWERED:
		//return (&escalationsAnswered_);
		break;
	case LIST_ALERTS:
		//return (&alerts_);
		break;
	case LIST_ALL:
		return (&allNotifications_);
		break;
	}

	return (NULL);
}

NotificationCtrl::NotificationCtrl(void) : Singleton<NotificationCtrl>()
{
	notificationHash_.clear();
}

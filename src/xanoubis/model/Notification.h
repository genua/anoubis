/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
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

#ifndef _NOTIFICATION_H_
#define _NOTIFICATION_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "AnListClass.h"

#include <anoubis_msg.h>
#include <wx/hashmap.h>
#include <wx/list.h>
#include <wx/string.h>

enum notifyKey {
	NOTIFY_KEY_MODULE = 0,
	NOTIFY_KEY_TIME,
	NOTIFY_KEY_LOGMSG
};

enum notifyListTypes {
	NOTIFY_LIST_NONE = 0,
	NOTIFY_LIST_NOTANSWERED,
	NOTIFY_LIST_MESSAGE,
	NOTIFY_LIST_ANSWERED,
	NOTIFY_LIST_ALL
};

class Notification : public AnListClass {
	protected:
		wxString		 module_;
		wxString		 timeStamp_;
		wxString		 logMessage_;
		struct anoubis_msg	*notify_;

		Notification(struct anoubis_msg *, bool free = false);
		wxString	assembleAddress(bool);
		wxString	localAlfAddress(void);
		wxString	remoteAlfAddress(void);
		virtual bool	isRwx(unsigned long);

	public:

		virtual ~Notification(void);

		virtual wxString getModule(void);
		virtual wxString getTime(void);
		virtual wxString getLogMessage(void);

		virtual wxString getAction(void);
		virtual wxString getOperation(void);
		virtual wxString getPath(void);
		virtual wxString filePath(void);
		virtual wxString getOrigin(void);
		virtual wxString getCtxOrigin(void);
		virtual wxString getFileChecksum(void);
		virtual unsigned int getRuleId(void);
		virtual bool isAdmin(void);
		virtual uid_t getUid(void);
		virtual bool isRead(void);
		virtual bool isWrite(void);
		virtual bool isExec(void);
		bool needFree(void) const {
			return free_;
		}
		void setNeedFree(void) {
			free_ = true;
		}
		int getSfsmatch(void);
		int getType(void);

		/**
		 */
		long getId(void) const;

		/**
		 */
		static Notification *factory(struct anoubis_msg *);

	private:
		long id_;
		/**
		 * True iff the notification should be freed after
		 * answering it.
		 */
		bool		free_;
};

/**
 * This declares a wxList of Notifications as new typ NotifyList.
 */
WX_DECLARE_LIST(Notification, NotifyList);

#endif	/* _NOTIFICATION_H_ */

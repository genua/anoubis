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

#ifndef _TRAYICON_H_
#define _TRAYICON_H_

#include <wx/icon.h>
#include <wx/event.h>
#include <wx/process.h>
#include <wx/taskbar.h>
#include <libnotify/notify.h>

#include "AnEvents.h"

class TrayIcon : public wxTaskBarIcon
{
	private:
		wxString		daemon_;
		unsigned int		messageByHandNo_;
		unsigned int		messageAlertCount_;
		unsigned int		messageEscalationCount_;
		wxIcon			*iconNormal_;
		wxIcon			*iconMsgProblem_;
		wxIcon			*iconMsgQuestion_;
		bool			sendAlert_;
		bool			sendEscalation_;
		unsigned int            escalationTimeout_;
		unsigned int            alertTimeout_;
		NotifyNotification	*notification;
		wxProcess		*dBusProc_;
		bool			acceptActions_;

		void		 update(void);
		bool		 systemNotify(const gchar*, const gchar*,
		    NotifyUrgency, const int);

		void initDBus(void);

		ANEVENTS_IDENT_BCAST_METHOD_DECLARATION;

	public:
		TrayIcon(void);
		~TrayIcon(void);

		void OnConnectionStateChange(wxCommandEvent&);
		void OnOpenAlerts(wxCommandEvent&);
		void OnOpenEscalations(wxCommandEvent&);
		void OnEscalationSettingsChanged(wxCommandEvent&);
		void OnAlertSettingsChanged(wxCommandEvent&);
		void OnLeftButtonClick(wxTaskBarIconEvent&);
		void OnGuiRestore(wxCommandEvent&);
		void OnGuiExit(wxCommandEvent&);
		void systemNotifyCallback(void);

		virtual wxMenu *CreatePopupMenu(void);

	DECLARE_EVENT_TABLE()

	enum {
		GUI_EXIT = 12001,
		GUI_RESTORE
	};
};

#endif	/* _TRAYICON_H_ */

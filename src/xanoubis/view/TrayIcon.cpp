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

#include <wx/icon.h>
#include <wx/event.h>
#include <wx/string.h>

#include <libnotify/notify.h>
#include "config.h"
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include "AnEvents.h"
#include "main.h"
#include "TrayIcon.h"

#define MAX_MESSAGE	128
#define MAX_PATH	1024
#define ONE_SECOND	1000

BEGIN_EVENT_TABLE(TrayIcon, wxTaskBarIcon)
	EVT_MENU(GUI_EXIT, TrayIcon::OnGuiExit)
	EVT_MENU(GUI_RESTORE, TrayIcon::OnGuiRestore)
	EVT_TASKBAR_LEFT_DCLICK(TrayIcon::OnLeftButtonDClick)
END_EVENT_TABLE()

TrayIcon::TrayIcon(void)
{
	iconNormal_    = wxGetApp().loadIcon(_T("ModAnoubis_black_48.png"));
	iconMsgProblem_ = wxGetApp().loadIcon(_T("ModAnoubis_alert_48.png"));
	iconMsgQuestion_ =
	    wxGetApp().loadIcon(_T("ModAnoubis_question_48.png"));

	messageAlertCount_ = 0;
	messageEscalationCount_ = 0;
	daemon_ = _("none");

	systemNotifyEnabled_ = false;
	systemNotifyTimeout_ = 10;

	notification = notify_notification_new("Anoubis", "", NULL, NULL);

	update();
	Connect(anEVT_COM_REMOTESTATION,
	    wxCommandEventHandler(TrayIcon::OnRemoteStation), NULL, this);
	Connect(anEVT_OPEN_ALERTS,
	    wxCommandEventHandler(TrayIcon::OnOpenAlerts), NULL, this);
	Connect(anEVT_OPEN_ESCALATIONS,
	    wxCommandEventHandler(TrayIcon::OnOpenEscalations), NULL, this);
	Connect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(TrayIcon::OnLogViewerShow), NULL, this);
	Connect(anEVT_SYSNOTIFICATION_OPTIONS,
	    wxCommandEventHandler(TrayIcon::OnSysNotifyChanged), NULL, this);
}

TrayIcon::~TrayIcon(void)
{
	/* free notification object */
	g_object_unref(G_OBJECT(notification));
	notify_uninit();

	delete iconNormal_;
	delete iconMsgProblem_;
	delete iconMsgQuestion_;
}

void
TrayIcon::OnRemoteStation(wxCommandEvent& event)
{
	daemon_ = *(wxString *)(event.GetClientObject());
	update();
	event.Skip();
}

void
TrayIcon::OnOpenAlerts(wxCommandEvent& event)
{
	messageAlertCount_ = event.GetInt();
	update();
}

void
TrayIcon::OnOpenEscalations(wxCommandEvent& event)
{
	messageEscalationCount_ = event.GetInt();
	update();
}

/*
 * [MPI] has decided that we clear Alerts on opening the LogViewer.
 */
void
TrayIcon::OnLogViewerShow(wxCommandEvent& event)
{
	if(event.GetInt())
	{
		messageAlertCount_ = 0;
		update();
	}
}

void
TrayIcon::OnLeftButtonDClick(wxTaskBarIconEvent& event)
{
	wxCommandEvent  showEvent(anEVT_MAINFRAME_SHOW);
	showEvent.SetInt(true);
	wxGetApp().sendEvent(showEvent);
}

void
TrayIcon::OnGuiRestore(wxCommandEvent& event)
{
	wxCommandEvent  showEvent(anEVT_MAINFRAME_SHOW);
	showEvent.SetInt(true);
	wxGetApp().sendEvent(showEvent);
}

void
TrayIcon::OnGuiExit(wxCommandEvent& event)
{
	wxGetApp().quit();
}

void
TrayIcon::OnSysNotifyChanged(wxCommandEvent& event)
{
	if (event.GetInt() > 0) {
		systemNotifyEnabled_ = true;
	} else {
		systemNotifyEnabled_ = false;
	}

	systemNotifyTimeout_ = event.GetExtraLong();
}

void
TrayIcon::SetConnectedDaemon(wxString daemon)
{
	daemon_ = daemon;
	update();
}

wxMenu*
TrayIcon::CreatePopupMenu(void)
{
	wxMenu *menue = new wxMenu;

	menue->Append(GUI_RESTORE, _("&Restore xanoubis"));
	menue->Append(GUI_EXIT, _("E&xit xanoubis"));

	return menue;
}

void
TrayIcon::update(void)
{
	wxString tooltip;
	wxIcon *icon;
	char message[MAX_MESSAGE];
	tooltip = _("Messages: ");

	/* escalations represent the highest priority */
	if (messageEscalationCount_ > 0) {
		tooltip += wxString::Format(wxT("%d\n"),
		    messageEscalationCount_);
		icon = iconMsgQuestion_;
		sprintf(message, "Received Escalations: %d\n",
		    messageEscalationCount_);
		if (systemNotifyEnabled_) {
			if (!systemNotify("ESCALATION", message,
			    NOTIFY_URGENCY_CRITICAL, systemNotifyTimeout_))
				wxGetApp().log(_("Couldn't send Escalation"));
		}
	} else {
		if (messageAlertCount_ > 0) {
			tooltip += wxString::Format(wxT("%d\n"),
			    messageAlertCount_);
			icon = iconMsgProblem_;
			sprintf(message, "Received Alerts: %d\n",
			    messageAlertCount_);
			if (systemNotifyEnabled_) {
				if (!systemNotify("ALERT", message,
				    NOTIFY_URGENCY_NORMAL,
				    systemNotifyTimeout_))
					wxGetApp().log(
					    _("Couldn't send Alert"));
			}
		} else {
			tooltip = _("No messages\n");
			icon = iconNormal_;
		}
	}

	/* connection to daemon established */
	if (!daemon_.Cmp(_("none"))) {
		tooltip += _("not connected");
	} else {
		tooltip += _("connected with ");
		tooltip += daemon_;
	}

	SetIcon(*icon, tooltip);
}

bool
TrayIcon::systemNotify(const gchar *module, const gchar *message,
    NotifyUrgency priority, const int timeout)
{
	char *uri = NULL;
	char buffer[MAX_PATH];
	wxString ipath = wxT("file://");
	int timeShown = (timeout * ONE_SECOND);

	NotifyUrgency messagePriority = priority;

	/* mandatory initialisation call */
	(module != NULL) ? notify_init(module) : notify_init("Anoubis");
	notify_notification_set_timeout(notification, timeShown);

	/* XXX ST: we disable the setting of the corresponding urgency level
	 *	   as it only renders the color area covered by the
	 *	   urgency icon (it's a libnotify bug)
	 *
	 * notify_notification_set_urgency(notification, messagePriority);
	 */

	/* determine the icon used for systemNotify */
	if (messagePriority == NOTIFY_URGENCY_LOW)
		ipath += wxGetApp().getIconPath(_T("ModAnoubis_black_48.png"));
	if (messagePriority == NOTIFY_URGENCY_NORMAL)
		ipath += wxGetApp().getIconPath(_T("ModAnoubis_alert_48.png"));
	if (messagePriority == NOTIFY_URGENCY_CRITICAL)
		ipath += wxGetApp().getIconPath(_T("ModAnoubis_question_48.png")
		    );

	strlcpy(buffer, (const char*)ipath.mb_str(wxConvUTF8), sizeof(buffer));
	uri = buffer;

	/* set notification properties */
	if (message != NULL) {
		notify_notification_update(notification, module, message, uri);

		if (!notify_notification_show (notification, NULL))
			return false;

	} else {
		return false;
	}

	return true;
}

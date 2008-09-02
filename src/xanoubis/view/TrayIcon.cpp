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
	EVT_TASKBAR_LEFT_DOWN(TrayIcon::OnLeftButtonClick)
END_EVENT_TABLE()


TrayIcon::TrayIcon(void)
{
	iconNormal_    = wxGetApp().loadIcon(wxT("ModAnoubis_black_48.png"));
	iconMsgProblem_ = wxGetApp().loadIcon(wxT("ModAnoubis_alert_48.png"));
	iconMsgQuestion_ =
	    wxGetApp().loadIcon(wxT("ModAnoubis_question_48.png"));

	messageAlertCount_ = 0;
	messageEscalationCount_ = 0;
	daemon_ = _("none");

	/* these defaults comply with the wxforrmbuilder settings */
	sendAlert_ = false;
	sendEscalation_ = true;
	escalationTimeout_ = 0;
	alertTimeout_ = 10;

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
	Connect(anEVT_ESCALATIONNOTIFY_OPTIONS,
	    wxCommandEventHandler(TrayIcon::OnEscalationSettingsChanged), NULL,
	    this);
	Connect(anEVT_ALERTNOTIFY_OPTIONS,
	    wxCommandEventHandler(TrayIcon::OnAlertSettingsChanged), NULL,
	    this);
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
	if(event.GetInt()) {
		messageAlertCount_ = 0;
		update();
	}
}

void
TrayIcon::OnLeftButtonClick(wxTaskBarIconEvent&)
{
	if (messageEscalationCount_ > 0 || messageAlertCount_ > 0) {
		this->systemNotifyCallback();
	} else {
		wxCommandEvent  showEvent(anEVT_MAINFRAME_SHOW);

		if (wxGetApp().showingMainFrame())
			showEvent.SetInt(false);
		else
			showEvent.SetInt(true);

		wxGetApp().sendEvent(showEvent);
	}
}

void
TrayIcon::OnGuiRestore(wxCommandEvent&)
{
	wxCommandEvent  showEvent(anEVT_MAINFRAME_SHOW);
	showEvent.SetInt(true);
	wxGetApp().sendEvent(showEvent);
}

void
TrayIcon::OnGuiExit(wxCommandEvent&)
{
	wxGetApp().quit();
}

void
TrayIcon::OnEscalationSettingsChanged(wxCommandEvent& event)
{
	if (event.GetInt() > 0) {
		sendEscalation_ = true;
	} else {
		sendEscalation_ = false;
	}
	escalationTimeout_ = event.GetExtraLong();
}

void
TrayIcon::OnAlertSettingsChanged(wxCommandEvent& event)
{
	if (event.GetInt() > 0) {
		sendAlert_ = true;
	} else {
		sendAlert_ = false;
	}
	alertTimeout_ = event.GetExtraLong();
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
		if (sendEscalation_) {
			if (!systemNotify("ESCALATION", message,
			    NOTIFY_URGENCY_CRITICAL, escalationTimeout_))
				wxGetApp().log(_("Couldn't send Escalation"));
		}
	} else {
		if (messageAlertCount_ > 0) {
			tooltip += wxString::Format(wxT("%d\n"),
			    messageAlertCount_);
			icon = iconMsgProblem_;
			sprintf(message, "Received Alerts: %d\n",
			    messageAlertCount_);
			if (sendAlert_) {
				if (!systemNotify("ALERT", message,
				    NOTIFY_URGENCY_NORMAL,
				    alertTimeout_))
					wxGetApp().log(
					    _("Couldn't send Alert"));
			}
		} else {
			tooltip = _("No messages\n");
			icon = iconNormal_;
		}
	}

	/* connection to daemon established */
	if (!daemon_.Cmp(wxT("none"))) {
		tooltip += _("not connected");
	} else {
		tooltip += _("connected with ");
		tooltip += daemon_;
	}

	SetIcon(*icon, tooltip);
}


static void
callback(NotifyNotification *notification, const char *action,
    void *user_data)
{
	assert(action != NULL);
	assert(strcmp("default", action) == 0);
	notify_notification_close(notification, NULL);

	TrayIcon* inst = (TrayIcon*)user_data;
	inst->systemNotifyCallback();
}

void
TrayIcon::systemNotifyCallback(void)
{
	wxCommandEvent  showEvent(anEVT_ESCALATIONS_SHOW);

	/* request notifications view of ESCALATIONS */
	if (messageEscalationCount_ > 0) {
		showEvent.SetInt(true);
		showEvent.SetString(wxT("ESCALATIONS"));
		wxGetApp().sendEvent(showEvent);
	}

	/* request notifications view of ALERTS */
	if (messageAlertCount_ > 0) {
		showEvent.SetInt(true);
		showEvent.SetString(wxT("ALERTS"));
		wxGetApp().sendEvent(showEvent);
	}
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
	notify_notification_add_action(notification, "default", "default cb",
	    (NotifyActionCallback)callback, this, NULL);

	/* XXX ST: we disable the setting of the corresponding urgency level
	 *	   as it only renders the color area covered by the
	 *	   urgency icon (it's a libnotify bug)
	 *
	 * notify_notification_set_urgency(notification, messagePriority);
	 */

	/* determine the icon used for systemNotify */
	if (messagePriority == NOTIFY_URGENCY_LOW)
		ipath += wxGetApp().getIconPath(wxT("ModAnoubis_black_48.png"));
	if (messagePriority == NOTIFY_URGENCY_NORMAL)
		ipath += wxGetApp().getIconPath(wxT("ModAnoubis_alert_48.png"));
	if (messagePriority == NOTIFY_URGENCY_CRITICAL)
		ipath += wxGetApp().getIconPath(
		    wxT("ModAnoubis_question_48.png"));

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

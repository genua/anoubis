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

/*
 * NOTE CEH: We had lot's of problems with the escalation notifies where
 * NOTE CEH: several notifiers where shown at the same time even though
 * NOTE CEH: we only ever create a singe one. This is apparently because
 * NOTE CEH: the gtk events generated by libnotify are not processed
 * NOTE CEH: properly or not in a timely manner.
 * NOTE CEH: We work around this by forcing a wxWidgets Yield() at several
 * NOTE CEH: places which among other things causes which pending gtk events
 * NOTE CEH: to be processed.
 */

#include <wx/icon.h>
#include <wx/event.h>
#include <wx/string.h>
#include <wx/utils.h>
#include <wx/txtstrm.h>

#include <libnotify/notify.h>
#include "config.h"
#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include "AnEvents.h"
#include "Debug.h"
#include "JobCtrl.h"
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
	AnEvents	*anEvents;
	JobCtrl		*jobCtrl;
	GList		*capabilities;

	iconNormal_    = wxGetApp().loadIcon(wxT("ModAnoubis_black_48.png"));
	iconMsgProblem_ = wxGetApp().loadIcon(wxT("ModAnoubis_alert_48.png"));
	iconMsgQuestion_ =
	    wxGetApp().loadIcon(wxT("ModAnoubis_question_48.png"));

	messageAlertCount_ = 0;
	messageEscalationCount_ = 0;
	daemon_ = wxEmptyString;
	/*
	 * This is a hack that is required in case of auto connect. In
	 * this case the connection is already up and running before the
	 * TrayIcon is constructed.
	 */
	if (JobCtrl::getInstance()->isConnected())
		daemon_ = wxT("localhost");

	/* these defaults comply with the wxforrmbuilder settings */
	sendAlert_ = false;
	sendEscalation_ = true;
	escalationTimeout_ = 0;
	alertTimeout_ = 10;
	acceptActions_ = false;

	initDBus();

	notify_init("Anoubis");
	/*
	 * NOTE: See https://wiki.ubuntu.com/NotificationDevelopmentGuidelines
	 */
	capabilities = notify_get_server_caps();
	if(capabilities != NULL) {
		for(GList *c = capabilities; c != NULL; c = c->next) {
			if(strcmp((char*)c->data, "actions") == 0 ) {
				acceptActions_ = true;
				break;
			}
		}
		g_list_foreach(capabilities, (GFunc)g_free, NULL);
		g_list_free(capabilities);
	}
	notification = notify_notification_new("Anoubis", "", NULL, NULL);
	anEvents = AnEvents::getInstance();
	jobCtrl = JobCtrl::getInstance();

	update();

	jobCtrl->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(TrayIcon::OnConnectionStateChange),
	    NULL, this);
	anEvents->Connect(anEVT_OPEN_ALERTS,
	    wxCommandEventHandler(TrayIcon::OnOpenAlerts), NULL, this);
	anEvents->Connect(anEVT_OPEN_ESCALATIONS,
	    wxCommandEventHandler(TrayIcon::OnOpenEscalations), NULL, this);
	anEvents->Connect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(TrayIcon::OnLogViewerShow), NULL, this);
	anEvents->Connect(anEVT_ESCALATIONNOTIFY_OPTIONS,
	    wxCommandEventHandler(TrayIcon::OnEscalationSettingsChanged), NULL,
	    this);
	anEvents->Connect(anEVT_ALERTNOTIFY_OPTIONS,
	    wxCommandEventHandler(TrayIcon::OnAlertSettingsChanged), NULL,
	    this);
}

TrayIcon::~TrayIcon(void)
{
	AnEvents *anEvents;

	anEvents = AnEvents::getInstance();

	/* free notification object */
	g_object_unref(G_OBJECT(notification));

	notify_uninit();
	if (dBusProc_ != NULL) {
		dBusProc_->Detach();
		dBusProc_->Kill(dBusProc_->GetPid(), wxSIGTERM,
		    wxKILL_CHILDREN);
		delete dBusProc_;
	}

	anEvents->Disconnect(anEVT_OPEN_ALERTS,
	    wxCommandEventHandler(TrayIcon::OnOpenAlerts), NULL, this);
	anEvents->Disconnect(anEVT_OPEN_ESCALATIONS,
	    wxCommandEventHandler(TrayIcon::OnOpenEscalations), NULL, this);
	anEvents->Disconnect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(TrayIcon::OnLogViewerShow), NULL, this);
	anEvents->Disconnect(anEVT_ESCALATIONNOTIFY_OPTIONS,
	    wxCommandEventHandler(TrayIcon::OnEscalationSettingsChanged), NULL,
	    this);
	anEvents->Disconnect(anEVT_ALERTNOTIFY_OPTIONS,
	    wxCommandEventHandler(TrayIcon::OnAlertSettingsChanged), NULL,
	    this);

	delete iconNormal_;
	delete iconMsgProblem_;
	delete iconMsgQuestion_;
}

void
TrayIcon::OnConnectionStateChange(wxCommandEvent& event)
{
	if (event.GetInt() == JobCtrl::CONNECTION_CONNECTED) {
		daemon_ = event.GetString();
	} else {
		daemon_ = wxEmptyString;
	}
	update();
	wxGetApp().Yield(false);	/* Process pending libnotify events. */
	event.Skip();
}

void
TrayIcon::OnOpenAlerts(wxCommandEvent& event)
{
	messageAlertCount_ = event.GetInt();
	update();
	wxGetApp().Yield(false);	/* Process pending libnotify events. */
	event.Skip();
}

void
TrayIcon::OnOpenEscalations(wxCommandEvent& event)
{
	messageEscalationCount_ = event.GetInt();
	update();
	wxGetApp().Yield(false);	/* Process pending libnotify events. */
	event.Skip();
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
	event.Skip();
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

		wxPostEvent(AnEvents::getInstance(), showEvent);
	}
}

void
TrayIcon::OnGuiRestore(wxCommandEvent&)
{
	wxCommandEvent  showEvent(anEVT_MAINFRAME_SHOW);

	showEvent.SetInt(true);

	wxPostEvent(AnEvents::getInstance(), showEvent);
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
	event.Skip();
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
	event.Skip();
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
			/* enforce hiding of the system notifier popup */
			notify_notification_close(notification, NULL);
			/* Process pending libnotify events. */
			wxGetApp().Yield(false);
		}
	}

	/* connection to daemon established */
	if (daemon_.IsEmpty()) {
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

	/* request notifications view of ESCALATIONS or ALERTS */
	if (messageEscalationCount_ > 0) {
		showEvent.SetInt(true);
		showEvent.SetString(wxT("ESCALATIONS"));
		wxPostEvent(AnEvents::getInstance(), showEvent);
	} else if (messageAlertCount_ > 0) {
		showEvent.SetInt(true);
		showEvent.SetString(wxT("ALERTS"));
		wxPostEvent(AnEvents::getInstance(), showEvent);
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
	
	/*
	 * A timeout of 0 used to mean that the notification is shown
	 * indefinitely. notify-osd no longer supports timeouts at all except
	 * for the special value 0. In this case a message box is displayed
	 * instead of a bubble. Using one hour instead of 0 to avoids this
	 * and one hour should be enough for notification-daemon.
	 */
	if (timeShown == 0)
		timeShown = 3600 * ONE_SECOND;
	notify_notification_set_timeout(notification, timeShown);
	if (acceptActions_)
		notify_notification_add_action(notification, "default",
		    "default cb", (NotifyActionCallback)callback, this, NULL);
#ifdef __WXGTK__
	{
		/*
		 * NOTE: notify-osd will ignore this but notification-daemon
		 * NOTE: accepts the position hints.
		 */
		wxWindow	*win = (wxWindow*)m_iconWnd;
		if (win) {
			notify_notification_attach_to_widget(notification,
			    win->m_widget);
		}
	}
#endif

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
		/* Process pending libnotify events. */
		wxGetApp().Yield(false);

	} else {
		return false;
	}

	return true;
}

void
TrayIcon::initDBus(void)
{
	wxString		 word;
	wxString		 dBusCmd;
	wxString		 envName;
	wxString		 envValue;
	wxInputStream		*input;
	wxTextInputStream	*text;

	dBusProc_ = NULL;
	envName = wxT("DBUS_SESSION_BUS_ADDRESS");
	if (wxGetEnv(envName, &envValue)) {
		/* DBUS already running - nothing to do. */
		return;
	}

	/* DBUS is not running - start it. */
	dBusCmd = wxT("dbus-launch --sh-syntax --exit-with-session");
	dBusProc_ = wxProcess::Open(dBusCmd,
	    wxEXEC_MAKE_GROUP_LEADER | wxEXEC_ASYNC);
	if (dBusProc_ == NULL) {
		/* Couldn't start DBus */
		Debug::instance()->log(wxT("Couldn't start DBUS!"), DEBUG_LOG);
		return;
	}

	/*
	 * A session DBus was just started. We make it available
	 * by setting the appropriate environment variables.
	 * We have to parse the following input:
	 * DBUS_SESSION_BUS_ADDRESS='unix:abstract=/tmp/dbus-xxx,guid=xxx';
	 * export DBUS_SESSION_BUS_ADDRESS;
	 * DBUS_SESSION_BUS_PID=14912;
	 */
	input = dBusProc_->GetInputStream();
	text = new wxTextInputStream(*input, wxT(" =\n"));
	while (!input->Eof()) {
		word = text->ReadWord();
		if (word.StartsWith(wxT("DBUS_SESSION_BUS_"))) {
			/* word == (..._ADDRESS or ..._PID) */
			envName = word;
			/*
			 * Read remaining line and
			 *   - remove trailing ';'
			 *   - also remove ticks
			 */
			envValue = text->ReadLine().RemoveLast();
			envValue.Replace(wxT("'"), wxT(""));
			wxSetEnv(envName, envValue.c_str());
		} else if (word == wxT("export")) {
			word = text->ReadLine();
			/* Do nothing with 'export' line. */
		}
	}
}

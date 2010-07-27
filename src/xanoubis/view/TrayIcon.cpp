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
 * NOTE CEH: we only ever create a single one. This is apparently because
 * NOTE CEH: the gtk events generated by libnotify are not processed
 * NOTE CEH: properly or not in a timely manner.
 * NOTE CEH: We work around this by forcing a wxWidgets Yield() at several
 * NOTE CEH: places which among other things causes pending gtk events
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
#include "AnIconList.h"
#include "Debug.h"
#include "JobCtrl.h"
#include "main.h"
#include "TrayIcon.h"
#include "MainUtils.h"
#include "NotificationCtrl.h"

#define MAX_MESSAGE	128
#define MAX_PATH	1024
#define ONE_SECOND	1000

#define SELECT_ICON_ID(id, basename, size) \
	do { \
		switch (size) { \
		case ICON_SIZE_NONE: \
			id = AnIconList::basename; \
			break; \
		case ICON_SIZE_16: \
			id = AnIconList::basename; \
			break; \
		case ICON_SIZE_20: \
			id = AnIconList::basename##_20; \
			break; \
		case ICON_SIZE_24: \
			id = AnIconList::basename##_24; \
			break; \
		case ICON_SIZE_32: \
			id = AnIconList::basename##_32; \
			break; \
		case ICON_SIZE_48: \
			id = AnIconList::basename##_48; \
			break; \
		} \
	} while (0)

DEFINE_LOCAL_EVENT_TYPE(TRAY_PGNOTIFY_CLOSED)

BEGIN_EVENT_TABLE(TrayIcon, wxTaskBarIcon)
	EVT_MENU(GUI_EXIT, TrayIcon::OnGuiExit)
	EVT_MENU(GUI_RESTORE, TrayIcon::OnGuiRestore)
	EVT_TASKBAR_LEFT_DOWN(TrayIcon::OnLeftButtonClick)
	EVT_COMMAND(wxID_ANY, TRAY_PGNOTIFY_CLOSED,
	    TrayIcon::OnPgNotifyClosed)
END_EVENT_TABLE()

static void
close_callback(NotifyNotification *, void *user_data)
{
	TrayIcon	*inst = (TrayIcon *)user_data;
	inst->notifyClosed();
}

static void
pg_close_callback(NotifyNotification *n, void *user_data)
{
	TrayIcon	*tray = (TrayIcon *)user_data;
	wxCommandEvent	 ev(TRAY_PGNOTIFY_CLOSED);

	ev.SetClientData(n);
	wxPostEvent(tray, ev);
}

TrayIcon::TrayIcon(void)
{
	AnEvents	*anEvents;
	JobCtrl		*jobCtrl;
	GList		*capabilities;

	messageAlertCount_ = 0;
	messageEscalationCount_ = 0;
	closed_ = false;
	daemon_ = wxEmptyString;
	/*
	 * This is a hack that is required in case of auto connect. In
	 * this case the connection is already up and running before the
	 * TrayIcon is constructed.
	 */
	if (JobCtrl::instance()->isConnected())
		daemon_ = wxT("localhost");

	/* these defaults comply with the wxforrmbuilder settings */
	sendAlert_ = false;
	sendEscalation_ = true;
	escalationTimeout_ = 0;
	alertTimeout_ = 10;
	acceptActions_ = false;

	initDBus();
	initIcon();

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
	g_signal_connect(notification, "closed",
	    G_CALLBACK(close_callback), this);
	anEvents = AnEvents::instance();
	jobCtrl = JobCtrl::instance();

	update(true);

	jobCtrl->Connect(anEVT_COM_CONNECTION,
	    wxCommandEventHandler(TrayIcon::OnConnectionStateChange),
	    NULL, this);
	anEvents->Connect(anEVT_OPEN_ALERTS,
	    wxCommandEventHandler(TrayIcon::OnOpenAlerts), NULL, this);
	anEvents->Connect(anEVT_OPEN_ESCALATIONS,
	    wxCommandEventHandler(TrayIcon::OnOpenEscalations), NULL, this);
	anEvents->Connect(anEVT_ESCALATIONNOTIFY_OPTIONS,
	    wxCommandEventHandler(TrayIcon::OnEscalationSettingsChanged), NULL,
	    this);
	anEvents->Connect(anEVT_ALERTNOTIFY_OPTIONS,
	    wxCommandEventHandler(TrayIcon::OnAlertSettingsChanged), NULL,
	    this);
	anEvents->Connect(anEVT_PLAYGROUND_FORCED,
	    wxCommandEventHandler(TrayIcon::OnPgForced), NULL, this);
}

TrayIcon::~TrayIcon(void)
{
	AnEvents *anEvents;

	anEvents = AnEvents::instance();

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
	anEvents->Disconnect(anEVT_ESCALATIONNOTIFY_OPTIONS,
	    wxCommandEventHandler(TrayIcon::OnEscalationSettingsChanged), NULL,
	    this);
	anEvents->Disconnect(anEVT_ALERTNOTIFY_OPTIONS,
	    wxCommandEventHandler(TrayIcon::OnAlertSettingsChanged), NULL,
	    this);
	anEvents->Disconnect(anEVT_PLAYGROUND_FORCED,
	    wxCommandEventHandler(TrayIcon::OnPgForced), NULL, this);
}

void
TrayIcon::notifyClosed(void)
{
	closed_ = true;
}

void
TrayIcon::OnConnectionStateChange(wxCommandEvent& event)
{
	if (event.GetInt() == JobCtrl::CONNECTED) {
		daemon_ = event.GetString();
	} else {
		daemon_ = wxEmptyString;
	}
	update(true);
	wxGetApp().Yield(false);	/* Process pending libnotify events. */
	event.Skip();
}

void
TrayIcon::OnOpenAlerts(wxCommandEvent& event)
{
	messageAlertCount_ = event.GetInt();
	update(event.GetExtraLong());
	wxGetApp().Yield(false);	/* Process pending libnotify events. */
	event.Skip();
}

void
TrayIcon::OnOpenEscalations(wxCommandEvent& event)
{
	messageEscalationCount_ = event.GetInt();
	update(event.GetExtraLong());
	wxGetApp().Yield(false);	/* Process pending libnotify events. */
	event.Skip();
}

void
TrayIcon::OnLeftButtonClick(wxTaskBarIconEvent&)
{
	if (messageEscalationCount_ > 0 || messageAlertCount_ > 0) {
		this->systemNotifyCallback();
	} else {
		wxWindow *topWindow = wxGetApp().GetTopWindow();
		topWindow->Show(!topWindow->IsShown());
	}
}

void
TrayIcon::OnGuiRestore(wxCommandEvent&)
{
	wxGetApp().GetTopWindow()->Show();
}

void
TrayIcon::OnGuiExit(wxCommandEvent&)
{
	MainFrame *mf = dynamic_cast<MainFrame*>(wxGetApp().GetTopWindow());
	if (mf != 0)
		mf->exitApp();
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
TrayIcon::update(bool increase)
{
	wxString tooltip;
	wxString connection;
	char message[MAX_MESSAGE];
	tooltip = _("Messages: ");

	/* connection to daemon established */
	if (daemon_.IsEmpty()) {
		connection = _("not connected");
	} else {
		connection = _("connected with ");
		connection += daemon_;
	}

	/* escalations represent the highest priority */
	if (messageEscalationCount_ > 0) {
		tooltip += wxString::Format(wxT("%d\n"),
		    messageEscalationCount_);
		tooltip += connection;
		sprintf(message, "Received Escalations: %d\n",
		    messageEscalationCount_);
		SetIcon(getIcon(ICON_TYPE_QUESTION), tooltip);
		if (sendEscalation_ && (increase || !closed_)) {
			if (!systemNotify("ESCALATION", message,
			    NOTIFY_URGENCY_CRITICAL, escalationTimeout_))
				Debug::err(_("Couldn't send Escalation"));
		}
	} else {
		if (messageAlertCount_ > 0) {
			tooltip += wxString::Format(wxT("%d\n"),
			    messageAlertCount_);
			tooltip += connection;
			sprintf(message, "Received Alerts: %d\n",
			    messageAlertCount_);
			SetIcon(getIcon(ICON_TYPE_ALERT), tooltip);
			if (sendAlert_ && (increase || !closed_)) {
				if (!systemNotify("ALERT", message,
				    NOTIFY_URGENCY_NORMAL,
				    alertTimeout_)){
					Debug::err(_("Couldn't send Alert"));
				}
			}
		} else {
			tooltip = _("No messages\n");
			tooltip += connection;
			SetIcon(getIcon(ICON_TYPE_NORMAL), tooltip);
			/* enforce hiding of the system notifier popup */
			notify_notification_close(notification, NULL);
		}
	}
	/* Process pending libnotify events. */
	wxGetApp().Yield(false);
}

static void
callback(NotifyNotification *notification, gchar *action, void *user_data)
{
	assert(action != NULL);
	assert(strcmp("default", action) == 0);
	notify_notification_close(notification, NULL);

	TrayIcon* inst = (TrayIcon*)user_data;
	inst->notifyClosed();
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
		wxPostEvent(AnEvents::instance(), showEvent);
	} else if (messageAlertCount_ > 0) {
		showEvent.SetInt(true);
		showEvent.SetString(wxT("ALERTS"));
		wxPostEvent(AnEvents::instance(), showEvent);
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
		    "default cb", callback, this, NULL);
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
		ipath += MainUtils::instance()->getIconPath(
		     wxT("ModAnoubis_black_48.png"));
	if (messagePriority == NOTIFY_URGENCY_NORMAL)
		ipath += MainUtils::instance()->getIconPath(
		    wxT("ModAnoubis_alert_48.png"));
	if (messagePriority == NOTIFY_URGENCY_CRITICAL)
		ipath += MainUtils::instance()->getIconPath(
		    wxT("ModAnoubis_question_48.png"));

	strlcpy(buffer, (const char*)ipath.mb_str(wxConvUTF8), sizeof(buffer));
	uri = buffer;

	/* set notification properties */
	if (message != NULL) {
		notify_notification_update(notification, module, message, uri);

		closed_ = false;
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
		Debug::err(wxT("Couldn't start DBUS!"));
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

void
TrayIcon::initIcon(void)
{
	wxIcon		 initIcon;
	wxSize		 windowSize;
	wxWindow	*iconWindow;

	orientation_ = TRAY_ORIENTATION_NONE;

	/* Set icon to initialize window within taskbar. */
	initIcon = AnIconList::instance()->GetIcon(
	    AnIconList::ICON_ANOUBIS_BLACK);
	wxTaskBarIcon::SetIcon(initIcon);

	/*
	 * m_iconWnd is a wxTaskBarIconArea and only known by a forward
	 * declaration. No matter which flavour of wxWidgets been used,
	 * wxTaskBarIconArea is inhereted form wxWidnow.
	 */
	iconWindow = (wxWindow *)m_iconWnd;
	if (iconWindow != NULL) {
		iconWindow->Connect(wxEVT_SIZE,
		    wxSizeEventHandler(TrayIcon::onSize), NULL, this);
	}
}

void
TrayIcon::onSize(wxSizeEvent &event)
{
	event.Skip();

	if (cachedIconWindowSize_.y != event.GetSize().y) {
		/* Height compared. */
		orientation_ = TRAY_ORIENTATION_HORIZONTAL;
	} else if (cachedIconWindowSize_.x != event.GetSize().x) {
		/* Width compared. */
		orientation_ = TRAY_ORIENTATION_VERTICAL;
	}

	cachedIconWindowSize_ = event.GetSize();
	update(true);
}

int
TrayIcon::detectTraySize(void) const
{
	int		 traySize;
	wxSize		 iconWindowSize;
	wxWindow	*iconWindow;

	traySize = -1;
	iconWindow = (wxWindow *)m_iconWnd;
	if (iconWindow != NULL) {
		iconWindowSize = iconWindow->GetSize();
		switch (orientation_) {
		case TRAY_ORIENTATION_VERTICAL:
			traySize = iconWindowSize.GetWidth();
			break;
		case TRAY_ORIENTATION_HORIZONTAL:
			traySize = iconWindowSize.GetHeight();
			break;
		case TRAY_ORIENTATION_NONE:
			traySize = -1;
			break;
		}
	}

	return (traySize);
}

TrayIcon::IconSize
TrayIcon::translateToIconSize(int traySize) const
{
	enum IconSize iconSize;

	/*
	 * To avoid placement offset done by wxWidgets we delay switching to
	 * new icon size until icon is smaller than window size.
	 */
	if (traySize < 20 + 1) {
		iconSize = ICON_SIZE_16;
	} else if (traySize < 24 + 1) {
		iconSize = ICON_SIZE_20;
	} else if (traySize < 32 + 1) {
		iconSize = ICON_SIZE_24;
	} else if (traySize < 48 + 1) {
		iconSize = ICON_SIZE_32;
	} else {
		iconSize = ICON_SIZE_48;
	}

	return (iconSize);
}

wxIcon
TrayIcon::getIcon(IconType type) const
{
	enum IconSize		size;
	enum AnIconList::IconId	iconId;

	size   = translateToIconSize(detectTraySize());
	iconId = AnIconList::ICON_NONE;

	switch (type) {
	case ICON_TYPE_NONE:
		iconId = AnIconList::ICON_NONE;
		break;
	case ICON_TYPE_NORMAL:
		SELECT_ICON_ID(iconId, ICON_ANOUBIS_BLACK, size);
		break;
	case ICON_TYPE_ALERT:
		SELECT_ICON_ID(iconId, ICON_ANOUBIS_ALERT, size);
		break;
	case ICON_TYPE_QUESTION:
		SELECT_ICON_ID(iconId, ICON_ANOUBIS_QUESTION, size);
		break;
	}

	return (AnIconList::instance()->GetIcon(iconId));
}

bool
TrayIcon::SetIcon(const wxIcon& icon, const wxString& tooltip)
{
	int		 traySize;
	int		 iconSize;
	wxWindow	*iconWindow;

	/* The icon is square shaped, width/height makes no difference. */
	iconSize = icon.GetWidth();
	traySize = detectTraySize();
	iconWindow = (wxWindow *)m_iconWnd;

	/* wxWindow->SetSize() does not work. Force size by gtk-call. */
#ifdef __WXGTK__
	if (iconWindow != NULL) {
		/*
		 * Depending on orientation, we want to limit the iconWindow
		 * to the size of the embedded icon.
		 */
		switch (orientation_) {
		case TRAY_ORIENTATION_VERTICAL:
			gtk_widget_set_size_request(iconWindow->m_widget,
			    traySize, iconSize);
			break;
		case TRAY_ORIENTATION_HORIZONTAL:
			gtk_widget_set_size_request(iconWindow->m_widget,
			    iconSize, traySize);
			break;
		case TRAY_ORIENTATION_NONE:
			break;
		}
	}
#endif /* __WXGTK__ */

	return (wxTaskBarIcon::SetIcon(icon, tooltip));
}

void
TrayIcon::OnPgNotifyClosed(wxCommandEvent &ev)
{
	NotifyNotification	*notify;

	notify = (NotifyNotification*)ev.GetClientData();
	if (notify) {
		wxGetApp().Yield(false);
		g_object_unref(notify);
	}
}

void
TrayIcon::OnPgForced(wxCommandEvent &ev)
{
	long			 id = ev.GetExtraLong();
	Notification		*n;
	NotifyNotification	*notify;
	wxString		 msg;

	n = NotificationCtrl::instance()->getNotification(id);
	if (n == NULL)
		return;
	msg = wxString::Format(_("Program %ls will be started in a playground"),
	    n->getPath().c_str());
	notify = notify_notification_new("Anoubis", msg.fn_str(), NULL, NULL);
	g_signal_connect(notify, "closed", G_CALLBACK(pg_close_callback), this);
	notify_notification_set_timeout(notify, 5*ONE_SECOND);
#ifdef __WXGTK__
	{
		/*
		 * NOTE: notify-osd will ignore this but notification-daemon
		 * NOTE: accepts the position hints.
		 */
		wxWindow        *win = (wxWindow*)m_iconWnd;
		if (win) {
			notify_notification_attach_to_widget(notify,
			    win->m_widget);
		}
	}
#endif
	if (notify_notification_show(notify, NULL))
		wxGetApp().Yield(false);
}

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
		bool			sendAlert_;
		bool			sendEscalation_;
		unsigned int            escalationTimeout_;
		unsigned int            alertTimeout_;
		NotifyNotification	*notification;
		NotifyNotification	*pgnotify;
		wxProcess		*dBusProc_;
		bool			acceptActions_;

		/**
		 * True if the notification is no longer showing. Do not
		 * update the notification balloon unless there really are
		 * new events if this is true.
		 */
		bool			closed_;

		void		 update(bool increase);

		/**
		 * Cache icon window size
		 */
		wxSize cachedIconWindowSize_;

		/**
		 * Enumeration to classify orientation.
		 */
		enum TrayOrientation {
			TRAY_ORIENTATION_NONE = -1,
			TRAY_ORIENTATION_HORIZONTAL,
			TRAY_ORIENTATION_VERTICAL,
		};

		/**
		 * Enumeration to classify icon sizes.
		 */
		enum IconSize {
			ICON_SIZE_NONE = -1,
			ICON_SIZE_16,
			ICON_SIZE_20,
			ICON_SIZE_24,
			ICON_SIZE_32,
			ICON_SIZE_48,
		};

		/**
		 * Enumeration to classify icon types.
		 */
		enum IconType {
			ICON_TYPE_NONE = -1,
			ICON_TYPE_NORMAL,
			ICON_TYPE_ALERT,
			ICON_TYPE_QUESTION,
		};

		/**
		 * Store current tray orientation.
		 */
		TrayOrientation orientation_;

		bool		 systemNotify(const gchar*, const gchar*,
		    NotifyUrgency, const int);

		void initDBus(void);

		/**
		 * Initialize icon.
		 * Set the first icon to initialize wxTaskBarIconArea and
		 * anything related to the icon.
		 * @param None.
		 * @return Nothing.
		 */
		void initIcon(void);

		/**
		 * Handle size events.
		 * This method is called if window size of icon has changed.
		 * @param[in] 1st The size event.
		 * @return Nothing.
		 */
		virtual void onSize(wxSizeEvent &);

		/**
		 * Detect size of tray.
		 * This method uses the internal tray icon area and query it
		 * for it's size. This can be done because its a child of
		 * wxWindow. As we get width and height from this window we use
		 * orientation from a previous size-event to decide which one to
		 * count on.
		 * @param None.
		 * @return The number of pixel or -1 if no window or no
		 *	orientation exists.
		 */
		int detectTraySize(void) const;

		/**
		 * Translate size to icon size.
		 * This method translates the given size (of the tray) to a
		 * fixed value of enum IconSize. This is used by getIcon()
		 * to return an icon of correct size.
		 * @param[in] 1st The size in question.
		 * @return The matching icon size.
		 */
		enum IconSize translateToIconSize(int) const;

		/**
		 * Get icon.
		 * Use this method to get an icon. The given type of icon and
		 * the size of the tray (by detectTraySize() and
		 * translateToIconSize() is used to calculate the concerning
		 * icon index. This index is feed to AnIconList to get the icon.
		 * @param[in] 1st The type of icon.
		 * @return The concerning wxIcon.
		 */
		wxIcon getIcon(IconType) const;

		ANEVENTS_IDENT_BCAST_METHOD_DECLARATION;

	public:
		static void waitForSystemTray(void);

		TrayIcon(void);
		~TrayIcon(void);

		/**
		 * Called by the notification callbacks if the notification
		 * is closed, either manually or due to a timeout.
		 */
		void		 notifyClosed(void);

		void OnConnectionStateChange(wxCommandEvent&);
		void OnOpenAlerts(wxCommandEvent&);
		void OnOpenEscalations(wxCommandEvent&);
		void OnEscalationSettingsChanged(wxCommandEvent&);
		void OnAlertSettingsChanged(wxCommandEvent&);
		void OnLeftButtonClick(wxTaskBarIconEvent&);
		void OnGuiRestore(wxCommandEvent&);
		void OnGuiExit(wxCommandEvent&);
		void OnPgNotifyClosed(wxCommandEvent&);
		void systemNotifyCallback(void);

		virtual wxMenu *CreatePopupMenu(void);

		/**
		 * Set icon.
		 * This is an overlay to the original wxTaskBarIcon::SetIcon().
		 * Before the icon is set to the tray, the responcible window
		 * is forcibly and depending on the orientation resized by
		 * gtk-call.
		 * @param[in] 1st The new icon.
		 * @param[in] 2nd The new tooltip.
		 * @return True on success.
		 */
		virtual bool SetIcon(const wxIcon&, const wxString&);

		/**
		 * This function can be called to notify the user of a
		 * process that has been forced into the playground by
		 * an APN rule. The ID of the notification must be in
		 * the GetExtraLong() field of the event.
		 *
		 * @param ev The event.
		 * @return None.
		 */
		void OnPgForced(wxCommandEvent &ev);

		/**
		 * This function is called in response to a PG_CHANGE event,
		 * i.e. a playground just terminated. This funtion will
		 * display a notification message to the user.
		 *
		 * @param ev The event.
		 */
		void OnPgChange(wxCommandEvent &ev);

		/**
		 * Show a notification message to the user.
		 *
		 * @param msg The message.
		 */
		void ShowPgAlert(wxString &msg);

	enum {
		GUI_EXIT = 12001,
		GUI_RESTORE
	};
	DECLARE_EVENT_TABLE()
};

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_LOCAL_EVENT_TYPE(TRAY_PGNOTIFY_CLOSED, wxNewEventType())
END_DECLARE_EVENT_TYPES()


#endif	/* _TRAYICON_H_ */

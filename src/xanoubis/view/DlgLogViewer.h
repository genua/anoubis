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

#ifndef __DlgLogViewer__
#define __DlgLogViewer__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "AnEvents.h"
#include "AnListClassProperty.h"
#include "AnListProperty.h"
#include "DlgLogViewerBase.h"
#include "Notification.h"
#include "Observer.h"

/**
 * This is the LogViewer.
 *
 * It's purpose is to show all log and escalation notifications
 * within a list. Status notifications are not shown.
 * By selecting a row from the list, the RuleEditor is opened,
 * showing the selected rule that caused the log entry.
 */
class DlgLogViewer : public DlgLogViewerBase, private Observer
{
	public:
		/**
		 * Constructor of DlgLogViewer.
		 * @param[in] 1st The parent window.
		 */
		DlgLogViewer(wxWindow *);

		/**
		 * Destructor of DlgLogViewer.
		 * @param None.
		 */
		~DlgLogViewer(void);

	private:
		/**
		 * Handle show events.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onShow(wxCommandEvent &);

		/**
		 * Handle close by window decoration.
		 * This will not just toggle the visability of this frame.
		 * Instead an event anEVT_LOGVIEWER_SHOW will been sent to
		 * inform all parts of the GUI (e.g MainFrame menue or the
		 * buttons of the status bar) about the closed LogViewer.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onClose(wxCloseEvent &);

		/**
		 * Handle selection of log entry.
		 * @param[in] 1st The event.
		 * @return Nothing.
		 */
		void onLogSelect(wxListEvent &);

		/**
		 * This is called when the observed perspective was modified.
		 * @param[in] 1st The changed perspectilve (aka subject).
		 * @return Nothing.
		 */
		void update(Subject *);

		/**
		 * This is called when the observed perspective is about to
		 * be destroyed.
		 * @param[in] 1st The changed perspective (aka subject).
		 * @return Nothing.
		 */
		void updateDelete(Subject *);

		ANEVENTS_IDENT_BCAST_METHOD_DECLARATION;
};

/**
 * Notification properties.
 *
 * This class defines properties for the Notification class. The constructor
 * specifies the role of the property.
 */
class LogViewerProperty : public AnListProperty
{
	public:
		/**
		 * Role of the property.
		 *
		 * The property can act in different roles. They are specified
		 * here.
		 */
		enum PropertyRole
		{
			PROPERTY_ICON,	/*!< Depending on the type of the
					     notification, the icon-property
					     displays another icon. */
			PROPERTY_TIME,	/*!< Time of notification */
			PROPERTY_MODULE,/*!< The module of the notification */
			PROPERY_MESSAGE	/*!< The message of the notification */
		};

		/**
		 * Constructs a LogViewerProperty.
		 *
		 * @param role The role of the property.
		 */
		LogViewerProperty(PropertyRole);

		wxString getHeader(void) const;
		wxString getText(AnListClass *) const;
		AnIconList::IconId getIcon(AnListClass *) const;
		wxColour getBackgroundColor(AnListClass *) const;

	private:
		PropertyRole role_;
};

/**
 * The property of the complete Notification.
 *
 * This class defines a property valid for all properties (aka AnListProperty)
 * of a Notification.
 */
class NotificationProperty : public AnListClassProperty
{
	public:
		/**
		 * Setups the background color for a Notification.
		 *
		 * If this is a admin-notification, the background-color
		 * switches to grey.
		 */
		wxColour getBackgroundColor(AnListClass *) const;
};

#endif /* __DlgLogViewer__ */

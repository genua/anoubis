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

#ifndef _ANEVENTS_H_
#define _ANEVENTS_H_

#include <wx/event.h>

#include "Singleton.h"

/**
 * \file AnEvents.h
 *
 * The internal events of xanoubis are defined here. They are wxCommandEvents
 * with a special type. In most cases they are used for signaling and
 * broadcasting information from the sender to (maybe many) recipiants.
 *
 * Events within wxWidgets works as follows. An event is created and send by
 * \c wxPostEvent() to a given event handler. This will call
 * \c evtHandler->ProcessEvent(event), checking  all entries within the table
 * of the handler. Depending on the value of propagation of the event, it's
 * passed to the parent of the event handler.
 * It's possible to modify the double-linked list of event handlers with
 * \c PushEventHandler() (defined by a window) or with
 * \c SetNextHandler() or \c SetPreviousHandler() directly.
 *
 * We have to admit, we had quite difficulties to model the list to fit our
 * needs (especally joining to window hirarchies). Thus we came up with the
 * solution of a central event handler for managing the broadcast events,
 * leaving the list of event handlers set up by widgets untouched.
 *
 * \note Implications and assumptions of this solution:
 * \li The events defined within this file are GUI internal. They are not
 *	defined by wxWidgets and not used by it's widgets.
 * \li In most cases the sender does not know who is receiving the event nor
 *	does receivers know who can/may send the event. This is a n:m relation
 *	with unknwon participants. Thus understand these events as broadcast
 *	events.
 * \li This lead us to two different classes of events. In the following we
 *	will refer them as \b broadcast- and \b widget-events.
 * \li Broadcast-events are send to the central event manager AnEvents. You
 *	can use either one of:
 *	@code
 *		wxPostEvent(AnEvents::instance(), event);
 *		AnEvents::instance()->ProssessEvent(event);
 *	@endcode
 * \li Broadcast-events are received by calling \c Connect() from \ref
 *	AnEvents. And remember to also \c Disconnect() (e.g with the
 *	destructor).
 * \li In the most cases widget-events are send by the widgets itself and you
 *	just want to register for them. But if you want to send one by your own
 *	just use:
 *	@code
 *		wxPostEvent(this, event);
 *	@endcode
 * \li Widget-events are received by calling \c Connect() on yourself. In most
 *	cases this is already done by the Base-Class created with
 *	wxFormBuilder. You only have to implement the concerning callback
 *	method.
 *
 * \par Receiving events:
 * For propper functionality of the event mechanism, we strongly suggest to
 * obey to the following hints:
 * \li For connecting to a broadcast event, use \c
 *	AnEvents::instance()->Connect() the way \ref
 *	ANEVENTS_IDENT_BCAST_REGISTRATION does.
 * \li For disconnecting from a broadcast event, use \c
 *	AnEvents::instance()->Disconnect() the way \ref
 *	ANEVENTS_IDENT_BCAST_DEREGISTRATION does.
 * \li It's (again) \em strongly \em suggested to use \c event.Skip() within
 *	your implementation of your callback method to ensure the event also
 *	reaches the other entries in the table.
 *
 * \par Debugging:
 * For debugging purpose only a special broadcast event is defined. It's called
 * "ident broadcast". If you have trouble with sending or receiving events it
 * should help you to identify sender and recipiants. For conveniant usage a
 * bunch of macros are defined. All macros can be enabled or disables at once
 * using \ref ANEVENTS_IDENT_BCAST_ENABLE. Use them to verify the ability of
 * sending and/or receiving events.\n
 * Once placed within the code, they may stay, because an undefined \c
 * ANEVENTS_IDENT_BCAST_ENABLE will switch them off. But you have to ensure
 * that \c ANEVENTS_IDENT_BCAST_ENABLE is not defined within productive code.
 * \li Modify your header file to meet the following exampel of code:
 *	@code
 *		#include "AnEvents.h"
 *
 *		class abc {
 *			ANEVENTS_IDENT_BCAST_METHOD_DECLARATION;
 *		}
 *	@endcode
 * \li Modify your source file to meet the following example of code:
 *	@code
 *		#include "AnEvents.h"
 *
 *		abc::abc() {
 *			ANEVENTS_IDENT_BCAST_REGISTRATION(abc);
 *		}
 *
 *		abc::~abc() {
 *			ANEVENTS_IDENT_BCAST_DEREGISTRATION(abc);
 *		}
 *
 *		ANEVENTS_IDENT_BCAST_METHOD_DEFINITION(abc)
 *	@endcode
 * \li For sending a ident broadcast event you may just write:
 *	@code
 *		#include "AnEvents.h"
 *
 *		... {
 *			ANEVENTS_IDENT_BCAST_SEND();
 *		}
 *	@endcode
 */

/**
 * Enable or disable all other macros for ident broadcast events.
 * Please make sure that this is only used during development (change) and
 * been disables within the final product (baseline).\n
 * Enable:\n
 *	\#define ANEVENTS_IDENT_BCAST_ENABLE\n
 * Disable:\n
 *	\#undef ANEVENTS_IDENT_BCAST_ENABLE\n
 */
#undef ANEVENTS_IDENT_BCAST_ENABLE

/**
 * In case of an enabled ident broadcast event, methods handling the event
 * will be named as defined here.
 */
#ifdef ANEVENTS_IDENT_BCAST_ENABLE
#define ANEVENTS_IDENT_BCAST_METHODNAME	onIdentBroadcast
#endif /* ANEVENTS_BROADCAST_METHOD */

/**
 * In case of an enabled ident broadcast event, this is the event name.
 */
#ifdef ANEVENTS_IDENT_BCAST_ENABLE
#define ANEVENTS_IDENT_BCAST_EVENTNAME	anEVT_IDENT_BROADCAST
#endif /* ANEVENTS_BROADCAST_METHOD */

/**
 * In case of an enabled ident broadcast event, this will declare the method
 * within the class receiving the ident broadcast event. If disabled just do
 * nothing.
 */
#ifdef ANEVENTS_IDENT_BCAST_ENABLE
#define ANEVENTS_IDENT_BCAST_METHOD_DECLARATION \
	private: \
		void ANEVENTS_IDENT_BCAST_METHODNAME(wxCommandEvent&)
#else
#define ANEVENTS_IDENT_BCAST_METHOD_DECLARATION
#endif /* ANEVENTS_BROADCAST_METHOD */

/**
 * In case of an enabled ident broadcast event, this will define the method
 * for the given class. If disabled just do nothing.
 * @param class The class name defining the method for.
 */
#ifdef ANEVENTS_IDENT_BCAST_ENABLE
#define ANEVENTS_IDENT_BCAST_METHOD_DEFINITION(class) \
	void \
	class::ANEVENTS_IDENT_BCAST_METHODNAME(wxCommandEvent& event) \
	{ \
		/* XXX CH: should we use class Debug for this? */ \
		fprintf(stderr, "onBroadcast @ %s:%d\n", \
		    __FILE__, __LINE__); \
		event.Skip(); \
	}
#else
#define ANEVENTS_IDENT_BCAST_METHOD_DEFINITION(class)
#endif /* ANEVENTS_BROADCAST_METHOD */

/**
 * In case of an enabled ident broadcast event, this will register a given
 * class for receiving those events. If disabled just do nothing.
 * @param class The class name used for registration.
 */
#ifdef ANEVENTS_IDENT_BCAST_ENABLE
#define ANEVENTS_IDENT_BCAST_REGISTRATION(class) \
	do { \
		AnEvents::instance()->Connect( \
		    ANEVENTS_IDENT_BCAST_EVENTNAME, wxCommandEventHandler( \
		    class::ANEVENTS_IDENT_BCAST_METHODNAME), NULL, this); \
	} while (0)
#else
#define ANEVENTS_IDENT_BCAST_REGISTRATION(class) \
	do { \
		/* Nothing */ \
	} while (0)
#endif /* ANEVENTS_IDENT_BCAST_ENABLE */

/**
 * In case of an enabled ident broadcast event, this will unregister a given
 * class from receiving those events. If disabled just do nothing.
 * @param class The class name used for deregistration.
 */
#ifdef ANEVENTS_IDENT_BCAST_ENABLE
#define ANEVENTS_IDENT_BCAST_DEREGISTRATION(class) \
	do { \
		AnEvents::instance()->Disconnect( \
		    ANEVENTS_IDENT_BCAST_EVENTNAME, wxCommandEventHandler( \
		    class::ANEVENTS_IDENT_BCAST_METHODNAME), NULL, this); \
	} while (0)
#else
#define ANEVENTS_IDENT_BCAST_DEREGISTRATION(class) \
	do { \
		/* Nothing */ \
	} while (0)
#endif /* ANEVENTS_IDENT_BCAST_ENABLE */

/**
 * In case of an enabled ident broadcast event, this will send an ident
 * broadcast event. This should not been called/included within productive
 * code. Thus if disables a compilation error is raised.
 */
#ifdef ANEVENTS_IDENT_BCAST_ENABLE
#define ANEVENTS_IDENT_BCAST_SEND() \
	do { \
		wxCommandEvent	_idBcastEvent(ANEVENTS_IDENT_BCAST_EVENTNAME); \
		\
		wxPostEvent(AnEvents::instance(), _idBcastEvent); \
	} while (0)
#else
/*
 * This is intentionally left empty, so a forgotten send macro will cause
 * compilation errors. Please remove the send macros from productive code
 * instead of defining an empty do-nothing version of the send macro.
 */
#endif /* ANEVENTS_IDENT_BCAST_ENABLE */

/**
 * This is the central event handler for GUI internal (aka broadcast) events.
 * Register here to receive broadcast events.
 * @see AnEvents.h for more detail about events within the GUI.
 */
class AnEvents : public wxEvtHandler, public Singleton<AnEvents>
{
	public:
		/**
		 * Destructor of AnEvents.
		 */
		~AnEvents(void);

	protected:
		/**
		 * Constructor of AnEvents.
		 */
		AnEvents(void);

	friend class Singleton<AnEvents>;
};


/**
 * @name Special event-types for xanoubis.
 * @{
 */
BEGIN_DECLARE_EVENT_TYPES()

#ifdef ANEVENTS_IDENT_BCAST_ENABLE
	/**
	 * Ident broadcast event.
	 * This event is used for debugging purpose only.
	 */
	DECLARE_LOCAL_EVENT_TYPE(ANEVENTS_IDENT_BCAST_EVENTNAME,
	    wxNewEventType())
#endif /* ANEVENTS_IDENT_BCAST_ENABLE */

	/**
	 * Show Wizard event.
	 * @param GetInt() Visability of DlgWizard:
	 *	- = 0 : hide
	 *	- > 0 : show
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_WIZARD_SHOW, wxNewEventType())

	/**
	 * Show LogViewer event.
	 * @param GetInt() Visability of DlgLogViewer:
	 *	- = 0 : hide
	 *	- > 0 : show
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_LOGVIEWER_SHOW, wxNewEventType())

	/**
	 * Show RuleEditor event.
	 * @param GetInt() Visability of DlgRuleEditor:
	 *	- = 0 : hide
	 *	- > 0 : show
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_RULEEDITOR_SHOW, wxNewEventType())

	/**
	 * Show escalation event.
	 * Request the View of the current Notifications due to user interaction
	 * caused by clicking on the TrayIcon or Notification Popup when
	 * Escalations or Alerts were reported by these, to the user of
	 * xanoubis.
	 *
	 * @param GetInt() Visability of MainFrame and ModAnoubis:
	 *	- = 0 : hide
	 *	- > 0 : show
	 * @param GetString() Select message type:
	 *	- "ESCALATIONS": Show Notification Tab and current notifications
	 *	- "ALERTS": Show Notification Tab and current messages
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ESCALATIONS_SHOW, wxNewEventType())

	/**
	 * Show AnoubisOptions event.
	 * Request the View of the optional settings of Module Anoubis
	 * (Options tab).
	 *
	 * @param GetInt() Visability of MainFrame and ModAnoubis:
	 *	- = 0 : hide
	 *	- > 0 : show
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ANOUBISOPTIONS_SHOW, wxNewEventType())

	/**
	 * Show Rule event.
	 * Request the visability / selection of a given rule within RuleEditor.
	 *
	 * @param GetInt() 'user' or 'admin' rule:
	 *	- = 0 : user
	 *	- > 0 : admin
	 * @param GetExtraLong() The id of the rule to be selected.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_SHOW_RULE, wxNewEventType())

	/**
	 * Escalation notify options event.
	 * The options of the system notification via libnotify for escalations
	 * have changed.
	 *
	 * @param GetInt() Open system notification:
	 *	- = 0 : don't show system notification
	 *	- > 0 : show system notification
	 * @param GetExtraLong() Time in seconds until notification disapears.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ESCALATIONNOTIFY_OPTIONS,
	    wxNewEventType())

	/**
	 * Alert notify options event.
	 * The options of the system notification via libnotify for alerts
	 * have changed.
	 *
	 * @param GetInt() Open system notification:
	 *	- = 0 : don't show system notification
	 *	- > 0 : show system notification
	 * @param GetExtraLong() Time in seconds until notification disapears.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ALERTNOTIFY_OPTIONS, wxNewEventType())

	/**
	 * Communication connect event.
	 * The connection-state to the daemon has changed.
	 *
	 * @param GetInt() Contains enum JobCtrl::ConnectionState stating the
	 *	state of the connection.
	 * @param GetString() In case of a connected-message the hostname,
	 *	where the daemon is running.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_COM_CONNECTION, wxNewEventType())

	/**
	 * Auto check checksum event.
	 * Send the new Status for the Auto Checksum Checkbox.
	 *
	 * @param GetInt() The status of the checkbox:
	 *	- = 0 : not checked - no auto check
	 *	- > 0 : checked - do auto check
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_SEND_AUTO_CHECK, wxNewEventType())

	/**
	 * Open alerts event.
	 * Broadcast event about the number of open/new alerts.
	 *
	 * @param GetInt() The count of open ALERTS.
	 * @param GetExtraLong() True if a new alert was added,
	 *     false otherwise.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_OPEN_ALERTS, wxNewEventType())

	/**
	 * Open escalations event.
	 * Broadcast event about the number of new/not answered escalations.
	 *
	 * @param GetInt() The count of open ESCALATIONS.
	 * @param GetExtraLong() True if this event notifies us about a
	 *     new escalation, false if an escalation was answered/removed.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_OPEN_ESCALATIONS, wxNewEventType())

	/**
	 * Open alf escalations event.
	 * Broadcast event about the number of new/not answered escalations
	 * caused by alf policies.
	 *
	 * @param GetInt() The count of open ESCALATIONS.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_OPEN_ALF_ESCALATIONS, wxNewEventType())

	/**
	 * Open sfs escalations event.
	 * Broadcast event about the number of new/not answered escalations
	 * caused by sfs policies.
	 *
	 * @param GetInt() The count of open ESCALATIONS.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_OPEN_SFS_ESCALATIONS, wxNewEventType())

	/**
	 * Open sandbox escalations event.
	 * Broadcast event about the number of new/not answered escalations
	 * caused by sfs policies.
	 *
	 * @param GetInt() The count of open ESCALATIONS.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_OPEN_SB_ESCALATIONS, wxNewEventType())

	/**
	 * Open playground escalations event.
	 * Broadcast event about the number of new/not answered escalations
	 * caused by playground policies.
	 *
	 * @param GetInt() The count of open ESCALATIONS.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_OPEN_PLAYGROUND_ESCALATIONS,
	    wxNewEventType())

	/**
	 * Notification Result from Daemon arrived.
	 * Transports the escalation object that was answered as client object.
	 * The actual Daemon Reply message is already destroyed, the
	 * notification is stored permanently by ModAnoubis.
	 * This event is basically for the benefit of the LogViewer.
	 *
	 * @param GetClientObject() Instance of Notification.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ADD_NOTIFYANSWER, wxNewEventType())

	/**
	 * Load ruleset event.
	 * A new rule set was loaded/stored to the policy controller.
	 * Your current one may be out of date.
	 *
	 * @param GetInt() The id of the old ruleset.
	 * @param GetExtraLong() The id of the new ruleset.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_LOAD_RULESET, wxNewEventType())

	/**
	 * Send ruleset event.
	 * The policy controler sent a ruleset to the daemon.
	 *
	 * @param GetInt() The id of the ruleset.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_SEND_RULESET, wxNewEventType())

	/**
	 * Load key event.
	 * A private key or a certificate was loaded to the key manager.
	 *
	 * @param GetInt()
	 * - If set to 0, the private key was loaded
	 * - If set to 1, the certificate was loaded
	 * - Any other value is undefined
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_LOAD_KEY, wxNewEventType())

	/**
	 * Operation of SfsCtrl has finished.
	 *
	 * Whenever an SfsCtrl-operation has finished, an wxCommandEvent of
	 * type anEVT_SFSOPERATION_FINISHED is generated.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_SFSOPERATION_FINISHED, wxNewEventType())

	/**
	 * Sfs directory changed event.
	 * The content of SfsDirectory has changed. The view needs to be
	 * updated.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_SFSDIR_CHANGED, wxNewEventType())

	/**
	 * Sfs entry error event.
	 *
	 * An error occured while updating one Sfs entrie(s) in a
	 * SfsDirectory.
	 *
	 * Use SfsCtrl::getErrors() to get a list of error-messages.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_SFSENTRY_ERROR, wxNewEventType())

	/**
	 * Policy Change event.
	 * @param GetInt() The priority of the ruleset that changed.
	 * @param GetExtraLong() The user-ID of the ruleset that changed.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_POLICY_CHANGE, wxNewEventType())

	/**
	 * A playground change event.
	 * @param GetExtraLong() The playground ID of the playground.
	 * @param GetString() The command which runs in the playground
	 * @param GetInt() Returns 1 of the playground was started. 0 is
	 *                 returned, if the playground was terminated.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_PG_CHANGE, wxNewEventType())

	/**
	 * Backup Policy event.
	 * This event is sent if a modified policy had to be replaced by
	 * an update version from its source (usually the daemon).
	 * @param GetExtraLong() The ruleset ID of the old ruleset.
	 * The ruleset was locked by the sender of the event and the
	 * consumer must make sure that the event gets unlocked.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_BACKUP_POLICY, wxNewEventType())

	/**
	 * Update perspectives event.
	 * This event is sent and received by NotificationCtrl. It's sent
	 * after a new notification was enqueued. On receipt of this event
	 * the perspectives are updated. With this trick/split we ensure
	 * the observers listening on the perspectives do their update
	 * within the main thread.\n
	 * This event carries the id of the new notification as
	 * ExtraLong (see SetExtraLong() and GetExtraLong()).
	 * Also see Bug #1298 and
	 * http://stackoverflow.com/questions/407004/notifications-in-wxwidgets
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_UPDATE_PERSPECTIVE, wxNewEventType())

	/**
	 * Show SFSBrowser event and automatically select upgraded files.
	 * @param GetInt() Visability of SFSBrowser:
	 *      - = 0 : hide
	 *      - > 0 : show
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_SFSBROWSER_SHOW, wxNewEventType())

	/**
	 * Update Anoubisoptions event.
	 * Updates only the option 'Enable Upgrade Message'
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ANOUBISOPTIONS_UPDATE, wxNewEventType())

	/**
	 * End of an Upgrade. This message is sent if the daemon notifies
	 * xanoubis of an upgrade with at least one upgraded file.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_UPGRADENOTIFY, wxNewEventType())

	/**
	 * Sent by a row provider if the number of elements in the model
	 * changes.
	 * @param GetInt() The new Size.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ROW_SIZECHANGE, wxNewEventType())

	/**
	 * Sent by a row provider if one or more rows in the model changed
	 * and need to be updated.
	 * @param GetInt() The first row that changed.
	 * @param GetExtraLong() The last row that changed. If this
	 *     value is -1 the receiver should check for size changes and
	 *     update all rows from the starting row up to the end of the list.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ROW_UPDATE, wxNewEventType())

	/**
	 * Playground error event.
	 *
	 * An error occured while updating playground or file list.
	 *
	 * Use PlaygroundCrl::getErrors() to get a list of error-messages.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_PLAYGROUND_ERROR, wxNewEventType())

	/**
	 * Playground completed event.
	 *
	 * This event is sent after the completion of certain playground
	 * operations. This can be used to re-enable controls that were
	 * disabled during the operation.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_PLAYGROUND_COMPLETED, wxNewEventType())

	/**
	 * Sent by the NotificationCtrl when notification informs the
	 * us about a program that was forced into a playground environment.
	 * @param GetExtraLong() The id of the notification event.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_PLAYGROUND_FORCED, wxNewEventType())

	/**
	 * Sent by the PSListCtrl if an error occured in a process list
	 * request.
	 * @param None.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_PSLIST_ERROR, wxNewEventType())
END_DECLARE_EVENT_TYPES()
/**
 * @}
 */

#endif	/* _ANEVENTS_H_ */

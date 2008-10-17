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

BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_LOCAL_EVENT_TYPE(anEVT_LOGVIEWER_SHOW, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(anEVT_RULEEDITOR_SHOW, wxNewEventType())
	DECLARE_LOCAL_EVENT_TYPE(anEVT_MAINFRAME_SHOW, wxNewEventType())

	/*
	 * Request the View of the current Notifications due to user interaction
	 * caused by clicking on the TrayIcon or Notification Popup when
	 * Escalations or Alerts were reported by these, to the user of xanoubis
	 *
	 * The integer field contains a boolean stating:
	 * true		=> show Widget
	 *
	 * The string field may contain the following strings stating:
	 * "ESCALATIONS" => Show Notification Tab and current notifications
	 * "ALERTS"	 => Show Notification Tab and current messages
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ESCALATIONS_SHOW, wxNewEventType())

	/*
	 * Request the View of the optional settings of Module Anoubis
	 *
	 * The integer field contains a boolean stating:
	 * true         => show Widget
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ANOUBISOPTIONS_SHOW, wxNewEventType())

	/*
	 * The integer field contains a boolean stating:
	 * true		=> show Widget
	 *
	 * The long  field contains a value stating:
	 * value	=> RuleId which is going to be shown/selected
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_SHOW_RULE, wxNewEventType())

	/*
	 * The options of the system notification via libnotify for ESCALATIONS
	 * or ALERTS have changed.
	 *
	 * The integer field contains a boolean stating:
	 * true
	 * false
	 *
	 * The long  field contains a value stating:
	 * value	=> time in seconds the ESCALATION/ALERT is shown
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ESCALATIONNOTIFY_OPTIONS,
	    wxNewEventType())

	DECLARE_LOCAL_EVENT_TYPE(anEVT_ALERTNOTIFY_OPTIONS,
	    wxNewEventType())

	/*
	 * A connection to the daemon was sucessfully established.
	 * This event is intended of been used between the communicator
	 * thread and communicator control only.
	 *
	 * The integer field contains the connectionState enum stating:
	 * a successfull connection	=> CONNECTION_CONNECTED
	 * normal disconnect		=> CONNECTION_DISCONNECTED
	 * an error (failed to connect)	=> CONNECTION_FAILED
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_COM_CONNECTION, wxNewEventType())

	/*
	 *  Send the new Status for the Auto Checksum Checkbox. The status
	 *  is stored in the int field of the event.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_SEND_AUTO_CHECK, wxNewEventType())

	/*
	 * The count of open ALERTS is stored in the events int field
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_OPEN_ALERTS, wxNewEventType())

	/*
	 * The count of open ESCALATIONS is stored in the events int field
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_OPEN_ESCALATIONS, wxNewEventType())

	/*
	 * The count of open ESCALATIONS for ALF is stored in the events int
	 * field
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_OPEN_ALF_ESCALATIONS, wxNewEventType())

	/*
	 * The count of open ESCALATIONS for SFS is stored in the events int
	 * field
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_OPEN_SFS_ESCALATIONS, wxNewEventType())

	/*
	 * A connection to the daemon was sucessfully established.
	 * The remote station is transmitted as client data.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_COM_REMOTESTATION, wxNewEventType())

	/*
	 * A notification was received. The client data points to the
	 * corresponding client protocol message.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_COM_NOTIFYRECEIVED, wxNewEventType())

	/*
	 * Transports a notification as client object.
	 * A notification will been stored and deleted by ModAnoubis.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ADD_NOTIFICATION, wxNewEventType())

	/*
	 * This event transports an escalation been answered to the
	 * communicator for sending the answer. The escalation is stored
	 * as client object.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ANSWER_ESCALATION, wxNewEventType())

	/*
	 * A new rule set was loaded/stored to the profile manager.
	 * Your current one may be out of date.
	 *
	 * Use GetExtraLong() to get the new rule set id.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_LOAD_RULESET, wxNewEventType())

	/*
	 * Send a Event if the rules of the anoubisd arrived, with the
	 * file name of the tmp-File which holds the rules.
	 * The Int field holds the priority of the ruleset.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ANOUBISD_RULESET_ARRIVED,
	    wxNewEventType())
	/*
	 * Send a Event if the checksum of a file from the shadowtree arrived.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ANOUBISD_CSUM_SHA_ARRIVED, \
	    wxNewEventType())

	/*
	 * Send a Event if the current checksum of a file arrived.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ANOUBISD_CSUM_CUR_ARRIVED, \
	    wxNewEventType())

	/*
	 * Send a Event if the checksum of a file from the shadowtree arrived.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ANOUBISD_CSUM_SHA, \
	    wxNewEventType())

	/*
	 * Send a Event if the current checksum of a file arrived.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_ANOUBISD_CSUM_CUR, \
	    wxNewEventType())

	/*
	 * An error Event from Communicator to CommunicatorCtrl. The Error
	 * Code is stored as an int
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_COMMUNICATOR_ERROR, wxNewEventType())

	/*
	 * An error happend while requesting checksum. The event storing an
	 * string which checksum request failed
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_CHECKSUM_ERROR, wxNewEventType())

	/*
	 * The content of SfsDirectory has changed. The view needs to be
	 * updated.
	 */
	DECLARE_LOCAL_EVENT_TYPE(anEVT_SFSDIR_CHANGED, wxNewEventType())
END_DECLARE_EVENT_TYPES()

#endif	/* _ANEVENTS_H_ */

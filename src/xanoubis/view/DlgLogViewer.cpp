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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <wx/string.h>
#include <wx/imaglist.h>

#include "AnEvents.h"
#include "AnListColumn.h"
#include "AnListCtrl.h"
#include "AnShortcuts.h"
#include "DlgLogViewer.h"
#include "main.h"
#include "Notification.h"
#include "AlertNotify.h"
#include "LogNotify.h"
#include "EscalationNotify.h"
#include "StatusNotify.h"
#include "DaemonAnswerNotify.h"
#include "NotificationCtrl.h"

DlgLogViewer::DlgLogViewer(wxWindow* parent) : DlgLogViewerBase(parent),
    Observer(NULL)
{
	AnEvents		*anEvents;
	NotificationCtrl	*notifyCtrl;
	AnListProperty		*property;
	AnListColumn		*col;

	anEvents = AnEvents::getInstance();
	notifyCtrl = NotificationCtrl::instance();

	shortcuts_  = new AnShortcuts(this);
	this->GetSizer()->Layout();

	property = new LogViewerProperty(LogViewerProperty::PROPERTY_ICON);
	col = logListCtrl->addColumn(property);
	col->setWidth(24);
	col->setAlign(wxLIST_FORMAT_CENTRE);

	property = new LogViewerProperty(LogViewerProperty::PROPERTY_TIME);
	col = logListCtrl->addColumn(property);
	col->setWidth(wxLIST_AUTOSIZE);

	property = new LogViewerProperty(LogViewerProperty::PROPERTY_MODULE);
	col = logListCtrl->addColumn(property);
	col->setWidth(wxLIST_AUTOSIZE);

	property = new LogViewerProperty(LogViewerProperty::PROPERY_MESSAGE);
	col = logListCtrl->addColumn(property);
	col->setWidth(wxLIST_AUTOSIZE);

	logListCtrl->setRowProperty(new NotificationProperty);

	anEvents->Connect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(DlgLogViewer::onShow), NULL, this);
	addSubject(notifyCtrl->getPerspective(NotificationCtrl::LIST_ALL));

	ANEVENTS_IDENT_BCAST_REGISTRATION(DlgLogViewer);
}

DlgLogViewer::~DlgLogViewer(void)
{
	AnEvents	*anEvents;

	anEvents = AnEvents::getInstance();
	anEvents->Disconnect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(DlgLogViewer::onShow), NULL, this);

	ANEVENTS_IDENT_BCAST_DEREGISTRATION(DlgLogViewer);

	delete shortcuts_;
}

void
DlgLogViewer::onShow(wxCommandEvent &event)
{
	this->Show(event.GetInt());
	event.Skip();
}

void
DlgLogViewer::onClose(wxCloseEvent &)
{
	wxCommandEvent	showEvent(anEVT_LOGVIEWER_SHOW);

	showEvent.SetInt(false);
	wxPostEvent(AnEvents::getInstance(), showEvent);
}

void
DlgLogViewer::onLogSelect(wxListEvent &event)
{
	wxCommandEvent		 showEvent(anEVT_SHOW_RULE);
	Notification		*notify;

	notify = dynamic_cast<Notification *>(
	    logListCtrl->getRowAt(event.GetIndex()));

	if (!notify)
		return;

	if (notify->getRuleId() > 0) {
		showEvent.SetInt(notify->isAdmin());
		showEvent.SetExtraLong(notify->getRuleId());
		wxPostEvent(AnEvents::getInstance(), showEvent);
	}
}

void
DlgLogViewer::update(Subject *subject)
{
	wxArrayLong::const_iterator	 it;
	Notification			*notify;
	NotificationCtrl		*notifyCtrl;
	NotificationPerspective		*perspective;

	perspective = wxDynamicCast(subject, NotificationPerspective);
	if (perspective == NULL) {
		return;
	}

	notifyCtrl = NotificationCtrl::instance();
	it = perspective->begin();
	it += logListCtrl->getRowCount();
	while (it != perspective->end()) {
		notify = notifyCtrl->getNotification(*it);
		/* XXX ch: this should be done at notifyCtrl */
		if (notify && IS_DAEMONANSWEROBJ(notify)) {
			/* Nothing */
		} else if (notify && ! IS_STATUSOBJ(notify)) {
			/* show all other notifies than a StatusNotify */
			logListCtrl->addRow(notify);
		}
		it++;
	}
}

void
DlgLogViewer::updateDelete(Subject *subject)
{
	removeSubject(subject);
}

ANEVENTS_IDENT_BCAST_METHOD_DEFINITION(DlgLogViewer)

LogViewerProperty::LogViewerProperty(PropertyRole role)
{
	this->role_ = role;
}

wxString
LogViewerProperty::getHeader(void) const
{
	switch (role_) {
		case PROPERTY_ICON: return wxEmptyString;
		case PROPERTY_TIME: return _("Time");
		case PROPERTY_MODULE: return _("Module");
		case PROPERY_MESSAGE: return _("Message");
	}

	return _("???");
}

wxString
LogViewerProperty::getText(AnListClass *obj) const
{
	Notification *notify = dynamic_cast<Notification *>(obj);

	if (notify == 0)
		return wxT("???");

	switch (role_) {
		case PROPERTY_ICON: return wxEmptyString;
		case PROPERTY_TIME: return notify->getTime();
		case PROPERTY_MODULE: return notify->getModule();
		case PROPERY_MESSAGE: return notify->getLogMessage();
	}

	return _("???");
}

AnIconList::IconId
LogViewerProperty::getIcon(AnListClass *obj) const
{
	if (role_ == PROPERTY_ICON) {
		Notification *notify = dynamic_cast<Notification *>(obj);

		if (typeid(*notify) == typeid(class EscalationNotify)) {
			return AnIconList::ICON_ANOUBIS_QUESTION;
		} else if (typeid(*notify) == typeid(class AlertNotify)) {
			return AnIconList::ICON_ANOUBIS_ALERT;
		} else {
			return AnIconList::ICON_ANOUBIS_BLACK;
		}
	} else
		return AnIconList::ICON_NONE;
}

wxColour
NotificationProperty::getBackgroundColor(AnListClass *obj) const
{
	Notification *notify = dynamic_cast<Notification *>(obj);

	if ((notify != 0) && notify->isAdmin())
		return wxTheColourDatabase->Find(wxT("LIGHT GREY"));
	else
		return (wxNullColour);
}

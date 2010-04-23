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
#include "DlgLogViewer.h"
#include "main.h"
#include "Notification.h"
#include "AlertNotify.h"
#include "LogNotify.h"
#include "EscalationNotify.h"
#include "StatusNotify.h"
#include "DaemonAnswerNotify.h"
#include "NotificationCtrl.h"

/**
 * Implement an AnRowProvider for the DlgLogViewer. This class simply
 * implements the AnRowProvider and the Observer interface around the
 * data contained in the LIST_ALL perspective.
 */
class LogProvider : public AnRowProvider, public Observer {
private:
	NotificationPerspective		*perspective_;
public:
	/**
	 * Constructor: Initialize perspective_ and add observe it.
	 */
	LogProvider(void) : Observer(NULL) {
		NotificationCtrl		*notifyCtrl;

		notifyCtrl = NotificationCtrl::instance();
		perspective_ = notifyCtrl->getPerspective(
		    NotificationCtrl::LIST_ALL);
		addSubject(perspective_);
	};
	/**
	 * Implementation of AnRowProvider::getSize().
	 */
	int		 getSize(void) const {
		if (!perspective_)
			return 0;
		return perspective_->getSize();
	};
	/**
	 * Implementation of AnRowProvider::getRow()
	 */
	AnListClass	*getRow(unsigned int idx) const {
		NotificationCtrl		*notifyCtrl;
		wxArrayLong::const_iterator	 it;

		if (perspective_ == NULL)
			return NULL;
		if ((int)idx < 0 || (int)idx >= perspective_->getSize())
			return NULL;
		it = perspective_->begin();
		it += idx;
		notifyCtrl = NotificationCtrl::instance();
		return notifyCtrl->getNotification(*it);
	};
	/**
	 * Implementation of Observer::update().
	 */
	void update(Subject *) {
		rowChangeEvent(0, -1);
	}
	/**
	 * Implementation of Observer::updateDelete().
	 */
	void updateDelete(Subject *subject) {
		removeSubject(subject);
		perspective_ = NULL;
		rowChangeEvent(0, -1);
	}
};

DlgLogViewer::DlgLogViewer(wxWindow* parent) : DlgLogViewerBase(parent)
{
	AnEvents		*anEvents;
	NotificationCtrl	*notifyCtrl;
	AnListProperty		*property;
	AnListColumn		*col;

	anEvents = AnEvents::getInstance();
	notifyCtrl = NotificationCtrl::instance();

	this->GetSizer()->Layout();

	logListCtrl->setRowProvider(new LogProvider);
	logListCtrl->setStateKey(wxT("/State/DlgLogViewer"));

	property = new LogViewerProperty(LogViewerProperty::PROPERTY_ICON);
	col = logListCtrl->addColumn(property);
	col->setWidth(24);
	col->setAlign(wxLIST_FORMAT_CENTRE);

	property = new LogViewerProperty(LogViewerProperty::PROPERTY_TIME);
	col = logListCtrl->addColumn(property);
	col->setWidth(200); /* Adjust column width (Bug #1321). */

	property = new LogViewerProperty(LogViewerProperty::PROPERTY_MODULE);
	col = logListCtrl->addColumn(property);
	col->setWidth(wxLIST_AUTOSIZE);

	property = new LogViewerProperty(LogViewerProperty::PROPERY_MESSAGE);
	col = logListCtrl->addColumn(property);
	col->setWidth(385); /* Adjust column width (Bug #1321). */

	logListCtrl->setRowProperty(new NotificationProperty);

	anEvents->Connect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(DlgLogViewer::onShow), NULL, this);

	ANEVENTS_IDENT_BCAST_REGISTRATION(DlgLogViewer);
}

DlgLogViewer::~DlgLogViewer(void)
{
	AnEvents	*anEvents;

	anEvents = AnEvents::getInstance();
	anEvents->Disconnect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(DlgLogViewer::onShow), NULL, this);

	ANEVENTS_IDENT_BCAST_DEREGISTRATION(DlgLogViewer);
	logListCtrl->setRowProvider(NULL);
}

void
DlgLogViewer::onShow(wxCommandEvent &event)
{
	wxCommandEvent	showEvent(anEVT_OPEN_ALERTS);
	showEvent.SetInt(0);
	showEvent.SetExtraLong(false);
	wxPostEvent(AnEvents::getInstance(), showEvent);

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
	wxCommandEvent			 showEvent(anEVT_SHOW_RULE);
	Notification			*notify;
	wxArrayLong::const_iterator	 it;
	NotificationCtrl		*notifyCtrl;
	NotificationPerspective		*perspective;

	notifyCtrl = NotificationCtrl::instance();
	perspective = notifyCtrl->getPerspective(NotificationCtrl::LIST_ALL);
	it = perspective->begin();
	it += event.GetIndex();
	notify = notifyCtrl->getNotification(*it);

	if (!notify)
		return;

	if (notify->getRuleId() > 0) {
		showEvent.SetInt(notify->isAdmin());
		showEvent.SetExtraLong(notify->getRuleId());
		wxPostEvent(AnEvents::getInstance(), showEvent);
	}
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

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

#define LOGVIEWER_COLUMN_ICON		0
#define LOGVIEWER_COLUMN_TIME		1
#define LOGVIEWER_COLUMN_MODULE		2
#define LOGVIEWER_COLUMN_MESSAGE	3

DlgLogViewer::DlgLogViewer(wxWindow* parent) : DlgLogViewerBase(parent),
    Observer(NULL)
{
	wxIcon			*icon;
	wxImageList		*imgList;
	AnEvents		*anEvents;
	NotificationCtrl	*notifyCtrl;

	anEvents = AnEvents::getInstance();
	notifyCtrl = NotificationCtrl::instance();

	imgList = new wxImageList(16, 16);
	shortcuts_  = new AnShortcuts(this);
	this->GetSizer()->Layout();

	lastIdx_ = 0;

	/* load icons */
	icon = wxGetApp().loadIcon(wxT("ModAnoubis_black_16.png"));
	defaultIconIdx_ = imgList->Add(*icon);

	icon = wxGetApp().loadIcon(wxT("ModAnoubis_alert_16.png"));
	alertIconIdx_ = imgList->Add(*icon);

	icon = wxGetApp().loadIcon(wxT("ModAnoubis_question_16.png"));
	escalationIconIdx_ = imgList->Add(*icon);

	logListCtrl->AssignImageList(imgList, wxIMAGE_LIST_SMALL);

	/* create columns */
	logListCtrl->InsertColumn(LOGVIEWER_COLUMN_ICON, wxT(""),
	    wxLIST_FORMAT_LEFT, 24);

	logListCtrl->InsertColumn(LOGVIEWER_COLUMN_TIME, _("Time"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);

	logListCtrl->InsertColumn(LOGVIEWER_COLUMN_MODULE, _("Module"),
	    wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);

	logListCtrl->InsertColumn(LOGVIEWER_COLUMN_MESSAGE, _("Message"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);

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
	NotificationCtrl	*notifyCtrl;

	notifyCtrl = NotificationCtrl::instance();
	notify = notifyCtrl->getNotification(event.GetData());

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
	it += lastIdx_;
	while (it != perspective->end()) {
		notify = notifyCtrl->getNotification(*it);
		/* XXX ch: this should be done at notifyCtrl */
		if (notify && IS_DAEMONANSWEROBJ(notify)) {
			/* Nothing */
		} else if (notify && ! IS_STATUSOBJ(notify)) {
			/* show all other notifies than a StatusNotify */
			addNotification(notify);
		}
		it++;
	}
	lastIdx_ = perspective->getSize();
}

void
DlgLogViewer::updateDelete(Subject *subject)
{
	removeSubject(subject);
}

void
DlgLogViewer::addNotification(Notification *notify)
{
	int listIdx;
	int iconIdx;

	listIdx = logListCtrl->GetItemCount();
	if (typeid(*notify) == typeid(class EscalationNotify)) {
		iconIdx = escalationIconIdx_;
	} else if (typeid(*notify) == typeid(class AlertNotify)) {
		iconIdx = alertIconIdx_;
	} else {
		iconIdx = defaultIconIdx_;
	}

	logListCtrl->InsertItem(listIdx, wxEmptyString, iconIdx);
	logListCtrl->SetItem(listIdx, LOGVIEWER_COLUMN_TIME,
	    notify->getTime());
	logListCtrl->SetItem(listIdx, LOGVIEWER_COLUMN_MODULE,
	    notify->getModule());
	logListCtrl->SetItem(listIdx, LOGVIEWER_COLUMN_MESSAGE,
	    notify->getLogMessage());
	logListCtrl->SetItemData(listIdx, notify->getId());

	if (notify->isAdmin()) {
		logListCtrl->SetItemBackgroundColour(listIdx,
		    wxTheColourDatabase->Find(wxT("LIGHT GREY")));
	}

	/* trigger new calculation of column width (all icons has same size) */
	logListCtrl->SetColumnWidth(LOGVIEWER_COLUMN_TIME, wxLIST_AUTOSIZE);
	logListCtrl->SetColumnWidth(LOGVIEWER_COLUMN_MODULE, wxLIST_AUTOSIZE);
	logListCtrl->SetColumnWidth(LOGVIEWER_COLUMN_MESSAGE, wxLIST_AUTOSIZE);
}

ANEVENTS_IDENT_BCAST_METHOD_DEFINITION(DlgLogViewer)

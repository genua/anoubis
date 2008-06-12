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

#define LOGVIEWER_COLUMN_ICON		0
#define LOGVIEWER_COLUMN_TIME		1
#define LOGVIEWER_COLUMN_MODULE		2
#define LOGVIEWER_COLUMN_MESSAGE	3

DlgLogViewer::DlgLogViewer(wxWindow* parent) : DlgLogViewerBase(parent)
{
	wxIcon		*icon;
	wxImageList	*imgList;

	imgList = new wxImageList(16, 16);
	shortcuts_  = new AnShortcuts(this);
	this->GetSizer()->Layout();

	defaultIconIdx_ = imgList->Add(wxNullIcon);
	icon = wxGetApp().loadIcon(wxT("ModAnoubis_alert_16.png"));
	alertIconIdx_ = imgList->Add(*icon);

	icon = wxGetApp().loadIcon(wxT("ModAnoubis_question_16.png"));
	escalationIconIdx_ = imgList->Add(*icon);
	lc_logList->AssignImageList(imgList, wxIMAGE_LIST_SMALL);

	lc_logList->InsertColumn(LOGVIEWER_COLUMN_ICON, wxT(""),
	    wxLIST_FORMAT_LEFT, 24);

	lc_logList->InsertColumn(LOGVIEWER_COLUMN_TIME, _("Time"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);

	lc_logList->InsertColumn(LOGVIEWER_COLUMN_MODULE, _("Module"),
	    wxLIST_FORMAT_CENTER, wxLIST_AUTOSIZE);

	lc_logList->InsertColumn(LOGVIEWER_COLUMN_MESSAGE, _("Message"),
	    wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE);

	parent->Connect(anEVT_ADD_NOTIFICATION,
	    wxCommandEventHandler(DlgLogViewer::OnAddNotification), NULL, this);
	parent->Connect(anEVT_LOGVIEWER_SHOW,
	    wxCommandEventHandler(DlgLogViewer::OnShow), NULL, this);
}

DlgLogViewer::~DlgLogViewer(void)
{
	delete shortcuts_;
}

void
DlgLogViewer::OnAddNotification(wxCommandEvent& event)
{
	Notification *notify;

	notify = (Notification *)(event.GetClientObject());
	if (!IS_STATUSOBJ(notify)) {
		/* show all other notifies than a StatusNotify */
		addNotification(notify);
	}
	/*
	 * The notify was also received by ModAnoubis, which stores
	 * and destroys it. Thus we must not delete it.
	 */
}

void
DlgLogViewer::addNotification(Notification *notify)
{
	int listIdx;
	int iconIdx;

	listIdx = lc_logList->GetItemCount();
	if (typeid(*notify) == typeid(class EscalationNotify)) {
		iconIdx = escalationIconIdx_;
	} else if (typeid(*notify) == typeid(class AlertNotify)) {
		iconIdx = alertIconIdx_;
	} else {
		iconIdx = defaultIconIdx_;
	}

	lc_logList->InsertItem(listIdx, wxEmptyString, iconIdx);
	lc_logList->SetItem(listIdx, LOGVIEWER_COLUMN_TIME, notify->getTime());
	lc_logList->SetItem(listIdx, LOGVIEWER_COLUMN_MODULE,
	    notify->getModule());
	lc_logList->SetItem(listIdx, LOGVIEWER_COLUMN_MESSAGE,
	    notify->getLogMessage());
	lc_logList->SetItemPtrData(listIdx, (wxUIntPtr)notify);

	/* trigger new calculation of column width (all icons has same size) */
	lc_logList->SetColumnWidth(LOGVIEWER_COLUMN_TIME, wxLIST_AUTOSIZE);
	lc_logList->SetColumnWidth(LOGVIEWER_COLUMN_MODULE, wxLIST_AUTOSIZE);
	lc_logList->SetColumnWidth(LOGVIEWER_COLUMN_MESSAGE, wxLIST_AUTOSIZE);
}

void
DlgLogViewer::OnClose(wxCloseEvent& event)
{
	wxCommandEvent	showEvent(anEVT_LOGVIEWER_SHOW);
	showEvent.SetInt(false);
	wxGetApp().sendEvent(showEvent);
}

void
DlgLogViewer::OnShow(wxCommandEvent& event)
{
	this->Show(event.GetInt());
}

void
DlgLogViewer::OnListItemSelected(wxListEvent& event)
{
	Notification	*notify;

	notify = (Notification*)event.GetData();

	if (!notify)
		return;

	wxCommandEvent  showEvent(anEVT_SHOW_RULE);
	showEvent.SetInt(true);
	showEvent.SetExtraLong(notify->getRuleId());
	wxGetApp().sendEvent(showEvent);
}

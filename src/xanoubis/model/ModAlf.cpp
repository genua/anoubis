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

#include <wx/stdpaths.h>
#include <wx/string.h>

#include "Module.h"
#include "ModAlf.h"
#include "ModAlfMainPanelImpl.h"
#include "ModAlfOverviewPanelImpl.h"
#include "Notification.h"
#include "NotificationCtrl.h"
#include "StatusNotify.h"

ModAlf::ModAlf(wxWindow *parent) : Module(), Observer(NULL)
{
	NotificationCtrl	*notifyCtrl;

	notifyCtrl = NotificationCtrl::instance();

	name_ = wxString(wxT("Application Level Firewall"));
	nick_ = wxString(wxT("ALF"));
	mainPanel_ = new ModAlfMainPanelImpl(parent,
	    MODALF_ID_MAINPANEL);
	overviewPanel_ = new ModAlfOverviewPanelImpl(parent,
	    MODALF_ID_OVERVIEWPANEL);

	isActive_ = false;

	loadIcon(wxT("ModAlf_black_48.png"));
	mainPanel_->Hide();
	overviewPanel_->Hide();

	addSubject(notifyCtrl->getPerspective(NotificationCtrl::LIST_STAT));
}

ModAlf::~ModAlf(void)
{
	removeSubject(NotificationCtrl::instance()->getPerspective(
	    NotificationCtrl::LIST_STAT));
	delete mainPanel_;
	delete overviewPanel_;
	delete icon_;
}

int
ModAlf::getBaseId(void)
{
	return (MODALF_ID_BASE);
}

int
ModAlf::getToolbarId(void)
{
	return (MODALF_ID_TOOLBAR);
}

void
ModAlf::update(void)
{
	ModAlfOverviewPanelImpl *instance =
	    (ModAlfOverviewPanelImpl*) overviewPanel_;

	if(instance)
		instance->update();
}

void
ModAlf::update(Subject *subject)
{
	wxArrayLong::const_iterator it;

	Notification		*notify;
	NotificationCtrl	*notifyCtrl;
	NotificationPerspective	*perspective;

	perspective = wxDynamicCast(subject, NotificationPerspective);
	if (perspective == NULL) {
		return;
	}

	notifyCtrl = NotificationCtrl::instance();
	it = perspective->begin();
	notify = notifyCtrl->getNotification(*it);
	if (notify && IS_STATUSOBJ(notify)) {
		isActive_ = ((StatusNotify *)notify)->hasAlfLoadtime();
	}

	update();
}

void
ModAlf::updateDelete(Subject *subject)
{
	removeSubject(subject);
	isActive_ = false;
	update();
}

bool
ModAlf::isActive(void)
{
	return (isActive_);
}

/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#include "AnIconList.h"
#include "Module.h"
#include "ModPlayground.h"
#include "ModPlaygroundMainPanelImpl.h"
#include "ModPlaygroundOverviewPanelImpl.h"
#include "Notification.h"
#include "NotificationCtrl.h"
#include "StatusNotify.h"

ModPlayground::ModPlayground(wxWindow *parent) : Module(), Observer(NULL)
{
	NotificationCtrl	*notifyCtrl;

	notifyCtrl = NotificationCtrl::instance();

	name_ = wxString(wxT("Playground"));
	nick_ = wxString(wxT("PG"));
	mainPanel_ = new ModPlaygroundMainPanelImpl(parent,
	    MODPG_ID_MAINPANEL);
	overviewPanel_ = new ModPlaygroundOverviewPanelImpl(parent,
	    MODPG_ID_OVERVIEWPANEL);

	isActive_ = false;

	mainPanel_->Hide();
	overviewPanel_->Hide();

	addSubject(notifyCtrl->getPerspective(NotificationCtrl::LIST_STAT));
	addSubject(
	    dynamic_cast<ModPlaygroundOverviewPanelImpl *>(overviewPanel_));
}

int
ModPlayground::getBaseId(void)
{
	return (MODPG_ID_BASE);
}

int
ModPlayground::getToolbarId(void)
{
	return (MODPG_ID_TOOLBAR);
}

wxIcon *
ModPlayground::getIcon(void) const
{
	return AnIconList::instance()->getIcon(AnIconList::ICON_PG_BLACK_48);
}

void
ModPlayground::update(void)
{
	ModPlaygroundOverviewPanelImpl	*inst = NULL;

	if (overviewPanel_) {
		inst = dynamic_cast<ModPlaygroundOverviewPanelImpl *>(
		    overviewPanel_);
	}

	if(inst)
		inst->update();
}

void
ModPlayground::update(Subject *subject)
{
	Notification		*notify;
	NotificationCtrl	*notifyCtrl;
	NotificationPerspective	*perspective;

	perspective = dynamic_cast<NotificationPerspective*>(subject);
	if (perspective == NULL) {
		return;
	}

	notifyCtrl = NotificationCtrl::instance();
	notify = notifyCtrl->getNotification(perspective->getId(0));
	if (notify && IS_STATUSOBJ(notify)) {
		isActive_ = ((StatusNotify *)notify)->hasPgLoadtime();
	}

	update();
}

void
ModPlayground::updateDelete(Subject *subject)
{
	if (subject && subject ==
	    dynamic_cast<ModPlaygroundOverviewPanelImpl *>(overviewPanel_)) {
		overviewPanel_ = NULL;
		return;
	}
	isActive_ = false;
	update();
}

bool
ModPlayground::isActive(void)
{
	return (isActive_);
}

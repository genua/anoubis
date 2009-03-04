/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#include "AnEvents.h"
#include "Module.h"
#include "ModSb.h"
#include "ModSbMainPanelImpl.h"
#include "ModSbOverviewPanelImpl.h"
#include "Notification.h"
#include "StatusNotify.h"

ModSb::ModSb(wxWindow *parent) : Module()
{
	name_ = wxString(wxT("Sandbox"));
	nick_ = wxString(wxT("SB"));
	mainPanel_ = new ModSbMainPanelImpl(parent, MODSB_ID_MAINPANEL);
	overviewPanel_ = new ModSbOverviewPanelImpl(parent,
	    MODSB_ID_OVERVIEWPANEL);

	isActive_ = false;

	loadIcon(wxT("ModSb_black_48.png"));
	mainPanel_->Hide();
	overviewPanel_->Hide();

	AnEvents::getInstance()->Connect(anEVT_ADD_NOTIFICATION,
	    wxCommandEventHandler(ModSb::OnAddNotification), NULL, this);
}

ModSb::~ModSb()
{
	AnEvents::getInstance()->Disconnect(anEVT_ADD_NOTIFICATION,
	    wxCommandEventHandler(ModSb::OnAddNotification), NULL, this);

	delete mainPanel_;
	mainPanel_ = NULL;
	delete overviewPanel_;
	overviewPanel_ = NULL;
	delete icon_;
	icon_ = NULL;
}

int
ModSb::getBaseId(void)
{
	return (MODSB_ID_BASE);
}

int
ModSb::getToolbarId(void)
{
	return (MODSB_ID_TOOLBAR);
}

void
ModSb::update(void)
{
	if (overviewPanel_ != NULL) {
		((ModSbOverviewPanelImpl *)overviewPanel_)->update();
	}
}

void
ModSb::OnAddNotification(wxCommandEvent& event)
{
	Notification *notify;

	notify = (Notification *)(event.GetClientObject());

	/*
	 * ModSB is considered to be running when ModSFS does.
	 */
	if (IS_STATUSOBJ(notify)) {
		isActive_ = ((StatusNotify *)notify)->hasSfsLoadtime();
	}
	update();

	event.Skip();
}

bool
ModSb::isActive(void)
{
	return (isActive_);
}

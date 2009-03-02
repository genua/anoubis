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

#include <wx/stdpaths.h>
#include <wx/string.h>

#include "AnEvents.h"
#include "Module.h"
#include "ModSfs.h"
#include "ModSfsMainPanelImpl.h"
#include "ModSfsOverviewPanelImpl.h"
#include "Notification.h"
#include "StatusNotify.h"

ModSfs::ModSfs(wxWindow *parent) : Module()
{
	name_ = wxString(wxT("Secure File System"));
	nick_ = wxString(wxT("SFS"));
	mainPanel_ = new ModSfsMainPanelImpl(parent,
	    MODSFS_ID_MAINPANEL);
	overviewPanel_ = new ModSfsOverviewPanelImpl(parent,
	    MODSFS_ID_OVERVIEWPANEL);

	isActive_ = false;

	loadIcon(wxT("ModSfs_black_48.png"));
	mainPanel_->Hide();
	overviewPanel_->Hide();

	AnEvents::getInstance()->Connect(anEVT_ADD_NOTIFICATION,
	    wxCommandEventHandler(ModSfs::OnAddNotification), NULL, this);
}

ModSfs::~ModSfs(void)
{
	AnEvents::getInstance()->Disconnect(anEVT_ADD_NOTIFICATION,
	    wxCommandEventHandler(ModSfs::OnAddNotification), NULL, this);

	delete mainPanel_;
	mainPanel_ = NULL;
	delete overviewPanel_;
	overviewPanel_ = NULL;
	delete icon_;
	icon_ = NULL;
}

int
ModSfs::getBaseId(void)
{
	return (MODSFS_ID_BASE);
}

int
ModSfs::getToolbarId(void)
{
	return (MODSFS_ID_TOOLBAR);
}

void
ModSfs::update(void)
{
	if (overviewPanel_ != NULL) {
		((ModSfsOverviewPanelImpl *)overviewPanel_)->update();
	}
}

void
ModSfs::OnAddNotification(wxCommandEvent& event)
{
	Notification *notify;

	notify = (Notification *)(event.GetClientObject());
	if (IS_STATUSOBJ(notify)) {
		isActive_ = ((StatusNotify *)notify)->hasSfsLoadtime();
	}
	update();

	event.Skip();
}

bool
ModSfs::isActive(void)
{
	return (isActive_);
}

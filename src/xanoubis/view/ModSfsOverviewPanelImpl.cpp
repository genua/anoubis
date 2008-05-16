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

#include <wx/icon.h>

#include "main.h"
#include "ModSfs.h"
#include "ModSfsOverviewPanelImpl.h"

ModSfsOverviewPanelImpl::ModSfsOverviewPanelImpl(wxWindow* parent,
    wxWindowID id) : ModSfsOverviewPanelBase(parent, id)
{
	stateIconNormal_ = wxGetApp().loadIcon(_T("ModSfs_ok_48.png"));
	stateIconError_ = wxGetApp().loadIcon(_T("ModSfs_error_48.png"));
	stateIconNotConnected_ = wxGetApp().loadIcon(_T("ModSfs_black_48.png"));
	notAnswered_.Printf(_T("%d"), 0);

	parent->Connect(anEVT_OPEN_SFS_ESCALATIONS,
            wxCommandEventHandler(ModSfsOverviewPanelImpl::OnOpenSfsEscalation), NULL, this);
}

ModSfsOverviewPanelImpl::~ModSfsOverviewPanelImpl(void)
{
	delete stateIconNormal_;
	delete stateIconError_;
	delete stateIconNotConnected_;
}

void
ModSfsOverviewPanelImpl::update(void)
{
	wxString	 stateText;
	wxIcon		*stateIcon;
	ModSfs		*module;

	module = (ModSfs *)(wxGetApp().getModule(SFS));
	if (!wxGetApp().getCommConnectionState()) {
		stateText = _("not connected");
		stateIcon = stateIconNotConnected_;
	} else {
		if (module->isActive()) {
			stateText = _("ok");
			stateIcon = stateIconNormal_;
		} else {
			stateText = _("not active");
			stateIcon = stateIconError_;
		}
	}

	txt_statusValue->SetLabel(stateText);
	txt_requestValue->SetLabel(notAnswered_);
	sfsStatusIcon->SetIcon(*stateIcon);
}

void
ModSfsOverviewPanelImpl::OnOpenSfsEscalation(wxCommandEvent& event)
{
	notAnswered_.Printf(_T("%d"), event.GetInt());
	update();
}

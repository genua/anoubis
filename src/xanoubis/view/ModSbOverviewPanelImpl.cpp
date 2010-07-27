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

#include <wx/icon.h>

#include "JobCtrl.h"
#include "MainUtils.h"
#include "ModSb.h"
#include "ModSbOverviewPanelImpl.h"
#include "ModSfs.h"

ModSbOverviewPanelImpl::ModSbOverviewPanelImpl(wxWindow* parent,
    wxWindowID id) : ModSbOverviewPanelBase(parent, id)
{
	MainUtils *utils = MainUtils::instance();

	stateIconNormal_ = utils->loadIcon(wxT("ModSb_ok_48.png"));
	stateIconError_ = utils->loadIcon(wxT("ModSb_error_48.png"));
	stateIconNotConnected_ = utils->loadIcon(wxT("ModSb_black_48.png"));
	notAnswered_.Printf(wxT("%d"), 0);

	AnEvents::instance()->Connect(anEVT_OPEN_SB_ESCALATIONS,
	    wxCommandEventHandler(ModSbOverviewPanelImpl::OnOpenSbEscalation),
	    NULL, this);
}

ModSbOverviewPanelImpl::~ModSbOverviewPanelImpl(void)
{
	AnEvents::instance()->Disconnect(anEVT_OPEN_SB_ESCALATIONS,
	    wxCommandEventHandler(ModSbOverviewPanelImpl::OnOpenSbEscalation),
	    NULL, this);

	delete stateIconNormal_;
	delete stateIconError_;
	delete stateIconNotConnected_;
}

void
ModSbOverviewPanelImpl::update(void)
{
	wxString	stateText;
	wxIcon		*stateIcon;
	ModSb		*module;

	module = (ModSb *)(MainUtils::instance()->getModule(SB));
	if (!JobCtrl::instance()->isConnected()) {
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
	sbStatusIcon->SetIcon(*stateIcon);
}

void ModSbOverviewPanelImpl::OnOpenSbEscalation(wxCommandEvent& event)
{
	notAnswered_.Printf(wxT("%d"), event.GetInt());
	update();
	event.Skip();
}

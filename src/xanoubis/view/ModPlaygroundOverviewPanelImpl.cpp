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

#include <wx/icon.h>

#include "JobCtrl.h"
#include "MainUtils.h"
#include "ModPlayground.h"
#include "ModPlaygroundOverviewPanelImpl.h"


ModPlaygroundOverviewPanelImpl::ModPlaygroundOverviewPanelImpl(wxWindow* parent,
    wxWindowID id) : ModPlaygroundOverviewPanelBase(parent, id)
{
	MainUtils *utils = MainUtils::instance();

	stateIconNormal_ = utils->loadIcon(wxT("ModPlayground_ok_48.png"));
	stateIconError_ = utils->loadIcon(wxT("ModPlayground_error_48.png"));
	stateIconNotConnected_ = utils->loadIcon(
	    wxT("ModPlayground_black_48.png"));
	runningPgs_.Printf(wxT("%d"), 0);
}

ModPlaygroundOverviewPanelImpl::~ModPlaygroundOverviewPanelImpl(void)
{
	delete stateIconNormal_;
	delete stateIconError_;
	delete stateIconNotConnected_;
}

void
ModPlaygroundOverviewPanelImpl::update(void)
{
	wxString	 stateText;
	wxIcon		*stateIcon;
	ModPlayground	*module;

	module = (ModPlayground *)(MainUtils::instance()->getModule(PG));

	if (!JobCtrl::getInstance()->isConnected()) {
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

	statusText->SetLabel(stateText);
	countText->SetLabel(runningPgs_);
	statusIcon->SetIcon(*stateIcon);
}

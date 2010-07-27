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

#include "VersionListCtrl.h"
#include "VersionCtrl.h"
#include "ApnVersion.h"
#include "ModAnoubisMainPanelImpl.h"
#include "VersionCtrl.h"

static wxString getAutoStore(bool autostore)
{
	return autostore ? _("Auto") : _("Manual");
}

VersionListCtrl::VersionListCtrl(wxWindow *w, wxWindowID id,
    const wxPoint &p, const wxSize &sz, long type)
    : AnListCtrl(w, id, p, sz, type | wxLC_VIRTUAL)
{
	setRowProvider(VersionCtrl::instance());
	setStateKey(wxT("/State/VersionListCtrl"));

	/* Setup properties of the view */
	addColumn(new AnFmtListProperty<ApnVersion, bool>(
	    _("Type"), &ApnVersion::isAutoStore, NULL, &getAutoStore));
	addColumn(new AnFmtListProperty<ApnVersion, wxDateTime>(
	    _("Date"), &ApnVersion::getTimestamp, NULL,
	    &DefaultConversions::toDate));
	addColumn(new AnFmtListProperty<ApnVersion, wxDateTime>(
	    _("Time"), &ApnVersion::getTimestamp, NULL,
	    &DefaultConversions::toTime));
	addColumn(new AnFmtListProperty<ApnVersion>(
	    _("User"), &ApnVersion::getUsername));
	addColumn(new AnFmtListProperty<ApnVersion, int>(
	    _("Version"), &ApnVersion::getVersionNo));
}

void
VersionListCtrl::update(void)
{
	VersionCtrl *ctrl = VersionCtrl::instance();
	int	nSize = ctrl->getNumVersions();

	SetItemCount(nSize);
	RefreshItems(0, nSize-1);
}

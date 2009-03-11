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

VersionListCtrl::VersionListCtrl(wxWindow *w, wxWindowID id,
    const wxPoint &p, const wxSize &sz, long type)
    : wxListCtrl(w, id, p, sz, type)
{
	error_ = wxEmptyString;
	savedWidth_ = 0;
}

void
VersionListCtrl::setError(const wxString &error)
{
	if (error != wxEmptyString) {
		SetItemCount(1);
		if (savedWidth_ == 0) {
			savedWidth_ =
			    GetColumnWidth(MODANOUBIS_VMLIST_COLUMN_DATE);
			SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_DATE, 350);
		}
	} else {
		if (savedWidth_) {
			SetColumnWidth(MODANOUBIS_VMLIST_COLUMN_DATE,
			    savedWidth_);
			savedWidth_ = 0;
		}
	}
	error_ = error;
	RefreshItem(0);
}

wxString
VersionListCtrl::OnGetItemText(long idx, long col) const
{
	VersionCtrl		*versionCtrl = VersionCtrl::getInstance();
	const ApnVersion	&version = versionCtrl->getVersion(idx);
	wxString		 res = wxEmptyString;

	if (error_ != wxEmptyString) {
		if (idx == 0 && col == MODANOUBIS_VMLIST_COLUMN_DATE) {
			res = error_;
		}
	} else {
		switch(col) {
		case MODANOUBIS_VMLIST_COLUMN_TYPE:
			res =  version.isAutoStore() ? _("Auto") : _("Manual");
			break;
		case MODANOUBIS_VMLIST_COLUMN_DATE:
			res =  version.getTimestamp().FormatDate();
			break;
		case MODANOUBIS_VMLIST_COLUMN_TIME:
			res = version.getTimestamp().FormatTime();
			break;
		case MODANOUBIS_VMLIST_COLUMN_USER:
			res = version.getUsername();
			break;
		case MODANOUBIS_VMLIST_COLUMN_VERSION:
			res = wxString::Format(wxT("%d"),
			    version.getVersionNo());
			break;
		}
	}
	return res;
}

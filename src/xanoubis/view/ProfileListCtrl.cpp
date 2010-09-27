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

#include "ProfileListCtrl.h"
#include "PolicyCtrl.h"
#include "ModAnoubisMainPanelImpl.h"

ProfileListCtrl::ProfileListCtrl(wxWindow *w, wxWindowID id,
    const wxPoint &p, const wxSize &sz, long type)
    : AnListCtrl(w, id, p, sz, type | wxLC_VIRTUAL)
{
	setRowProvider(PolicyCtrl::instance());
	setStateKey(wxT("/State/ProfileListCtrl"));

	/* Setup properties of the view */
	addColumn(new ProfilePermissionProperty, 140);
	addColumn(new ProfileNameProperty, 384);
}

AnIconList::IconId
ProfileProperty::getIcon(AnListClass *) const
{
	return (AnIconList::ICON_NONE);
}

wxString
ProfileNameProperty::getHeader(void) const
{
	return _("Profile");
}

wxString
ProfileNameProperty::getText(AnListClass *obj) const
{
	Profile *profile = dynamic_cast<Profile *>(obj);

	if (profile != 0)
		return profile->getProfileName();
	else
		return _("???");
}

wxString
ProfilePermissionProperty::getHeader(void) const
{
	return wxEmptyString;
}

wxString
ProfilePermissionProperty::getText(AnListClass *obj) const
{
	Profile *profile = dynamic_cast<Profile *>(obj);
	PolicyCtrl *ctrl = PolicyCtrl::instance();

	if (!ctrl->isProfileWritable(profile->getProfileName()))
		return _("read-only");
	else
		return wxEmptyString;
}

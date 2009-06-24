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

VersionListCtrl::VersionListCtrl(wxWindow *w, wxWindowID id,
    const wxPoint &p, const wxSize &sz, long type)
    : AnListCtrl(w, id, p, sz, type | wxLC_VIRTUAL)
{
	/* Setup properties of the view */
	addColumn(new VersionTypeProperty);
	addColumn(new VersionDateProperty);
	addColumn(new VersionTimeProperty);
	addColumn(new VersionUsernameProperty);
	addColumn(new VersionNoProperty);
}

void
VersionListCtrl::update(void)
{
	VersionCtrl *ctrl = VersionCtrl::getInstance();

	/* Remove and... */
	clearRows();

	/* reassign versions to view */
	unsigned int count = ctrl->getNumVersions();
	for (unsigned int i = 0; i < count; i++) {
		addRow(ctrl->getVersion(i));
	}
}

AnIconList::IconId
ApnVersionProperty::getIcon(AnListClass *) const
{
	return (AnIconList::ICON_NONE);
}

wxString
VersionTypeProperty::getHeader(void) const
{
	return _("Type");
}

wxString
VersionTypeProperty::getText(AnListClass *obj) const
{
	ApnVersion *version = dynamic_cast<ApnVersion *>(obj);

	if (version != 0)
		return (version->isAutoStore() ? _("Auto") : _("Manual"));
	else
		return _("???");
}

wxString
VersionDateProperty::getHeader(void) const
{
	return _("Date");
}

wxString
VersionDateProperty::getText(AnListClass *obj) const
{
	ApnVersion *version = dynamic_cast<ApnVersion *>(obj);

	if (version != 0)
		return (version->getTimestamp().FormatDate());
	else
		return _("???");
}

wxString
VersionTimeProperty::getHeader(void) const
{
	return _("Time");
}

wxString
VersionTimeProperty::getText(AnListClass *obj) const
{
	ApnVersion *version = dynamic_cast<ApnVersion *>(obj);

	if (version != 0)
		return (version->getTimestamp().FormatTime());
	else
		return _("???");
}

wxString
VersionUsernameProperty::getHeader(void) const
{
	return _("User");
}

wxString
VersionUsernameProperty::getText(AnListClass *obj) const
{
	ApnVersion *version = dynamic_cast<ApnVersion *>(obj);

	if (version != 0)
		return (version->getUsername());
	else
		return _("???");
}

wxString
VersionNoProperty::getHeader(void) const
{
	return _("Version");
}

wxString
VersionNoProperty::getText(AnListClass *obj) const
{
	ApnVersion *version = dynamic_cast<ApnVersion *>(obj);

	if (version != 0)
		return (wxString::Format(wxT("%d"), version->getVersionNo()));
	else
		return _("???");
}

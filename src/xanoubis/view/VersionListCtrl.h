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

#ifndef _VERSIONLISTCTRL_H_
#define _VERSIONLISTCTRL_H_

#include "AnListCtrl.h"
#include "AnListProperty.h"

/**
 * A list-control displays versions fetched from VersionCtrl.
 *
 * Used to display ApnVersion instances. The update() method can be used
 * to reassign ApnVersion instances to the list. The view does not change the
 * profile (VersionCtrl::fetchVersionList() is not invoked)! If you want to
 * change the profile, you need to call VersionCtrl::fetchVersionList() and
 * then update() again.
 */
class VersionListCtrl : public AnListCtrl {
	public:
		VersionListCtrl(wxWindow *, wxWindowID, const wxPoint&,
		    const wxSize&, long);

		/**
		 * Updates the view with version-information fetched from
		 * VersionCtrl.
		 *
		 * You need to call this method, if another version-list was
		 * fetched by VersionCtrl::fetchVersionList().
		 */
		void update(void);
};

/**
 * Basic ApnVersion property.
 *
 * getIcon() is already implemented because ApnVersion does not use icons.
 * The class is still abstract!
 *
 * @see VersionTypeProperty
 * @see VersionDateProperty
 * @see VersionTimeProperty
 * @see VersionNoProperty
 */
class ApnVersionProperty : public AnListProperty
{
	public:
		AnIconList::IconId getIcon(AnListClass *) const;
};

/**
 * Property for ApnVersion::isAutoStore().
 */
class VersionTypeProperty : public ApnVersionProperty
{
	public:
		wxString getHeader(void) const;
		wxString getText(AnListClass *c) const;
};

/**
 * Property for the date of ApnVersion::getTimestamp()
 */
class VersionDateProperty : public ApnVersionProperty
{
	public:
		wxString getHeader(void) const;
		wxString getText(AnListClass *c) const;
};

/**
 * Property for the time of ApnVersion::getTimestamp()
 */
class VersionTimeProperty : public ApnVersionProperty
{
	public:
		wxString getHeader(void) const;
		wxString getText(AnListClass *c) const;
};

/**
 * Property of ApnVersion::getVersionNo().
 */
class VersionNoProperty : public ApnVersionProperty
{
	public:
		wxString getHeader(void) const;
		wxString getText(AnListClass *c) const;
};

#endif	/* _VERSIONLISTCTRL_H_ */

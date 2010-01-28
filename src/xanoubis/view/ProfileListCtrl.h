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

#ifndef _PROFILELISTCTRL_H_
#define _PROFILELISTCTRL_H_

#include "AnListCtrl.h"
#include "AnListProperty.h"

/**
 * A list-control displays profiles shipped by Anoubis or created by user.
 */

class ProfileListCtrl : public AnListCtrl
{
	public:
		ProfileListCtrl(wxWindow *, wxWindowID, const wxPoint&,
		    const wxSize&, long);
};

/**
 * Basic Profile property.
 *
 * getIcon() is already implemented because Profile does not use icons.
 * The class is still abstract!
 *
 * @see ProfileNameProperty
 * @see ProfileTypeProperty
 */
class ProfileProperty : public AnListProperty
{
	public:
		AnIconList::IconId getIcon(AnListClass *) const;
};

/**
 * Property for the name of the Profile, also used to find the
 * profile in the directory.
 */
class ProfileNameProperty : public ProfileProperty
{
	public:
		wxString getHeader(void) const;
		wxString getText(AnListClass *c) const;
};

/**
 * Property for the type of the Profile. Depending on the type
 * the Permission of the Profile is choosed.
 */
class ProfilePermissionProperty : public ProfileProperty
{
	public:
		wxString getHeader(void) const;
		wxString getText(AnListClass *c) const;
};

#endif	/* _PROFILELISTCTRL_H_ */

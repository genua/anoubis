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

#ifndef _ANLISTPROPERTY_H_
#define _ANLISTPROPERTY_H_

#include <wx/string.h>

#include "AnIconList.h"

class AnListClass;

/**
 * A property of an AnListClass.
 *
 * An AnListClass consists of several properties. A property must fulfill the
 * following interface to let someone other ask for data of the property.
 */
class AnListProperty
{
	public:
		/**
		 * D'tor.
		 */
		virtual ~AnListProperty(void) {}

		/**
		 * Returns an header of the property.
		 *
		 * This information is used by AnListCtrl to get a text for a
		 * column.
		 *
		 * @return Header-text of the property
		 */
		virtual wxString getHeader(void) const = 0;

		/**
		 * Returns a text of the property.
		 *
		 * Reads out the property from the specified AnListClass and
		 * provides a feasible text for it.
		 *
		 * @param c Source AnListClass-instance- The property reads the
		 *          instance.
		 * @return Property-value of the specified AnListClass-instance
		 */
		virtual wxString getText(AnListClass *) const = 0;

		/**
		 * Returns an icon of the property.
		 *
		 * Reads out the property from the specified AnListClass and
		 * provides a feasible icon for it. If this property does not
		 * need an icon, AnIconList::ICON_NONE should be returned.
		 *
		 * The icon-data are not returned directly. Instead you need
		 * to provide an index to the icon. The raw-data of the icons
		 * are managed by AnIconList.
		 *
		 * @param c Source AnListClass-instance- The property reads the
		 *          instance.
		 * @return Property-icon of the specified AnListClass-instance.
		 */
		virtual AnIconList::IconId getIcon(AnListClass *) const = 0;
};

#endif	/* _ANLISTPROPERTY_H_ */

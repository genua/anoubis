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
#include <wx/datetime.h>

#include "AnIconList.h"
#include "DefaultConversions.h"

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

/**
 * This template can be used to construct list properties without the
 * need to implement them manually. This template class can be used if
 * the following conditions are met:
 * - All rows have a common base class that allows us to extract the
 *   property.
 * - There is a function in the base class that returns the "natural"
 *   representation of the property. This "natural" representation is
 *   converted to a wxString (or AnIconList::IconId) with a conversion
 *   function that must be provided by the user.
 *
 * @param ROWTYPE The common base type of all rows.
 * @param TEXTTYPE The natural type of the property for the string value.
 *     This defaults to wxString.
 * @param ICONTYPE The naturlal type of the property for the icon value.
 *     This default to AnIcontList::IconId.
 */
template <typename ROWTYPE, typename TEXTTYPE = wxString,
    typename ICONTYPE = AnIconList::IconId>
class AnFmtListProperty : public AnListProperty {
private:
	/**
	 * The column header for this property. This is the value
	 * returned by the getHeader function.
	 */
	wxString		header_;

	/**
	 * A member function pointer that points to a member function
	 * of the row type ROWTYPE. This member function should return natural
	 * representation of the value that is returned by the getText
	 * function of this property.
	 */
	TEXTTYPE		(ROWTYPE::*textptr_)(void) const;

	/**
	 * A member function pointer that  points to a member function
	 * of the row type ROWTYPE. This member function should return the
	 * natural representation of the value that is returned by the
	 * getIcon function of this property.
	 */
	ICONTYPE		(ROWTYPE::*iconptr_)(void) const;

	/**
	 * A function pointer that can convert a TEXTTYPE value to
	 * a wxString. This is used to convert the natural representation
	 * of the getText value to a wxString. If this value is NULL the
	 * appropriate toString function from the DefaultConversions class
	 * is used.
	 */
	wxString		(*textconv_)(TEXTTYPE);

	/**
	 * A pointer to a function that can convert an ICONTYPE value
	 * into an AnIconList::IconId. This is used to convert the natural
	 * representation of the getIcon value to an AnIconList::IconId. If
	 * this value is NULL the appropriate toString function from the
	 * DefaultConversions class is used.
	 */
	AnIconList::IconId	(*iconconv_)(ICONTYPE);

public:

	/**
	 * Constructor.
	 *
	 * @param hdr Used to initialize the header_ member.
	 * @param tptr Used to initialize the textptr_ member.
	 * @param iptr Used to initialize the iconptr_ member.
	 *     If this argument is omitted it defaults to NULL.
	 * @param tconv Used to initialize the textconv_ member.
	 * @param iconconv Used to initialize the iconconv_ member.
	 */
	AnFmtListProperty(
	    wxString hdr,
	    TEXTTYPE (ROWTYPE::*tptr)(void) const,
	    ICONTYPE (ROWTYPE::*iptr)(void) const = NULL,
	    wxString (*tconv)(TEXTTYPE) = NULL,
	    AnIconList::IconId (*iconconv)(ICONTYPE) = NULL
	) : header_(hdr), textptr_(tptr), iconptr_(iptr),
	    textconv_(tconv), iconconv_(iconconv) {
	}

	/**
	 * This implements AnListProperty::getHeader. It returns the
	 * value stored in the header_ member.
	 *
	 * @param None.
	 * @return The column header.
	 */
	wxString getHeader(void) const {
		return header_;
	}

	/**
	 * This implements AnListProperty::getText. It extracts the natural
	 * value from the row object using the method pointed to by
	 * textptr_. Either the textconv_ function or a default conversion
	 * is used to convert this to a wxString.
	 *
	 * @param obj The row object.
	 * @return The column text for this row. If the textptr_ or the
	 *     textconv_ member is NULL or obj is not of type ROWTYPE the text
	 *     "???" is returned.
	 */
	wxString getText(AnListClass *obj) const {
		ROWTYPE		*tobj = dynamic_cast<ROWTYPE *>(obj);

		if (tobj != NULL && textptr_ != NULL) {
			TEXTTYPE	val = (tobj->*textptr_)();

			if (textconv_) {
				return (*textconv_)(val);
			} else {
				return DefaultConversions::toString(val);
			}
		} else {
			return _("???");
		}
	}

	/**
	 * This implements AnListProperty::getIcon. It extracts the natural
	 * value from the row object using the method pointed to by
	 * iconptr_. Either the iconconv_ function or a default conversion
	 * is used to convert this to an AnIconList::IconId.
	 *
	 * @param The row object.
	 * @return The iconptr_ member is NULL or obj is not of type ROWTYPE
	 *     ICON_NONE is returned.
	 */
	AnIconList::IconId getIcon(AnListClass *obj) const {
		ROWTYPE		*tobj = dynamic_cast<ROWTYPE *>(obj);

		if (tobj != NULL && iconptr_ != NULL) {
			ICONTYPE	val = (tobj->*iconptr_)();

			if (iconconv_) {
				return (*iconconv_)(val);
			} else {
				return DefaultConversions::toIcon(val);
			}
		} else {
			return AnIconList::ICON_NONE;
		}
	}
};

#endif	/* _ANLISTPROPERTY_H_ */

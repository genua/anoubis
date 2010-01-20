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

#ifndef _MODSFSLISTPROPERTY_H_
#define _MODSFSLISTPROPERTY_H_

#include <AnListProperty.h>

class SfsEntry;

/**
 * Properties of SfsEntry used by ModSfsListCtrl.
 *
 * Depending on the assigned ModSfsListProperty::Type, the propery-class
 * acts as a different property.
 */
class ModSfsListProperty : public AnListProperty
{
	public:
		/**
		 * Property-type.
		 */
		enum Type
		{
			PATH,		/*!< Path-property. */
			CHECKSUM,	/*!< Checksum-property. Evaluates
					     checksum-state of the SfsEntry. */
			SIGNATURE	/*!< Signature-property. Evaluates the
					     signature-state if the
					     SfsEntry. */
		};

		/**
		 * Creates a property of the given type.
		 *
		 * @param type Property-type to be assigned
		 */
		ModSfsListProperty(Type);

		/**
		 * Implementation of AnListProperty::getHeader().
		 *
		 * Depending of the type, a different header is returned.
		 *
		 * @return Header of property.
		 */
		wxString getHeader(void) const;

		/**
		 * Implementation of AnListProperty::getText().
		 *
		 * Depending of the type, another text to be displayed is
		 * returned.
		 *
		 * @param obj The SfsEntry to be displayed
		 * @return Text of property
		 */
		wxString getText(AnListClass *) const;

		/**
		 * Implementation of AnListProperty::getIcon().
		 *
		 * Depending of the type, another icon to be displayed is
		 * returned.
		 *
		 * @param obj The SfsEntry to the displayed
		 * @return Icon of property
		 */
		AnIconList::IconId getIcon(AnListClass *) const;

	private:
		/**
		 * Type of property.
		 */
		Type type_;

		/**
		 * Returns the text for the PATH-property.
		 *
		 * @param entry The SfsEntry to be read
		 * @return Text of PATH-property.
		 */
		wxString getTextPath(SfsEntry *) const;

		/**
		 * Returns the text of the CHECKSUM-property.
		 *
		 * @param entry The SfsEntry to be read
		 * @return Text of CHECKSUM-property.
		 */
		wxString getTextChecksum(SfsEntry *) const;

		/**
		 * Returns the text of the SIGNATURE-property.
		 *
		 * @param entry The SfsEntry to be read
		 * @return Text of SIGNATURE-property.
		 */
		wxString getTextSignature(SfsEntry *) const;

		/**
		 * Returns the icon for the PATH-property.
		 *
		 * @param entry The SfsEntry to be read
		 * @return Icon of PATH-property.
		 */
		AnIconList::IconId getIconPath(SfsEntry *) const;

		/**
		 * Returns the icon for the CHECKUM-property.
		 *
		 * @param entry The SfsEntry to be read
		 * @return Icon of CHECKUM-property.
		 */
		AnIconList::IconId getIconChecksum(SfsEntry *) const;

		/**
		 * Returns the icon for the SIGNATURE-property.
		 *
		 * @param entry The SfsEntry to be read
		 * @return Icon of SIGNATURE-property.
		 */
		AnIconList::IconId getIconSignature(SfsEntry *) const;
};

#endif	/* _MODSFSLISTPROPERTY_H_ */

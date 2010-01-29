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

#ifndef _RULEWIZARDSERVICEPROPERTY_H_
#define _RULEWIZARDSERVICEPROPERTY_H_

#include <AnListProperty.h>

/**
 * Properties of the ServiceList.
 *
 * The assigned RuleWizardServiceProperty::Type specifies the type the
 * property. Instances of the Service-class are passed to the property.
 */
class RuleWizardServiceProperty : public AnListProperty
{
	public:
		/**
		 * Property-type
		 */
		enum Type {
			NAME = 0,	/**< The name of the service */
			PORT,		/**< The port-number of the service */
			PROTOCOL,	/**< The underlaying protocol */
			DEFAULT		/**< Service from the default-list? */
		};

		/**
		 * Creates a service-property.
		 * @param type Type to be assigned to the property
		 */
		RuleWizardServiceProperty(Type);

		/**
		 * Implementation of AnListProperty::getHeader().
		 *
		 * Returns the header of the property (depending on the assigned
		 * Type).
		 *
		 * @return Header of property
		 */
		wxString getHeader(void) const;

		/**
		 * Implementation of AnListProperty::getText().
		 *
		 * Returns the text to be displayed (depending on the assigned
		 * Type).
		 *
		 * @param obj Service to be displayed
		 * @return Text of property
		 */
		wxString getText(AnListClass *) const;

		/**
		 * Implementation of AnListProperty::getIcon().
		 *
		 * Returns the icon to be displayed (depending on the assigned
		 * Type).
		 *
		 * @param obj Service to be displayed
		 * @return Icon of property
		 */
		AnIconList::IconId getIcon(AnListClass *) const;

	private:
		/**
		 * The property-type.
		 */
		Type type_;
};

#endif	/* _RULEWIZARDSERVICEPROPERTY_H_ */

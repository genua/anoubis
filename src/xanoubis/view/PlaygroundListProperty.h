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

#ifndef _PLAYGROUNDPROPERTY_H_
#define _PLAYGROUNDPROPERTY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "AnListClass.h"
#include "AnListProperty.h"
#include "AnIconList.h"

/**
 * Attribute provider for the Playground List grids, listing playgrounds and
 * their files / contents. Implementation defined by AnListProperty.
 */
class PlaygroundListProperty : public AnListProperty
{
	public:
		/**
		 * Enumeration of playground list properties.
		 */
		enum PropertyType {
			/**
			 * playground id.
			 */
			PROPERTY_ID,

			/**
			 * user id.
			 */
			PROPERTY_USER,

			/**
			 * status.
			 */
			PROPERTY_STAT,

			/**
			 * number of files.
			 */
			PROPERTY_FILES,

			/**
			 * start time of the playground.
			 */
			PROPERTY_TIME,

			/**
			 * command.
			 */
			PROPERTY_COMMAND
		};

		/**
		 * Constructor.
		 * @param[in] 1st Type of property.
		 */
		PlaygroundListProperty(PropertyType);

		/**
		 * Implementation of AnListProperty::getHeader().
		 * Depending of the type, a different header is returned.
		 * @return Header of property.
		 */
		wxString getHeader(void) const;

		/**
		 * Implementation of AnListProperty::getText().
		 * Depending of the type, another text to be displayed is
		 * returned.
		 * @param[in] 1st PlaygroundInfoEntry object
		 * @return Text of property.
		 */
		wxString getText(AnListClass *) const;

		/**
		 * Implementation of AnListProperty::getIcon().
		 * ATTENTION - This method exists only to satisfy the interface.
		 * It returns ICON_NONE in any case.
		 * @param[in] 1st PlaygroundInfoEntry object
		 * @return Text of property.
		 */
		AnIconList::IconId getIcon(AnListClass *) const;

	private:
		/**
		 * Store type of this property instance.
		 */
		PropertyType type_;
};

#endif	/* _PLAYGROUNDPROPERTY_H_ */

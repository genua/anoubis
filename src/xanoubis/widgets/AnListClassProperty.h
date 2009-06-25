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

#ifndef _ANLISTCLASSPROPERTY_H_
#define _ANLISTCLASSPROPERTY_H_

#include <wx/colour.h>
#include <wx/font.h>

class AnListClass;

/**
 * The property of a complete AnListClass.
 *
 * As distinguished from AnListProperty, this property is valid for a complete
 * AnListClass.
 *
 * Assigning a AnListClassProperty to a AnListCtrl specifies general formatting
 * options depending on an AnListClass.
 */
class AnListClassProperty
{
	public:
		/**
		 * D'tor.
		 */
		virtual ~AnListClassProperty(void);

		/**
		 * Returns the foreground color (font color).
		 *
		 * You can overwrite the method in order to adjust the
		 * foreground color depending on the given AnListClass
		 * instance. The default implementation returns the system
		 * default color.
		 *
		 * @param c Source AnListClass-instance. The property reads the
		 *          instance.
		 * @return Foreground color used to format the given
		 *         AnListClass instance.
		 */
		virtual wxColour getColor(AnListClass *) const;

		/**
		 * Returns the background color.
		 *
		 * You can overwrite the method in order to adjust the
		 * background color depending on the given AnListClass
		 * instance. The default implementation returns the system
		 * default background color.
		 *
		 * @param c Source AnListClass-instance. The property reads the
		 *          instance.
		 * @return Background color used to format the given
		 *         AnListClass instance.
		 */
		virtual wxColour getBackgroundColor(AnListClass *) const;

		/**
		 * Returns the font.
		 *
		 * You can overwrite the method in order to adjust the font
		 * depending on the given AnListClass instance. The default
		 * implementation returns the system default font.
		 *
		 * @param c Source AnListClass-instance. The property reads the
		 *          instance.
		 * @return Font used to format the given AnListClass instance.
		 */
		virtual wxFont getFont(AnListClass *) const;
};

#endif	/* _ANLISTCLASSPROPERTY_H_ */

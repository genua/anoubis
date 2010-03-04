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

#ifndef _ANGRIDCOLUMN_H_
#define _ANGRIDCOLUMN_H_

#include <wx/grid.h>

#include "AnListProperty.h"

/**
 * A column of an AnTable.
 * This is a single column internaly handled by AnTable. The property
 * defines the data to be displayed in the column.
 */
class AnGridColumn
{
	public:
		/**
		 * Tests whether the column is visible.
		 * @param None.
		 * @return true if visible, false otherwise.
		 */
		bool isVisible(void) const;

		/**
		 * Shows/hides the column.
		 * @param[in] 1st If set to true, the column is shown,
		 *	otherwise it is hidden.
		 * @return Nothing.
		 */
		void setVisibility(bool);

		/**
		 * Returns the assigned property of the column.
		 * The property defines the data to be displayed here.
		 * @param None.
		 * @return The property of the column.
		 */
		AnListProperty *getProperty(void) const;

		/**
		 * Get column width.
		 * Return the width of this column.
		 * @param None.
		 * @return The width of this column.
		 */
		int getWidth(void) const;

		/**
		 * Set column width.
		 * Set the new width of this column.
		 * @param[in] 1st The new width.
		 * @return Nothing.
		 */
		void setWidth(int);

	private:
		/**
		 * Constructor
		 * This constructor is private, because only AnTable
		 * is allowed to create instances of this class.
		 * The visibility is stored within the config at config path.
		 * @param[in] 1st The assigned property.
		 * @param[in] 2nd The configuration path.
		 * @param[in] 3rd The default width.
		 */
		AnGridColumn(AnListProperty *, const wxString &, int);

		/**
		 * Destructor
		 * This constructor is private, because only AnTable
		 * is allowed to detroy instances of this class.
		 */
		~AnGridColumn(void);

		/**
		 * Store visibility.
		 */
		bool visibility_;

		/**
		 * Store width of this column.
		 */
		int width_;

		/**
		 * Store property.
		 */
		AnListProperty *property_;

		/**
		 * Store configuration path.
		 */
		wxString configPath_;

	friend class AnTable;
};

#endif	/* _ANGRIDCOLUMN_H_ */

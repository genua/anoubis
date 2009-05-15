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

#ifndef _ANLISTCOLUMN_H_
#define _ANLISTCOLUMN_H_

#include <wx/listctrl.h>

class AnListCtrl;
class AnListProperty;

/**
 * A column of an AnListCtrl.
 *
 * This is a single column and needs to be inserted into the list. The property
 * (getProperty()) defines the data to be displayed in the column. There are
 * several options, which defines the look of the column.
 *
 * The column takes over the owership of the assigned property. It means, when
 * the column is removed, the assigned property is also destroyed.
 */
class AnListColumn
{
	public:
		/**
		 * Returns the index of the column.
		 *
		 * This is not necessarily the index of a visible column. This
		 * is the index you specified while fetching the column-object
		 * (AnListCtrl::getColumn()).
		 *
		 * @return Index of column
		 */
		unsigned int getIndex(void) const;

		/**
		 * Returns the assigned property of the column.
		 *
		 * The property defines the data to be displayed here.
		 *
		 * @return The property of the column
		 */
		AnListProperty *getProperty(void) const;

		/**
		 * Tests whether the column is visible.
		 *
		 * @return true if visible, false otherwise.
		 */
		bool isVisible(void) const;

		/**
		 * Shows/hides the column.
		 *
		 * @param visible If set to true, the column is shown,
		 *                otherwise it is hidden.
		 */
		void setVisible(bool);

		/**
		 * Returns the width of the column.
		 *
		 * @return Width of the column
		 */
		int getWidth(void) const;

		/**
		 * Updates the width of the column.
		 *
		 * @param width Can be a width in pixels or wxLIST_AUTOSIZE
		 *              (-1) or wxLIST_AUTOSIZE_USEHEADER (-2).
		 *              wxLIST_AUTOSIZE will resize the column to the
		 *              length of its longest item.
		 */
		void setWidth(int);

		/**
		 * Returns the alignment of the column.
		 *
		 * Can be one of wxLIST_FORMAT_LEFT, wxLIST_FORMAT_RIGHT or
		 * wxLIST_FORMAT_CENTRE.
		 *
		 * @return The alignment of the column.
		 */
		wxListColumnFormat getAlign(void) const;

		/**
		 * Updates the alignment of the column.
		 *
		 * @param align The new alignment. Can be one of
		 *              wxLIST_FORMAT_LEFT, wxLIST_FORMAT_RIGHT or
		 *              wxLIST_FORMAT_CENTRE.
		 */
		void setAlign(wxListColumnFormat);

	private:
		/**
		 * C'tor made private.
		 *
		 * Only AnListCtrl is allowed to create instances of the class.
		 *
		 * @param property The property to be assigned
		 * @param parent The parent (owner) list
		 */
		AnListColumn(AnListProperty *, AnListCtrl *);

		/**
		 * D'tor.
		 *
		 * Made private because only AnListCtrl is allowed to destroy
		 * instances of the class.
		 */
		~AnListColumn(void);

		/**
		 * Updates the index of the column.
		 *
		 * @param idx The new index
		 */
		void setIndex(unsigned int);

		/**
		 * Updates the corresponding column in wxListCtrl.
		 *
		 * This method should be called after modifying the column.
		 */
		void updateColumn(void);

		/**
		 * The parent list.
		 */
		AnListCtrl *parent_;

		/**
		 * The assigned property.
		 */
		AnListProperty *property_;

		/**
		 * Several information about the column.
		 */
		wxListItem columnInfo_;

		/**
		 * Index of column.
		 */
		unsigned int columnIdx_;

	friend class AnListCtrl;
};

#endif	/* _ANLISTCOLUMN_H_ */

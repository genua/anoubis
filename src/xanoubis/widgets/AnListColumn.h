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
#include <wx/grid.h>

#include "AnListProperty.h"

/**
 * A column of an AnListCtrl or an AnGrid/AnTable.
 *
 * This is a single column and needs to be inserted into the list or grid.
 * The property (getProperty()) defines the data to be displayed in the
 * column. There are several options, which define the look of the column.
 *
 * The column takes over the owership of the assigned property. This means,
 * when the column is removed, the assigned property is also destroyed.
 */
class AnListColumn
{
	public:
		/**
		 * Tests whether the column should be visible.
		 *
		 * @return true if visible, false otherwise.
		 */
		bool isVisible(void) const {
			return visible_;
		}

		/**
		 * Inform the column that is now is visible/hidden.
		 * Call this to make sure that a modified visibility
		 * state is written to the xanouis.conf file. This is
		 * not actually change the visibility status of the column
		 * in the view. This must be done by the caller.
		 *
		 * @param visible True if the column is visible, false if
		 *     it is hidden.
		 */
		void setVisible(bool);

		/**
		 * Return the current width of the column as stored in
		 * the column object.
		 *
		 * @param None.
		 * @return The current width.
		 */
		int getWidth(void) const {
			return width_;
		}

		/**
		 * Tell the column that its column width was changed. The
		 * column will make sure that the updated with makes it
		 * to the config file.
		 *
		 * @param width The new width of the column.
		 * @return None.
		 */
		void setWidth(int width);

		/**
		 * Returns the assigned property of the column.
		 *
		 * The property defines the data to be displayed here.
		 *
		 * @return The property of the column
		 */
		AnListProperty *getProperty(void) const;

		/**
		 * Return information about the column in the form of
		 * a wxListItem that can be used in a wxListCtrl.
		 *
		 * @param None.
		 * @return The wxListItem describing the column.
		 */
		wxListItem &getColumnInfo(void);

	private:
		/**
		 * Private constructor. Only AnListCtrl and AnTable are
		 * allowed to create column objects.
		 *
		 * Only AnListCtrl is allowed to create instances of the class.
		 *
		 * @param property The property to be assigned
		 * @param configKey The Key that should be used to store
		 *     the column width an visibility in the config file.
		 *     Use wxEmptyString if you do not want to store the
		 *     this data permanently.
		 * @param width The default width of the column. A configured
		 *     value for the width stored in the config file
		 *     takes precedence.
		 * @param align The column alignmnet. Defaults to LEFT.
		 *     NOTE: Currenty, this parameter is not used for Grids.
		 */
		AnListColumn(AnListProperty *, wxString configKey,
		    int width, wxListColumnFormat align = wxLIST_FORMAT_LEFT);

		/**
		 * Private Destructor. Only AnListCtrl and AnTable are
		 * allowed to destroy column objects.
		 */
		~AnListColumn(void);

		/**
		 * The assigned property.
		 */
		AnListProperty			*property_;

		/**
		 * The width of the column.
		 */
		int				 width_;

		/**
		 * The alignment of the column.
		 */

		wxListColumnFormat		 align_;
		/**
		 * The visibility status of the column.
		 */
		bool				 visible_;

		/**
		 * The configuration key that is used to store the column
		 * with and visibility.
		 */
		wxString configPath_;

		/**
		 * The column information for the column. This is filled
		 * on demand and only used for AnListCtrls.
		 */
		wxListItem	colInfo_;

	friend class AnListCtrl;
	friend class AnTable;
};

#endif	/* _ANLISTCOLUMN_H_ */

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

#ifndef _LISTCTRLCOLUMN_H_
#define _LISTCTRLCOLUMN_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/string.h>

/**
 * This class represents the meta data of a column of a wxListCtrl.
 * It's only purpose is to group all related data of a column and
 * make the managemenmt easier.
 */
class ListCtrlColumn
{
	public:
		/**
		 * Default constructor of ListCtrlColumn.
		 * @param None.
		 */
		ListCtrlColumn(void);

		/**
		 * Title constructor of ListCtrlColumn.
		 * This will set the width to wxLIST_AUTOSIZE_USEHEADER.
		 * @param[in] 1st The column title.
		 */
		ListCtrlColumn(const wxString &);

		/**
		 * Title constructor of ListCtrlColumn.
		 * This will set the width to wxLIST_AUTOSIZE_USEHEADER.
		 * @param[in] 1st The column title.
		 * @param[in] 2nd The key name of the column used in config file
		 */
		ListCtrlColumn(const wxString &, const wxString &);

		/**
		 * Title constructor of ListCtrlColumn with size setting.
		 * This will explicitly set the width.
		 * @param[in] 1st The column title.
		 * @param[in] 2nd The key name of column used in config file.
		 * @param[in] 3rd The column width in pixels.
		 */
		ListCtrlColumn(const wxString &, const wxString &, int);

		/**
		 * Set the index of listCtrl this column is part of.
		 * Use the result of InsertColumn() for this.
		 * @param[in] 1st The new index.
		 * @return Nothing.
		 */
		void setIndex(long);

		/**
		 * Get the index of this column within listCtrl.
		 * @param None.
		 * @return The index (>=0) or -1 if not visible.
		 */
		long getIndex(void) const;

		/**
		 * Set the title of this column.
		 * @param[in] 1st The title.
		 * @return Nothing.
		 */
		void setTitle(const wxString &);

		/**
		 * Get the title of this column.
		 * @param None.
		 * @return The title.
		 */
		wxString getTitle(void) const;

		/**
		 * Set the column width.
		 * @param[in] 1st The new width.
		 * @return Nothing.
		 */
		void setWidth(int);

		/**
		 * Get the column width.
		 * @param None.
		 * @return The width of this column.
		 */
		int getWidth(void) const;

		/**
		 * Set the visability of this column.
		 * @param[in] 1st Visability flag: true for visible.
		 * @return Nothing.
		 */
		void setVisability(bool);

		/**
		 * Is this column visible?
		 * @param None.
		 * @return True if visisble.
		 */
		bool isVisible(void) const;

		/**
		 * Get the key for the column header visibility
		 * @param None.
		 * @return The key
		 */
		wxString getConfKey(void) const;

	private:
		long index_;		/**< Index at listCtrl. */
		wxString title_;	/**< Column title. */
		wxString confkey_;	/**< Key name in config file. */
		int width_;		/**< Column width. */
		bool visability_;	/**< Visability flag. */
};

#endif	/* _LISTCTRLCOLUMN_H_ */

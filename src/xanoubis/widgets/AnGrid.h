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

#ifndef _ANGRID_H_
#define _ANGRID_H_

#include <wx/grid.h>

/**
 * This realizes the Anoubis Grid-Widget.
 *
 * The AnGrid is an ordinary wxGrid, extended ty an event handler and
 * the ability to show and hide the columns on users choice. It is intended
 * to work as a more flexible replacement for an AnListCtrl. Please consider
 * using a ListCtrl instead of a Grid.
 *
 * An AnGrid does assumes that the selection mode is wxGridSelectRows
 * and enforces that only a single row is selected at any given time.
 */
class AnGrid : public wxGrid
{
	public:
		/**
		 * Constructor of AnGrid.
		 * It has to have the same signature as a ordinary wxGrid,
		 * so wxformbuilder can just exchange wxGrid with AnGrid.
		 */
		AnGrid(wxWindow *parent, wxWindowID id,
		    const wxPoint& pos = wxDefaultPosition,
		    const wxSize& size = wxDefaultSize,
		    long style = wxWANTS_CHARS,
		    const wxString& name = wxGridNameStr);

		/**
		 * Destructor.
		 */
		~AnGrid(void);

		/**
		 * Set cursor visibility.
		 * With this method the cursor showing the current cell
		 * can be switched on and off.
		 * @param[in] 1st Visibility of cursor: true=on false=off
		 */
		void setCursorVisibility(bool);

		/**
		 * Is cursor visible?
		 * @param None.
		 * @return True if visible.
		 */
		bool isCursorVisible(void) const;

	private:
		/**
		 * Store cursor visibility.
		 */
		bool isCursorVisible_;

		/**
		 * Event handler for right mouse button clicks.
		 * It will show a choice dialog and updates the columns.
		 * @param[in] 1st The grid event.
		 * @return Nothing.
		 */
		void onLabelRightClick(wxGridEvent &);

		/**
		 * Event handler for column size changes.
		 * It will calculate the new width of a colunn and write
		 * this width to column data model.
		 * @param[in] 1st The grid size event.
		 * @return Nothing.
		 */
		void onColumnSize(wxGridSizeEvent &);

		/**
		 * Overwrite method of wxGrid to intercept drawing of cursor.
		 */
		void DrawCellHighlight(wxDC &, const wxGridCellAttr *);

		/**
		 * Event handler for range select events. We use this
		 * to fixup the selection if the user manages to select
		 * multiple rows.
		 */
		void onGridCmdRangeSelect(wxGridRangeSelectEvent &);
};

#endif	/* _ANGRID_H_ */

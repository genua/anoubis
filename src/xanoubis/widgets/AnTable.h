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

#ifndef _ANTABLE_H_
#define _ANTABLE_H_

#include <wx/dynarray.h>
#include <wx/event.h>
#include <wx/grid.h>

#include <vector>

#include "AnGridColumn.h"
#include "AnListProperty.h"
#include "AnRowProvider.h"

/**
 * Common table for AnGrids.
 *
 * This is the common table to provide data for a AnGrid.
 * The columns are feed by adding properties and a
 * row provider is asked for the data.
 */
class AnTable : public wxGridTableBase
{
	public:
		/**
		 * Constructor.
		 * @param[in] 1st The configuration path.
		 */
		AnTable(const wxString &);

		/**
		 * Destructor.
		 */
		~AnTable(void);

		/**
		 * Get column names.
		 * This method returns a list of all column headers.
		 * @param None.
		 * @return List of headers.
		 */
		wxArrayString getColumnName(void) const;

		/**
		 * Add a property.
		 * This method adds a given property to the end of the
		 * list of properties. The order of added properties
		 * represents the order in the grid.
		 * @param[in] 1st The new property.
		 * @return Nothing.
		 */
		void addProperty(AnListProperty *);

		/**
		 * Get column count.
		 * This method returns the number of known columns.
		 * @param None.
		 * @return The number of columns.
		 */
		size_t getColumnCount(void) const;

		/**
		 * Get visibility of a column.
		 * This method returns the visibility of a column. The
		 * column is addressed by it's index in the list.
		 * @param[in] 1st The index of column in question.
		 * @return True if visible.
		 */
		bool isColumnVisible(unsigned int) const;

		/**
		 * Set visibility of a column.
		 * This method sets the visibility of a column. The
		 * column is addressed by it's index in the list.
		 * @param[in] 1st The index of column in question.
		 * @param[in] 2nd The new visibility.
		 */
		void setColumnVisability(unsigned int, bool);

		/**
		 * Set row provider.
		 * This method clears the table, remember the new provider
		 * and inform the grid about new rows.
		 * @param[in] 1st The new row provider.
		 * @return Nothing.
		 */
		void setRowProvider(AnRowProvider *);

		/**
		 * Get row provider.
		 * This method returns the registered row provider.
		 * @param None.
		 * @return The row provider.
		 */
		AnRowProvider *getRowProvider(void) const;

		/**
		 * Returns the number of rows.
		 * This method is part of the wxGridTableBase-interface.
		 * @param None.
		 * @return Number of rows.
		 */
		int GetNumberRows(void);

		/**
		 * Returns the number of columns.
		 * This method is part of the wxGridTableBase-interface.
		 * @param None.
		 * @return Number of columns
		 */
		int GetNumberCols(void);

		/**
		 * Returns the headers of columns.
		 * This method is part of the wxGridTableBase-interface.
		 * @param[in] 1st The column number.
		 * @return Header of the given column.
		 */
		wxString GetColLabelValue(int);

		/**
		 * Tests whether the given cell is empty.
		 * This method is part of the wxGridTableBase-interface.
		 * @param[in] 1st The row of the cell.
		 * @param[in] 2nd The column of the cell.
		 * @return true if the given cell is empty.
		 */
		bool IsEmptyCell(int, int);

		/**
		 * Returns the value of the given cell.
		 * This method is part of the wxGridTableBase-interface.
		 * @param[in] 1st The row of the cell.
		 * @param[in] 2nd The column of the cell.
		 * @return Value of the given cell.
		 */
		wxString GetValue(int, int);

		/**
		 * Updates the value at the given cell.
		 * This method is part of the wxGridTableBase-interface.
		 * ATTENTION: This method is only needed to fullfill the
		 * wxGridTableBase-interface. This method does nothing!
		 * @param[in] 1st The row of the cell.
		 * @param[in] 2nd The column of the cell.
		 * @param[in] 3rd The new value.
		 * @return Nothing.
		 */
		void SetValue(int, int, const wxString &);

	private:
		/**
		 * List of all known properties.
		 */
		std::vector<AnGridColumn *> columnList_;

		/**
		 * List of visible colums indices to indices in property list.
		 */
		std::vector<int> visibilityList_;

		/**
		 * Row provider we ask for a specific row.
		 */
		AnRowProvider *rowProvider_;

		/**
		 * Store configuration path.
		 */
		wxString configPath_;

		/**
		 * Fires an row-insert-message.
		 * So the assigned view can react on such a situation.
		 * @param[in] 1st The index of first new row.
		 * @param[in] 2nd The number of new rows.
		 * @return Nothing.
		 */
		void fireRowsInserted(int, int);

		/**
		 * Fires an row-remove-message.
		 * So the assigned view can react on such a situation.
		 * @param[in] 1st The index of first row to remove.
		 * @param[in] 2nd The number of rows to remove.
		 * @return Nothing.
		 */
		void fireRowsRemoved(int, int);

		/**
		 * Fires an column-insert-message.
		 * So the assigned view can react on such a situation.
		 * @param[in] 1st The index of first new column.
		 * @param[in] 2nd The number of new columns.
		 * @return Nothing.
		 */
		void fireColsInserted(int, int);

		/**
		 * Fires an column-remove-message.
		 * So the assigned view can react on such a situation.
		 * @param[in] 1st The index of first column to remove.
		 * @param[in] 2nd The number of columns to remove.
		 * @return Nothing.
		 */
		void fireColsRemoved(int, int);
};

#endif	/* _ANTABLE_H_ */

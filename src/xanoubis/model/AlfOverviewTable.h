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

#ifndef _ALFOVERVIEWTABLE_H_
#define _ALFOVERVIEWTABLE_H_

#include "SimpleOverviewTable.h"

/**
 * Concrete <i>Alf Overview Table</i>.
 *
 * Takes data from SimpleOverviewTable and breaks them into several columns.
 */
class AlfOverviewTable : public SimpleOverviewTable
{
	public:
		/**
		 * C'tor.
		 */
		AlfOverviewTable(void);

		/**
		 * D'tor.
		 */
		~AlfOverviewTable(void);

		/**
		 * Returns the number of rows.
		 *
		 * This method is part of the wxGridTableBase-interface.
		 *
		 * @return Number of rows
		 */
		int GetNumberRows();

		/**
		 * Returns the number of columns.
		 *
		 * This method is part of the wxGridTableBase-interface.
		 *
		 * @return Number of columns
		 */
		int GetNumberCols();

		/**
		 * Returns the headers of columns.
		 *
		 * This method is part of the wxGridTableBase-interface.
		 *
		 * @param col The column number
		 * @return Header of the given column
		 */
		wxString GetColLabelValue(int);

		/**
		 * Tests whether the given cell is empty.
		 *
		 * This method is part of the wxGridTableBase-interface.
		 *
		 * @param row The row of the cell
		 * @param col The column of the cell
		 * @return true if the given cell is empty.
		 */
		bool IsEmptyCell(int, int);

		/**
		 * Returns the value of the given cell.
		 *
		 * This method is part of the wxGridTableBase-interface.
		 *
		 * @param row The row of the cell
		 * @param col The column of the cell
		 * @return Value of the given cell
		 */
		wxString GetValue(int row, int col);

		/**
		 * Updates the value at the given cell.
		 *
		 * This method is part of the wxGridTableBase-interface.
		 *
		 * @param row The row of the cell.
		 * @param col The column of the cell.
		 * @param value The new value
		 */
		void SetValue(int, int, const wxString &);

	private:
		/**
		 * Columns of the table.
		 */
		enum Column {
			COL_PROGRAM = 0,
			COL_SERVICE,
			COL_ROLE,
			COL_ACTION,
			COL_RESTRICTIONS,
			COL_USER,
			COL_EOF
		};

		/**
		 * Returns the value of the program-column.
		 *
		 * @param row The content of the requested row
		 * @return Value of program-column
		 */
		wxString getProgramText(SimpleOverviewRow *) const;

		/**
		 * Returns the value of a filter-column.
		 *
		 * @param col The requested column
		 * @param row The content of the requested row
		 * @return Value of filter-column
		 */
		wxString getFilterText(int, SimpleOverviewRow *) const;

		/**
		 * Returns the value of the user-column.
		 *
		 * @param row The content of the requested row
		 * @return Value of user-column
		 */
		wxString getUserText(SimpleOverviewRow *) const;

		/**
		 * Returns the column-value of the given AlfFilterPolicy.
		 *
		 * Depending on the column-number another string is displayed.
		 *
		 * @param col The requested column
		 * @param policy The source policy
		 * @return Value to be displayed at the given column
		 */
		wxString getAlfText(int, AlfFilterPolicy *) const;

		/**
		 * Returns the column-value of the given
		 * AlfCapabilityFilterPolicy.
		 *
		 * Depending on the column-number another string is displayed.
		 *
		 * @param col The requested column
		 * @param policy The source policy
		 * @return Value to be displayed at the given column
		 */
		wxString getAlfCapabilityText(int,
		    AlfCapabilityFilterPolicy *) const;

		/**
		 * Returns the column-value of the given DefaultFilterPolicy.
		 *
		 * Depending on the column-number another string is displayed.
		 *
		 * @param col The requested column
		 * @param policy The source policy
		 * @return Value to be displayed at the given column
		 */
		wxString getDefaultText(int, DefaultFilterPolicy *) const;
};

#endif	/* _ALFOVERVIEWTABLE_H_ */

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

#ifndef _ANGENERICROWPROVIDER_H_
#define _ANGENERICROWPROVIDER_H_

#include <vector>
#include <map>

#include <AnListClass.h>
#include <AnRowProvider.h>
#include <AnListProperty.h>

/**
 * This class implements a row provider that exposes an interface to
 * external callers that is similar to that used in traditional list handling.
 * Users can directly add and delte rows in the provider and the provider
 * will handle the communication with AnListCtrl.
 *
 * This interface may be useful for lists that do not change very often
 * and that are known to be of small size. Additionally, this provider
 * adds functions that allow the suer to sort the list based on different
 * criteria. Filtering is supported, too. However, the implementation is
 * not suited for very large lists.
 *
 * Objects in the list are monitored individually with the subject/observer
 * pattern.
 */
class AnGenericRowProvider : public AnRowProvider, private Observer
{
	public:
		/**
		 * Constructor.
		 */
		AnGenericRowProvider(void);

		/**
		 * Destructor.
		 * Will free the internal list but not the data pointed to by
		 * the list elements.
		 */
		~AnGenericRowProvider(void);

		/**
		 * Assigns rows to the list.
		 *
		 * Any previously assigned rows are removed before.
		 *
		 * @param rowData Data to be assigned to the list
		 * @see clearRows()
		 */
		void setRowData(const std::vector<AnListClass *> &);

		/**
		 * Appends a row at the end of the list.
		 *
		 * @param row Row to be appended
		 */
		void addRow(AnListClass *);

		/**
		 * Removes a single row from the list.
		 *
		 * @param idx Index of row to be removed. This is _not_ the
		 *     index in the visible list but in the unordered over all
		 *     list. Do not use this function if this is avoidable.
		 * @return On success true is returned. If the index is out
		 *     of range, nothing is removed  and false is returned.
		 * @note The corresponding AnListClass instance is <i>not</i>
		 *     destroyed!
		 */
		bool removeRawRow(unsigned int);

		/**
		 * Removes all rows from the list.
		 *
		 * @note The corresponding AnListClass instances are <i>not</i>
		 *     destroyed!
		 */
		void clearRows(void);

		/**
		 * Implementation of AnRowProvider::getRow()
		 * @param idx The row index in the list of _visible_ rows.
		 * @return The AnListClass object associated with the index.
		 */
		AnListClass *getRow(unsigned int idx) const;

		/**
		 * Implementation of AnRowProvider::getSize()
		 * @return The total number of rows currently visible.
		 */
		int getSize(void) const;

		/**
		 * Return the total number of rows, visible or not.
		 */
		int getRawSize(void) const {
			return allrows_.size();
		}

		/**
		 * Return the row with the given index from the
		 * list of all rows (visible or not). You are probably
		 * interested in getRow instead.
		 *
		 * @param idx The raw index of the row.
		 * @return The row object with the given index.
		 */
		AnListClass *getRawRow(unsigned int idx) const {
			if (idx >= allrows_.size())
				return NULL;
			return allrows_[idx];
		}

		/**
		 * Set the filter property. This is mandatory for the
		 * filter to work reliably. Setting the property resets
		 * the filter string, i.e. all rows are initially visible.
		 * The caller must make sure that the property lives as
		 * long as the provider.
		 *
		 * @param[in] 1st The filter property.
		 * @return None.
		 */
		void setFilterProperty(AnListProperty *);

		/**
		 * Set the filter String. This string is used to filter
		 * rows. Rows are only visible if the string extracted by
		 * filterProperty matches the current filter String.
		 * An empty filter String implies no filtering.
		 *
		 * @param[in] 1st The new filter string.
		 * @return None.
		 */
		void setFilterString(const wxString &);

		/**
		 * Set the property that is used to sort the rows.
		 * If no sort property is set the list is not sorted. The
		 * caller must make sure that the property lives as long
		 * as the provider.
		 *
		 * @param[in] 1st The new sort property.
		 * @return None.
		 */
		void setSortProperty(AnListProperty *);

		/**
		 * Implementation of the StrictWeakOrdering Model.
		 */
		bool operator() (unsigned int, unsigned int) const;

	private:
		/**
		 * Implementation of Observer::update().
		 *
		 * Refresh the view.
		 */
		void update(Subject *);

		/**
		 * Implementation of Observer::updateDelete().
		 *
		 * Remove the object from the list.
		 */
		void updateDelete(Subject *);

		/**
		 * Update the list of visible columns after the sorting
		 * changes or filtering changes.
		 *
		 * @params None.
		 * @return None.
		 */
		void updateVisible(void);
		/**
		 * List of all rows.
		 *
		 * Used to select a row of the list in constant time.
		 */
		std::vector<AnListClass *>	allrows_;

		/**
		 * List of visible rows. Visible rows can differ from
		 * all rows if a search string is given or if the or if
		 * the list is sorted.
		 */
		std::vector<unsigned int>	visiblerows_;

		/**
		 * This property (if any) is used to extract a text
		 * version of each row for filtering.
		 */
		AnListProperty			*filterProperty_;

		/**
		 * This filter is applied to all rows. A row is visible iff
		 * the string extracted by filterProperty_ from the row matches
		 * filterString_.
		 */
		wxString			 filterString_;

		/**
		 * It set this property is used to sort all visible rows
		 * according to the string extracted from each row by
		 * this property.
		 */
		AnListProperty			*sortProperty_;
};

#endif	/* _ANGENERICROWPROVIDER_H_ */

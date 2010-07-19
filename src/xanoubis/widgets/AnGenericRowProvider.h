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
#include <list>
#include <map>

#include <AnListClass.h>
#include <AnRowProvider.h>

/**
 * This class implements a row provider that exposes an interface to
 * external callers that is similar to that used in traditional list handling.
 * Users can directly add and delte rows in the provider and the provider
 * will handle the communication with AnListCtrl.
 *
 * This interface may be useful for lists that do not change very often
 * and that are known to be of small size.
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
		 * Assigns rows to the list.
		 *
		 * Any previously assigned rows are removed before.
		 *
		 * @param rowData Data to be assigned to the list
		 * @see clearRows()
		 */
		void setRowData(const std::list<AnListClass *> &);

		/**
		 * Appends a row at the end of the list.
		 *
		 * @param row Row to be appended
		 */
		void addRow(AnListClass *);

		/**
		 * Appends a row <i>before</i> the specified index.
		 *
		 * @param row Row to be appended
		 * @param before Index of row-list. The new index is inserted
		 *               this index. If the indexis out of range,
		 *               the row is appended at the end of the list.
		 */
		void addRow(AnListClass *, unsigned int);

		/**
		 * Removes a single row from the list.
		 *
		 * @param idx Index of row to be removed.
		 * @return On success true is returned. If the index is out
		 *         of range, nothing is removed  and false is returned.
		 * @note The corresponding AnListClass instance is <i>not</i>
		 *       destroyed!
		 */
		bool removeRow(unsigned int);

		/**
		 * Removes a range of rows from the list.
		 *
		 * first needs to be smaller than last and last must not be out
		 * of range. If the condition is not fulfilled, nothing is
		 * removed from the list.
		 *
		 * @param first First index of range
		 * @param last Last index of range
		 * @return On success true is returned. If
		 *         <code>first > last</code> or last is out of range,
		 *         nothing is removed and false is returned.
		 * @note The corresponding AnListClass instances are <i>not</i>
		 *       destroyed!
		 */
		bool removeRows(unsigned int, unsigned int);

		/**
		 * Removes all rows from the list.
		 *
		 * @note The corresponding AnListClass instances are <i>not</i>
		 *       destroyed!
		 */
		void clearRows(void);

		/**
		 * Implementation of AnRowProvider::getRow()
		 * @param idx The row index.
		 * @return The AnListClass object associated with the index.
		 */
		AnListClass *getRow(unsigned int idx) const;

		/**
		 * Implementation of AnRowProvider::getSize()
		 * @return The total number of rows managed by this class.
		 */
		int getSize(void) const;

		/**
		 * Returns the row-index of the specified AnListClass instance.
		 *
		 * @param c The requested AnListClass instance
		 * @return Index of specified AnListClass instance. If the
		 *         object is not registered here, -1 is returned.
		 */
		int getRowIndex(AnListClass *) const;

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
		 * List of rows.
		 *
		 * Used to select a row of the list in constant time.
		 */
		std::vector<AnListClass *>	rows_;

		/**
		 * Reverse map of rows.
		 *
		 * Maps an AnListClass instance to an index in rows_-attribute.
		 * For efficiency a map is used. You can find out the index by
		 * walking through the rows_-list in linear time. But searching
		 * for an index in a map only takes logarithmical time.
		 */
		std::map<AnListClass *, unsigned int> reverseRows_;
};

#endif	/* _ANGENERICROWPROVIDER_H_ */

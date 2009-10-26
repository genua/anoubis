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

#ifndef _ANLISTCTRL_H_
#define _ANLISTCTRL_H_

#include <list>
#include <map>
#include <vector>

#include <wx/listctrl.h>

#include "Observer.h"

class AnListClass;
class AnListClassProperty;
class AnListColumn;
class AnListProperty;

/**
 * A general list used all over the Anoubis-application.
 *
 * This list combines several features used by all lists. It brings a general
 * interface for connecting data with a list.
 *
 * Columns of the list are represented by the AnListColumn class. Use
 * addColumn() to append a column to the list.
 *
 * Rows of the list are represented by the AnListClass class. Use addRow()
 * (or one of the similar methods) to append a row to the list.
 *
 * An AnListClass consists of several properties (represented by
 * AnListProperty). A column of the list displays a property. That's why, an
 * AnListProperty instance is assigned to the column.
 *
 * A complete AnListClass is described by AnListClassProperty. This kind of
 * property describes a single row (which contains an AnListClass).
 *
 * When the list needs to fill a cell of the table, it selects the AnListClass
 * of the row and the AnListColumn of the column to receive the AnListProperty.
 * Then it asks the property to return information to fill out the cell. The
 * formatting options of a complete row are received by asking the assigned
 * AnListClassProperty.
 *
 * The list can save its state. The feature is enabled by providing a
 * configuration key (setStateKey()). Properties, such as width of columns,
 * are dumped below this key and restored again, when the application is
 * restarted.
 *
 * By default the list works old-fashioned, you need to fill the list manually
 * by hand. To enable all the features of the class, you needs to specify the
 * wxLC_VIRTUAL-flag as a window-style!
 */
class AnListCtrl : public wxListCtrl, private Observer
{
	public:
		/**
		 * Constructor, creating and showing a list control.
		 *
		 * @param parent Parent window. Must not be NULL.
		 * @param id Window identifier. A value of -1 indicates a
		 *           default value.
		 * @param pos Window position.
		 * @param size Window size. If the default size (-1, -1) is
		 *             specified then the window is sized
		 *             appropriately.
		 * @param style Window style.
		 * @param validator Window validator.
		 * @param name Window name.
		 */
		AnListCtrl(wxWindow *parent, wxWindowID id,
		    const wxPoint &pos = wxDefaultPosition,
		    const wxSize &size = wxDefaultSize,
		    long style = wxLC_ICON,
		    const wxValidator &validator = wxDefaultValidator,
		    const wxString &name = wxListCtrlNameStr);

		/**
		 * D'tor.
		 */
		~AnListCtrl(void);

		/**
		 * Returns the configuration key of the list.
		 *
		 * The list dumps its state-information below this key. If the
		 * application is restarted, the state is restored.
		 *
		 * @return Configuration key containing state-information
		 */
		wxString getStateKey(void) const;

		/**
		 * Assigns a configuration key to the list.
		 *
		 * If assigned, dumping/restoring of state-information is
		 * enabled.
		 *
		 * @param key Path of key
		 */
		void setStateKey(const wxString &);

		/**
		 * Appends a column to the list.
		 *
		 * The column is appended at the end of the column-list.
		 *
		 * @param property The property to be displayed at the column.
		 * @return The new column.
		 * @note The column is now the owner of the specified property.
		 *       If the column is removed or the complete list is
		 *       destroyed, the property is also destroyed.
		 */
		AnListColumn *addColumn(AnListProperty *);

		/**
		 * Returns the column at the specified index.
		 *
		 * @param idx The index of the column to be returned.
		 * @return Column at the specified index. If the index is out
		 *         of range, NULL is returned.
		 */
		AnListColumn *getColumn(unsigned int) const;

		/**
		 * Removes a column from the list.
		 *
		 * Note, that the assigned AnListProperty is also destroyed!
		 *
		 * @param idx Index of column to be removed.
		 * @return true on success, false if the specified index is out
		 *         of range.
		 */
		bool removeColumn(unsigned int);

		/**
		 * Returns the number of assigned columns.
		 *
		 * @return Number of columns.
		 */
		unsigned int getColumnCount(void) const;

		/**
		 * Tests whether the specified column is visible.
		 *
		 * @param col Column to test
		 * @return true if visible, false otherwise.
		 */
		bool isColumnVisible(const AnListColumn *) const;

		/**
		 * Shows/hides the specified column.
		 *
		 * @param col Column to show/hide
		 * @param visible If set to true, the column is shown,
		 *                otherwise it is hidden.
		 */
		void setColumnVisible(AnListColumn *, bool);

		/**
		 * Returns a list with all assigned rows.
		 *
		 * @return All available rows.
		 */
		const std::vector<AnListClass *> getRowData(void) const;

		/**
		 * Returns the number of assigned rows.
		 *
		 * @return Number of assigned rows.
		 */
		unsigned int getRowCount(void) const;

		/**
		 * Assigns rows to the list.
		 *
		 * Any previously assigns rows are removed before.
		 *
		 * @param rowData Data to be appended to the list
		 * @see clearRows()
		 */
		void setRowData(const std::vector<AnListClass *> &);

		/**
		 * Assigns rows to the list.
		 *
		 * Any previously assigns rows are removed before.
		 *
		 * @param rowData Data to be appended to the list
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
		 * Returns the AnListClass instance at the specified row-index.
		 *
		 * @param idx Row-index of AnListClass instance to be returned.
		 * @return The requested AnListClass instance. If the specified
		 *         row-index is out of range, NULL is returned.
		 */
		AnListClass *getRowAt(unsigned int) const;

		/**
		 * Returns the row-index of the specified AnListClass instance.
		 *
		 * @param c The requested AnListClass instance
		 * @return Index of specified AnListClass instance. If the
		 *         object is not registered here, -1 is returned.
		 */
		int getRowIndex(AnListClass *) const;

		/**
		 * Returns the property used to format the rows of the table.
		 *
		 * @return Property used to format the rows. If NULL is
		 *         returned, the default formatting options are used.
		 */
		AnListClassProperty *getRowProperty(void) const;

		/**
		 * Assigns a row-property to the table.
		 *
		 * This property is used to format a complete AnListClass
		 * (aka row). If no AnListClassProperty is assigned, the
		 * default formatting options are used.
		 *
		 * @param property The row-property to be assigned
		 * @note The list-control is now the owner of the specified
		 *       property. If the list is removed, the property is also
		 *       destroyed.
		 */
		void setRowProperty(AnListClassProperty *);

		/**
		 * Refreshes the visible area of the list.
		 */
		void refreshVisible(void);

		/**
		 * Returns true if at least one line in the list is
		 * selected, false otherwise. This function has an
		 * average runtime that should be constant. However,
		 * individual invocations can still have linear runtime.
		 * @param None.
		 * @return True if a selection exists.
		 */
		bool hasSelection(void);

		/**
		 * Returns the index of the first selected row.
		 *
		 * Do not use this function to test if a selection exists.
		 * This can cause severe Hangs with large lists because the
		 * average runtime of this function is linear in the number
		 * of elements in the list.
		 *
		 * If nothing is selected, -1 is returned.
		 *
		 * @return Index of first selected row.
		 */
		int getFirstSelection(void) const;

		/**
		 * Returns the index of a selected row starting at the given
		 * index.
		 *
		 * Searches for an selected item, starting from previous but
		 * excluding the item itself. If previous is -1, the first
		 * selected item will be returned.
		 *
		 * The search is linear in the number of elements in the
		 * list.
		 *
		 * @param previous Previous index. Where to start searching
		 * @return Index of next selection or -1 if no selection was
		 *         found.
		 */
		int getNextSelection(int) const;

	protected:
		/**
		 * Overloaded wxListCtrl::OnGetItemText().
		 *
		 * Identifies the text to be displayed at the specified cell.
		 *
		 * @note The method is defined virtual in the base-class but it
		 *       is not practical to overwrite this method again, if
		 *       you derivate from this class.
		 */
		wxString OnGetItemText(long, long) const;

		/**
		 * Overloaded wxListCtrl::OnGetItemColumnImage().
		 *
		 * Identifies the icon to be displayed at the specified cell.
		 *
		 * @note The method is defined virtual in the base-class but it
		 *       is not practical to overwrite this method again, if
		 *       you derivate from this class.
		 */
		int OnGetItemColumnImage(long, long) const;

		/**
		 * Overloader wxListCtrl::OnGetItemAttr().
		 *
		 * Specifies item attributes the for given item.
		 *
		 * @note The method is defined virtual in the base-class but it
		 *       is not practical to overwrite this method again, if
		 *       you derivate from this class.
		 */
		wxListItemAttr *OnGetItemAttr(long) const;

	private:
		/**
		 * Path of key, where state-information are dumped.
		 */
		wxString stateKey_;

		/**
		 * List of columns.
		 */
		std::vector<AnListColumn *> columnList_;

		/**
		 * Lists all visible columns.
		 *
		 * Contains indexes pointing into the columnList_-attribute.
		 */
		std::vector<unsigned int> visibleColumns_;

		/**
		 * List of rows.
		 *
		 * Used for selecting a row of the list in constant time.
		 */
		std::vector<AnListClass *> rows_;

		/**
		 * Reverse map of rows.
		 *
		 * Maps an AnListClass instance to an index in rows_-attribute.
		 * For efficiency a map is used. You can find out the index by
		 * walking through the rows_-list in linear time. But searching
		 * for an index in a map only takes logarithmical time.
		 */
		std::map<AnListClass *, unsigned int> reverseRows_;

		/**
		 * Row-property (if any).
		 */
		AnListClassProperty *rowProperty_;

		/**
		 * The item-attribute setup and returned by OnGetItemAttr().
		 */
		wxListItemAttr *itemAttr_;

		/**
		 * The last element that was found to be selected by
		 * hasSelection(). The function hasSelection() optimizes
		 * its search by starting at this element instead of the
		 * beginning of the list.
		 */
		int	hasSelectionResult_;

		/**
		 * Implementation of Observer::update().
		 *
		 * Simply refreshes the view.
		 */
		void update(Subject *);

		/**
		 * Implementation of Observer::updateDelete().
		 *
		 * Removes the object from the list.
		 */
		void updateDelete(Subject *);

		/**
		 * Inserts a new entry into the list of visible columns
		 * (visibleColumns_).
		 *
		 * @param value Value to be inserted. Usually
		 *              AnListColumn::getIndex().
		 * @return Index in visibleColumns_, where the value was
		 *         inserted.
		 */
		unsigned int insertVisible(unsigned int);

		/**
		 * Removes an entry from the list of visible columns
		 * (visibleColumns_).
		 *
		 * @param value Value to be removed. Usually
		 *              AnListColumn::getIndex().
		 * @return Index in visibleColumns_, where the value was
		 *         previously stored. A return-value of -1 means, that
		 *         value is not stored in the list.
		 */
		int removeVisible(unsigned int);

		/**
		 * Returns the selected index or -1 if nothing is selected.
		 */
		long getSelectedIndex(void);
};

#endif	/* _ANLISTCTRL_H_ */

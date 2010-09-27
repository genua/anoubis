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

#include "AnRowProvider.h"
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
class AnListCtrl : public wxListCtrl, public Observer
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
		 * @param width The width of the column.
		 * @param align The alignment of the column.
		 * @return The logical index of the column. Use this for
		 *     subsequent calls to functions that expect a column
		 *     index.
		 * @note The column is now the owner of the specified property.
		 *       If the column is removed or the complete list is
		 *       destroyed, the property is also destroyed.
		 */
		void addColumn(AnListProperty *, int width,
		    wxListColumnFormat align = wxLIST_FORMAT_LEFT);

		/**
		 * Shows/hides the specified column.
		 *
		 * @param idx Index of the column to show/hide
		 * @param visible If set to true, the column is shown,
		 *                otherwise it is hidden.
		 */
		void setColumnVisible(unsigned int idx, bool visible);

		/**
		 * Returns the number of assigned rows.
		 *
		 * @return Number of assigned rows.
		 */
		unsigned int getRowCount(void) const;

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
		 * Clear the entire selection of the list. This function
		 * is linear in the number of list item, i.e. it should
		 * be avoided for lists that can contain many elements.
		 *
		 * @param None.
		 * @return None.
		 */
		void clearSelection(void);

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

		/**
		 * Assign a row provider to the list. The data displayed
		 * in the list will then come from this data model.
		 * @param provider The new row provider.
		 */
		void setRowProvider(AnRowProvider *provider);

		/**
		 * Shows a choice dialog that allows to hide and show columns
		 * of this AnListCtrl.
		 * @return Nothing.
		 */
		void showColumnVisibilityDialog(void);

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
		 * Overloaded wxListCtrl::OnGetItemAttr().
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
		 * Handle anEVT_ROW_SIZECHANGE events sent by the row provider
		 * @param event The size change event sent by the provider.
		 * @return None.
		 */
		void onSizeChange(wxCommandEvent &event);

		/**
		 * Handle anEVT_ROW_UPDATE events sent by the row provider
		 * @param event The event sent by the provider.
		 * @return None.
		 */
		void onRowUpdate(wxCommandEvent &event);

		/**
		 * Handle a column size change event.
		 *
		 * @param event The event sent by the list control.
		 * @return None.
		 */
		void onColumnSize(wxListEvent &event);

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
		 * Implementation of the Observer interface.
		 *
		 * @param subject The subject.
		 * @return None.
		 */
		void update(Subject *);

		/**
		 * Implementation of the Observer interface.
		 *
		 * @param subject The subject.
		 * @return None.
		 */
		void updateDelete(Subject *);

		AnRowProvider	*rowProvider_;
};

#endif	/* _ANLISTCTRL_H_ */
